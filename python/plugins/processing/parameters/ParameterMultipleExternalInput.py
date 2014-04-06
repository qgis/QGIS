# -*- coding: utf-8 -*-

"""
***************************************************************************
    ParameterMultipleExternalInput.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
                           (C) 2013 by CS Systemes d'information (CS SI)
    Email                : volayaf at gmail dot com
                           otb at c-s dot fr (CS SI)
    Contributors         : Victor Olaya  - basis from ParameterMultipleInput
                           Alexia Mondot (CS SI) - managing the new parameter ParameterMultipleExternalInput
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

from processing.parameters.ParameterDataObject import ParameterDataObject
from qgis.core import *

class ParameterMultipleExternalInput(ParameterDataObject):
    '''A parameter representing several data objects.
    Its value is a string with substrings separated by semicolons, each of which
    represents the data source location of each element'''

    exported = None

    def __init__(self, name="", description="", optional = False):
        ParameterDataObject.__init__(self, name, description)
        self.datatype = None # to delete
        self.optional = optional
        self.value = None
        self.exported = None
        self.default=""

    def setValue(self, obj):
        if isinstance(obj, list):
            if len(obj) == 0:
                if self.optional:
                    return True
                else:
                    return False
            s = ""
            idx = 0
            for filename in obj:
                s += filename
                if idx < len(obj) - 1:
                    s+=" "
                    idx=idx+1;
            self.value = s;
            return True


    def getSafeExportedLayers(self):
        '''Returns not the value entered by the user, but a string with semicolon-separated filenames
        which contains the data of the selected layers, but saved in a standard format (currently
        shapefiles for vector layers and GeoTiff for raster) so that they can be opened by most
        external applications.
        If there is a selection and QGIS is configured to use just the selection, if exports
        the layer even if it is already in a suitable format.
        Works only if the layer represented by the parameter value is currently loaded in QGIS.
        Otherwise, it will not perform any export and return the current value string.
        If the current value represents a layer in a suitable format, it does no export at all
        and returns that value.
        Currently, it works just for vector layer. In the case of raster layers, it returns the
        parameter value.
        The layers are exported just the first time the method is called. The method can be called
        several times and it will always return the same string, performing the export only the first time.'''
        if self.exported:
            return self.exported
        self.exported = self.value
        layers = self.value.split(";")
        if layers == None or len(layers) == 0:
            return self.value

        return layers



    def serialize(self):
        return self.__module__.split(".")[-1] + "|" + self.name + "|" + self.description + "|" + str(self.optional)

    def deserialize(self, s):
        """
        in : string : ParameterNumber|-nodatalabel|Label for the NoData class|None|None|0
        returns the current class object from extracted information from s
        """
        tokens = s.split("|")
        return ParameterMultipleExternalInput(tokens[1], tokens[2], tokens[3] == str(True))

    def getAsScriptCode(self):
        return "##" + self.name
