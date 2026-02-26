#!/bin/bash

# Color definitions
GREEN_BOLD='\033[1;32m'
RED_BOLD='\033[1;31m'
BLUE_BOLD='\033[1;34m'
RESET='\033[0m'

# Test counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Compile ttcp
echo "Compiling ttcp..."
make
if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi
echo "Compilation successful."

# Function to run a test case
run_test() {
    local description="$1"
    local receiver_cmd="$2"
    local transmitter_cmd="$3"
    
    echo "--------------------------------------------------"
    echo "Running Test: $description"
    
    # Start receiver in background
    $receiver_cmd > /dev/null 2>&1 &
    RECEIVER_PID=$!
    
    # Wait for receiver to start up
    sleep 1
    
    # Run transmitter
    $transmitter_cmd
    TRANSMITTER_EXIT=$?
    
    # Check if transmitter succeeded
    if [ $TRANSMITTER_EXIT -eq 0 ]; then
        echo -e "${GREEN_BOLD}Test PASSED: $description${RESET}"
        ((PASSED_TESTS++))
    else
        echo -e "${RED_BOLD}Test FAILED: $description (Exit code: $TRANSMITTER_EXIT)${RESET}"
        ((FAILED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    
    # Kill receiver process since it runs in an infinite loop
    kill $RECEIVER_PID 2>/dev/null
    wait $RECEIVER_PID 2>/dev/null
 
    echo ""
    
}

# Function to run a negative test case (expect failure)
run_negative_test() {
    local description="$1"
    local cmd="$2"
    
    echo "--------------------------------------------------"
    echo "Running Negative Test: $description"
    
    # Run command and capture output
    $cmd > /dev/null 2>&1
    CMD_EXIT=$?
    
    # For negative tests, non-zero exit code is expected (success for the test)
    if [ $CMD_EXIT -ne 0 ]; then
        echo -e "${GREEN_BOLD}Test PASSED: $description (Expected failure, got exit code: $CMD_EXIT)${RESET}"
        ((PASSED_TESTS++))
    else
        echo -e "${RED_BOLD}Test FAILED: $description (Expected failure but succeeded)${RESET}"
        ((FAILED_TESTS++))
    fi
    ((TOTAL_TESTS++))
    
    echo ""
    
}

# Test 1: TCP Transfer (default port 5001)
# Receiver: Sink mode (-s), Receive (-r)
# Transmitter: Source mode (-s), Transmit (-t), 1000 buffers (-n 1000), to localhost
run_test "TCP Transfer (1000 buffers)" "./ttcp.out -r -s" "./ttcp.out -t -s -n 1000 localhost"

# Test 2: UDP Transfer
# Receiver: UDP (-u), Sink mode (-s), Receive (-r)
# Transmitter: UDP (-u), Source mode (-s), Transmit (-t), 1000 buffers (-n 1000), to localhost
run_test "UDP Transfer (1000 buffers)" "./ttcp.out -r -s -u" "./ttcp.out -t -s -u -n 1000 localhost"

# Test 3: Custom Buffer Size
# Receiver: Buffer size 4096 (-l 4096), Sink mode (-s), Receive (-r)
# Transmitter: Buffer size 4096 (-l 4096), Source mode (-s), Transmit (-t), 1000 buffers (-n 1000), to localhost
run_test "TCP Transfer with Custom Buffer Size (4096)" "./ttcp.out -r -s -l 4096" "./ttcp.out -t -s -l 4096 -n 1000 localhost"

# Test 4: IPv4 Only
run_test "TCP Transfer IPv4 Only" "./ttcp.out -r -s -4" "./ttcp.out -t -s -4 -n 500 localhost"

# Test 5: Custom Port
run_test "TCP Transfer with Custom Port (5002)" "./ttcp.out -r -s -p 5002" "./ttcp.out -t -s -p 5002 -n 500 localhost"

# Test 6: TCP_NODELAY option
run_test "TCP Transfer with TCP_NODELAY" "./ttcp.out -r -s" "./ttcp.out -t -s -D -n 500 localhost"

# Test 7: Socket Buffer Size
run_test "TCP Transfer with Socket Buffer Size (64KB)" "./ttcp.out -r -s -b 65536" "./ttcp.out -t -s -b 65536 -n 500 localhost"

# Test 8: Verbose Mode
run_test "TCP Transfer with Verbose Mode" "./ttcp.out -r -s -v" "./ttcp.out -t -s -v -n 500 localhost"

# Test 9: Write Interval (microseconds between writes)
run_test "TCP Transfer with Write Interval (100us)" "./ttcp.out -r -s" "./ttcp.out -t -s -w 100 -n 500 localhost"

# Test 10: Touch Data (access each byte as it's read)
run_test "TCP Transfer with Touch Data" "./ttcp.out -r -s -T" "./ttcp.out -t -s -n 500 localhost"

# Test 11: Output Format - Kilobits
run_test "TCP Transfer with Output Format (k - kilobits)" "./ttcp.out -r -s" "./ttcp.out -t -s -f k -n 500 localhost"

# Test 12: Output Format - Megabytes
run_test "TCP Transfer with Output Format (M - megabytes)" "./ttcp.out -r -s" "./ttcp.out -t -s -f M -n 500 localhost"

# Test 13: Buffer Alignment
run_test "TCP Transfer with Buffer Alignment (4096)" "./ttcp.out -r -s -A 4096" "./ttcp.out -t -s -A 4096 -n 500 localhost"

# Test 14: Buffer Offset
run_test "TCP Transfer with Buffer Offset (256)" "./ttcp.out -r -s -O 256" "./ttcp.out -t -s -O 256 -n 500 localhost"

# Test 15: Small Buffer Size (minimum for UDP is 5)
run_test "UDP Transfer with Small Buffer (64 bytes)" "./ttcp.out -r -s -u -l 64" "./ttcp.out -t -s -u -l 64 -n 1000 localhost"

# Test 16: Large Number of Buffers
run_test "TCP Transfer with Large Number of Buffers (5000)" "./ttcp.out -r -s" "./ttcp.out -t -s -n 5000 localhost"

# Test 17: Combined Options (verbose + nodelay + custom buffer)
run_test "TCP Transfer with Combined Options" "./ttcp.out -r -s -v -l 16384" "./ttcp.out -t -s -v -D -l 16384 -n 500 localhost"

# Test 18: Source Port binding
run_test "TCP Transfer with Source Port (6000)" "./ttcp.out -r -s" "./ttcp.out -t -s -P 6000 -n 500 localhost"

# ============================================
# Negative Test Cases (Expected to Fail)
# ============================================

# Negative Test 1: Connect to non-existent host
run_negative_test "Connect to Non-existent Host" "./ttcp.out -t -s -n 10 this.host.does.not.exist.example"

# Negative Test 2: Invalid port number (too low)
run_negative_test "Invalid Port Number (Port 0)" "./ttcp.out -t -s -p 0 -n 10 localhost"

# Negative Test 3: Missing required host argument for transmitter
run_negative_test "Missing Host Argument for Transmitter" "./ttcp.out -t -s -n 10"

# Negative Test 4: Invalid option
run_negative_test "Invalid Option (-Z)" "./ttcp.out -t -s -Z -n 10 localhost"

# ============================================
# Forced Failure Test Cases (Demonstrate Failure Output)
# ============================================

# Forced Failure 1: Transmit to port with no receiver (will timeout/fail)
run_test "TCP Transfer to Port with No Receiver (Forced Failure)" "sleep 1" "./ttcp.out -t -s -p 9999 -n 10 localhost"

# Forced Failure 2: Use non-existent executable
run_test "Non-existent Executable (Forced Failure)" "sleep 1" "./nonexistent_ttcp.out -t -s -n 10 localhost"

echo "=================================================="
echo -e "${BLUE_BOLD}Test Summary${RESET}"
echo "=================================================="
echo -e "Total Tests:  $TOTAL_TESTS"
echo -e "Passed:       ${GREEN_BOLD}$PASSED_TESTS${RESET}"
echo -e "Failed:       ${RED_BOLD}$FAILED_TESTS${RESET}"
echo "=================================================="

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN_BOLD}All tests completed successfully!${RESET}"
    exit 0
else
    echo -e "${RED_BOLD}Some tests failed!${RESET}"
    exit 1
fi
