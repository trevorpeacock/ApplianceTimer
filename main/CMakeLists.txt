#include(u8g2/CMakeLists.txt)
#add_subdirectory(u8g2)

set(U8G2_DIR u8g2)
file(GLOB U8G2_SRC_FILES "${U8G2_DIR}/csrc/*.c")
file(GLOB U8G2_HEADER_FILES "${U8G2_DIR}/csrc/*.h")

file(GLOB SRC_FILES *.cpp)
add_executable(main
        main.cpp
        ${U8G2_HEADER_FILES} ${U8G2_SRC_FILES}
        )


# pull in common dependencies
target_link_libraries(main pico_stdlib hardware_pwm hardware_flash hardware_sync hardware_i2c)

# create map/bin/hex file etc.
pico_add_extra_outputs(main)

# add url via pico_set_program_url
example_auto_set_url(main)


