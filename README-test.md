# Naming, filing and coding conventions #

## Naming rules for files and folders ##

To be followed in this order, i.e. 1) has highest priority  
1) When converting data (e.g. converting images from raw to RGB), mirror the folder structure. If the input data is in a subfolder of a folder called "raw" (or "inputdata"), use for the output the same path except that instead of "raw" (resp. "inpudata") use "processed".  
2) Avoid changing names of files and folders that already exist. This is to avoid that different people in the group use the same file but call it different names.  
3) Do not use spaces in foldernames and in filenames. Use lower case snake case: use only lower case and replace spaces by underscores, e.g. /this_is_my_folder/and_this_is_my_file.txt. If you put the date into a filename or folder name, use the YYYYMMDD order. If you indicate the time as well, use YYYYMMDD_hhmmss format.


## Filing system ##

- Data coming off the device or vehicle (SeaXerocks, AUV, ROV, ship, etc.) are saved in a folder called "raw" (or "inputdata").  
- Scripts and calibration files are in a folder called "scripts" (or "scriptetc"). This is a git-repository so the history of scripts and parameters is preserved.   
- Processed data including intermediate and final results are saved in a folder "processed". One exception is if during recording data was recorded wrongly (e.g. the clock of the PC was off by 1 hour and so all timestamps are off by exactly 1h). In this case the corrected version of the file shoud be saved in the "raw" (resp. "inputdata") folder with a telling name (e.g. append "_corrected" to the original name). Do not delete the original file. Camera calibration files go into the "raw" (resp. "inputdata") folder.


## Code stlye ##

- For C++ code follow the "Google C++ Style Guide" available at https://google.github.io/styleguide/cppguide.html, in particular for naming variables, functions and classes, unless a different style is already used in an existing file or project.
- For Python code follow the "PEP 8 -- Style Guide for Python Code" available at https://www.python.org/dev/peps/pep-0008, unless a different style is already used in an existing file or project. Compliance with the style guide can be tested using Pylint. In the Spyder IDE it can be run by choosing "Run static code analysis (F8)" from the "Source" menu. There are plugins for the Sublimetext editor. Checking the compliance with naming rules has to specified separately.
- Put units in variables names, e.g. `distance_m`. For products, split factors with underscore, e.g. `torque_n_m`. For quotients split dividend and divisor with `per`, e.g. `speed_cm_per_s`. Append exponents to the unit, e.g. `area_m2`.
- Follow the "Continuous Integration (CI)" development practice where possible.
- When committing to git, make 1 commit per (major) change, i.e. if you make many unrelated changes, commit them separately. In particular, unless where necessary, make separate commits for changes in files and changes of file names or folder names. Write meaningful git commit messages in imperative mode. Guide for writing commit messages: https://chris.beams.io/posts/git-commit/

