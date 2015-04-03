# -*- coding: utf-8 -*-

"""
***************************************************************************
    OTBUtils.py
    ---------------------
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

When QGIS is run, OTB algorithms are created according to xml files from description/ directory.
"""

__author__ = 'Julien Malik, Oscar Picas, Alexia Mondot'
__date__ = 'December 2013'
__copyright__ = '(C) 2013, CS Systemes d\'information  (CS SI)'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'
__version__ = "3.8"

import copy

try:
    import processing
except ImportError, e:
    raise Exception("Processing must be installed and available in PYTHONPATH")

try:
    import otbApplication
except ImportError, e:
    raise Exception("OTB python plugins must be installed and available in PYTHONPATH")

from processing.algs.otb.OTBUtils import (renameValueField,
                                    remove_dependant_choices,
                                    remove_other_choices,
                                    remove_parameter_by_key,
                                    defaultSplit,
                                    split_by_choice,
                                    defaultWrite,
                                    remove_choice,
                                    remove_independant_choices )


def getBinaryMorphologicalOperation(available_app, original_dom_document):
    """
    Let ball as only available structype.
    Split the application according to its filter dilate, erode, opening, closing.
    """
    the_root = original_dom_document
    renameValueField(the_root, 'structype.ball.xradius', 'name', 'The Structuring Element Radius')
    renameValueField(the_root, 'structype.ball.xradius', 'description', 'The Structuring Element Radius')
    remove_dependant_choices(the_root, 'structype', 'ball')
    remove_other_choices(the_root, 'structype', 'ball')
    remove_dependant_choices(the_root, 'filter', 'dilate')
    remove_parameter_by_key(the_root, 'structype.ball.yradius')
    #defaultWrite(available_app, the_root)
    the_list = defaultSplit(available_app, the_root, 'filter')
    return the_list


def getEdgeExtraction(available_app, original_dom_document):
    """
    Let ball as only available filter (not an oval).
    Split the application according to its filter gradient, sobel, touzi.
    """
    the_root = original_dom_document
    renameValueField(the_root, 'filter.touzi.xradius', 'name', 'The Radius')
    renameValueField(the_root, 'filter.touzi.xradius', 'description', 'The Radius')
    remove_parameter_by_key(the_root, 'filter.touzi.yradius')
    split = split_by_choice(the_root, 'filter')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list


def getGrayScaleMorphologicalOperation(available_app, original_dom_document):
    """
    Let ball as only available structype.
    Split the application according to its filter dilate, erode, opening, closing.
    """
    the_root = original_dom_document
    renameValueField(the_root, 'structype.ball.xradius', 'name', 'The Structuring Element Radius')
    renameValueField(the_root, 'structype.ball.xradius', 'description', 'The Structuring Element Radius')
    remove_dependant_choices(the_root, 'structype', 'ball')
    remove_other_choices(the_root, 'structype', 'ball')
    remove_parameter_by_key(the_root, 'structype.ball.yradius')

    split = defaultSplit(available_app, the_root, 'filter')
    return split


