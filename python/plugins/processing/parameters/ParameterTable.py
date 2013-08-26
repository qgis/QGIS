# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterTable.py
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
from processing.core.LayerExporter import LayerExporter

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from processing.parameters.ParameterDataObject import ParameterDataObject
from processing.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterTable(ParameterDataObject):

    def __init__(self, name="", description="", optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = optional
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
            source = unicode(obj.source())
            self.value = source
            return True
        else:
            layers = QGisLayers.getVectorLayers()
            for layer in layers:
                if layer.name() == self.value:
                    source = unicode(layer.source())
                    self.value = source
                    return True
            val = unicode(obj)
            self.value = val
            return os.path.exists(self.value)

    def getSafeExportedTable(self):
        '''Returns not the value entered by the user, but a string with a filename which
        contains the data of this table, but saved in a standard format (currently always
        a dbf file) so that it can be opened by most external applications.
        Works only if the table represented by the parameter value is currently loaded in QGIS.
        Otherwise, it will not perform any export and return the current value string.
        If the current value represents a table in a suitable format, it does not export at all
        and returns that value.
        The table is exported just the first time the method is called. The method can be called
        several times and it will always return the same file, performing the export only the first time.'''
        if self.exported:
            return self.exported
        table = QGisLayers.getObjectFromUri(self.value, False)
        if table:
            self.exported = LayerExporter.exportTable(table)
        else:
            self.exported = self.value
        return self.exported

    def getFileFilter(self,alg):
        exts = ['csv', 'dbf']
        for i in range(len(exts)):
            exts[i] = exts[i].upper() + " files(*." + exts[i].lower() + ")"
        return ";;".join(exts)

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterTable(tokens[1], tokens[2], str(True) == tokens[3])

    def getAsScriptCode(self):
        return "##" + self.name + "=table"