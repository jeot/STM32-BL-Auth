/**
  ******************************************************************************
  * @file    Boot/Src/jumper.c
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32L4xx HAL API.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "jumper.h"
#include "securitycheck_boot.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
#if defined (__ICCARM__)
extern uint32_t __ICFEDIT_region_RAM_start__;
#define BL_SRAM1_START ((uint32_t)& __ICFEDIT_region_RAM_start__)
extern uint32_t __ICFEDIT_region_RAM_end__ ;
#define BL_SRAM1_END ((uint32_t)& __ICFEDIT_region_RAM_end__)
#elif defined (__CC_ARM)
extern uint32_t Image$$ER_IROM1$$Limit;
extern uint32_t Image$$RW_IRAM1$$Base;
extern uint32_t Image$$RW_IRAM1$$Limit;
#define BL_SRAM1_START       ((uint32_t)&Image$$RW_IRAM1$$Base)
#define BL_SRAM1_END         ((uint32_t)&Image$$RW_IRAM1$$Limit)
#elif defined(__GNUC__)
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;
// extern uint32_t _eram;
// #define BL_SRAM1_START ((uint32_t)&_sdata)
// #define BL_SRAM1_END ((uint32_t)&_eram)
#endif

// #define SRAM_Erase() \
//   {\
//     uint32_t *pRam;\
//     for (pRam = (uint32_t *)BL_SRAM1_START; pRam < (uint32_t *)BL_SRAM1_END; pRam++)\
//     {\
//       *pRam = 0U;\
//     }\
//   }

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void jump_to_application(void);

  /**
  * @brief  A new UserApp Fw was available. Start to download it
  * @param  None
  * @note   This function must set the next State Machine State
  * @retval None
  */
void jump_to_application(void)
{
  printf("@jump_to_application()\n");
#if !(defined(SECURE_USER_PROTECT_ENABLE))
  printf("SECURE_USER_PROTECT_ENABLE not defined!\n");
  uint32_t jump_address;
  typedef void (*Function_Pointer)(void);
  Function_Pointer  p_jump_to_function;

  /* Destroy the Volatile data and CSTACK in SRAM used by Secure Boot in order to prevent any access to sensitive data
     from the loader.
  */
  // SRAM_Erase();

  // uint32_t test = 0;
  // printf("_sdata: %08x\n", &_sdata);
  // printf("_edata: %08x\n", &_edata);
  // printf("_sbss: %08x\n", &_sbss);
  // printf("_ebss: %08x\n", &_ebss);
  // printf("p_jump_to_function: %08x\n", &p_jump_to_function);
  // printf("jump_address: %08x\n", &jump_address);
  // printf("test: %08x\n", &test);
  // while(1);

  /* Initialize address to jump */
  jump_address = *(__IO uint32_t *)(((uint32_t)APP_REGION_ROM_START + 4));
  p_jump_to_function = (Function_Pointer) jump_address;

  printf("setting sp...\n");
  /* Initialize loader's Stack Pointer */
  __set_MSP(*(__IO uint32_t *)(APP_REGION_ROM_START));

  printf("jumping...\n");
  /* Jump into loader */
  p_jump_to_function();

  printf("jumped! not reachable!\n");

#else
  printf("SECURE_USER_PROTECT_ENABLE defined!\n");

#define BL_EXIT_STICKY 0x1FFF6800
#define MAGIC_NUMBER   0x08192A3C

#define APPLICATION_ADDRESS APP_REGION_ROM_START

  {
    //R0: BL_EXIT_STICKY vector table address
    //R1: magic number
    //R2: application address
    typedef  void (*pFunction)(uint32_t a, uint32_t b, uint32_t c);

    pFunction JumpToApplication;
    uint32_t JumpAddress;

    //SRAM_Erase();

    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (BL_EXIT_STICKY + 4);
    JumpToApplication = (pFunction) JumpAddress;
    JumpToApplication(JumpAddress,MAGIC_NUMBER,APPLICATION_ADDRESS);

  }

#endif /* SECURE_USER_PROTECT_ENABLE */


  /* The point below should NOT be reached */
  return;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
