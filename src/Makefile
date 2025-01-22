
CC = gcc
CFLAGS = -Wall -fPIC
LDFLAGS = -shared
TARGET_DIR = /lib/imperva
TARGET = $(TARGET_DIR)/hello_hook.so
SRC = aso_dso.c

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p $(TARGET_DIR)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)
	@echo "Shared object created at $(TARGET)"

clean:
	rm -f $(TARGET)
	@echo "Cleaned up $(TARGET)"
