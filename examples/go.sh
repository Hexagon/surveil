#!/bin/bash

# The image is created and updated by some other program in this example
FILENAME=/run/shm/surveil.png
DESTDIR=/home/pi/motion
SENSITIVITY=27
PASSES=6

surveil --current $FILENAME --sensitivity $SENSITIVITY --passes $PASSES --out "$DESTDIR/motion_%F_%H%M%S.png" -v
