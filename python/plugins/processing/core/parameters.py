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
from builtins import str
from builtins import range
from builtins import object
from builtins import basestring

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys
import os
import math
from inspect import isclass
from copy import deepcopy
import numbers

from qgis.core import QgsProcessingUtils

from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsRasterLayer, QgsVectorLayer, QgsMapLayer, QgsCoordinateReferenceSystem,
                       QgsExpression,
                       QgsProject,
                       QgsRectangle,
                       QgsVectorFileWriter,
                       QgsProcessing,
                       QgsProcessingParameters,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterRasterLayer,
                       QgsProcessingParameterVectorLayer,
                       QgsProcessingParameterBoolean,
                       QgsProcessingParameterCrs,
                       QgsProcessingParameterRange,
                       QgsProcessingParameterPoint,
                       QgsProcessingParameterEnum,
                       QgsProcessingParameterExtent,
                       QgsProcessingParameterMatrix,
                       QgsProcessingParameterFile,
                       QgsProcessingParameterField,
                       QgsProcessingParameterVectorDestination,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingParameterString,
                       QgsProcessingParameterMultipleLayers,
                       QgsProcessingParameterFeatureSource,
                       QgsProcessingParameterNumber)

from processing.tools.vector import resolveFieldIndex
from processing.tools import dataobjects
from processing.core.outputs import OutputNumber, OutputRaster, OutputVector


def parseBool(s):
    if s is None or s == str(None).lower():
        return None
    return str(s).lower() == str(True).lower()


def _splitParameterOptions(line):
    tokens = line.split('=', 1)
    if tokens[1].lower().strip().startswith('optional'):
        isOptional = True
        definition = tokens[1].strip()[len('optional') + 1:]
    else:
        isOptional = False
        definition = tokens[1]
    return isOptional, tokens[0], definition


class Parameter(object):

    """
    Base class for all parameters that a geoalgorithm might
    take as input.
    """

    def __init__(self, name='', description='', default=None, optional=False,
                 metadata={}):
        self.value = default

    def __str__(self):
        return u'{} <{}>'.format(self.name(), self.__class__.__name__)

    def todict(self):
        o = deepcopy(self.__dict__)
        del o['metadata']
        return o

    def tr(self, string, context=''):
        if context == '':
            context = 'Parameter'
        return QCoreApplication.translate(context, string)


class ParameterBoolean(Parameter):

    def __init__(self, name='', description='', default=None, optional=False, metadata={}):
        Parameter.__init__(self, name, description, parseBool(default), optional, metadata)


class ParameterCrs(Parameter):

    def __init__(self, name='', description='', default=None, optional=False, metadata={}):
        '''The value is a string that uniquely identifies the
        coordinate reference system. Typically it is the auth id of the CRS
        (if the authority is EPSG) or proj4 string of the CRS (in case
        of other authorities or user defined projections).'''
        Parameter.__init__(self, name, description, default, optional, metadata)
        if self.value == 'ProjectCrs':
            self.value = QgsProject.instance().crs().authid()


class ParameterExtent(Parameter):

    USE_MIN_COVERING_EXTENT = 'USE_MIN_COVERING_EXTENT'

    def __init__(self, name='', description='', default=None, optional=True):
        Parameter.__init__(self, name, description, default, optional)
        # The value is a string in the form "xmin, xmax, ymin, ymax"
        self.skip_crs_check = False


class ParameterPoint(Parameter):

    def __init__(self, name='', description='', default=None, optional=False):
        Parameter.__init__(self, name, description, default, optional)
        # The value is a string in the form "x, y"


class ParameterFile(Parameter):

    def __init__(self, name='', description='', isFolder=False, optional=True, ext=None):
        Parameter.__init__(self, name, description, None, parseBool(optional))
        self.ext = ext
        self.isFolder = parseBool(isFolder)


