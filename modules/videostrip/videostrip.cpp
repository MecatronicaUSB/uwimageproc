/********************************************************************/
/* File: template.cpp                                                 */
/* Last Edition: 30/01/2017, 07:12 PM.                              */
/********************************************************************/
/* Programmed by:                                                   */
/* Jose Cappelletto                                                 */
/********************************************************************/
/* Project: imageproc								                */
/* File: 	videostrip.cpp							                */
/********************************************************************/

//Basic C and c++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <stdlib.h>


// OpenCV libraries
#include <opencv2/core.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>
#include "opencv2/xfeatures2d/cuda.hpp"

//CUDA libraries
#include <opencv2/cudafilters.hpp>
#include "opencv2/cudafeatures2d.hpp"

#define TARGET_WIDTH    640        //< Resized image width
#define TARGET_HEIGHT    480        //< Resized image height
#define OVERLAP_MIN        0.4        //< Minimum desired overlap among consecutive key frames
#define DEFAULT_KWINDOW 10        //< Search window size for best blur-based frame, after new key frame

using namespace cv;
using namespace cv::cuda;
using namespace cv::xfeatures2d;
using namespace std;

char keyboard=0;

// Timing monitor
double t;

//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Start extraccting frames
//**** 4- Select 1st key frame
//**** 5- Extract following frames
//****	5.1- Compute Homography matrix
//****	5.2- Estimate overlapping of current frame with previous keyframe
//	5.3- If it falls below treshold, pick best quality frame in the neighbourhood
//****	5.4- Asign it as next keyframe
//****	5.5 Repeat from 5

float calcOverlap(Mat image_scene, Mat image_object);
float calcBlur(Mat frame);

int videoWidth, videoHeight;
float hResizeFactor;

/*!
	@fn		int main(int argc, char* argv[])
	@brief	Main function
*/
int main(int argc, char *argv[]) {
//*********************************************************************************
/* PARSER */
    String keys =
            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
                    "{@output |<none> | Prefix for output .jpg images}" // output prefix is the second argument (positional)
                    "{p      |0.95  | Percent of desired overlap between consecutive frames (0.0 to 1.0)}"
                    "{k      |      | Defines window size of k-frames for keyframe tuning}"
                    "{help h usage ?  |      | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about("videostrip module v0.3");

    if (argc < 3 || cvParser.has("help")) {
        cout << "Tool to automatically extract frames from video for 2D mosaic generation" << endl;
        cout <<
        "Computes frame quality based on Laplacian variance, to select best frame that overlaps with previous selected frame" <<
        endl;
        cout << "Overlap of consecutive selected frames is estimated through homography matrix H" << endl;
        cvParser.printMessage();
        cout << endl << "\tExample:" << endl;
        cout << "\t$ videostrip -p 0.6 input.avi vdout_" << endl;
        cout <<
        "\tThis will open 'input.avi' file, extract frames with 60% of overlapping and export into 'vdout_XXXX.jpg' images" <<
        endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);
    String OutputFile = cvParser.get<cv::String>(1);
    ostringstream OutputFileName;

    int kWindow = cvParser.get<int>("k");
    float overlap = cvParser.get<float>("p");

    if (!cvParser.check()) {
        cvParser.printErrors();
        return -1;
    }

    //************************************************************************
    /* FILENAME */
    //gets the path of the input source
    string FileName = InputFile.substr(InputFile.find_last_of("/") + 1);
    string BasePath = InputFile.substr(0, InputFile.length() - FileName.length());

    //determines the filetype
    string FileType;
    if (InputFile.find_last_of(".") == -1) // DOT (.) not found, so filename doesn't contain extension
        FileType = "";
    else
        FileType = InputFile.substr();

    // now we build the FileBase from input FileName
    string FileBase = FileName.substr(0, FileName.length() - FileType.length());

    //**************************************************************************
    /* CUDA */
    int nCuda = -1;    //<Defines number of detected CUDA devices

    nCuda = cuda::getCudaEnabledDeviceCount();
    cout << "Built with OpenCV " << CV_VERSION << endl;
    cuda::DeviceInfo deviceInfo;

    if (nCuda > 0)
        cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
    else
        cout << "No CUDA device detected" << endl;

    cuda::setDevice(0);
    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;

    //**************************************************************************
    /* VIDEO INPUT */

    //create the capture object
    VideoCapture capture(InputFile);
    if (!capture.isOpened()) {
        //error in opening the video input
        cerr << "Unable to open video file: " << InputFile << endl;
        exit(EXIT_FAILURE);
    }
    //now we retrieve and print info about input video
    videoWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
    videoHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

    // we compute the resize factor for the horizontal dimension. As we preserve the aspect ratio, is the same for the veritcal resizing
    hResizeFactor = (float) TARGET_WIDTH / videoWidth;

    float   videoFPS = capture.get(CV_CAP_PROP_FPS);
    int     videoFrames = capture.get(CV_CAP_PROP_FRAME_COUNT);

    cout << "Metadata:" << endl;
    cout << "\tSize:\t" << videoWidth << " x " << videoHeight << endl;
    cout << "\tFrames: " << videoFrames << " @ " << videoFPS << endl;
    cout << "\thResize: " << hResizeFactor << endl;

    cout << "Target overlap: " << overlap << endl;
    cout << "K-Window size: " << kWindow << endl;

    //**************************************************************************
    /* PROCESS START */
    // Next, we start reading frames from video
    Mat frame(videoWidth, videoHeight, CV_8UC1);
    Mat keyframe, bestframe, res_keyframe, res_frame;

    bool bImagen = false;

    // we use the first frame as keyframe (so far, further implementations should include cli arg to pick one by user)
    capture.read(keyframe);
    // resizing for speed purposes
    resize(keyframe, res_keyframe, cv::Size(hResizeFactor * keyframe.cols, hResizeFactor * keyframe.rows), 0, 0,
           CV_INTER_LINEAR);

    //int g = 0;
    float over;
    int out_frame = 0, g = 0;

    OutputFileName.str("");
    OutputFileName << OutputFile << setfill('0') << setw(4) << out_frame << ".jpg";
    imwrite(OutputFileName.str(), keyframe);

    while (keyboard != 'q' && keyboard != 27) {
        t = (double) getTickCount();
        //read the current frame, if fails, the quit
        if (!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }
//		cvtColor(frame,frame,COLOR_BGR2GRAY);
        float bestBlur = 0.0, currBlur;    //we start using the current frame blur as best blur value yet

        resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);
        over = calcOverlap(res_keyframe, res_frame);
        cout << '\r' << "Frame: " << g++ << "\tOverlap: " << over << std::flush;

        if ((over < overlap) && (over > OVERLAP_MIN)) {
            cout << "\t>> Overlap threshold, refining for Blur" << endl;
            /*!
            Start to search best frames in i+k frames, according to "blur level" estimator (based on Laplacian variance)
            We start using current frame as best frame so far
            */
            bestBlur = calcBlur(res_frame);
            bestframe = frame.clone();
            //for each frame inside the k-consecutive frame window, we refine the search
            for (int n = 0; n < kWindow; n++) {
                capture.read(frame);    //we capture a new frame
                resize(frame, res_frame, cv::Size(), hResizeFactor, hResizeFactor);    //uses a resized version
                currBlur = calcBlur(res_frame);    //we operate over the resampled image for speed purposes

                cout << '\r' << ">> Frame: " << n << "\tBlur: " << currBlur << "\tBest: " << bestBlur << std::flush;
                if (currBlur > bestBlur) {    //if current blur is better, replaces best frame
                    bestBlur = currBlur;
                    bestframe = frame.clone();
                }
            }

            keyframe = bestframe.clone();
            cout << " >> best frame found";
            out_frame++;
            OutputFileName.str("");
            OutputFileName << OutputFile << setfill('0') << setw(4) << out_frame << ".jpg";
            cout << "  >> storing best frame... ";
            imwrite(OutputFileName.str(), bestframe);

            t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
//            cout << endl << "BestBlur: " << t << " ms" << endl;
            t = (double) getTickCount();

            resize(keyframe, res_keyframe, cv::Size(), hResizeFactor, hResizeFactor);
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
    @brief Calculates the "blur" of given \a Mat frame, based on the standard deviation of the Laplacian of the input fram
    @param frame OpenCV matrix container of the input frame
	@retval The estimated blur for the given frame
*/
float calcBlur(Mat frame) {
    // Avg time: 0.7 ms GPU/ 23ms CPU
    Mat grey, laplacian;
    cuda::GpuMat gpuFrame, gpuLaplacian;
    cvtColor(frame, grey, COLOR_BGR2GRAY);

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
    double m = mean.val[0];
    double s = stdev.val[0];
    // we return the standar deviation
    return s;
}

/*! @fn float calcOverlap(Mat img_scene, Mat img_object)
    @brief Calculates the percentage of overlapping among two frames, by estimating the Homography matrix.
    @param
            img_scene	Mat OpenCV matrix container of reference frame
    @param img_object	Mat OpenCV matrix container of target frame
	@brief retval		The normalized overlap among two given frame
*/
float calcOverlap(Mat img_scene, Mat img_object) {
    if (!img_object.data || !img_scene.data) {
        cout << " --(!) Error reading images " << std::endl;
        return -1;
    }

    //-- Step 1: Detect the keypoints using SURF Detector
    int minHessian = 300;
    Mat descriptors_object, descriptors_scene;
    vector<KeyPoint> keypoints_object, keypoints_scene;

    cuda::GpuMat gpu_img_object, gpu_img_scene;
    cuda::GpuMat gpu_keypoints_object, gpu_keypoints_scene;
    cuda::GpuMat gpu_descriptors_object, gpu_descriptors_scene;

    cvtColor(img_object, img_object, COLOR_BGR2GRAY);
    cvtColor(img_scene, img_scene, COLOR_BGR2GRAY);

    gpu_img_object.upload(img_object);
    gpu_img_scene.upload(img_scene);

    cuda::SURF_CUDA surf(minHessian);
    surf(gpu_img_object, cuda::GpuMat(), gpu_keypoints_object, gpu_descriptors_object);
    surf(gpu_img_scene, cuda::GpuMat(), gpu_keypoints_scene, gpu_descriptors_scene);//*/

    //-- Step 3: Matching descriptor vectors using BruteForceMatcher
    surf.downloadKeypoints(gpu_keypoints_scene, keypoints_scene);
    surf.downloadKeypoints(gpu_keypoints_object, keypoints_object);

    t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
    cout << endl << "SURF@GPU: " << t << " ms " ;
    t = (double) getTickCount();

    //***************************************************************//
    //-- Step 3: Matching descriptor vectors using GPU BruteForce matcher (instead CPU FLANN)
    // Avg time: 2.5 ms GPU / 21 ms CPU
    double max_dist = 0;
    double min_dist = 100;

    Ptr<cuda::DescriptorMatcher> matcher_gpu = cuda::DescriptorMatcher::createBFMatcher();
    vector<vector<DMatch> > matches_gpu;
    matcher_gpu->knnMatch(gpu_descriptors_object, gpu_descriptors_scene, matches_gpu, 2);

    //-- Step 4: Select only good matches
    std::vector<DMatch> good_matches_gpu;
    for (int k = 0; k < std::min(keypoints_object.size() - 1, matches_gpu.size()); k++) {
        if ((matches_gpu[k][0].distance < 0.6 * (matches_gpu[k][1].distance)) &&
            ((int) matches_gpu[k].size() <= 2 && (int) matches_gpu[k].size() > 0)) {
            // take the first result only if its distance is smaller than 0.6*second_best_dist
            // that means this descriptor is ignored if the second distance is bigger or of similar
            good_matches_gpu.push_back(matches_gpu[k][0]);
        }
    }

    t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
    cout << "\t | BFMatcher GPU: " << t << " ms ";
    t = (double) getTickCount();

    //***************************************************************//
    //we must check if found H matrix is good enough. It requires at least 4 points
    if (good_matches_gpu.size() < 4) {
        cout << "[WARN] Not enough good matches!" << endl;
        //we fail to estimate new overlap
        return 0;
    }
    else {
        //-- Localize the object
        vector<Point2f> obj;
        vector<Point2f> scene;

        for (int i = 0; i < good_matches_gpu.size(); i++) {
            //-- Get the keypoints from the good matches
            obj.push_back(keypoints_object[good_matches_gpu[i].queryIdx].pt);
            scene.push_back(keypoints_scene[good_matches_gpu[i].trainIdx].pt);
        }

        // TODO: As OpenCV 3.2, there is no CUDA based implementation for findHomography.
        // Check http://nghiaho.com/?page_id=611 for an external solution
        // Avg time: 0.7 ms CPU
        Mat H = findHomography(obj, scene, RANSAC);
        float dx = fabs(H.at<double>(0, 2));
        float dy = fabs(H.at<double>(1, 2));

        float overlap = (videoWidth - dx) * (videoHeight - dy) / (videoWidth * videoHeight);
        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        cout << "\t | Homography: " << t << " ms" << endl;
        t = (double) getTickCount();
        return overlap;
    }
}
