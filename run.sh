#!/bin/bash
# ---------------------------------------------------------------------
set -e

echo "Current working directory: $(pwd)"
echo "Starting run at: $(date)"

TASKS_PER_NODE=2

mpirun --oversubscribe -n $TASKS_PER_NODE -display-map --bind-to none --map-by core --report-bindings ./bin/main


# ---------------------------------------------------------------------
echo "Finishing run at: $(date)"
# ---------------------------------------------------------------------
