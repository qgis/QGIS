# -*- coding: utf-8 -*-

"""
***************************************************************************
    OutputVector.py
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
from sextante.core.QGisLayers import QGisLayers

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from PyQt4.QtCore import *
from qgis.core import *
from sextante.outputs.Output import Output
from sextante.core.SextanteVectorWriter import SextanteVectorWriter
from sextante.core.SextanteUtils import SextanteUtils


class OutputVector(Output):

    encoding = None
    compatible = None

    def getFileFilter(self,alg):
        exts = QGisLayers.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)


    def getDefaultFileExtension(self, alg):
        return alg.provider.getSupportedOutputVectorLayerExtensions()[0]

    def getCompatibleFileName(self, alg):
        '''Returns a filename that is compatible with the algorithm that is going to generate this output.
        If the algorithm supports the file format of the current output value, it returns that value. If not,
        it returns a temporary file with a supported file format, to be used to generate the output result.'''
        ext = self.value[self.value.rfind(".") + 1:]
        if ext in alg.provider.getSupportedOutputVectorLayerExtensions():
            return self.value
        else:
            if self.compatible is None:
                self.compatible = SextanteUtils.getTempFilename(self.getDefaultFileExtension(alg))
            return self.compatible;


    def getVectorWriter(self, fields, geomType, crs, options=None):
        '''Returns a suitable writer to which features can be added as a
        result of the algorithm. Use this to transparently handle output
        values instead of creating your own method.

        Executing this method might modify the object, adding additional
        information to it, so the writer can be later accessed and processed
        within QGIS. It should be called just once, since a new call might
        result in previous data being replaced, thus rendering a previously
        obtained writer useless

        @param fields   a list  of QgsField
        @param geomType a suitable geometry type, as it would be passed
                        to a QgsVectorFileWriter constructor
        @param crs      the crs of the layer to create

        @return writer  instance of the vector writer class
        '''

        if self.encoding is None:
            settings = QSettings()
            self.encoding = settings.value("/SextanteQGIS/encoding", "System")

        w = SextanteVectorWriter(self.value, self.encoding, fields, geomType, crs, options)
        self.memoryLayer = w.memLayer
        return w
