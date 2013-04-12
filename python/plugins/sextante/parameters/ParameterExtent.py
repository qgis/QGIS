# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterExtent.py
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

from sextante.parameters.Parameter import Parameter

class ParameterExtent(Parameter):

    USE_MIN_COVERING_EXTENT = "USE_MIN_COVERING_EXTENT"

    def __init__(self, name="", description="", default="0,1,0,1"):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None #The value is a string in the form "xmin, xmax, ymin, y max"

    def setValue(self, text):
        if text is None:
            self.value = self.default
            return True
        tokens = text.split(",")
        if len(tokens)!= 5:
            return False
        try:
            n1 = float(tokens[1])
            n2 = float(tokens[2])
            n3 = float(tokens[3])
            n4 = float(tokens[4])
            self.value=text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterExtent(tokens[0], tokens[1], tokens[2])

