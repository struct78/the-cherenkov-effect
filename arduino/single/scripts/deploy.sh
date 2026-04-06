#!/bin/bash

# Build and Deploy Script for Arduino Giga - The Cherenkov Effect
# This script checks for required libraries, compiles, and uploads the sketch

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
ARDUINO_CLI_PATH="arduino-cli"
BOARD_FQBN="arduino:mbed_giga:giga"
SKETCH_PATH="./TheCherenkovEffect"
SERIAL_BAUD_RATE=9600
PORT=""

# Required libraries (name:version format, empty version means latest)
REQUIRED_LIBS=(
    "Control Surface"
)

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to check if arduino-cli is installed
check_arduino_cli() {
    print_status "Checking for arduino-cli..."
    if ! command -v $ARDUINO_CLI_PATH &> /dev/null; then
        print_error "arduino-cli not found. Please install it first:"
        echo "  - macOS: brew install arduino-cli"
        echo "  - Or download from: https://arduino.github.io/arduino-cli/latest/installation/"
        exit 1
    fi
    print_success "arduino-cli found: $(arduino-cli version)"
}

# Function to update core index
update_core_index() {
    print_status "Updating Arduino core index..."
    $ARDUINO_CLI_PATH core update-index
}

# Function to install Arduino Giga core
install_board_core() {
    print_status "Checking if Arduino Giga core is installed..."
    if ! $ARDUINO_CLI_PATH core list | grep -q "arduino:mbed_giga"; then
        print_status "Installing Arduino Giga core..."
        $ARDUINO_CLI_PATH core install arduino:mbed_giga
        print_success "Arduino Giga core installed"
    else
        print_success "Arduino Giga core already installed"
    fi
}

# Function to check and install required libraries
check_and_install_libraries() {
    print_status "Checking required libraries..."
    
    for lib_spec in "${REQUIRED_LIBS[@]}"; do
        # Split library name and version
        IFS=':' read -r lib_name lib_version <<< "$lib_spec"
        
        print_status "Checking library: $lib_name"
        
        # Check if library is installed
        if ! $ARDUINO_CLI_PATH lib list | grep -q "^$lib_name"; then
            print_status "Installing library: $lib_name"
            if [ -n "$lib_version" ]; then
                $ARDUINO_CLI_PATH lib install "$lib_name@$lib_version"
            else
                $ARDUINO_CLI_PATH lib install "$lib_name"
            fi
            print_success "Installed: $lib_name"
        else
            print_success "Already installed: $lib_name"
        fi
    done
}

# Function to detect available ports
detect_port() {
    print_status "Detecting available Arduino ports..."
    
    # Get available ports
    available_ports=$($ARDUINO_CLI_PATH board list | grep -E "(tty\.usbmodem|cu\.usbmodem)" | awk '{print $1}')
    
    if [ -z "$available_ports" ]; then
        print_warning "No Arduino boards detected. Please ensure your Arduino Giga is connected."
        read -p "Enter the port manually (e.g., /dev/tty.usbmodem101): " manual_port
        if [ -n "$manual_port" ]; then
            PORT="$manual_port"
        else
            print_error "No port specified. Cannot upload."
            exit 1
        fi
    else
        # If only one port, use it automatically
        port_count=$(echo "$available_ports" | wc -l)
        if [ "$port_count" -eq 1 ]; then
            PORT="$available_ports"
            print_success "Auto-detected port: $PORT"
        else
            # Multiple ports, ask user to choose
            print_status "Multiple ports detected:"
            echo "$available_ports" | nl -w2 -s') '
            read -p "Select port number: " port_choice
            PORT=$(echo "$available_ports" | sed -n "${port_choice}p")
            if [ -z "$PORT" ]; then
                print_error "Invalid selection."
                exit 1
            fi
            print_success "Selected port: $PORT"
        fi
    fi
}

# Function to compile the sketch
compile_sketch() {
    print_status "Compiling sketch..."
    
    if ! $ARDUINO_CLI_PATH compile --fqbn $BOARD_FQBN "$SKETCH_PATH"; then
        print_error "Compilation failed!"
        exit 1
    fi
    
    print_success "Compilation successful!"
}

# Function to upload the sketch
upload_sketch() {
    if [ -z "$PORT" ]; then
        detect_port
    fi
    
    print_status "Uploading sketch to $PORT..."
    
    if ! $ARDUINO_CLI_PATH upload -p "$PORT" --fqbn $BOARD_FQBN "$SKETCH_PATH"; then
        print_error "Upload failed!"
        exit 1
    fi
    
    print_success "Upload successful!"
}

