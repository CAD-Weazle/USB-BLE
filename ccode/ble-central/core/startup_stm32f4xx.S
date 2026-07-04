  .syntax unified
  .cpu cortex-m3
  .fpu softvfp
  .thumb

.global vectors
.global Default_Handler


.word _sidata

.word _sdata

.word _edata

.word _sbss

.word _ebss
  .section .text
  .weak Reset_Handler
  .type Reset_Handler, %function

Reset_Handler:

  movs r1, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r3, =_sidata
  ldr r3, [r3, r1]
  str r3, [r0, r1]
  adds r1, r1, #4

LoopCopyDataInit:
  ldr r0, =_sdata
  ldr r3, =_edata
  adds r2, r0, r1
  cmp r2, r3
  bcc CopyDataInit
  ldr r2, =_sbss
  b LoopFillZerobss

FillZerobss:
  movs r3, #0
  str r3, [r2], #4

LoopFillZerobss:
  ldr r3, = _ebss
  cmp r2, r3
  bcc FillZerobss

  bl main
  bx lr
.size Reset_Handler, .-Reset_Handler

    .section .text,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
  .size Default_Handler, .-Default_Handler



  .section .vectors,"a",%progbits

vectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler


  .word WWDG_irq
  .word PVD_irq
  .word TAMP_STAMP_irq
  .word RTC_WKUP_irq
  .word FLASH_irq
  .word RCC_irq
  .word EXTI0_irq
  .word EXTI1_irq
  .word EXTI2_irq
  .word EXTI3_irq
  .word EXTI4_irq
  .word DMA1_Stream0_irq
  .word DMA1_Stream1_irq
  .word DMA1_Stream2_irq
  .word DMA1_Stream3_irq
  .word DMA1_Stream4_irq
  .word DMA1_Stream5_irq
  .word DMA1_Stream6_irq
  .word ADC_irq
  .word CAN1_TX_irq
  .word CAN1_RX0_irq
  .word CAN1_RX1_irq
  .word CAN1_SCE_irq
  .word EXTI9_5_irq
  .word TIM1_BRK_TIM9_irq
  .word TIM1_UP_TIM10_irq
  .word TIM1_TRG_COM_TIM11_irq
  .word TIM1_CC_irq
  .word TIM2_irq
  .word TIM3_irq
  .word TIM4_irq
  .word I2C1_EV_irq
  .word I2C1_ER_irq
  .word I2C2_EV_irq
  .word I2C2_ER_irq
  .word SPI1_irq
  .word SPI2_irq
  .word USART1_irq
  .word USART2_irq
  .word USART3_irq
  .word EXTI15_10_irq
  .word RTC_Alarm_irq
  .word OTG_FS_WKUP_irq
  .word TIM8_BRK_TIM12_irq
  .word TIM8_UP_TIM13_irq
  .word TIM8_TRG_COM_TIM14_irq
  .word TIM8_CC_irq
  .word DMA1_Stream7_irq
  .word FSMC_irq
  .word SDIO_irq
  .word TIM5_irq
  .word SPI3_irq
  .word UART4_irq
  .word UART5_irq
  .word TIM6_DAC_irq
  .word TIM7_irq
  .word DMA2_Stream0_irq
  .word DMA2_Stream1_irq
  .word DMA2_Stream2_irq
  .word DMA2_Stream3_irq
  .word DMA2_Stream4_irq
  .word ETH_irq
  .word ETH_WKUP_irq
  .word CAN2_TX_irq
  .word CAN2_RX0_irq
  .word CAN2_RX1_irq
  .word CAN2_SCE_irq
  .word OTG_FS_irq
  .word DMA2_Stream5_irq
  .word DMA2_Stream6_irq
  .word DMA2_Stream7_irq
  .word USART6_irq
  .word I2C3_EV_irq
  .word I2C3_ER_irq
  .word OTG_HS_EP1_OUT_irq
  .word OTG_HS_EP1_IN_irq
  .word OTG_HS_WKUP_irq
  .word OTG_HS_irq
  .word DCMI_irq
  .word CRYP_irq
  .word HASH_RNG_irq
  .word FPU_irq



   .weak NMI_Handler
   .thumb_set NMI_Handler,Default_Handler

   .weak HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler

   .weak MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler

   .weak BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler

   .weak WWDG_irq
   .thumb_set WWDG_irq,Default_Handler

   .weak PVD_irq
   .thumb_set PVD_irq,Default_Handler

   .weak TAMP_STAMP_irq
   .thumb_set TAMP_STAMP_irq,Default_Handler

   .weak RTC_WKUP_irq
   .thumb_set RTC_WKUP_irq,Default_Handler

   .weak FLASH_irq
   .thumb_set FLASH_irq,Default_Handler

   .weak RCC_irq
   .thumb_set RCC_irq,Default_Handler

   .weak EXTI0_irq
   .thumb_set EXTI0_irq,Default_Handler

   .weak EXTI1_irq
   .thumb_set EXTI1_irq,Default_Handler

   .weak EXTI2_irq
   .thumb_set EXTI2_irq,Default_Handler

   .weak EXTI3_irq
   .thumb_set EXTI3_irq,Default_Handler

   .weak EXTI4_irq
   .thumb_set EXTI4_irq,Default_Handler

   .weak DMA1_Stream0_irq
   .thumb_set DMA1_Stream0_irq,Default_Handler

   .weak DMA1_Stream1_irq
   .thumb_set DMA1_Stream1_irq,Default_Handler

   .weak DMA1_Stream2_irq
   .thumb_set DMA1_Stream2_irq,Default_Handler

   .weak DMA1_Stream3_irq
   .thumb_set DMA1_Stream3_irq,Default_Handler

   .weak DMA1_Stream4_irq
   .thumb_set DMA1_Stream4_irq,Default_Handler

   .weak DMA1_Stream5_irq
   .thumb_set DMA1_Stream5_irq,Default_Handler

   .weak DMA1_Stream6_irq
   .thumb_set DMA1_Stream6_irq,Default_Handler

   .weak ADC_irq
   .thumb_set ADC_irq,Default_Handler

   .weak CAN1_TX_irq
   .thumb_set CAN1_TX_irq,Default_Handler

   .weak CAN1_RX0_irq
   .thumb_set CAN1_RX0_irq,Default_Handler

   .weak CAN1_RX1_irq
   .thumb_set CAN1_RX1_irq,Default_Handler

   .weak CAN1_SCE_irq
   .thumb_set CAN1_SCE_irq,Default_Handler

   .weak EXTI9_5_irq
   .thumb_set EXTI9_5_irq,Default_Handler

   .weak TIM1_BRK_TIM9_irq
   .thumb_set TIM1_BRK_TIM9_irq,Default_Handler

   .weak TIM1_UP_TIM10_irq
   .thumb_set TIM1_UP_TIM10_irq,Default_Handler

   .weak TIM1_TRG_COM_TIM11_irq
   .thumb_set TIM1_TRG_COM_TIM11_irq,Default_Handler

   .weak TIM1_CC_irq
   .thumb_set TIM1_CC_irq,Default_Handler

   .weak TIM2_irq
   .thumb_set TIM2_irq,Default_Handler

   .weak TIM3_irq
   .thumb_set TIM3_irq,Default_Handler

   .weak TIM4_irq
   .thumb_set TIM4_irq,Default_Handler

   .weak I2C1_EV_irq
   .thumb_set I2C1_EV_irq,Default_Handler

   .weak I2C1_ER_irq
   .thumb_set I2C1_ER_irq,Default_Handler

   .weak I2C2_EV_irq
   .thumb_set I2C2_EV_irq,Default_Handler

   .weak I2C2_ER_irq
   .thumb_set I2C2_ER_irq,Default_Handler

   .weak SPI1_irq
   .thumb_set SPI1_irq,Default_Handler

   .weak SPI2_irq
   .thumb_set SPI2_irq,Default_Handler

   .weak USART1_irq
   .thumb_set USART1_irq,Default_Handler

   .weak USART2_irq
   .thumb_set USART2_irq,Default_Handler

   .weak USART3_irq
   .thumb_set USART3_irq,Default_Handler

   .weak EXTI15_10_irq
   .thumb_set EXTI15_10_irq,Default_Handler

   .weak RTC_Alarm_irq
   .thumb_set RTC_Alarm_irq,Default_Handler

   .weak OTG_FS_WKUP_irq
   .thumb_set OTG_FS_WKUP_irq,Default_Handler

   .weak TIM8_BRK_TIM12_irq
   .thumb_set TIM8_BRK_TIM12_irq,Default_Handler

   .weak TIM8_UP_TIM13_irq
   .thumb_set TIM8_UP_TIM13_irq,Default_Handler

   .weak TIM8_TRG_COM_TIM14_irq
   .thumb_set TIM8_TRG_COM_TIM14_irq,Default_Handler

   .weak TIM8_CC_irq
   .thumb_set TIM8_CC_irq,Default_Handler

   .weak DMA1_Stream7_irq
   .thumb_set DMA1_Stream7_irq,Default_Handler

   .weak FSMC_irq
   .thumb_set FSMC_irq,Default_Handler

   .weak SDIO_irq
   .thumb_set SDIO_irq,Default_Handler

   .weak TIM5_irq
   .thumb_set TIM5_irq,Default_Handler

   .weak SPI3_irq
   .thumb_set SPI3_irq,Default_Handler

   .weak UART4_irq
   .thumb_set UART4_irq,Default_Handler

   .weak UART5_irq
   .thumb_set UART5_irq,Default_Handler

   .weak TIM6_DAC_irq
   .thumb_set TIM6_DAC_irq,Default_Handler

   .weak TIM7_irq
   .thumb_set TIM7_irq,Default_Handler

   .weak DMA2_Stream0_irq
   .thumb_set DMA2_Stream0_irq,Default_Handler

   .weak DMA2_Stream1_irq
   .thumb_set DMA2_Stream1_irq,Default_Handler

   .weak DMA2_Stream2_irq
   .thumb_set DMA2_Stream2_irq,Default_Handler

   .weak DMA2_Stream3_irq
   .thumb_set DMA2_Stream3_irq,Default_Handler

   .weak DMA2_Stream4_irq
   .thumb_set DMA2_Stream4_irq,Default_Handler

   .weak ETH_irq
   .thumb_set ETH_irq,Default_Handler

   .weak ETH_WKUP_irq
   .thumb_set ETH_WKUP_irq,Default_Handler

   .weak CAN2_TX_irq
   .thumb_set CAN2_TX_irq,Default_Handler

   .weak CAN2_RX0_irq
   .thumb_set CAN2_RX0_irq,Default_Handler

   .weak CAN2_RX1_irq
   .thumb_set CAN2_RX1_irq,Default_Handler

   .weak CAN2_SCE_irq
   .thumb_set CAN2_SCE_irq,Default_Handler

   .weak OTG_FS_irq
   .thumb_set OTG_FS_irq,Default_Handler

   .weak DMA2_Stream5_irq
   .thumb_set DMA2_Stream5_irq,Default_Handler

   .weak DMA2_Stream6_irq
   .thumb_set DMA2_Stream6_irq,Default_Handler

   .weak DMA2_Stream7_irq
   .thumb_set DMA2_Stream7_irq,Default_Handler

   .weak USART6_irq
   .thumb_set USART6_irq,Default_Handler

   .weak I2C3_EV_irq
   .thumb_set I2C3_EV_irq,Default_Handler

   .weak I2C3_ER_irq
   .thumb_set I2C3_ER_irq,Default_Handler

   .weak OTG_HS_EP1_OUT_irq
   .thumb_set OTG_HS_EP1_OUT_irq,Default_Handler

   .weak OTG_HS_EP1_IN_irq
   .thumb_set OTG_HS_EP1_IN_irq,Default_Handler

   .weak OTG_HS_WKUP_irq
   .thumb_set OTG_HS_WKUP_irq,Default_Handler

   .weak OTG_HS_irq
   .thumb_set OTG_HS_irq,Default_Handler

   .weak DCMI_irq
   .thumb_set DCMI_irq,Default_Handler

   .weak CRYP_irq
   .thumb_set CRYP_irq,Default_Handler

   .weak HASH_RNG_irq
   .thumb_set HASH_RNG_irq,Default_Handler

   .weak FPU_irq
   .thumb_set FPU_irq,Default_Handler
