#!/bin/bash

NUM_INSTANCES=16

EXECUTABLE="./client"

for ((i=1; i<=NUM_INSTANCES; i++)); do
    $EXECUTABLE &
done
