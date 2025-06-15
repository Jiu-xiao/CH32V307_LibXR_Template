#include "FreeRTOS.h"
#include "task.h"
#include "math.h"
#include "app_main.h"

#include "tusb.h"

__attribute__((interrupt)) void USBHS_IRQHandler(void)
{
	tud_int_handler(0);
	tud_task();
}

__attribute__((interrupt)) void USB_LP_CAN1_RX0_IRQHandler(void)
{
	tud_int_handler(0);
	tud_task();
}

__attribute__((interrupt)) void USB_HP_CAN1_TX_IRQHandler(void)
{
	tud_int_handler(0);
	tud_task();
}

static void DefaultTask(void *pvParameters)
{
	(void)(pvParameters);
	app_main();
	while (1)
	{
		vTaskDelay(1000);
	}
}

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	SystemInit();
	SystemCoreClockUpdate();
	RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);
	RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);
	RCC_USBHSConfig(RCC_USBPLL_Div2);
	RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);
	RCC_USBHSPHYPLLALIVEcmd(ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
	NVIC_SetPriority(USBHS_IRQn, 3);
	NVIC_SetPriority(USBFS_IRQn, 3);
	uint8_t otg_div;
	switch (SystemCoreClock)
	{
	case 48000000:
		otg_div = RCC_OTGFSCLKSource_PLLCLK_Div1;
		break;
	case 96000000:
		otg_div = RCC_OTGFSCLKSource_PLLCLK_Div2;
		break;
	case 144000000:
		otg_div = RCC_OTGFSCLKSource_PLLCLK_Div3;
		break;
	default:
		TU_ASSERT(0, );
		break;
	}
	RCC_OTGFSCLKConfig(otg_div);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_OTG_FS, ENABLE);
	__enable_irq();
	xTaskCreate(DefaultTask, "DefaultTask", 4096, NULL, 3, NULL);
	vTaskStartScheduler();
	return 0;
}
