#!/bin/bash

# Function to run a single test case
#testcases in input.txt should be of format first line t =  number of testcases, then  x y m whenre x rows, y columns and m instuctions, then m instructions for each tc
run_test_case() {
    local rows=$1
    local cols=$2
    local num_instructions=$3
    local test_num=$4
    local executable="target/release/spreadsheet"

    echo "Running test case $test_num: $rows x $cols grid with $num_instructions instructions"

    # Extract only the next `num_instructions` lines into a temporary file
    head -n "$num_instructions" temp_input.txt > temp_instructions.txt

    # Create an expect script
    cat > temp_expect.sh << EOF
#!/usr/bin/expect -f
set timeout -1
spawn $executable $rows $cols

# Read and execute each instruction
set file [open "temp_instructions.txt" r]
while {[gets \$file line] != -1} {
    expect "> "
    send -- "\$line\r"
}
close \$file

# Send quit command
expect "> "
send -- "q\r"
expect eof
EOF

    # Make the expect script executable
    chmod +x temp_expect.sh

    # Run the expect script and capture output
    ./temp_expect.sh > temp_output.txt 2>/dev/null
    
    # Append the output to the main output file
    cat temp_output.txt >> output.txt
    
    # Add separator after each test case
    echo "*****" >> output.txt

    # Remove temporary files
    rm temp_expect.sh temp_instructions.txt temp_output.txt

    # Skip processed lines in temp_input.txt
    tail -n +$((num_instructions + 1)) temp_input.txt > temp_input2.txt
    mv temp_input2.txt temp_input.txt
}

# Main testing logic
main() {
    local executable="target/release/spreadsheet"
    
    # Clear previous output file
    > output.txt

    # Check if the executable exists
    if [ ! -f "$executable" ]; then
        echo "Error: $executable not found. Please run 'make' first."
        exit 1
    fi

    # Read number of test cases
    read num_tests < input.txt

    # Skip the first line (we already read it)
    tail -n +2 input.txt > temp_input.txt

    # Process each test case
    for ((test=1; test<=num_tests; test++)); do
        # Read test case parameters
        read rows cols num_instructions < temp_input.txt
        tail -n +2 temp_input.txt > temp_input2.txt
        mv temp_input2.txt temp_input.txt

        # Run the test case
        run_test_case "$rows" "$cols" "$num_instructions" "$test"
    done

    # Clean up temporary files
    rm -f temp_input.txt

    # Compare output with expected output
    if diff -w output.txt expected_output.txt > /dev/null; then
        echo "All tests passed successfully!"
        exit 0
    else
        echo "Tests failed. Differences found:"
        diff -w output.txt expected_output.txt
        exit 1
    fi
}

# Check if required files exist
if [ ! -f "target/release/spreadsheet" ]; then
    echo "Error: spreadsheet executable not found. Please run 'make' first."
    exit 1
fi

if [ ! -f "input.txt" ]; then
    echo "Error: input.txt not found"
    exit 1
fi

if [ ! -f "expected_output.txt" ]; then
    echo "Error: expected_output.txt not found"
    exit 1
fi

# Check if expect is installed
if ! command -v expect &> /dev/null; then
    echo "Error: 'expect' command not found. Please install expect package."
    exit 1
fi

# Run the test suite
main
