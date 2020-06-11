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

// #cmakedefine USE_GPU

#if USE_GPU
    extern GpuMat auxD;    // Auxilary GpuMat to keep track of descriptors
#endif


// Video width and height
extern int videoWidth, videoHeight;
// Resize factor from original video frame size to desired TARGET_WIDTH or TARGET_HEIGHT
// This is for fast motion estimation through homography. Perhaps some optical-flow approach could work faster
// Image tiling may improve homography quality by forcing well-spread control points along the image (See CIRS paper)
extern float hResizeFactor;


//TODO: improve names and description of local variables for several specific local-scope use

#if USE_GPU
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

float calcOverlapGPU(keyframe* kframe, Mat img_object) {
	// if any of the input images are empty, then exits with error code
    if (! img_object.data || ! kframe->res_img.data) {
        cout << "calcOverlapGPU: Error reading image data" << std::endl;
        return - 1;
    }
    //-- Step 1: Detect the keypoints using SURF Detector
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
            // take the first result only if its distance is smaller than 0.8*second_best_dist
            // that means this descriptor is ignored if the second distance is bigger or similar
            good_matches.push_back(matches[k][0]);  //push into the good_matches list
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
        cout << "[calcOverlapGPU] Not enough good matches!" << endl;
        //we fail to estimate new overlap, return a non-valid overlap value to signal termination
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
#endif //endif GPU

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
    @param img_scene	Mat OpenCV matrix container of reference frame
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
        //we fail to estimate new minOverlap
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

        // Old minOverlap area calc method ----
        // float dx = fabs(H.at<double>(0, 2));
        // float dy = fabs(H.at<double>(1, 2));
        // float minOverlap = (videoWidth - dx) * (videoHeight - dy) / (videoWidth * videoHeight);
        // ---------------------------------
        
        float minOverlap = overlapArea(H);

#ifdef _VERBOSE_ON_
        t = 1000 * ((double) getTickCount() - t) / getTickFrequency();
        cout << "\t | Homography: " << t << " ms" << endl;
        t = (double) getTickCount();
#endif
        return minOverlap;
    }
}
// TODO: IMPROVE OVERLAP AREA WHEN ASPECT RATIO IS NOT THE SAME
float overlapArea(Mat H){
    vector<Point2f> points, final_points;
    float area_percent, area_currOverlap, area_img1, area_img2;
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
	// TODO: overlap area can be computed faster through geometrical methods using the vertex of the convex polygon
    fillConvexPoly( mask, points_array, 4, Scalar(255,255,255));

    area_img1 = videoWidth * videoHeight;
    area_img2 = contourArea(final_points);
    area_currOverlap = countNonZero(mask);

    //it is supposed that both images have (almost) the same area, so another definition could be area_currOverlap / area_imgRef
    area_percent = area_currOverlap / ( area_img1 + area_img2 - area_currOverlap );

    return area_percent;
}
