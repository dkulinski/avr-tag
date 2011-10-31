CC=avr-gcc
CXX=avr-g++
CCOPTS=-Os -Wall -mmcu=atmega328
CXXOPTS=-Os -Wall -mmcu=atmega328
OBJ2HEX=avr-objcopy
MCU=atmega328

%.o : %.c
	$(CC) $(CCOPTS) $< -o $@

%.o : %.cc
	$(CXX) $(CXXOPTS) $< -o $@

%.hex : %.o
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

clean:
	rm -f *.hex *.obj *.o
