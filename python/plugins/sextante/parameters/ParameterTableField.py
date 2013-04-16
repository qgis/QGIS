# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterTableField.py
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

class ParameterTableField(Parameter):

    DATA_TYPE_NUMBER = 0
    DATA_TYPE_STRING = 1
    DATA_TYPE_ANY = -1

    def __init__(self, name="", description="", parent=None, datatype=-1, optional=False):
        Parameter.__init__(self, name, description)
        self.parent = parent
        self.value = None
        self.datatype = datatype
        self.optional= optional

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    def getAsScriptCode(self):
        return "##" + self.name + "=field " + str(self.parent)

    def setValue(self, field):
        if field is None:
            return self.optional
        elif len(field) > 0:
            self.value = str(field)
        else:
            return self.optional
        return True

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                "|" + str(self.parent) + "|" + str(self.datatype)

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterTableField(tokens[1], tokens[2], tokens[3], int(tokens[4]), tokens[5] == str(True))

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +" from " + self.parent     + ">"
