#!/bin/bash
# A launcher helper script to help linux find
# the included shared libraries

app=$(basename "$0" .sh)
export LD_LIBRARY_PATH=$(pwd):$LD_LIBRARY_PATH
./$app