def getOrthoRectification(available_app, original_dom_document):
    """
    Let only mode auto.
    Remove all parameters which should be updated once the input file given.
    Split by SRS : EPSG, fit to ortho, lambert-wgs84 and UTM.
    Each of these SRS have their own parameters modified in this fonction.
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document

    remove_choice(the_root, 'outputs.mode', 'auto')
    remove_independant_choices(the_root, 'outputs.mode', 'auto')
    remove_choice(the_root, 'outputs.mode', 'outputroi')
    remove_independant_choices(the_root, 'outputs.mode', 'outputroi')
    remove_parameter_by_key(the_root, 'outputs.ulx')
    remove_parameter_by_key(the_root, 'outputs.uly')
    remove_parameter_by_key(the_root, 'outputs.sizex')
    remove_parameter_by_key(the_root, 'outputs.sizey')
    remove_parameter_by_key(the_root, 'outputs.spacingx')
    remove_parameter_by_key(the_root, 'outputs.spacingy')
    remove_parameter_by_key(the_root, 'outputs.lrx')
    remove_parameter_by_key(the_root, 'outputs.lry')
    remove_parameter_by_key(the_root, 'opt.rpc')

    deleteGeoidSrtm(the_root)

    remove_parameter_by_key(the_root, 'outputs.isotropic')

    emptyMap = copy.deepcopy(the_root)

    remove_parameter_by_key(the_root, 'outputs.ortho')
    remove_choice(the_root, 'outputs.mode', 'orthofit')
    remove_independant_choices(the_root, 'outputs.mode', 'orthofit')
    merged = copy.deepcopy(the_root)




    split = split_by_choice(the_root, 'map')
    the_list = []

    for key in split:
        if key == 'utm':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'map.epsg.code')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)
        elif key == 'epsg':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'map.utm.northhem')
            remove_parameter_by_key(the_doc, 'map.utm.zone')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)

    remove_choice(merged, 'map', 'utm')
    remove_choice(merged, 'map', 'epsg')
    remove_parameter_by_key(merged, 'map.epsg.code')
    remove_parameter_by_key(merged, 'map.utm.northhem')
    remove_parameter_by_key(merged, 'map.utm.zone')
    old_app_name = merged.find('key').text
    merged.find('key').text = '%s-%s' % (old_app_name, 'lambert-WGS84')
    merged.find('longname').text = '%s (%s)' % (old_app_name, 'lambert-WGS84')
    defaultWrite('%s-%s' % (available_app, 'lambert-WGS84'), merged)
    the_list.append(merged)

    remove_parameter_by_key(emptyMap, 'map')
    remove_parameter_by_key(emptyMap, 'map.epsg.code')
    remove_parameter_by_key(emptyMap, 'map.utm.northhem')
    remove_parameter_by_key(emptyMap, 'map.utm.zone')
    remove_choice(emptyMap, 'outputs.mode', 'autosize')
    remove_independant_choices(emptyMap, 'outputs.mode', 'autosize')
    remove_choice(emptyMap, 'outputs.mode', 'autospacing')
    remove_independant_choices(emptyMap, 'outputs.mode', 'autospacing')
    old_app_name = emptyMap.find('key').text
    emptyMap.find('key').text = '%s-%s' % (old_app_name, 'fit-to-ortho')
    emptyMap.find('longname').text = '%s (%s)' % (old_app_name, 'fit-to-ortho')
    defaultWrite('%s-%s' % (available_app, 'fit-to-ortho'), emptyMap)
    the_list.append(emptyMap)

    return the_list


def getDimensionalityReduction(available_app, original_dom_document):
    """
    Remove rescale.outmin and rescale.outmax and split by method (ica, maf, napca and pca) and adjust parameters of each resulting app.
    """
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'rescale.outmin')
    remove_parameter_by_key(the_root, 'rescale.outmax')
    split = split_by_choice(the_root, 'method')
    the_list = []
    for key in split:
        if key == 'maf':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'outinv')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)
        else:
            defaultWrite('%s-%s' % (available_app, key), split[key])
            the_list.append(split[key])
    return the_list


def getPansharpening(available_app, original_dom_document):
    """
    Split by method (bayes, lmvm, rcs)
    """
    the_root = original_dom_document
    split = split_by_choice(the_root, 'method')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list


def getPixelValue(available_app, original_dom_document):
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'cl')
    defaultWrite(available_app, the_root)
    return [the_root]


def getExtractROI(available_app, original_dom_document):
    """
    Split by mode (standard, fit)
    Adapt parameters of each resulting app.
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'cl')
    deleteGeoidSrtm(the_root)
    split = split_by_choice(the_root, 'mode')
    the_list = []
    for key in split:
        if key == 'standard':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'mode.fit.elev.dem')
            remove_parameter_by_key(the_doc, 'mode.fit.elev.geoid')
            remove_parameter_by_key(the_doc, 'mode.fit.elev.default')
            remove_parameter_by_key(the_doc, 'mode.fit.ref')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)
        else:
            #key == 'fit'
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'startx')
            remove_parameter_by_key(the_doc, 'starty')
            remove_parameter_by_key(the_doc, 'sizex')
            remove_parameter_by_key(the_doc, 'sizey')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(split[key])
    return the_list


def getQuicklook(available_app, original_dom_document):
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'cl')
    defaultWrite(available_app, the_root)
    return [the_root]


