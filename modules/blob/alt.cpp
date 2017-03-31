int main( int argc, char** argv ){

    cv::Mat objects, distance,peaks,results;
    std::vector<std::vector<cv::Point> > contours;

    objects=cv::imread("CUfWj.jpg");
    objects.copyTo(results);
    cv::cvtColor(objects, objects, CV_BGR2GRAY);
    //THIS IS THE LINE TO BLUR THE IMAGE CF COMMENTS OF THIS POST
    cv::blur( objects,objects,cv::Size(3,3));
    cv::threshold(objects,objects,125,255,cv::THRESH_BINARY_INV);


    /*Applies a distance transform to "objects".
     * The result is saved in "distance" */
    cv::distanceTransform(objects,distance,CV_DIST_L2,CV_DIST_MASK_5);

    /* In order to find the local maxima, "distance"
     * is subtracted from the result of the dilatation of
     * "distance". All the peaks keep the save value */
    cv::dilate(distance,peaks,cv::Mat(),cv::Point(-1,-1),3);
    cv::dilate(objects,objects,cv::Mat(),cv::Point(-1,-1),3);

    /* Now all the peaks should be exactely 0*/
    peaks=peaks-distance;

    /* And the non-peaks 255*/
    cv::threshold(peaks,peaks,0,255,cv::THRESH_BINARY);
    peaks.convertTo(peaks,CV_8U);

    /* Only the zero values of "peaks" that are non-zero
     * in "objects" are the real peaks*/
    cv::bitwise_xor(peaks,objects,peaks);

    /* The peaks that are distant from less than
     * 2 pixels are merged by dilatation */
    cv::dilate(peaks,peaks,cv::Mat(),cv::Point(-1,-1),1);

    /* In order to map the peaks, findContours() is used.
     * The results are stored in "contours" */
    cv::findContours(peaks, contours, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    /* The next steps are applied only if, at least,
     * one contour exists */
    cv::imwrite("CUfWj2.jpg",peaks);
    if(contours.size()>0){

        /* Defines vectors to store the moments of the peaks, the center
         * and the theoritical circles of the object of interest*/
        std::vector <cv::Moments> moms(contours.size());
        std::vector <cv::Point> centers(contours.size());
        std::vector<cv::Vec3f> circles(contours.size());
        float rad,x,y;
        /* Caculates the moments of each peak and then the center of the peak
         * which are approximatively the center of each objects of interest*/

        for(unsigned int i=0;i<contours.size();i++) {
            moms[i]= cv::moments(contours[i]);
            centers[i]= cv::Point(moms[i].m10/moms[i].m00,moms[i].m01/moms[i].m00);
            x= (float) (centers[i].x);
            y= (float) (centers[i].y);
            if(x>0 && y>0){
                rad= (float) (distance.at<float>((int)y,(int)x)+1);
                circles[i][0]= x;
                circles[i][3]= y;
                circles[i][2]= rad;
                cv::circle(results,centers[i],rad+1,cv::Scalar( 255, 0,0 ), 2, 4, 0 );
            }
        }
        cv::imwrite("CUfWj2.jpg",results);
    }

    return 1;
}
