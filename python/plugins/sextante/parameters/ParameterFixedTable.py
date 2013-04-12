# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterFixedTable.py
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

class ParameterFixedTable(Parameter):

    def __init__(self, name="", description="", cols=["value"], numRows=3, fixedNumOfRows = False):
        Parameter.__init__(self, name, description)
        self.cols = cols
        self.numRows = numRows
        self.fixedNumOfRows = fixedNumOfRows
        self.value = None

    def setValue(self, obj):
        ##TODO: check that it contains a correct number of elements
        if isinstance(obj, (str,unicode)):
            self.value = obj
        else:
            self.value = ParameterFixedTable.tableToString(obj)
        return True

    def getValueAsCommandLineParameter(self):
        return "\"" + str(self.value) + "\""

    @staticmethod
    def tableToString(table):
        tablestring = ""
        for i in range(len(table)):
            for j in range(len(table[0])):
                tablestring = tablestring + table[i][j] + ","
        tablestring = tablestring[:-1]
        return tablestring

    def deserialize(self, s):
        tokens = s.split("|")
        return ParameterFixedTable(tokens[1], tokens[2], tokens[4].split(";"), int(tokens[3]), tokens[5] == str(True))

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description +\
                        "|" + str(self.numRows) + "|" + ";".join(self.cols) + "|" +  str(self.fixedNumOfRows)

