# Installing OpenCV and CUDA 

Step-by-step installation instructions for the development environment setup, with OpenCV 3.2 and contrib with CUDA support for Ubuntu 16.04 x64.

## Table of Contents
- [Preparing the system](#preparing-the-system)
- [OpenCV 3.2](#opencv-32)
- [CUDA 8.0](#cuda-80-optional)
- [Installation without CUDA](#installation-without-cuda)
- [Installation with CUDA](#installation-with-cuda)
- [License](#license)

## Preparing the system

### Dependencies

**Essential**
```bash
sudo apt-get install build-essential cmake pkg-config
```
**Image codecs**
```bash
sudo apt-get install libjpeg8-dev libtiff5-dev libjasper-dev libpng12-dev
```

**Video codecs**
```bash
sudo apt-get install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt-get install libxvidcore-dev libx264-dev
```

**GUI with GTK**
```bash
sudo apt-get install libgtk-3-dev
```

**Matrix operations**
```bash
sudo apt-get install libatlas-base-dev gfortran
```

**Python**
```bash
sudo apt-get install python2.7-dev python3.5-dev
```

## OpenCV 3.2
Choose a path for installation:
```bash
cd <some_directory>
```

**Download OpenCV source code**
```bash
wget https://github.com/opencv/opencv/archive/3.2.0.tar.gz .
tar xvf opencv-3.2.0.tar.gz 
```

**Download OpenCV contrib**
```bash
wget https://codeload.github.com/opencv/opencv_contrib/zip/master -O opencv_contrib-master.zip
unzip opencv_contrib-master.zip
```
*Specified contrib package may have some bugs when compiled against OpenCV 3.2.0*

**OpenCL Support**
```bash
sudo apt-get install libgtkglext1 libgtkglext1-dev
```

### CUDA 8.0 (optional)
The recommended version is 8.0, yet you could try with a different version. We would like to know about implementations with CUDA 9.0.
```bash-
wget https://developer.nvidia.com/compute/cuda/8.0/Prod2/local_installers/cuda-repo-ubuntu1604-8-0-local-ga2_8.0.61-1_amd64-deb
sudo dpkg -i cuda-repo-ubuntu1604-8-0-local_8.0.44-1_amd64.deb
sudo apt-get update
sudo apt-get install cuda
```

### Installation without CUDA
```bash
cd opencv-3.2.0/
mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D WITH_OPENGL=ON \
    -D WITH_CUDA=OFF \
    -D ENABLE_FAST_MATH=1 \
    -D CUDA_FAST_MATH=0 \
    -D WITH_CUBLAS=1 \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-master/modules \
    -DBUILD_PNG=ON \
    -DBUILD_TIFF=ON \
    -DBUILD_JPEG=OFF \
    -DBUILD_ZLIB=ON \
    -DWITH_OPENCL=ON \
    -DWITH_OPENMP=ON \
    -DWITH_FFMPEG=ON \
    -DWITH_GTK=ON \
    -DWITH_VTK=ON \
    -DWITH_TBB=ON \
    -DINSTALL_C_EXAMPLES=ON \
    -DINSTALL_TESTS=OFF \
	-D BUILD_DOCS=OFF \
	-D OPENCV_ENABLE_NONFREE=ON \
    -D BUILD_EXAMPLES=OFF ..

make
sudo make install
```

### Installation with CUDA
```bash
cd opencv-3.2.0/
mkdir build
cd build

cmake -D CMAKE_BUILD_TYPE=RELEASE \
    -D CMAKE_INSTALL_PREFIX=/usr/local \
    -D WITH_OPENGL=ON \
    -D WITH_CUDA=ON \
    -D ENABLE_FAST_MATH=1 \
    -D CUDA_FAST_MATH=1 \
    -D WITH_CUBLAS=1 \
    -D INSTALL_PYTHON_EXAMPLES=OFF \
    -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-master/modules \
    -DBUILD_PNG=ON \
    -DBUILD_TIFF=ON \
    -DBUILD_JPEG=OFF \
    -DBUILD_ZLIB=ON \
    -DWITH_OPENCL=ON \
    -DWITH_OPENMP=ON \
    -DWITH_FFMPEG=ON \
    -DWITH_GTK=ON \
    -DWITH_VTK=ON \
    -DWITH_TBB=ON \
    -DINSTALL_C_EXAMPLES=ON \
    -DINSTALL_TESTS=OFF \
	-D BUILD_DOCS=OFF \
	-D OPENCV_ENABLE_NONFREE=ON \
    -D BUILD_EXAMPLES=OFF ..

make
sudo make install
```

## License

Copyright (c) 2017-2018 Grupo de Investigación y Desarrollo en Mecatrónica (<mecatronica@usb.ve>).
Released under the [GNU GPLv3.0 License](LICENSE). 