class ParameterFixedTable(Parameter):

    def __init__(self, name='', description='', numRows=3,
                 cols=['value'], fixedNumOfRows=False, optional=False):
        Parameter.__init__(self, name, description, None, optional)
        self.cols = cols
        if isinstance(cols, str):
            self.cols = self.cols.split(";")
        self.numRows = int(numRows)
        self.fixedNumOfRows = parseBool(fixedNumOfRows)

    @staticmethod
    def tableToString(table):
        tablestring = ''
        for i in range(len(table)):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ','
        tablestring = tablestring[:-1]
        return tablestring


class ParameterMultipleInput(Parameter):

    """A parameter representing several data objects.

    Its value is a string with substrings separated by semicolons,
    each of which represents the data source location of each element.
    """

    exported = None

    def __init__(self, name='', description='', datatype=-1, optional=False, metadata={}):
        Parameter.__init__(self, name, description, None, optional, metadata=metadata)
        self.datatype = int(float(datatype))
        self.exported = None
        self.minNumInputs = 0

    """ Set minimum required number of inputs for parameter

    By default minimal number of inputs is set to 1

    @type _minNumInputs: numeric type or None
    @param _minNumInputs: required minimum number of inputs for parameter. \
                          If user will pass None as parameter, we will use default minimal number of inputs (1)
    @return: result, if the minimum number of inputs were set.
    """

    def setMinNumInputs(self, _minNumInputs):
        if _minNumInputs is None:
            self.minNumInputs = 0
            return True

        if _minNumInputs < 1 and not self.flags() & QgsProcessingParameterDefinition.FlagOptional:
            # don't allow setting negative or null number of inputs if parameter isn't optional
            return False

        self.minNumInputs = int(_minNumInputs)
        return True

    """ Get minimum required number of inputs for parameter

    @return: minimum number of inputs required for this parameter
    @see: setMinNumInputs()
    """

    def getMinNumInputs(self):
        return self.minNumInputs

    def getSafeExportedLayers(self):
        """
        Returns not the value entered by the user, but a string with
        semicolon-separated filenames which contains the data of the
        selected layers, but saved in a standard format (currently
        shapefiles for vector layers and GeoTiff for raster) so that
        they can be opened by most external applications.

        If there is a selection and QGIS is configured to use just the
        selection, it exports the layer even if it is already in a
        suitable format.

        Works only if the layer represented by the parameter value is
        currently loaded in QGIS. Otherwise, it will not perform any
        export and return the current value string.

        If the current value represents a layer in a suitable format,
        it does no export at all and returns that value.

        Currently, it works just for vector layer. In the case of
        raster layers, it returns the parameter value.

        The layers are exported just the first time the method is
        called. The method can be called several times and it will
        always return the same string, performing the export only the
        first time.
        """
        context = dataobjects.createContext()
        if self.exported:
            return self.exported
        self.exported = self.value
        layers = self.value.split(';')
        if layers is None or len(layers) == 0:
            return self.value
        if self.datatype == dataobjects.TYPE_RASTER:
            for layerfile in layers:
                layer = QgsProcessingUtils.mapLayerFromString(layerfile, context, False)
                if layer:
                    filename = dataobjects.exportRasterLayer(layer)
                    self.exported = self.exported.replace(layerfile, filename)
            return self.exported
        elif self.datatype == dataobjects.TYPE_FILE:
            return self.value
        else:
            for layerfile in layers:
                layer = QgsProcessingUtils.mapLayerFromString(layerfile, context, False)
                if layer:
                    filename = dataobjects.exportVectorLayer(layer)
                    self.exported = self.exported.replace(layerfile, filename)
            return self.exported

    def getAsString(self, value):
        if self.datatype == dataobjects.TYPE_RASTER:
            if isinstance(value, QgsRasterLayer):
                return str(value.dataProvider().dataSourceUri())
            else:
                s = str(value)
                layers = QgsProcessingUtils.compatibleRasterLayers(QgsProject.instance())
                for layer in layers:
                    if layer.name() == s:
                        return str(layer.dataProvider().dataSourceUri())
                return s

        if self.datatype == dataobjects.TYPE_FILE:
            return str(value)
        else:
            if isinstance(value, QgsVectorLayer):
                return str(value.source())
            else:
                s = str(value)
                if self.datatype != dataobjects.TYPE_VECTOR_ANY:
                    layers = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [self.datatype], False)
                else:
                    layers = QgsProcessingUtils.compatibleVectorLayers(QgsProject.instance(), [], False)
                for layer in layers:
                    if layer.name() == s:
                        return str(layer.source())
                return s

    def dataType(self):
        if self.datatype == dataobjects.TYPE_VECTOR_POINT:
            return 'points'
        elif self.datatype == dataobjects.TYPE_VECTOR_LINE:
            return 'lines'
        elif self.datatype == dataobjects.TYPE_VECTOR_POLYGON:
            return 'polygons'
        elif self.datatype == dataobjects.TYPE_RASTER:
            return 'rasters'
        elif self.datatype == dataobjects.TYPE_FILE:
            return 'files'
        else:
            return 'any vectors'


