#!/bin/bash
# 2015-09-03
# Automatic frame extraction for video based 3D reconstruction
# List, moves, and strip TOD/MPG files into jpg frames at user-specified frame rate
# Merges functionality from 'split_files.sh', and includes 'find' for video listing
# v 0.5

# if no argument is provided, the print basic usage
if [ -z "$1" ]; then 
	echo Usage: \n
	echo extract_frames -i input [-v video_format] [-r frame_rate] [-n num_subfolders] [-c] [-f output_format] [-p name_prefix]
	echo "************************************************************************************************"
	echo "Example: extract_frames -i /directory/with/videos -v mpg -r 5 -n 4 -f jpg"
	echo -e '\t' "This example will search for 'mpg' files contained at '/directory/with/videos' and will extract"
	echo -e '\t' "frames at a rate of 5 fps, and stored as 'jpg' files. Those frames will be stored into 5 subfolders"
	echo -e '\t' "When the '-c' flag is activated, the script will create a subfolder which will contain the resulting images"
	exit
fi

#######################################################################################################################
# Parsing method extracted from http://wiki.bash-hackers.org/howto/getopts_tutorial
#######################################################################################################################
PATH_BASE='.'
VIDEO_FMT='TOD'
FRAME_RATE=4
#NUM_FOLDERS=1
OUTPUT_FMT='jpg'
CREATE_FOLDER=false

while getopts "i:r:v:f:c:p" opt; do
  case $opt in
    i)
	PATH_BASE=$OPTARG 
	;;
    v)
	VIDEO_FMT=$OPTARG 
	;;
    r)
	FRAME_RATE=$OPTARG 
	;;
#    n)
#	NUM_FOLDERS=$OPTARG 
#	;;
    f)
	OUTPUT_FMT=$OPTARG 
	;;
    p)
	NAME_PREFIX=$OPTARG 
	;;
    c)
	CREATE_FOLDER=true
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

echo -e "Input path:\t $PATH_BASE" >&2
echo -e "Video format:\t $VIDEO_FMT" >&2
echo -e "Frame rate:\t $FRAME_RATE" >&2
#echo -e "Subfolders:\t $NUM_FOLDERS" >&2
echo -e "Output format:\t $OUTPUT_FMT" >&2

# Retrieves the list of all video files with $VIDEO_FMT extension

shopt -s nullglob

FILE_LIST=$(find $PATH_BASE -name '*.'$VIDEO_FMT)

echo $FILE_LIST

for file in $FILE_LIST; do
	filename=$(basename "$file")	#extract file name with extension
	DIR_PATH=$(dirname "$file")	#extract directory path
	ID=${filename%.TOD}		#strip extension for file name, and use it as ID
	FULL_PATH=$DIR_PATH		#construct absolute path
	echo "Video found -> $ID.$VIDEO_FMT"
#	echo "Absolute path -> $FULL_PATH"
	echo "**************************************************"
	INPUT=$file

	# Now, we check for the '-c' flag, in order to create the required folder
	OUTPUT=$FULL_PATH"/"$NAME_PREFIX$ID"_%4d."$OUTPUT_FMT
	if ($CREATE_FOLDER); then
		echo "Creating directory "$ID
		mkdir $FULL_PATH'/'$ID

		# Finally we must modify the $OUTPUT path in order to include the folder
		OUTPUT=$FULL_PATH"/$ID/"$ID"_%4d."$OUTPUT_FMT
	fi
	
	# TODO: maybe we could check if ffmpeg is installed instead of avconv. Or provide a feature with smart selection between both 	
	$(avconv -i $INPUT -filter:v yadif -r $FRAME_RATE -f image2 -q 1.0 $OUTPUT)
	
done
