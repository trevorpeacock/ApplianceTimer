
export PICO_SDK_PATH=$(realpath ./lib/pico-sdk)

.DEFAULT_GOAL := build

#sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential libstdc++-arm-none-eabi-newlib

lib:
	if ! (dpkg-query -l cmake > /dev/null) then echo "Need to install cmake"; sudo apt install cmake; fi
	if ! (dpkg-query -l gcc-arm-none-eabi > /dev/null) then echo "Need to install gcc-arm-none-eabi"; sudo apt install gcc-arm-none-eabi; fi
	if ! (dpkg-query -l libnewlib-arm-none-eabi > /dev/null) then echo "Need to install libnewlib-arm-none-eabi"; sudo apt install libnewlib-arm-none-eabi; fi
	if ! (dpkg-query -l build-essential > /dev/null) then echo "Need to install build-essential"; sudo apt install build-essential; fi
	if ! (dpkg-query -l libstdc++-arm-none-eabi-newlib > /dev/null) then echo "Need to install libstdc++-arm-none-eabi-newlib"; sudo apt install libstdc++-arm-none-eabi-newlib; fi
	mkdir lib
	cd lib && git clone -b master https://github.com/raspberrypi/pico-sdk.git --depth 1 --shallow-submodules
	cd lib/pico-sdk && git submodule update --init
	cd main && git clone -b master https://github.com/olikraus/u8g2.git --depth 1

clean:
	rm -dfr build

clean_libs:
	rm -dfr lib
	rm -dfr main/u8g2

build: lib
	mkdir build
	cd build && cmake -G "Unix Makefiles" ..
	cd build && make

flash:
	#stty -F /dev/ttyACM0 1200
	#sleep 5
	cp build/main/main.uf2 /media/${USER}/RPI-RP2/