class ParameterNumber(Parameter):

    def __init__(self, name='', description='', minValue=None, maxValue=None,
                 default=None, optional=False, metadata={}):
        Parameter.__init__(self, name, description, default, optional, metadata)

        if default is not None:
            try:
                self.default = int(str(default))
                self.isInteger = True
            except ValueError:
                self.default = float(default)
                self.isInteger = False
        else:
            self.isInteger = False

        if minValue is not None:
            self.min = int(float(minValue)) if self.isInteger else float(minValue)
        else:
            self.min = None
        if maxValue is not None:
            self.max = int(float(maxValue)) if self.isInteger else float(maxValue)
        else:
            self.max = None
        self.value = self.default

    def _layerVariables(self, element, alg=None):
        variables = {}
        context = createContext()
        layer = QgsProcessingUtils.mapLayerFromString(element.value, context)
        if layer is not None:
            name = element.name if alg is None else "%s_%s" % (alg.name, element.name)
            variables['@%s_minx' % name] = layer.extent().xMinimum()
            variables['@%s_miny' % name] = layer.extent().yMinimum()
            variables['@%s_maxx' % name] = layer.extent().yMaximum()
            variables['@%s_maxy' % name] = layer.extent().yMaximum()
            if isinstance(element, (ParameterRaster, OutputRaster)):
                stats = layer.dataProvider().bandStatistics(1)
                variables['@%s_avg' % name] = stats.mean
                variables['@%s_stddev' % name] = stats.stdDev
                variables['@%s_min' % name] = stats.minimumValue
                variables['@%s_max' % name] = stats.maximumValue
        return variables


class ParameterRange(Parameter):

    def __init__(self, name='', description='', default=None, optional=False):
        Parameter.__init__(self, name, description, default, optional)

        if default is not None:
            values = default.split(',')
            try:
                int(values[0])
                int(values[1])
                self.isInteger = True
            except:
                self.isInteger = False
        else:
            self.isInteger = False


class ParameterRaster(Parameter):

    def __init__(self, name='', description='', optional=False, showSublayersDialog=True):
        Parameter.__init__(self, name, description, None, optional)
        self.showSublayersDialog = parseBool(showSublayersDialog)