# Installation of libraries and programs on Windows #
These are the instructions for Windows. See here for [Linux instructions](#linux-installation)


## Install Sublime Text (optional) ##
If you do not already have, I recommend to install a proper text editor, such as Sublime Text from http://www.sublimetext.com/


## Install Visual Studio 2013 ##

Currently (6 Sep. 2016) gisinternals.com does not provide any GDAL binaries for VS2015, therefore we are forced to stick with Visual Studio 2013 (also referred to by MSVC12) for now.

You can download Visual Studio 2013 Community from https://docs.microsoft.com/en-gb/visualstudio/releasenotes/vs2013-version-history for free. Make sure to get "Visual Studio 2013 Update 5". If you are at the IIS, you can also pull the installer from \\\\oplab-source\data\tools\Microsoft_Visual_Studio_2013  
After you launch the installer, choose to install "Microsoft Foundation Classes for C++". You don't need any of the other optional features.   
Download the "Multibyte MFC Library for Visual Studio 2013" from https://www.microsoft.com/en-us/download/details.aspx?id=40770 and install it. This is only required for raw2rgbConverterGUI.


## Install GIT (optional, but strongly recommended) ##

You don't need GIT to compile or run the program, however as a version control system it is very useful to pull updates from github and to submit changes if you modify the code.  
In order to install GIT, download it from https://git-scm.com/download/win and install it with the default configurations.  
On Windows 10 the installation progress bar might progress to 100% and then stay there calm for several minutes before the install actually completes.

Go to the control panel -> Folder Options -> View. In the Advanced settings, untick "Hide extensions for known file types". This step is not absolutely required, but if you don't change this setting, the ".gitignore" file appears as file without filename.


### Install CMake ###

Version shouldn't matter, but here I installed 3.6.1  
https://cmake.org/download/  
-> cmake-3.6.1-win64-x64.msi  
Instal with default options


## Install Boost ##

Go to https://sourceforge.net/projects/boost/files/boost-binaries/1.64.0/
and download boost_1_64_0-msvc-12.0-64.exe
Right-click the file and choose "Run as administrator".   
Unpack to  
C:/Program Files/boost_1_64_0  
Open the environmental variable PATH and add  
C:/Program Files/boost_1_64_0 and  
C:/Program Files/boost_1_64_0/lib64-msvc-12.0


If you want to test if everything worked up to here, pull generate_nav_files from github/ocean-perception using git (or download it as zip if you're not using git), run CMake to build the solution for Visual Studio 12 (2013) and verify if the project compiles in Visual Studio.


## Install OpenCV ##

Download https://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.13/opencv-2.4.13.exe/download
Extract to C:/OpenCV2.4.13  
Create environment variable OPENCV_DIR and save the as value C:/OpenCV2.4.13/opencv/build  
Open environmental variable PATH and add  
C:/OpenCV2.4.13/opencv/build/x64/vc12/bin and  
C:/OpenCV2.4.13/opencv/build/x86/vc12/bin


## Install CGAL (only used in 3D_mapping) ##

While it is possible to have several CGAL versions in parallel, I recommend to uninstall any older versions first, in case you have any installed.  
Go to https://github.com/CGAL/cgal/releases and download CGAL-4.9.1-Setup.exe, then open the installer  
Next  
I Agree  
Next  
64-bits. Next  
Change Destination folder to C:\Program Files\CGAL-4.9.1\.  Next  
All users. Install  
Next  
Finish  
If you get an error "The user environment variable PATH seems empty, and cannot be modified; set it manually to   
C:\Program Files\CGAL-4.9.1\auxiliary\gmp\lib", do so (append the path to the existing entries).  
 
Open CMake-GUI in administrator mode (Right click -> Run as administrator)  
For both "Where is the source code" and "Where to build the binaries", specify the CGAL Installation folder  (C:\Program Files\CGAL-4.9.1)  
Click on Configure  
Choose "Visual Studio 12 2013 Win64"  
Click on "Generate"  
Click on "Open Project"    
Build in Debug mode by clicking on Build -> Build solution  
Build in Release mode by changing the "Solution configurations" to "Release" and then select Build -> Build solution  
Add the following two directories to the environmental variable Path:  
C:\Program Files\CGAL-4.9.1\bin  
C:\Program Files\CGAL-4.9.1\lib  
Before your start building and linking your own projects against CGAL, restart the computer or alternatively close explorer and restart it. This is to make sure the changes in the environment variable PATH become active.  
To close the explorer in win7: ctrl+shift+right-click in an empty part of the start menu -> exit explorer, in win8 and win10: same as for win7 but right click in task bar  
To restart explorer: press ctrl+shift+esc and click on File -> New task (Run...) and type "explorer"

**Side note regarding QT:** CGAL's CMakeLists.txt is configured to look for QT5 and links against it if it finds it. There are 3 scenarios: 
1) Qt is not installed. Other than what the CGAL install guide might suggest, there is no need to install QT if you don't have it. If CMake doesn't find it, it sets Qt5_DIR to Qt5_DIR-NOTFOUND and configures and generates the project fine. Nothing more to do.
2) A compatible version of QT is installed. CMake finds it and builds it into the project, and Visual Studio links to it. Nothing more to do.
3) An older version of QT is installed, which CMake finds and associates to the project, but when trying to compile the project in Visual Studio, it fails during linking.  
In this case go back to CMake and click [Configure].  
Uncheck WITH_CGAL_Qt5 and click [Configure] again. 
Click [Generate] and proceed once again as written above.


## Install GDAL (only used in 3D_mapping) ##

