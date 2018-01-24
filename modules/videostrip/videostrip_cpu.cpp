/********************************************************************/
/* Project: imageproc								                */
/* Module: 	Videostrip								                */
/* File: 	videostrip.cpp                                          */
/* Created:		11/12/2016                                          */
/* Edited:		20/01/2018, 07:12 PM                                */
/* Description:						                                
	Module that extracts frames from video for 2D mosaic or 3D model reconstruction. It estimates the overlap among frames
	by computing the homography matrix. CPU based implementation
*/

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
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>

/// Constant definitios
#define TARGET_WIDTH	640        //< Resized image width
#define TARGET_HEIGHT	480        //< Resized image height
#define OVERLAP_MIN  	0.4        //< Minimum desired overlap among consecutive key frames
#define DEFAULT_KWINDOW 11         //< Search window size for best blur-based frame, after new key frame

// #define _VERBOSE_ON_
// C++ namespaces
using namespace cv;
using namespace cv::cuda;
using namespace cv::xfeatures2d;
using namespace std;

char keyboard = 0;	// keyboard input character

double t;	// Timing monitor

// Structure to save the reference frame data, useful to reuse keypoints and descriptors
typedef struct {
    bool new_img;               // boolean value to know if it have the keypoints data stored
    vector<KeyPoint> keypoints; // keypoints of refererence frame
    Mat descriptors;            // Descriptors of refererence frame
    Mat img;                    // reference frame
    Mat res_img;                // resized frame to TARGET_WIDTH x TARGET_HEIGHT
} struct_keyframe;

// General structure index:
//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Start extracting frames
//**** 4- Select 1st key frame
//**** 5- Extract following frames
//****	5.1- Compute Homography matrix
//****	5.2- Estimate overlapping of current frame with previous keyframe
//****  5.3- If it falls below threshold, pick best quality frame in the neighbourhood
//****	5.4- Assign it as next keyframe
//****	5.5 Repeat from 5

// See description in function definition
float calcOverlap(struct_keyframe* key_frame, Mat image_object);
// See description in function definition
float calcBlur(Mat frame);

