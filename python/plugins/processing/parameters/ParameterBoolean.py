# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterBoolean.py
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

class ParameterBoolean(Parameter):

    def __init__(self, name="", description="", default=True):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None

    def setValue(self, value):
        if value is None:
            self.value = self.default
            return True
        self.value = str(value) == str(True)
        return True

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description + "|" + str(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterBoolean (tokens[1], tokens[2], tokens[3] == str(True))

    def getAsScriptCode(self):
        return "##" + self.name + "=boolean " + str(self.default)