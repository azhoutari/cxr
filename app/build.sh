#!/bin/bash

# Check if the first argument is "rebuild"
if [ "$1" == "rebuild" ]; then
    echo "Rebuilding the project..."
    rm -rf build/*
    cmake -S . -B build
fi 

cmake --build build
