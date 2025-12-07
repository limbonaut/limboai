#!/bin/bash
# Parallel Evolutionary Optimizer for GOAP AI
# Runs multiple Godot instances to speed up fitness evaluation

GODOT_PATH="/Applications/Godot.app/Contents/MacOS/Godot"
PROJECT_PATH="/Users/mfuechec/Desktop/GauntletProjects/limboai/demo"
SCENE_PATH="res://demo/ai/goap/optimization/optimization_runner.tscn"

# Configuration
NUM_WORKERS=${1:-4}  # Number of parallel Godot instances (default: 4)
GENERATIONS=${2:-20}  # Number of generations to run
OUTPUT_DIR="${PROJECT_PATH}/demo/ai/goap/optimization/results"

echo "=========================================="
echo "  Parallel GOAP Evolutionary Optimizer"
echo "=========================================="
echo "Workers: ${NUM_WORKERS}"
echo "Generations: ${GENERATIONS}"
echo "Output: ${OUTPUT_DIR}"
echo ""

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Kill any existing Godot processes to start clean
pkill -f "Godot.*optimization" 2>/dev/null
pkill -f "Godot.*headless" 2>/dev/null
sleep 1

echo "Starting ${NUM_WORKERS} parallel optimization workers..."
echo ""

# Start workers
PIDS=()
LOG_FILES=()

for i in $(seq 1 $NUM_WORKERS); do
    LOG_FILE="${OUTPUT_DIR}/worker_${i}.log"
    LOG_FILES+=("$LOG_FILE")

    echo "Starting worker ${i}..."

    # Run Godot in headless mode with worker ID
    "${GODOT_PATH}" --headless \
        --path "${PROJECT_PATH}" \
        "${SCENE_PATH}" \
        -- --worker-id=${i} --total-workers=${NUM_WORKERS} --generations=${GENERATIONS} \
        > "${LOG_FILE}" 2>&1 &

    PIDS+=($!)

    # Small delay to avoid race conditions
    sleep 0.5
done

echo ""
echo "All workers started. PIDs: ${PIDS[*]}"
echo ""
echo "Monitoring progress... (Ctrl+C to stop)"
echo ""

# Function to show progress from all workers
show_progress() {
    clear
    echo "=========================================="
    echo "  Parallel Optimization Progress"
    echo "=========================================="
    echo ""

    for i in $(seq 1 $NUM_WORKERS); do
        LOG_FILE="${OUTPUT_DIR}/worker_${i}.log"
        if [ -f "$LOG_FILE" ]; then
            # Get last few relevant lines
            LAST_GEN=$(grep -o "Generation [0-9]*/[0-9]*" "$LOG_FILE" 2>/dev/null | tail -1)
            BEST_FIT=$(grep -o "Best fitness: [0-9.]*" "$LOG_FILE" 2>/dev/null | tail -1)
            STATUS=$(grep -E "(Tournament|Creating|Converged|complete)" "$LOG_FILE" 2>/dev/null | tail -1)

            echo "Worker ${i}: ${LAST_GEN:-Starting...} | ${BEST_FIT:-...} | ${STATUS:-Initializing}"
        else
            echo "Worker ${i}: Waiting for log..."
        fi
    done

    echo ""
    echo "----------------------------------------"
    echo "Logs saved to: ${OUTPUT_DIR}"
}

# Monitor loop
RUNNING=true
trap 'RUNNING=false; echo ""; echo "Stopping workers..."; kill ${PIDS[*]} 2>/dev/null' INT TERM

while $RUNNING; do
    # Check if any workers are still running
    ALL_DONE=true
    for pid in "${PIDS[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            ALL_DONE=false
            break
        fi
    done

    if $ALL_DONE; then
        echo ""
        echo "All workers completed!"
        break
    fi

    show_progress
    sleep 5
done

echo ""
echo "=========================================="
echo "  Optimization Complete!"
echo "=========================================="
echo ""

# Find best genome across all workers
echo "Collecting results..."
BEST_FITNESS=0
BEST_FILE=""

for LOG_FILE in "${LOG_FILES[@]}"; do
    if [ -f "$LOG_FILE" ]; then
        FIT=$(grep -o "Best fitness: [0-9.]*" "$LOG_FILE" 2>/dev/null | tail -1 | grep -o "[0-9.]*")
        if [ -n "$FIT" ]; then
            # Compare floats using bc
            if (( $(echo "$FIT > $BEST_FITNESS" | bc -l) )); then
                BEST_FITNESS=$FIT
                BEST_FILE=$LOG_FILE
            fi
        fi
    fi
done

echo ""
echo "Best overall fitness: ${BEST_FITNESS}"
echo "From: ${BEST_FILE}"
echo ""
echo "Check individual logs in: ${OUTPUT_DIR}"
