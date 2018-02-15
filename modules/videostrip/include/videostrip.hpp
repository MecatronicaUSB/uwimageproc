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
//#define _VERBOSE_ON_

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

/// OpenCV libraries. May need review for the final release
#include <opencv2/core.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>

/// CUDA specific libraries
#if USE_GPU
    #include <opencv2/cudafilters.hpp>
    #include "opencv2/cudafeatures2d.hpp"
    #include "opencv2/xfeatures2d/cuda.hpp"
#endif

/// Constant definitios
#define TARGET_WIDTH	640        //< Resized image width
#define TARGET_HEIGHT	480        //< Resized image height
#define OVERLAP_MIN  	0.4        //< Minimum desired minOverlap among consecutive key frames
#define DEFAULT_KWINDOW 11         //< Search window size for best blur-based frame, after new key frame
#define DEFAULT_TIMESKIP 0         //< Search window size for best blur-based frame, after new key frame

// C++ namespaces
using namespace cv;
using namespace cv::cuda;
using namespace cv::xfeatures2d;
using namespace std;

// Structure to save the reference frame data, useful to reuse keypoints and descriptors

typedef struct {
    bool new_img;               // boolean value to know if it has the keypoints data stored
    vector<KeyPoint> keypoints; // keypoints of refererence frame
    Mat descriptors;            // Descriptors of refererence frame
    Mat img;                    // reference frame
    Mat res_img;                // resized frame to TARGET_WIDTH x TARGET_HEIGHT
} keyframe;

/** @brief Obtains the area of the overlap between two frames from their homography matrix

The homography matrix must be previously computed (and validated) using any method of estimation, between an origin image and a reference image. Then it creates a 2D rect polygon representing the boundaries of the origin image, and transforms it according the homography H. Current implementation creates an intersection mask and counts the resulting non-zero elements. A calling example would be:

@code{.cpp}
        Mat H = findHomography(obj, scene, RANSAC);
	if (H.empty())	return -2.0;
        float calcOverlap = overlapArea(H);
   ...
@endcode

@param H cv::Mat containing the homography transformation
@return float The normalized overlap among two given frames
 */
float calcOverlap(keyframe* kframe, Mat image_object);


// See description in function definition
float calcOverlapGPU(keyframe* kframe, Mat image_object);


/*! @fn float calcBlur (Mat frame)
    @brief Calculates the "blur" of a given Mat frame, based on the standard deviation of the Laplacian of the input frame
 
    Applies a Laplacian filter to the input image, and then return its standard deviation as an estimated of the image "blur". It assumes that more blurred images produces a lower value ot stdev(Laplacion(img)), because the Laplacian acts as a simple border detector.

    @param frame cv::Mat container of the input frame
    @retval float The estimated blur for the given frame*/
float calcBlur(Mat frame);


/*! @fn float calcBlurGPU (Mat frame)
    @brief Calculates the "blur" of a given Mat frame using GPU, based on the standard deviation of the Laplacian of the input frame
    @param frame OpenCV matrix container of the input frame
	@retval The estimated blur for the given frame
*/
float calcBlurGPU(Mat frame);


/*! @fn float calcOverlapGPU(keyframe* kframe, Mat img_object)
    @brief Calculates the percentage of overlapping among two frames using GPU, by estimating the Homography matrix.

    Given two images, computes their homography matrix H using SURF features. With H, calls overlapArea(H) to obtain their normalized overlap area. Both images must have enough common features to provide a valid homography matrix

    @param img_scene	keyframe* pointer to current keyframe structure
    @param img_object	cv::Mat container of target frame to be compared against current keyframe
	@brief retval		The normalized overlap among two given frame
*/
float overlapArea(Mat H);

