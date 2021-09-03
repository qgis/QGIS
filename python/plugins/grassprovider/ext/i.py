# -*- coding: utf-8 -*-

"""
***************************************************************************
    i.py
    ----
    Date                 : April 2016
    Copyright            : (C) 2016 by Médéric Ribreux
    Email                : mederic dot ribreux at medspx dot fr
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
__date__ = 'April 2016'
__copyright__ = '(C) 2016, Médéric Ribreux'

import os
from processing.tools.system import (isWindows, getTempFilename)
from grassprovider.Grass7Utils import Grass7Utils
from qgis.PyQt.QtCore import QDir
from qgis.core import QgsProcessingParameterString
from qgis.core import QgsMessageLog


def orderedInput(alg, parameters, context, src, tgt, numSeq=None):
    """Import multiple rasters in order
    :param alg: algorithm object.
    :param parameters: algorithm parameters dict.
    :param context: algorithm context.
    :param src: Name of the source parameter.
    :param tgt: Name of a new input parameter.
    :param numSeq: List of a sequence for naming layers.
    """
    rootFilename = 'rast_{}.'.format(os.path.basename(getTempFilename()))
    # parameters[tgt] = rootFilename
    param = QgsProcessingParameterString(tgt, 'virtual input',
                                         rootFilename, False, False)
    alg.addParameter(param)

    rasters = alg.parameterAsLayerList(parameters, src, context)
    # Handle specific range
    if numSeq is None:
        numSeq = list(range(1, len(rasters) + 1))

    for idx, raster in enumerate(rasters):
        rasterName = '{}{}'.format(rootFilename, numSeq[idx])
        alg.loadRasterLayer(rasterName, raster, False, None, rasterName)

    # Don't forget to remove the old input parameter
    alg.removeParameter(src)


def regroupRasters(alg, parameters, context, src, group, subgroup=None, extFile=None):
    """
    Group multiple input rasters into a group
    * If there is a subgroupField, a subgroup will automatically be created.
    * When an external file is provided, the file is copied into the respective
    directory of the subgroup.
    :param parameters:
    :param context:
    :param src: name of input parameter with multiple rasters.
    :param group: name of group.
    :param subgroup: name of subgroup.
    :param extFile: dict : parameterName:directory name
    """
    # Create a group parameter
    groupName = 'group_{}'.format(os.path.basename(getTempFilename()))
    param = QgsProcessingParameterString(group, 'virtual group',
                                         groupName, False, False)
    alg.addParameter(param)

    # Create a subgroup
    subgroupName = None
    if subgroup:
        subgroupName = 'subgroup_{}'.format(os.path.basename(getTempFilename()))
        param = QgsProcessingParameterString(subgroup, 'virtual subgroup',
                                             subgroupName, False, False)
        alg.addParameter(param)

    # Compute raster names
    rasters = alg.parameterAsLayerList(parameters, src, context)
    rasterNames = []
    for idx, raster in enumerate(rasters):
        name = '{}_{}'.format(src, idx)
        if name in alg.exportedLayers:
            rasterNames.append(alg.exportedLayers[name])

    # Insert a i.group command
    command = 'i.group group={}{} input={}'.format(
        groupName,
        ' subgroup={}'.format(subgroupName) if subgroup else '',
        ','.join(rasterNames))
    alg.commands.append(command)

    # Handle external files
    # if subgroupField and extFile:
    #     for ext in extFile.keys():
    #         extFileName = new_parameters[ext]
    #         if extFileName:
    #             shortFileName = os.path.basename(extFileName)
    #             destPath = os.path.join(Grass7Utils.grassMapsetFolder(),
    #                                     'PERMANENT',
    #                                     'group', new_parameters[group.name()],
    #                                     'subgroup', new_parameters[subgroup.name()],
    #                                     extFile[ext], shortFileName)
    #             copyFile(alg, extFileName, destPath)

    alg.removeParameter(src)

    return groupName, subgroupName


def importSigFile(alg, group, subgroup, src, sigDir='sig'):
    """
    Import a signature file into an
    internal GRASSDB folder
    """
    shortSigFile = os.path.basename(src)
    interSig = os.path.join(Grass7Utils.grassMapsetFolder(),
                            'PERMANENT', 'group', group, 'subgroup',
                            subgroup, sigDir, shortSigFile)
    copyFile(alg, src, interSig)
    return shortSigFile


def exportSigFile(alg, group, subgroup, dest, sigDir='sig'):
    """
    Export a signature file from internal GRASSDB
    to final destination
    """
    shortSigFile = os.path.basename(dest)
    interSig = os.path.join(Grass7Utils.grassMapsetFolder(),
                            'PERMANENT', 'group', group, 'subgroup',
                            subgroup, sigDir, shortSigFile)
    moveFile(alg, interSig, dest)
    return interSig


def exportInputRasters(alg, parameters, context, rasterDic):
    """
    Export input rasters
    Use a dict to make input/output link:
    { 'inputName1': 'outputName1', 'inputName2': 'outputName2'}
    """
    createOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_OPT, context)
    metaOpt = alg.parameterAsString(parameters, alg.GRASS_RASTER_FORMAT_META, context)

    # Get inputs and outputs
    for inputName, outputName in rasterDic.items():
        fileName = os.path.normpath(
            alg.parameterAsOutputLayer(parameters, outputName, context))
        grassName = alg.exportedLayers[inputName]
        outFormat = Grass7Utils.getRasterFormatFromFilename(fileName)
        alg.exportRasterLayer(grassName, fileName, True, outFormat, createOpt, metaOpt)


def verifyRasterNum(alg, parameters, context, rasters, mini, maxi=None):
    """Verify that we have at least n rasters in multipleInput"""
    num = len(alg.parameterAsLayerList(parameters, rasters, context))
    if num < mini:
        return False, 'You need to set at least {} input rasters for this algorithm!'.format(mini)
    if maxi and num > maxi:
        return False, 'You need to set a maximum of {} input rasters for this algorithm!'.format(maxi)
    return True, None


# def file2Output(alg, output):
#     """Transform an OutputFile to a parameter"""
#     # Get the outputFile
#     outputFile = alg.getOutputFromName(output)
#     alg.removeOutputFromName(output)

#     # Create output parameter
#     param = getParameterFromString("ParameterString|{}|output file|None|False|False".format(output), 'GrassAlgorithm')
#     param.value = outputFile.value
#     alg.addParameter(param)

#     return outputFile


def createDestDir(alg, toFile):
    """ Generates an mkdir command for GRASS7 script """
    # Creates the destination directory
    command = "{} \"{}\"".format(
        "MD" if isWindows() else "mkdir -p",
        QDir.toNativeSeparators(os.path.dirname(toFile))
    )
    alg.commands.append(command)


def moveFile(alg, fromFile, toFile):
    """ Generates a move command for GRASS7 script """
    createDestDir(alg, toFile)
    command = "{} \"{}\" \"{}\"".format(
        "MOVE /Y" if isWindows() else "mv -f",
        QDir.toNativeSeparators(fromFile),
        QDir.toNativeSeparators(toFile)
    )
    alg.commands.append(command)


def copyFile(alg, fromFile, toFile):
    """ Generates a copy command for GRASS7 script """
    createDestDir(alg, toFile)
    command = "{} \"{}\" \"{}\"".format(
        "COPY /Y" if isWindows() else "cp -f",
        QDir.toNativeSeparators(fromFile),
        QDir.toNativeSeparators(toFile))
    alg.commands.append(command)
