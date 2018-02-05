/**
 * @file preprocessing.h
 * @brief Functions for pre-processing and enhanced images
 * @version 1.0
 * @date 20/01/2018
 * @author Victor Garcia
 * @author José Cappelletto
 */
#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

/**
 * @brief Computes the intensity distribution histograms for the three channels
 * @function getHistogram(cv::Mat img, int histogram[3][256])
 * @param img OpenCV Matrix container input image
 * @param histogram Integer matrix to store the histogram
 */
void getHistogram(cv::Mat img, int histogram[256]);
// Histogram[256] has been modified into a single dimension 256-elements vector, as it will operate in a single channel


// TODO: Perhaps this function will be deprecated, or just kept back for visualization purposes (discuss it)
/**
 * @brief Creates an image that represents the Histogram
 * @function printHistogram(int histogram[256], std::string filename, cv::Scalar color)
 * @param histogram Integer array correspond to a histogram
 * @param filename name of output file to print the histogram
 * @param color OpenCV Scalar containing the color of bars in histogram
 */
void printHistogram(int histogram[256], std::string filename, cv::Scalar color);

/**
 * @brief Transform imgOriginal so that, for each channel histogram, its
          lowerPercentile and higherPercentile values are moved to 0 and 255, respectively
 * @function colorChannelStretch(cv::Mat imgOriginal, cv::Mat imgStretched, int lowerPercentile, int higherPercentile)
 * @param imgOriginal OpenCV Matrix containing input image
 * @param imgStretched OpenCV Matrix to store the stretched output image
 * @param lowerPercentile Percentile to trunk the lower values
 * @param higherPercentile Percentile to trunk the higher values
 * \n
 * \b CONSTRAINTS: \n
 * \e imgOriginal and \e imgStretched must have the same dimensions.\n
 * \e lowerPercentile and \e higherPercentle must be integers between
 * 0 and 100.\n
 * \e lowerPercentile must be smaller than \e higherPercentile
 */
void imgChannelStretch(cv::Mat imgOriginal, cv::Mat imgStretched, int lowerPercentile=0, int higherPercentile=100);
// Transform imgOriginal so that, for each channel histogram, its
// lowerPercentile and higherPercentile values are moved to 0 and 255,
// respectively. Values in between are linearly scaled. Values smaller
// than lowerPercentile are set to 0, and values greater than
// higherPercentle are set to 1. The resulting image is saved in
// imgStretched.
// CONSTRAINTS:
//      * imgOriginal and imgStretched must have the same dimensions.
//      * lowerPercentile and higherPercentle must be integers between
//        0 and 100, and lowerPercentile must be smaller than
//        higherPercentile

#endif
