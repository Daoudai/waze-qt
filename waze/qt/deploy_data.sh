#!/bin/sh

SOURCE_DIR=/opt/waze/data
MAP_TARGET_DIR=/home/user/MyDocs/.waze
WAZE_TARGET_DIR=/home/user/.waze

[ -e ${WAZE_TARGET_DIR}/data_deployed_0.0.3 ] && exit 0

echo Deploying Waze preferences to ${WAZE_TARGET_DIR}
mkdir -p ${WAZE_TARGET_DIR}
cp ${SOURCE_DIR}/* ${WAZE_TARGET_DIR} 2> /dev/null

if [ ! -e ${MAP_TARGET_DIR}/sound ]; then
    echo Deploying Waze sound files to ${WAZE_TARGET_DIR}/sound
    cp -r ${SOURCE_DIR}/sound ${WAZE_TARGET_DIR} 2> /dev/null
fi

if [ ! -e ${MAP_TARGET_DIR}/maps ]; then
    echo Preparing Waze map directory in ${MAP_TARGET_DIR}
    mkdir -p ${MAP_TARGET_DIR}
    cp -r ${SOURCE_DIR}/maps ${MAP_TARGET_DIR} 2> /dev/null
fi

if [ ! -e ${WAZE_TARGET_DIR}/skins/default ]; then
    echo Deploying day/night schema directories to ${WAZE_TARGET_DIR}/skins/default
    mkdir -p ${WAZE_TARGET_DIR}/skins/default
    cp -r ${SOURCE_DIR}/skins/default/day ${WAZE_TARGET_DIR}/skins/default 2> /dev/null
    cp -r ${SOURCE_DIR}/skins/default/night ${WAZE_TARGET_DIR}/skins/default 2> /dev/null
fi

touch ${WAZE_TARGET_DIR}/data_deployed_0.0.3
