#!/bin/bash
# 2015-10-26
# List all image files on specified path, and store it in OpenCV XML compatible file format
# Improved argument parsing
# v 0.2

# if no argument is provided, the print basic usage
if [ -z "$1" ]; then 
	echo Usage: \n
	echo list_images -i search_path [-o output_file] [-f file_extension]
	echo "************************************************************************************************"
	echo "Example: list_images -i /directory/with/images -o List.txt -f jpg"
	echo -e '\t' "This example will search for 'jpg' files contained at '/directory/with/images' and will store"
	echo -e '\t' "as an OpenCV XML compatible file list"
	echo -e '\t' "Visit http://docs.opencv.org/2.4/doc/tutorials/calib3d/camera_calibration/camera_calibration.html#results"
	exit
fi

#######################################################################################################################
# Parsing method extracted from http://wiki.bash-hackers.org/howto/getopts_tutorial
#######################################################################################################################

SEARCH_PATH='.'
FILE_EXT='jpg'
OUTPUT_FILE='ImageList.xml'
CREATE_FOLDER=false

while getopts "i:o:f:" opt; do
  case $opt in
    i)
	SEARCH_PATH=$OPTARG 
	;;
    o)
	OUTPUT_FILE=$OPTARG 
	;;
    f)
	FILE_EXT=$OPTARG 
	;;
    \?)
	echo "Invalid option: -$OPTARG" >&2
	exit 1
	;;
    :)
	echo "Option -$OPTARG requires an argument." >&2
	exit 1
	;;
  esac
done

echo -e "Search path:\t $SEARCH_PATH" >&2
echo -e "Ouput file:\t $OUTPUT_FILE" >&2
echo -e "File extension:\t $FILE_EXT" >&2

FILE_EXT=*.$FILE_EXT

shopt -s extglob

$(echo '<?xml version="1.0"?>' > $OUTPUT_FILE)
$(echo '<opencv_storage>' >> $OUTPUT_FILE)
$(echo '<images>' >> $OUTPUT_FILE)

$(ls -R -1 -d $SEARCH_PATH/$FILE_EXT >> $OUTPUT_FILE)

$(echo '</images>' >> $OUTPUT_FILE)
$(echo '</opencv_storage>' >> $OUTPUT_FILE)
		