// Video width and height
int videoWidth, videoHeight;
// Resize factor from original video frame size to desired TARGET_WIDTH or TARGET_HEIGHT
// This is for fast motion estimation through homography. Perhaps some optical-flow approach could work faster
// Image tiling may improve homography quality by forcing well-spread control points along the image (See CIRS paper)
float hResizeFactor;

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
                    "{@output |<none> | Prefix for output .jpg images}" // output prefix is the second argument (positional)
                    "{p      |0.95  | Percent of desired overlap between consecutive frames (0.0 to 1.0)}"
                    "{k      |      | Defines window size of k-frames for keyframe tuning}"
                    "{s      | 0    | Skip NN seconds from the start of the video}"
                    "{help h usage ?  |       | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about("videostrip module v0.3");	//adds "about" information to the parser method

	//if the number of arguments is lower than 3, or contains "help" keyword, then we show the help
	if (argc < 3 || cvParser.has("help")) {
        cout << "Automatically extract video frames for 2D mosaic generation or 3D model reconstruction" << endl;
        cout <<
        "Computes frame quality based on Laplacian variance, to select best frame that overlaps with previous selected frame" <<
        endl;
        cout << "Overlap of consecutive selected frames is estimated through homography matrix H" << endl;
        cvParser.printMessage();
        cout << endl << "\tExample:" << endl;
        cout << "\t$ videostrip -p=0.6 -k=5 -s=12 input.avi vdout_" << endl;
        cout <<
        "\tThis will open 'input.avi' file, extract frames with 60% of overlapping, skipping first 12 seconds, and export into 'vdout_XXXX.jpg' images" << endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);		//String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(1);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;						// output string that will contain the desired output file name

	int timeSkip = cvParser.get<int>("s"); 		// gets argument -s=NN, where NN is the number of seconds to skip from the start of the video
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

    //**************************************************************************
    /* VIDEO INPUT */

    //<create the capture object
    VideoCapture capture(InputFile);
    if (! capture.isOpened()) {
        //error while opening the video input
        cerr << "Unable to open video file: " << InputFile << endl;
        exit(EXIT_FAILURE);
    }
    //now we retrieve and print info about input video
    videoWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    videoHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

    // we compute the resize factor for the horizontal dimension. As we preserve the aspect ratio, is the same for the vertical resizing
    hResizeFactor = (float) TARGET_WIDTH / videoWidth;

    float videoFPS = capture.get(CV_CAP_PROP_FPS);
    int videoFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);

    cout << "Video metadata:" << endl;
    cout << "\tSize:\t" << videoWidth << " x " << videoHeight << endl;
    cout << "\tFrames:\t" << videoFrames << " @ " << videoFPS << endl;
    cout << "\thResize:\t" << hResizeFactor << endl;
    cout << "Target overlap:\t" << overlap << endl;
    cout << "K-Window size:\t" << kWindow << endl;
	if (timeSkip > 0) cout << "Time skip:\t" << timeSkip << endl;

	//we compute the (exact) number of frames to be skipped, given a desired amount of seconds to skip from start
	float frameSkip;
	if (timeSkip > 0){
		frameSkip = (float)timeSkip * videoFrames;
		capture.set(CV_CAP_PROP_POS_MSEC, timeSkip*1000);
	}

    //**************************************************************************
    /* PROCESS START */
    // Next, we start reading frames from the input video
    Mat frame(videoWidth, videoHeight, CV_8UC1);
    Mat bestframe, res_frame;
    struct_keyframe key_frame;

    float over;
    int out_frame = 0, read_frame = 0;

    // we use the first frame as keyframe (so far, further implementations should include cli arg to pick one by user)
    capture.read(key_frame.img);    read_frame ++;
    key_frame.new_img = true;

    // resizing for speed purposes
    resize(key_frame.img, key_frame.res_img, cv::Size(hResizeFactor * key_frame.img.cols, hResizeFactor * key_frame.img.rows), 0, 0,
           CV_INTER_LINEAR);

    OutputFileName.str("");
    OutputFileName << OutputFile << setfill('0') << setw(4) << out_frame << ".jpg";
    imwrite(OutputFileName.str(), key_frame.img);

    while (keyboard != 'q' && keyboard != 27) {
        t = (double) getTickCount();
        //read the current frame, if fails, the quit
        if (!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        read_frame ++;

        float bestBlur = 0.0, currBlur;    //we start using the current frame blur as best blur value yet
        resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);

        over = calcOverlap(&key_frame, res_frame);
        cout << '\r' << "Frame: " << read_frame << " [" << out_frame << "]\tOverlap: " << over << std::flush;
		//special case: overlap cannot be computed, we force it with an impossible negative value
        // TODO: check better numerically stable way to detect failed overlap detection, rather through forced value
		if (over == -2.0){
/*			cout << endl << "Forcing current new keyframe" << endl;
			keyframe = frame.clone();		
	        resize(keyframe, res_keyframe, cv::Size(), hResizeFactor, hResizeFactor);*/
			over = OVERLAP_MIN + 0.01;
		}
		if ((over < overlap) && (over > OVERLAP_MIN)) {
			cout << endl;
            /*!
            Start to search best frames in i+k frames, according to "blur level" estimator (based on Laplacian variance)
            We start using current frame as best frame so far
            */
            bestBlur = calcBlur(res_frame);
            bestframe = frame.clone();
            //for each frame inside the k-consecutive frame window, we refine the search
            for (int n = 0; n < kWindow; n ++) {

				if (! capture.read(frame)) {
				    cerr << endl << "Unable to read next frame." << endl;
				    cerr << "Ending..." << endl;
				    exit(EXIT_FAILURE);
				}
                read_frame ++;
                resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);    //uses a resized version
                
                currBlur = calcBlur(res_frame);    //we operate over the resampled image for speed purposes

                cout << '\r' << "Refining for Blur [" << n+1 << "/" << kWindow << "]\tBlur: " << currBlur << "\tBest: " << bestBlur << std::flush;
                if (currBlur > bestBlur) {    //if current blur is better, replaces best frame
                    bestBlur = currBlur;
                    bestframe = frame.clone();  //best fram is a copy of frame
                }
            }
            //< finally the new keyframe is the best frame from las iteration
            key_frame.img = bestframe.clone();
            key_frame.new_img = true;
            out_frame ++;

            OutputFileName.str("");
            OutputFileName << OutputFile << setfill('0') << setw(4) << out_frame << ".jpg";

            imwrite(OutputFileName.str(), bestframe);

#ifdef _VERBOSE_ON_
            t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
            cout << endl << "BestBlur: " << t << " ms" << endl;
            t = (double) getTickCount();
#endif
			cout << "..." << endl;
            resize(key_frame.img, key_frame.res_img, cv::Size(), hResizeFactor, hResizeFactor);
        }

        //get the input from the keyboard
        keyboard = (char) waitKey(5);
    }
    //delete capture object
    capture.release();

//*****************************************************************************
    return 0;
}

