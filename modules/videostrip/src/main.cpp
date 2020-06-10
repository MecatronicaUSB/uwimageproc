/********************************************************************/
/* Project: uwimageproc							*/
/* Module: 	Videostrip						*/
/* File: 	videostrip.cpp                                          */
/* Created:		11/12/2016                                          */
/* Description
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

#include "../include/videostrip.hpp"
#include "../include/options.h"

// #cmakedefine USE_GPU

char keyboard = 0;	// keyboard input character
double t;			// Timing monitor

#if USE_GPU
    GpuMat auxD;    // Auxilary GpuMat to keep track of descriptors
#endif

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
    std::string descriptionString = \
    "videostrip - module part of [uwimageproc] toolbox, for smart extraction of video frames.\
    Employs feature-based homography matrix estimator to calculate the minOverlap among best frames.\
    Frame quality is determined using Laplacian variance, in order to select the best frame that overlaps with previous selected frame";

    argParser.Description(descriptionString);
    argParser.Epilog("Author: J. Cappelletto (GitHub: @cappelletto)\nVisit [https://github.com/mecatronicaUSB] for project information\n");
    argParser.Prog(argv[0]);
    argParser.helpParams.width = 120;

    cout << "\tOpenCV version:\t" << yellow << CV_VERSION << reset << std::endl;
    cout << "\tGit commit:\t" << yellow << GIT_COMMIT << reset << std::endl;
    cout << "\tBuilt:\t" << __DATE__ << " - " << __TIME__ << std::endl;

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

    int CUDA = 0;                                       //Default option (running with CPU)

#ifdef USE_GPU
    cout << "CUDA mode enabled" << std::endl;
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
    float minOverlap = OVERLAP_MIN;	    // desired minOverlap percentage between frames

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
        cout << "minOverlap value provided: " << (minOverlap = args::get(argOverlap)) << endl;
    }
    else{
        cout << "minOverlap default value: " << minOverlap << endl;
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
    cout << "Target minOverlap:\t" << minOverlap << endl;
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
    
    float currOverlap;	//current overlap value
    int out_frame = 0, read_frame = 0;

    // we use the first frame as keyframe (so far, further implementations should include cli arg to pick one by user)
    capture.read(kframe.img);    read_frame ++;
    kframe.new_img = true;
    // resizing for speed purposes
    resize(kframe.img, kframe.res_img, cv::Size(hResizeFactor * kframe.img.cols, hResizeFactor * kframe.img.rows), 0, 0,
           CV_INTER_LINEAR);
    // TODO: This second way to call 'resize' may be faster
    //resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);

    // we save the first keyframe. Using zero padding up to 4 digits for output frames enumeration
    OutputFileName.str("");
    OutputFileName << OutputFile << setfill('0') << setw(4) << out_frame << ".jpg";
    imwrite(OutputFileName.str(), kframe.img);

    // exits when pressed 'ESC' or 'q'
    while (keyboard != 'q' && keyboard != 27) {
        t = (double) getTickCount();
        //read the current frame, if fails, the quit
        if (!capture.read(frame)) {
            cerr << "\nUnable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
        read_frame ++;

        float bestBlur = 0.0, currBlur;    //we start using the current frame blur as best blur value
        resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);
        #if USE_GPU
        if(CUDA)
            currOverlap = calcOverlapGPU(&kframe, res_frame);
        #endif
        if(not CUDA)
            currOverlap = calcOverlap(&kframe, res_frame);

        cout << '\r' << "Frame: " << read_frame << " [" << out_frame << "]\tOverlap: " << currOverlap << std::flush;

	//special case: minOverlap cannot be computed, we force it with an impossible negative value
        // TODO: check better numerically stable way to detect failed minOverlap detection, rather through forced value
		if (currOverlap == -2.0){
            /*cout << endl << "Forcing current new keyframe" << endl;
			keyframe = frame.clone();		
	        resize(keyframe, res_keyframe, cv::Size(), hResizeFactor, hResizeFactor);*/
			currOverlap = OVERLAP_MIN + 0.01;	//by doing this, we may trigger a new keyframe 
		}
		// should we trigger a new keyframe search? TODO: improve this conditional
//		if ((currOverlap <= minOverlap) && (currOverlap > OVERLAP_MIN)) {
		if ((currOverlap <= minOverlap)) {
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

