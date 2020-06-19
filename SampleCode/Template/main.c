/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * @brief    A project template for M031 MCU.
 *
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>

#include "NuMicro.h"
#include "I2C_LSM6DSL.h"

extern uint8_t FlagSwitch;

void convertDecToBin(int n)
{
	int k = 0;
	unsigned char *p = (unsigned char*)&n;
	int val2 = 0;
	int i = 0;
	for(k = 0; k <= 1; k++)
	{
		val2 = *(p+k);
		for (i = 7; i >= 0; i--)
		{
			if(val2 & (1 << i))
				printf("1");
			else
				printf("0");
		}
		printf(" ");
	}
}


/*
	If the SDO/SA0 pin is connected to the supply voltage, 
		LSb is ．1・ (address 1101011b); 
	else if the SDO/SA0 pin is 
		connected to ground, the LSb value is ．0・ (address 1101010b)

	PIN#2 : SCL	(use external pull up res.)
	PIN#3 : SDA	(use external pull up res.)
	
	PIN#4 (SA0/SDO) : HIGH
	PIN#5 (CS) : HIGH

*/

void I2C0_Init(void)	//PC1 : SCL , PC0 : SDA
{
    SYS_ResetModule(I2C0_RST);

    /* Open I2C module and set bus clock */
    I2C_Open(I2C0, LSM6DSL_I2C_SPEED);

    I2C_SetSlaveAddr(I2C0, 0, LSM6DSL_ADDRESS, 0);   /* Slave Address : 1101011b */

    /* Get I2C0 Bus Clock */
    printf("I2C clock %d Hz\n", I2C_GetBusClockFreq(I2C0));

	#if defined (ENABLE_I2C_IRQ)
    I2C_EnableInt(MASTER_I2C);
    NVIC_EnableIRQ(MASTER_I2C_IRQn);
	#endif
	
}

void GPIO_Init (void)
{
    GPIO_SetMode(PB, BIT14, GPIO_MODE_OUTPUT);
}

void TMR3_IRQHandler(void)
{
//	static uint32_t LOG = 0;
	static uint16_t CNT = 0;
	static uint16_t CNT_SWITCH = 0;
//	uint8_t data = 0;
	
    if(TIMER_GetIntFlag(TIMER3) == 1)
    {
        TIMER_ClearIntFlag(TIMER3);
	
		if (CNT++ >= 1000)
		{		
			CNT = 0;
			PB14 ^= 1;
			
//        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
//			convertDecToBin(LIS3DH_ADDRESS);
//			printf("\r\n");

		}

		if (CNT_SWITCH++ >= 10000)
		{		
			CNT_SWITCH = 0;
			FlagSwitch ^= 1;

//        	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
//        	printf("addr : 0x%2X\r\n",LSM6DS3_ADDRESS);			
//			convertDecToBin(LSM6DS3_ADDRESS);
//			printf("\r\n");			
		}		
    }
}

void TIMER3_Init(void)
{
    TIMER_Open(TIMER3, TIMER_PERIODIC_MODE, 1000);
    TIMER_EnableInt(TIMER3);
    NVIC_EnableIRQ(TMR3_IRQn);	
    TIMER_Start(TIMER3);
}

void UART0_Init(void)
{
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);

	/* Set UART receive time-out */
	UART_SetTimeoutCnt(UART0, 20);

	printf("\r\nCLK_GetCPUFreq : %8d\r\n",CLK_GetCPUFreq());
	printf("CLK_GetHXTFreq : %8d\r\n",CLK_GetHXTFreq());
	printf("CLK_GetLXTFreq : %8d\r\n",CLK_GetLXTFreq());	
	printf("CLK_GetPCLK0Freq : %8d\r\n",CLK_GetPCLK0Freq());
	printf("CLK_GetPCLK1Freq : %8d\r\n",CLK_GetPCLK1Freq());	
}

void SYS_Init(void)
{
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable HIRC clock (Internal RC 48MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
//    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
	
    /* Wait for HIRC clock ready */
    CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);
//    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);
	
    /* Select HCLK clock source as HIRC and HCLK source divider as 1 */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK(1));
//    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));
	
    /* Enable UART0 clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PCLK0, CLK_CLKDIV0_UART0(1));
	
    CLK_EnableModuleClock(TMR3_MODULE);
    CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);
	
    CLK_EnableModuleClock(I2C0_MODULE);

    /* Update System Core Clock */
    SystemCoreClockUpdate();

    /* Set PB multi-function pins for UART0 RXD=PB.12 and TXD=PB.13 */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB12MFP_Msk | SYS_GPB_MFPH_PB13MFP_Msk))    |       \
                    (SYS_GPB_MFPH_PB12MFP_UART0_RXD | SYS_GPB_MFPH_PB13MFP_UART0_TXD);


    /* Set I2C0 multi-function pins */
    SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC1MFP_Msk | SYS_GPC_MFPL_PC0MFP_Msk)) |
                    (SYS_GPC_MFPL_PC0MFP_I2C0_SDA | SYS_GPC_MFPL_PC1MFP_I2C0_SCL);

    /* Lock protected registers */
    SYS_LockReg();
}

/*
 * This is a template project for M031 series MCU. Users could based on this project to create their
 * own application without worry about the IAR/Keil project settings.
 *
 * This template application uses external crystal as HCLK source and configures UART0 to print out
 * "Hello World", users may need to do extra system configuration based on their system design.
 */

int main()
{
    SYS_Init();

    UART0_Init();

	GPIO_Init();
	
    I2C0_Init();
	
	TIMER3_Init();	

	appLSM6DSL_Setup();

    /* Got no where to go, just loop forever */
    while(1)
    {
		appLSM6DSL_GetData();

    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
