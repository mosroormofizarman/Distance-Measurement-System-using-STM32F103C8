#include "main.h"
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c1;
TIM_HandleTypeDef htim2;
UART_HandleTypeDef huart1;

volatile uint32_t echo_start = 0;
volatile uint32_t echo_end = 0;
volatile uint8_t echo_captured = 0;
volatile float distance = 0.0;

#define TRIG_PORT GPIOA
#define TRIG_PIN GPIO_PIN_0
#define LED_PORT GPIOA
#define LED_PIN GPIO_PIN_2

#define SOUND_SPEED 343.0
#define THRESHOLD_DISTANCE 20.0

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);

void Delay_us(uint32_t us);
void HC_SR04_Init(void);
void Measure_Distance(void);
void Display_Distance_on_LCD(void);
void Check_Threshold(void);

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
        {
            if (echo_captured == 0)
            {
                echo_start = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
                echo_captured = 1;
            }
            else
            {
                echo_end = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
                echo_captured = 2;
            }
        }
    }
}

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();

  lcd_init(&hi2c1);
  lcd_put_cur(0, 0);
  lcd_send_string("Distance Meter");
  lcd_put_cur(1, 0);
  lcd_send_string("Ready...");
  
  HC_SR04_Init();
  
  HAL_Delay(2000);
  
  lcd_clear();
  
  while (1)
  {
    Measure_Distance();
    Display_Distance_on_LCD();
    Check_Threshold();
    
    HAL_Delay(500);
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 71;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 65535;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_2, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Delay_us(uint32_t us)
{
  uint32_t start = SysTick->VAL;
  uint32_t ticks = (us * (SystemCoreClock / 1000000)) / 8;
  
  while ((start - SysTick->VAL) < ticks)
  {
  }
}

void HC_SR04_Init(void)
{
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
}

void Measure_Distance(void)
{
  uint32_t echo_time = 0;
  
  echo_captured = 0;
  echo_start = 0;
  echo_end = 0;
  
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
  Delay_us(10);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
  
  uint32_t timeout = 0;
  while (echo_captured < 2 && timeout < 100000)
  {
    timeout++;
  }
  
  if (echo_captured == 2)
  {
    if (echo_end > echo_start)
    {
      echo_time = echo_end - echo_start;
    }
    else
    {
      echo_time = (65536 - echo_start) + echo_end;
    }
    
    distance = (SOUND_SPEED * echo_time) / (2.0 * 10000.0);
  }
}

void Display_Distance_on_LCD(void)
{
  char buffer[32];
  
  lcd_clear();
  lcd_put_cur(0, 0);
  lcd_send_string("Dist:");
  
  if (distance < 0 || distance > 400)
  {
    lcd_put_cur(0, 6);
    lcd_send_string("Out of range");
  }
  else
  {
    sprintf(buffer, "%.1f cm", distance);
    lcd_put_cur(0, 6);
    lcd_send_string(buffer);
  }
  
  lcd_put_cur(1, 0);
  lcd_send_string("Alert:");
  
  if (distance < THRESHOLD_DISTANCE)
  {
    lcd_put_cur(1, 7);
    lcd_send_string("ON");
  }
  else
  {
    lcd_put_cur(1, 7);
    lcd_send_string("OFF");
  }
}

void Check_Threshold(void)
{
  if (distance < THRESHOLD_DISTANCE && distance > 0)
  {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif