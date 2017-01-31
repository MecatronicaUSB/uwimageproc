#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video.hpp"
//C
#include <stdio.h>
//C++
#include <iostream>
#include <sstream>

using namespace cv;
using namespace std;

char keyboard;

//1- Parse arguments from CLI
//2- Read input file
//3- Start extraccting frames
//4- Select 1st key frame
//5- Extract following frames
//	5.1- Compute Homography matrix
//	5.2- Estimate overlapping of current frame with previous keyframe
//	5.3- If falls below treshold, pick best quality frame in the neighbourhood
//	5.4- Asign it as next keyframe
//	5.5 Repeat from 5


int main(int argc, char* argv[])
{
	cout << "Tool to strip automatically extract frames for 2D mosaic generation" << endl;
	cout << "Computes frame quality based on Laplacian variance, to select best frame that overlaps with previous selected frame" << endl;
	cout << "Overlap of consecutive selected frames is estimated through homography matrix H" << endl;  
    cout << "Built with OpenCV " << CV_VERSION << endl;

    Mat frame; // currently extracted frame
	Mat image; // current work image
	char *videoFilename; // string containing the name of the video to be processed

	FILE *apOutFile;

	videoFilename = argv[1];	// we retrieve the input video file name from CLI args

    //create the capture object
    VideoCapture capture(videoFilename);
    if(!capture.isOpened()){
        //error in opening the video input
        cerr << "Unable to open video file: " << videoFilename << endl;
        exit(EXIT_FAILURE);
    }

	apOutFile=fopen ("out.txt", "w+");	

    //read input data. ESC or 'q' for quitting
    keyboard = 0;
	bool bImagen = false;
    while( keyboard != 'q' && keyboard != 27 ){
        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        //get the input from the keyboard
        keyboard = (char)waitKey( 5 );
		if (keyboard == ' ') bImagen = !bImagen;

        Laplacian(frame,image,CV_16S, 7);

        Scalar mean, stdev;
        meanStdDev(image,mean,stdev);
        double m = mean.val[0];
        double s = stdev.val[0];

		fprintf (apOutFile, "%3f\t%3f\n", m, s);
        //get the frame number and write it on the current frame
        stringstream ss,tt;
        rectangle(frame, cv::Point(10, 2), cv::Point(300,20),
                  cv::Scalar(255,255,255), -1);
        ss << "F: " << capture.get(CAP_PROP_POS_FRAMES) << "\tstd: " << s;
        string frameNumberString = ss.str();
        putText(frame, frameNumberString.c_str(), cv::Point(15, 15),
                FONT_HERSHEY_SIMPLEX, 0.5 , cv::Scalar(0,0,0));
        //show the current frame and the fg masks

		if (bImagen)
	        imshow("Frame", image);
		else
	        imshow("Frame", frame);
    }
    //delete capture object
    capture.release();
	fclose (apOutFile);
	cout << "Exit";
    return 0;
}


