#!/bin/bash
# 2015-09-07
# List/group files into 'group_number' groups
# and move into subfolders
# v 0.2

# if no argument is provided, the print basic usage
if [ -z "$1" ]; then 
	echo usage: $0 path_to_images [number_groups]
        exit
fi

GROUP_NUMBER=$2

if [ -z "$2" ]; then 
	echo "Using default value for number_groups: 10"
        GROUP_NUMBER=10
fi

# path to files
PATH_BASE=$1

shopt -s extglob

# retrieve the NUMEBR of ".jpg" files
list=$(ls -1 -tr $PATH_BASE*.jpg | wc -l)
echo "# Files: " $list
files_per_group=$(($list/$GROUP_NUMBER))
files_excess=$(($list%$GROUP_NUMBER))
echo "# Groups: " $GROUP_NUMBER
echo "Files/Group: " $files_per_group
echo "Files in excess: " $files_excess	
echo "................."

for (( i=1; i<=$GROUP_NUMBER; i++ ))
do
	var=$(printf "%02d" $i)
	echo "Creating directory $var/$GROUP_NUMBER"
	$(mkdir $PATH_BASE$var)
	files_this_group=$files_per_group
	if (($i<=$files_excess)); then
		files_this_group=$(($files_per_group+1))
	fi
	echo "Files in this directory:" $files_this_group
	
	# now, we proceed to move 'files_this_group' files in each subdirectory
	# retrieve the list of TOP 'files_this_group' ".jpg" files
	list=$(ls -1 -tr $PATH_BASE*.jpg | head -n$files_this_group)
	for file in $list; do
		$(mv $file $PATH_BASE$var)
	done
done

