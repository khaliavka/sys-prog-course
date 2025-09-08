#!/bin/bash

NUM_INSTANCES=64

EXECUTABLE="./client"

for ((i=1; i<=NUM_INSTANCES; i++)); do
    $EXECUTABLE &
done
