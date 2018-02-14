/********************************************************************/
/* Project: imageproc								                */
/* Module: 	Videostrip								                */
/* File: 	videostrip.cpp                                          */
/* Created:		11/12/2016                                          */
/* Edited:		30/01/2017, 07:12 PM                                */
/* Description:						                                
	Module that extracts frames from video for 2D mosaic or 3D model reconstruction. It estimates the overlap among frames
	by computing the homography matrix. Current OpenCV implementation uses GPU acceleration for feature detection and matching
	through CUDA library.
*/

/********************************************************************/
/* Created by:                                                      */
/* Jose Cappelletto - cappelletto@usb.ve			                */
/* Collaborators:                                                   */
/* Victor Garcia - victorygarciac@gmail.com                         */
/********************************************************************/

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
//#include <stdlib.h>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>

<<<<<<< HEAD
#include "../include/options.h"
// #cmakedefine USE_GPU

=======
>>>>>>> videostriGPUonoff
/// CUDA specific libraries
#if USE_GPU
    #include <opencv2/cudafilters.hpp>
    #include "opencv2/cudafeatures2d.hpp"
    #include "opencv2/xfeatures2d/cuda.hpp"
#endif

/// Constant definitios
#define TARGET_WIDTH	640        //< Resized image width
#define TARGET_HEIGHT	480        //< Resized image height
#define OVERLAP_MIN  	0.8        //< Minimum desired overlap among consecutive key frames
#define DEFAULT_KWINDOW 11         //< Search window size for best blur-based frame, after new key frame
#define DEFAULT_TIMESKIP 0         //< Search window size for best blur-based frame, after new key frame

//#define _VERBOSE_ON_
// C++ namespaces
using namespace cv;
using namespace cv::cuda;
using namespace cv::xfeatures2d;
using namespace std;

char keyboard = 0;	// keyboard input character

double t;	// Timing monitor

#if USE_GPU
    GpuMat auxD;    // Auxilary GpuMat to keep track of descriptors
#endif


// Structure to save the reference frame data, useful to reuse keypoints and descriptors

typedef struct {
    bool new_img;               // boolean value to know if it has the keypoints data stored
    vector<KeyPoint> keypoints; // keypoints of refererence frame
    Mat descriptors;            // Descriptors of refererence frame
    Mat img;                    // reference frame
    Mat res_img;                // resized frame to TARGET_WIDTH x TARGET_HEIGHT
} keyframe;

// General structure index:
//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Start extracting frames
//**** 4- Select 1st key frame
//**** 5- Extract following frames
//****	5.1- Compute Homography matrix
//****	5.2- Estimate overlapping of current frame with previous keyframe
//	5.3- If it falls below threshold, pick best quality frame in the neighbourhood
//****	5.4- Assign it as next keyframe
//****	5.5 Repeat from 5

// See description in function definition
float calcOverlap(keyframe* kframe, Mat image_object);
// See description in function definition
float calcOverlapGPU(keyframe* kframe, Mat image_object);
// See description in function definition
float calcBlur(Mat frame);
// See description in function definition
float calcBlurGPU(Mat frame);
// See description in function definition
float overlapArea(Mat H);

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
<<<<<<< HEAD
    std::string descriptionString = \
    "videostrip - module part of [uwimageproc] toolbox, for smart extraction of video frames.\
    Employs a feature-based homography matrix estimation to calculate the overlap among best frames.\
    Frame quality is based on Laplacian variance, in order to select the best frame that overlaps with previous selected frame";

    argParser.Description(descriptionString);
    argParser.Epilog("Author: J. Cappelletto (GitHub: @cappelletto)\nVisit [https://github.com/mecatronicaUSB] for project information\n");
    argParser.Prog(argv[0]);
    argParser.helpParams.width = 120;

    try{
        argParser.ParseCLI(argc, argv);
    }
    catch (args::Help){    // if argument asking for help, show this message
        cout << argParser;
        return 1;
    }
    catch (args::ParseError e){  //if some error ocurr while parsing, show summary
        std::cerr << e.what() << std::endl;
        std::cerr << "Use -h, --help command to see usage" << std::endl;
        return 1;
    }
    catch (args::ValidationError e){ // if some error at argument validation, show
        std::cerr << "Bad input commands" << std::endl;
        std::cerr << "Use -h, --help command to see usage" << std::endl;
        return 1;
    }

    /*
     * Display son build info
     */
