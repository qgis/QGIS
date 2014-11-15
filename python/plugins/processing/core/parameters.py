
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
from PyQt4.QtCore import *
from qgis.core import *
from processing.tools.system import *
from processing.tools import dataobjects


def getParameterFromString(s):
    tokens = s.split("|")
    params = [t if unicode(t) != "None" else None for t in tokens[1:]]
    clazz = getattr(sys.modules[__name__], tokens[0])
    return clazz(*params)


def parseBool(s):
    if s == unicode(None):
        return None
    return unicode(s).lower() == unicode(True).lower()


class Parameter:
    """
    Base class for all parameters that a geoalgorithm might
    take as input.
    """

    def __init__(self, name='', description=''):
        self.name = name
        self.description = description
        self.value = None

        self.isAdvanced = False

        # A hidden parameter can be used to set a hard-coded value.
        # It can be used as any other parameter, but it will not be
        # shown to the user
        self.hidden = False

    def setValue(self, obj):
        """
        Sets the value of the parameter.

        Returns true if the value passed is correct for the type
        of parameter.
        """
        self.value = unicode(obj)
        return True

    def __str__(self):
        return self.name + ' <' + self.__module__.split('.')[-1] + '>'

    def getValueAsCommandLineParameter(self):
        """
        Returns the value of this parameter as it should have been
        entered in the console if calling an algorithm using the
        Processing.runalg() method.
        """
        return unicode(self.value)

    def parameterName(self):
        return self.__module__.split('.')[-1]

    def todict(self):
        return self.__dict__

    def tr(self, string, context=''):
        if context == '':
            context = 'Parameter'
        return QCoreApplication.translate(context, string)


class ParameterBoolean(Parameter):

    def __init__(self, name='', description='', default=True):
        Parameter.__init__(self, name, description)
        self.default = parseBool(default)
        self.value = None

    def setValue(self, value):
        if value is None:
            self.value = self.default
            return True
        if isinstance(value, basestring):
            self.value = unicode(value).lower() == unicode(True).lower()
        else:
            self.value = bool(value)
        return True


class ParameterCrs(Parameter):

    def __init__(self, name='', description='', default='EPSG:4326'):
        '''The value is the auth id of the CRS'''
        Parameter.__init__(self, name, description)
        self.value = None
        self.default = default

    def setValue(self, value):
        if value is None:
            self.value = self.default
            return True

        # TODO: check it is a valid authid
        self.value = unicode(value)
        return True

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'


class ParameterDataObject(Parameter):

    def getValueAsCommandLineParameter(self):
        if self.value is None:
            return unicode(None)
        else:
            if not isWindows():
                return '"' + unicode(self.value) + '"'
            else:
                return '"' + unicode(self.value).replace('\\', '\\\\') + '"'


class ParameterExtent(Parameter):

    USE_MIN_COVERING_EXTENT = 'USE_MIN_COVERING_EXTENT'

    def __init__(self, name='', description='', default='0,1,0,1'):
        Parameter.__init__(self, name, description)
        self.default = default
        # The value is a string in the form "xmin, xmax, ymin, ymax"
        self.value = None

    def setValue(self, text):
        if text is None:
            self.value = self.default
            return True
        tokens = text.split(',')
        if len(tokens) != 4:
            return False
        try:
            n1 = float(tokens[0])
            n2 = float(tokens[1])
            n3 = float(tokens[2])
            n4 = float(tokens[3])
            self.value = text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'


class ParameterFile(Parameter):

    def __init__(self, name='', description='', isFolder=False, optional=True, ext = None):
        Parameter.__init__(self, name, description)
        self.value = None
        self.ext = ext
        self.isFolder = parseBool(isFolder)
        self.optional = parseBool(optional)

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def setValue(self, obj):
        self.value = unicode(obj)
        if self.value.strip() == '' or self.value is None:
            if not self.optional:
                return False
            else:
                self.value = ''
        if self.ext is not None and self.value != '':
            return self.value.endswith(self.ext)
        return True


