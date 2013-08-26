# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterCrs.py
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

from processing.parameters.Parameter import Parameter

class ParameterCrs(Parameter):

    def __init__(self, name="", description="", default = "EPSG:4326"):
        '''The value is the auth id of the CRS'''
        Parameter.__init__(self, name, description)
        self.value = None
        self.default = default

    def setValue(self, value):
        if value is None:
            self.value = self.default
            return True
        #TODO: check it is a valid authid
        self.value = str(value)
        return True

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        if tokens[3]==str(None):
            tokens[3] = None
        return ParameterCrs(tokens[1], tokens[2], tokens[3])

    def getAsScriptCode(self):
        return "##" + self.name + "=crs " + str(self.default)
