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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import sys
from qgis.core import *
from PyQt4.QtCore import *
from processing.tools.system import *
from processing.tools.vector import VectorWriter, TableWriter
from processing.tools import dataobjects

def getOutputFromString(s):
    tokens = s.split("|")
    params = [t if unicode(t) != "None" else None for t in tokens[1:]]
    clazz = getattr(sys.modules[__name__], tokens[0])
    return clazz(*params)

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
        return self.name + ' <' + self.__class__.__name__ + '>'

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

    def outputTypeName(self):
        return self.__module__.split('.')[-1]

    def tr(self, string, context=''):
        if context == '':
            context = 'Output'
        return QCoreApplication.translate(context, string)


class OutputDirectory(Output):
    directory = True


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


class OutputFile(Output):

    def __init__(self, name='', description='', ext = None):
        Output.__init__(self, name, description)
        self.ext = ext

    def getFileFilter(self, alg):
        if self.ext is None:
            return self.tr('All files(*.*)', 'OutputFile')
        else:
            return self.tr('%s files(*.%s)', 'OutputFile') % (self.ext, self.ext)

    def getDefaultFileExtension(self, alg):
        return self.ext or 'file'


class OutputHTML(Output):

    def getFileFilter(self, alg):
        return self.tr('HTML files(*.html)', 'OutputHTML')

    def getDefaultFileExtension(self, alg):
        return 'html'


class OutputNumber(Output):

    def __init__(self, name='', description=''):
        self.name = name
        self.description = description
        self.value = None
        self.hidden = True


class OutputRaster(Output):

    compatible = None

    def getFileFilter(self, alg):
        providerExts = alg.provider.getSupportedOutputRasterLayerExtensions()
        if providerExts == ['tif']:
            # use default extensions
            exts = dataobjects.getSupportedOutputRasterLayerExtensions()
        else:
            # use extensions given by the algorithm provider
            exts = providerExts
        for i in range(len(exts)):
            exts[i] = self.tr('%s files(*.%s)', 'OutputRaster') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputRasterLayerExtensions()[0]

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
                self.compatible = getTempFilenameInTempFolder(self.name + '.'
                        + self.getDefaultFileExtension(alg))
            return self.compatible


class OutputString(Output):

    def __init__(self, name='', description=''):
        self.name = name
        self.description = description
        self.value = None
        self.hidden = True


class OutputTable(Output):

    encoding = None
    compatible = None

    def getFileFilter(self, alg):
        exts = ['csv']
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + ' files(*.' + exts[i].lower() + ')'
        return ';;'.join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputTableExtensions()[0]

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
                self.compatible = getTempFilenameInTempFolder(self.name + '.'
                        + self.getDefaultFileExtension(alg))
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

    def getFileFilter(self, alg):
        exts = dataobjects.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = self.tr('%s files (*.%s)', 'OutputVector') % (exts[i].upper(), exts[i].lower())
        return ';;'.join(exts)

    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputVectorLayerExtensions()[0]

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
                self.compatible = getTempFilenameInTempFolder(self.name + '.'
                        + self.getDefaultFileExtension(alg))
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
        self.memoryLayer = w.memLayer
        return w
