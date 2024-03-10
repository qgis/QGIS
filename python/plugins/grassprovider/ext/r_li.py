"""
***************************************************************************
    r_li.py
    -------
    Date                 : February 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : medspx at medspx dot fr
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Médéric Ribreux'
__date__ = 'February 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

import shutil
from qgis.core import QgsProcessingParameterString
from processing.tools.system import (isWindows, mkdir,
                                     getTempFilename)
from grassprovider.grass_utils import GrassUtils

import os

# for MS-Windows users who have MBCS chars in their name:
if os.name == 'nt':
    import win32api


def rliPath():
    """Return r.li GRASS user dir"""
    grass_version = GrassUtils.installedVersion().split('.')[0]
    if isWindows():
        homeDir = win32api.GetShortPathName(os.path.expanduser('~'))
        return os.path.join(homeDir, 'AppData', 'Roaming', f'GRASS{grass_version}', 'r.li')
    else:
        return os.path.join(os.path.expanduser("~"), f'.grass{grass_version}', 'r.li')


def removeConfigFile(alg, parameters, context):
    """ Remove the r.li user dir config file """
    configPath = alg.parameterAsString(parameters, 'config', context)
    if isWindows():
        command = "DEL {}".format(os.path.join(rliPath(), configPath))
    else:
        command = "rm {}".format(os.path.join(rliPath(), configPath))
    alg.commands.append(command)


def checkMovingWindow(alg, parameters, context, outputTxt=False):
    """ Verify if we have the right parameters """
    configTxt = alg.parameterAsString(parameters, 'config_txt', context)
    config = alg.parameterAsString(parameters, 'config', context)
    if configTxt and config:
        return False, alg.tr("You need to set either inline configuration or a configuration file!")

    # Verify that configuration is in moving window
    movingWindow = False
    if configTxt:
        if 'MOVINGWINDOW' in configTxt:
            movingWindow = True

    # Read config file:
    if config:
        with open(config) as f:
            for line in f:
                if 'MOVINGWINDOW' in line:
                    movingWindow = True

    if not movingWindow and not outputTxt:
        return False, alg.tr('Your configuration needs to be a "moving window" configuration!')

    if movingWindow and outputTxt:
        return False, alg.tr('Your configuration needs to be a non "moving window" configuration!')

    return True, None


def configFile(alg, parameters, context, feedback, outputTxt=False):
    """Handle inline configuration
    :param parameters:
    """
    # Where is the GRASS user directory ?
    user_grass_path = rliPath()
    if not os.path.isdir(user_grass_path):
        mkdir(user_grass_path)
    if not os.path.isdir(os.path.join(user_grass_path, 'output')):
        mkdir(os.path.join(user_grass_path, 'output'))

    # If we have a configuration file, we need to copy it into user dir
    if parameters['config']:
        fileName = alg.parameterAsString(parameters, 'config', context)
        configFilePath = os.path.join(user_grass_path, os.path.basename(fileName))
        # Copy the file
        shutil.copy(parameters['config'], configFilePath)
        # Change the parameter value
        parameters['config'] = os.path.basename(configFilePath)
    # Handle inline configuration
    elif parameters['config_txt']:
        # Creates a temporary txt file in user r.li directory
        tempConfig = os.path.basename(getTempFilename(context=context))
        configFilePath = os.path.join(user_grass_path, tempConfig)
        # Inject rules into temporary txt file
        with open(configFilePath, "w") as f:
            f.write(alg.parameterAsString(parameters, 'config_txt', context))
            f.write("\n")

        # Use temporary file as rules file
        parameters['config'] = os.path.basename(configFilePath)
        alg.removeParameter('config_txt')

    # For ascii output, we need a virtual output
    if outputTxt:
        param = QgsProcessingParameterString(
            'output', 'virtual output',
            'a' + os.path.basename(getTempFilename(context=context)),
            False, False)
        alg.addParameter(param)

    alg.processCommand(parameters, context, feedback, outputTxt)

    # Remove Config file:
    removeConfigFile(alg, parameters, context)


def moveOutputTxtFile(alg, parameters, context):
    # Find output file name:
    txtPath = alg.parameterAsString(parameters, 'output_txt', context)
    user_grass_path = rliPath()

    output = os.path.join(user_grass_path, 'output',
                          alg.parameterAsString(parameters, 'output', context))
    # move the file
    if isWindows():
        command = "MOVE /Y {} {}".format(output, txtPath)
    else:
        command = "mv -f {} {}".format(output, txtPath)
    alg.commands.append(command)
