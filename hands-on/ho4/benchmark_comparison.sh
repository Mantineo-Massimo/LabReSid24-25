#!/bin/bash

# Benchmark Comparison Script for select (HO3) vs epoll (HO4)
# Usage: ./benchmark_comparison.sh <num_requests>

NUM_REQS=${1:-100}
CONCURRENCY=5
PORT=8080

# Ensure both servers are compiled
gcc ../ho3/server_multiplex.c -o server_selective
gcc ./server_epoll.c -o server_epoll
gcc ./client_test.c -o client_test

run_benchmark() {
    local server_bin=$1
    local name=$2

    echo "Testing $name..." >&2
    ./$server_bin > /dev/null 2>&1 &
    local server_pid=$!
    sleep 1

    local t1=$(/usr/bin/date +%s.%N)
    for ((i=1; i<=NUM_REQS; i+=CONCURRENCY)); do
        for ((j=0; j<CONCURRENCY; j++)); do
            ./client_test "Benchmarking $name request $((i+j))" > /dev/null &
        done
        wait
    done
    local t2=$(/usr/bin/date +%s.%N)
    local diff=$(echo "$t2 - $t1" | bc)
    
    echo "$name result: $diff seconds" >&2
    
    kill $server_pid
    wait $server_pid 2>/dev/null
    sleep 1
    echo "$diff"
}

S_TIME=$(run_benchmark server_selective "select (HO3)")
E_TIME=$(run_benchmark server_epoll "epoll (HO4)")

echo "select (HO3) total: $S_TIME"
echo "epoll (HO4) total: $E_TIME"
