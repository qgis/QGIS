
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
import os

from processing.tools.vector import resolveFieldIndex
from processing.tools.vector import features
from qgis.PyQt.QtCore import QCoreApplication
from qgis.core import QgsRasterLayer
from qgis.core import QgsVectorLayer
from processing.tools.system import isWindows
from processing.tools import dataobjects


def getParameterFromString(s):
    tokens = s.split("|")
    params = [t if unicode(t) != unicode(None) else None for t in tokens[1:]]
    clazz = getattr(sys.modules[__name__], tokens[0])
    return clazz(*params)


def parseBool(s):
    if s is None or s == unicode(None).lower():
        return None
    return unicode(s).lower() == unicode(True).lower()


class Parameter:

    """
    Base class for all parameters that a geoalgorithm might
    take as input.
    """

    def __init__(self, name='', description='', default=None, optional=False):
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

        self.value = unicode(obj)
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
        return unicode(self.value)

    def typeName(self):
        return self.__class__.__name__.replace('Parameter', '').lower()

    def todict(self):
        return self.__dict__

    def tr(self, string, context=''):
        if context == '':
            context = 'Parameter'
        return QCoreApplication.translate(context, string)


class ParameterBoolean(Parameter):

    def __init__(self, name='', description='', default=None, optional=False):
        Parameter.__init__(self, name, description, parseBool(default), optional)

    def setValue(self, value):
        if value is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(value, basestring):
            self.value = unicode(value).lower() == unicode(True).lower()
        else:
            self.value = bool(value)
        return True

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'boolean '
        return '##' + self.name + '=' + param_type + str(self.default)


class ParameterCrs(Parameter):

    def __init__(self, name='', description='', default=None, optional=False):
        '''The value is a string that uniquely identifies the
        coordinate reference system. Typically it is the auth id of the CRS
        (if the authority is EPSG) or proj4 string of the CRS (in case
        of other authorities or user defined projections).'''
        Parameter.__init__(self, name, description, default, optional)

    def setValue(self, value):
        if value is None or value.strip() == '':
            if not self.optional:
                return False
            self.value = None if value is None else value.strip()
            return True

        # TODO: check it is a valid authid
        self.value = unicode(value)
        return True

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'crs '
        return '##' + self.name + '=' + param_type + str(self.default)


class ParameterDataObject(Parameter):

    def getValueAsCommandLineParameter(self):
        if self.value is None:
            return unicode(None)
        else:
            s = dataobjects.normalizeLayerSource(unicode(self.value))
            s = '"%s"' % s
            return s


class ParameterExtent(Parameter):

    USE_MIN_COVERING_EXTENT = 'USE_MIN_COVERING_EXTENT'

    def __init__(self, name='', description='', default=None, optional=False):
        Parameter.__init__(self, name, description, default, optional)
        # The value is a string in the form "xmin, xmax, ymin, ymax"

    def setValue(self, text):
        if text is None:
            if not self.optional:
                return False
            self.value = None
            return True

        tokens = unicode(text).split(',')
        if len(tokens) != 4:
            return False
        try:
            float(tokens[0])
            float(tokens[1])
            float(tokens[2])
            float(tokens[3])
            self.value = text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'extent'
        return '##' + self.name + '=' + param_type


class ParameterPoint(Parameter):

    def __init__(self, name='', description='', default=None, optional=False):
        Parameter.__init__(self, name, description, default, optional)
        # The value is a string in the form "x, y"

    def setValue(self, text):
        if text is None:
            if not self.optional:
                return False
            self.value = None
            return True

        tokens = unicode(text).split(',')
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
        return '"' + unicode(self.value) + '"'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'point'
        return '##' + self.name + '=' + param_type


class ParameterFile(Parameter):

    def __init__(self, name='', description='', isFolder=False, optional=True, ext=None):
        Parameter.__init__(self, name, description, None, parseBool(optional))
        self.ext = ext
        self.isFolder = parseBool(isFolder)

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def setValue(self, obj):
        if obj is None or obj.strip() == '':
            if not self.optional:
                return False
            self.value = None if obj is None else obj.strip()
            return True

        if self.ext is not None and obj != '' and not obj.endswith(self.ext):
            return False
        self.value = unicode(obj)
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


class ParameterFixedTable(Parameter):

    def __init__(self, name='', description='', numRows=3,
                 cols=['value'], fixedNumOfRows=False, optional=False):
        Parameter.__init__(self, name, description, None, optional)
        self.cols = cols
        if isinstance(cols, basestring):
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
        if isinstance(obj, (str, unicode)):
            self.value = obj
        else:
            self.value = ParameterFixedTable.tableToString(obj)
        return True

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    @staticmethod
    def tableToString(table):
        tablestring = ''
        for i in range(len(table)):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ','
        tablestring = tablestring[:-1]
        return tablestring


