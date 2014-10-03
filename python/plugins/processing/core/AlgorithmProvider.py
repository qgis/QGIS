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

import os
from PyQt4 import QtGui, QtCore
from qgis.core import *
from processing.core.ProcessingConfig import Setting, ProcessingConfig


class AlgorithmProvider:
    """This is the base class for algorithms providers.

    An algorithm provider is a set of related algorithms, typically
    from the same external application or related to a common area
    of analysis.
    """

    def __init__(self):
        # Indicates if the provider should be active by default.
        # For provider relying on an external software, this should be
        # False, so the user should activate them manually and install
        # the required software in advance.
        self.activate = True
        self.actions = []
        self.contextMenuActions = []

    def loadAlgorithms(self):
        self.algs = []
        name = 'ACTIVATE_' + self.getName().upper().replace(' ', '_')
        if not ProcessingConfig.getSetting(name):
            return
        else:
            self._loadAlgorithms()
            for alg in self.algs:
                alg.provider = self

    # Methods to be overridden.
    def _loadAlgorithms(self):
        """Algorithm loading should take place here, filling self.algs,
        which is a list of elements of class GeoAlgorithm. Use that
        class to create your own algorithms.
        """
        pass

    def initializeSettings(self):
        """This is the place where you should add config parameters
        using the ProcessingConfig class.

        This method is called when a provider is added to the
        Processing framework. By default it just adds a setting to
        activate or deactivate algorithms from the provider.
        """
        ProcessingConfig.settingIcons[self.getDescription()] = self.getIcon()
        name = 'ACTIVATE_' + self.getName().upper().replace(' ', '_')
        ProcessingConfig.addSetting(Setting(self.getDescription(), name,
                                    self.tr('Activate'), self.activate))

    def unload(self):
        """Do here anything that you want to be done when the provider
        is removed from the list of available ones.

        This method is called when you remove the provider from
        Processing. Removal of config setting should be done here.
        """
        name = 'ACTIVATE_' + self.getName().upper().replace(' ', '_')
        ProcessingConfig.removeSetting(name)

    def getName(self):
        """Returns the name to use to create the command-line name.
        Should be a short descriptive name of the provider.
        """
        return 'processing'

    def getDescription(self):
        """Returns the full name of the provider.
        """
        return self.tr('Generic algorithm provider')

    def getIcon(self):
        return QtGui.QIcon(os.path.dirname(__file__) + '/../images/alg.png')

    def getSupportedOutputRasterLayerExtensions(self):
        return ['tif']

    def getSupportedOutputVectorLayerExtensions(self):
        formats = QgsVectorFileWriter.supportedFiltersAndFormats()
        extensions = ['shp']  # shp is the default, should be the first
        for extension in formats.keys():
            extension = unicode(extension)
            extension = extension[extension.find('*.') + 2:]
            extension = extension[:extension.find(' ')]
            if extension.lower() != 'shp':
                extensions.append(extension)
        return extensions

    def getSupportedOutputTableExtensions(self):
        return ['csv']

    def supportsNonFileBasedOutput(self):
        return False

    def tr(self, string, context=''):
        if context == '':
            context = 'AlgorithmProvider'
        return QtCore.QCoreApplication.translate(context, string)
