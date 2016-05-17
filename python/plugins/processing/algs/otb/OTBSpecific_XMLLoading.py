# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBSpecific_XMLLoading.py
    -------------------------
    Date                 : 11-12-13
    Copyright            : (C) 2013 by CS Systemes d'information (CS SI)
    Email                : otb at c-s dot fr (CS SI)
    Contributors         : Julien Malik (CS SI)  - creation of otbspecific
                           Oscar Picas (CS SI)   -
                           Alexia Mondot (CS SI) - split otbspecific into 2 files
                                           add functions
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************

When an OTB algorithms is run, this file allows adapting user parameter to fit the otbapplication.

Most of the following functions are like follows :
    adaptNameOfTheOTBApplication(commands_list)
The command list is a list of all parameters of the given algorithm with all user values.
"""


__author__ = 'Julien Malik, Oscar Picas, Alexia Mondot'
__date__ = 'December 2013'
__copyright__ = '(C) 2013, CS Systemes d\'information  (CS SI)'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
__version__ = "3.8"

import os

try:
    import processing  # NOQA
except ImportError as e:
    raise Exception("Processing must be installed and available in PYTHONPATH")

from processing.core.ProcessingConfig import ProcessingConfig

from . import OTBUtils


def adaptBinaryMorphologicalOperation(commands_list):
    val = commands_list[commands_list.index("-filter") + 1]

    def replace_dilate(param, value):
        if ".dilate" in str(param):
            return param.replace("dilate", value)
        else:
            return param

    import functools
    com_list = map(functools.partial(replace_dilate, value=val), commands_list)

    val = com_list[com_list.index("-structype.ball.xradius") + 1]

    pos = com_list.index("-structype.ball.xradius") + 2

    com_list.insert(pos, '-structype.ball.yradius')
    com_list.insert(pos + 1, val)

    return com_list


def adaptEdgeExtraction(commands_list):
    """
    Add filter.touzi.yradius as the same value as filter.touzi.xradius
    """
    val = commands_list[commands_list.index("-filter") + 1]
    if val == 'touzi':
        bval = commands_list[commands_list.index("-filter.touzi.xradius") + 1]
        pos = commands_list.index("-filter.touzi.xradius") + 2
        commands_list.insert(pos, "-filter.touzi.yradius")
        commands_list.insert(pos + 1, bval)
    return commands_list


def adaptGrayScaleMorphologicalOperation(commands_list):
    """
    Add structype.ball.yradius as the same value as structype.ball.xradius (as it is a ball)
    """
    val = commands_list[commands_list.index("-structype.ball.xradius") + 1]
    pos = commands_list.index("-structype.ball.xradius") + 2
    commands_list.insert(pos, "-structype.ball.yradius")
    commands_list.insert(pos + 1, val)
    return commands_list


def adaptSplitImage(commands_list):
    """
    Ran by default, the extension of output file is .file. Replace it with ".tif"
    If no extension given, put ".tif" at the end of the filename.
    """
    commands_list2 = []
    for item in commands_list:
        if ".file" in item:
            item = item.replace(".file", ".tif")
        if item == "-out":
            index = commands_list.index(item)
            if "." not in os.path.basename(commands_list[index + 1]):
                commands_list[index + 1] = commands_list[index + 1][:-1] + ".tif" + commands_list[index + 1][-1]
        commands_list2.append(item)
    return commands_list2


def adaptLSMSVectorization(commands_list):
    """
    Ran by default, the extension of output file is .file. Replace it with ".shp"
    If no extension given, put ".shp" at the end of the filename.
    """
    commands_list2 = []
    for item in commands_list:
        if ".file" in item:
            item = item.replace(".file", ".shp")
        if item == "-out":
            index = commands_list.index(item)
            if "." not in os.path.basename(commands_list[index + 1]):
                commands_list[index + 1] = commands_list[index + 1][:-1] + ".shp" + commands_list[index + 1][-1]
        commands_list2.append(item)

    return commands_list2


def adaptComputeImagesStatistics(commands_list):
    """
    Ran by default, the extension of output file is .file. Replace it with ".xml"
    If no extension given, put ".shp" at the end of the filename.
    """
    commands_list2 = []
    for item in commands_list:
        if ".file" in item:
            item = item.replace(".file", ".xml")
        commands_list2.append(item)
        if item == "-out":
            index = commands_list.index(item)
            if "." not in os.path.basename(commands_list[index + 1]):
                commands_list[index + 1] = commands_list[index + 1][:-1] + ".xml" + commands_list[index + 1][-1]

    return commands_list2


def adaptKmzExport(commands_list):
    """
    Ran by default, the extension of output file is .file. Replace it with ".kmz"
    If no extension given, put ".kmz" at the end of the filename.
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    adaptGeoidSrtm(commands_list)
    commands_list2 = []
    for item in commands_list:
        if ".file" in item:
            item = item.replace(".file", ".kmz")
        if item == "-out":
            index = commands_list.index(item)
            if "." not in os.path.basename(commands_list[index + 1]):
                commands_list[index + 1] = commands_list[index + 1][:-1] + ".kmz" + commands_list[index + 1][-1]

        commands_list2.append(item)
    return commands_list2


def adaptColorMapping(commands_list):
    """
    The output of this algorithm must be in uint8.
    """
    indexInput = commands_list.index("-out")
    commands_list[indexInput + 1] = commands_list[indexInput + 1] + '" "uint8"'
    return commands_list