=======
/*  Uses built-in OpenCV parsing method cv::CommandLineParser. It requires a string containing the arguments to be parsed from
	the command line. Further details can be obtained from opencv webpage
*/
    String keys =
            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
                    "{@output |<none> | Prefix for output .jpg images}" // output prefix is the second argument (positional)
                    "{p      |0.95  | Percent of desired overlap between consecutive frames (0.0 to 1.0)}"
                    "{k      |      | Defines window size of k-frames for keyframe tuning}"
                    "{s      | 0    | Skip NN seconds from the start of the video}"
                    "{cuda    |       | Use CUDA or not (CUDA ON: 1, CUDA OFF: 0)}"         // Use CUDA (if available) or not   
                    "{help h usage ?  |      | show this help message}";      // optional, show help optional

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
        cout << "\t$ videostrip -p=0.6 -k=5 -s=12 -cuda=0 input.avi vdout_" << endl;
        cout <<
        "\tThis will open 'input.avi' file, extract frames with 60% of overlapping, skipping first 12 seconds, and export into 'vdout_XXXX.jpg' images" << endl << endl;
        return 0;
    }
    int CUDA = 0;                                       //Default option (running with CPU)
    String InputFile = cvParser.get<cv::String>(0);		//String containing the input file path+name from cvParser function
    String OutputFile = cvParser.get<cv::String>(1);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;						// output string that will contain the desired output file name
>>>>>>> videostriGPUonoff

    cout << "Built with OpenCV " << CV_VERSION << " @ " << __DATE__ << " - " << __TIME__ << endl;
#ifdef USE_GPU
    cout << "CUDA mode enabled" << endl;
