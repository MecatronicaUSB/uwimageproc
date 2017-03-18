/********************************************************************/
/* File: aclahe.cpp                                                 */
/* Last Edition: 30/01/2017, 07:12 PM.                              */
/********************************************************************/
/* Programmed by:                                                   */
/* Jose Cappelletto                                                 */
/********************************************************************/
/* Project: imageproc								                */
/* File: 	aclahe.cpp							                */
/********************************************************************/

//Basic C and c++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>


// OpenCV libraries
#include <opencv2/core.hpp>
#include "opencv2/core/ocl.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/video.hpp>
/*#include <opencv2/features2d.hpp>
#include "opencv2/calib3d.hpp"
#include <opencv2/xfeatures2d.hpp>
#include "opencv2/xfeatures2d/cuda.hpp"*/

//CUDA libraries
#include <opencv2/cudafilters.hpp>
//#include "opencv2/cudafeatures2d.hpp"

//D-LIB Curve fitting
#include <dlib/optimization.h>

//#define _VERBOSE_ON_
#define CL_MIN 0.0
#define CL_MAX 25.0
#define CL_STEP 0.25

using namespace dlib;
using namespace cv;
using namespace cv::cuda;
//using namespace cv::xfeatures2d;
using namespace std;


char keyboard = 0;
// Timing monitor
double t;
// we create a 3-tuple structure to store BlockSize, ClipLimit and Entropy for each CLAHE image
struct _entropy {
    int BS;
    float CL;
    float Entropy;
};

//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Compute Entropy for different Clip Limit and Block Size
//**** 4- Curve fitting to extract curvature
//**** 5- Select breakpoint for Entropy vs CL
//**** 6- Applies CL/BS to image

double calcEntropy(Mat image);

int imgWidth, imgHeight;

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
                    "{display  |      | show the input/output images}"      // optional, show images
                    "{help h usage ?  |      | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, argv, keys);
    cvParser.about("aclahe module v0.1");

    if (argc < 3 || cvParser.has("help")) {
        cout << "Tool to automatically compute Clip Limit and Block Size for CLAHE" << endl;
        cout << "Identifies maximum curvature point for adjusted Entropy vs CL/BS" <<  endl;
        cvParser.printMessage();
        cout << endl << "\tExample:" << endl;
        cout << "\t$ aclahe input.jpg output.jpg " << endl;
        cout <<
        "\tThis will open 'input.jpg' extract best CL/BS values, apply them, and store into 'output.jpg'" <<
        endl << endl;
        return 0;
    }

    String InputFile = cvParser.get<cv::String>(0);
    String OutputFile = cvParser.get<cv::String>(1);
    ostringstream OutputFileName;

    ofstream outfile("out.txt", ios::out);


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

    //**************************************************************************
    /* CUDA */

/*    int nCuda = - 1;    //<Defines number of detected CUDA devices
    nCuda = cuda::getCudaEnabledDeviceCount();
    cout << "Built with OpenCV " << CV_VERSION << endl;
    cuda::DeviceInfo deviceInfo;

    if (nCuda > 0)
        cout << "CUDA enabled devices detected: " << deviceInfo.name() << endl;
    else {
        cout << "No CUDA device detected" << endl;
        cout << "Exiting... use non-GPU version instead" << endl;
    }

    cuda::setDevice(0);
*/

    //**************************************************************************
    /* INPUT IMAGE */
    //**************************************************************************

    cout << "***************************************" << endl;
    cout << "Input: " << InputFile << endl;

    cout << "Reading input file" << endl;

    Mat src = imread(InputFile, IMREAD_GRAYSCALE);
    Mat dst = src.clone();
    Mat tmp = dst.clone();

    if (src.empty()) {
        cout << "Error loading input file (empty resulting matrix" << endl;
    }

/*    imshow("Source image", src);
    waitKey(100);*/

    //**************************************************************************
    /* GAUSSIAN FILTER */
    //**************************************************************************
    GaussianBlur(src, dst, Size(3, 3), 0, 0);

/*    imshow("Source image", dst);
    waitKey(100);*/

    //**************************************************************************
    /* CREATE CL AND BS VECTOR */
    //**************************************************************************
    static int m[] = {2, 4, 8, 16, 32};
    vector<int> BlockSize(m, m + sizeof(m) / sizeof(m[0]));
    vector<float> ClipLimit;

    // Now, we must create a vector as in Matlab [0.0 : 0.25 : 25.0]
    for (float f = CL_MIN; f <= CL_MAX; f += CL_STEP)
        ClipLimit.push_back(f);

    //**************************************************************************
    /* APPLY CLAHE FOR EACH CL/BS COMBINATION */
    //**************************************************************************
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    vector<_entropy> entropy;

    // for speed purpose, we extract BSxBS subimages, and we operate over it
    for (int j = 0; j < BlockSize.size(); j++) {
        cout << "BS: " << BlockSize.at(j) << endl;
        for (int i = 0; i < ClipLimit.size(); i++) {
            //        cout << "CL: " << ClipLimit.at(i) << endl;
            int BS = BlockSize.at(j);
            clahe->setClipLimit(ClipLimit.at(i));
            clahe->setTilesGridSize(Size(BS, BS));
            clahe->apply(dst, tmp);
            //**************************************************************************
            /* COMPUTE ENTROPY FOR EACH CASE */
            //**************************************************************************
            //we store a copy of the current 3-tuple entropy data
            _entropy curr_entropy;
            curr_entropy.BS = BS;
            curr_entropy.CL = ClipLimit.at(i);
            curr_entropy.Entropy = calcEntropy(tmp);
            entropy.push_back(curr_entropy);

            outfile << curr_entropy.Entropy << '\t';
            cout << curr_entropy.Entropy << '\t';
        }
        outfile << endl;
        cout << endl;
    }

    cout << "CL Size" << ClipLimit.size() << endl;

    outfile.close();
    //**************************************************************************
    /* CURVE FITTING */
    //**************************************************************************

    //**************************************************************************
    /* BREAKPOINT DETECTION */
    //**************************************************************************
    /* FINAL CLAHE APPLIED WITH SELECTED CL/BS */
	//*****************************************************************************
    return 0;
}

/*! @fn float calcEntropy (Mat frame)
    @brief Calculates the Entropy for the given frame
    @param frame OpenCV matrix container of the input frame
	@retval The estimated entropy for the given frame
*/
double calcEntropy(Mat image) {
    // Avg time: 0.7 ms GPU/ 23ms CPU
    /*def Entropia(imagen):
    histograma = cv2.calcHist([imagen],[0],None,[256],[0,256])
    histograma = histograma.ravel()/histograma.sum()
    AproxLogs = np.log2(histograma+0.00001)
    entropia = -1 * (histograma*AproxLogs).sum()
    return entropia*/

    MatND hist;
    int channels[] = {0};
    float grange[] = {0.0, 256.0};
    const float *range[] = {grange};
    int histSize[] = {256};
    //computing the histogram for the input image
    calcHist(&image, 1, channels, Mat(), hist, 1, histSize, range, true, false);

    //we compute the sum of all elements (for a given histogram of a MxN matrix, it should give MxN)
    double sum = image.size().width * image.size().height;

    hist = hist / sum;
    Mat aprox_log;
    cv::log(hist + 0.00001, aprox_log);

    double e = -1.0 * cv::sum((hist.mul(aprox_log)))[0];
    return e;
}


