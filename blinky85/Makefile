CXX=avr-g++
CXX_FLAGS=-g -Os -Wall -mcall-prologues -mmcu=attiny85
BASE=blinky

.PHONY: install clean

$(BASE).hex : $(BASE).elf
	avr-objcopy -R .eeprom -O ihex $^ $@

$(BASE).elf : $(BASE).o
	$(CXX) -g -mmcu=attiny85 -o $@ $^

$(BASE).o : $(BASE).cc
	$(CXX) $(CXX_FLAGS) -c $^ 


install:$(BASE).hex
	install-hex $<

clean :
	rm -f *.hex *.obj *.o *.elf

