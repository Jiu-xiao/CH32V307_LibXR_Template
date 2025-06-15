#include <cmath>
#include "ch32v30x_gpio.h"
#include "libxr.hpp"
#include "ch32_timebase.hpp"
#include "tusb.h"
#include "tinyusb_virtual_uart.hpp"

extern "C" void app_main()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    LibXR::CH32Timebase timebase;

    LibXR::PlatformInit(3, 4096);

    static LibXR::TinyUSBVirtualUART uart;

    void (*blink_task)(void *) = [](void *)
    {
        static bool flag = false;
        if (flag)
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_4);
            flag = false;
        }
        else
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_4);
            flag = true;
        }
        static LibXR::WriteOperation op;
    };

    LibXR::STDIO::read_ = uart.read_port_;
    LibXR::STDIO::write_ = uart.write_port_;

    auto blink_task_handle = LibXR::Timer::CreateTask(blink_task, reinterpret_cast<void *>(0), 500);
    LibXR::Timer::Add(blink_task_handle);
    LibXR::Timer::Start(blink_task_handle);

    LibXR::RamFS ramfs;

    LibXR::Terminal<> terminal(ramfs);

    auto terminal_task_handle = LibXR::Timer::CreateTask(terminal.TaskFun, &terminal, 1);
    LibXR::Timer::Add(terminal_task_handle);
    LibXR::Timer::Start(terminal_task_handle);


    while (1)
    {
        LibXR::Thread::Sleep(1000);
    }
}