def adaptStereoFramework(commands_list):
    """
    Remove parameter and user value instead of giving None.
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    commands_list2 = commands_list
    adaptGeoidSrtm(commands_list2)
    for item in commands_list:
        if "None" in item:
            index = commands_list2.index(item)
            argumentToRemove = commands_list2[index - 1]
            commands_list2.remove(item)
            commands_list2.remove(argumentToRemove)
    return commands_list2


def adaptComputeConfusionMatrix(commands_list):
    """
    Ran by default, the extension of output file is .file. Replace it with ".csv"
    If no extension given, put ".csv" at the end of the filename.
    """
    commands_list2 = []
    for item in commands_list:
        if ".file" in item:
            item = item.replace(".file", ".csv")
        if item == "-out":
            index = commands_list.index(item)
            if "." not in os.path.basename(commands_list[index + 1]):
                commands_list[index + 1] = commands_list[index + 1][:-1] + ".csv" + commands_list[index + 1][-1]

        commands_list2.append(item)
    return commands_list2


def adaptRadiometricIndices(commands_list):
    """
    Replace indice nickname by its corresponding entry in the following dictionary :
    indices = {"ndvi" : "Vegetation:NDVI", "tndvi" : "Vegetation:TNDVI",  "rvi" : "Vegetation:RVI",  "savi" : "Vegetation:SAVI",
           "tsavi" : "Vegetation:TSAVI", "msavi" : "Vegetation:MSAVI",  "msavi2" : "Vegetation:MSAVI2",  "gemi" : "Vegetation:GEMI",
           "ipvi" : "Vegetation:IPVI",
           "ndwi" : "Water:NDWI", "ndwi2" : "Water:NDWI2", "mndwi" :"Water:MNDWI" , "ndpi" : "Water:NDPI",
           "ndti" : "Water:NDTI",
           "ri" : "Soil:RI", "ci" : "Soil:CI", "bi" : "Soil:BI", "bi2" : "Soil:BI2"}
    """
#                 "laindvilog" : , "lairefl" : , "laindviformo" : ,
    indices = {"ndvi": "Vegetation:NDVI", "tndvi": "Vegetation:TNDVI", "rvi": "Vegetation:RVI", "savi": "Vegetation:SAVI",
               "tsavi": "Vegetation:TSAVI", "msavi": "Vegetation:MSAVI", "msavi2": "Vegetation:MSAVI2", "gemi": "Vegetation:GEMI",
               "ipvi": "Vegetation:IPVI",
               "ndwi": "Water:NDWI", "ndwi2": "Water:NDWI2", "mndwi": "Water:MNDWI", "ndpi": "Water:NDPI",
               "ndti": "Water:NDTI",
               "ri": "Soil:RI", "ci": "Soil:CI", "bi": "Soil:BI", "bi2": "Soil:BI2"}
    for item in commands_list:
        if item in indices:
            commands_list[commands_list.index(item)] = indices[item]
    return commands_list


def adaptDisparityMapToElevationMap(commands_list):
    """
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    adaptGeoidSrtm(commands_list)
    return commands_list


def adaptConnectedComponentSegmentation(commands_list):
    """
    Remove parameter and user value instead of giving None.
    """
    commands_list2 = commands_list
    adaptGeoidSrtm(commands_list2)
    for item in commands_list:
        if "None" in item:
            index = commands_list2.index(item)
            argumentToRemove = commands_list2[index - 1]
            commands_list2.remove(item)
            commands_list2.remove(argumentToRemove)
        #commands_list2.append(item)
    return commands_list2


def adaptSuperimpose(commands_list):
    """
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    adaptGeoidSrtm(commands_list)
    return commands_list


def adaptOrthoRectification(commands_list):
    """
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    adaptGeoidSrtm(commands_list)
    return commands_list


def adaptExtractROI(commands_list):
    """
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    adaptGeoidSrtm(commands_list)
    return commands_list


def adaptTrainImagesClassifier(commands_list):
    """
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    adaptGeoidSrtm(commands_list)
    return commands_list


def adaptGeoidSrtm(commands_list):
    """
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    srtm, geoid = ckeckGeoidSrtmSettings()

    if srtm:
        if commands_list[0].endswith("ExtractROI"):
            commands_list.append("-mode.fit.elev.dem")
            commands_list.append(srtm)
        else:
            commands_list.append("-elev.dem")
            commands_list.append(srtm)
    if geoid:
        if commands_list[0].endswith("ExtractROI"):
            commands_list.append("-mode.fit.elev.geoid")
            commands_list.append(geoid)
        else:
            commands_list.append("-elev.geoid")
            commands_list.append(geoid)


def adaptComputePolylineFeatureFromImage(commands_list):
    """
    Remove parameter and user value instead of giving None.
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    commands_list2 = commands_list
    adaptGeoidSrtm(commands_list2)
    for item in commands_list:
        if "None" in item:
            index = commands_list2.index(item)
            argumentToRemove = commands_list2[index - 1]
            commands_list2.remove(item)
            commands_list2.remove(argumentToRemove)
        # commands_list2.append(item)
    return commands_list2


def adaptComputeOGRLayersFeaturesStatistics(commands_list):
    """
    Remove parameter and user value instead of giving None.
    Check geoid file, srtm folder and given elevation and manage arguments.
    """
    commands_list2 = commands_list
    adaptGeoidSrtm(commands_list2)
    for item in commands_list:
        if "None" in item:
            index = commands_list2.index(item)
            argumentToRemove = commands_list2[index - 1]
            commands_list2.remove(item)
            commands_list2.remove(argumentToRemove)
        # commands_list2.append(item)
    return commands_list2


def ckeckGeoidSrtmSettings():
    folder = ProcessingConfig.getSetting(OTBUtils.OTB_SRTM_FOLDER)
    if folder is None:
        folder = ""

    filepath = ProcessingConfig.getSetting(OTBUtils.OTB_GEOID_FILE)
    if filepath is None:
        filepath = ""

    return folder, filepath
