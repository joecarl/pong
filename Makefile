DEBUG ?= 1
ifeq ($(DEBUG),1)
TARGET_MODE := debug
DEBUG_FLAGS := -g
else
TARGET_MODE := release
DEBUG_FLAGS :=
endif

BUILD_BASE_DIR := ./build/$(TARGET_MODE)
OBJ_DIR := ./obj/$(TARGET_MODE)
SRC_DIRS := ./src ./libs/serendipia-engine/src
PCH_SRC_FILE := ./scripts/pch.hpp
PCH_OBJ_FILE := $(OBJ_DIR)/pch.hpp
GCH_OBJ_FILE := $(PCH_OBJ_FILE).gch

ifndef TARGET_EXEC

.PHONY: all client server
.NOTPARALLEL: all

CLIENT_SRC_DIRS = $(addsuffix /client,$(SRC_DIRS))
SERVER_SRC_DIRS = $(addsuffix /server,$(SRC_DIRS))
UPDATER_SRC_DIRS = ./src/client/updater

ALLEGRO_LIBS := -lallegro -lallegro_image -lallegro_main -lallegro_ttf -lallegro_primitives -lallegro_audio -lallegro_font
ifeq ($(OS), Windows_NT)
COMMON_BOOST_LIBS := -lboost_chrono-mt -lboost_system-mt -lboost_thread-mt -lboost_json-mt -lws2_32
client: BOOST_LIBS := $(COMMON_BOOST_LIBS)
server: BOOST_LIBS :=  -static -lmswsock $(COMMON_BOOST_LIBS)
else
BOOST_LIBS := -lboost_chrono -lboost_system -lboost_thread -lboost_json -lpthread
endif

all: client server

client: export TARGET_EXEC := pong
client: export BUILD_DIR := $(BUILD_BASE_DIR)/client
client: export RESOURCES_DIR := ./src/client/resources
client: export EXCLUDE_SRC_DIRS := $(SERVER_SRC_DIRS) $(UPDATER_SRC_DIRS)
client: export LINKER_LIBS := -static-libstdc++ -static-libgcc $(ALLEGRO_LIBS) $(BOOST_LIBS)
client: export WINDRES_DIR := ./src/client/windres
ifeq ($(TARGET_MODE),release)
client: export ADDL_LDFLAGS := -mwindows
endif

server: export TARGET_EXEC := pongserver
server: export BUILD_DIR := $(BUILD_BASE_DIR)/server
server: export RESOURCES_DIR := ./src/server/resources
server: export EXCLUDE_SRC_DIRS := $(CLIENT_SRC_DIRS)
server: export LINKER_LIBS := $(BOOST_LIBS)
server: export WINDRES_DIR := ./src/server/windres

client server:
	@$(MAKE) --no-print-directory

else

# Find all the C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
ALL_SRCS := $(shell find $(SRC_DIRS) -name '*.cpp')
EXCLUDE_SRCS := $(shell find $(EXCLUDE_SRC_DIRS) -name '*.cpp')
SRCS := $(filter-out $(EXCLUDE_SRCS),$(ALL_SRCS))

# Do windows specific tasks such as finding windres files and appending .exe to the target exec file
ifeq ($(OS), Windows_NT)
WINDRES_SRCS := $(shell if [ -d '$(WINDRES_DIR)' ]; then find $(WINDRES_DIR) -name '*.rc'; fi)
TARGET_EXEC := $(TARGET_EXEC).exe
endif

# Prepends OBJ_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./obj/./your_dir/hello.cpp.o
CPP_OBJS := $(SRCS:%=$(OBJ_DIR)/%.o)
OBJS := $(CPP_OBJS) $(WINDRES_SRCS:%=$(OBJ_DIR)/%.res)

# String substitution (suffix version without %).
# As an example, ./obj/hello.cpp.o turns into ./obj/hello.cpp.d
DEPS := $(CPP_OBJS:.o=.d)

INC_DIRS := ./libs/serendipia-engine/include
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The -MMD and -MP flags together generate Makefiles for us!
# These files will have .d instead of .o as the output.
GLOBAL_CPPFLAGS := -std=c++17 -Wall -fexceptions $(DEBUG_FLAGS)
CPPFLAGS := $(GLOBAL_CPPFLAGS) -Wno-misleading-indentation -include $(PCH_OBJ_FILE) $(INC_FLAGS) -MMD -MP
LDFLAGS := $(LINKER_LIBS) $(ADDL_LDFLAGS)

# The final build step.
$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	@mkdir -p $(dir $@)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)
	@if [ -d "$(RESOURCES_DIR)" ]; then cp -rv $(RESOURCES_DIR) $(BUILD_DIR); fi
	@if [ '$(OS)' = 'Windows_NT' ]; then bash scripts/copy-dlls.sh $(BUILD_DIR); fi

# Build step for C++ source
$(OBJ_DIR)/%.cpp.o: %.cpp $(GCH_OBJ_FILE)
	@mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# Precompiled headers
# Please note we first copy the header file to the target obj dir
# This allows having different gch outputs for debug/release builds
$(GCH_OBJ_FILE): $(PCH_SRC_FILE)
	@mkdir -p $(dir $@)
	@cp $< $(PCH_OBJ_FILE)
	$(CXX) $(GLOBAL_CPPFLAGS) -x c++-header -c $(PCH_OBJ_FILE)

# Build step for windres resources
$(OBJ_DIR)/%.rc.res: %.rc
	@mkdir -p $(dir $@)
	windres $< -O coff -o $@

# Include the .d makefiles. The - at the front suppresses the errors of missing
# Makefiles. Initially, all the .d files will be missing, and we don't want those
# errors to show up.
-include $(DEPS)

endif


.PHONY: clean
clean:
	rm -rf ./build
	rm -rf ./obj