/*
 * prototypes interrupt vectors
 */

#if defined ( __GNUC__ )
/* GCC */
#define PLATFORM_DEFINE_ISR( function ) \
        void function( void ); \
        __attribute__(( interrupt, used, section( IRQ_SECTION ) )) void function( void )

#define PLATFORM_SET_DEFAULT_ISR( irq_handler, default_handler ) \
        __attribute__(( weak, alias( #default_handler ))) void irq_handler ( void );

#elif defined ( __IAR_SYSTEMS_ICC__ )
/* IAR Systems */
#define PLATFORM_DEFINE_ISR( function ) \
        void function( void ); \
        __irq __root void function( void )

#define PLATFORM_SET_DEFAULT_ISR( irq_handler, default_handler ) \
        _Pragma( TO_STRING( weak irq_handler=default_handler ) )

#else

#define PLATFORM_DEFINE_ISR( name )
#define PLATFORM_SET_DEFAULT_ISR( irq_handler, default_handler )

#endif

// cortex system interrupts
void NMIException( void );              // Non Maskable Interrupt
void HardFaultException( void );        // Hard Fault interrupt
void MemManageException( void );        // Memory Management Fault interrupt
void BusFaultException( void );         // Bus Fault interrupt
void UsageFaultException( void );       // Usage Fault interrupt
void SVC_irq( void );                   // SVC interrupt
void DebugMonitor( void );              // Debug Monitor interrupt
void PENDSV_irq( void );                // PendSV interrupt
void SYSTICK_irq( void );               // Sys Tick Interrupt

// stm32 interrupts
void WWDG_irq( void );                  // Window WatchDog
void PVD_irq( void );                   // PVD through EXTI Line detection
void TAMP_STAMP_irq( void );            // Tamper and TimeStamps through the EXTI line
void RTC_WKUP_irq( void );              // RTC Wakeup through the EXTI line
void FLASH_irq( void );                 // FLASH
void RCC_irq( void );                   // RCC
void EXTI0_irq( void );                 // EXTI Line0
void EXTI1_irq( void );                 // EXTI Line1
void EXTI2_TS_irq( void );                 // EXTI Line2
void EXTI3_irq( void );                 // EXTI Line3
void EXTI4_irq( void );                 // EXTI Line4
void DMA1_CH1_irq( void );          // DMA1 CH 1
void DMA1_CH2_irq( void );          // DMA1 CH 2
void DMA1_CH3_irq( void );          // DMA1 CH 3
void DMA1_CH4_irq( void );          // DMA1 CH 4
void DMA1_CH5_irq( void );          // DMA1 CH 5
void DMA1_CH6_irq( void );          // DMA1 CH 6
void DMA1_CH7_irq( void );          // DMA1 CH 7
void ADC1_irq( void );                   // ADC1, ADC2 and ADC3s
void CAN_TX_irq( void );               // CAN1 TX
void CAN_RXD_irq( void );              // CAN1 RX0
void CAN_RXI_irq( void );              // CAN1 RX1
void CAN_SCE_irq( void );              // CAN1 SCE
void EXTI5_9_irq( void );               // External Line[9:5]s

void TIM2_irq( void );                  // TIM2
void TIM3_irq( void );                  // TIM3
void TIM4_irq( void );                  // TIM4
void TIM5_irq( void );                  // TIM4
void TIM6_DAC1_irq( void );              // TIM6 and DAC1&2 underrun errors
void TIM7_irq( void );                  // TIM4
void TIM12_irq( void );         // TIM1 Update and TIM10
void TIM13_irq( void );    // TIM1 Trigger and Commutation and TIM11
void TIM14_irq( void );    // TIM1 Trigger and Commutation and TIM11
void TIM15_irq( void );         // TIM1 Break and TIM9
void TIM16_irq( void );         // TIM1 Break and TIM9
void TIM17_irq( void );         // TIM1 Break and TIM9
void TIM18_DAC2_irq( void );         // TIM1 Break and TIM9
void TIM19_irq( void );         // TIM1 Break and TIM9

//void TIM1_CC_irq( void );               // TIM1 Capture Compare
void I2C1_EV_irq( void );               // I2C1 Event
void I2C1_ER_irq( void );               // I2C1 Error
void I2C2_EV_irq( void );               // I2C2 Event
void I2C2_ER_irq( void );               // I2C2 Error
void SPI1_irq( void );                  // SPI1
void SPI2_irq( void );                  // SPI2
void USART1_irq( void );                // USART1
void USART2_irq( void );                // USART2
void USART3_irq( void );                // USART3
void EXTI15_10_irq( void );             // External Line[15:10]s
void RTC_ALARM_IT_irq( void );             // RTC Alarm (A and B) through EXTI Line
void DMA1_CH7_irq( void );          // DMA1 CH7
void SPI3_irq( void );                  // SPI3
void UART4_irq( void );                 // UART4
void UART5_irq( void );                 // UART5
void DMA2_CH1_irq( void );          // DMA2 CH 0
void DMA2_CH2_irq( void );          // DMA2 CH 1
void DMA2_CH3_irq( void );          // DMA2 CH 2
void DMA2_CH4_irq( void );          // DMA2 CH 3
void DMA2_CH5_irq( void );          // DMA2 CH 4
void USART6_irq( void );                // USART6
void CEC_irq( void );                  // CRYP crypto
void FPU_irq( void );                  // CRYP crypto
void USB_LP_irq( void );
void USB_HP_irq( void );
void USB_WAKEUP_irq( void );
void SDADC1_irq( void );
void SDADC2_irq( void );
void SDADC3_irq( void );
void COMP1_2_irq( void );

