# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterSelection.py
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

class ParameterSelection(Parameter):

    def __init__(self, name="", description="", options=[], default = 0):
        Parameter.__init__(self, name, description)
        self.options = options
        self.value = None
        self.default = default

    def setValue(self, n):
        if n is None:
            self.value = self.default
            return True
        try:
            n = int(n)
            self.value = n
            return True
        except:
            return False

    def getAsScriptCode(self):
        return "##" + self.name + "=selection " + ";".join(self.options)

    def deserialize(self, s):
        tokens = s.split("|")
        if len(tokens) == 5:
            return ParameterSelection(tokens[1], tokens[2], tokens[3].split(";"), int(tokens[4]))
        else:
            return ParameterSelection(tokens[1], tokens[2], tokens[3].split(";"))

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + ";".join(self.options)