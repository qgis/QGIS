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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

from sextante.parameters.ParameterDataObject import ParameterDataObject
from sextante.core.QGisLayers import QGisLayers
from qgis.core import *

class ParameterTable(ParameterDataObject):

    def __init__(self, name="", description="", optional=False):
        ParameterDataObject.__init__(self, name, description)
        self.optional = optional
        self.value = None

    def setValue(self, obj):
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

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.optional)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterTable(tokens[0], tokens[1], str(True) == tokens[2])

    def getAsScriptCode(self):
        return "##" + self.name + "=table"