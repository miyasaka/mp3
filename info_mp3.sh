#!/bin/bash

FTP_MUSIC_PATH="/home/miyasaka/mp3/Music"
FTP_MUSIC_LIB_HOME=${FTP_MUSIC_PATH}"/*/*/*.mp3"
UPLOAD_ENDS=${FTP_MUSIC_PATH}"/end.txt"

DST_MUSIC_PATH="/var/tmp/test"

PROG="/home/miyasaka/mp3/mp3"

if test -f ${UPLOAD_ENDS}
then
	rm ${UPLOAD_ENDS}
for f in ${FTP_MUSIC_LIB_HOME}
do
	#### echo "aaa ${f}"
	if test -f "${f}"; then
		# xxx/yyyy/Music/[Artist]/[Album]/[songs]
		# -> Music/[Artist]/[Album]/[songs]
		p=`expr "${f}" : ".*\(Music/.*\)"`
		###echo "${f%/*/*}"
		$PROG "${f}" "${p}" 
		if test $? -eq 0; then
			dst="${DST_MUSIC_PATH}/${p%/*}"
			mkdir -p "${dst}"
			cp -p "${f}" "${dst}"
			if test $? -eq 0; then
				echo "`date +"%Y/%m/%d %H:%M:%S"` ${p##*/} -> ${dst}" >>/var/tmp/`date +"%Y%m%d"`_info_mp3.log
				# remove FTP file
				rm "${f}"
				# remove album directory if empty
				rmdir "${f%/*}" 2>/dev/null
				# remove artist directory if empty
				rmdir "${f%/*/*}" 2>/dev/null
			else
				echo "`date +"%Y/%m/%d %H:%M:%S"` ${f} -> ${dst} Copy Failed" >>/var/tmp/`date +"%Y%m%d"`_info_mp3.log
			fi
		else
			echo "`date +"%Y/%m/%d %H:%M:%S"` ${f} -> PostgreSQL Failed" >>/var/tmp/`date +"%Y%m%d"`_info_mp3.log
		fi
	else
		echo "`date +"%Y/%m/%d %H:%M:%S"` ${f} -> Not found mp3 files" >>/var/tmp/`date +"%Y%m%d"`_info_mp3.log
	fi
done
fi

## Get Only File Name
## echo ${f##*/}
##
## Get Only File Name without extension
## echo ${f%.*}
##
## Get Only Directory
## echo ${f%/*}
##
## Get Only extension
## echo ${f##*.}
