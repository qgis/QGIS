#!/bin/bash
# Setup QGIS for the automated tests
# This is normally called from Travis or rundockertests.sh
# before running the tests for a particular plugin
#
# Note: on QGIS3 assumes the default profile for root user
#
# - create the folders
# - install startup.py monkey patches
# - disable tips
# - enable the plugin

PLUGIN_NAME=$1
CONF_FOLDER="/root/.config/QGIS"
CONF_FILE="${CONF_FOLDER}/QGIS2.conf"
CONF_MASTER_FOLDER="/root/.local/share/QGIS/QGIS3/profiles/default/QGIS/"
CONF_MASTER_FILE="${CONF_MASTER_FOLDER}/QGIS3.ini"
QGIS_FOLDER="/root/.qgis2"

QGIS_MASTER_FOLDER="/root/.local/share/QGIS/QGIS3/profiles/default"
PLUGIN_FOLDER="${QGIS_FOLDER}/python/plugins"
PLUGIN_MASTER_FOLDER="${QGIS_MASTER_FOLDER}/python/plugins"

STARTUP_FOLDER="${QGIS_FOLDER}/python"
STARTUP_MASTER_FOLDER="/root/.local/share/QGIS/QGIS3/"

# Creates the config file
mkdir -p $CONF_FOLDER
if [ -e "$CONF_FILE" ]; then
    rm -f $CONF_FILE
fi
touch $CONF_FILE


mkdir -p $CONF_MASTER_FOLDER
if [ -e "$CONF_MASTER_FILE" ]; then
    rm -f $CONF_MASTER_FILE
fi
touch $CONF_MASTER_FILE

# Creates plugin folder
mkdir -p $PLUGIN_FOLDER
mkdir -p $PLUGIN_MASTER_FOLDER
mkdir -p $STARTUP_MASTER_FOLDER

# Install the monkey patches to prevent modal stacktrace on python errors
cp /usr/bin/qgis_startup.py ${STARTUP_FOLDER}/startup.py
cp /usr/bin/qgis_startup.py ${STARTUP_MASTER_FOLDER}/startup.py

# Disable tips
printf "[Qgis]\n" >> $CONF_FILE
# !!!! Note that on master it is lowercase !!!!
printf "[qgis]\n" >> $CONF_MASTER_FILE
SHOW_TIPS=$(qgis --help 2>&1 | head -2 | grep 'QGIS - ' | perl -npe 'chomp; s/QGIS - (\d+)\.(\d+).*/showTips\1\2=false/')
printf "%s\n\n" "$SHOW_TIPS" >> $CONF_FILE
printf "%s\n\n" "$SHOW_TIPS" >> $CONF_MASTER_FILE

if [ -n "$PLUGIN_NAME" ]; then
    # Enable plugin
    printf '[PythonPlugins]\n' >> $CONF_FILE
    printf "%s=true\n\n" "$PLUGIN_NAME" >> $CONF_FILE

    printf '[PythonPlugins]\n' >> $CONF_MASTER_FILE
    printf "%s=true\n\n" "$PLUGIN_NAME" >> $CONF_MASTER_FILE
fi

# Disable firstRunVersionFlag for master
{
    printf
    "\n[migration]\n"
    "fileVersion=2\n"
    "firstRunVersionFlag=29900\n"
    "settings=true\n\n"
} >> $CONF_MASTER_FILE


# Install the plugin
if  [ ! -L "${PLUGIN_FOLDER}/${PLUGIN_NAME}" ]; then
    ln -s "/tests_directory/${PLUGIN_NAME}" "${PLUGIN_FOLDER}"
    echo "Plugin folder linked in ${PLUGIN_FOLDER}/${PLUGIN_NAME}"
fi
if [ ! -d "${PLUGIN_MASTER_FOLDER}/${PLUGIN_NAME}" ]; then
    ln -s "/tests_directory/${PLUGIN_NAME}" "${PLUGIN_MASTER_FOLDER}"
    echo "Plugin master folder linked in ${PLUGIN_MASTER_FOLDER}/${PLUGIN_NAME}"
fi
