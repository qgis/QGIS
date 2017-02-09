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

from qgis.utils import iface
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import (QgsRasterLayer, QgsVectorLayer, QgsMapLayer, QgsCoordinateReferenceSystem,
                       QgsExpressionContext, QgsExpressionContextUtils, QgsExpression, QgsExpressionContextScope,
                       QgsProject)

from processing.tools.vector import resolveFieldIndex, features
from processing.tools import dataobjects
from processing.core.outputs import OutputNumber, OutputRaster, OutputVector
from processing.tools.dataobjects import getObject


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


def _createDescriptiveName(s):
    return s.replace('_', ' ')


def _expressionContext():
    context = QgsExpressionContext()
    context.appendScope(QgsExpressionContextUtils.globalScope())
    context.appendScope(QgsExpressionContextUtils.projectScope(QgsProject.instance()))

    if iface.mapCanvas():
        context.appendScope(QgsExpressionContextUtils.mapSettingsScope(iface.mapCanvas().mapSettings()))

    processingScope = QgsExpressionContextScope()

    extent = iface.mapCanvas().fullExtent()
    processingScope.setVariable('fullextent_minx', extent.xMinimum())
    processingScope.setVariable('fullextent_miny', extent.yMinimum())
    processingScope.setVariable('fullextent_maxx', extent.xMaximum())
    processingScope.setVariable('fullextent_maxy', extent.yMaximum())
    context.appendScope(processingScope)
    return context


def _resolveLayers(value):
    layers = dataobjects.getAllLayers()
    if value:
        inputlayers = value.split(';')
        for i, inputlayer in enumerate(inputlayers):
            for layer in layers:
                if layer.name() == inputlayer:
                    inputlayers[i] = layer.source()
                    break
        return ";".join(inputlayers)


class Parameter(object):

    """
    Base class for all parameters that a geoalgorithm might
    take as input.
    """

    default_metadata = {}

    def __init__(self, name='', description='', default=None, optional=False,
                 metadata={}):
        self.name = name
        self.description = description
        self.default = default
        self.value = default

        self.isAdvanced = False

        # A hidden parameter can be used to set a hard-coded value.
        # It can be used as any other parameter, but it will not be
        # shown to the user
        self.hidden = False

        self.optional = parseBool(optional)

        self.metadata = deepcopy(self.default_metadata)
        self.metadata.update(deepcopy(metadata))

    def setValue(self, obj):
        """
        Sets the value of the parameter.

        Returns true if the value passed is correct for the type
        of parameter.
        """
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        self.value = str(obj)
        return True

    def setDefaultValue(self):
        """
        Sets the value of the parameter to the default one

        Returns true if the default value is correct for the type
        of parameter.
        """
        return self.setValue(self.default)

    def __str__(self):
        return u'{} <{}>'.format(self.name, self.__class__.__name__)

    def getValueAsCommandLineParameter(self):
        """
        Returns the value of this parameter as it should have been
        entered in the console if calling an algorithm using the
        Processing.runalg() method.
        """
        return str(self.value)

    def typeName(self):
        return self.__class__.__name__.replace('Parameter', '').lower()

    def todict(self):
        o = deepcopy(self.__dict__)
        del o['metadata']
        return o

    def tr(self, string, context=''):
        if context == '':
            context = 'Parameter'
        return QCoreApplication.translate(context, string)

    def wrapper(self, dialog, row=0, col=0):
        wrapper = self.metadata.get('widget_wrapper', None)
        params = {}
        # wrapper metadata should be a dict with class key
        if isinstance(wrapper, dict):
            params = deepcopy(wrapper)
            wrapper = params.pop('class')
        # wrapper metadata should be a class path
        if isinstance(wrapper, str):
            tokens = wrapper.split('.')
            mod = __import__('.'.join(tokens[:-1]), fromlist=[tokens[-1]])
            wrapper = getattr(mod, tokens[-1])
        # or directly a class object
        if isclass(wrapper):
            wrapper = wrapper(self, dialog, row, col, **params)
        # or a wrapper instance
        return wrapper

    def evaluate(self, alg):
        pass

    def evaluateForModeler(self, value, model):
        return value


