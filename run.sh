#!/bin/bash
# ---------------------------------------------------------------------
set -e

echo "Current working directory: $(pwd)"
echo "Starting run at: $(date)"

TASKS_PER_NODE=2

mpirun -n "$TASKS_PER_NODE" --bind-to core --map-by slot:PE="$CPUS_PER_TASK" --report-bindings ./bin/main


# ---------------------------------------------------------------------
echo "Finishing run at: $(date)"
# ---------------------------------------------------------------------