/*! @fn float calcBlur (Mat frame)
    @brief Calculates the "blur" of a given Mat frame, based on the standard deviation of the Laplacian of the input frame
    @param frame OpenCV matrix container of the input frame
	@retval The estimated blur for the given frame
*/
float calcBlur(Mat frame) {
    // Avg time: 0.7 ms GPU/ 23ms CPU
    Mat grey, laplacian;    
    cvtColor(frame, grey, COLOR_BGR2GRAY);

    //perform Laplacian filter
    Laplacian(grey, laplacian, grey.type(), CV_16S);

    //we compute the laplacian of the current frame
    Scalar mean, stdev;
    // then we compute the mean and stdev of that frame
    meanStdDev(laplacian, mean, stdev);
    // double m = mean.val[0];
    // double s = stdev.val[0];
    // we return the standard deviation (stdev)
    return stdev.val[0];
}

/*! @fn float calcOverlap(struct_keyframe* key_frame, Mat img_object)
    @brief Calculates the percentage of overlapping among two frames, by estimating the Homography matrix.
    @param key_frame	Structure container of reference frame, keypoints and descriptors
    @param img_object	Mat OpenCV matrix container of target frame
	@brief retval		The normalized overlap among two given frame
*/
float calcOverlap(struct_keyframe* key_frame, Mat img_object) {
	// if any of the input images are empty, then exits with error code
    if (! img_object.data || ! key_frame->res_img.data) {
        cout << " --(!) Error reading images " << std::endl;
        return - 1;
    }

    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    Mat descriptors_object, descriptors_scene;
    vector<KeyPoint> keypoints_object, keypoints_scene;
    Ptr<SURF> detector = SURF::create(minHessian);

    // Convert to grayscale
    cvtColor(img_object, img_object, COLOR_BGR2GRAY);

    // Detect keypoints
    detector->detectAndCompute(img_object, Mat(), keypoints_object, descriptors_object);

    // If we have a new keyframe compute the keypoints
    if(key_frame->new_img){
        cvtColor(key_frame->res_img, key_frame->res_img, COLOR_BGR2GRAY);
        detector->detectAndCompute(key_frame->res_img, Mat(), key_frame->keypoints, key_frame->descriptors);
        key_frame->new_img = false;
    }
    // else, reuse them from key_frame structure
    keypoints_scene = key_frame->keypoints;
    descriptors_scene = key_frame->descriptors;

#ifdef _VERBOSE_ON_
    t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
    cout << endl << "SURF: " << t << " ms ";
    t = (double) getTickCount();
#endif

    //***************************************************************//
    //-- Step 2: Matching descriptor vectors using Flann based matcher
    // Avg time: 2.5 ms GPU / 21 ms CPU
        //	cout << ">>[OV] SURF ok-------" << endl;
    vector<DMatch> matches;

    FlannBasedMatcher matcher;
    matcher.match(descriptors_object, descriptors_scene, matches);

    //-- Step 3: Select only good matches based on euclidean distance between descriptors
    vector<DMatch> good_matches;
    double min_dist = 100;
    for( int i = 0; i < matches.size(); i++ ){
        double dist = matches[i].distance;
        if( dist < min_dist )
            min_dist = dist;
    }
    for( int i = 0; i < matches.size(); i++ ){
        if( matches[i].distance <= max(2*min_dist, 0.6) ){
            good_matches.push_back( matches[i]);
        }
    }

#ifdef _VERBOSE_ON_
    t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
    cout << "\t | BFMatcher GPU: " << t << " ms ";
    t = (double) getTickCount();
#endif


    //***************************************************************//
    //we must check if found H matrix is good enough. It requires at least 4 points
    if (good_matches.size() < 4) {
        cout << "[WARN] Not enough good matches!" << endl;
        //we fail to estimate new overlap
        return -2.0;
    }
    else {
        //-- Localize the object
        vector<Point2f> obj, scene;

        for (int i = 0; i < good_matches.size(); i ++) {
            //-- Get the keypoints from the good matches
            obj.push_back(keypoints_object[good_matches[i].queryIdx].pt);
            scene.push_back(keypoints_scene[good_matches[i].trainIdx].pt);
        }

        // TODO: As OpenCV 3.2, there is no GPU based implementation for findHomography.
        // Check http://nghiaho.com/?page_id=611 for an external solution
        // Avg time: 0.7 ms CPU
        Mat H = findHomography(obj, scene, RANSAC);
		
		if (H.empty())	return -2.0;

        float dx = fabs(H.at<double>(0, 2));
        float dy = fabs(H.at<double>(1, 2));
        float overlap = (videoWidth - dx) * (videoHeight - dy) / (videoWidth * videoHeight);

#ifdef _VERBOSE_ON_
        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        cout << "\t | Homography: " << t << " ms" << endl;
        t = (double) getTickCount();
#endif
        return overlap;
    }
}