class ParameterBoolean(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.BooleanWidgetWrapper'
    }

    def __init__(self, name='', description='', default=None, optional=False, metadata={}):
        Parameter.__init__(self, name, description, parseBool(default), optional, metadata)

    def setValue(self, value):
        if value is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(value, str):
            self.value = str(value).lower() == str(True).lower()
        else:
            self.value = bool(value)
        return True

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'boolean '
        return '##' + self.name + '=' + param_type + str(self.default)

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.startswith("boolean"):
            descName = _createDescriptiveName(name)
            default = definition.strip()[len('boolean') + 1:] or None
            if default == 'None':
                default = None
            if default:
                param = ParameterBoolean(name, descName, default)
            else:
                param = ParameterBoolean(name, descName)
            param.optional = isOptional
            return param


class ParameterCrs(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.CrsWidgetWrapper'
    }

    def __init__(self, name='', description='', default=None, optional=False, metadata={}):
        '''The value is a string that uniquely identifies the
        coordinate reference system. Typically it is the auth id of the CRS
        (if the authority is EPSG) or proj4 string of the CRS (in case
        of other authorities or user defined projections).'''
        Parameter.__init__(self, name, description, default, optional, metadata)

    def setValue(self, value):
        if not bool(value):
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(value, QgsCoordinateReferenceSystem):
            self.value = value.authid()
            return True
        if isinstance(value, QgsMapLayer):
            self.value = value.crs().authid()
            return True
        try:
            layer = dataobjects.getObjectFromUri(value)
            if layer is not None:
                self.value = layer.crs().authid()
                return True
        except:
            pass

        # TODO: check it is a valid authid
        self.value = value
        return True

    def getValueAsCommandLineParameter(self):
        return '"' + str(self.value) + '"'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'crs '
        return '##' + self.name + '=' + param_type + str(self.default)

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.startswith("crs"):
            descName = _createDescriptiveName(name)
            default = definition.strip()[len('crs') + 1:]
            if default == 'None':
                default = None
            if default:
                return ParameterCrs(name, descName, default, isOptional)
            else:
                return ParameterCrs(name, descName, None, isOptional)


class ParameterDataObject(Parameter):

    def getValueAsCommandLineParameter(self):
        if self.value is None:
            return str(None)
        else:
            s = dataobjects.normalizeLayerSource(str(self.value))
            s = '"%s"' % s
            return s

    def evaluate(self, alg):
        self.value = _resolveLayers(self.value)


class ParameterExtent(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.ExtentWidgetWrapper'
    }

    USE_MIN_COVERING_EXTENT = 'USE_MIN_COVERING_EXTENT'

    def __init__(self, name='', description='', default=None, optional=True):
        Parameter.__init__(self, name, description, default, optional)
        # The value is a string in the form "xmin, xmax, ymin, ymax"

    def setValue(self, value):
        if not value:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(value, QgsMapLayer):
            rect = value.extent()
            self.value = '{},{},{},{}'.format(
                rect.xMinimum(), rect.xMaximum(), rect.yMinimum(), rect.yMaximum())
            return True

        try:
            layer = dataobjects.getObjectFromUri(value)
            if layer is not None:
                rect = layer.extent()
                self.value = '{},{},{},{}'.format(
                    rect.xMinimum(), rect.xMaximum(), rect.yMinimum(), rect.yMaximum())
                return True
        except:
            pass

        tokens = str(value).split(',')
        if len(tokens) != 4:
            return False
        try:
            float(tokens[0])
            float(tokens[1])
            float(tokens[2])
            float(tokens[3])
            self.value = value
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        if self.value is not None:
            return '"' + str(self.value) + '"'
        else:
            return str(None)

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'extent'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.startswith("extent"):
            descName = _createDescriptiveName(name)
            default = definition.strip()[len('extent') + 1:] or None
            return ParameterExtent(name, descName, default, isOptional)

    def evaluate(self, alg):
        if self.optional and not bool(self.value):
            self.value = self.getMinCoveringExtent(alg)

    def getMinCoveringExtent(self, alg):
        first = True
        found = False
        for param in alg.parameters:
            if param.value:
                if isinstance(param, (ParameterRaster, ParameterVector)):
                    if isinstance(param.value, (QgsRasterLayer,
                                                QgsVectorLayer)):
                        layer = param.value
                    else:
                        layer = dataobjects.getObject(param.value)
                    if layer:
                        found = True
                        self.addToRegion(layer, first)
                        first = False
                elif isinstance(param, ParameterMultipleInput):
                    layers = param.value.split(';')
                    for layername in layers:
                        layer = dataobjects.getObject(layername)
                        if layer:
                            found = True
                            self.addToRegion(layer, first)
                            first = False
        if found:
            return '{},{},{},{}'.format(
                self.xmin, self.xmax, self.ymin, self.ymax)
        else:
            return None

    def addToRegion(self, layer, first):
        if first:
            self.xmin = layer.extent().xMinimum()
            self.xmax = layer.extent().xMaximum()
            self.ymin = layer.extent().yMinimum()
            self.ymax = layer.extent().yMaximum()
        else:
            self.xmin = min(self.xmin, layer.extent().xMinimum())
            self.xmax = max(self.xmax, layer.extent().xMaximum())
            self.ymin = min(self.ymin, layer.extent().yMinimum())
            self.ymax = max(self.ymax, layer.extent().yMaximum())


