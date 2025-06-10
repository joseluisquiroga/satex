#!/bin/bash

CURR_DIR=$(pwd)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "YUNLOAD ${1} using "${BASH_SOURCE[0]}

PRM=${1}

# cd $SCRIPT_DIR

YHOME=`cat ${HOME}/.yhome.txt`

if [ ${#YHOME} = 0 ]
then echo "YHOME=${YHOME} is not set";
exit;
fi

CRY_FILE=`ls -t ${YHOME}/../../*.txt | head -n 1`
echo "CRY_FILE="${CRY_FILE}

${YHOME}/scripts/yclean_sh

BASE=`basename ${YHOME}`
UPTOD=`find ${YHOME} \! -type d -newer ${CRY_FILE}`

if [ ${#UPTOD} != 0 -a ${PRM} != "force" ]
then 
	echo ${UPTOD};
	echo "Not up to date. ysave_sh first.";
	if [ ${PRM} != 0 ]
	then 
		echo "Use 'force' as parameter to force the unload.";
	fi
exit;
fi

UNLD=`which yunload_sh`

diff ${YHOME}/scripts/yunload_sh ${UNLD}

cd ${YHOME}/../..
BASE_MNT=`pwd`
MNT_DIR=${BASE_MNT}/tmp

echo "Wiping "${YHOME}
# wipe -rfc ${YHOME}

echo "Umounting "${MNT_DIR}
cd ${BASE_MNT}
sudo umount ${MNT_DIR}

MNT=`sudo df -a | grep none | grep tmp`

if [ ${#MNT} != 0 ]
then echo "CANNOT UMOUNT RAMFS";
exit;
fi

echo "Delete links"

cd ~/devel/bin

rm -f ./yclean_sh 
rm -f ./yrmdb_sh 
rm -f ./yis_updated_sh 
rm -f ./ydebug_sh
rm -f ./ysave_sh
rm -f ./yverif_sh
rm -f ./ymake_sh

echo "Remove YHOME="${HOME}/.yhome.txt

rm ${HOME}/.yhome.txt

echo "Clean history"

history -c
history -w

cd $CURR_DIR


