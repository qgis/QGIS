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

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

from processing.core.parameters import ParameterRaster, getParameterFromString
from processing.tools.system import isWindows
from ..Grass7Utils import Grass7Utils
from os import path


def multipleOutputDir(alg, field, basename=None):
    """
    Handle multiple output of rasters into a
    directory.
    """
    # We need to know where is the output directory
    outputDir = alg.getOutputValue(field)

    # We need to grab the variable basename
    if basename:
        commands = ["for r in $(g.list type=rast pattern='{}*'); do".format(basename)]
    # Otherwise, export everything
    else:
        commands = ["for r in $(g.list type=rast); do".format(basename)]
    commands.append("  r.out.gdal -c -t -f input=${{r}} output={}/${{r}}.tif createopt=\"TFW=YES,COMPRESS=LZW\"".format(outputDir))
    commands.append("done")
    alg.commands.extend(commands)
    alg.outputCommands.extend(commands)


def orderedInput(alg, inputParameter, targetParameterDef, numSeq=None):
    """Inport multiple rasters in the order"""
    rasters = alg.getParameterValue(inputParameter).split(';')
    # TODO: make targetParameter
    inputParameter = getParameterFromString(targetParameterDef)
    rootFilename = '{}_'.format(alg.getTempFilename())
    inputParameter.value = rootFilename
    alg.addParameter(inputParameter)
    # Handle specific range
    if numSeq is None:
        numSeq = range(1, len(rasters) + 1)

    for idx in range(len(rasters)):
        layer = rasters[idx]
        if layer in alg.exportedLayers.keys():
            continue
        else:
            destFilename = '{}{}'.format(rootFilename, numSeq[idx])
            alg.setSessionProjectionFromLayer(layer, alg.commands)
            alg.exportedLayers[layer] = destFilename
            command = 'r.external input={} band=1 output={} --overwrite -o'.format(layer, destFilename)
            alg.commands.append(command)

    alg.setSessionProjectionFromProject(alg.commands)

    region = \
        unicode(alg.getParameterValue(alg.GRASS_REGION_EXTENT_PARAMETER))
    regionCoords = region.split(',')
    command = 'g.region'
    command += ' -a'
    command += ' n=' + unicode(regionCoords[3])
    command += ' s=' + unicode(regionCoords[2])
    command += ' e=' + unicode(regionCoords[1])
    command += ' w=' + unicode(regionCoords[0])
    cellsize = alg.getParameterValue(alg.GRASS_REGION_CELLSIZE_PARAMETER)
    if cellsize:
        command += ' res=' + unicode(cellsize)
    else:
        command += ' res=' + unicode(alg.getDefaultCellsize())
    alignToResolution = \
        alg.getParameterValue(alg.GRASS_REGION_ALIGN_TO_RESOLUTION)
    if alignToResolution:
        command += ' -a'
    alg.commands.append(command)
    return rootFilename


def regroupRasters(alg, field, groupField, subgroupField=None, extFile=None):
    """
    Group multiple input rasters into a group
    * If there is a subgroupField, a subgroup will automatically created.
    * When an external file is provided, the file is copied into the respective
      directory of the subgroup.
    * extFile is a dict of the form 'parameterName':'directory name'.
    """
    # List of rasters names
    rasters = alg.getParameterFromName(field)
    rastersList = rasters.value.split(';')
    alg.parameters.remove(rasters)

    # Insert a i.group command
    group = getParameterFromString("ParameterString|{}|group of rasters|None|False|False".format(groupField))
    group.value = alg.getTempFilename()
    alg.addParameter(group)

    if subgroupField:
        subgroup = getParameterFromString("ParameterString|{}|subgroup of rasters|None|False|False".format(subgroupField))
        subgroup.value = alg.getTempFilename()
        alg.addParameter(subgroup)

    command = 'i.group group={}{} input={}'.format(
        group.value,
        ' subgroup={}'.format(subgroup.value) if subgroupField else '',
        ','.join([alg.exportedLayers[f] for f in rastersList])
    )
    alg.commands.append(command)

    # Handle external files
    origExtParams = {}
    if subgroupField and extFile:
        for ext in extFile.keys():
            extFileName = alg.getParameterValue(ext)
            if extFileName:
                shortFileName = path.basename(extFileName)
                destPath = path.join(Grass7Utils.grassMapsetFolder(),
                                     'PERMANENT',
                                     'group', group.value,
                                     'subgroup', subgroup.value,
                                     extFile[ext], shortFileName)
            copyFile(alg, extFileName, destPath)
            origExtParams[ext] = extFileName
            alg.setParameterValue(ext, shortFileName)

    # modify parameters values
    alg.processCommand()

    # Re-add input rasters
    alg.addParameter(rasters)

    # replace external files value with original value
    for param in origExtParams.keys():
        alg.setParameterValue(param, origExtParams[param])

    # Delete group:
    alg.parameters.remove(group)
    if subgroupField:
        alg.parameters.remove(subgroup)
        return group.value, subgroup.value

    return group.value


def exportInputRasters(alg, rasterDic):
    """
    Export input rasters
    Use a dict to make input/output link:
    { 'inputName1': 'outputName1', 'inputName2': 'outputName2'}
    """
    # Get inputs and outputs
    for inputName, outputName in rasterDic.iteritems():
        inputRaster = alg.getParameterValue(inputName)
        outputRaster = alg.getOutputFromName(outputName)
        command = 'r.out.gdal -c -t -f --overwrite createopt="TFW=YES,COMPRESS=LZW" input={} output=\"{}\"'.format(
            alg.exportedLayers[inputRaster],
            outputRaster.value
        )
        alg.commands.append(command)
        alg.outputCommands.append(command)


def verifyRasterNum(alg, rasters, mini, maxi=None):
    """Verify if we have at least n rasters in multipleInput"""
    num = len(alg.getParameterValue(rasters).split(';'))
    if num < mini:
        return 'You need to set at least {} input rasters for this algorithm!'.format(mini)
    if maxi and num > maxi:
        return 'You need to set a maximum of {} input rasters for this algorithm!'.format(maxi)
    return None


def file2Output(alg, output):
    """Transform an OutputFile to a parameter"""
    # Get the outputFile
    outputFile = alg.getOutputFromName(output)
    alg.removeOutputFromName(output)

    # Create output parameter
    param = getParameterFromString("ParameterString|{}|output file|None|False|False".format(output))
    param.value = outputFile.value
    alg.addParameter(param)

    return outputFile


def createDestDir(alg, toFile):
    """ Generates an mkdir command for GRASS7 script """
    # Creates the destination directory
    command = "{} {}".format(
        "MD" if isWindows() else "mkdir -p",
        path.dirname(toFile)
    )
    alg.commands.append(command)


def moveFile(alg, fromFile, toFile):
    """ Generates a move command for GRASS7 script """
    createDestDir(alg, toFile)
    command = "{} {} {}".format(
        "MOVE /Y" if isWindows() else "mv -f",
        fromFile,
        toFile
    )
    alg.commands.append(command)


def copyFile(alg, fromFile, toFile):
    """ Generates a copy command for GRASS7 script """
    createDestDir(alg, toFile)
    command = "{} {} {}".format(
        "COPY /Y" if isWindows() else "cp -f",
        fromFile,
        toFile)
    alg.commands.append(command)
