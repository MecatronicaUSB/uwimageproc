/********************************************************************/
/* File: template.cpp                                                 */
/* Last Edition: 30/01/2017, 07:12 PM.                              */
/********************************************************************/
/* Programmed by:                                                   */
/* Jose Cappelletto                                                 */
/********************************************************************/
/* Project: imageproc								                */
/* File: 	template.cpp							                */
/********************************************************************/

//basic c and c++ libraries
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

// OpenCV libraries
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/calib3d.hpp"
#include "opencv2/xfeatures2d.hpp"


#define TARGET_WIDTH 	640
#define TARGET_HEIGHT 	480

using namespace cv;
using namespace cv::xfeatures2d;
using namespace std;

char keyboard;

//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Start extraccting frames
//**** 4- Select 1st key frame
//**** 5- Extract following frames
//****	5.1- Compute Homography matrix
//****	5.2- Estimate overlapping of current frame with previous keyframe
//	5.3- If falls below treshold, pick best quality frame in the neighbourhood
//****	5.4- Asign it as next keyframe
//****	5.5 Repeat from 5

float calcOverlap(Mat image_scene, Mat image_object);

int videoWidth;
int videoHeight;
float hResizeFactor;

/*
	Usage: videostrip -i input [-r argument -o output]
*/
void printUsage(){
	cout << "Tool to automatically extract frames from video for 2D mosaic generation" << endl;
	cout << "Computes frame quality based on Laplacian variance, to select best frame that overlaps with previous selected frame" << endl;
	cout << "Overlap of consecutive selected frames is estimated through homography matrix H" << endl;  
	cout << "Built with OpenCV " << CV_VERSION << endl;
    cout << "\tUsage: videostrip -i input_file [-p overlap_factor -o output_file]" << endl;
    cout << "\t-h\tShow this help message" << endl;
    cout << "\t-i\tVideo input file" << endl;
    cout << "\t-p\t[optional] Percent of desired overlap between consecutive frames (0.0 to 1.0)" << endl;
    cout << "\t-k\t[optional] Defines window size of k-frames for keyframe tuning" << endl;
    cout << "\t-o\t[optional] Output files format, as file name base" << endl;
    cout <<  endl << "\tExample:" << endl;
    cout << "\t$ videostrip -i input.avi -o vdout -p 0.6" << endl;
    cout << "\tThis will open 'input.avi' file, extract frames with 60% of overlapping and export into 'vdout_XXXX.jpg' images" << endl << endl;
}

//********************************
int main(int argc, char* argv[]){

	// If not enough arguments or '-h' argument is called, the print usage instructions
	if (argc<3 || (string (argv[1]) == "-h")){
		printUsage();
		return 0;
	}
	//read the arguments provided through the command line
	string argument, argval;
	string InputFile; //strings input and output file names
	ostringstream OutputFile;

	int opt = 0;	//getop aux var
	float overlap = 0;	// target overlap among frame
	int kWindow = 0;

	// ********************************************************************************
	// parses the arg string, searching for arguments
	while ((opt = getopt(argc, argv, "i:p:k:o:")) != -1) {
	switch(opt) {
		case 'i':
			InputFile = string (optarg);
			break;
		case 'p':
			overlap = atof (optarg);
			break;
		case 'k':
			kWindow = atoi (optarg);
			break;
		case 'o':
			OutputFile << optarg;
			break;
		case '?':
			/* Case when user enters the command as
			* $ ./cmd_exe -i
			*/
			if (optopt == 'i') {
				cout << "Missing mandatory input option: -i input_file" << endl;
			}
			else if (optopt == 'o') {
				cout << "Missing mandatory output option: -o output_file" << endl;
			} 
			else if (optopt == 'p') {
				cout << "Missing mandatory overlap option: -o overlap_factor" << endl;
			} 
			else if (optopt == 'k') {
				cout << "Missing mandatory k-frame window option: -k k_frames" << endl;
			} 
			else {
				cout << "Invalid option received" << endl;
				}
			break;
		}
	}

	// Example of how to parse input file name
	//gets the path of the input source
	string FileName = InputFile.substr(InputFile.find_last_of("/")+1);
	string BasePath = InputFile.substr(0,InputFile.length() - FileName.length());
	//determines the filetype
	string FileType;
	int k = InputFile.find_last_of(".");
	if (k==-1) // DOT (.) not found, so filename doesn't contain extension
		FileType = "";
	else
		FileType = InputFile.substr();
	// now we build the FileBase from input FileName
	string FileBase = FileName.substr(0,FileName.length() - FileType.length());

	cout << "Input: " << InputFile << endl;
/*	cout << "Path:  " << BasePath << endl;
	cout << "File:  " << FileName << endl;
	cout << "Type:  " << FileType << endl;
	cout << "Base:  " << FileBase << endl;
	cout << "Output:" << OutputFile.str().c_str() << endl;//*/

    //create the capture object
    VideoCapture capture(InputFile);
    if(!capture.isOpened()){
        //error in opening the video input
//        cerr << "Unable to open video file: " << InputFile << endl;
        exit(EXIT_FAILURE);
    }
	// ********************************************************************************
	//now we retrieve and print info about input video
	videoWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	videoHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

	// we compute the resize factor for the horizontal dimension. As we preserve the aspect ratio, is the same for the veritcal resizing
	hResizeFactor = (float)TARGET_WIDTH / videoWidth;

	float videoFPS = capture.get(CV_CAP_PROP_FPS);
	int videoFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);

	cout << "Metadata:" << endl;
	cout << "\tSize:\t" << videoWidth << " x " << videoHeight << endl;
	cout << "\tFrames: " << videoFrames << " @ " << videoFPS << endl;
	cout << "\thResize: " << hResizeFactor << endl;

	// ********************************************************************************
	// Next, we start reading frames from video
	Mat frame, laplacian, keyframe, bestframe;
	Mat res_keyframe, res_frame;

	bool bImagen = false;
	keyboard = 0;

	// we use the first frame as keyframe (so far, further implementations should include cli arg to pick one by user)
	capture.read (keyframe);
	resize (keyframe, res_keyframe, cv::Size(hResizeFactor * keyframe.cols, hResizeFactor * keyframe.rows), 0, 0, CV_INTER_LINEAR);

	int g=0;
	float over;
	int out_frame = 0;
	char out_name[10];

    while( keyboard != 'q' && keyboard != 27 ){
        //read the current frame, if fails, the quit
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

		resize (frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);

		over = calcOverlap(res_keyframe, res_frame);
		cout << "Frame: " << g++ << "\t Overlap: " << over << endl;
		if (over<overlap){
			cout << "********* New keyframe" << endl;
			capture.read(keyframe);
			//here we should store this new frame (thinking is good enough). 
			/*TODO
			Startto search best frames in i+k frames, according to "blur level" estimator (based on Laplacian variance)
			*/
			out_frame++;
			sprintf (out_name, "OUT_%04d.jpg",out_frame);
			cout << "Writting: " << out_name << endl;
			imwrite (out_name, keyframe);			

			resize (keyframe, res_keyframe, cv::Size(hResizeFactor * keyframe.cols, hResizeFactor * keyframe.rows), 0, 0, CV_INTER_LINEAR);
		}	

        //get the input from the keyboard
        keyboard = (char)waitKey( 5 );
/*
		if (keyboard == ' ') bImagen = !bImagen;

        Laplacian(frame,laplacian,CV_16S, 7);

        Scalar mean, stdev;
        meanStdDev(laplacian,mean,stdev);
        double m = mean.val[0];
        double s = stdev.val[0];

        stringstream ss,tt;
        rectangle(frame, cv::Point(10, 2), cv::Point(300,20),
                  cv::Scalar(255,255,255), -1);
        ss << "F: " << capture.get(CAP_PROP_POS_FRAMES) << "\tstd: " << s;
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks

		if (bImagen)
	        imshow("Frame", laplacian);
		else
	        imshow("Frame", frame);*/
    }
    //delete capture object
    capture.release();