class ParameterMultipleInput(ParameterDataObject):

    """A parameter representing several data objects.

    Its value is a string with substrings separated by semicolons,
    each of which represents the data source location of each element.
    """

    exported = None

    TYPE_VECTOR_ANY = -1
    TYPE_VECTOR_POINT = 0
    TYPE_VECTOR_LINE = 1
    TYPE_VECTOR_POLYGON = 2
    TYPE_RASTER = 3
    TYPE_FILE = 4

    def __init__(self, name='', description='', datatype=-1, optional=False):
        ParameterDataObject.__init__(self, name, description, None, optional)
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
            self.value = unicode(obj)
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
        if self.datatype == ParameterMultipleInput.TYPE_RASTER:
            for layerfile in layers:
                layer = dataobjects.getObjectFromUri(layerfile, False)
                if layer:
                    filename = dataobjects.exportRasterLayer(layer)
                    self.exported = self.exported.replace(layerfile, filename)
            return self.exported
        elif self.datatype == ParameterMultipleInput.TYPE_FILE:
            return self.value
        else:
            for layerfile in layers:
                layer = dataobjects.getObjectFromUri(layerfile, False)
                if layer:
                    filename = dataobjects.exportVectorLayer(layer)
                    self.exported = self.exported.replace(layerfile, filename)
            return self.exported

    def getAsString(self, value):
        if self.datatype == ParameterMultipleInput.TYPE_RASTER:
            if isinstance(value, QgsRasterLayer):
                return unicode(value.dataProvider().dataSourceUri())
            else:
                s = unicode(value)
                layers = dataobjects.getRasterLayers()
                for layer in layers:
                    if layer.name() == s:
                        return unicode(layer.dataProvider().dataSourceUri())
                return s

        if self.datatype == ParameterMultipleInput.TYPE_FILE:
            return unicode(value)
        else:
            if isinstance(value, QgsVectorLayer):
                return unicode(value.source())
            else:
                s = unicode(value)
                layers = dataobjects.getVectorLayers([self.datatype])
                for layer in layers:
                    if layer.name() == s:
                        return unicode(layer.source())
                return s

    def getFileFilter(self):
        if self.datatype == ParameterMultipleInput.TYPE_RASTER:
            exts = dataobjects.getSupportedOutputRasterLayerExtensions()
        elif self.datatype == ParameterMultipleInput.TYPE_FILE:
            return self.tr('All files (*.*)', 'ParameterMultipleInput')
        else:
            exts = dataobjects.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'ParameterMultipleInput') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def dataType(self):
        if self.datatype == self.TYPE_VECTOR_POINT:
            return 'points'
        elif self.datatype == self.TYPE_VECTOR_LINE:
            return 'lines'
        elif self.datatype == self.TYPE_VECTOR_POLYGON:
            return 'polygons'
        elif self.datatype == self.TYPE_RASTER:
            return 'rasters'
        elif self.datatype == self.TYPE_FILE:
            return 'files'
        else:
            return 'any vectors'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        if self.datatype == self.TYPE_RASTER:
            param_type += 'multiple raster'
        if self.datatype == self.TYPE_FILE:
            param_type += 'multiple file'
        else:
            param_type += 'multiple vector'
        return '##' + self.name + '=' + param_type


class ParameterNumber(Parameter):

    def __init__(self, name='', description='', minValue=None, maxValue=None,
                 default=None, optional=False):
        Parameter.__init__(self, name, description, default, optional)

        if default is not None:
            try:
                self.default = int(unicode(default))
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
            return False

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'number'
        return '##' + self.name + '=' + param_type + str(self.default)


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
        return '"' + unicode(self.value) + '"' if self.value is not None else unicode(None)


class ParameterRaster(ParameterDataObject):

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
            self.value = unicode(obj.dataProvider().dataSourceUri())
            return True
        else:
            self.value = unicode(obj)
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


class ParameterSelection(Parameter):

    def __init__(self, name='', description='', options=[], default=None, isSource=False,
                 optional=False):
        Parameter.__init__(self, name, description, default, optional)
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
                        self.options.append(unicode(feature.attributes()[index]))
                except ValueError:
                    pass
        elif isinstance(self.options, basestring):
            self.options = self.options.split(";")

        if default is not None:
            try:
                self.default = int(default)
            except:
                self.default = 0
            self.value = self.default

    def setValue(self, n):
        if n is None:
            if not self.optional:
                return False
            self.value = 0
            return True

        try:
            n = int(n)
            self.value = n
            return True
        except:
            return False


class ParameterString(Parameter):

    NEWLINE = '\n'
    ESCAPED_NEWLINE = '\\n'

    def __init__(self, name='', description='', default=None, multiline=False,
                 optional=False, evaluateExpressions=False):
        Parameter.__init__(self, name, description, default, optional)
        self.multiline = parseBool(multiline)
        self.evaluateExpressions = parseBool(evaluateExpressions)

    def setValue(self, obj):
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        self.value = unicode(obj).replace(
            ParameterString.ESCAPED_NEWLINE,
            ParameterString.NEWLINE
        )
        return True

    def getValueAsCommandLineParameter(self):
        return ('"' + unicode(self.value.replace(ParameterString.NEWLINE,
                                                 ParameterString.ESCAPED_NEWLINE)) + '"'
                if self.value is not None else unicode(None))

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'string'
        return '##' + self.name + '=' + param_type + self.default


