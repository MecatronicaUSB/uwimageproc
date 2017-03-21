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
#include <vector>


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
#include "/media/ssd/installs/dlib-19.4/dlib/optimization.h"

//#define _VERBOSE_ON_
#define CL_MIN 0.001
#define CL_MAX 1.0
#define CL_STEP 0.05

using namespace cv;
using namespace cv::cuda;
using namespace std;
using namespace dlib;


char keyboard = 0;
// Timing monitor
double t;
// we create a 3-tuple structure to store BlockSize, ClipLimit and Entropy for each CLAHE image
struct _entropy {
    int BS;
    float CL;
    float Entropy;
};//*/

typedef dlib::matrix<double,1,1> input_vector;
typedef dlib::matrix <double,5,1> parameter_vector;

parameter_vector par_x;

double calcEntropy(Mat image);
double curveModel (const input_vector& input, const parameter_vector& params);
double curveResidual (const std::pair<input_vector, double>& data, const parameter_vector& params);
int curveFitting( std::vector<std::pair<input_vector, double> > data_samples);

int imgWidth, imgHeight;

//**** 1- Parse arguments from CLI
//**** 2- Read input file
//**** 3- Compute Entropy for different Clip Limit and Block Size
//**** 4- Curve fitting to extract curvature
//**** 5- Select breakpoint for Entropy vs CL
//**** 6- Applies CL/BS to image

/*!
	@fn		int main(int argc, char* argv[])
	@brief	Main function
*/
int main(int argc, const char *const *argv) {
//*********************************************************************************
/* PARSER */
    String keys =
            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
                    "{@output |<none> | Prefix for output .jpg images}" // output prefix is the second argument (positional)
                    "{display  |      | show the input/output images}"      // optional, show images
                    "{help h usage ?  |      | show this help message}";      // optional, show help optional

    CommandLineParser cvParser(argc, (const char *const *) argv, keys);
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
//    static int m[] = {8};

//    std::vector<int> BlockSize(m, m + sizeof(m) / sizeof(m[0]));
    std::vector<float> ClipLimit;

    // Now, we must create a vector as in Matlab [0.0 : 0.25 : 25.0]
    /* In OpenCV, ClipLimit is the non-normalized value to limit tileHist[i] array values */
    /* We should feed OpenCV the non-normalized value, but for DLib curve fitting, must be normalized */
    /* The maximum value is given by the number of pixels in the tile (BS*BS)*/
    /* The step can be fixed or relative */
    for (float f = CL_MIN; f <= CL_MAX; f += CL_STEP)
        ClipLimit.push_back(f);

    //**************************************************************************
    /* APPLY CLAHE FOR EACH CL/BS COMBINATION */
    //**************************************************************************
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
   std::vector<_entropy> entropy;

    //we create the conatiner for the data pairs vector
    std::vector<std::pair<input_vector, double> > data_samples;
    input_vector input;

    // for speed purpose, we start with BS = 8x8, while varying CL
    int BlockSize = 8;
    float fEntropy;

    for (int i = 0; i < ClipLimit.size(); i++) {
        //        cout << "CL: " << ClipLimit.at(i) << endl;
        int BS = BlockSize;
        double CL = ClipLimit.at(i);
        clahe->setClipLimit(CL);
        clahe->setTilesGridSize(Size(BS, BS));
        clahe->apply(dst, tmp);
        fEntropy = calcEntropy(tmp);
        //**************************************************************************
        /* COMPUTE ENTROPY FOR EACH CASE */
        //**************************************************************************
        //we store a copy of the current 3-tuple entropy data
        _entropy curr_entropy;
        curr_entropy.BS = BS;
        curr_entropy.CL = ClipLimit.at(i);
        curr_entropy.Entropy = fEntropy;
        entropy.push_back(curr_entropy);

        //now, we push the CL & Entropy into the data vector
        input(0) = CL;
        data_samples.push_back(make_pair(input, fEntropy));

        //outfile << CL << '\t' << fEntropy << endl;
        cout << curr_entropy.Entropy << '\t';
    }
    cout << endl;

/*    cout << "CL Vector Size: " << ClipLimit.size() << endl;
    cout << "Data Matrix Size: " << data_samples.size() << endl;*/
    //**************************************************************************
    /* CURVE FITTING */
    //**************************************************************************
	//Now we proceed to adjust curve to obtained data, so we can identify maximum curvature point (breakpoint)
    curveFitting(data_samples);

    //HERE, WE EXPORT DE DATA_SAMPLES TO CHECK TRAINING
    for (int g=0; g < data_samples.size(); g++){
        input(0) = (double) data_samples.at(g).first;
        outfile << input(0) << '\t' << (double) data_samples.at(g).second << '\t';
        outfile << (double) curveModel(input, par_x) << endl;
    }
    outfile.close();
    //**************************************************************************
    /* BREAKPOINT DETECTION */
    //**************************************************************************

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

    double _entropy = -1.0 * cv::sum((hist.mul(aprox_log)))[0];
    return _entropy;
}

/*! @fn float curveModel (const input_vector& input, const parameter_vector& params)
    @brief Target curve model to be fitted with Entropy vs CL data
    @param input vector containing the input data
    @param params vector containing function parameters c1, c2, l1, l2 to be optimized
	@retval The output for the target curve being fitted
*/
double curveModel (const input_vector& input, const parameter_vector& params){
    // Here are the 4 parameters to be optimized while fitting the exponential function
    const double a = params(0);
    const double b = params(1);
    const double c = params(2);
    const double d = params(3);
    const double e = params(4);
    // This is the single input 'x' for our fitting curve
    const double x = input(0);

    const double output = a*exp(-b*x) + c*exp(-d*x);
//    const double output = a*log(e*x*x + b*x + c) + d;
//    const double output = a + b*x + c*x*x + d*x*x*x + e*x*x*x*x;

    return output;
}

/*! @fn float curveResidual (const std::pair<input_vector, double>& data, const parameter_vector& params)
    @brief Computes the amount of error between the model and the expected output
    @param data vector containing the data input and output pair
    @param params vector containing function parameters c1, c2, l1, l2 to be optimized, to be fed to the curveModel
	@retval The output error
*/
double curveResidual (const std::pair<input_vector, double>& data, const parameter_vector& params){
    return curveModel(data.first, params) - data.second;
}


int curveFitting( std::vector<std::pair<input_vector, double> > dacata_samples){
    try{

        //generate a random seed parameters vector to start. If we know a better start seed, we could include it
        const parameter_vector params = 4*randm(5,1);
//        cout << "Params: " << trans(params) << endl;

        //now, we must generate the input/output pairs according to our model, and the CL vs Entropy data
        par_x = 1;
        cout << "Use Levenberg-Marquardt, approximate derivatives" << endl;
        // If we didn't create the residual_derivative function then we could
        // have used this method which numerically approximates the derivatives for you.
        solve_least_squares_lm(objective_delta_stop_strategy(1e-7).be_verbose(),
                               curveResidual,
                               derivative(curveResidual),
                               data_samples,
                               par_x);

        // Now x contains the solution.  If everything worked it will be equal to params.
        cout << "inferred parameters: "<< trans(par_x) << endl;
        cout << "solution error:      "<< length(par_x - params) << endl;
        cout << endl;
    }

    catch (std::exception& e)
    {
        cout << e.what() << endl;
    }
    return 0;
}