# uwimageproc

Toolbox for underwater video and image processing. It's intended to be collection of compatible, OpenCV 3+ based, modules for:

- [videostrip](https://github.com/MecatronicaUSB/uwimageproc/tree/master/modules/videostrip) Automatic frame extraction for 2D mosaic and/or 3D reconstruction 
- [aclahe](https://github.com/MecatronicaUSB/uwimageproc/tree/master/modules/aclahe) Automatic Clip-Limit AHE (CLAHE)
- [histretch](https://github.com/MecatronicaUSB/uwimageproc/tree/master/modules/histretch) Histogram stretch w/channel selection
- Automatic 2D mosaic generation > to be moved to (https://github.com/MecatronicaUSB/mosaic-2d)
- 3D sparse and dense reconstruction > to be moved to (https://github.com/MecatronicaUSB/uw-slam)
- GPU based feature detection for homography estimation in underwater videos

Each module or script contains information describing its usage, with (usually) some useful README. Code documentation is provided in Doxygen-compatible format. The current release contains C/C++, Python and/or Matlab implementations.

Each module is covered by GNU GPL v3 license unless otherwise is specified (see LICENSE and README.md) .

