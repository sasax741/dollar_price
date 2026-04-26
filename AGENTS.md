# AGENTS.md - Dollar Price Checker

This document provides essential information for AI agents working with the Dollar Price Checker project.

## Project Overview

A simple command-line application written in C that fetches current USD exchange rates in Argentina from the public API at `https://dolarapi.com`. The goal is to have a quick command available in the system PATH for checking dollar exchange rates from any terminal.

## Platform Support

- **Linux** (primary target)
- **macOS** (with Homebrew for dependency management)

## Project Structure

```
.
├── dollar.c          # Main C source code
├── Makefile          # Build automation
├── README.md         # Project documentation
├── logo.svg          # Project logo
├── .gitignore        # Git ignore rules
└── AGENTS.md         # This file
```

## Essential Commands

### Build Commands

```bash
# Compile the project (default target)
make

# Compile and install to /usr/local/bin (requires sudo)
sudo make install

# Clean compiled binaries
make clean
```

### Running the Application

```bash
# After installation
dollar

# Run directly from project directory (after compilation)
./dollar
```

## Dependencies

### Required Libraries
- **libcurl**: HTTP client library for API requests
- **jansson**: JSON parsing library for processing API responses

### Installation Commands

**Debian/Ubuntu (Linux):**
```bash
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev libjansson-dev
```

**macOS (with Homebrew):**
```bash
brew install curl jansson
```

## Build System Details

### Makefile Features

1. **Cross-platform support**: Automatically detects Linux vs macOS
2. **macOS Homebrew integration**: Adds Homebrew library paths when available
3. **Dependency checking**: On macOS, verifies Homebrew libraries are installed
4. **Standard targets**: `all`, `install`, `clean`

### Compilation Flags
- `-Wall -Wextra`: Enable all warnings
- `-std=c99`: Use C99 standard
- `-g`: Include debug symbols
- `-lcurl -ljansson`: Link against required libraries

## Code Patterns and Conventions

### Source Code Structure (`dollar.c`)

1. **Response buffer**: `struct ResponseBuffer` - stores accumulated HTTP response data with size tracking
2. **Callback function**: `WriteCallback()` - properly accumulates HTTP response chunks across multiple calls
3. **Main function flow**:
   - Initialize libcurl
   - Make HTTP GET request to API
   - Parse JSON response with jansson
   - Iterate through exchange rate data
   - Display formatted results
   - Clean up resources

### Key Implementation Details

1. **Memory Management**:
   - Uses `malloc()` and `realloc()` for dynamic memory allocation
   - Always calls `free()` and `json_decref()` for cleanup
   - Includes error checking for memory allocation failures

2. **JSON Processing**:
   - Handles both integer and real number types for prices
   - Extracts: `casa`, `moneda`, `compra`, `venta`, `fechaActualizacion`
   - Converts all prices to `double` for consistent formatting

3. **Error Handling**:
   - Checks CURL operation results
   - Validates JSON parsing
   - Handles unexpected data types gracefully

4. **Platform-Specific Code**:
   - Uses `#ifndef _WIN32` to exclude Windows-specific code
   - Includes timeout and SSL verification for secure HTTP requests

## Testing

**Note**: No formal test suite exists. The application is tested by:
1. Compilation without warnings
2. Successful execution and API response
3. Proper formatting of output data

## Gotchas and Important Notes

1. **Memory Management**: Uses a `ResponseBuffer` struct to properly accumulate HTTP response data across multiple callbacks. The global variable has been eliminated.

2. **API Dependency**: The application depends on the external API at `https://dolarapi.com`. If the API changes or is unavailable, the application will fail.

3. **Memory Safety**: The `ResponseBuffer` struct ensures proper accumulation of HTTP response chunks. Always check `response.data` before parsing JSON.

4. **macOS Compatibility**: The Makefile has special handling for macOS with Homebrew, but requires Homebrew to be installed and configured correctly.

5. **Installation Path**: The `install` target copies to `/usr/local/bin` which requires `sudo` privileges.

## Development Workflow

1. **Make changes** to `dollar.c`
2. **Compile**: `make`
3. **Test**: `./dollar`
4. **Install**: `sudo make install`
5. **Clean**: `make clean` (optional)

## API Information

- **Endpoint**: `https://dolarapi.com/v1/ambito/dolares`
- **Response Format**: JSON array of objects
- **Required Fields**: `casa`, `moneda`, `compra`, `venta`, `fechaActualizacion`
- **Price Types**: Can be integers or floating-point numbers

## Style Guidelines

- **Indentation**: 4 spaces (no tabs)
- **Brace Style**: K&R style (opening brace on same line)
- **Variable Names**: snake_case
- **Function Names**: PascalCase for callback, camelCase for others
- **Comments**: In Spanish (matching existing code)
- **Error Messages**: Printed to stderr with descriptive text

## Future Considerations

1. **Error Recovery**: Add retry logic for failed API requests
2. **Configuration**: Allow custom API endpoints or network interfaces
3. **Testing**: Add unit tests for JSON parsing and error cases
4. **Packaging**: Create distribution packages for easier installation