#endif
    /*
     * Start parsing mandatory arguments
     */

    if (!argInput){
        cerr << "Mandatory <input> file name missing" << endl;
        cerr << "Use -h, --help command to see usage" << endl;
        return 1;
    }

    if (!argOutput){
        cerr << "Mandatory <output> file name missing" << endl;
        cerr << "Use -h, --help command to see usage" << endl;
        return 1;
    }

    String InputFile = args::get(argInput);	//String containing the input file path+name from cvParser function
    String OutputFile = args::get(argOutput);	//String containing the output file template from cvParser function
    ostringstream OutputFileName;				// output string that will contain the desired output file name
    /*
     * These were the mandatory arguments. Now we proceed to optional parameters.
     * When each variable is defined, we assign the default value.
     */
    int timeSkip = DEFAULT_TIMESKIP;	// number of seconds to skip from the start of the video
    int kWindow = DEFAULT_KWINDOW;		// size of the search window for the best frame
    float overlap = OVERLAP_MIN;	    // desired overlap percentage between frames

    /*
     * Now, start verifying each optional argument from argParser
     */

    if (argTimeSkip){
        cout << "timeSkip value provided: " << (timeSkip = args::get(argTimeSkip)) << endl;
    }
    else{
        cout << "timeSkip default value: " << timeSkip << endl;
    }

    if (argWindowSize){
        cout << "windowSize value provided: " << (kWindow = args::get(argWindowSize)) << endl;
    }
    else{
        cout << "windowSize default value: " << kWindow << endl;
    }

    if (argOverlap){
        cout << "overlap value provided: " << (overlap = args::get(argOverlap)) << endl;
    }
    else{
        cout << "overlap default value: " << overlap << endl;
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
    int nCuda = - 1;    //<Defines number of detected CUDA devices. By default, -1 acting as error value
<<<<<<< HEAD

#ifdef USE_GPU
    /* CUDA */
    // TODO: read about possible failure at runtime when calling CUDA methods in non-CUDA hardware.
    // CHECK whether it is possible with try-catch pair
    nCuda = cuda::getCudaEnabledDeviceCount();	// we try to detect any existing CUDA device
    cuda::DeviceInfo deviceInfo;

    if (nCuda > 0){
        cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
        cuda::setDevice(0);
    }
    else {
#undef USE_GPU
        cout << "No CUDA device detected" << endl;
        cout << "Exiting... use non-GPU version instead" << endl;
    }
#endif
=======
    #if USE_GPU
        CUDA = cvParser.get<int>("cuda");	        // gets argument -cuda=x, where 'x' define to use CUDA or not
        nCuda = cuda::getCudaEnabledDeviceCount();	// Try to detect any existing CUDA device
        // Deactivate CUDA from parse
        if (CUDA == 0){            
            cout << "CUDA deactivated" << endl;
            cout << "Exiting... use non-GPU version instead" << endl;
        }
        // Find CUDA devices
        else if (nCuda > 0){   
            cuda::DeviceInfo deviceInfo;
            cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
            cuda::setDevice(0);
        }
        else {
            CUDA = 0;
            cout << "No CUDA device detected" << endl;
            cout << "Exiting... use non-GPU version instead" << endl;
        }
    #endif
>>>>>>> videostriGPUonoff
    // TODO: How to operate when multiple CUDA devices are detected?
    // So far, we work with the first detected CUDA device. Maybe, add some CUDA probe mode when called

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
    // struct keyframe
    keyframe kframe; 
    
    float over;
    int out_frame = 0, read_frame = 0;

    // we use the first frame as keyframe (so far, further implementations should include cli arg to pick one by user)
    capture.read(kframe.img);    read_frame ++;
    kframe.new_img = true;
    // resizing for speed purposes
    resize(kframe.img, kframe.res_img, cv::Size(hResizeFactor * kframe.img.cols, hResizeFactor * kframe.img.rows), 0, 0,
           CV_INTER_LINEAR);

    OutputFileName.str("");
    OutputFileName << OutputFile << setfill('0') << setw(4) << out_frame << ".jpg";
    imwrite(OutputFileName.str(), kframe.img);

    while (keyboard != 'q' && keyboard != 27) {
        t = (double) getTickCount();
        //read the current frame, if fails, the quit
        if (!capture.read(frame)) {
            cerr << "\nUnable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        read_frame ++;

        float bestBlur = 0.0, currBlur;    //we start using the current frame blur as best blur value yet
        resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);
        #if USE_GPU
        if(CUDA)
            over = calcOverlapGPU(&kframe, res_frame);
        #endif
        if(not CUDA)
            over = calcOverlap(&kframe, res_frame);

        cout << '\r' << "Frame: " << read_frame << " [" << out_frame << "]\tOverlap: " << over << std::flush;

		//special case: overlap cannot be computed, we force it with an impossible negative value
        // TODO: check better numerically stable way to detect failed overlap detection, rather through forced value
		if (over == -2.0){
            /*cout << endl << "Forcing current new keyframe" << endl;
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
            #if USE_GPU
            if(CUDA)
                bestBlur = calcBlurGPU(res_frame);
            #endif
            if(not CUDA)
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

                //we operate over the resampled image for speed purposes
                #if USE_GPU
                if(CUDA)
                    currBlur = calcBlurGPU(res_frame);
                #endif
                if(not CUDA)
                    currBlur = calcBlur(res_frame);    

                cout << '\r' << "Refining for Blur [" << n+1 << "/" << kWindow << "]\tBlur: " << currBlur << "\tBest: " << bestBlur << std::flush;
                if (currBlur > bestBlur) {    //if current blur is better, replaces best frame
                    bestBlur = currBlur;
                    bestframe = frame.clone();  //best fram is a copy of frame
                }
            }
            //< finally the new keyframe is the best frame from las iteration
            kframe.img = bestframe.clone();
            kframe.new_img = true;
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
            resize(kframe.img, kframe.res_img, cv::Size(), hResizeFactor, hResizeFactor);
        }

        //get the input from the keyboard
        keyboard = (char) waitKey(5);
    }
    //delete capture object
    capture.release();

//*****************************************************************************
    return 0;
}

#if USE_GPU
/*! @fn float calcBlurGPU (Mat frame)
    @brief Calculates the "blur" of a given Mat frame using GPU, based on the standard deviation of the Laplacian of the input frame
    @param frame OpenCV matrix container of the input frame
	@retval The estimated blur for the given frame
*/
float calcBlurGPU(Mat frame) {
    // Avg time: 0.7 ms GPU/ 23ms CPU
    Mat grey, laplacian;
    cvtColor(frame, grey, COLOR_BGR2GRAY);

    cuda::GpuMat gpuFrame, gpuLaplacian;
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
    // double m = mean.val[0];
    // double s = stdev.val[0];
    // we return the standard deviation (stdev)
    return stdev.val[0];
}

/*! @fn float calcOverlapGPU(Mat img_scene, Mat img_object)
    @brief Calculates the percentage of overlapping among two frames using GPU, by estimating the Homography matrix.
    @param
            img_scene	Mat OpenCV matrix container of reference frame
    @param img_object	Mat OpenCV matrix container of target frame
	@brief retval		The normalized overlap among two given frame
*/
float calcOverlapGPU(keyframe* kframe, Mat img_object) {
	// if any of the input images are empty, then exits with error code
    if (! img_object.data || ! kframe->res_img.data) {
        cout << " --(!) Error reading images " << std::endl;
        return - 1;
    }
    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    // Convert to grayscale
    cvtColor(img_object, img_object, COLOR_BGR2GRAY);

    vector<KeyPoint> keypoints_object, keypoints_scene;

    cuda::GpuMat gpu_img_objectGPU, gpu_img_sceneGPU;
    cuda::GpuMat keypoints_objectGPU, keypoints_sceneGPU;
    cuda::GpuMat descriptors_objectGPU, descriptors_sceneGPU;
    // Upload to GPU
    gpu_img_objectGPU.upload(img_object);
    // Detect keypoints
    cuda::SURF_CUDA surf;
    surf(gpu_img_objectGPU, cuda::GpuMat(), keypoints_objectGPU, descriptors_objectGPU);
    surf.downloadKeypoints(keypoints_objectGPU, keypoints_object);
    if(kframe->new_img){
        cvtColor(kframe->res_img, kframe->res_img, COLOR_BGR2GRAY);
        gpu_img_sceneGPU.upload(kframe->res_img);
        surf(gpu_img_sceneGPU, cuda::GpuMat(), keypoints_sceneGPU, descriptors_sceneGPU);
        auxD = descriptors_sceneGPU;
        surf.downloadKeypoints(keypoints_sceneGPU, kframe->keypoints);
        kframe->new_img = false;
    }

    keypoints_scene = kframe->keypoints;
    descriptors_sceneGPU = auxD;
 

#ifdef _VERBOSE_ON_
    t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
    cout << endl << "SURF@GPU: " << t << " ms ";
    t = (double) getTickCount();
#endif

    //***************************************************************//
    //-- Step 3: Matching descriptor vectors using GPU BruteForce matcher (instead CPU FLANN)
    // Avg time: 2.5 ms GPU / 21 ms CPU
    double min_dist = 100;

    Ptr<cuda::DescriptorMatcher> matcher_gpu = cuda::DescriptorMatcher::createBFMatcher();
    vector< vector< DMatch> > matches;
    matcher_gpu->knnMatch(descriptors_objectGPU, descriptors_sceneGPU, matches, 2);

    //-- Step 4: Select only good matches
    vector<DMatch> good_matches;
    for (int k = 0; k < std::min(keypoints_scene.size() - 1, matches.size()); k ++) {
        if ((matches[k][0].distance < 0.8 * (matches[k][1].distance)) &&
            ((int) matches[k].size() <= 2 && (int) matches[k].size() > 0)) {
            // take the first result only if its distance is smaller than 0.6*second_best_dist
            // that means this descriptor is ignored if the second distance is bigger or of similar
            good_matches.push_back(matches[k][0]);
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

        // Old overlap area calc method ----
        // float dx = fabs(H.at<double>(0, 2));
        // float dy = fabs(H.at<double>(1, 2));
        // float overlap = (videoWidth - dx) * (videoHeight - dy) / (videoWidth * videoHeight);
        
        float overlap = overlapArea(H)/ (videoWidth * videoHeight);

#ifdef _VERBOSE_ON_
        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        cout << "\t | Homography: " << t << " ms" << endl;
        t = (double) getTickCount();
#endif
        return overlap;
    }
}
#endif

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
    // the we compute the mean and stdev of that frame
    meanStdDev(laplacian, mean, stdev);
    // double m = mean.val[0];
    // double s = stdev.val[0];
    // we return the standard deviation (stdev)
    return stdev.val[0];
}

/*! @fn float calcOverlap(Mat img_scene, Mat img_object)
    @brief Calculates the percentage of overlapping among two frames, by estimating the Homography matrix.
    @param
            img_scene	Mat OpenCV matrix container of reference frame
    @param img_object	Mat OpenCV matrix container of target frame
	@brief retval		The normalized overlap among two given frame
*/
float calcOverlap(keyframe* kframe, Mat img_object) {
	// if any of the input images are empty, then exits with error code
    if (! img_object.data || ! kframe->res_img.data) {
        cout << " --(!) Error reading images " << std::endl;
        return - 1;
    }

    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 400;
    // Convert to grayscale
    cvtColor(img_object, img_object, COLOR_BGR2GRAY);

    Mat descriptors_object, descriptors_scene;
    vector<KeyPoint> keypoints_object, keypoints_scene;
    Ptr<SURF> detector = SURF::create(minHessian);
    // If we have a new keyframe compute the keypoints
    detector->detectAndCompute(img_object, Mat(), keypoints_object, descriptors_object);
    if(kframe->new_img){
        cvtColor(kframe->res_img, kframe->res_img, COLOR_BGR2GRAY);
        detector->detectAndCompute(kframe->res_img, Mat(), kframe->keypoints, kframe->descriptors);
        kframe->new_img = false;
    }

    keypoints_scene = kframe->keypoints;
    descriptors_scene = kframe->descriptors;
 
#ifdef _VERBOSE_ON_
    t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
    cout << endl << "SURF@GPU: " << t << " ms ";
    t = (double) getTickCount();
#endif

    //***************************************************************//
    //-- Step 3: Matching descriptor vectors using GPU BruteForce matcher (instead CPU FLANN)
    // Avg time: 2.5 ms GPU / 21 ms CPU
    double min_dist = 100;

    Ptr<BFMatcher> matcher = BFMatcher::create();
    vector<vector<DMatch> > matches;
    matcher->knnMatch(descriptors_object, descriptors_scene, matches, 2);

    //-- Step 4: Select only good matches
    std::vector<DMatch> good_matches;
    for (int k = 0; k < std::min(keypoints_object.size() - 1, matches.size()); k ++) {
        if ((matches[k][0].distance < 0.8 * (matches[k][1].distance)) &&
            ((int) matches[k].size() <= 2 && (int) matches[k].size() > 0)) {
            // take the first result only if its distance is smaller than 0.6*second_best_dist
            // that means this descriptor is ignored if the second distance is bigger or of similar
            good_matches.push_back(matches[k][0]);
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

        // Old overlap area calc method ----
        // float dx = fabs(H.at<double>(0, 2));
        // float dy = fabs(H.at<double>(1, 2));
        // float overlap = (videoWidth - dx) * (videoHeight - dy) / (videoWidth * videoHeight);
        // ---------------------------------
        
        float overlap = overlapArea(H);

#ifdef _VERBOSE_ON_
        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        cout << "\t | Homography: " << t << " ms" << endl;
        t = (double) getTickCount();
#endif
        return overlap;
    }
}

/**
 * @brief Function to obtain the real overlap area between transformed and reference frame
 * @param H OpenCV Matrix containing the homography transformation
 * @return float The normalized overlap among two given frame
 */
float overlapArea(Mat H){
    vector<Point2f> points, final_points;
    float area_percent, area_over, area_img1, area_img2;
    // initialize the initial points in the corners of the original image
	points.push_back(Point2f(0,0));
	points.push_back(Point2f(TARGET_WIDTH,0));
	points.push_back(Point2f(TARGET_WIDTH,TARGET_HEIGHT));
	points.push_back(Point2f(0,TARGET_HEIGHT));

    // transform the original points by the given homography matrix
	perspectiveTransform(points,final_points, H);

    // Save the transformed points into an array for fillConvexPoly function
    Point points_array[4] = {final_points[0], final_points[1], final_points[2], final_points[3]};

    Mat mask(TARGET_HEIGHT, TARGET_WIDTH, CV_8UC1, Scalar(0,0,0));
    // Fill the area inside the transformed points
    fillConvexPoly( mask, points_array, 4, Scalar(255,255,255));

    area_img1 = videoWidth * videoHeight;
    area_img2 = contourArea(final_points);
    area_over = countNonZero(mask);

    area_percent = area_over / ( area_img1 + area_img2 - area_over );

    return area_percent;
}
