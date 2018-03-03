# -*- coding: utf-8 -*-

"""
***************************************************************************
    Parameters.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys

from qgis.core import (QgsRasterLayer,
                       QgsVectorLayer,
                       QgsMapLayer,
                       QgsCoordinateReferenceSystem,
                       QgsExpression,
                       QgsProject,
                       QgsRectangle,
                       QgsVectorFileWriter,
                       QgsProcessing,
                       QgsProcessingUtils,
                       QgsProcessingParameters,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterBand,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterRange,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterExpression,
                       QgsProcessingParameterMatrix,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterField,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterString,
                       QgsProcessingParameterMapLayer,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber)

from PyQt5.QtCore import QCoreApplication

PARAMETER_NUMBER = 'Number'
PARAMETER_RASTER = 'Raster Layer'
PARAMETER_TABLE = 'Vector Layer'
PARAMETER_VECTOR = 'Vector Features'
PARAMETER_STRING = 'String'
PARAMETER_EXPRESSION = 'Expression'
PARAMETER_BOOLEAN = 'Boolean'
PARAMETER_TABLE_FIELD = 'Vector Field'
PARAMETER_EXTENT = 'Extent'
PARAMETER_FILE = 'File'
PARAMETER_POINT = 'Point'
PARAMETER_CRS = 'CRS'
PARAMETER_MULTIPLE = 'Multiple Input'
PARAMETER_BAND = 'Raster Band'
PARAMETER_MAP_LAYER = 'Map Layer'
PARAMETER_RANGE = 'Range'
PARAMETER_ENUM = 'Enum'
PARAMETER_MATRIX = 'Matrix'
PARAMETER_VECTOR_DESTINATION = 'Vector Destination'
PARAMETER_FILE_DESTINATION = 'File Destination'
PARAMETER_FOLDER_DESTINATION = 'Folder Destination'
PARAMETER_RASTER_DESTINATION = 'Raster Destination'


def getParameterFromString(s):
    # Try the parameter definitions used in description files
    if '|' in s and (s.startswith("QgsProcessingParameter") or s.startswith("*QgsProcessingParameter") or s.startswith('Parameter') or s.startswith('*Parameter')):
        isAdvanced = False
        if s.startswith("*"):
            s = s[1:]
            isAdvanced = True
        tokens = s.split("|")
        params = [t if str(t) != str(None) else None for t in tokens[1:]]

        if True:
            clazz = getattr(sys.modules[__name__], tokens[0])
            # convert to correct type
            if clazz == QgsProcessingParameterRasterLayer:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterVectorLayer:
                if len(params) > 2:
                    params[2] = [int(p) for p in params[2].split(';')]
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterBoolean:
                if len(params) > 2:
                    params[2] = True if params[2].lower() == 'true' else False
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterPoint:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterCrs:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterRange:
                if len(params) > 2:
                    try:
                        params[2] = int(params[2])
                    except:
                        params[2] = getattr(QgsProcessingParameterNumber, params[2].split(".")[1])
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterExtent:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterEnum:
                if len(params) > 2:
                    params[2] = params[2].split(';')
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
                if len(params) > 4:
                    # For multiple values; default value is a list of int
                    if params[3] == True:
                        params[4] = [int(v) for v in params[4].split(',')] if params[4] is not None else None
                    else:
                        params[4] = int(params[4]) if params[4] is not None else None
                if len(params) > 5:
                    params[5] = True if params[5].lower() == 'true' else False
            elif clazz == QgsProcessingParameterFeatureSource:
                if len(params) > 2:
                    try:
                        params[2] = [int(p) for p in params[2].split(';')]
                    except:
                        params[2] = [getattr(QgsProcessing, p.split(".")[1]) for p in params[2].split(';')]
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterMultipleLayers:
                if len(params) > 2:
                    try:
                        params[2] = int(params[2])
                    except:
                        params[2] = getattr(QgsProcessing, params[2].split(".")[1])
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterMatrix:
                if len(params) > 2:
                    params[2] = int(params[2])
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
                if len(params) > 4:
                    params[4] = params[4].split(';')
                if len(params) > 6:
                    params[6] = True if params[6].lower() == 'true' else False
            elif clazz == QgsProcessingParameterField:
                if len(params) > 4:
                    try:
                        params[4] = int(params[4])
                    except:
                        params[4] = getattr(QgsProcessingParameterField, params[4].split(".")[1])
                if len(params) > 5:
                    params[5] = True if params[5].lower() == 'true' else False
                if len(params) > 6:
                    params[6] = True if params[6].lower() == 'true' else False
            elif clazz == QgsProcessingParameterFile:
                if len(params) > 2:
                    try:
                        params[2] = int(params[2])
                    except:
                        params[2] = getattr(QgsProcessingParameterFile, params[2].split(".")[1])
                if len(params) > 5:
                    params[5] = True if params[5].lower() == 'true' else False
            elif clazz == QgsProcessingParameterNumber:
                if len(params) > 2:
                    try:
                        params[2] = int(params[2])
                    except:
                        params[2] = getattr(QgsProcessingParameterNumber, params[2].split(".")[1])
                if len(params) > 3:
                    params[3] = float(params[3].strip()) if params[3] is not None else None
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
                if len(params) > 5:
                    params[5] = float(params[5].strip()) if params[5] is not None else -sys.float_info.max + 1
                if len(params) > 6:
                    params[6] = float(params[6].strip()) if params[6] is not None else sys.float_info.max - 1
            elif clazz == QgsProcessingParameterString:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterFileDestination:
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterFolderDestination:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterRasterDestination:
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
            elif clazz == QgsProcessingParameterVectorDestination:
                if len(params) > 2:
                    try:
                        params[2] = int(params[2])
                    except:
                        params[2] = getattr(QgsProcessing, params[2].split(".")[1])
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False

            param = clazz(*params)
            if isAdvanced:
                param.setFlags(param.flags() | QgsProcessingParameterDefinition.FlagAdvanced)

            return param
        else:
            return None
    else:  # try script syntax

        # try native method
        param = QgsProcessingParameters.parameterFromScriptCode(s)
        if param:
            return param


def initializeParameters():
    from processing.core.Processing import Processing

    Processing.registerParameter(PARAMETER_MAP_LAYER, QCoreApplication.translate('Processing', 'Map Layer'),
                                 QgsProcessingParameterMapLayer,
                                 description=QCoreApplication.translate('Processing', 'A generic map layer parameter, which accepts either vector or raster layers.'))
    Processing.registerParameter(PARAMETER_BAND, QCoreApplication.translate('Processing', 'Raster Band'),
                                 QgsProcessingParameterBand,
                                 description=QCoreApplication.translate('Processing', 'A raster band parameter, for selecting an existing band from a raster source.'))
    Processing.registerParameter(PARAMETER_EXPRESSION, QCoreApplication.translate('Processing', 'Expression'),
                                 QgsProcessingParameterExpression,
                                 description=QCoreApplication.translate('Processing', 'A QGIS expression parameter, which presents an expression builder widget to users.'))
    Processing.registerParameter(PARAMETER_RASTER, QCoreApplication.translate('Processing', 'Raster Layer'), QgsProcessingParameterRasterLayer,
                                 description=QCoreApplication.translate('Processing', 'A raster layer parameter.'))
    Processing.registerParameter(PARAMETER_TABLE, QCoreApplication.translate('Processing', 'Vector Layer'), QgsProcessingParameterVectorLayer,
                                 description=QCoreApplication.translate('Processing', 'A vector feature parameter, e.g. for algorithms which operate on the features within a layer.'))

    Processing.registerParameter(PARAMETER_VECTOR_DESTINATION, QCoreApplication.translate('Processing', 'Vector Destination'), QgsProcessingParameterVectorDestination, exposeToModeller=False)
    Processing.registerParameter(PARAMETER_FILE_DESTINATION, QCoreApplication.translate('Processing', 'File Destination'), QgsProcessingParameterFileDestination, exposeToModeller=False)
    Processing.registerParameter(PARAMETER_FOLDER_DESTINATION, QCoreApplication.translate('Processing', 'Folder Destination'), QgsProcessingParameterFolderDestination, exposeToModeller=False)
    Processing.registerParameter(PARAMETER_RASTER_DESTINATION, QCoreApplication.translate('Processing', 'Raster Destination'), QgsProcessingParameterRasterDestination, exposeToModeller=False)
    Processing.registerParameter(PARAMETER_STRING, QCoreApplication.translate('Processing', 'String'), QgsProcessingParameterString,
                                 description=QCoreApplication.translate('Processing', 'A freeform string parameter.'))
    Processing.registerParameter(PARAMETER_MULTIPLE, QCoreApplication.translate('Processing', 'Multiple Layers'), QgsProcessingParameterMultipleLayers,
                                 description=QCoreApplication.translate('Processing', 'An input allowing selection of multiple sources, including multiple map layers or file sources.'))
    Processing.registerParameter(PARAMETER_VECTOR, QCoreApplication.translate('Processing', 'Feature Source'), QgsProcessingParameterFeatureSource)
    Processing.registerParameter(PARAMETER_NUMBER, QCoreApplication.translate('Processing', 'Number'), QgsProcessingParameterNumber,
                                 description=QCoreApplication.translate('Processing', 'A numeric parameter, including float or integer values.'))
    Processing.registeredParameters()
