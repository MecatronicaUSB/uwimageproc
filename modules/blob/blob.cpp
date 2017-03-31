/**
 * OpenCV SimpleBlobDetector Example
 *
 * Copyright 2015 by Satya Mallick <spmallick@gmail.com>
 *
 */

//Basic C and c++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>

// OpenCV libraries
#include <opencv2/core.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
//*********************************************************************************
/* PARSER */
    String keys =
            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
            "{help h usage ?  |      | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, (const char *const *) argv, keys);
    cvParser.about("Blob detection module v0.1");

    if (argc < 2 || cvParser.has("help")) {
        cout << "Tool to detect blobs on medium-high contrast" << endl;
        cout << "with enough flexibility to be applied to multiple capture scenarios" <<  endl;
        cout << "It should generate an output file containint information about detected blobs (position, area and size)" <<  endl;
        cvParser.printMessage();
        cout << endl << "\tExample:" << endl;
        cout << "\t$ blob input.jpg" << endl;
        cout <<
        "\tThis will open 'input.jpg' and apply default detection parameters (so far) to extracct blobs" <<
        endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);
//    String OutputFile = cvParser.get<cv::String>(1);
//    ostringstream OutputFileName;

    ofstream outfile("out.txt", ios::out);
//    ofstream histfile("hist.txt", ios::out);


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

    //************************************************************************
    /* IMAGE LOADING */
	// Read image
	Mat im = imread( InputFile);
//	Mat im = imread( InputFile, IMREAD_GRAYSCALE );

	// Setup SimpleBlobDetector parameters.
	SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 10;
	params.maxThreshold = 240;

	// Filter by Area.
	params.filterByArea = true;
	params.minArea = 50;

	// Filter by Circularity
	params.filterByCircularity = false;
	params.minCircularity = 0.4;

	// Filter by Convexity
	params.filterByConvexity = false;
	params.minConvexity = 0.87;

	// Filter by Inertia
	params.filterByInertia = false;
	params.minInertiaRatio = 0.01;


	// Storage for blobs
	vector<KeyPoint> keypoints;

	Mat dst, bgr[3];

	cv::cvtColor(im,dst,cv::COLOR_BGR2HLS);

	cv::split(dst, bgr);

	imshow("H", bgr[0]);
	waitKey(0);
	imshow("L", bgr[1]);
	waitKey(0);
	imshow("S", bgr[2]);
	waitKey(0);

	cv::threshold (bgr[2], dst, 0, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);

	// Set up detector with params
	Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);   

	// Detect blobs
	detector->detect( dst, keypoints);

	// Draw detected blobs as red circles.
	// DrawMatchesFlags::DRAW_RICH_KEYPOINTS flag ensures
	// the size of the circle corresponds to the size of blob

	Mat im_with_keypoints;
	drawKeypoints( dst, keypoints, im_with_keypoints, Scalar(0,0,255), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

	// Show blobs
	imshow("keypoints", im_with_keypoints );
	waitKey(0);

}
