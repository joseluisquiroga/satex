#!/bin/bash

CURR_DIR=$(pwd)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "yfinish_load ${1} using "${BASH_SOURCE[0]}

# cd $SCRIPT_DIR

PRJ_NAME=${1}
YHOME=`cat ${HOME}/.yhome.txt`

if [ ${#YHOME} = 0 ]
then echo "YHOME=${YHOME} is not set";
exit;
fi

echo "YHOME=${YHOME}"

SCRP=${YHOME}/scripts
cd ${SCRP}

echo "Security check"

#set -x

CRY_ENC=`which cry3`
YLO=`which yload.sh`
YUL=`which yunload.sh`

diff ${SCRP}/yload.sh ${YLO}
diff ${SCRP}/yunload.sh ${YUL}

echo "Using "${CRY_ENC}

#set +x

echo "Finished security check"

echo "Create script links"

DIR_BIN=~/devel/bin
cd ${DIR_BIN}

rm -f ${DIR_BIN}/${PRJ_NAME}
ln -s ${YHOME}/bin/${PRJ_NAME}

rm -f ${DIR_BIN}/yclean_sh 
rm -f ${DIR_BIN}/yrmdb_sh 
rm -f ${DIR_BIN}/yis_updated_sh 
rm -f ${DIR_BIN}/ydebug_sh
rm -f ${DIR_BIN}/ysave_sh
rm -f ${DIR_BIN}/yverif_sh
rm -f ${DIR_BIN}/ymake_sh

ln -s ${SCRP}/yclean_sh 
ln -s ${SCRP}/yrmdb_sh 
ln -s ${SCRP}/yis_updated_sh 
ln -s ${SCRP}/ydebug_sh
ln -s ${SCRP}/ysave_sh
ln -s ${SCRP}/yverif_sh
ln -s ${SCRP}/ymake_sh

cd $CURR_DIR

