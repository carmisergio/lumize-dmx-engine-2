#!/bin/bash

EXECUTABLE_INSTALL_PATH=/usr/bin/lumizedmxengine2
CONFIG_FILE_INSTALL_PATH=/etc/lumizedmxengine2.conf
PERSISTENCY_FOLDER=/var/lib/lumizedmxengine2
SERVICE_UNIT_FILE_PATH=/etc/systemd/system/lumizedmxengine2.service
SERVICE_NAME=lumizedmxengine2.service

# Colors
RED_TEXT='\033[0;31m' 
DEFAULT_TEXT='\033[0m'

dependency_missing=false

# Check that script is being run as root
if [[ $(id -u) -ne 0 ]]; then
	echo -e "${RED_TEXT}[ERROR] Please run make uninstall as root!${DEFAULT_TEXT}"
	exit 1
fi


echo "##### Lumize DMX Engine 2 Uninstallation #####"

#################### SYSTEMD SERVICE
echo "Stopping and disabling service $SERVICE_NAME"
systemctl stop $SERVICE_NAME
systemctl disable $SERVICE_NAME

#################### FILE COPY
# Delete executable
echo "Deleting executable from $EXECUTABLE_INSTALL_PATH"
rm $EXECUTABLE_INSTALL_PATH

# Delete config file
 echo "Deleting config file $CONFIG_FILE_INSTALL_PATH"
 rm $CONFIG_FILE_INSTALL_PATH

# Delete service unit file
echo "Deleting service unit file"
rm $SERVICE_UNIT_FILE_PATH

#################### PERSISTENCY DIRECTORY CREATION
echo "Deleting folder $PERSISTENCY_FOLDER" 
rm -r $PERSISTENCY_FOLDER


echo "Done uninstalling Lumize DMX Engine 2!"


