# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterVector.py
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

from processing.parameters.ParameterDataObject import ParameterDataObject
from processing.core.QGisLayers import QGisLayers
from qgis.core import *
from processing.core.LayerExporter import LayerExporter

class ParameterVector(ParameterDataObject):

    VECTOR_TYPE_POINT = 0
    VECTOR_TYPE_LINE = 1
    VECTOR_TYPE_POLYGON = 2
    VECTOR_TYPE_ANY = -1

    def __init__(self, name="", description="", shapetype=[-1], optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = optional
        if isinstance(shapetype, int):
            shapetype = [shapetype]
        self.shapetype = shapetype
        self.value = None
        self.exported = None

    def setValue(self, obj):
        self.exported = None
        if obj == None:
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
            layers = QGisLayers.getVectorLayers(self.shapetype)
            for layer in layers:
                if layer.name() == self.value  or layer.source() == self.value:
                    self.value = unicode(layer.source())
                    return True
            return os.path.exists(self.value)

    def getSafeExportedLayer(self):
        '''Returns not the value entered by the user, but a string with a filename which
        contains the data of this layer, but saved in a standard format (currently always
        a shapefile) so that it can be opened by most external applications.
        If there is a selection and QGIS is configured to use just the selection, if exports
        the layer even if it is already in a suitable format.
        Works only if the layer represented by the parameter value is currently loaded in QGIS.
        Otherwise, it will not perform any export and return the current value string.
        If the current value represents a layer in a suitable format, it does not export at all
        and returns that value.
        The layer is exported just the first time the method is called. The method can be called
        several times and it will always return the same file, performing the export only the first time.'''
        if self.exported:
            return self.exported
        layer = QGisLayers.getObjectFromUri(self.value, False)
        if layer:
            self.exported = LayerExporter.exportVectorLayer(layer)
        else:
            self.exported = self.value
        return self.exported

    def getFileFilter(self):
        exts = QGisLayers.getSupportedOutputVectorLayerExtensions()
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + ",".join(str(t) for t in self.shapetype) + "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterVector(tokens[1], tokens[2], [int(t) for t in tokens[3].split(",")], str(True) == tokens[4])

    def getAsScriptCode(self):
        return "##" + self.name + "=vector"