class ParameterPoint(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.PointWidgetWrapper'
    }

    def __init__(self, name='', description='', default=None, optional=False):
        Parameter.__init__(self, name, description, default, optional)
        # The value is a string in the form "x, y"

    def setValue(self, text):
        if text is None:
            if not self.optional:
                return False
            self.value = None
            return True

        tokens = str(text).split(',')
        if len(tokens) != 2:
            return False
        try:
            float(tokens[0])
            float(tokens[1])
            self.value = text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return '"' + str(self.value) + '"'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'point'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.startswith("point"):
            descName = _createDescriptiveName(name)
            default = definition.strip()[len('point') + 1:] or None
            return ParameterPoint(name, descName, default, isOptional)


class ParameterFile(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.FileWidgetWrapper'
    }

    def __init__(self, name='', description='', isFolder=False, optional=True, ext=None):
        Parameter.__init__(self, name, description, None, parseBool(optional))
        self.ext = ext
        self.isFolder = parseBool(isFolder)

    def getValueAsCommandLineParameter(self):
        return '"' + str(self.value) + '"'

    def setValue(self, obj):
        if obj is None or obj.strip() == '':
            if not self.optional:
                return False
            self.value = None if obj is None else obj.strip()
            return True

        if self.ext is not None and obj != '' and not obj.endswith(self.ext):
            return False
        self.value = str(obj)
        return True

    def typeName(self):
        if self.isFolder:
            return 'directory'
        else:
            return 'file'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        if self.isFolder:
            param_type += 'folder'
        else:
            param_type += 'file'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.startswith("file") or definition.startswith("folder"):
            descName = _createDescriptiveName(name)
            return ParameterFile(name, descName, definition.startswith("folder"), isOptional)


class ParameterFixedTable(Parameter):

    def __init__(self, name='', description='', numRows=3,
                 cols=['value'], fixedNumOfRows=False, optional=False):
        Parameter.__init__(self, name, description, None, optional)
        self.cols = cols
        if isinstance(cols, str):
            self.cols = self.cols.split(";")
        self.numRows = int(numRows)
        self.fixedNumOfRows = parseBool(fixedNumOfRows)

    def setValue(self, obj):
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        # TODO: check that it contains a correct number of elements
        if isinstance(obj, str):
            self.value = obj
        else:
            self.value = ParameterFixedTable.tableToString(obj)
        return True

    def getValueAsCommandLineParameter(self):
        return '"' + str(self.value) + '"'

    @staticmethod
    def tableToString(table):
        tablestring = ''
        for i in range(len(table)):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ','
        tablestring = tablestring[:-1]
        return tablestring

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'fixedtable'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.startswith("fixedtable"):
            descName = _createDescriptiveName(name)
            default = definition.strip()[len('fixedtable') + 1:] or None
            return ParameterFixedTable(name, descName, optional=isOptional)


