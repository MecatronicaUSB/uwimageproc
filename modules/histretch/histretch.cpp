/********************************************************************/
/* Project: uwimageproc								                */
/* Module: 	histretch	- Histogram Stretching		                */
/* File: 	histretch.cpp                                           */
/* Created:		30/01/2018                                          */
/* Description:
    Histogram stretching, for simple color balance. C++ port from module
    prototype implemented in Python by Armando Longart
    TODO: add [ref] to original package
 */
/********************************************************************/

/********************************************************************/
/* Created by:                                                      */
/* Jose Cappelletto - cappelletto@usb.ve			                */
/* Collaborators:                                                   */
/* Armando Longart                      							*/
/********************************************************************/

#define ABOUT_STRING "Histogram Stretching tool with channel selection v0.4"

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

// #define _VERBOSE_ON_
// C++ namespaces
using namespace cv;
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
            "{@input |<none>  | Input image file}"    // input image is the first argument (positional)
                    "{@output |<none> | Output image file}" // output prefix is the second argument (positional)
                    "{c       |r      | Channel to apply histogram equalization}"
                    "{help h usage ?  |       | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about(ABOUT_STRING);	//adds "about" information to the parser method

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
        cout << endl << "\tExample:" << endl;
        cout << "\t$ histretch -c=HV input.jpg output.jpg" << endl;
        cout <<
        "\tThis will open 'input.jpg' file, operate on the 'H' and 'V' channels, and write it in 'output.jpg'" << endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);		//String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(1);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;						// output string that will contain the desired output file name

    // TODO: still unable to correctly parse the argument (in a non-positional way)
    String cChannel = cvParser.get<cv::String>("c");	// gets argument -c=x, where 'x' is the image channel
    // possible values of channel: r, g, b, h, s, v  TODO: add a full fledged implementation for all possible channels

	// Check if occurred any error during parsing process
    if (! cvParser.check()) {
        cvParser.printErrors();
        return -1;
    }

    //************************************************************************
    /* FILENAME */
    //gets the path of the input source
/*    string FileName = InputFile.substr(InputFile.find_last_of("/") + 1);
    string BasePath = InputFile.substr(0, InputFile.length() - FileName.length());

    //determines the input file extension
    string FileType;
    if (InputFile.find_last_of(".") == - 1) // DOT (.) not found, so filename doesn't contain extension
        FileType = "";
    else
        FileType = InputFile.substr();

    // now we build the FileBase from input FileName
    string FileBase = FileName.substr(0, FileName.length() - FileType.length());//*/

    //**************************************************************************
    cout << ABOUT_STRING << endl;
    cout << "Built with OpenCV " << CV_VERSION << endl;
    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;
    cout << "Output: " << OutputFile << endl;
    cout << "Channel: " << cChannel  << endl;

    //**************************************************************************

    Mat src, dst, srcBGR[3], dstBGR[3], dstHSV[3], dstHLS[3], dstLab[3], dstYCC[3];
    const char* src_window = "Source image";
    const char* dst_window = "Destination image";
    int curColSpace = COLOR_RGB2BGR; // default colour space: BGR

    src = imread (InputFile,CV_LOAD_IMAGE_COLOR);
    //split source image into Blue-Green-Red channels (OpenCV uses BGR order)
    //split(src, srcBGR);

    int num_convert = cChannel.length();

    namedWindow( src_window, WINDOW_AUTOSIZE);
    imshow (src_window, src);

    cout << "Applying " << num_convert << " histretch" << endl;

    // Now, according to parameters provided at CLI calling time, we must split and process the image
    for (int nc=0; nc<num_convert; nc++)
    {
        char c = cChannel[nc];
        cout << "\tChannel[" << nc << "]: " << c << endl;
        switch (c)
        {
            case 'B':   // stretch Blue  channel of RGB space
                // at this point the data is supposed to be already in BGR
                split (src, srcBGR);
                // then, we operate in the RED channel
                equalizeHist (srcBGR[0], srcBGR[0]);
                merge (srcBGR, 3, src);
                break;
            case 'G':   // stretch Green channel of RGB space
                // at this point the data is supposed to be already in BGR
                split (src, srcBGR);
                // then, we operate in the RED channel
                equalizeHist (srcBGR[1], srcBGR[1]);
                merge (srcBGR, 3, src);
                break;
            case 'R':   // stretch Red  channel of RGB space
                // at this point the data is supposed to be already in BGR
                split (src, srcBGR);
                // then, we operate in the RED channel
                equalizeHist (srcBGR[2], srcBGR[2]);
                merge (srcBGR, 3, src);
                break;

            case 'H':   // stretch Hue  channel of HSV space
                // convert to HSV
                cvtColor(src,dst,COLOR_BGR2HSV);
                // split channels
                split (dst, dstHSV);
                // then, we operate in the Hue channel
                equalizeHist (dstHSV[0], dstHSV[0]);
                // merge back into a single matrix
                merge (dstHSV, 3, dst);
                // and convert back to BGR
                cvtColor(dst,src,COLOR_HSV2BGR);
                break;
            case 'S':   // stretch Saturation channel of HSV space
                // convert to HSV
                cvtColor(src,dst,COLOR_BGR2HSV);
                // split channels
                split (dst, dstHSV);
                // then, we operate in the Sat channel
                equalizeHist (dstHSV[1], dstHSV[1]);
                // merge back into a single matrix
                merge (dstHSV, 3, dst);
                // and convert back to BGR
                cvtColor(dst,src,COLOR_HSV2BGR);
                break;
            case 'V':   // stretch Value channel of HSV space
                // convert to HSV
                cvtColor(src,dst,COLOR_BGR2HSV);
                // split channels
                split (dst, dstHSV);
                // then, we operate in the Val channel
                equalizeHist (dstHSV[2], dstHSV[2]);
                // merge back into a single matrix
                merge (dstHSV, 3, dst);
                // and convert back to BGR
                cvtColor(dst,src,COLOR_HSV2BGR);
                break;

            case 'h':
//                break;
            case 's':
//                break;
            case 'l':
                cout << "'hsl' not implemented yet..." << endl;
                break;

            case 'L':
//                break;
            case 'a':
//                break;
            case 'b':
                cout << "'L.a.b.' not implemented yet..." << endl;
                break;

            case 'Y':
//                break;
            case 'C':
//                break;
            case 'X':
                cout << "'YCrCb' not implemented yet..." << endl;
                break;

            default:
                cout << "Option " << c << " not recognized, skipping..." << endl;
                break;
        }
    }
/*
    // Conversion to HSV space
    cvtColor (src, dst, COLOR_BGR2HSV);
    split(dst, dstHSV);

    // Conversion to HLS space
    cvtColor (src, src, COLOR_BGR2HLS);
    split(dst, dstHLS);

    // Conversion to L.a.b space
    cvtColor (src, dst, COLOR_BGR2Lab);
    split(dst, dstLab);

    // Conversion to YCrCb space (JPEG)
    cvtColor (src, dst, COLOR_BGR2YCrCb);
    split(dst, dstYCC);//*/

    namedWindow( dst_window, WINDOW_AUTOSIZE);

/*
    imshow ("b", srcBGR[0]);
    imshow ("G", srcBGR[1]);
    imshow ("r", srcBGR[2]);

    imshow ("bo", dstBGR[0]);
    imshow ("Go", dstBGR[1]);
    imshow ("ro", dstBGR[2]);
*/

/*    merge (dstBGR,3,dst);*/
    imshow (dst_window, src);

    imwrite (OutputFile, src);

    waitKey(0);

	return 0;
}