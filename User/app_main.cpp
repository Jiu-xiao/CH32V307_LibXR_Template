#include <cmath>

#include "ch32_gpio.hpp"
#include "ch32_timebase.hpp"
#include "ch32_uart.hpp"
#include "ch32v30x_gpio.h"
#include "libxr.hpp"
#include "tinyusb_virtual_uart.hpp"
#include "tusb.h"

extern "C" void app_main()
{
  LibXR::CH32Timebase timebase;

  LibXR::PlatformInit(3, 8192);

  LibXR::CH32GPIO led_b(GPIOB, GPIO_Pin_4);
  LibXR::CH32GPIO led_r(GPIOA, GPIO_Pin_15);
  LibXR::CH32GPIO key(GPIOB, GPIO_Pin_3, LibXR::CH32GPIO::Direction::FALL_INTERRUPT,
                      LibXR::CH32GPIO::Pull::UP, EXTI3_IRQn);

  void (*key_cb_fun)(bool, LibXR::GPIO *) = [](bool, LibXR::GPIO *led)
  {
    static bool flag = false;
    flag = !flag;
    led->Write(flag);
  };

  auto key_cb =
      LibXR::GPIO::Callback::Create(key_cb_fun, reinterpret_cast<LibXR::GPIO *>(&led_r));

  key.RegisterCallback(key_cb);

  key.DisableInterrupt();

  static LibXR::TinyUSBVirtualUART uart;
  static uint8_t uart1_tx_buffer[64], uart1_rx_buffer[64];

  LibXR::CH32UART uart1(CH32_USART2, uart1_rx_buffer, uart1_tx_buffer, GPIOA, GPIO_Pin_2,
                        GPIOA, GPIO_Pin_3, 0, 25);

  uart1.SetConfig({
      .baudrate = 115200,
      .parity = LibXR::UART::Parity::NO_PARITY,
      .data_bits = 8,
      .stop_bits = 1,
  });

  void (*blink_task)(LibXR::GPIO *) = [](LibXR::GPIO *led)
  {
    static bool flag = false;
    if (flag)
    {
      flag = false;
    }
    else
    {
      flag = true;
    }

    led->Write(flag);
  };

  LibXR::STDIO::read_ = uart1.read_port_;
  LibXR::STDIO::write_ = uart1.write_port_;

  auto blink_task_handle =
      LibXR::Timer::CreateTask(blink_task, reinterpret_cast<LibXR::GPIO *>(&led_b), 500);
  LibXR::Timer::Add(blink_task_handle);
  LibXR::Timer::Start(blink_task_handle);

  LibXR::RamFS ramfs;

  LibXR::Terminal<> terminal(ramfs);

  auto terminal_task_handle = LibXR::Timer::CreateTask(terminal.TaskFun, &terminal, 1);
  LibXR::Timer::Add(terminal_task_handle);
  LibXR::Timer::Start(terminal_task_handle);
  LibXR::WriteOperation op;
  uart1.Write(LibXR::ConstRawData("Hello World!\r\n"), op);
  while (1)
  {
    LibXR::Thread::Sleep(1000);
  }
}