def getRigidTransformResample(available_app, original_dom_document):
    """
    split by transformation (id, rotation, translation)
    """
    the_root = original_dom_document
    split = split_by_choice(the_root, 'transform.type')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list


def getHomologousPointsExtraction(available_app, original_dom_document):
    the_list = defaultSplit(available_app, original_dom_document, 'mode')
    return the_list


def getGenerateRPCSensorModel(available_app, original_dom_document):
    the_root = original_dom_document
    remove_dependant_choices(the_root, 'map', 'wgs')
    remove_other_choices(the_root, 'map', 'wgs')
    defaultWrite(available_app, the_root)
    return [the_root]


def getRefineSensorModel(available_app, original_dom_document):
    the_root = original_dom_document
    remove_dependant_choices(the_root, 'map', 'wgs')
    remove_other_choices(the_root, 'map', 'wgs')
    defaultWrite(available_app, the_root)
    return [the_root]


def getSegmentation(available_app, original_dom_document):
    """
    Remove the choice raster and split by filter (cc, edison, meanshift, mprofiles, watershed)
    """
    the_root = original_dom_document
    #remove_choice(the_root, 'filter', 'edison')
    #remove_independant_choices(the_root, 'filter', 'edison')
    #remove_choice(the_root, 'filter', 'meanshift')
    #remove_independant_choices(the_root, 'filter', 'meanshift')
    remove_choice(the_root, 'mode', 'raster')
    remove_independant_choices(the_root, 'mode', 'raster')
    split = split_by_choice(the_root, 'filter')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list


def getKMeansClassification(available_app, original_dom_document):
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'rand')
    defaultWrite(available_app, the_root)
    return [the_root]


def getTrainSVMImagesClassifier(available_app, original_dom_document):
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'rand')
    defaultWrite(available_app, the_root)
    return [the_root]


def getComputeConfusionMatrix(available_app, original_dom_document):
    """
    Split by ref (raster, vector)
    """
    the_root = original_dom_document
    #remove_independant_choices(the_root, 'ref', 'vector')
    #remove_choice(the_root, 'ref', 'vector')
    #defaultWrite(available_app, the_root)

    split = split_by_choice(the_root, 'ref')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list

    return [the_root]


def getOpticalCalibration(available_app, original_dom_document):
    """
    Remove toc options (let toa) and remove all about atmo
    """
    #the_list = defaultSplit(available_app, original_dom_document, 'level')
    the_root = original_dom_document
    remove_independant_choices(the_root, 'level', 'toc')
    remove_choice(the_root, 'level', 'toc')
    remove_parameter_by_key(the_root, 'atmo.aerosol')
    remove_parameter_by_key(the_root, 'atmo.oz')
    remove_parameter_by_key(the_root, 'atmo.wa')
    remove_parameter_by_key(the_root, 'atmo.pressure')
    remove_parameter_by_key(the_root, 'atmo.opt')
    remove_parameter_by_key(the_root, 'atmo.aeronet')
    remove_parameter_by_key(the_root, 'radius')
    defaultWrite(available_app, the_root)
    return [the_root]


def getSarRadiometricCalibration(available_app, original_dom_document):
    # TODO ** before doing anything, check support for SAR data in Qgis
    the_root = original_dom_document
    defaultWrite(available_app, the_root)
    return [the_root]


def getSmoothing(available_app, original_dom_document):
    """
    Split by type (anidif, gaussian, mean)
    """

    #import copy
    #the_root = copy.deepcopy(original_dom_document)
    #remove_dependant_choices(the_root, 'type', 'anidif')
    #remove_other_choices(the_root, 'type', 'anidif')
    #defaultWrite('%s-anidif' % available_app, the_root)

    #the_root = copy.deepcopy(original_dom_document)
    #remove_independant_choices(the_root, 'type', 'anidif')
    #remove_choice(the_root, 'type', 'anidif')
    #defaultWrite(available_app, the_root)

    the_root = original_dom_document
    split = split_by_choice(the_root, 'type')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])

    return the_list
    #split = split_by_choice(the_root, 'type')
    #the_list = []
    #for key in split:
    #    defaultWrite('%s-%s' % (available_app, key), split[key])
    #    the_list.append(split[key])
    #return the_list