class ParameterMultipleInput(ParameterDataObject):

    """A parameter representing several data objects.

    Its value is a string with substrings separated by semicolons,
    each of which represents the data source location of each element.
    """

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.MultipleInputWidgetWrapper'
    }

    exported = None

    def __init__(self, name='', description='', datatype=-1, optional=False, metadata={}):
        ParameterDataObject.__init__(self, name, description, None, optional, metadata=metadata)
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

        if _minNumInputs < 1 and not self.optional:
            # don't allow to set negative or null number of inputs if parameter isn't optional
            return False

        self.minNumInputs = int(_minNumInputs)
        return True

    """ Get minimum required number of inputs for parameter

    @return: minimum number of inputs required for this parameter
    @see: setMinNumInputs()
    """

    def getMinNumInputs(self):
        return self.minNumInputs

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(obj, list):
            if len(obj) == 0:
                if self.optional:
                    self.value = None
                    return True
                else:
                    return False
            # prevent setting value if we didn't provide required minimal number of inputs
            elif len(obj) < self.minNumInputs:
                return False

            self.value = ";".join([self.getAsString(lay) for lay in obj])
            return True
        else:
            self.value = str(obj)
            return True

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

        if self.exported:
            return self.exported
        self.exported = self.value
        layers = self.value.split(';')
        if layers is None or len(layers) == 0:
            return self.value
        if self.datatype == dataobjects.TYPE_RASTER:
            for layerfile in layers:
                layer = dataobjects.getObjectFromUri(layerfile, False)
                if layer:
                    filename = dataobjects.exportRasterLayer(layer)
                    self.exported = self.exported.replace(layerfile, filename)
            return self.exported
        elif self.datatype == dataobjects.TYPE_FILE:
            return self.value
        else:
            for layerfile in layers:
                layer = dataobjects.getObjectFromUri(layerfile, False)
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
                layers = dataobjects.getRasterLayers()
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
                layers = dataobjects.getVectorLayers([self.datatype])
                for layer in layers:
                    if layer.name() == s:
                        return str(layer.source())
                return s

    def getFileFilter(self):
        if self.datatype == dataobjects.TYPE_RASTER:
            exts = dataobjects.getSupportedOutputRasterLayerExtensions()
        elif self.datatype == dataobjects.TYPE_FILE:
            return self.tr('All files (*.*)', 'ParameterMultipleInput')
        else:
            exts = dataobjects.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'ParameterMultipleInput') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

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

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        if self.datatype == dataobjects.TYPE_RASTER:
            param_type += 'multiple raster'
        if self.datatype == dataobjects.TYPE_FILE:
            param_type += 'multiple file'
        else:
            param_type += 'multiple vector'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip() == 'multiple raster':
            return ParameterMultipleInput(name, descName,
                                          dataobjects.TYPE_RASTER, isOptional)
        elif definition.lower().strip() == 'multiple vector':
            return ParameterMultipleInput(name, definition,
                                          dataobjects.TYPE_VECTOR_ANY, isOptional)

    def evaluate(self, alg):
        self.value = _resolveLayers(self.value)


