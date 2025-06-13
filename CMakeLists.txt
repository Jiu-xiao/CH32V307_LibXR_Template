set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.19)

set(COMPILER_PREFIX "riscv-wch-elf-")
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
project(CH32V307VC C ASM)
set(CMAKE_C_STANDARD 99)

# compile and running settings
add_compile_options(-march=rv32imafcxw -mabi=ilp32f -msmall-data-limit=8 -mno-save-restore -fmessage-length=0)
add_compile_options(-fsigned-char -ffunction-sections -fdata-sections -fsingle-precision-constant -Wunused -Wuninitialized)
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)
add_compile_options(-Og -g)

add_link_options(-nostartfiles -Xlinker --gc-sections -Wl,--print-memory-usage,-Map,${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(--specs=nano.specs --specs=nosys.specs -march=rv32imafcxw -mabi=ilp32f -flto)

add_executable(${PROJECT_NAME}.elf)

file(GLOB ${PROJECT_NAME}_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.c")
file(GLOB ${PROJECT_NAME}_PERIPHERAL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/Peripheral/src/*.c")
file(GLOB ${PROJECT_NAME}_USER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/User/*.c")
file(GLOB ${PROJECT_NAME}_FREERTOS_SOURCES 
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/GCC/RISC-V/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/Common/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS/portable/MemMang/*.c"
)

target_sources(${PROJECT_NAME}.elf 
        PRIVATE 
        ${${PROJECT_NAME}_CORE_SOURCES}
        ${${PROJECT_NAME}_PERIPHERAL_SOURCES}
        ${${PROJECT_NAME}_USER_SOURCES}
        ${${PROJECT_NAME}_FREERTOS_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/startup_ch32v30x_D8C.S
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

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}
Building ${BIN_FILE}")

set(LIST_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.lst)
add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJDUMP} --all-headers --demangle --disassemble -M xw $<TARGET_FILE:${PROJECT_NAME}.elf> > ${LIST_FILE}
        COMMENT "Building ${LIST_FILE}")
