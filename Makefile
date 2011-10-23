CC=avr-gcc
CCOPTS=-Os -Wall -mmcu=atmega328
OBJ2HEX=avr-objcopy
MCU=atmega328

%.o : %.c
	$(CC) $(CCOPTS) $< -o $@

%.hex : %.o
	$(OBJ2HEX) -R .eeprom -O ihex $< $@

clean:
	rm -f *.hex *.obj *.o
