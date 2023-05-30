#
#
#
#
CC = clang
XCC =clang++
LD =clang++
SZ=llvm-size

TARGET = prog

#===========
DEFS = \


CXX_INCLUDE = \
tinyxml2 \
json/include \
CLI11/include \

C_INCLUDE = \

#=============
CXX_SOURCES = \
main.cpp \
uvconvertor.cpp \
tinyxml2/tinyxml2.cpp \

C_SOURCES = \

#=============
LIBPATH = \

LIBS = \

#-----------------------------------------------
CXXFLAGS = \
$(patsubst %, -D%, $(DEFS)) \
$(patsubst %, -I%, $(CXX_INCLUDE)) \
-std=c++17 \
-g

CFLAGS = \
-std=c99 \
$(patsubst %, -D%, $(DEFS)) \
$(patsubst %, -I%, $(C_INCLUDE)) \

LDFLAGS = \
$(patsubst %, -L%, $(LIBPATH)) \
$(patsubst %, -l%, $(LIBS)) \
-std=c++17

DEBUG = \
-O0
#-----------------------------------------------
BUILD_DIR = build


#-----------------------------------------------
C_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
CXX_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(CXX_SOURCES:.cpp=.o)))
OBJECTS = $(C_OBJECTS) $(CXX_OBJECTS)

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.cpp $(sort $(dir $(CXX_SOURCES)))
#-----------------------------------------------
$(TARGET).exe: $(OBJECTS) Makefile
	$(LD) $(OBJECTS) $(LDFLAGS) $(DEBUG) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(XCC) --driver-mode=gcc $(DEBUG) -c $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) --driver-mode=gcc  $(DEBUG) -c $(CFLAGS)   $< -o $@

$(BUILD_DIR):
	mkdir $@
#-----------------------------------------------
.PHONY: clean

clean:
	find $(BUILD_DIR) -name "*.o" |xargs rm -rf
#find . -type f | grep .o$ | xargs rm -r
