set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.19)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(COMPILER_PREFIX "riscv32-unknown-elf-")
set(CMAKE_C_COMPILER ${COMPILER_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${COMPILER_PREFIX}g++)
set(CMAKE_ASM_COMPILER ${COMPILER_PREFIX}gcc)
set(CMAKE_LINKER ${COMPILER_PREFIX}gcc)
set(CMAKE_AR ${COMPILER_PREFIX}ar)
set(CMAKE_OBJCOPY ${COMPILER_PREFIX}objcopy)
set(CMAKE_OBJDUMP ${COMPILER_PREFIX}objdump)
set(SIZE ${COMPILER_PREFIX}size)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build)

# project settings
project(CH32V307VC)

enable_language(C CXX ASM)

# compile and running settings
# CPU 架构相关
set(CPU_FLAGS "-march=rv32imacf_zicsr_zifencei -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore")

# 通用编译选项
set(GENERAL_FLAGS "-Wall -Wextra -fno-builtin -ffunction-sections -fdata-sections -fsigned-char -fsingle-precision-constant -Wunused -Wuninitialized")

# C 编译器专用 flags
set(CMAKE_C_FLAGS "${CPU_FLAGS} ${GENERAL_FLAGS} -fmessage-length=0"
    CACHE INTERNAL "" FORCE)

# C++ 编译器专用 flags
set(CMAKE_CXX_FLAGS "${CPU_FLAGS} ${GENERAL_FLAGS} -fno-use-cxa-atexit -fno-exceptions -fno-rtti -fno-threadsafe-statics"
    CACHE INTERNAL "" FORCE)

# 汇编器选项
set(CMAKE_ASM_FLAGS "${CPU_FLAGS} -x assembler-with-cpp"
    CACHE INTERNAL "" FORCE)
add_compile_options(-g)

add_link_options(
  -nostartfiles
  -Xlinker --gc-sections
  -Wl,--print-memory-usage
  -Wl,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map
)

add_link_options(
  --specs=nano.specs
  --specs=nosys.specs
  -march=rv32imacf_zicsr_zifencei
  -mabi=ilp32f
  -flto
  -lm
  -lc
)

# target
add_executable(${PROJECT_NAME}.elf)

file(GLOB ${PROJECT_NAME}_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c")
file(GLOB ${PROJECT_NAME}_PERIPHERAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/src/*.c")
file(GLOB ${PROJECT_NAME}_USER_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/User/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/User/*.cpp"
)
file(GLOB ${PROJECT_NAME}_FREERTOS_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/MemMang/heap_4.c"
)

target_sources(${PROJECT_NAME}.elf 
        PRIVATE 
        ${${PROJECT_NAME}_CORE_SOURCES}
        ${${PROJECT_NAME}_PERIPHERAL_SOURCES}
        ${${PROJECT_NAME}_USER_SOURCES}
        ${${PROJECT_NAME}_FREERTOS_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ch32v30x_D8C.S
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V/portASM.S
)

target_include_directories(${PROJECT_NAME}.elf
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/User
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/include
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/MemMang
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/Link.ld)
target_link_options(${PROJECT_NAME}.elf PRIVATE -T ${LINKER_SCRIPT})

set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

# libxr
set(LIBXR_SYSTEM FreeRTOS)
set(LIBXR_DRIVER ch)
set(LIBXR_CH32_CONFIG_FILE ch32v30x_conf.h)

add_subdirectory(libxr)

target_include_directories(xr
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/Core
        ${CMAKE_CURRENT_SOURCE_DIR}/User
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/include
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V/chip_specific_extensions/RV32I_PFIC_no_extensions
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/MemMang
)

target_include_directories(${PROJECT_NAME}.elf PRIVATE
    PUBLIC $<TARGET_PROPERTY:xr,INTERFACE_INCLUDE_DIRECTORIES>
    PUBLIC User
)

target_link_libraries(${PROJECT_NAME}.elf
    PUBLIC xr
)

# tinyusb

file(GLOB ${PROJECT_NAME}_TINYUSB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/tusb.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/common/tusb_fifo.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/device/usbd.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/device/usbd_control.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/cdc/cdc_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/dfu/dfu_rt_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/hid/hid_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/midi/midi_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/msc/msc_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/net/net_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/usbtmc/usbtmc_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/class/vendor/vendor_device.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/wch/dcd_ch32_usbhs.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/tinyusb/src/portable/wch/dcd_ch32_usbfs.c"
)

target_sources(${PROJECT_NAME}.elf
    PRIVATE ${${PROJECT_NAME}_TINYUSB_SOURCES}
)

target_include_directories(${PROJECT_NAME}.elf
    PUBLIC tinyusb/src
    PUBLIC tinyusb/src/portable/wch
)

target_include_directories(xr
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libxr/driver/tinyusb
    PUBLIC tinyusb/src
    PUBLIC tinyusb/src/portable/wch
)

target_sources(xr
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libxr/driver/tinyusb/tinyusb_virtual_uart.cpp
)

# build settings
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")
