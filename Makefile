CC=gcc

EXEC=example_it
CXXFLAGS += -Wall -Wextra -g

SOURCE_PATH=src
BIN_PATH=bin
SRC = $(SOURCE_PATH)/avl_tree.c $(SOURCE_PATH)/interval_tree.c $(SOURCE_PATH)/example_it.c
INC = $(SOURCE_PATH)/avl_tree.h $(SOURCE_PATH)/interval_tree.h
OBJ = $(SRC:.c=.o)

LINKER_FLAGS= -o $(BIN_PATH)/$(EXEC) 


all: example_it

.PHONY: create_bin


create_bin:
	@mkdir -p bin

example_it: create_bin  $(OBJ)  Makefile
	$(CC) $(CFLAGS)  $(OBJ) $(LINKER_FLAGS)	


$(OBJ): %.o : %.c $(INC) 
	$(CC) -c $(CXXFLAGS) $< -o $@


clean:
	@rm -rf $(SOURCE_PATH)/*.o $(BIN_PATH)
	@rm -f *~ */*~


.PHONY: help

help:
	@echo "-------------------------------------------------------------------------------------------------"
	@echo "This makefile supports the following options:"
	@echo "-------------------------------------------------------------------------------------------------"
	@echo "     + make all: Generates user  design under the bin path."
	@echo "     + make clean: Removes user  design."
	@echo "--------------------------------------------------------------------José Fernando Zazo Rollón----"
