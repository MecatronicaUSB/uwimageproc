/*********************************************************************/
/* File: Undistort.cpp                                               */
/* Last Edition: 28/01/2016, 20:51 PM.                               */
/*********************************************************************/
/* Programmed by:                                                    */
/* Bernardo Aceituno C                                               */
/* Jose Cappelletto                                                  */
/*********************************************************************/
/*Image undistort program, takes the Output .XML and the source as   */
/*inputs and outputs the undistorted source (image, list or video    */
/*Requires OpenCV 2.0.0 or higher                                    */
/*********************************************************************/

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>

#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS
#endif
    
using namespace cv;
using namespace std;

bool readStringList( const string& filename, vector<string>& l){

    cout << "DEBUG: Reading and parsing " << filename << endl;

    l.clear();

    FileStorage fs(filename, FileStorage::READ);
    
    // if file is not open, then exits
    if( !fs.isOpened() ) return false;
    
    //retrieve top level node of XML structure
    FileNode n = fs.getFirstTopLevelNode();
    
    //if not sequential type, exits
    if( n.type() != FileNode::SEQ ) return false;
    
    //retrieve both first and last node through iterator
    FileNodeIterator it = n.begin(), it_end = n.end();
    
    //from first to last node, push it into 'l' string
    for( ; it != it_end; ++it) l.push_back((string)*it);
    
    return true;
}

void error(){
    cout << "Error! wrong or no input provided!" << endl;
    cout << "See -h option for further details" << endl;    
}

void help(){
    cout << "Help: the input arguments shall be entered the following way" << endl;
    cout << "-c : calibration .XML in OpenCV format." << endl;
    cout << "-i : Input source" << endl;
    cout << "     .xml: assumed for imagelist" << endl; 
    cout << "     .avi: for video (testing)" << endl;
    cout << "         : other formats will be assumed for image" << endl; 
    cout << "-o : Output path [optional], if not given set as the input source directory" << endl;
    cout << "-p : Output prefix [optional] if not given set as UND" << endl;
    cout << "-r : video sampling rate [optional] if not given set as 1" << endl;
    cout << "-v : creating a video output [optional]" << endl;
    //cout << "-f : use fisheye camera model [optional]" << endl;
    cout << endl;
    cout << "example: " << endl;
    cout << "$ ./Undistort -c CALIBRATION.xml -i VIDEO.avi -o /home/user/Documents/ -r 5 -p REMAP_ -v" << endl;            
}

int str2int(const string& str) {

    // Still need to add error checking 

    int i = 0;
    string::const_iterator it = str.begin();
    while (it != str.end()) {
        i *= 10;
        i += *it++ - '0';
    }

    return i;
}