def getColorMapping(available_app, original_dom_document):
    """
    Remove the option colortolabel
    Split by method : custom, continuous, optimal and image and adapt parameters of each resulting app
    """
    the_root = original_dom_document
    remove_independant_choices(the_root, 'op', 'colortolabel')
    remove_choice(the_root, 'op', 'colortolabel')
    split = split_by_choice(the_root, 'method')
    the_list = []
    for key in split:
        if key == 'custom':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'method.continuous.lut')
            remove_parameter_by_key(the_doc, 'method.continuous.min')
            remove_parameter_by_key(the_doc, 'method.continuous.max')
            remove_parameter_by_key(the_doc, 'method.optimal.background')
            remove_parameter_by_key(the_doc, 'method.image.in')
            remove_parameter_by_key(the_doc, 'method.image.low')
            remove_parameter_by_key(the_doc, 'method.image.up')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)
        elif key == 'continuous':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'method.custom.lut')
            remove_parameter_by_key(the_doc, 'method.optimal.background')
            remove_parameter_by_key(the_doc, 'method.image.in')
            remove_parameter_by_key(the_doc, 'method.image.low')
            remove_parameter_by_key(the_doc, 'method.image.up')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)
        elif key == 'optimal':
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'method.custom.lut')
            remove_parameter_by_key(the_doc, 'method.continuous.lut')
            remove_parameter_by_key(the_doc, 'method.continuous.min')
            remove_parameter_by_key(the_doc, 'method.continuous.max')
            remove_parameter_by_key(the_doc, 'method.image.in')
            remove_parameter_by_key(the_doc, 'method.image.low')
            remove_parameter_by_key(the_doc, 'method.image.up')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(the_doc)
        else:
            #key == 'image'
            the_doc = split[key]
            remove_parameter_by_key(the_doc, 'method.custom.lut')
            remove_parameter_by_key(the_doc, 'method.continuous.lut')
            remove_parameter_by_key(the_doc, 'method.continuous.min')
            remove_parameter_by_key(the_doc, 'method.continuous.max')
            remove_parameter_by_key(the_doc, 'method.optimal.background')
            defaultWrite('%s-%s' % (available_app, key), the_doc)
            the_list.append(split[key])
    return the_list



def getFusionOfClassifications(available_app, original_dom_document):
    """
    Split by method of fusion of classification (dempstershafer, majorityvoting)
    """
    the_root = original_dom_document
    split = split_by_choice(the_root, 'method')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list