# Function to start serial monitor
start_serial_monitor() {
    if [ -z "$PORT" ]; then
        detect_port
    fi
    
    print_status "Starting serial monitor on $PORT at ${SERIAL_BAUD_RATE} baud..."
    print_status "Press Ctrl+C to exit the serial monitor"
    echo ""
    
    # Wait for the port to re-enumerate after upload/reset
    sleep 2
    
    # Start the serial monitor with baud rate configuration
    if ! $ARDUINO_CLI_PATH monitor -p "$PORT" --config baudrate=$SERIAL_BAUD_RATE; then
        print_error "Serial monitor failed to start!"
        exit 1
    fi
}

# Function to show sketch info
show_sketch_info() {
    print_status "Sketch Information:"
    echo "  Path: $(realpath "$SKETCH_PATH")"
    echo "  Board: Arduino Giga"
    echo "  FQBN: $BOARD_FQBN"
    if [ -n "$PORT" ]; then
        echo "  Port: $PORT"
    fi
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  -p, --port PORT     Specify the Arduino port (e.g., /dev/tty.usbmodem101)"
    echo "  -c, --compile-only  Only compile, don't upload"
    echo "  -u, --upload-only   Only upload, skip compilation"
    echo "  -l, --libs-only     Only check and install libraries"
    echo "  -s, --serial        Start serial monitor after upload (or use alone)"
    echo "  -i, --info         Show sketch information"
    echo "  -h, --help         Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                          # Full build and deploy"
    echo "  $0 -c                       # Compile only"
    echo "  $0 -s                       # Upload and start serial monitor"
    echo "  $0 --serial                 # Start serial monitor only"
    echo "  $0 -p /dev/tty.usbmodem101  # Use specific port"
    echo "  $0 -l                       # Install libraries only"
}

# Parse command line arguments
COMPILE_ONLY=false
UPLOAD_ONLY=false
LIBS_ONLY=false
SERIAL_MONITOR=false
SHOW_INFO=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--port)
            PORT="$2"
            shift 2
            ;;
        -c|--compile-only)
            COMPILE_ONLY=true
            shift
            ;;
        -u|--upload-only)
            UPLOAD_ONLY=true
            shift
            ;;
        -l|--libs-only)
            LIBS_ONLY=true
            shift
            ;;
        -s|--serial)
            SERIAL_MONITOR=true
            shift
            ;;
        -i|--info)
            SHOW_INFO=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Main execution
echo "==============================================="
echo "  Arduino Giga - The Cherenkov Effect"
echo "  Build and Deploy Script"
echo "==============================================="
echo ""

# Show info and exit if requested
if [ "$SHOW_INFO" = true ]; then
    show_sketch_info
    exit 0
fi

# Check sketch exists
if [ ! -f "$SKETCH_PATH/TheCherenkovEffect.ino" ]; then
    print_error "Sketch not found at: $SKETCH_PATH/TheCherenkovEffect.ino"
    exit 1
fi

# Execute based on options
if [ "$LIBS_ONLY" = true ]; then
    check_arduino_cli
    update_core_index
    check_and_install_libraries
    exit 0
fi

if [ "$UPLOAD_ONLY" = true ]; then
    check_arduino_cli
    upload_sketch
    exit 0
fi

if [ "$SERIAL_MONITOR" = true ] && [ "$COMPILE_ONLY" = false ] && [ "$UPLOAD_ONLY" = false ] && [ "$LIBS_ONLY" = false ]; then
    check_arduino_cli
    start_serial_monitor
    exit 0
fi

# Default: Full build and deploy process
check_arduino_cli
update_core_index
install_board_core
check_and_install_libraries

if [ "$COMPILE_ONLY" = false ]; then
    compile_sketch
fi

if [ "$COMPILE_ONLY" = false ]; then
    upload_sketch
    
    if [ "$SERIAL_MONITOR" = true ]; then
        echo ""
        print_success "Build and deploy completed successfully!"
        print_status "Your Arduino Giga is now running The Cherenkov Effect!"
        echo ""
        start_serial_monitor
    else
        echo ""
        print_success "Build and deploy completed successfully!"
        print_status "Your Arduino Giga is now running The Cherenkov Effect!"
    fi
else
    echo ""
    print_success "Compilation completed successfully!"
fi