int main(int argc, char* argv[]){
    
    //verifies that at least one argument is given
    if(argc < 2){
        error();
        return -1;
    }

    //displays the help if required
    if(string(argv[1])=="-h"){
        help();
        return 0;
    }

    //determines if the two minimum inputs arguments are given
    if(argc < 3){
        error();
        return -1;
    }

    //declares the camera coefficient arrays
    Mat cameraMatrix, distCoeffs;

    //declares the image dimensions
    int width, height;

    //declares the input files
    string CoeffFilename;
    string InputFilename;
    string BasePath;
    int videoyes = 0;
    int fisheyemod = 0;

    //reads the inputs
    int i;
    int fr = 1;
    string argument, prefix, argval;
    
    //reads over each argument to get the inputs
    for(i=1;i<argc - 1;i++){
        //reads the argument
        argument = string(argv[i]);
        //parses the argument
        argval = string(argv[i+1]);
        //verifies the prefix of the argument to assign it
        if(argument == "-i") InputFilename = argval;
        else if(argument == "-c") CoeffFilename = argval;
        else if(argument == "-o") BasePath = argval;
        else if(argument == "-p") prefix = argval;
        else if(argument == "-r") fr = str2int(argval);
        else if(argument == "-v") videoyes = 1;
        //else if(argument == "-f") fisheyemod = 1;
    }

    //checks that the coefficients and the input source are given
    if(InputFilename.empty() || CoeffFilename.empty() || InputFilename.length() == 2 || CoeffFilename.length() == 2){
        cout << "Error! wrong or no input provided!" << endl;
        cout << "Set -h as first parameter for further details" << endl;
        return -1;
    }

    cout << "Input file:\t" << InputFilename << endl;
    cout << "Calib. file:\t" << CoeffFilename << endl;
    
    //gets the path of the input source
    string filelen = InputFilename.substr(InputFilename.find_last_of("/")+1);
    string basepath_input = InputFilename.substr(InputFilename.length(),InputFilename.length() - filelen.length());
    //if there is no output path given the path of the source is taken as output file

    if(BasePath.empty()) BasePath = basepath_input;
    if(prefix.empty()) prefix = "UND_";

    cout << "Ouput prefix:\t" << prefix << endl;

    //opens the coefficients file
    FileStorage f(CoeffFilename,FileStorage::READ); 

    //reads the data form the input
    f["Camera_Matrix"] >> cameraMatrix;
    f["Distortion_Coefficients"] >> distCoeffs;
    f["image_Width"] >> width;
    f["image_Height"] >> height;

    //determines the filetype
    const string filetype = (InputFilename.substr(InputFilename.find_last_of(".")));

    //Computes the compression parameters for the camera
    vector<int> compression_params; //vector that stores the compression parameters of the image
    compression_params.push_back(CV_IMWRITE_JPEG_QUALITY); //specify the compression technique
    compression_params.push_back(100); //specify techniquehe compression quality
    
    cout << "Filetype:\t" << filetype << endl;
    //
    if(filetype == ".xml" || filetype == ".XML") {
        //declares some variables
        vector<string> imageList;

        //reads the imagelist
        bool readed = readStringList(InputFilename, imageList);

        cout << "Provided XML Image List File" << InputFilename << endl;
//        cout << "ImageList" << imageList << endl;

        //checks that the list was oppened succesfully
        if(!readed){
            cout << "Failed to open image list" << endl;
            return -1;
        }

        //declares the matrix for the current image
        Mat image;

        //size of tue image
        Size imageSize;

        //declares a counter for the loop
        int atImageList = 0;

        //reads the first image from the list
        image = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);

        //enters the calibration loop
        while(atImageList < (int)imageList.size()){
            //gets the image size
            imageSize = image.size();
            //declares the outputs
            Mat Output(width, height, CV_16UC3, Scalar(0,50000, 50000));
            
            //remaps the input
            if(fisheyemod) cout << "testing stage" << endl; //fisheye::undistortImage(image, Output, cameraMatrix, distCoeffs);
            else{
                Mat map1, map2;
                initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize, CV_16SC2, map1, map2);
                remap(image, Output, map1, map2, INTER_LINEAR);
            }

            //gets the name of the original file
            string fullpath = imageList[atImageList];

            //reads the extension
            string extension = fullpath.substr(fullpath.find_last_of("."));

            //gets the filename
            string filename = fullpath.substr(fullpath.find_last_of("/") + 1,fullpath.length() - extension.length());

            ostringstream Namefr;
            if(filename.empty()) Namefr << prefix << fullpath;
            else Namefr << BasePath << prefix << filename;

            //saves the undistorted file
            bool save = imwrite(Namefr.str(), Output, compression_params);
  
            //checks if the image was saved correctly;
            if(!(save)){
                cout << "Frame could not be saved" << endl;
                return -1;
            }
            
            cout << "image " << atImageList << " from imagelist done!" << endl; 
            //loads the next image
            image = imread(imageList[atImageList++], CV_LOAD_IMAGE_COLOR);
        }
    }
    else if(filetype == ".avi"){
        
        VideoCapture Video(InputFilename);
        if(!Video.isOpened()){
            cout << "Video could not be opened!" << endl;
            return -1;
        }

        //declares the variable for each frame
        Mat image;

        //size of tue image
        Size imageSize;

        //reads the first fream
        bool bSuccess = Video.read(image);

        //gets the image size
        imageSize = image.size();

        //gets the name of the input
        string videoname = InputFilename.substr(InputFilename.find_last_of("/") + 1,InputFilename.length() - 4);
        
        //creates a video writer
        VideoWriter videowr;

        //creates a new video if requested
        if(videoyes){
            //creates the output name
            ostringstream Out_videoname;
            Out_videoname << BasePath << prefix << videoname << ".avi";
            //initializes the writer
            videowr.open(Out_videoname.str(), CV_FOURCC('P','I','M','1'), 30, imageSize, true);
            if (!videowr.isOpened()){
                cout << "error: failed to write the video" << endl;
                return -1;
            }
        }

        //declares a counter
        int cnt = 0;

        while(!image.empty()){
            if(!bSuccess){
                cout << "error with the video frame" << endl;
                return -1;
            }
            
            //check that the rate is the specified
            if(cnt%fr==0){
                //checks if a video must be written
                if(videoyes){
                    //declares the outputs
                    Mat Output(width, height, CV_16UC3, Scalar(0,50000, 50000));
                    
                    //remaps the input
                    if(fisheyemod) cout << "testing stage" << endl; //fisheye::undistortImage(image, Output, cameraMatrix, distCoeffs);
                    else{
                        Mat map1, map2;
                        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize, CV_16SC2, map1, map2);
                        remap(image, Output, map1, map2, INTER_LINEAR);
                    }

                    videowr.write(Output);
                }
                else{
                    //declares the outputs
                    Mat Output(width, height, CV_16UC3, Scalar(0,50000, 50000));
                    
                    //remaps the input
                    if(fisheyemod) cout << "testing stage" << endl; //fisheye::undistortImage(image, Output, cameraMatrix, distCoeffs);
                    else{
                        Mat map1, map2;
                        initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize, CV_16SC2, map1, map2);
                        remap(image, Output, map1, map2, INTER_LINEAR);
                    }

                    //saves the remaped frame
                    ostringstream namevidefr;
                    namevidefr << BasePath << prefix << videoname << "_" << cnt;
                    
                    bool save = imwrite(namevidefr.str(), Output, compression_params);

                    //checks if the image was saved correctly;
                    if(!(save)){
                        cout << "video frame could not be saved" << endl;
                        return -1;
                    }
                }
            }

            //reads the next image
            bSuccess = Video.read(image);

            //increases the counter
            cnt++;
        }
    }
    else{
        //reads the image
        Mat image = imread(InputFilename,CV_LOAD_IMAGE_UNCHANGED);

        //checks if the image is empty
        if(image.empty()){
            cout << "Error! invalid imagefile" << endl;
            return -1;
        }

        //gets the size of the image
        Size imageSize = image.size();

        //declares the outputs
        Mat Output(width, height, CV_16UC3, Scalar(0,50000, 50000));
        
        //remaps the input
        if(fisheyemod) cout << "testing stage" << endl; //fisheye::undistortImage(image, Output, cameraMatrix, distCoeffs);
        else{
            Mat map1, map2;
            initUndistortRectifyMap(cameraMatrix, distCoeffs, Mat(), getOptimalNewCameraMatrix(cameraMatrix, distCoeffs, imageSize, 1, imageSize, 0), imageSize, CV_16SC2, map1, map2);
            remap(image, Output, map1, map2, INTER_LINEAR);
        }

        string extension = InputFilename.substr(InputFilename.find_last_of("."));
        string imagefilename = InputFilename.substr(InputFilename.find_last_of("/") + 1,InputFilename.length() - extension.length());

        ostringstream outname;

        outname << BasePath << prefix << imagefilename;
        //saves the calibrated frame
        bool save = imwrite(outname.str(), Output, compression_params);

        //checks if it was saved correctly
        if(!(save)){
            cout << "Image could not be saved" << endl;
            return -1;
        }

        cout << "image stored!" << endl;
    }
    
    return 0;
}
