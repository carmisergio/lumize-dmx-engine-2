#!/bin/bash
set -e # Exit on command failure

EXECUTABLE_NAME=lumizedmxengine2
EXECUTABLE_INSTALL_PATH=/usr/bin
CONFIG_FILE_NAME=installer/lumizedmxengine2.conf
CONFIG_FILE_INSTALL_PATH=/etc/
LUMIZE_USERNAME=lumize
PERSISTENCY_FOLDER=/var/lib/lumizedmxengine2
SERVICE_UNIT_FILE_NAME=installer/lumizedmxengine2.service
SERVICE_NAME=lumizedmxengine2.service
SERVICE_UNIT_FILE_INSTALL_PATH=/etc/systemd/system/

# Colors
RED_TEXT='\033[0;31m' 
DEFAULT_TEXT='\033[0m'

dependency_missing=false

# Check that script is being run as root
if [[ $(id -u) -ne 0 ]]; then
	echo -e "${RED_TEXT}[ERROR] Please run make install as root!${DEFAULT_TEXT}"
	exit 1
fi


echo "##### Lumize DMX Engine 2 Installation #####"

#################### FILE COPY
# Copy executable
echo "Copying executable to $EXECUTABLE_INSTALL_PATH"
install $EXECUTABLE_NAME $EXECUTABLE_INSTALL_PATH

# Copy config file
echo "Copying config file to $CONFIG_FILE_INSTALL_PATH"
cp $CONFIG_FILE_NAME $CONFIG_FILE_INSTALL_PATH

# Copy service unit file
echo "Copying service unit file"
cp $SERVICE_UNIT_FILE_NAME $SERVICE_UNIT_FILE_INSTALL_PATH

#################### PERSISTENCY DIRECTORY CREATION
echo "Creating folder $PERSISTENCY_FOLDER if it doesn't exist" 
mkdir -p $PERSISTENCY_FOLDER

#################### SYSTEMD SERVICE
systemctl daemon-reload
if systemctl is-enabled $SERVICE_NAME | grep -q "disabled"; then
	echo "Enabling $SERVICE_NAME service"
	systemctl enable $SERVICE_NAME
else
	echo "Service $SERVICE_NAME already enabled!"
fi

echo "Done installing Lumize DMX Engine 2!"


