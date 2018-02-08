/********************************************************************/
/* Project: uwimageproc					            */
/* Module: 	template				            */
/* File: 	sample.cpp                                          */
/* Created:		30/01/2017                                  */
/* Edited:		20/01/2018, 07:12 PM                        */
/* Description:						                                
	Module template, including basic dependencies and CLI argument
parsing*/
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

#define ABOUT_STRING "ACLAHE C++ module v0.1"

// #define _VERBOSE_ON_
// C++ namespaces
using namespace cv;
//using namespace cv::cuda;
using namespace std;

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
    cvParser.about(ABOUT_STRING);	//adds "about" information to the parser method

	//if the number of arguments is lower than 3, or contains "help" keyword, then we show the help
   if (argc < 3 || cvParser.has("help")) {
        cout << "ACLAHE: a C++ implementation Automatic Contrast Limited Adaptive Histogram Equalization" << endl;
        cvParser.printMessage();

        cout << endl << "\tExample:" << endl;
        cout << "\t$ aclahe input.jpg output.jpg" << endl;
        cout <<
        "\tThis will apply ACLAHE to gray levels of 'input.jpg' image file, and save it into 'output.jg'" << endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);		//String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(1);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;						// output string that will contain the desired output file name

	// Check if occurred any error during parsing process
    if (! cvParser.check()) {
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
    if (InputFile.find_last_of(".") == - 1) // DOT (.) not found, so filename doesn't contain extension
        FileType = "";
    else
        FileType = InputFile.substr();

    // now we build the FileBase from input FileName
    string FileBase = FileName.substr(0, FileName.length() - FileType.length());

    //**************************************************************************
    cout << "Built with OpenCV " << CV_VERSION << endl;
    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;
    cout << "Output: " << OutputFile << endl;

    //**************************************************************************
	//Image reading
    //**************************************************************************
	//Convert image into grayscale
	//TODO: Check if could be used another color space, as HSV (and apply aclahe to V)
	//TODO: It seems like everybody applies Gaussian blur to image
    //**************************************************************************
	//Create base vector of BlockSize and ClipLimit values
    //**************************************************************************
	//Create container matrix with future results of entropy values for each CL/BS pair
    //**************************************************************************
	//Applies clahe with values of CL/BS from test vector
    //**************************************************************************
	//computes resulting entropy for each case
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
	return 0;
}																																																											