class ParameterTable(ParameterDataObject):

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
            source = unicode(obj.source())
            self.value = source
            return True
        else:
            self.value = unicode(obj)
            layers = dataobjects.getTables()
            for layer in layers:
                if layer.name() == self.value or layer.source() == self.value:
                    source = unicode(layer.source())
                    self.value = source
                    return True
            val = unicode(obj)
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


class ParameterTableField(Parameter):

    """A parameter representing a table field.
    Its value is a string that represents the name of the field.

    In a script you can use it with
    ##Field=[optional] field [number|string] Parentinput
    """

    DATA_TYPE_NUMBER = 0
    DATA_TYPE_STRING = 1
    DATA_TYPE_ANY = -1

    def __init__(self, name='', description='', parent=None, datatype=-1,
                 optional=False):
        Parameter.__init__(self, name, description, None, optional)
        self.parent = parent
        self.datatype = int(datatype)

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"' if self.value is not None else unicode(None)

    def setValue(self, value):
        if value is None:
            if not self.optional:
                return False
            self.value = None
            return True

        elif len(value) == 0 and not self.optional:
            return False
        self.value = unicode(value)
        return True

    def __str__(self):
        return self.name + ' <' + self.__module__.split('.')[-1] + ' from ' \
            + self.parent + '>'

    def dataType(self):
        if self.datatype == self.DATA_TYPE_NUMBER:
            return 'numeric'
        elif self.datatype == self.DATA_TYPE_STRING:
            return 'string'
        else:
            return 'any'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'field'
        return '##' + self.name + '=' + param_type + self.parent


class ParameterTableMultipleField(Parameter):

    """A parameter representing several table fields.
        Its value is a string with items separated by semicolons, each of
        which represents the name of each field.

        In a script you can use it with
        ##Fields=[optional] multiple field [number|string] Parentinput

        In the batch runner simply use a string with items separated by
        semicolons, each of which represents the name of each field.

        see algs.qgis.DeleteColumn.py for an usage example
    """

    DATA_TYPE_NUMBER = 0
    DATA_TYPE_STRING = 1
    DATA_TYPE_ANY = -1

    def __init__(self, name='', description='', parent=None, datatype=-1,
                 optional=False):
        Parameter.__init__(self, name, description, None, optional)
        self.parent = parent
        self.datatype = int(datatype)

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"' if self.value is not None else unicode(None)

    def setValue(self, obj):
        if obj is None:
            if self.optional:
                self.value = None
                return True
            return False

        if isinstance(obj, list):
            if len(obj) == 0:
                if self.optional:
                    self.value = None
                    return True
                return False
            self.value = ";".join(obj)
            return True
        else:
            self.value = unicode(obj)
            return True

    def __str__(self):
        return self.name + ' <' + self.__module__.split('.')[-1] + ' from ' \
            + self.parent + '>'

    def dataType(self):
        if self.datatype == self.DATA_TYPE_NUMBER:
            return 'numeric'
        elif self.datatype == self.DATA_TYPE_STRING:
            return 'string'
        else:
            return 'any'

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'multiple field '
        return '##' + self.name + '=' + param_type + self.parent


class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = -1

    def __init__(self, name='', description='', shapetype=[-1],
                 optional=False):
        ParameterDataObject.__init__(self, name, description, None, optional)
        if isinstance(shapetype, int):
            shapetype = [shapetype]
        elif isinstance(shapetype, basestring):
            shapetype = [int(t) for t in shapetype.split(',')]
        self.shapetype = shapetype
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if not self.optional:
                return False
            self.value = None
            return True

        if isinstance(obj, QgsVectorLayer):
            self.value = unicode(obj.source())
            return True
        else:
            self.value = unicode(obj)
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
        types = ''
        for shp in self.shapetype:
            if shp == self.VECTOR_TYPE_POINT:
                types += 'point, '
            elif shp == self.VECTOR_TYPE_LINE:
                types += 'line, '
            elif shp == self.VECTOR_TYPE_POLYGON:
                types += 'polygon, '
            else:
                types += 'any, '

        return types[:-2]

    def getAsScriptCode(self):
        param_type = ''
        if self.optional:
            param_type += 'optional '
        param_type += 'vector'
        return '##' + self.name + '=' + param_type


class ParameterGeometryPredicate(Parameter):

    predicates = ('intersects',
                  'contains',
                  'disjoint',
                  'equals',
                  'touches',
                  'overlaps',
                  'within',
                  'crosses')

    def __init__(self, name='', description='', left=None, right=None,
                 optional=False, enabledPredicates=None):
        Parameter.__init__(self, name, description, None, optional)
        self.left = left
        self.right = right
        self.value = None
        self.enabledPredicates = enabledPredicates
        if self.enabledPredicates is None:
            self.enabledPredicates = self.predicates

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def setValue(self, value):
        if value is None:
            if not self.optional:
                return False
            self.value = None
            return True
        elif len(value) == 0 and not self.optional:
            return False

        if isinstance(value, unicode):
            self.value = value.split(';')  # relates to ModelerAlgorithm.resolveValue
        else:
            self.value = value
        return True
