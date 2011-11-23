#!/bin/sh

[ -e /home/user/.waze/data_deployed_0.0.2 ] && exit 0

SOURCE_DIR=/opt/waze/data
MAP_TARGET_DIR=/home/user/MyDocs/.waze
WAZE_TARGET_DIR=/home/user/.waze

echo Deploying Waze preferences to ${WAZE_TARGET_DIR}
mkdir -p ${WAZE_TARGET_DIR}
cp ${SOURCE_DIR}/* ${WAZE_TARGET_DIR} 2> /dev/null

echo Deploying Waze sound files to ${WAZE_TARGET_DIR}/sound
cp -r ${SOURCE_DIR}/sound ${WAZE_TARGET_DIR} 2> /dev/null

echo Preparing Waze map directory in ${MAP_TARGET_DIR}
mkdir -p ${MAP_TARGET_DIR}
cp -r ${SOURCE_DIR}/maps ${MAP_TARGET_DIR} 2> /dev/null

echo Deploying day/night schema directories to ${WAZE_TARGET_DIR}/skins/default
mkdir -p ${WAZE_TARGET_DIR}/skins/default
cp -r ${SOURCE_DIR}/skins/default/day ${WAZE_TARGET_DIR}/skins/default 2> /dev/null
cp -r ${SOURCE_DIR}/skins/default/night ${WAZE_TARGET_DIR}/skins/default 2> /dev/null

touch /home/user/.waze/data_deployed_0.0.2
