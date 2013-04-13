# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterString.py
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

class ParameterString(Parameter):

    NEWLINE = "\n"
    ESCAPED_NEWLINE = "\\n"

    def __init__(self, name="", description="", default="", multiline = False, optional = False):
        Parameter.__init__(self, name, description)
        self.default = default
        self.value = None
        self.multiline = multiline
        self.optional = optional

    def setValue(self, obj):
        if obj is None:
            self.value = self.default
            return True
        self.value = unicode(obj).replace(ParameterString.ESCAPED_NEWLINE,ParameterString.NEWLINE)
        return True

    def getValueAsCommandLineParameter(self):
        return "\"" + unicode(self.value.replace(ParameterString.NEWLINE,ParameterString.ESCAPED_NEWLINE)) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + unicode(self.default)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterString(tokens[1], tokens[2], tokens[3])

    def getAsScriptCode(self):
        return "##" + self.name + "=string " + self.default
