#compiler and linker flags
CC			:= gcc
SRC_DIRS	:= src	src/user_lib	src/user_lib/mb_slave
INC_DIRS	:= 
LIB_DIRS	:=
BIN_DIR 	:= bin
BIN_EXE		:= test
OBJ_DIR		:= obj
CPP_FLAGS	:= -O0 -g
LD_FLAGS	:= -O0
MAKEFLAGS	:=

# Decide whether the commands will be shwon or not
VERBOSE = TRUE

LIBS 		:= $(foreach dir, $(LIB_DIRS), $(addprefix -L, $(dir)))
INCLUDES 	:= $(foreach dir, $(SRC_DIRS) $(INC_DIRS), $(addprefix -I, $(dir)))
TARGETDIRS 	:= $(foreach dir, $(SRC_DIRS), $(addprefix $(OBJ_DIR)/, $(dir)))
SRC_FILES	:= $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
H_FILES		:= $(foreach dir,$(SRC_DIRS) $(INC_DIRS),$(wildcard $(dir)/*.h))
OBJ_FILES	:= $(addprefix $(OBJ_DIR)/, $(patsubst %.c, %.o, $(SRC_FILES)))
#OUTPUT		:= $(BIN_DIR)/$(BIN_EXE)

# OS specific part
OUTPUT		:= $(BIN_DIR)/$(if $(findstring Windows_NT, $(OS)),$(BIN_EXE).exe,$(BIN_EXE))

VPATH = $(SRC_DIRS)

# Hide or not the calls depending of VERBOSE
ifeq ($(VERBOSE),TRUE)
    HIDE =
else
    HIDE = @
endif
# OS specific part
ifeq ($(OS),Windows_NT)
    RM = del /F /Q 
    RMDIR = -RMDIR /S /Q
    MKDIR = -mkdir
    ERRIGNORE = 2>NUL || (exit 0)
    SEP=\\
else
    RM = rm -rf 
    RMDIR = rm -rf 
    MKDIR = mkdir -p
    ERRIGNORE = 2>/dev/null
    SEP=/
endif

# Remove space after separator
PSEP = $(strip $(SEP))

# Define the function that will generate each rule
define generateRules
$(1)/%.o: %.c
	@echo Building $$@
	$$(HIDE) $$(CC) $$(CPP_FLAGS) $$(INCLUDES) -c -o $$(subst /,$$(PSEP),$$@) $$(subst /,$$(PSEP),$$<) -MMD
endef
#$(CPP_FLAGS)
.PHONY: all clean directories new

all:	$(info TARGETDIRS is $(TARGETDIRS))\
		directories $(OUTPUT)

# Generate rules
$(foreach targetdir, $(TARGETDIRS), $(eval $(call generateRules, $(targetdir))))

$(OUTPUT): $(OBJ_FILES)
	$(HIDE)echo Linking $@
	$(HIDE)$(CC) $(LIBS) $(LD_FLAGS) $(OBJ_FILES) -o $(OUTPUT)

directories:
	$(HIDE)$(MKDIR) $(subst /,$(PSEP),$(TARGETDIRS)) $(ERRIGNORE)
	
# Remove all objects, dependencies and executable files generated during the build
clean:
	$(HIDE)$(RMDIR) $(subst /,$(PSEP),$(TARGETDIRS)) $(ERRIGNORE)
	$(HIDE)$(RM) $(subst /,$(PSEP),$(OUTPUT)) $(ERRIGNORE)
	@echo Cleaning done !

new: clean
	$(MAKE)

run: new
	./bin/test