SOURCE_DIR = $(PWD)
INCLUDE_DIR = $(SOURCE_DIR)
LIBRARY_DIR = $(SOURCE_DIR)
BUILD_DIR = $(SOURCE_DIR)/build


DATABASE_LDFLAGS = -lpthread -ldl
    
#   all
ALL_SRC =                       \
    libreceiver.c               \
    example.c
    
ALL_INC =                       \
    libreceiver.h               \
    
ALL_CFLAGS =                    \
    -I./                        \
    -DOS_LINUX                  \
    -g -Wall
    
ALL_LDFLAGS =                   \
    -L$(LIBRARY_DIR)            \
    -lpthread -ldl
    
ALL_OBJ = $(notdir $(patsubst %.c, %.o, $(ALL_SRC)))

$(CC) = gcc

all: example

example: $(addprefix $(BUILD_DIR)/, $(ALL_OBJ))
	$(CC) $^ -o $@ $(ALL_CFLAGS) $(ALL_LDFLAGS)


$(addprefix $(BUILD_DIR)/, $(ALL_OBJ)): $(ALL_SRC)
	$(CC) $^ -c -I$(INCLUDE_DIR) $(ALL_CFLAGS)
	mkdir -p $(BUILD_DIR)
	mv *.o $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

debug_msg:
	@echo $(DATABASE_SRC)