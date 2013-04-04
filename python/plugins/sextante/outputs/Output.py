# -*- coding: utf-8 -*-

"""
***************************************************************************
    Output.py
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

from sextante.core.SextanteUtils import SextanteUtils

class Output(object):

    def __init__(self, name="", description="", hidden=False):
        self.name = name
        self.description = description
        #the value of an output is a string representing the location of the output.
        #For a file based output, it should be the filepath to it.
        self.value = None
        # a hidden output will not be shown to the user, who will not be able to select where to store it
        # Use this to generate outputs that are modified version of inputs (like a selection in a vector layer)
        # In the case of layers, hidden outputs are not loaded into QGIS after the algorithm is executed.
        # Other outputs not representing layers or tables should always be hidden.
        self.hidden = hidden
        #this value indicates whether the output has to be opened after being produced by the algorithm or not
        self.open = True

    def __str__(self):
        return self.name + " <" + self.__module__.split(".")[-1] +">"

    def getValueAsCommandLineParameter(self):
        if self.value == None:
            return str(None)
        else:
            if not SextanteUtils.isWindows():
                return "\"" + str(self.value) + "\""
            else:
                return "\"" + str(self.value).replace("\\", "\\\\") + "\""

    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description

    def setValue(self, value):
        try:
            if value != None and isinstance(value, basestring):
                value = value.strip()
            self.value = value
            return True
        except:
            return False

    def outputTypeName(self):
        return self.__module__.split(".")[-1]