class ParameterFixedTable(Parameter):

    def __init__(self, name='', description='', numRows=3,
                 cols=['value'], fixedNumOfRows=False):
        Parameter.__init__(self, name, description)
        self.cols = cols
        if isinstance(cols, basestring):
            self.cols = self.cols.split(";")
        self.numRows = int(numRows)
        self.fixedNumOfRows = fixedNumOfRows
        self.value = None

    def setValue(self, obj):
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
        ParameterDataObject.__init__(self, name, description)
        self.datatype = int(float(datatype))
        self.optional = parseBool(optional)
        self.value = None
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if self.optional:
                self.value = None
                return True
            else:
                return False

        if isinstance(obj, list):
            if len(obj) == 0:
                if self.optional:
                    return True
                else:
                    return False
            self.value = ";".join([self.getAsString(lay) for lay in obj])
            return True
        else:
            self.value = unicode(obj)
            return True

    def getSafeExportedLayers(self):
        """Returns not the value entered by the user, but a string with
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


class ParameterNumber(Parameter):

    def __init__(self, name='', description='', minValue=None, maxValue=None,
                 default=0.0):
        Parameter.__init__(self, name, description)
        try:
            self.default = int(unicode(default))
            self.isInteger = True
        except:
            self.default = float(default)
            self.isInteger = False
        if minValue is not None:
            self.min = int(float(minValue)) if self.isInteger else float(minValue)
        else:
            self.min = None
        if maxValue is not None:
            self.max = int(float(maxValue)) if self.isInteger else float(maxValue)
        else:
            self.max = None
        self.value = None

    def setValue(self, n):
        if n is None:
            self.value = self.default
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


class ParameterRange(Parameter):

    def __init__(self, name='', description='', default='0,1'):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None

        values = default.split(',')
        try:
            minVal = int(values[0])
            maxVal = int(values[1])
            self.isInteger = True
        except:
            self.isInteger = False

    def setValue(self, text):
        if text is None:
            self.value = self.default
            return True
        tokens = text.split(',')
        if len(tokens) != 2:
            return False
        try:
            n1 = float(tokens[0])
            n2 = float(tokens[1])
            self.value = text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'


class ParameterRaster(ParameterDataObject):

    def __init__(self, name='', description='', optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = parseBool(optional)
        self.value = None
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
            if self.optional:
                self.value = None
                return True
            else:
                return False
        if isinstance(obj, QgsRasterLayer):
            self.value = unicode(obj.dataProvider().dataSourceUri())
            return True
        else:
            self.value = unicode(obj)
            layers = dataobjects.getRasterLayers()
            for layer in layers:
                if layer.name() == self.value:
                    self.value = unicode(layer.dataProvider().dataSourceUri())
                    return True
            return os.path.exists(self.value)

    def getFileFilter(self):
        exts = dataobjects.getSupportedOutputRasterLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'ParameterRaster') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)


class ParameterSelection(Parameter):

    def __init__(self, name='', description='', options=[], default=0):
        Parameter.__init__(self, name, description)
        self.options = options
        if isinstance(self.options, basestring):
            self.options = self.options.split(";")
        self.value = None
        self.default = int(default)

    def setValue(self, n):
        if n is None:
            self.value = self.default
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

    def __init__(self, name='', description='', default='', multiline=False,
                 optional=False):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None
        self.multiline = multiline
        self.optional = parseBool(optional)

    def setValue(self, obj):
        if obj is None:
            if self.optional:
                self.value = ''
                return True
            self.value = self.default
            return True
        self.value = unicode(obj).replace(ParameterString.ESCAPED_NEWLINE,
                ParameterString.NEWLINE)
        return True

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value.replace(ParameterString.NEWLINE,
                             ParameterString.ESCAPED_NEWLINE)) + '"'


class ParameterTable(ParameterDataObject):

    def __init__(self, name='', description='', optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = parseBool(optional)
        self.value = None
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if self.optional:
                self.value = None
                return True
            else:
                return False
        if isinstance(obj, QgsVectorLayer):
            source = unicode(obj.source())
            self.value = source
            return True
        else:
            layers = dataobjects.getVectorLayers()
            for layer in layers:
                if layer.name() == self.value:
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


class ParameterTableField(Parameter):

    DATA_TYPE_NUMBER = 0
    DATA_TYPE_STRING = 1
    DATA_TYPE_ANY = -1

    def __init__(self, name='', description='', parent=None, datatype=-1,
                 optional=False):
        Parameter.__init__(self, name, description)
        self.parent = parent
        self.value = None
        self.datatype = int(datatype)
        self.optional = parseBool(optional)

    def getValueAsCommandLineParameter(self):
        return '"' + unicode(self.value) + '"'

    def setValue(self, value):
        if value is None:
            return self.optional
        elif len(value) > 0:
            self.value = unicode(value)
        else:
            return self.optional
        return True

    def __str__(self):
        return self.name + ' <' + self.__module__.split('.')[-1] + ' from ' \
            + self.parent + '>'


class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = -1

    def __init__(self, name='', description='', shapetype=[-1],
                 optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = parseBool(optional)
        if isinstance(shapetype, int):
            shapetype = [shapetype]
        elif isinstance(shapetype, basestring):
            shapetype = [int(t) for t in shapetype.split(',')]
        self.shapetype = shapetype
        self.value = None
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj is None:
            if self.optional:
                self.value = None
                return True
            else:
                return False
        if isinstance(obj, QgsVectorLayer):
            self.value = unicode(obj.source())
            return True
        else:
            self.value = unicode(obj)
            layers = dataobjects.getVectorLayers(self.shapetype)
            for layer in layers:
                if layer.name() == self.value or layer.source() == self.value:
                    self.value = unicode(layer.source())
                    return True
            return os.path.exists(self.value)

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