(There's a GDAL install manual that instructs to first install Python, but that's apparently not necessary, unless you do want to use Python, so we don't do that here.)  
Go to  
http://www.gisinternals.com/release.php  
look in the upper section for MSVC 2013 x64 and click on the link in column Downloads. As of 8 March 2018 it is release-1800-x64-gdal-2-2-3-mapserver-7-0-7  
On the following page download   
- "Compiled binaries in a single .zip package" ("release-1800-x64-gdal-2-2-3-mapserver-7-0-7.zip" as of 8 March 2018)
- "Generic installer for the GDAL core components" ("gdal-202-1800-x64-core.msi" as of 8 March 2018)

Doubleclick the msi file and install with the [typical] settings.  
Unzip the zip file and copy its content to the GDAL folder  
Open environmental variable path and add C:\Program Files\GDAL


## Generate Visual Studio projects and compile ##

Clone the repositories, e.g. using the command line command `git clone --recursive https://github.com/ocean-perception/3d_mapping.git`  
Open CMake-GUI
In "Where is the source code" indicate the folder path of the project, e.g. D:/3D_mapping/git/3d_mapping.  
In "Where to build the binaries" indicate a subfolder "build" of the project, e.g. D:/3D_mapping/git/3d_mapping/build. The subfolder "build" doesn't need to exist, CMake-GUI will create it for you.  
Click "Configure". If it asks whether it should create the build directory, click "Yes".  
In the following dialog window, under "Specify the generator for this project" choose "Visual Studio 12 2013 Win64" and click "Finish".  
If the configuration process is successful, click "Generate".  
Go to the "build" folder that was created by CMake and open the sln file in Visual Studio 2013 (or directly click on the "Open Project" button in the CMake-GUI) and compile it in Release mode.

Alternatively you can use the script [clone_and_compile_programs.cmd](scripts/clone_and_compile_programs.cmd), which installs all programs at D:/3D_mapping/software/git_cpp/


# Linux installation
These instructions were tested on Ubuntu 16.04.3 LTS 64 bits for Desktop. However, they should work without major changes for other Debian based distros.

## System preparation
Start by getting the system up-to-date
```bash
sudo apt-get update
sudo apt-get upgrade
```

## Sublime installation
Instructions extracted from [Sublime website](http://www.sublimetext.com/docs/3/linux_repositories.html)

Install the GPG key:
```bash
wget -qO - https://download.sublimetext.com/sublimehq-pub.gpg | sudo apt-key add -
```
Ensure apt is set up to work with https sources:
```bash
sudo apt-get install apt-transport-https
```
Select the channel to use:
Stable
```bash
    echo "deb https://download.sublimetext.com/ apt/stable/" | sudo tee /etc/apt/sources.list.d/sublime-text.list
```
Update apt sources and install Sublime Text
```bash
	sudo apt-get update
	sudo apt-get install sublime-text
```

## boost 1.64.0 installation
boost 1.64.0 See [boost instructions](https://www.boost.org/doc/libs/1_64_0/more/getting_started/unix-variants.html)

Download source
```bash
wget https://netix.dl.sourceforge.net/project/boost/boost/1.64.0/boost_1_64_0.tar.gz .
```

Decompress
```bash
tar xvf boost_1_64_0.tar.gz
```

```bash
Enter source directory and configure. We are configuring boost system wide in /usr/local (default location) 
cd <path/to/boost>
sudo ./bootstrap.sh
```

Finally, perform the compilation/installation. This will take a while...
```bash
sudo ./b2 install
```

## OpenCV 2.4.13.6 installation. 
See: https://docs.opencv.org/2.4/doc/tutorials/introduction/linux_install/linux_install.html

Install dependencies and some optional tools
**Compiler**
```bash
sudo apt-get install build-essential
```
**Required**
```bash
sudo apt-get install cmake git libgtk2.0-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev
```

_Optional_
```bash
sudo apt-get install python-dev python-numpy libtbb2 libtbb-dev libjpeg-dev libpng-dev libtiff-dev libjasper-dev libdc1394-22-dev
```

Download source files and decompress it
```bash
wget https://github.com/opencv/opencv/archive/2.4.13.6.zip .
unzip opencv-2.4.13.6.zip
```
Create build directory and CMake
```bash
cd opencv-2.4.13.6
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..
```

Make and install. This while take a while to complete. You could try using `make -j2 ` or higher for multicore systems
```bash
make
sudo make install
```
## CGAL 4.9.1 installation 
CGAL requires several libreries such as:

### zlib
Download source
```bash
wget http://www.zlib.net/zlib-1.2.11.tar.gz .
tar xvf zlib-1.2.11.tar.gz
cd zlib-1.2.11/
./configure --prefix=/usr/local/zlib
make
sudo make install
```

### Installing QT 5. (optional)
CGAL may configure/compile without QT5, so install QT only if necessary
```bash
sudo apt-get install qt5-default
```

### GDAL 
In case you need GDAL, proceed to install 2.2.3 from source. For further instructions, please visit: https://trac.osgeo.org/gdal/wiki/BuildingOnUnix
```bash
wget https://download.osgeo.org/gdal/2.2.3/gdal-2.2.3.tar.gz 
tar xvf gdal-2.2.3.tar.gz 
cd gdal-2.2.3/
./configure
```
**Warning**: This may take a lot of time
```bash
sudo make install
```

### MPFR
For MPFR (another recommended library for CGAL). Install it just when the static library compilation for CGAL failed.
```bash
sudo apt-get install libmpfr-dev libmpfr-doc libmpfr4 libmpfr4-dbg
```

### Installation
Always refer to [CGAL 4.9.1 installation instructions](https://github.com/CGAL/cgal/releases/tag/releases%2FCGAL-4.9.1)
Be carefull with newer versions, as there are important changes during configuration/make process. Useful instructions are available at [CGAL documentation])(https://doc.cgal.org/latest/Manual/installation.html).
There is also an option to install from supported OS repositories via: `sudo apt-get install libcgal-dev`. For source based installation:

Download and unzip source
```bash
wget https://github.com/CGAL/cgal/archive/releases/CGAL-4.9.1.tar.gz .
tar xvf CGAL-4.9.1.tar.gz
cd cgal-releases-CGAL-4.9.1/
```
% Now, configure to headers-only mode (which is supported since CGAL 4.9 and CMake 3.0). This is faster as it doesn't require code compilation.
```bash
cmake -DCGAL_HEADER_ONLY=ON .
```
Make & install
```bash
make
sudo make install
```

### 3D Mapping compilation
Once installed all the dependencies, proceed to the *3d_mapping* folder, and create a *build* subfolder
```bash
cd <path/to/working/folder>
git clone --recursive https://github.com/ocean-perception/3d_mapping.git
cd 3d_mapping
mkdir build 
cd build
```
Then, use CMake for building project configuration. Check all the flags and options in the CMake report.
```bash
cmake ..
```
Finally, *make* it
```bash
make
```
Don't forget to perform a quick test by running `./3d_mapping`.


# Data processing #

The mapping pipeline consists of several programs. Each program has a help function that can be displayed by executing the program with the command line parameter `-h` or `--help`.  
The scripts assume that the input data that is on oplab-surf at \\oplab-surf\data\reconstruction\raw\2015\NT15-13 is copied to D:/inputdata/cruises/NT15-13, the outputdata that is on oplab-surf at \\oplab-surf\data\reconstruction\processed\2015\NT15-13 is at D:/processed/cruises/NT15-13 and the scripts and calibration files in this repository are at D:/scriptetc/cruises/NT15-13.  
The script [run_all.cmd](scripts/run_all.cmd) runs all the below scripts, apart from the camera calibrtion.  
While it is possible to launch a script by double-clicking the cmd file, this is not recommended, as the command prompt closes automatically after the script has finished, so the outputs cannot be seen anymore. Instead it is recommeded to open the command prompt and call the script from there.

## Camera calibration ##

The lm165 camera images are saved as tiffs which can be directly opened and viewed. The xviii camera images however are saved in a raw format and first need to be converted to RGB. To find suitable colour balance parameters, use raw2rgbConverterGUI (Windows only. Doubleclick exe file to launch GUI) available at https://github.com/ocean-perception/raw2rgbConverterGUI on one of the raw images. To convert all the camera calibration images use `image_conversion` available at https://github.com/ocean-perception/image_conversion as shown in script [demosaic_calibration_images.cmd](scripts/demosaic_calibration_images.cmd)

For each camera, pick 20 to 100 good calibration images and copy them to a new folder. "Good" images are clear (in focus and no motion blur) and all dots (if a dot-pattern is used) or rectangles (if a checker-board is used) of the calibration pattern are inside the picture and evenly illuminated. A good set of images covers the entire camera's field of view evenly, in particular it also covers all 4 corners. Especially when using a dot-pattern calibration board, make sure to avoid images taken at too close range where the dots are too large.  
When you run the calibration code, it shows in the end a "summary" of where the dots (resp. chessboard corners) were identified. If there is an area that is not sufficiently covered, add images showing the calibration image in that area.

E.g.  
`   D:/inputdata/cruises/NT15-13/calibration/after_cruise/20150817_144801/LM165/000`  
`-> D:/processed/cruises/NT15-13/calibration/after_cruise/20150817_144801/LM165/000_selection`  
`   D:/processed/cruises/NT15-13/calibration/after_cruise/20150817_144801/Xviii/Cam303235080/`  
`-> D:/processed/cruises/NT15-13/calibration/after_cruise/20150817_144801/Xviii/Cam303235080_selection`  
`   D:/processed/cruises/NT15-13/calibration/after_cruise/20150817_144801/Xviii/Cam303235081/`  
`-> D:/processed/cruises/NT15-13/calibration/after_cruise/20150817_144801/Xviii/Cam303235081_selection`  

Run `calibrate_camera` available at https://github.com/ocean-perception/calibrate_camera to compute the camera parameters, as shown in script [compute_camera_parameters.cmd](scripts/compute_camera_parameters.cmd)

Note: the dot-pattern calibration function in OpenCV expects black dots on a white background, but the calibration board we used is white dots on a black background. For this reason, our calibration program inverts all the images by default. In case a board with black dots on a white background is used, set `--f_white_on_black` to `false` to disable inverting the images.

To check that it did the right thing, the calibration images can be undistorted using `image_conversion` as shown in script [undistort_lm165_calibration_images.cmd](scripts/undistort_lm165_calibration_images.cmd).


## Generate pose files ##

Generate pose files containing the position and orientation for each time step using `auv_nav`, available at https://github.com/ocean-perception/auv_nav. 
Previously `generate_nav_files` available at https://github.com/ocean-perception/generate_nav_files was used for this as shown in script [compute_pose_estimates.cmd](scripts/compute_pose_estimates.cmd). 
The sample script uses Hyper-Dolphin PHINS data. If you use a different data source (e.g. DVL recorded by SeaXerocks or ACFR nav data), use the corresponding exe-file in generate_nav_files.


## Generate bathymetry ##

Generate the bathymetry map using `laser_bathymetry` available at https://github.com/ocean-perception/laser_bathymetry as shown in script [laser_bathymetry.cmd](scripts/laser_bathymetry.cmd) with ini-file [laser_bathymetry.ini](scripts/laser_bathymetry.ini).  
(Note: camara LM165 was mounted upside down during cruisee NT15-13. This just corresponds to a roll angle of 180°)

If there are outliers, remove them using `./3d_colour_mapping REMOVE_OUTLIERS`. `3d_colour_mapping` is available at https://github.com/ocean-perception/3d_mapping and an example of how to use it is shown in [remove_outliers.cmd](scripts/remove_outliers.cmd).  
It is also possible to remove outliers by hand in Meshlab. To do so, download Meshlab from the network drive (Tokyo: \\oplab-source\data\tools\link_meshlab\distrib_20150515.exe ; Southampton: \\oplab-surf\data\tools\link_meshlab\distrib_20150515.exe) . To save the cleaned mesh, go to File -> Export Mesh... and untick [Binary encoding] before you click [OK]. Do not use the original version of Meshlab that can be downloaded from their website, as it reduces the precision when saving as ascii (non-binary) ply file. For viewing it is fine, though.

Optional: Smooth bathymetry to smoothen out small spikes using ./3d_colour_mapping SMOOTH. An example of how to use it is shown in [smooth_bathymetry.cmd](scripts/smooth_bathymetry.cmd) 


## Generate texture mapped 3D reconstruction ##

Convert xviii raw images to RGB and correct for the distortion at the same time using `image_conversion` available at https://github.com/ocean-perception/image_conversion as shown in script [demosaic_and_undistort.cmd](scripts/demosaic_and_undistort.cmd)  

In case the pose files for the xviii images aren't generated yet, generate them as described in section [Generate pose files](#generate-pose-files).

Generate the texture mapped 3D reconstruction using ./3d_colour_mapping textureMapAndSimplify. There are two examples of how to use it; one that only uses command line arguments ( [texturemap_from_cmd.cmd](scripts/texturemap_from_cmd.cmd) ) and one that uses a ini-file that is passed to the program on the command line ( scipt: [texturemap_using_ini.cmd](scripts/texturemap_using_ini.cmd), ini-file: [texturemap.ini](scripts/texturemap.ini) )
Note: This requires a lot of RAM. If the computer runs out of RAM the program will crash. If this happens, set gridSpacing to a higher value.