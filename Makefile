BUILD       = build
TARGET      = target
GCC         = aarch64-linux-gnu-g++
BIN         = v4l2-test
CFLAGS      = -ggdb -Wall -O3 -fopenmp -o $(BUILD)/$(BIN)
TARGET_USER = root
TARGET_IP   = 192.168.1.109
# SSHPASS     = sshpass -p "twqo43iP"
SSHPASS     =
TEST        = /home/$(TARGET_USER)/test
ARGS        = -s
RM          = rm -rf
INCLUDES    = -I include/errno \
	      -I include/utils \
              -I include/v4l2
SOURCES     = src/errno/errno.cpp \
	      src/v4l2/v4l2image.cpp \
	      src/v4l2/v4l2imagesource.cpp \
              src/utils/commandargs.cpp \
	      src/utils/framebuffer.cpp \
	      src/v4l2-test/v4l2-test.cpp

all: clean default test

setup:
	scp $(TARGET)/* $(TARGET_USER)@$(TARGET_IP):$(TEST)

clean:
	$(RM) $(BUILD)/$(BIN)

default: configure $(BIN)

configure:
	mkdir -p $(BUILD)

$(BIN): $(SOURCES)
	$(GCC) $(CFLAGS) $(INCLUDES) $(SOURCES)

test: $(BIN)
	# ssh $(TARGET_USER)@$(TARGET_IP) $(TEST)/pkill.sh $(BIN)
	$(SSHPASS) scp $(BUILD)/$(BIN) $(TARGET_USER)@$(TARGET_IP):$(TEST)

gdbserver: test
	ssh $(TARGET_USER)@$(TARGET_IP) "cd $(TEST); ./gdbserver.sh $(BIN) $(ARGS)"