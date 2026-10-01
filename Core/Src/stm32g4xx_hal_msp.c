/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : stm32g4xx_hal_msp.c
  * @brief          : This file provides code for the MSP Initialization
  *                   and de-Initialization codes.
  ******************************************************************************
  * @attention
  *
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "tps55289.hpp"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/* MSP Initialization */
/******************************************************************************/

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @brief QSPI MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hqspi: QSPI handle pointer
  * @retval None
  */
void HAL_QSPI_MspInit(QSPI_HandleTypeDef* hqspi)
{
  /* USER CODE BEGIN QSPI_MspInit 0 */

  /* USER CODE END QSPI_MspInit 0 */
    /* USER CODE BEGIN QSPI_MspInit 1 */

    /* USER CODE END QSPI_MspInit 1 */
}

/**
  * @brief QSPI MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hqspi: QSPI handle pointer
  * @retval None
  */
void HAL_QSPI_MspDeInit(QSPI_HandleTypeDef* hqspi)
{
  /* USER CODE BEGIN QSPI_MspDeInit 0 */

  /* USER CODE END QSPI_MspDeInit 0 */
    /* USER CODE BEGIN QSPI_MspDeInit 1 */

    /* USER CODE END QSPI_MspDeInit 1 */
}

/**
  * @brief I2C MSP Initialization
  * This function configures the hardware resources used in this example
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
  /* USER CODE BEGIN I2C_MspInit 0 */

  /* USER CODE END I2C_MspInit 0 */
    /* USER CODE BEGIN I2C_MspInit 1 */

    /* USER CODE END I2C_MspInit 1 */
}

/**
  * @brief I2C MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hi2c: I2C handle pointer
  * @retval None
  */
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  /* USER CODE BEGIN I2C_MspDeInit 0 */

  /* USER CODE END I2C_MspDeInit 0 */
    /* USER CODE BEGIN I2C_MspDeInit 1 */

    /* USER CODE END I2C_MspDeInit 1 */
}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
