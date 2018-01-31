# Module: videostrip

/* Project: imageproc								                */
/* Module: 	Videostrip								                */
/* File: 	videostrip.cpp                                          */
/* Created:		11/12/2016                                          */
/* Edited:		30/01/2017, 07:12 PM                                */
/* Description:						                                
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

            "{@input |<none>  | Input video path}"    // input image is the first argument (positional)
                    "{@output |<none> | Prefix for output .jpg images}" // output prefix is the second argument (positional)
                    "{p      |0.95  | Percent of desired overlap between consecutive frames (0.0 to 1.0)}"
                    "{k      |      | Defines window size of k-frames for keyframe tuning}"
                    "{s      | 0    | Skip NN seconds from the start of the video}"
                    "{help h usage ?  |      | show this help message}";      // optional, show help optional

Automatically extract video frames for 2D mosaic generation or 3D model reconstruction. 
Computes frame quality based on Laplacian variance, to select best frame that overlaps with previous selected frame
Overlap of consecutive selected frames is estimated through homography matrix H

Example:
 
$ videostrip -p=0.6 -k=5 -s=12 input.avi vdout_
This will open 'input.avi' file, extract frames with 60% of overlapping, skipping first 12 seconds, and export into 'vdout_XXXX.jpg' images