class ParameterNumber(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.NumberWidgetWrapper'
    }

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

    def setValue(self, n):
        if n is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(n, str):
            try:
                v = self._evaluate(n)
                self.value = float(v)
                if self.isInteger:
                    self.value = int(math.floor(self.value))
                return True
            except:
                return False
        else:
            try:
                if float(n) - int(float(n)) == 0:
                    value = int(float(n))
                else:
                    value = float(n)
                if self.min is not None:
                    if value < self.min:
                        return False
                if self.max is not None:
                    if value > self.max:
                        return False
                self.value = value
                return True
            except:
                raise
                return False

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'number'
        code = '##' + self.name + '=' + param_type
        if self.default:
            code += str(self.default)
        return code

    @classmethod
    def fromScriptCode(self, line):

        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip().startswith('number'):
            default = definition.strip()[len('number'):] or None
            if default == 'None':
                default = None
            return ParameterNumber(name, descName, default=default, optional=isOptional)

    def _evaluate(self, value):
        exp = QgsExpression(value)
        if exp.hasParserError():
            raise ValueError(self.tr("Error in parameter expression: ") + exp.parserErrorString())
        result = exp.evaluate(_expressionContext())
        if exp.hasEvalError():
            raise ValueError("Error evaluating parameter expression: " + exp.evalErrorString())
        if self.isInteger:
            return math.floor(result)
        else:
            return result

    def evaluate(self, alg):
        if isinstance(self.value, str) and bool(self.value):
            self.value = self._evaluate(self.value)

    def _layerVariables(self, element, alg=None):
        variables = {}
        layer = getObject(element.value)
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

    def evaluateForModeler(self, value, model):
        if isinstance(value, numbers.Number):
            return value
        variables = {}
        for param in model.parameters:
            if isinstance(param, ParameterNumber):
                variables["@" + param.name] = param.value
            if isinstance(param, (ParameterRaster, ParameterVector)):
                variables.update(self._layerVariables(param))

        for alg in list(model.algs.values()):
            for out in alg.algorithm.outputs:
                if isinstance(out, OutputNumber):
                    variables["@%s_%s" % (alg.name, out.name)] = out.value
                if isinstance(out, (OutputRaster, OutputVector)):
                    variables.update(self._layerVariables(out, alg))
        for k, v in list(variables.items()):
            value = value.replace(k, str(v))

        return value

    def expressionContext(self):
        return _expressionContext()

    def getValueAsCommandLineParameter(self):
        if self.value is None:
            return str(None)
        if isinstance(self.value, str):
            return '"%s"' + self.value
        return str(self.value)


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

    def setValue(self, text):
        if text is None:
            if not self.optional:
                return False
            self.value = None
            return True

        tokens = text.split(',')
        if len(tokens) != 2:
            return False
        try:
            float(tokens[0])
            float(tokens[1])
            self.value = text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return '"' + str(self.value) + '"' if self.value is not None else str(None)