//*****************************************************************************
	return 0;
}																																																											


float calcOverlap(Mat img_scene, Mat img_object)
{
/*  Mat img_object = imread( argv[1], IMREAD_GRAYSCALE );
  Mat img_scene = imread( argv[2], IMREAD_GRAYSCALE );*/

	if( !img_object.data || !img_scene.data )
	{ std::cout<< " --(!) Error reading images " << std::endl; return -1; }

	//-- Step 1: Detect the keypoints using SURF Detector
	int minHessian = 400;

	Ptr<SURF> detector = SURF::create(minHessian);
 
	std::vector<KeyPoint> keypoints_object, keypoints_scene;

	detector->detect( img_object, keypoints_object );
	detector->detect( img_scene, keypoints_scene );

	//-- Step 2: Calculate descriptors (feature vectors)
	Ptr<SURF> extractor = SURF::create();

	Mat descriptors_object, descriptors_scene;

	extractor->compute( img_object, keypoints_object, descriptors_object );
	extractor->compute( img_scene, keypoints_scene, descriptors_scene );

	//-- Step 3: Matching descriptor vectors using FLANN matcher
	FlannBasedMatcher matcher;
	std::vector< DMatch > matches;
	matcher.match( descriptors_object, descriptors_scene, matches );

	double max_dist = 0; double min_dist = 100;

	//-- Quick calculation of max and min distances between keypoints
	for( int i = 0; i < descriptors_object.rows; i++ )
	{ 
		double dist = matches[i].distance;
		if( dist < min_dist ) min_dist = dist;
		if( dist > max_dist ) max_dist = dist;
	}

	//-- Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	std::vector< DMatch > good_matches;

	for( int i = 0; i < descriptors_object.rows; i++ )
	{ 	
		if( matches[i].distance < 2*min_dist )
			{ good_matches.push_back( matches[i]); }
	}

	Mat img_matches;
	drawMatches( img_object, keypoints_object, img_scene, keypoints_scene, good_matches, img_matches, Scalar::all(-1), Scalar::all(-1), vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

	//-- Localize the object
	std::vector<Point2f> obj;
	std::vector<Point2f> scene;

	for( int i = 0; i < good_matches.size(); i++ )
	{
		//-- Get the keypoints from the good matches
		obj.push_back( keypoints_object[ good_matches[i].queryIdx ].pt );
		scene.push_back( keypoints_scene[ good_matches[i].trainIdx ].pt );
	}

  	Mat H = findHomography( obj, scene, RANSAC );
//	cout << H << endl;
	float  dx = fabs(H.at<double>(0,2));
	float  dy = fabs(H.at<double>(1,2));

	float overlap = (videoWidth - dx)*(videoHeight - dy)/(videoWidth * videoHeight);
	return overlap;
//	cout << "Overlap: " << overlap << endl;
}
