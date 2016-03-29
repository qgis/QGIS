# -*- coding: utf-8 -*-

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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import shutil
from processing.tools.system import isWindows, isMac, userFolder, mkdir
from processing.core.parameters import getParameterFromString
import os

# for MS-Windows users who have MBCS chars in their name:
if os.name == 'nt':
    import win32api


def rliPath():
    """Return r.li GRASS7 user dir"""
    if isWindows():
        homeDir = win32api.GetShortPathName(os.path.expanduser('~'))
        return os.path.join(homeDir, 'AppData', 'Roaming', 'GRASS7', 'r.li')
    else:
        return os.path.join(os.path.expanduser("~"), '.grass7', 'r.li')


def removeConfigFile(alg):
    """ Remove the r.li user dir config file """
    configPath = alg.getParameterValue('config')
    if isWindows():
        command = "DEL {}".format(os.path.join(rliPath(), configPath))
    else:
        command = "rm {}".format(os.path.join(rliPath(), configPath))
    alg.commands.append(command)


def checkMovingWindow(alg, outputTxt=False):
    """ Verify if we have the right parameters """
    configTxt = alg.getParameterValue('config_txt')
    config = alg.getParameterValue('config')
    if configTxt and config:
        return alg.tr("You need to set either inline configuration or a configuration file!")

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
        return alg.tr('Your configuration needs to be a "moving window" configuration!')

    if movingWindow and outputTxt:
        return alg.tr('Your configuration needs to be a non "moving window" configuration!')

    return None


def configFile(alg, outputTxt=False):
    """ Handle inline configuration """
    # Where is the GRASS7 user directory ?
    userGrass7Path = rliPath()
    if not os.path.isdir(userGrass7Path):
        mkdir(userGrass7Path)
    if not os.path.isdir(os.path.join(userGrass7Path, 'output')):
        mkdir(os.path.join(userGrass7Path, 'output'))
    origConfigFile = alg.getParameterValue('config')

    # Handle inline configuration
    configTxt = alg.getParameterFromName('config_txt')
    if configTxt.value:
        # Creates a temporary txt file in user r.li directory
        tempConfig = alg.getTempFilename()
        configFilePath = os.path.join(userGrass7Path, tempConfig)
        # Inject rules into temporary txt file
        with open(configFilePath, "w") as f:
            f.write(configTxt.value)

        # Use temporary file as rules file
        alg.setParameterValue('config', os.path.basename(configFilePath))
        alg.parameters.remove(configTxt)

    # If we have a configuration file, we need to copy it into user dir
    if origConfigFile:
        configFilePath = os.path.join(userGrass7Path, os.path.basename(origConfigFile))
        # Copy the file
        shutil.copy(origConfigFile, configFilePath)

        # Change the parameter value
        alg.setParameterValue('config', os.path.basename(configFilePath))

    origOutput = alg.getOutputFromName('output')
    if outputTxt:
        param = getParameterFromString("ParameterString|output|txt output|None|False|True")
        param.value = os.path.basename(origOutput.value)
        alg.addParameter(param)
        alg.removeOutputFromName('output')

    alg.processCommand()

    # Remove Config file:
    removeConfigFile(alg)

    # re-add configTxt
    alg.addParameter(configTxt)
    alg.setParameterValue('config', origConfigFile)
    if outputTxt:
        for param in [f for f in alg.parameters if f.name == 'output']:
            alg.parameters.remove(param)
        alg.addOutput(origOutput)


def moveOutputTxtFile(alg):
    # Find output file name:
    origOutput = alg.getOutputValue('output')
    userGrass7Path = rliPath()

    outputDir = os.path.join(userGrass7Path, 'output')
    output = os.path.join(outputDir, os.path.basename(origOutput))

    # move the file
    if isWindows():
        command = "MOVE /Y {} {}".format(output, origOutput)
    else:
        command = "mv -f {} {}".format(output, origOutput)
    alg.commands.append(command)
