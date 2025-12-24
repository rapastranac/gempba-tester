# ---------------------------------------------------------------------
$ErrorActionPreference = "Stop"

Write-Host "Current working directory: $( Get-Location )"
Write-Host "Starting run at: $( Get-Date )"

$TASKS_PER_NODE = 2

mpiexec.exe -n $TASKS_PER_NODE ./bin/main

# ---------------------------------------------------------------------
Write-Host "Finishing run at: $( Get-Date )"
# ---------------------------------------------------------------------
