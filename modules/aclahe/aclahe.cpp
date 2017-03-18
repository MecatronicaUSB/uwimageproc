/********************************************************************/
/* File: aclahe.cpp                                                 */
/* Last Edition: 30/01/2017, 07:12 PM.                              */
/********************************************************************/
/* Programmed by:                                                   */
/* Jose Cappelletto                                                 */
/********************************************************************/
/* Project: imageproc								                */
/* File: 	aclahe.cpp							                */
/********************************************************************/

//Basic C and c++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdlib.h>


// OpenCV libraries
#include <opencv2/core.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>
#include "opencv2/xfeatures2d/cuda.hpp"

//CUDA libraries
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudafeatures2d.hpp"

//#define _VERBOSE_ON_

using namespace cv;
using namespace cv::cuda;
using namespace cv::xfeatures2d;
using namespace std;

char keyboard = 0;

// Timing monitor
double t;

//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Compute Entropy for different Clip Limit and Block Size
//**** 4- Curve fitting to extract curvature
//**** 5- Select breakpoint for Entropy vs CL
//**** 6- Applies CL/BS to image

float calcEntropy(Mat image);

int imgWidth, imgHeight;

/*!
	@fn		int main(int argc, char* argv[])
	@brief	Main function
*/
int main(int argc, char *argv[]) {
//*********************************************************************************
/* PARSER */
    String keys =
            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
                    "{@output |<none> | Prefix for output .jpg images}" // output prefix is the second argument (positional)
                    "{display  |      | show the input/output images}"      // optional, show images
                    "{help h usage ?  |      | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about("aclahe module v0.1");

    if (argc < 3 || cvParser.has("help")) {
        cout << "Tool to automatically compute Clip Limit and Block Size for CLAHE" << endl;
        cout << "Identifies maximum curvature point for adjusted Entropy vs CL/BS" <<  endl;
        cvParser.printMessage();
        cout << endl << "\tExample:" << endl;
        cout << "\t$ aclahe input.jpg output.jpg " << endl;
        cout <<
        "\tThis will open 'input.jpg' extract best CL/BS values, apply them, and store into 'output.jpg'" <<
        endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);
    String OutputFile = cvParser.get<cv::String>(1);
    ostringstream OutputFileName;

    if (! cvParser.check()) {
        cvParser.printErrors();
        return - 1;
    }

    //************************************************************************
    /* FILENAME */
    //gets the path of the input source
    string FileName = InputFile.substr(InputFile.find_last_of("/") + 1);
    string BasePath = InputFile.substr(0, InputFile.length() - FileName.length());

    //determines the filetype
    string FileType;
    if (InputFile.find_last_of(".") == - 1) // DOT (.) not found, so filename doesn't contain extension
        FileType = "";
    else
        FileType = InputFile.substr();

    // now we build the FileBase from input FileName
    string FileBase = FileName.substr(0, FileName.length() - FileType.length());

    //**************************************************************************
    /* CUDA */
    int nCuda = - 1;    //<Defines number of detected CUDA devices

    nCuda = cuda::getCudaEnabledDeviceCount();
    cout << "Built with OpenCV " << CV_VERSION << endl;
    cuda::DeviceInfo deviceInfo;

    if (nCuda > 0)
        cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
    else {
        cout << "No CUDA device detected" << endl;
        cout << "Exiting... use non-GPU version instead" << endl;
    }

    cuda::setDevice(0);
    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;

    //**************************************************************************
    /* GAUSSIAN FILTER */
    //**************************************************************************
    /* CREATE CL AND BS VECTOR */
    //**************************************************************************
    /* COMPUTE ENTROPY FOR EACH CASE */
    //**************************************************************************
    /* CURVE FITTING */
    //**************************************************************************
    /* BREAKPOINT DETECTION */
    //**************************************************************************
    /* FINAL CLAHE APPLIED WITH SELECTED CL/BS */
	//*****************************************************************************
    return 0;
}

/*! @fn float calcBlur (Mat frame)
    @brief Calculates the "blur" of given \a Mat frame, based on the standard deviation of the Laplacian of the input fram
    @param frame OpenCV matrix container of the input frame
	@retval The estimated blur for the given frame
*/
float calcEntropy(Mat image) {
    // Avg time: 0.7 ms GPU/ 23ms CPU
    Mat grey, laplacian;
    cuda::GpuMat gpuFrame, gpuLaplacian;
    cvtColor(image, grey, COLOR_BGR2GRAY);

    //upload into GPU memory
    gpuFrame.upload(grey);
    //perform Laplacian filter
    Ptr<cuda::Filter> filter = cuda::createLaplacianFilter(gpuFrame.type(), gpuLaplacian.type(), 1, 1);
    filter->apply(gpuFrame, gpuLaplacian);    //**/
    //download from GPU memory
    gpuLaplacian.download(laplacian);

    //we compute the laplacian of the current frame
    Scalar mean, stdev;
    // the we compute the mean and stdev of that frame
    meanStdDev(laplacian, mean, stdev);
    double m = mean.val[0];
    double s = stdev.val[0];
    // we return the standar deviation
    return s;
}


