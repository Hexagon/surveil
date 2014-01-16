Surveil
=======

Simple C-program for realtime motion detection of png images.


Installing
=======
git clone https://github.com/Hexagon/surveil.git
cd surveil
make
sudo make install


Usage
=======

Mode 1 - Continous motion detection
---
This program has two modes, in mode 1 (-o) it constantly polls the file provided by -c for changes in motion, and saves current frame to the file specified by -o.

Example:
surveil --current /tmp/surveil.png --out "/home/motion/detections/motion_%F_%H%M%S.png" --sensitivity 27 --passes 6

 * --sensitivity represents the minimum change of intensity (0-255) to identify a change as motion.
 * --passes represents the number of passes of blob reduction (to reduce unwanted noise as snowflakes).

Mode 2 - Difference
---
Not as interesting as motion detection, I'll write a guide to this when there is time for it.


Usage examples is also provided in examples/ dir.

Command line options
=======

	-h, --help		This help

Input:
	-c, --current		Path to current frame (png) (used for both -o and -d)
	-p, --previous		Path to save previous frame (used for -d)

Modes:
	-o, --out		Path to save current frame to on detected motion
	-d, --diff		Path to save cleaned frame to on changed pixels

Options:
	-s, --sensitivity		Intensity treshold, a value between 0 and 255 -s
	-e, --passes		Passes, the minimum radius of an object that should be counted as an object (in pixels) -s
	-t, --tune		Get a suitable vaiable for -s
	-v, --verbose		Print interesting information along the way
