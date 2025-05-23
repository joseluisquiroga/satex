#!/bin/bash

CURR_DIR=$(pwd)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "YLOAD ${1} using "${BASH_SOURCE[0]}

# cd $SCRIPT_DIR

CRY_FILE=${1}
TMP_DIR=${CURR_DIR}/tmp
PRJ_NAME="bs"

if [ ${#CRY_FILE} = 0 ]
then 
	echo "Please provide an argument";
	echo "use: yload.sh <file_nam>";
exit;
fi

if [ ! -e ${CRY_FILE} -o -d ${CRY_FILE} ] 
then 
	echo "Cannot find file "${CRY_FILE}" or it is a directory";
	echo "use: yload.sh <file_nam>";
exit;
fi

if [ ! -e ${TMP_DIR} ] 
then echo "Error. No encontre directorio tmp. Creelo primero.";
echo "use: yload.sh <file_nam>";
exit;
fi

if [ -e ${TMP_DIR} -a ! -d ${TMP_DIR} ] 
then echo "Error. tmp debe ser un directorio.";
echo "use: yload.sh <file_nam>";
exit;
fi

IS_MNT=`sudo df -a | grep none | grep ${TMP_DIR}`

if [ ${#IS_MNT} != 0 -a -d ${TMP_DIR}/${CRY_FILE} ]
then echo "${TMP_DIR} is already mounted as ramfs. Run yunload_sh first.";
exit;
fi

THE_FS=`sudo umount ${TMP_DIR}`

if [ ${#THE_FS} = 0 ]
then sudo mount -t ramfs none ${TMP_DIR};
sudo chown ${USER} ${TMP_DIR}
fi

MNT=`df -a | grep none | grep ${TMP_DIR}`

if [ ${#MNT} = 0 ]
then echo "CANNOT MOUNT RAMFS";
exit;
fi

set -x

rm -rf ${TMP_DIR}/*
cp ${CRY_FILE} ${TMP_DIR}

cd ${TMP_DIR}
echo "CURRENT DIRECTORY="`pwd`
cry3 ${CRY_FILE} -d
rm -f ${CRY_FILE}

IS_GZ=`file ${CRY_FILE}.decry | grep gzip`

if [ ${#IS_GZ} = 0 ]
then echo "Check .decry file. It seems it is not a gzip file.";
exit;
fi

tar xvzf ${CRY_FILE}.decry
rm -f ${CRY_FILE}.decry

if [ ! -d ${PRJ_NAME} ] 
then echo "Did not find ${PRJ_NAME} directory. Can not finish loading !!";
exit;
fi

cd ${PRJ_NAME}
YHOME=`pwd`
echo ${YHOME} > ${HOME}/.yhome.txt
cat ${HOME}/.yhome.txt

set +x

cd ${YHOME}/scripts
sh yfinish_load.sh ${PRJ_NAME}


cd $CURR_DIR
