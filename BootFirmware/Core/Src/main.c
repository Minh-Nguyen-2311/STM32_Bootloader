/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "iwdg.h"
#include "lwip.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "flash_if.h"
#include "dp83848.h"
#include <stdbool.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEM_MEMORY_START			ADDR_FLASH_SECTOR_5											/* Start @ of user Flash area */
#define TEMP_MEMORY_STOP    		ADDR_FLASH_SECTOR_7  +  GetSectorSize(FLASH_SECTOR_7) -1 	/* End @ of user Flash area : sector start address + sector size -1 */
#define INFORMATION_MEMORY			ADDR_FLASH_SECTOR_11
#define MAIN_MEMORY					ADDR_FLASH_SECTOR_8

#define VALID_LINK_ESTABLISHED		0x0001

#define UPDATE_FIRMWARE_DETECT		0x55
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern dp83848_Object_t DP83848;
extern struct netif gnetif;
uint32_t save_add = ADDR_FLASH_SECTOR_11;
uint32_t num_word = 0;
uint8_t ret;
volatile bool updateEvent = false;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
static void JumpToMain(void);
uint8_t CheckFirmwareUpdateRequest(void);
void Flag_UpdateFOTA(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /*Activate mask fault*/
  SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
  SCB->SHCSR |= SCB_SHCSR_BUSFAULTACT_Msk;
  SCB->SHCSR |= SCB_SHCSR_USGFAULTENA_Msk;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  ethernetif_init(&gnetif);
  /*Check ethernet connection*/
  uint32_t regvalue = 0;

  while (!(regvalue & VALID_LINK_ESTABLISHED))
  {
	  HAL_ETH_ReadPHYRegister(&heth, DP83848_PHY_ADDRESS, PHY_SR, &regvalue);
	  printf("Plug in Ethernet Jack\n\r");
	  HAL_Delay(100);
  }
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LWIP_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  uint32_t tick_current = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  tick_current = HAL_GetTick();
	  while(HAL_GetTick() - tick_current < 5000) //Loop in Bootloader Firmware for 5s
	  {
		  MX_LWIP_Process();
		  ret = CheckFirmwareUpdateRequest();
		  switch(ret)
		  {
		  case 0:
			  printf("False state");
			  Error_Handler();
			  break;
		  case 1:
			  Flag_UpdateFOTA();
			  JumpToMain();
			  break;
		  case 2:
			  updateEvent = true;
			  //Dummy OTA event handling here
			  break;
		  default:
			  break;
		  }
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
uint8_t dummy_event_1 = 0, dummy_event_2 = 0;
uint8_t CheckFirmwareUpdateRequest(void)
{
	//Implement your OTA checking event here
	if(dummy_event_1){ //Suppose this is normal boot->main operation
		return 1;
	}
	if(dummy_event_2){ //Suppose this is OTA handling state
		return 2;
	}
	else	//Error handler
		return 0;
}

void Flag_UpdateFOTA(void)
{
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_11, FLASH_VOLTAGE_RANGE_3);
	FLASH_FlushCaches();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t) INFORMATION_MEMORY, UPDATE_FIRMWARE_DETECT); //enter FOTA firmware
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, (uint32_t) INFORMATION_MEMORY + 4, num_word); //the number of data
	HAL_FLASH_Lock();
}

static void JumpToMain(void)
{
	uint32_t dest_addr, source_addr;
	uint32_t num_ww = 0;

	uint8_t flag = *(__IO uint32_t*)save_add;
	//Check flag, if 0x55 then update firmware
	if (flag == UPDATE_FIRMWARE_DETECT) {
		printf("New firmware\n\r");
		uint32_t length = *(__IO uint32_t*) (save_add + 4);
		printf("-> Delete main program\n\r");
		//Store new firmware
		Flash_If_Erase(MAIN_MEMORY, INFORMATION_MEMORY);
		HAL_FLASH_Unlock();
		dest_addr = MAIN_MEMORY;
		source_addr = TEM_MEMORY_START;
		printf("-> Write main program\n\r");
		while (num_ww < length) {
			if (HAL_FLASH_Program(TYPEPROGRAM_WORD, dest_addr, (*(__IO uint32_t*) source_addr)) == HAL_OK) {
				dest_addr += 4;
				source_addr += 4;
				num_ww++;
			} else
				Error_Handler();
		}
		HAL_FLASH_Lock();
		printf("-> Write success\n\r");
		//Erase flag after done writting to main memory in case of power loss event
		Flash_If_Erase(save_add, save_add);
	}
	printf("Proceeding to main memory now");

	SysTick->CTRL = 0x00; //Disables SysTick timer and its related interrupt
	SCB->SHCSR &= ~( SCB_SHCSR_USGFAULTENA_Msk |\
			SCB_SHCSR_BUSFAULTENA_Msk |
			SCB_SHCSR_MEMFAULTENA_Msk);
	// Set Stack Pointer Position
	__set_MSP(*((volatile uint32_t*)MAIN_MEMORY));
	__DMB(); //ARM says to use a DMB instruction before relocating VTOR */
	// Set Program Counter Position
	uint32_t JumpAddress = *((volatile uint32_t*) (MAIN_MEMORY + 4));
	void (*reset_handler)(void) = (void *)JumpAddress;
	reset_handler(); // jump to the main program
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
