#!/bin/bash

CURR_DIR=$(pwd)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "YLOAD ${1} using "${BASH_SOURCE[0]}

cd $SCRIPT_DIR

CRY_FILE=${1}
TMP_DIR=${CURR_DIR}/tmp
PRJ_NAME="bible_sat"

echo "CURR_DIR="${CURR_DIR}
echo "SCRIPT_DIR="${SCRIPT_DIR}
echo "CRY_FILE="${CRY_FILE}
echo "TMP_DIR="${TMP_DIR}


cd $CURR_DIR
