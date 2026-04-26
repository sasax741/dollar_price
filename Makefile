# Makefile - Dollar Price Checker
# Professional build system with security best practices

# Compiler and tools
CC := gcc
AR := ar
RM := rm -f
MKDIR := mkdir -p

# Version
VERSION := 1.0.0

# Installation directories
PREFIX ?= /usr/local
BINDIR := $(PREFIX)/bin
MANDIR := $(PREFIX)/share/man/man1

# Compiler flags
CFLAGS_BASE := -Wall -Wextra -std=c99 -g -O2
CFLAGS_BASE += -DVERSION=\"$(VERSION)\" -D_DEFAULT_SOURCE
CFLAGS := $(CFLAGS_BASE)
LDFLAGS := -lcurl -ljansson

# Platform detection
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Platform-specific flags
ifeq ($(UNAME_S), Linux)
    CFLAGS += -DLINUX
endif

ifeq ($(UNAME_S), Darwin)
    BREW_PREFIX := $(shell command -v brew >/dev/null && brew --prefix)
    ifneq ($(BREW_PREFIX),)
        CFLAGS += -I$(BREW_PREFIX)/include
        LDFLAGS += -L$(BREW_PREFIX)/lib
    endif
endif

# Target executable
TARGET := dollar
STATIC_TARGET := dollar-static

# Source directories
SRCDIR := src
INCDIR := include
OBJDIR := obj

# Source files
SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

# Header files
HEADERS := $(wildcard $(INCDIR)/*.h)

# Test files
TESTDIR := tests
TESTSOURCES := $(wildcard $(TESTDIR)/*.c)
TEST_OBJS := $(patsubst $(TESTDIR)/%.c,$(OBJDIR)/test_%,$(TESTSOURCES))

# Phony targets
.PHONY: all clean install uninstall test check-deps test-parser test-security

# Default target
all: check-deps $(TARGET)

# Check dependencies
check-deps:
	@command -v $(CC) >/dev/null 2>&1 || { echo "Error: $(CC) not found"; exit 1; }
	@command -v curl-config >/dev/null 2>&1 || { echo "Error: libcurl not found"; exit 1; }
	@pkg-config --exists jansson 2>/dev/null || { echo "Error: jansson not found"; exit 1; }

# Check test dependencies
check-test-deps:
	@pkg-config --exists cmocka 2>/dev/null || { echo "Error: cmocka not found (install libcmocka-dev)"; exit 1; }

# Include directory
$(shell $(MKDIR) $(OBJDIR) 2>/dev/null)

# Create object directory
$(OBJDIR):
	$(MKDIR) $(OBJDIR)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) | $(OBJDIR)
	@echo "CC  $@"
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Link executable
$(TARGET): $(OBJECTS)
	@echo "LD  $@"
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Static build (optional)
static: $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
	@echo "LD  $(STATIC_TARGET) (static)"
	$(CC) $(CFLAGS) $^ -o $(STATIC_TARGET) -static $(LDFLAGS)

# Install target (safe installation)
install: $(TARGET)
	@echo "Installing $(TARGET)..."

	# Check if destination is writable
	@if [ -w $(BINDIR) ]; then \
		echo "Installing to $(BINDIR) (no sudo needed)"; \
		cp $(TARGET) $(BINDIR)/; \
		chmod 755 $(BINDIR)/$(TARGET); \
	else \
		echo "Destination $(BINDIR) is not writable by current user."; \
		echo "Options:"; \
		echo "  1. Run: sudo make install"; \
		echo "  2. Install locally: make install PREFIX=~/local"; \
		echo "  3. Add user to group with write permissions"; \
		exit 1; \
	fi

	@echo "Installation complete!"
	@echo "Run '$(TARGET)' to use the application."

# Install with sudo (requires explicit confirmation)
sudo-install: $(TARGET)
	@echo "Installing $(TARGET) with sudo..."
	@read -p "This will use sudo. Continue? (y/N) " -n 1 -r; \
	echo; \
	if [[ $$REPLY =~ ^[Yy]$$ ]]; then \
		sudo cp $(TARGET) $(BINDIR)/; \
		sudo chmod 755 $(BINDIR)/$(TARGET); \
		echo "Installation complete!"; \
	else \
		echo "Installation cancelled."; \
		exit 1; \
	fi

# Uninstall target
uninstall:
	@echo "Uninstalling $(TARGET)..."
	@if [ -f $(BINDIR)/$(TARGET) ]; then \
		if [ -w $(BINDIR) ]; then \
			rm -f $(BINDIR)/$(TARGET); \
			echo "Uninstalled $(TARGET)"; \
		else \
			sudo rm -f $(BINDIR)/$(TARGET); \
			echo "Uninstalled $(TARGET) with sudo"; \
		fi \
	else \
		echo "$(TARGET) is not installed in $(BINDIR)"; \
	fi

# Test targets
test: check-test-deps $(TARGET) test-parser test-security
	@echo "All tests passed!"

TEST_LDFLAGS := -lcmocka -lcurl -ljansson -lm

$(OBJDIR)/test_parser: tests/test_parser.c $(filter-out $(OBJDIR)/main.o, $(OBJECTS))
	@echo "CC  $@"
	$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@ $(TEST_LDFLAGS)

$(OBJDIR)/test_security: tests/test_security.c $(OBJDIR)/security.o
	@echo "CC  $@"
	$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@ $(TEST_LDFLAGS) -lm

test-parser: $(OBJDIR)/test_parser
	@echo "Running parser tests..."
	@./$(OBJDIR)/test_parser

test-security: $(OBJDIR)/test_security
	@echo "Running security tests..."
	@./$(OBJDIR)/test_security

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	$(RM) $(TARGET) $(STATIC_TARGET) $(TEST_OBJS)
	$(RM) -r $(OBJDIR)
	$(RM) -r *.o
	@echo "Cleanup complete."

# Distribution clean (removes everything including static builds)
distclean: clean
	@echo "Cleaning distribution files..."

# Development helpers
dev-deps:
	@echo "Checking development dependencies..."
	@command -v valgrind >/dev/null 2>&1 || echo "Warning: valgrind not found (optional)"
	@command -v cppcheck >/dev/null 2>&1 || echo "Warning: cppcheck not found (optional)"

# Static analysis
check:
	@echo "Running static analysis..."
	@cppcheck --enable=all --suppress=missingIncludeSystem $(SRCDIR)/*.c 2>/dev/null || echo "cppcheck not available"

# Help
help:
	@echo "Dollar Price Checker - Build System"
	@echo "Usage:"
	@echo "  make              # Build the application"
	@echo "  make install      # Install (safe, checks permissions)"
	@echo "  make sudo-install # Install with sudo (requires confirmation)"
	@echo "  make uninstall    # Uninstall"
	@echo "  make clean        # Clean build files"
	@echo "  make check        # Run static analysis"
	@echo "  make test         # Run tests"

# Show configuration
info:
	@echo "Dollar Price Checker Build Info:"
	@echo "  Version:        $(VERSION)"
	@echo "  Platform:       $(UNAME_S) ($(UNAME_M))"
	@echo "  Compiler:       $(CC)"
	@echo "  Prefix:         $(PREFIX)"
	@echo "  Bin Directory:  $(BINDIR)"
	@echo "  CFLAGS:         $(CFLAGS)"
	@echo "  LDFLAGS:        $(LDFLAGS)"

# Version file for inclusion
version.c: $(SOURCES)
	@echo "const char *VERSION = \"$(VERSION)\";" > $(SRCDIR)/version.c

# Dependencies
$(OBJECTS): $(HEADERS)