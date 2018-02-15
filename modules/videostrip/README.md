# Project: uwimageproc
# Module: videostrip

Module for smart video frames extraction, useful for 2D mosaic or 3D model reconstruction. It estimates the overlap among frames by computing the homography matrix and intersecting frame boundaries. Current OpenCV 3.2 implementation uses GPU acceleration for feature detection and matching through CUDA library. The provided **cmake** file autodetect if CUDA is present, and enable GPU support. Current version supports only JPEG files as output, but further output file formats will be added soon.

## Getting Started

### Prerequisites

* OpenCV 3.2
* opencv-contrib 3.2.1
* CUDA 8.0 (for GPU support)

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uwimageproc.git
cd modules/videostrip
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:


```
$ videostrip -p 0.6 -k 5 -s 12 input.avi vdout_
```

This will open 'input.avi' file, extract the frames with a target of 60% of overlapping, while skipping the first 12 seconds. It will export the frames as 'vdout_XXXX.jpg' images


## Built With
* [cmake 2.8](https://cmake.org/) - cmake making it happen
* [CLion](https://www.jetbrains.com/clion/) - Just another IDE, pick anyone

## Contributing

See contributing guidelines for base project **uwimageproc**

## Versioning

GitHub

## Authors and Contributors

* **José Cappelletto** - *Initial work and maintenance* - [cappelletto](https://github.com/cappelletto)

* **Víctor García** - *CPU/GPU merge and rework* - [vgarciac](https://github.com/vgarciac)

* **Fabio Morales** - *Arg parsing, cmake improvements* - [fmoralesh](https://github.com/fmoralesh)

## License

This project is licensed under GNU GPLv3 - see the [LICENSE](LICENSE) file for details