class ParameterSelection(Parameter):

    def __init__(self, name='', description='', options=[], default=None, isSource=False,
                 multiple=False, optional=False, metadata={}):
        Parameter.__init__(self, name, description, default, optional, metadata)
        self.multiple = multiple
        isSource = parseBool(isSource)
        self.options = options
        if isSource:
            self.options = []
            layer = QgsVectorLayer(options[0], "layer", "ogr")
            if layer.isValid():
                try:
                    index = resolveFieldIndex(layer, options[1])
                    feats = QgsProcessingUtils.getFeatures(layer, dataobjects.createContext())
                    for feature in feats:
                        self.options.append(str(feature.attributes()[index]))
                except ValueError:
                    pass
        elif isinstance(self.options, str):
            self.options = self.options.split(";")

        # compute options as (value, text)
        options = []
        for i, option in enumerate(self.options):
            if option is None or isinstance(option, basestring):
                options.append((i, option))
            else:
                options.append((option[0], option[1]))
        self.options = options
        self.values = [option[0] for option in options]

        self.value = None

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = QgsProcessingParameters.descriptionFromName(name)
        if definition.lower().strip().startswith('selectionfromfile'):
            options = definition.strip()[len('selectionfromfile '):].split(';')
            return ParameterSelection(name, descName, options, isSource=True, optional=isOptional)
        elif definition.lower().strip().startswith('selection'):
            options = definition.strip()[len('selection '):].split(';')
            return ParameterSelection(name, descName, options, optional=isOptional)
        elif definition.lower().strip().startswith('multipleselectionfromfile'):
            options = definition.strip()[len('multipleselectionfromfile '):].split(';')
            return ParameterSelection(name, descName, options, isSource=True,
                                      multiple=True, optional=isOptional)
        elif definition.lower().strip().startswith('multipleselection'):
            options = definition.strip()[len('multipleselection '):].split(';')
            return ParameterSelection(name, descName, options, multiple=True, optional=isOptional)


class ParameterEvaluationException(Exception):

    def __init__(self, param, msg):
        Exception.__init__(msg)
        self.param = param


class ParameterString(Parameter):

    def __init__(self, name='', description='', default=None, multiline=False,
                 optional=False, evaluateExpressions=False, metadata={}):
        Parameter.__init__(self, name, description, default, optional, metadata)
        self.multiline = parseBool(multiline)


class ParameterExpression(Parameter):

    def __init__(self, name='', description='', default=None, optional=False, parent_layer=None):
        Parameter.__init__(self, name, description, default, optional)
        self.parent_layer = parent_layer


class ParameterTable(Parameter):

    def __init__(self, name='', description='', optional=False):
        Parameter.__init__(self, name, description, None, optional)
        self.exported = None

    def getSafeExportedTable(self):
        """Returns not the value entered by the user, but a string with
        a filename which contains the data of this table, but saved in
        a standard format (currently always a DBF file) so that it can
        be opened by most external applications.

        Works only if the table represented by the parameter value is
        currently loaded in QGIS. Otherwise, it will not perform any
        export and return the current value string.

        If the current value represents a table in a suitable format,
        it does not export at all and returns that value.

        The table is exported just the first time the method is called.
        The method can be called several times and it will always
        return the same file, performing the export only the first
        time.
        """
        context = dataobjects.createContext()
        if self.exported:
            return self.exported
        table = QgsProcessingUtils.mapLayerFromString(self.value, context, False)
        if table:
            self.exported = dataobjects.exportTable(table)
        else:
            self.exported = self.value
        return self.exported


class ParameterTableField(Parameter):

    """A parameter representing a table field.
    Its value is a string that represents the name of the field.
    """

    DATA_TYPE_NUMBER = 0
    DATA_TYPE_STRING = 1
    DATA_TYPE_DATETIME = 2
    DATA_TYPE_ANY = -1

    def __init__(self, name='', description='', parent=None, datatype=-1,
                 optional=False, multiple=False):
        Parameter.__init__(self, name, description, None, optional)
        self.parent = parent
        self.multiple = multiple
        self.datatype = int(datatype)

    def __str__(self):
        return self.name() + ' <' + self.__module__.split('.')[-1] + ' from ' \
            + self.parent + '>'

    def dataType(self):
        if self.datatype == self.DATA_TYPE_NUMBER:
            return 'numeric'
        elif self.datatype == self.DATA_TYPE_STRING:
            return 'string'
        elif self.datatype == self.DATA_TYPE_DATETIME:
            return 'datetime'
        else:
            return 'any'


