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
from processing.tools.system import isWindows, mkdir
from processing.core.parameters import getParameterFromString
import os
from copy import deepcopy

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


def configFile(alg, parameters, context, outputTxt=False):
    """ Handle inline configuration
    :param parameters:
    """
    # Where is the GRASS7 user directory ?

    new_parameters = deepcopy(parameters)

    userGrass7Path = rliPath()
    if not os.path.isdir(userGrass7Path):
        mkdir(userGrass7Path)
    if not os.path.isdir(os.path.join(userGrass7Path, 'output')):
        mkdir(os.path.join(userGrass7Path, 'output'))
    origConfigFile = new_parameters['config']

    # Handle inline configuration
    if new_parameters['config_txt']:
        # Creates a temporary txt file in user r.li directory
        tempConfig = alg.getTempFilename()
        configFilePath = os.path.join(userGrass7Path, tempConfig)
        # Inject rules into temporary txt file
        with open(configFilePath, "w") as f:
            f.write(new_parameters['config_txt'])

        # Use temporary file as rules file
        new_parameters['config'] = os.path.basename(configFilePath)
        del new_parameters['config_txt']

    # If we have a configuration file, we need to copy it into user dir
    if origConfigFile:
        configFilePath = os.path.join(userGrass7Path, os.path.basename(origConfigFile))
        # Copy the file
        shutil.copy(origConfigFile, configFilePath)

        # Change the parameter value
        new_parameters['config'] = os.path.basename(configFilePath)

    origOutput = alg.getOutputFromName('output')
    if new_parameters['output']:
        param = getParameterFromString("ParameterString|output|txt output|None|False|True")
        new_parameters[param.name()] = origOutput.value
        alg.removeOutputFromName('output')

    alg.processCommand(new_parameters, context)

    # Remove Config file:
    removeConfigFile(alg, new_parameters, context)

    # re-add configTxt
    if outputTxt:
        alg.addOutput(origOutput)


def moveOutputTxtFile(alg, parameters, context):
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
