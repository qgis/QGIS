# -*- coding: utf-8 -*-

"""
***************************************************************************
    Output.py
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
import os


__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys

from qgis.PyQt.QtCore import QCoreApplication, QSettings

from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.system import isWindows, getTempFilenameInTempFolder, getTempDirInTempFolder
from processing.tools.vector import VectorWriter, TableWriter
from processing.tools import dataobjects

from qgis.core import QgsExpressionContext, QgsExpressionContextUtils, QgsExpression, QgsExpressionContextScope


def _expressionContext(alg):
    context = QgsExpressionContext()
    context.appendScope(QgsExpressionContextUtils.globalScope())
    context.appendScope(QgsExpressionContextUtils.projectScope())
    processingScope = QgsExpressionContextScope()
    for param in alg.parameters:
        processingScope.setVariable('%s_value' % param.name, '')
    context.appendScope(processingScope)
    return context


class Output(object):

    def __init__(self, name='', description='', hidden=False):
        self.name = name
        self.description = description

        # The value of an output is a string representing the location
        # of the output. For a file based output, it should be the
        # filepath to it.
        self.value = None

        # A hidden output will not be shown to the user, who will not
        # be able to select where to store it. Use this to generate
        # outputs that are modified version of inputs (like a selection
        # in a vector layer). In the case of layers, hidden outputs are
        # not loaded into QGIS after the algorithm is executed. Other
        # outputs not representing layers or tables should always be hidden.
        self.hidden = unicode(hidden).lower() == unicode(True).lower()

        # This value indicates whether the output has to be opened
        # after being produced by the algorithm or not
        self.open = True

    def __str__(self):
        return u'{} <{}>'.format(self.name, self.__class__.__name__)

    def getValueAsCommandLineParameter(self):
        if self.value is None:
            return unicode(None)
        else:
            if not isWindows():
                return '"' + unicode(self.value) + '"'
            else:
                return '"' + unicode(self.value).replace('\\', '\\\\') + '"'

    def setValue(self, value):
        try:
            if value is not None and isinstance(value, basestring):
                value = value.strip()
            self.value = value
            return True
        except:
            return False

    def _resolveTemporary(self, alg):
        ext = self.getDefaultFileExtension()
        return getTempFilenameInTempFolder(self.name + '.' + ext)

    def _supportedExtensions(self):
        return []

    def resolveValue(self, alg):
        if self.hidden:
            return
        if not bool(self.value):
            self.value = self._resolveTemporary(alg)
        else:
            exp = QgsExpression(self.value)
            if not exp.hasParserError():
                value = exp.evaluate(_expressionContext(alg))
                if not exp.hasEvalError():
                    self.value = value

        if ":" not in self.value:
            if not os.path.isabs(self.value):
                self.value = os.path.join(ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER),
                                          self.value)
            supported = self._supportedExtensions()
            if supported:
                idx = self.value.rfind('.')
                if idx == -1:
                    self.value = self.value + '.' + self.getDefaultFileExtension()
                else:
                    ext = self.value[idx + 1:]
                    if ext not in supported:
                        self.value = self.value + '.' + self.getDefaultFileExtension()

    def expressionContext(self, alg):
        return _expressionContext(alg)

    def typeName(self):
        return self.__class__.__name__.replace('Output', '').lower()

    def tr(self, string, context=''):
        if context == '':
            context = 'Output'
        return QCoreApplication.translate(context, string)


class OutputDirectory(Output):

    def resolveValue(self, alg):
        self.value = getTempDirInTempFolder()


class OutputExtent(Output):

    def __init__(self, name='', description=''):
        self.name = name
        self.description = description
        self.value = None
        self.hidden = True

    def setValue(self, value):
        try:
            if value is not None and isinstance(value, basestring):
                value = value.strip()
            else:
                self.value = ','.join([unicode(v) for v in value])
            return True
        except:
            return False


class OutputCrs(Output):

    def __init__(self, name='', description=''):
        Output.__init__(self, name, description, True)


class OutputFile(Output):

    def __init__(self, name='', description='', ext=None):
        Output.__init__(self, name, description)
        self.ext = ext

    def getFileFilter(self, alg):
        if self.ext is None:
            return self.tr('All files(*.*)', 'OutputFile')
        else:
            return self.tr('%s files(*.%s)', 'OutputFile') % (self.ext, self.ext)

    def getDefaultFileExtension(self):
        return self.ext or 'file'


class OutputHTML(Output):

    def getFileFilter(self, alg):
        return self.tr('HTML files(*.html)', 'OutputHTML')

    def getDefaultFileExtension(self):
        return 'html'


class OutputNumber(Output):

    def __init__(self, name='', description=''):
        Output.__init__(self, name, description, True)


class OutputRaster(Output):

    compatible = None

    def getFileFilter(self, alg):
        exts = dataobjects.getSupportedOutputRasterLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files (*.%s)', 'OutputVector') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def getDefaultFileExtension(self):
        return ProcessingConfig.getSetting(ProcessingConfig.DEFAULT_OUTPUT_RASTER_LAYER_EXT)

    def getCompatibleFileName(self, alg):
        """
        Returns a filename that is compatible with the algorithm
        that is going to generate this output. If the algorithm
        supports the file format of the current output value, it
        returns that value. If not, it returns a temporary file with
        a supported file format, to be used to generate the output
        result.
        """

        ext = self.value[self.value.rfind('.') + 1:]
        if ext in alg.provider.getSupportedOutputRasterLayerExtensions():
            return self.value
        else:
            if self.compatible is None:
                supported = alg.provider.getSupportedOutputRasterLayerExtensions()
                default = ProcessingConfig.getSetting(ProcessingConfig.DEFAULT_OUTPUT_RASTER_LAYER_EXT)
                ext = default if default in supported else supported[0]
                self.compatible = getTempFilenameInTempFolder(self.name + '.' + ext)
            return self.compatible


class OutputString(Output):

    def __init__(self, name='', description=''):
        Output.__init__(self, name, description, True)


class OutputTable(Output):

    encoding = None
    compatible = None

    def getFileFilter(self, alg):
        exts = ['dbf']
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + ' files(*.' + exts[i].lower() + ')'
        return ';;'.join(exts)

    def getDefaultFileExtension(self):
        return "dbf"

    def getCompatibleFileName(self, alg):
        """Returns a filename that is compatible with the algorithm
        that is going to generate this output.

        If the algorithm supports the file format of the current
        output value, it returns that value. If not, it returns a
        temporary file with a supported file format, to be used to
        generate the output result.
        """

        ext = self.value[self.value.rfind('.') + 1:]
        if ext in alg.provider.getSupportedOutputTableExtensions():
            return self.value
        else:
            if self.compatible is None:
                self.compatible = getTempFilenameInTempFolder(
                    self.name + '.' + alg.provider.getSupportedOutputTableExtensions()[0])
            return self.compatible

    def getTableWriter(self, fields):
        """
        Returns a suitable writer to which records can be added as a
        result of the algorithm. Use this to transparently handle
        output values instead of creating your own method.

        @param fields   a list of field titles

        @return writer  instance of the table writer class
        """

        if self.encoding is None:
            settings = QSettings()
            self.encoding = settings.value('/Processing/encoding', 'System')

        return TableWriter(self.value, self.encoding, fields)


class OutputVector(Output):

    encoding = None
    compatible = None

    def __init__(self, name='', description='', hidden=False, base_input=None, datatype=[-1]):
        Output.__init__(self, name, description, hidden)
        self.base_input = base_input
        self.base_layer = None
        if isinstance(datatype, int):
            datatype = [datatype]
        elif isinstance(datatype, basestring):
            datatype = [int(t) for t in datatype.split(',')]
        self.datatype = datatype

    def hasGeometry(self):
        if self.base_layer is None:
            return True
        return dataobjects.canUseVectorLayer(self.base_layer, [-1])

    def getSupportedOutputVectorLayerExtensions(self):
        exts = dataobjects.getSupportedOutputVectorLayerExtensions()
        if not self.hasGeometry():
            exts = ['dbf'] + [ext for ext in exts if ext in VectorWriter.nogeometry_extensions]
        return exts

    def getFileFilter(self, alg):
        exts = self.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files (*.%s)', 'OutputVector') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def getDefaultFileExtension(self):
        if self.hasGeometry():
            default = ProcessingConfig.getSetting(ProcessingConfig.DEFAULT_OUTPUT_VECTOR_LAYER_EXT)
        else:
            default = 'dbf'
        return default

    def getCompatibleFileName(self, alg):
        """Returns a filename that is compatible with the algorithm
        that is going to generate this output.

        If the algorithm supports the file format of the current
        output value, it returns that value. If not, it returns a
        temporary file with a supported file format, to be used to
        generate the output result.
        """
        ext = self.value[self.value.rfind('.') + 1:]
        if ext in alg.provider.getSupportedOutputVectorLayerExtensions():
            return self.value
        else:
            if self.compatible is None:
                default = self.getDefaultFileExtension()
                supported = alg.provider.getSupportedOutputVectorLayerExtensions()
                ext = default if default in supported else supported[0]
                self.compatible = getTempFilenameInTempFolder(self.name + '.' + ext)
            return self.compatible

    def getVectorWriter(self, fields, geomType, crs, options=None):
        """Returns a suitable writer to which features can be added as
        a result of the algorithm. Use this to transparently handle
        output values instead of creating your own method.

        Executing this method might modify the object, adding additional
        information to it, so the writer can be later accessed and
        processed within QGIS. It should be called just once, since a
        new call might result in previous data being replaced, thus
        rendering a previously obtained writer useless.

        @param fields   a list  of QgsField
        @param geomType a suitable geometry type, as it would be passed
                        to a QgsVectorFileWriter constructor
        @param crs      the crs of the layer to create

        @return writer  instance of the vector writer class
        """

        if self.encoding is None:
            settings = QSettings()
            self.encoding = settings.value('/Processing/encoding', 'System', str)

        w = VectorWriter(self.value, self.encoding, fields, geomType,
                         crs, options)
        self.layer = w.layer
        self.value = w.destination
        return w

    def dataType(self):
        return dataobjects.vectorDataType(self)

    def _resolveTemporary(self, alg):
        if alg.provider.supportsNonFileBasedOutput():
            return "memory:"
        else:
            ext = self.getDefaultFileExtension()
            return getTempFilenameInTempFolder(self.name + '.' + ext)


def getOutputFromString(s):
    try:
        if "|" in s:
            tokens = s.split("|")
            params = [t if unicode(t) != "None" else None for t in tokens[1:]]
            clazz = getattr(sys.modules[__name__], tokens[0])
            return clazz(*params)
        else:
            tokens = s.split("=")
            token = tokens[1].strip()[len('output') + 1:]
            out = None

            if token.lower().strip().startswith('raster'):
                out = OutputRaster()
            elif token.lower().strip() == 'vector':
                out = OutputVector()
            elif token.lower().strip() == 'vector point':
                out = OutputVector(datatype=[dataobjects.TYPE_VECTOR_POINT])
            elif token.lower().strip() == 'vector line':
                out = OutputVector(datatype=[OutputVector.TYPE_VECTOR_LINE])
            elif token.lower().strip() == 'vector polygon':
                out = OutputVector(datatype=[OutputVector.TYPE_VECTOR_POLYGON])
            elif token.lower().strip().startswith('table'):
                out = OutputTable()
            elif token.lower().strip().startswith('html'):
                out = OutputHTML()
            elif token.lower().strip().startswith('file'):
                out = OutputFile()
                subtokens = token.split(' ')
                if len(subtokens) > 2:
                    out.ext = subtokens[2]
            elif token.lower().strip().startswith('directory'):
                out = OutputDirectory()
            elif token.lower().strip().startswith('number'):
                out = OutputNumber()
            elif token.lower().strip().startswith('string'):
                out = OutputString()
            elif token.lower().strip().startswith('extent'):
                out = OutputExtent()

            return out
    except:
        return None
