#!/bin/bash
# Quick progress check for parallel optimization

RESULTS_DIR="/Users/mfuechec/Desktop/GauntletProjects/limboai/demo/demo/ai/goap/optimization/results"

echo "=========================================="
echo "  Parallel Optimization Progress"
echo "=========================================="
echo ""

for i in 1 2 3 4; do
    LOG="${RESULTS_DIR}/worker_${i}.log"
    if [ -f "$LOG" ]; then
        GEN=$(grep -o "Generation [0-9]*/[0-9]*" "$LOG" | tail -1)
        # Get ALL-TIME best from "New best genome!" messages
        ALLTIME_BEST=$(grep "New best genome" "$LOG" | tail -1 | grep -o "Fitness: [0-9.]*" | grep -o "[0-9.]*")
        LATEST=$(grep -E "(Tournament|Creating|Gen)" "$LOG" | tail -1)
        echo "Worker $i: ${GEN:-Starting} | All-time Best: ${ALLTIME_BEST:-?} | ${LATEST}"
    fi
done

echo ""
echo "Active Godot processes:"
pgrep -c -f "Godot.*headless" || echo "0"

echo ""
echo "To stop all workers: pkill -f 'Godot.*headless'"
