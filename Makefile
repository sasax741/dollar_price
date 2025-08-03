CC = gcc
CFLAGS_BASE = -Wall -Wextra -std=c99 -g
LDFLAGS_BASE = -lcurl -ljansson

UNAME_S := $(shell uname -s)

CFLAGS = $(CFLAGS_BASE)
LDFLAGS = $(LDFLAGS_BASE)

ifeq ($(UNAME_S), Darwin)
    BREW_PREFIX := $(shell command -v brew >/dev/null && brew --prefix)
    ifneq ($(BREW_PREFIX),)
        CFLAGS += -I$(BREW_PREFIX)/include
        LDFLAGS += -L$(BREW_PREFIX)/lib
        check_libs:
	        @if ! brew ls --versions jansson >/dev/null; then echo "Warning: jansson not found. Install with 'brew install jansson'"; fi
	        @if ! brew ls --versions curl >/dev/null; then echo "Warning: curl not found. Install with 'brew install curl'"; fi
        all: check_libs $(TARGET)
    else
        check_brew:
	        @echo "Warning: Homebrew not found on macOS. Please install it from https://brew.sh to manage dependencies."
        all: check_brew $(TARGET)
    endif
else
    all: $(TARGET)
endif

TARGET = dollar

.PHONY: all

$(TARGET): dollar.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -o $(TARGET) $< $(LDFLAGS)
	@echo "$(TARGET) has been compiled successfully."

.PHONY: install
install: all
	@echo "Installing $(TARGET) to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/
	@echo "Installation complete. You can now run '$(TARGET)' from your terminal."

.PHONY: clean
clean:
	@echo "Cleaning up build files..."
	rm -f $(TARGET)
	@echo "Cleanup complete."