class ParameterRaster(ParameterDataObject):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.RasterWidgetWrapper'
    }

    def __init__(self, name='', description='', optional=False, showSublayersDialog=True):
        ParameterDataObject.__init__(self, name, description, None, optional)
        self.showSublayersDialog = parseBool(showSublayersDialog)
        self.exported = None

    def getSafeExportedLayer(self):
        """Returns not the value entered by the user, but a string with
        a filename which contains the data of this layer, but saved in
        a standard format (currently always a geotiff file) so that it
        can be opened by most external applications.

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

        if self.exported:
            return self.exported
        layer = dataobjects.getObjectFromUri(self.value, False)
        if layer:
            self.exported = dataobjects.exportRasterLayer(layer)
        else:
            self.exported = self.value
        return self.exported

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(obj, QgsRasterLayer):
            self.value = str(obj.dataProvider().dataSourceUri())
            return True
        else:
            self.value = str(obj)
            return True

    def getFileFilter(self):
        exts = dataobjects.getSupportedOutputRasterLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'ParameterRaster') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'raster'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip().startswith('raster'):
            return ParameterRaster(name, descName, optional=isOptional)


class ParameterSelection(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.SelectionWidgetWrapper'
    }

    def __init__(self, name='', description='', options=[], default=None, isSource=False,
                 multiple=False, optional=False):
        Parameter.__init__(self, name, description, default, optional)
        self.multiple = multiple
        isSource = parseBool(isSource)
        self.options = options
        if isSource:
            self.options = []
            layer = QgsVectorLayer(options[0], "layer", "ogr")
            if layer.isValid():
                try:
                    index = resolveFieldIndex(layer, options[1])
                    feats = features(layer)
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
        if default is not None:
            self.setValue(self.default)

    def setValue(self, value):
        if value is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(value, list):
            if not self.multiple:
                return False
            values = []
            for v in value:
                if v in self.values:
                    values.append(v)
                    continue
                try:
                    v = int(v)
                except:
                    pass
                if not v in self.values:
                    return False
                values.append(v)
            if not self.optional and len(values) == 0:
                return False
            self.value = values
            return True
        else:
            if value in self.values:
                self.value = value
                return True
            try:
                value = int(value)
            except:
                pass
            if not value in self.values:
                return False
            self.value = value
            return True

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
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

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.StringWidgetWrapper'
    }

    NEWLINE = '\n'
    ESCAPED_NEWLINE = '\\n'

    def __init__(self, name='', description='', default=None, multiline=False,
                 optional=False, evaluateExpressions=False, metadata={}):
        Parameter.__init__(self, name, description, default, optional, metadata)
        self.multiline = parseBool(multiline)
        self.evaluateExpressions = parseBool(evaluateExpressions)

    def setValue(self, obj):
        if not bool(obj):
            if not self.optional:
                return False
            self.value = None
            return True

        self.value = str(obj).replace(
            ParameterString.ESCAPED_NEWLINE,
            ParameterString.NEWLINE
        )
        return True

    def getValueAsCommandLineParameter(self):
        return ('"' + str(self.value.replace(ParameterString.NEWLINE,
                                             ParameterString.ESCAPED_NEWLINE)) + '"'
                if self.value is not None else str(None))

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'string '
        return '##' + self.name + '=' + param_type + repr(self.default)

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip().startswith('string'):
            default = definition.strip()[len('string') + 1:] or None
            if default == 'None':
                default = None
            elif default.startswith('"') or default.startswith('\''):
                default = eval(default)
            if default:
                return ParameterString(name, descName, default, optional=isOptional)
            else:
                return ParameterString(name, descName, optional=isOptional)
        elif definition.lower().strip().startswith('longstring'):
            default = definition.strip()[len('longstring') + 1:]
            if default:
                return ParameterString(name, descName, default, multiline=True, optional=isOptional)
            else:
                return ParameterString(name, descName, multiline=True, optional=isOptional)

    def evaluate(self, alg):
        if isinstance(self.value, str) and bool(self.value) and self.evaluateExpressions:
            exp = QgsExpression(self.value)
            if exp.hasParserError():
                raise ValueError(self.tr("Error in parameter expression: ") + exp.parserErrorString())
            result = exp.evaluate(_expressionContext())
            if exp.hasEvalError():
                raise ValueError("Error evaluating parameter expression: " + exp.evalErrorString())
            self.value = result

    def expressionContext(self):
        return _expressionContext()


class ParameterExpression(Parameter):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.ExpressionWidgetWrapper'
    }

    NEWLINE = '\n'
    ESCAPED_NEWLINE = '\\n'

    def __init__(self, name='', description='', default=None, optional=False, parent_layer=None):
        Parameter.__init__(self, name, description, default, optional)
        self.parent_layer = parent_layer

    def setValue(self, obj):
        if not bool(obj):
            if not self.optional:
                return False
            self.value = None
            return True

        self.value = str(obj).replace(
            ParameterString.ESCAPED_NEWLINE,
            ParameterString.NEWLINE
        )
        return True

    def getValueAsCommandLineParameter(self):
        return ('"' + str(self.value.replace(ParameterExpression.NEWLINE,
                                             ParameterExpression.ESCAPED_NEWLINE)) + '"'
                if self.value is not None else str(None))

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'expression '
        return '##' + self.name + '=' + param_type + str(self.default)

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        if definition.lower().strip().startswith('expression'):
            descName = _createDescriptiveName(name)
            default = definition.strip()[len('expression') + 1:] or None
            if default == 'None':
                default = None
            if default:
                return ParameterExpression(name, descName, default, optional=isOptional)
            else:
                return ParameterExpression(name, descName, optional=isOptional)


class ParameterTable(ParameterDataObject):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.TableWidgetWrapper'
    }

    def __init__(self, name='', description='', optional=False):
        ParameterDataObject.__init__(self, name, description, None, optional)
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(obj, QgsVectorLayer):
            source = str(obj.source())
            self.value = source
            return True
        else:
            self.value = str(obj)
            layers = dataobjects.getTables()
            for layer in layers:
                if layer.name() == self.value or layer.source() == self.value:
                    source = str(layer.source())
                    self.value = source
                    return True
            val = str(obj)
            self.value = val
            return os.path.exists(self.value)

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

        if self.exported:
            return self.exported
        table = dataobjects.getObjectFromUri(self.value, False)
        if table:
            self.exported = dataobjects.exportTable(table)
        else:
            self.exported = self.value
        return self.exported

    def getFileFilter(self):
        exts = ['csv', 'dbf']
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'ParameterTable') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'table'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip().startswith('table'):
            return ParameterTable(name, descName, isOptional)


class ParameterTableField(Parameter):

    """A parameter representing a table field.
    Its value is a string that represents the name of the field.
    """

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.TableFieldWidgetWrapper'
    }

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

    def getValueAsCommandLineParameter(self):
        return '"' + str(self.value) + '"' if self.value is not None else str(None)

    def setValue(self, value):
        if not bool(value):
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(value, list):
            if not self.multiple and len(value) > 1:
                return False
            self.value = ";".join(value)
            return True
        else:
            self.value = str(value)
        return True

    def __str__(self):
        return self.name + ' <' + self.__module__.split('.')[-1] + ' from ' \
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

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'field'
        return '##' + self.name + '=' + param_type + str(self.parent)

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip().startswith('field'):
            if definition.lower().strip().startswith('field number'):
                parent = definition.strip()[len('field number') + 1:]
                datatype = ParameterTableField.DATA_TYPE_NUMBER
            elif definition.lower().strip().startswith('field string'):
                parent = definition.strip()[len('field string') + 1:]
                datatype = ParameterTableField.DATA_TYPE_STRING
            elif definition.lower().strip().startswith('field datetime'):
                parent = definition.strip()[len('field datetime') + 1:]
                datatype = ParameterTableField.DATA_TYPE_DATETIME
            else:
                parent = definition.strip()[len('field') + 1:]
                datatype = ParameterTableField.DATA_TYPE_ANY

            return ParameterTableField(name, descName, parent, datatype, isOptional)


class ParameterVector(ParameterDataObject):

    default_metadata = {
        'widget_wrapper': 'processing.gui.wrappers.VectorWidgetWrapper'
    }

    def __init__(self, name='', description='', datatype=[-1],
                 optional=False):
        ParameterDataObject.__init__(self, name, description, None, optional)
        if isinstance(datatype, int):
            datatype = [datatype]
        elif isinstance(datatype, str):
            datatype = [int(t) for t in datatype.split(',')]
        self.datatype = datatype
        self.exported = None
        self.allowOnlyOpenedLayers = False

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(obj, QgsVectorLayer):
            self.value = str(obj.source())
            return True
        else:
            self.value = str(obj)
            return True

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

        if self.exported:
            return self.exported
        layer = dataobjects.getObjectFromUri(self.value, False)
        if layer:
            self.exported = dataobjects.exportVectorLayer(layer)
        else:
            self.exported = self.value
        return self.exported

    def getFileFilter(self):
        exts = dataobjects.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'ParameterVector') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def dataType(self):
        return dataobjects.vectorDataType(self)

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'vector'
        return '##' + self.name + '=' + param_type

    @classmethod
    def fromScriptCode(self, line):
        isOptional, name, definition = _splitParameterOptions(line)
        descName = _createDescriptiveName(name)
        if definition.lower().strip() == 'vector':
            return ParameterVector(name, descName,
                                   [dataobjects.TYPE_VECTOR_ANY], isOptional)
        elif definition.lower().strip() == 'vector point':
            return ParameterVector(name, descName,
                                   [dataobjects.TYPE_VECTOR_POINT], isOptional)
        elif definition.lower().strip() == 'vector line':
            return ParameterVector(name, descName,
                                   [dataobjects.TYPE_VECTOR_LINE], isOptional)
        elif definition.lower().strip() == 'vector polygon':
            return ParameterVector(name, descName,
                                   [dataobjects.TYPE_VECTOR_POLYGON], isOptional)


paramClasses = [c for c in list(sys.modules[__name__].__dict__.values()) if isclass(c) and issubclass(c, Parameter)]


def getParameterFromString(s):
    # Try the parameter definitions used in description files
    if '|' in s and (s.startswith("Parameter") or s.startswith("*Parameter")):
        isAdvanced = False
        if s.startswith("*"):
            s = s[1:]
            isAdvanced = True
        tokens = s.split("|")
        params = [t if str(t) != str(None) else None for t in tokens[1:]]
        try:
            clazz = getattr(sys.modules[__name__], tokens[0])
            param = clazz(*params)
            param.isAdvanced = isAdvanced
            return param
        except:
            return None
    else:  # try script syntax
        for paramClass in paramClasses:
            try:
                param = paramClass.fromScriptCode(s)
                if param is not None:
                    return param
            except:
                pass
