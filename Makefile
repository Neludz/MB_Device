#compiler and linker flags
CC			:= gcc
SRC_DIR		:= src
SRC_DIRS	:= src src/user_lib
BIN_DIR 	:= bin
BIN_EXE		:= $(BIN_DIR)/test
OUTPUT		:= $(BIN_EXE)
OBJ_DIR		:= obj/
INC_DIRS	:= -I$(SRC_DIR/user_lib) -I$(SRC_DIR)
LIB_DIRS	:=
SRC_FILES	:= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
H_FILES		:= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))#$(wildcard $(SRC_DIR)/*/*.h $(SRC_DIR)/*.h)
OBJ_FILES	:= $(addprefix $(OBJ_DIR), $(patsubst %.c, %.o, $(SRC_FILES)))#TARGETDIRS = $(foreach dir, $(DIRS), $(addprefix $(BUILDDIR)/, $(dir)))

CPP_FLAF	:= -o3
LD_FLAGS	:= -o3
MAKEFLAGS	:=
# $(wildcard $(SRC_DIR)/*/*.c $(SRC_DIR)/*.c)
$(OBJ_DIR)/%.o:	$(SRC_FILES)/%.c $(H_FILES)
	$(CC) $(CPP_FLAF) $(INC_DIRS) -c -o $@ $<

$(OUTPUT): $(OBJ_FILES) Makefile
	$(CC) $(LIB_DIRS) $(LD_FLAGS) $(OBJ_FILES) -o $(OUTPUT)

all: $(info SRC_FILES is $(SRC_FILES)) $(info H_FILES is $(H_FILES))  $(info OBJ_FILES is $(OBJ_FILES)) $(info OBJ_DIR is $(OBJ_DIR)) $(OUTPUT)