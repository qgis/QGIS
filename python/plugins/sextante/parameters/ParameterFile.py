# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterFile.py
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

class ParameterFile(Parameter):

    def __init__(self, name="", description="", isFolder = False):
        Parameter.__init__(self, name, description)
        self.value = None
        self.isFolder = isFolder

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.isFolder)

    def setValue(self, obj):
        self.value = str(obj)
        if self.value.strip() == "":
            return False
        return True
    
    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterFile(tokens[0], tokens[1], tokens[2] == str(True))

    def getAsScriptCode(self):
        if self.isFolder:
            return "##" + self.name + "=folder"
        else:
            return "##" + self.name + "=file"