def getTrainImagesClassifier(available_app, original_dom_document):
    """
    Split by  classifier (ann, bayes, boost, dt, gbt, knn, libsvm, rf, svm)
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    deleteGeoidSrtm(the_root)
    split = split_by_choice(the_root, 'classifier')
    the_list = []
    for key in split:
        defaultWrite('%s-%s' % (available_app, key), split[key])
        the_list.append(split[key])
    return the_list



def getLineSegmentDetection(available_app, original_dom_document):
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'elev.default')
    remove_parameter_by_key(the_root, 'elev.geoid')
    remove_parameter_by_key(the_root, 'elev.dem')
    defaultWrite(available_app, the_root)
    return [the_root]



def getImageEnvelope(available_app, original_dom_document):
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'elev.default')
    remove_parameter_by_key(the_root, 'elev.geoid')
    remove_parameter_by_key(the_root, 'elev.dem')
    defaultWrite(available_app, the_root)
    return [the_root]


def getReadImageInfo(available_app, original_dom_document):
    """
    Remove parameters that are output of the application.
    """
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'outkwl')
    remove_parameter_by_key(the_root, 'indexx')
    remove_parameter_by_key(the_root, 'indexy')
    remove_parameter_by_key(the_root, 'sizex')
    remove_parameter_by_key(the_root, 'sizey')
    remove_parameter_by_key(the_root, 'spacingx')
    remove_parameter_by_key(the_root, 'spacingy')
    remove_parameter_by_key(the_root, 'originx')
    remove_parameter_by_key(the_root, 'originy')
    remove_parameter_by_key(the_root, 'estimatedgroundspacingx')
    remove_parameter_by_key(the_root, 'estimatedgroundspacingy')
    remove_parameter_by_key(the_root, 'numberbands')
    remove_parameter_by_key(the_root, 'sensor')
    remove_parameter_by_key(the_root, 'id')
    remove_parameter_by_key(the_root, 'time')
    remove_parameter_by_key(the_root, 'ullat')
    remove_parameter_by_key(the_root, 'ullon')
    remove_parameter_by_key(the_root, 'urlat')
    remove_parameter_by_key(the_root, 'urlon')
    remove_parameter_by_key(the_root, 'lrlat')
    remove_parameter_by_key(the_root, 'lrlon')
    remove_parameter_by_key(the_root, 'lllat')
    remove_parameter_by_key(the_root, 'lllon')
    remove_parameter_by_key(the_root, 'town')
    remove_parameter_by_key(the_root, 'country')
    remove_parameter_by_key(the_root, 'rgb.r')
    remove_parameter_by_key(the_root, 'rgb.g')
    remove_parameter_by_key(the_root, 'rgb.b')
    remove_parameter_by_key(the_root, 'projectionref')
    remove_parameter_by_key(the_root, 'keyword')
    remove_parameter_by_key(the_root, 'gcp.count')
    remove_parameter_by_key(the_root, 'gcp.proj')
    defaultWrite(available_app, the_root)
    return [the_root]



def getComputeModulusAndPhase(available_app, original_dom_document):
    """
    Split the application according the field nbinput.
    For each of the resulting apps, give a new name.
    """
    the_root = original_dom_document
    split = split_by_choice(the_root, 'nbinput')
    the_list = []
    for key in split:
        if key == 'one':
            the_doc = split[key]
            old_app_name = the_doc.find('key').text
            the_doc.find('key').text = '%s-%s' % (old_app_name, 'OneEntry')
            the_doc.find('longname').text = '%s (%s)' % (old_app_name, 'OneEntry')
            defaultWrite('%s-%s' % (available_app, 'OneEntry'), the_doc)
            the_list.append(the_doc)
        else :
            the_doc = split[key]
            old_app_name = the_doc.find('key').text
            the_doc.find('key').text = '%s-%s' % (old_app_name, 'TwoEntries')
            the_doc.find('longname').text = '%s (%s)' % (old_app_name, 'TwoEntries')
            defaultWrite('%s-%s' % (available_app, 'TwoEntries'), the_doc)
            the_list.append(the_doc)
    return the_list


def getCompareImages(available_app, original_dom_document):
    """
    Remove mse, mae, psnr as they are output of the algorithm.
    """
    the_root = original_dom_document
    remove_parameter_by_key(the_root, 'mse')
    remove_parameter_by_key(the_root, 'mae')
    remove_parameter_by_key(the_root, 'psnr')
    defaultWrite(available_app, the_root)
    return [the_root]


def getRadiometricIndices(available_app, original_dom_document):
    """
    These 3 indices are missing. Remove them from the list.
    """
    the_root = original_dom_document
    remove_choice(the_root, 'list', 'laindvilog')
    remove_choice(the_root, 'list', 'lairefl')
    remove_choice(the_root, 'list', 'laindviformo')
    defaultWrite(available_app, the_root)
    return [the_root]


def getConnectedComponentSegmentation(available_app, original_dom_document):
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    deleteGeoidSrtm( the_root )
    defaultWrite(available_app, the_root)
    return [the_root]


def getKmzExport(available_app, original_dom_document):
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    deleteGeoidSrtm( the_root )
    defaultWrite(available_app, the_root)
    return [the_root]


def getSuperimpose(available_app, original_dom_document):
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    deleteGeoidSrtm( the_root )
    defaultWrite(available_app, the_root)
    return [the_root]


def getStereoFramework(available_app, original_dom_document):
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    the_root = original_dom_document
    deleteGeoidSrtm( the_root )
    defaultWrite(available_app, the_root)
    return [the_root]



def deleteGeoidSrtm(doc) :
    """
    Delete GEOID and DEM parameter as they are not updated at the creation of the otb algorithms when you launch QGIS.
    The values are picked from the settings.
    """
    t4 = [item for item in doc.findall('.//parameter') if item.find('key').text.endswith("elev.geoid")]
    for t5 in t4:
        doc.remove(t5)

    t4 = [item for item in doc.findall('.//parameter') if item.find('key').text.endswith("elev.dem")]
    for t5 in t4:
        doc.remove(t5)
