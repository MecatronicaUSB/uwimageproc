#!/bin/bash
# 2019-09-11
# Batch processing
# Based on extract_frames.sh v0.5 original script

# if no argument is provided, the print basic usage
if [ -z "$1" ]; then 
	echo Usage: \n
	echo batch_process -i input [-v JPG] [-p output_prefix]
	echo "************************************************************************************************"
	echo "Example: batch_process -i /directory/with/images -v jpg -p IMGOUT_"
	echo -e '\t' "This example will search for 'jpg' files contained at '/directory/with/images' and will apply the specified filter"
	echo -e '\t' "and store them as 'IMGOUT_<original_name>' files."
	exit
fi

#######################################################################################################################
# Parsing method extracted from http://wiki.bash-hackers.org/howto/getopts_tutorial
#######################################################################################################################
PATH_BASE='.'
IMG_FMT='jpg'
#NUM_FOLDERS=1
OUTPUT_FMT='jpg'
NAME_PREFIX="CC_"

while getopts "i:v:p" opt; do
  case $opt in
    i)
	PATH_BASE=$OPTARG 
	;;
    v)
	IMG_FMT=$OPTARG 
	;;
    p)
	echo "NAME PREFIX ARG PROVIDED"
	NAME_PREFIX=$OPTARG 
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
echo -e "File format:\t $FILE_FMT" >&2
#echo -e "Subfolders:\t $NUM_FOLDERS" >&2
#echo -e "Output format:\t $OUTPUT_FMT" >&2
echo -e "Output prefix:\t $NAME_PREFIX" >&2

# Retrieves the list of all video files with $VIDEO_FMT extension

shopt -s nullglob

FILE_LIST=$(find $PATH_BASE -name '*.'$IMG_FMT)

#echo $FILE_LIST

for file in $FILE_LIST; do
	filename=$(basename "$file")	#extract file name with extension
	DIR_PATH=$(dirname "$file")	#extract directory path
	ID=${filename%.$IMG_FMT}		#strip extension for file name, and use it as ID
	FULL_PATH=$DIR_PATH		#construct absolute path
	echo "File found -> $ID.$IMG_FMT"
#	echo "Absolute path -> $FULL_PATH"
#	echo "**************************************************"
	INPUT=$file

	# Now, we check for the '-c' flag, in order to create the required folder
#	OUTPUT=$FULL_PATH"/"$NAME_PREFIX$ID"."$OUTPUT_FMT	
	OUTPUT=$NAME_PREFIX$ID"."$OUTPUT_FMT	
	# TODO: maybe we could check if ffmpeg is installed instead of avconv. Or provide a feature with smart selection between both 	
	COMMAND_LINE="ccorrect -m=L -show=0 -cuda=0 -time=0 $INPUT $OUTPUT"
#	echo $COMMAND_LINE
#	$($COMMAND_LINE)
	
	ccorrect -m=L -show=0 -cuda=0 -time=0 $INPUT $OUTPUT
	
done
