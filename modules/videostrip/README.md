## Project: uwimageproc
# Module: videostrip

Module that extracts frames from video for 2D mosaic or 3D model reconstruction. It estimates the overlap among frames by computing the homography matrix. Current OpenCV implementation uses GPU acceleration for feature detection and matching through CUDA library.

## Getting Started


### Prerequisites

* OpenCV 3.2
* opencv-contrib 3.2.1
* CUDA 8.0 (for GPU support)


### Installing

A step by step series of examples that tell you have to get a development env running


```
git clone https://github.com/MecatronicaUSB/uwimageproc.git
cd modules/videostrip
mkdir build
cd build
cmake ..
make
```

## Running 

```
$ videostrip -p=0.6 -k=5 -s=12 input.avi vdout_
```

This will open 'input.avi' file, extract frames with 60% of overlapping, skipping first 12 seconds, and export into 'vdout_XXXX.jpg' images


## Built With

* [CLion](https://www.jetbrains.com/clion/) - Dependency Management

## Contributing

TBA

## Versioning

Github

## Authors and Contributors

* **José Cappelletto** - *Initial work* - [cappelletto](https://github.com/cappelletto)

* **Víctor García** - *CPU/GPU merge* - [vgarciac](https://github.com/vgarciac)

## License

This project is licensed under GPL v3 - see the [LICENSE.md](LICENSE.md) file for details

