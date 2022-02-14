SRCDIR		= ./src
VPATH		= $(SRCDIR)
SRC_C		= $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
OBJ_C		= $(notdir $(patsubst %.c, %.o, $(SRC_C)))
CD_OBJECTS	   = 
OBJECTS        = $(OBJ_C)
ELF_TARGET     = mytest.elf
BIN_TARGET     = mytest.bin
ADD_FILES      = 
CDOUT          = mytest_cd

include example.mk
