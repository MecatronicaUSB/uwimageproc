/********************************************************************/
/* Project: uwimageproc								                */
/* Module: 	template								                */
/* File: 	sample.cpp                                              */
/* Created:		30/01/2017                                          */
/* Edited:		20/01/2018, 07:12 PM                                */
/* Description:						                                
	Module template, including basic dependencies and CLI argument
parsing*/
/********************************************************************/

/********************************************************************/
/* Created by:                                                      */
/* Jose Cappelletto - cappelletto@usb.ve			                */
/* Collaborators:                                                   */
/* Victor Garcia - victorygarciac@gmail.com							*/
/********************************************************************/

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdlib.h>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>

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
                    "{p      |0.95  | A float number (0.0 to 1.0) with a default value}"
                    "{k      |      | An intenger number}"
                    "{help h usage ?  |       | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about("sample module v0.2");	//adds "about" information to the parser method

	//if the number of arguments is lower than 3, or contains "help" keyword, then we show the help
	if (argc < 3 || cvParser.has("help")) {
        cout << "This is a sample module to be employed as template for functional modules" << endl;
        cout << "Receives some mandatory and other optional arguments from the command line" << endl;
        cvParser.printMessage();
        cout << endl << "\tExample:" << endl;
        cout << "\t$ template -p=0.6 -k=5 input.avi vdout_" << endl;
        cout <<
        "\tThis will open 'input.avi' file, asign p=0.6 and k=5, and set output string to 'vdout_" << endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);		//String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(1);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;						// output string that will contain the desired output file name

    int kWindow = cvParser.get<int>("k");		// gets argument -k=WW, where WW is the size of the search window for the best frame 
    float overlap = cvParser.get<float>("p");	// gets argument -p=OO, where OO is the desired overlap among frames

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
    cout << "k: " << kWindow  << endl;
    cout << "p: " << overlap << endl;

    //**************************************************************************
	return 0;
}																																																											

