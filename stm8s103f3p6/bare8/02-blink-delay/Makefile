#CC = sdcc
#CFLAGS = -mstm8
#CFLAGS += -I..
#VPATH = ..

#HEX = app.ihx

RELS = main.rel gpio.rel

include ../bare8.mk

#all : $(HEX)


#%.rel : %.c
#	$(CC) $(CFLAGS) -c $<

#$(HEX) : $(RELS)
#	$(CC) $(CFLAGS) $(RELS)  --out-fmt-ihx -o$(HEX)

#flash: $(HEX)
#	stm8flash -cstlinkv2 -pstm8s103f3 -w$(HEX)

#clean:
#	rm -f .ihx *.lst *.asm *.sym *.rst *.rel *.cdb *.ihx *.lk *.map *.mem
