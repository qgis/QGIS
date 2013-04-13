# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterRange.py
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

class ParameterRange(Parameter):

    def __init__(self, name="", description="", default="0,1"):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None

    def setValue(self, text):
        if text is None:
            self.value = self.default
            return True
        tokens = text.split(",")
        if len(tokens)!= 2:
            return False
        try:
            n1 = float(tokens[0])
            n2 = float(tokens[1])
            self.value=text
            return True
        except:
            return False

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def deserialize(self, s):
        tokens = s.split("|")
        if len(tokens) == 4:
            return ParameterRange(tokens[1], tokens[2], tokens[3])
        else:
            return ParameterRange(tokens[0], tokens[1])