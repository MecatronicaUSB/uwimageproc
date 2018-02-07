# Project: uwimageproc
# Module: videostrip

Module that applies percentile based Histogram Stretching for specific channels of an input image. Histogram stretching is applied one at time, and then converted back to RGB colour space.
Current OpenCV 3.2 implementation uses GPU acceleration for channel split/merge and pixel value mapping through CUDA library.

Based on A. Longart Python prototype, Diego Peña, Fabio Morales & Victor García percentile based approach, and OpenCV online documentation.

        cout << "Complete options are:" << endl;
        cout << "\t-c=R|G|B\tfor RGB space" << endl;
        cout << "\t-c=H|S|V\tfor HSV space" << endl;
        cout << "\t-c=h|s|l\tfor HSL space" << endl;
        cout << "\t-c=L|a|b\tfor Lab space" << endl;
        cout << "\t-c=Y|C|X\tfor YCrCb space" << endl;
        cout << "\t-cuda=0 or -cuda=1 (CUDA ON: 1, CUDA OFF: 0, if available)" << endl;
        cout << endl << "\tExample:" << endl;


The provided **cmake** file autodetect if CUDA is present, and enable GPU support.

## Getting Started



### Prerequisites

* OpenCV 3.2
* opencv-contrib 3.2.1
* CUDA 8.0 (for GPU support)

### Installing

No special procedures are required to build this specific module. Just standard clone, cmake and make steps.

```
git clone https://github.com/MecatronicaUSB/uwimageproc.git
cd modules/histretch
mkdir build
cd build
cmake ..
make
```

## Running 

For detailed usage information and available options, please run the module without arguments or with 'help'. It can be run directly from console as:

        "\t" << endl << endl;

```
$ histretch -c=HV input.jpg output.jpg -cuda=0 -time=1
```
This will open 'input.jpg' file, operate on the 'H' and 'V' channels, and write it in 'output.jpg', while disabling GPU support, and showing total execution time.


## Built With
* [cmake 3+](https://cmake.org/) - cmake making it happen
* [CLion](https://www.jetbrains.com/clion/) - Just another IDE, pick anyone

## Contributing

TBA

## Versioning

Github

## Authors and Contributors

* **José Cappelletto** - *Initial work* - [cappelletto](https://github.com/cappelletto)
* **Fabio Morales** - *CPU/GPU merge and rework* - [fmoralesh](https://github.com/fmoralesh)

## License

This project is licensed under GNU GPLv3 - see the [LICENSE](LICENSE) file for details

