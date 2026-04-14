#!/bin/bash

# Simple benchmarking script for epoll server
# Usage: ./benchmark_epoll.sh <num_requests>

NUM_REQS=${1:-50}
CONCURRENCY=5
PORT=8080

echo "Running benchmark with $NUM_REQS requests (concurrency: $CONCURRENCY)..."

# Ensure server and client are compiled
gcc server_epoll.c -o server_epoll
gcc client_test.c -o client_test

# Start the server if it's not already running
./server_epoll > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1

# Performance measurement
start_time=$(date +%s.%N)

# Launch requests in batches
for ((i=1; i<=NUM_REQS; i+=CONCURRENCY)); do
    for ((j=0; j<CONCURRENCY; j++)); do
        ./client_test "Benchmarking request $((i+j))" > /dev/null &
    done
    wait
done

end_time=$(date +%s.%N)
duration=$(echo "$end_time - start_time" | bc)

echo "Total time: $duration seconds"
echo "Avg time per request: $(echo "scale=4; $duration / $NUM_REQS" | bc) seconds"

# Terminate server
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null
echo "Benchmark completed."
