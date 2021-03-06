/********************************************************************/
/* Project: uwimageproc								                */
/* Module: 	histretch	- Histogram Stretching		                */
/* File: 	histretch.cpp                                           */
/* Created:		30/01/2018                                          */
/* Description:
    Histogram stretching, for simple color balance. C++ port from module
    prototype implemented in Python by Armando Longart @ajlongart
    See [https://github.com/ajlongart/Tesis-UIP]
 */
/********************************************************************/

/********************************************************************/
/* Created by:                                                      */
/* Jose Cappelletto - cappelletto@usb.ve			                */
/* Collaborators:                                                   */
/* Armando Longart                      							*/
/* Fabio Morales                                                    */
/* Victor Garcia                                                    */
/********************************************************************/

#define ABOUT_STRING "Histogram Stretching tool with channel selection v0.4"

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

/// Include auxiliary utility libraries
// TODO: change directory structure to math proposed template  (see mosaic repo)
#include "../../common/preprocessing.h"

// C++ namespaces
using namespace cv;
using namespace cv::cuda;
using namespace std;

// Time measurements
#define _VERBOSE_ON_
double t ;	// Timing monitor

/// CUDA specific libraries
#if USE_GPU
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudafeatures2d.hpp"
#include "opencv2/xfeatures2d/cuda.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudaarithm.hpp"
#endif

