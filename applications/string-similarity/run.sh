#!/bin/bash

# Define arrays for threads and total invocations and iterations per invocation

# Function to handle SIGINT (Ctrl+C)
function handle_sigint() {
    echo "Script interrupted. Exiting..."
    exit 1
}

# Trap SIGINT and call the handle_sigint function
trap handle_sigint SIGINT

filename="strings.txt"
# Define arrays for threads and pairs of invocations and iterations per invocation
threads=(2 4 8)
invocations_iterations=(
#  "10 10"
  "200 500"
  "500 200"
  "1000 100"
  "2000 50"
  "10000 1"
)
export FLEXI_PROVIDER=AWS

# Loop through each combination of threads, invocations, and iterations per invocation
for t in "${threads[@]}"; do
  for pair in "${invocations_iterations[@]}"; do
    # Extract invocations and iterations per invocation from the pair
    read -r invocations iterations <<< "$pair"

    # Execute the ./main command
    ./main $filename $t $invocations $iterations

    # Execute the ./mainlocal command
    ./mainlocal $filename $t $invocations $iterations
  done
done

