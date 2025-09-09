#!/bin/bash

NUM_INSTANCES=640

EXECUTABLE="./client"

for ((i=0; i<NUM_INSTANCES; i++)); do
    $EXECUTABLE &
done