class ParameterVector(Parameter):

    def __init__(self, name='', description='', datatype=[-1],
                 optional=False):
        Parameter.__init__(self, name, description, None, optional)
        if isinstance(datatype, int):
            datatype = [datatype]
        elif isinstance(datatype, str):
            datatype = [int(t) for t in datatype.split(',')]
        self.datatype = datatype
        self.exported = None
        self.allowOnlyOpenedLayers = False

    def getSafeExportedLayer(self):
        """Returns not the value entered by the user, but a string with
        a filename which contains the data of this layer, but saved in
        a standard format (currently always a shapefile) so that it can
        be opened by most external applications.

        If there is a selection and QGIS is configured to use just the
        selection, if exports the layer even if it is already in a
        suitable format.

        Works only if the layer represented by the parameter value is
        currently loaded in QGIS. Otherwise, it will not perform any
        export and return the current value string.

        If the current value represents a layer in a suitable format,
        it does not export at all and returns that value.

        The layer is exported just the first time the method is called.
        The method can be called several times and it will always
        return the same file, performing the export only the first
        time.
        """
        context = dataobjects.createContext()

        if self.exported:
            return self.exported
        layer = QgsProcessingUtils.mapLayerFromString(self.value, context, False)
        if layer:
            self.exported = dataobjects.exportVectorLayer(layer)
        else:
            self.exported = self.value
        return self.exported

    def dataType(self):
        return dataobjects.vectorDataType(self)


paramClasses = [c for c in list(sys.modules[__name__].__dict__.values()) if isclass(c) and issubclass(c, Parameter)]


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
                    params[2] = QgsProcessingParameterNumber.Integer if params[2].lower().endswith('integer') else QgsProcessingParameterNumber.Double
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
                        params[4] = [int(v) for v in params[4].split(',')]
                    else:
                        params[4] = int(params[4])
                if len(params) > 5:
                    params[5] = True if params[5].lower() == 'true' else False
            elif clazz == QgsProcessingParameterFeatureSource:
                if len(params) > 2:
                    params[2] = [int(p) for p in params[2].split(';')]
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterMultipleLayers:
                if len(params) > 2:
                    params[2] = int(params[2])
                if len(params) > 4:
                    params[4] = True if params[4].lower() == 'true' else False
            elif clazz == QgsProcessingParameterMatrix:
                if len(params) > 2:
                    params[2] = int(params[2])
                if len(params) > 3:
                    params[3] = True if params[3].lower() == 'true' else False
                if len(params) > 4:
                    params[4] = params[4].split(';')
            elif clazz == QgsProcessingParameterField:
                if len(params) > 4:
                    params[4] = int(params[4])
                if len(params) > 5:
                    params[5] = True if params[5].lower() == 'true' else False
                if len(params) > 6:
                    params[6] = True if params[6].lower() == 'true' else False
            elif clazz == QgsProcessingParameterFile:
                if len(params) > 2:
                    params[2] = QgsProcessingParameterFile.File if params[2].lower() == 'false' else QgsProcessingParameterFile.Folder
                if len(params) > 5:
                    params[5] = True if params[5].lower() == 'true' else False
            elif clazz == QgsProcessingParameterNumber:
                if len(params) > 2:
                    params[2] = QgsProcessingParameterNumber.Integer if params[2].lower().endswith('int') else QgsProcessingParameterNumber.Double
                if len(params) > 3:
                    params[3] = float(params[3].strip()) if params[3] is not None else 0
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
                    if params[2].lower().endswith('point'):
                        params[2] = QgsProcessing.TypeVectorPoint
                    elif params[2].lower().endswith('line'):
                        params[2] = QgsProcessing.TypeVectorLine
                    elif params[2].lower().endswith('polygon'):
                        params[2] = QgsProcessing.TypeVectorPolygon
                    elif params[2].lower().endswith('geometry'):
                        params[2] = QgsProcessing.TypeVectorAnyGeometry
                    elif params[2].lower().endswith('vector'):
                        params[2] = QgsProcessing.TypeVector
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

        # try Python duck-typed method
        for paramClass in paramClasses:
            try:
                param = paramClass.fromScriptCode(s)
                if param is not None:
                    return param
            except:
                pass