/*!
	@fn		int main(int argc, char* argv[])
	@brief	Main function
*/
int main(int argc, char *argv[]) {

//*********************************************************************************
/*	PARSER section */
/*  Uses built-in OpenCV parsing method cv::CommandLineParser. It requires a string containing the arguments to be parsed from
	the command line. Further details can be obtained from opencv webpage
*/
    String keys =
            "{@input |<none>  | Input image file}"    // input image is the first argument (positional)
                    "{@output |<none> | Output image file}" // output prefix is the second argument (positional)
                    "{c       |r      | Channel to apply histogram equalization}"
                        "{cuda    |       | Use CUDA or not (CUDA ON: 1, CUDA OFF: 0)}"         // Use CUDA (if available) or not
                            "{time    |       | Show time measurements or not (ON: 1, OFF: 0)}" // Show time measurements or not
                    "{help h usage ?  |       | show this help message}";         // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about(ABOUT_STRING);	//adds "about" information to the parser method

    //**************************************************************************
    cout << ABOUT_STRING << endl;
    cout << "Built with OpenCV " << CV_VERSION << endl;

	//if the number of arguments is lower than 3, or contains "help" keyword, then we show the help
	if (argc < 3 || cvParser.has("help")) {
        cout << "C++ implementation of Histogram Stretching for specific channels of input image" << endl;
        cout << "Based on A. Longart Python prototype and OpenCV online documentation" << endl;
        cvParser.printMessage();
        cout << "Argument 'c=<channels>' is a string containing an ordered list of desired channels to be stretched" << endl;
        cout << "Histogram stretching is applied one at time, and then converted back to RGB colour space" << endl;
        cout << "Complete options are:" << endl;
        cout << "\t-c=R|G|B\tfor RGB space" << endl;
        cout << "\t-c=H|S|V\tfor HSV space" << endl;
        cout << "\t-c=h|s|l\tfor HSL space" << endl;
        cout << "\t-c=L|a|b\tfor Lab space" << endl;
        cout << "\t-c=Y|C|X\tfor YCrCb space" << endl;
        cout << "\t-cuda=0 or -cuda=1 (CUDA ON: 1, CUDA OFF: 0, if available)" << endl;
        cout << endl << "\tExample:" << endl;
        cout << "\t$ histretch -c=HV input.jpg output.jpg -cuda=0 -time=1" << endl;
        cout <<
        "\tThis will open 'input.jpg' file, operate on the 'H' and 'V' channels, and write it in 'output.jpg'" << endl << endl;
        return 0;
    }
    int CUDA = 0;                                       //Default option (running with CPU)
    int Time = 0;                                       //Default option (not showing time)
    String InputFile = cvParser.get<cv::String>(0);		//String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(1);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;						// output string that will contain the desired output file name
    String implementation;

    String cChannel = cvParser.get<cv::String>("c");	// gets argument -c=x, where 'x' is the image channel
    Time = cvParser.get<int>("time");	                // gets argument -time=x, where 'x' define to show time execution or not
	// Check if occurred any error during parsing process
    if (! cvParser.check()) {
        cvParser.printErrors();
        return -1;
    }

    //**************************************************************************
    int nCuda = - 1;    // Defines number of detected CUDA devices. By default, -1 acting as error value
    #if USE_GPU
        CUDA = cvParser.get<int>("cuda");	        // gets argument -cuda=x, where 'x' define to use CUDA or not
        nCuda = cuda::getCudaEnabledDeviceCount();	// Try to detect any existing CUDA device
        // Deactivate CUDA from parse
        if (CUDA == 0){            
            implementation = "CPU";
            cout << "CUDA deactivated" << endl;
            cout << "Exiting... use non-GPU version instead" << endl;
        }
        // Find CUDA devices
        else if (nCuda > 0){
            implementation = "GPU";     
            cuda::DeviceInfo deviceInfo;
            cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
            cuda::setDevice(0);
        }
        else {
            CUDA = 0;
            implementation = "CPU";
            cout << "No CUDA device detected" << endl;
            cout << "Exiting... use non-GPU version instead" << endl;
        }
    #endif

    //**************************************************************************
    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;
    cout << "Output: " << OutputFile << endl;
    cout << "Channel: " << cChannel  << endl;

    Mat src, dst;
    const char* src_window = "Source image";
    const char* dst_window = "Destination image";
    int num_convert = cChannel.length();
    int min_percent = 2, max_percent = 98;
    int transformation[4][2] = {COLOR_BGR2HSV, COLOR_HSV2BGR, COLOR_BGR2HLS, COLOR_HLS2BGR, 
                                COLOR_BGR2Lab, COLOR_Lab2BGR, COLOR_BGR2YCrCb, COLOR_YCrCb2BGR};

    src = imread (InputFile,CV_LOAD_IMAGE_COLOR);
    namedWindow( src_window, WINDOW_AUTOSIZE);
    imshow (src_window, src);

    cout << "Applying " << num_convert << " histretch" << endl;

    // Start time measurement
    t = (double) getTickCount();

    // GPU Implementation
    #if USE_GPU
        if(CUDA){
            GpuMat srcGPU, dstGPU;
            GpuMat dstSpacesGPU[6][3];
            Mat dstSpaces[6][3];

            srcGPU.upload(src);
            dstGPU.upload(dst);

            // Now, according to parameters provided at CLI calling time, we must split and process the image
            for (int nc=0; nc<num_convert; nc++){
                char c = cChannel[nc];
                int channel = numChannel(c);
                int space = numSpace(c);
                cout << "\tChannel[" << nc << "]: " << c << endl;

                // If the option is recognized
                if(!(space == -1)){
                    // Spaces HSV, hsl, Lab, YCX
                    if(space == 1 || space == 2 || space == 3 || space == 4 ){
                        // Convert space
                        cuda::cvtColor(srcGPU, dstGPU, transformation[space - 1][0], 3);
                        // Split channels
                        cuda::split(dstGPU, dstSpacesGPU[space]);
                        // Stretch of selected channel
                        imgChannelStretchGPU(dstSpacesGPU[space][channel], dstSpacesGPU[space][channel], min_percent, max_percent);
                        // Convert back to BGR space
                        cuda::cvtColor(dstGPU, srcGPU, transformation[space - 1][1], 3);
                        // Merge channels to dst
                        cuda::merge (dstSpacesGPU[space], 3, dstGPU);
                    }
                    // Space BGR
                    else{
                        // Split channels
                        cuda::split (srcGPU, dstSpacesGPU[space]);
                        // Stretch of selected channel
                        imgChannelStretchGPU(dstSpacesGPU[space][channel], dstSpacesGPU[space][channel], min_percent, max_percent);
                        // Merge channels to src
                        cuda::merge (dstSpacesGPU[space], 3, srcGPU);
                    }
                // If the option is not recognized
                }else cout << "Option " << c << " not recognized, skipping..." << endl;
            }

            srcGPU.download(src);
            dstGPU.download(dst);
            
        }
    #endif
    if(not CUDA){
        // CPU Implementation
        Mat dstSpaces[6][3];
        // Now, according to parameters provided at CLI calling time, we must split and process the image
        for (int nc=0; nc<num_convert; nc++){
            char c = cChannel[nc];
            int channel = numChannel(c);
            int space = numSpace(c);
            cout << "\tChannel[" << nc << "]: " << c << endl;

            // If the option is recognized
            if(!(space == -1)){
                // Spaces HSV, hsl, Lab, YCX
                if(space == 1 || space == 2 || space == 3 || space == 4 ){
                    // Convert space
                    cv::cvtColor(src,dst,transformation[space - 1][0]);
                    // Split channels
                    split (dst, dstSpaces[space]);
                    // Stretch of selected channel
                    imgChannelStretch(dstSpaces[space][channel], dstSpaces[space][channel], min_percent, max_percent);
                    // Convert back to BGR space
                    cv::cvtColor(dst,src,transformation[space - 1][1]);
                    // Merge channels to dst
                    merge (dstSpaces[space], 3, dst);
                }
                // Space BGR
                else{
                    // Split channels
                    split (src, dstSpaces[space]);
                    // Stretch of selected channel
                    imgChannelStretch(dstSpaces[space][channel], dstSpaces[space][channel], min_percent, max_percent);
                    // Merge channels to src
                    merge (dstSpaces[space], 3, src);
                }
            // If the option is not recognized
            }else cout << "Option " << c << " not recognized, skipping..." << endl;
        }
    }

    //  End time measurement (Showing time results is optional)
    if( Time == 1 ){
        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        cout << endl << "Execution Time "<< implementation << " :" << t << " ms " << endl;
        t = (double) getTickCount();
    }

    namedWindow( dst_window, WINDOW_AUTOSIZE);
    cout << "hS: showing resulting window" << endl;
    imshow (dst_window, src);
    cout << "hS: saving to disk" << endl;
    // Saving output file in specified directory
    imwrite (OutputFile, src);

    waitKey(0);
	return 0;
}
