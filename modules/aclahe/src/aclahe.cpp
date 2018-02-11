/********************************************************************/
/* Project: uwimageproc					            */
/* Module: 	aclahe				            */
/* File: 	aclahe.cpp                                          */
/* Created:		06/02/2018                                  */
/* Description:
    C++ module of Automatic Contrast Limited Adaptive Histogram Equalization
 */
/********************************************************************/

/********************************************************************/
/* Created by:                                                      */
/* Jose Cappelletto (@cappelletto)                                  */
/* Based on:                                                        */
/* Original Python implementation by Carlos Sanoja (@CarSanoja)	    */
/********************************************************************/

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdlib.h>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

//#cmakedefine FOUND_CUDA

/// CUDA specific libraries
#ifdef FOUND_CUDA
    #include <opencv2/cudafilters.hpp>
    #include "opencv2/cudafeatures2d.hpp"
    #include "opencv2/xfeatures2d/cuda.hpp"
    #include "opencv2/cudaimgproc.hpp"
    #include "opencv2/cudaarithm.hpp"
#endif

/// Include auxiliary utility libraries
// TODO: change directory structure to math proposed template  (see mosaic repo)
#include "../../common/preprocessing.h"

#define ABOUT_STRING "ACLAHE C++ module v0.2"

// #define _VERBOSE_ON_
// C++ namespaces
using namespace cv;
//using namespace cv::cuda;
using namespace std;


/*!
	@fn		aclaheEntropy(cv::Mat img)
	@brief	Computes the entropy of a single channel image, usually the luminance channel
*/
float aclaheEntropy(cv::Mat img);

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
            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
                    "{@output |<none> | Prefix for output file}" // output prefix is the second argument (positional)
                    "{help h usage ?  |       | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about(ABOUT_STRING);    //adds "about" information to the parser method
    //**************************************************************************
    cout << ABOUT_STRING << endl;
    cout << "Built with OpenCV " << CV_VERSION << endl;

    //if the number of arguments is lower than 3, or contains "help" keyword, then we show the help
    if (argc < 3 || cvParser.has("help")) {
        cout << "ACLAHE: a C++ implementation Automatic Contrast Limited Adaptive Histogram Equalization" << endl;
        cvParser.printMessage();

        cout << endl << "\tExample:" << endl;
        cout << "\t$ aclahe input.jpg output.jpg" << endl;
        cout <<
             "\tThis will apply ACLAHE to gray levels of 'input.jpg' image file, and save it into 'output.jg'" << endl
             << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(
            0);        //String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(
            1);    //String containing the output file template from cvParser function
    ostringstream OutputFileName;                        // output string that will contain the desired output file name

    // Check if occurred any error during parsing process
    if (!cvParser.check()) {
        cvParser.printErrors();
        return -1;
    }

    //************************************************************************
    /* FILENAME */
    //gets the path of the input source
    string FileName = InputFile.substr(InputFile.find_last_of("/") + 1);
    string BasePath = InputFile.substr(0, InputFile.length() - FileName.length());

    //determines the input file extension
    string FileType;
    if (InputFile.find_last_of(".") == -1) // DOT (.) not found, so filename doesn't contain extension
        FileType = "";
    else
        FileType = InputFile.substr();

    // now we build the FileBase from input FileName
    string FileBase = FileName.substr(0, FileName.length() - FileType.length());

    //**************************************************************************
    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;
    cout << "Output: " << OutputFile << endl;

    //**************************************************************************
    //Image reading

    Mat src, dst, channels[3];
    const char *src_window = "Source image";
    const char *dst_window = "Destination image";

    src = imread(InputFile, CV_LOAD_IMAGE_COLOR);
/*    if (src == NULL){
        cout << "Failed to read input image, exiting..." << endl;
        return -1;
    }*/

    cout << "Input image loaded..." << endl;
    namedWindow(src_window, WINDOW_AUTOSIZE);
    namedWindow(dst_window, WINDOW_AUTOSIZE);
    imshow(src_window, src);

    //**************************************************************************
    //Convert image into grayscale
    //TODO: Check if could be used another color space, as HSV (and apply aclahe to V)
    //TODO: It seems like everybody applies Gaussian blur to image
    // Conerting image to HSV space
    cvtColor(src, dst, CV_BGR2HSV, 3);
    // Split image into 3 different channels. Split[2] will be V channel
    split(dst, channels);

    imshow(dst_window, channels[1]);

    //**************************************************************************
    //Create base vector of BlockSize and ClipLimit values
    int BlockSize[5] = {2, 4, 8, 16, 32};
    // ContrastLimit must go from minContrastLimit to maxContrastLimit
    float minContrastLimit = 0.0;
    float maxContrastLimit = 25.0;
    float stepContrastLimit = 0.5;

    //**************************************************************************
    //Create container matrix with future results of entropy values for each CL/BS pair
    vector<vector<double> > entropyResults;
    vector<double> row;

    //**************************************************************************
    //Applies clahe with values of CL/BS from test vector
    //and computes resulting entropy for each case

    cv::Ptr<cv::CLAHE> clahe;
    clahe = cv::createCLAHE();

    float entropy;

    for (int i = 0; i < 5; i++) {
        for (float cl = minContrastLimit; cl <= maxContrastLimit; cl += stepContrastLimit) {
            // set up ClipLimit (ContrastLimit) and TileSize(BlockSize)
            cv::Size bs(BlockSize[i],BlockSize[i]);
            clahe->setClipLimit(cl);
            clahe->setTilesGridSize(bs);
            // apply clahe to the V channel of HSV space
            clahe->apply(channels[2], dst);
            // compute the entropy (Shannon Index) for the resulte dst image after CLAHE
            entropy = aclaheEntropy(dst);
            row.push_back(entropy);
        }
        entropyResults.push_back(row);
    }

    // Image Quality depends on CL rather than BS. So, first they compute the entropy curve with fixed BS=8x8
    // while varying CL along all its range
    // CAUTION: OpenCV CL range differs to Matlab implementation

    for (int i=0; i<5; i++){
        row = entropyResults[i];
        for (int j=0; j<row.size(); j++){
            double s = row[j];
            cout << s << " ";
        }
        cout << endl;
    }//*/


    //**************************************************************************
    //find the CL value with highest curvature
    //**************************************************************************
    //for the resulting CL*, find the BS with highest curvature on the entropy
    //**************************************************************************
    //for resulting CL/BS, apply classic clahe
    //**************************************************************************
    //transform back image
    //**************************************************************************
    //saves resulting image
    //**************************************************************************
    waitKey(0);
    return 0;
}

/*!
	@fn		aclaheEntropy(cv::Mat img)
	@brief	Computes the entropy of a single channel image, usually the luminance channel
*/
float aclaheEntropy(cv::Mat img){
    float entropy = 0;
    cv::Mat hist;      // 256-bin histogram (pixel count)
    float histN[256];   // normalized histogram

    int width, height;
    width = img.size().width;
    height = img.size().height;

    // Finds the image histogram
    getHistogram(&img, &hist);
    // Normalize the histogram, by the total number of pixels, and also convert it into log2
    // Computes the entropy as the Shannon Index
    for (int i=0; i< 256; i++){
        histN[i] = hist.at<float>(i,0) / (width * height);
        // we add 0.0000001 just in case there is a NULL bin in the histogram
        entropy = entropy + ( histN[i] * log2(histN[i] + 0.00001));
        // real entropy value is negative of this result
    }
    return (-entropy);
}
