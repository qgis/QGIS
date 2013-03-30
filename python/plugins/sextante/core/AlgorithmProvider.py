# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmProvider.py
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

from sextante.core.SextanteConfig import Setting, SextanteConfig
import os
from PyQt4 import QtGui
from qgis.core import *

class AlgorithmProvider():
    '''this is the base class for algorithms providers.
    An algorithm provider is a set of related algorithms, typically from the same
    external application or related to a common area of analysis.
    '''

    def __init__(self):
        #indicates if the provider should be active by default.
        #For provider relying on an external software, this should be
        #false, so the user should activate them manually and install
        #the required software in advance.
        self.activate = True
        self.actions = []
        self.contextMenuActions = []


    def loadAlgorithms(self):
        self.algs = []
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        if not SextanteConfig.getSetting(name):
            return
        else:
            self._loadAlgorithms()
            for alg in self.algs:
                alg.provider = self

    #methods to be overridden.
    #==============================

    def _loadAlgorithms(self):
        '''Algorithm loading should take place here, filling self.algs, which is a list of
        elements of class GeoAlgorithm. Use that class to create your own algorithms'''
        pass

    def initializeSettings(self):
        '''this is the place where you should add config parameters to SEXTANTE using the SextanteConfig class.
        this method is called when a provider is added to SEXTANTE.
        By default it just adds a setting to activate or deactivate algorithms from the provider'''
        SextanteConfig.settingIcons[self.getDescription()] = self.getIcon()
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        SextanteConfig.addSetting(Setting(self.getDescription(), name, "Activate", self.activate))

    def unload(self):
        '''Do here anything that you want to be done when the provider is removed from the list of available ones.
        This method is called when you remove the provider from Sextante.
        Removal of config setting should be done here'''
        name = "ACTIVATE_" + self.getName().upper().replace(" ", "_")
        SextanteConfig.removeSetting(name)

    def getName(self):
        '''Returns the name to use to create the command-line name. Should be a short descriptive name of the provider'''
        return "sextante"

    def getDescription(self):
        '''Returns the full name of the provider'''
        return "Generic algorithm provider"

    def getPostProcessingErrorMessage(self, wrongLayers):
        '''Returns the message to be shown to the user when after running an algorithm for this provider, 
        there is a problem loading the resulting layer. 
        This method should analyze if the problem is caused by wrong entry data, a wrong or missing 
        installation of a required 3rd party app, or any other cause, and create an error response accordingly.
        Message is provided as an HTML code that will be displayed to the user, and which might contains
        links to installation paths for missing 3rd party apps.
        - wrongLayers: a list of Output objects that could not be loaded.'''  
        
        html ="<p>Oooops! SEXTANTE could not open the following output layers</p><ul>\n"        
        for layer in wrongLayers:
            html += '<li>' + layer.description + ': <font size=3 face="Courier New" color="ff0000">' + layer.value + "</font></li>\n"
        html +="</ul><p>The above files could not be opened, which probably indicates that they were not correctly produced by the executed algorithm</p>"
        html +="<p>Checking the log information might help you see why those layers were not created as expected</p>"
        return html
            
    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + "/../images/alg.png")

    def getSupportedOutputRasterLayerExtensions(self):
        return ["tif"]

    def getSupportedOutputVectorLayerExtensions(self):
        formats = QgsVectorFileWriter.supportedFiltersAndFormats()
        extensions = ["shp"]#shp is the default, should be the first
        for extension in formats.keys():
            extension = unicode(extension)
            extension = extension[extension.find('*.') + 2:]
            extension = extension[:extension.find(" ")]
            if extension.lower() != "shp":
                extensions.append(extension)
        return extensions

    def getSupportedOutputTableExtensions(self):
        return ["csv"]

    def supportsNonFileBasedOutput(self):
        return False
