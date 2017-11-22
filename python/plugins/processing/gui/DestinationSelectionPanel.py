# -*- coding: utf-8 -*-

"""
***************************************************************************
    OutputSelectionPanel.py
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
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import re
import os

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, QDir
from qgis.PyQt.QtWidgets import QDialog, QMenu, QAction, QFileDialog, QInputDialog
from qgis.PyQt.QtGui import QCursor
from qgis.gui import QgsEncodingSelectionDialog
from qgis.core import (QgsDataSourceUri,
                       QgsCredentials,
                       QgsExpression,
                       QgsSettings,
                       QgsProcessingParameterFeatureSink,
                       QgsProcessingParameterRasterDestination,
                       QgsProcessingOutputLayerDefinition,
                       QgsProcessingParameterDefinition,
                       QgsProcessingParameterFileDestination,
                       QgsProcessingParameterFolderDestination)
from processing.core.ProcessingConfig import ProcessingConfig
from processing.tools.dataobjects import createContext
from processing.gui.PostgisTableSelector import PostgisTableSelector
from processing.gui.ParameterGuiUtils import getFileFilter

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class DestinationSelectionPanel(BASE, WIDGET):

    SAVE_TO_TEMP_FILE = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Save to temporary file]')
    SAVE_TO_TEMP_LAYER = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Create temporary layer]')
    SKIP_OUTPUT = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Skip output]')

    def __init__(self, parameter, alg):
        super(DestinationSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.parameter = parameter
        self.alg = alg
        settings = QgsSettings()
        self.encoding = settings.value('/Processing/encoding', 'System')
        self.use_temporary = True

        if hasattr(self.leText, 'setPlaceholderText'):
            if parameter.flags() & QgsProcessingParameterDefinition.FlagOptional and not parameter.createByDefault():
                self.leText.setPlaceholderText(self.SKIP_OUTPUT)
                self.use_temporary = False
            elif isinstance(self.parameter, QgsProcessingParameterFeatureSink) \
                    and alg.provider().supportsNonFileBasedOutput():
                # use memory layers for temporary files if supported
                self.leText.setPlaceholderText(self.SAVE_TO_TEMP_LAYER)
            elif not isinstance(self.parameter, QgsProcessingParameterFolderDestination):
                self.leText.setPlaceholderText(self.SAVE_TO_TEMP_FILE)

        self.btnSelect.clicked.connect(self.selectOutput)
        self.leText.textEdited.connect(self.textChanged)

    def textChanged(self):
        self.use_temporary = False

    def skipOutput(self):
        self.leText.setPlaceholderText(self.SKIP_OUTPUT)
        self.leText.setText('')
        self.use_temporary = False

    def selectOutput(self):
        if isinstance(self.parameter, QgsProcessingParameterFolderDestination):
            self.selectDirectory()
        else:
            popupMenu = QMenu()

            if self.parameter.flags() & QgsProcessingParameterDefinition.FlagOptional:
                actionSkipOutput = QAction(
                    self.tr('Skip output'), self.btnSelect)
                actionSkipOutput.triggered.connect(self.skipOutput)
                popupMenu.addAction(actionSkipOutput)

            if isinstance(self.parameter, QgsProcessingParameterFeatureSink) \
                    and self.alg.provider().supportsNonFileBasedOutput():
                # use memory layers for temporary layers if supported
                actionSaveToTemp = QAction(
                    self.tr('Create temporary layer'), self.btnSelect)
            else:
                actionSaveToTemp = QAction(
                    self.tr('Save to a temporary file'), self.btnSelect)
            actionSaveToTemp.triggered.connect(self.saveToTemporary)
            popupMenu.addAction(actionSaveToTemp)

            actionSaveToFile = QAction(
                self.tr('Save to file...'), self.btnSelect)
            actionSaveToFile.triggered.connect(self.selectFile)
            popupMenu.addAction(actionSaveToFile)

            if isinstance(self.parameter, QgsProcessingParameterFeatureSink) \
                    and self.alg.provider().supportsNonFileBasedOutput():
                actionSaveToGpkg = QAction(
                    self.tr('Save to GeoPackage...'), self.btnSelect)
                actionSaveToGpkg.triggered.connect(self.saveToGeopackage)
                popupMenu.addAction(actionSaveToGpkg)
                actionSaveToPostGIS = QAction(
                    self.tr('Save to PostGIS table...'), self.btnSelect)
                actionSaveToPostGIS.triggered.connect(self.saveToPostGIS)
                settings = QgsSettings()
                settings.beginGroup('/PostgreSQL/connections/')
                names = settings.childGroups()
                settings.endGroup()
                actionSaveToPostGIS.setEnabled(bool(names))
                popupMenu.addAction(actionSaveToPostGIS)

            actionSetEncoding = QAction(
                self.tr('Change file encoding ({})...').format(self.encoding), self.btnSelect)
            actionSetEncoding.triggered.connect(self.selectEncoding)
            popupMenu.addAction(actionSetEncoding)

            popupMenu.exec_(QCursor.pos())

    def saveToTemporary(self):
        if isinstance(self.parameter, QgsProcessingParameterFeatureSink) and self.alg.provider().supportsNonFileBasedOutput():
            self.leText.setPlaceholderText(self.SAVE_TO_TEMP_LAYER)
        else:
            self.leText.setPlaceholderText(self.SAVE_TO_TEMP_FILE)
        self.leText.setText('')
        self.use_temporary = True

    def saveToPostGIS(self):
        dlg = PostgisTableSelector(self, self.parameter.name().lower())
        dlg.exec_()
        if dlg.connection:
            self.use_temporary = False
            settings = QgsSettings()
            mySettings = '/PostgreSQL/connections/' + dlg.connection
            dbname = settings.value(mySettings + '/database')
            user = settings.value(mySettings + '/username')
            host = settings.value(mySettings + '/host')
            port = settings.value(mySettings + '/port')
            password = settings.value(mySettings + '/password')
            uri = QgsDataSourceUri()
            uri.setConnection(host, str(port), dbname, user, password)
            uri.setDataSource(dlg.schema, dlg.table,
                              "the_geom" if isinstance(self.parameter, QgsProcessingParameterFeatureSink) and self.parameter.hasGeometry() else None)

            connInfo = uri.connectionInfo()
            (success, user, passwd) = QgsCredentials.instance().get(connInfo, None, None)
            if success:
                QgsCredentials.instance().put(connInfo, user, passwd)
            self.leText.setText("postgis:" + uri.uri())

    def saveToGeopackage(self):
        file_filter = self.tr('GeoPackage files (*.gpkg);;All files (*.*)', 'OutputFile')

        settings = QgsSettings()
        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)

        filename, filter = QFileDialog.getSaveFileName(self, self.tr("Save to GeoPackage"), path,
                                                       file_filter, options=QFileDialog.DontConfirmOverwrite)

        if filename is None:
            return

        layer_name, ok = QInputDialog.getText(self, self.tr('Save to GeoPackage'), self.tr('Layer name'), text=self.parameter.name().lower())
        if ok:
            self.use_temporary = False
            if not filename.lower().endswith('.gpkg'):
                filename += '.gpkg'
            settings.setValue('/Processing/LastOutputPath',
                              os.path.dirname(filename))

            uri = QgsDataSourceUri()
            uri.setDatabase(filename)
            uri.setDataSource('', layer_name,
                              'geom' if isinstance(self.parameter, QgsProcessingParameterFeatureSink) and self.parameter.hasGeometry() else None)
            self.leText.setText("ogr:" + uri.uri())

    def selectFile(self):
        file_filter = getFileFilter(self.parameter)
        settings = QgsSettings()
        if isinstance(self.parameter, QgsProcessingParameterFeatureSink):
            last_ext_path = '/Processing/LastVectorOutputExt'
            last_ext = settings.value(last_ext_path, '.gpkg')
        elif isinstance(self.parameter, QgsProcessingParameterRasterDestination):
            last_ext_path = '/Processing/LastRasterOutputExt'
            last_ext = settings.value(last_ext_path, '.tif')
        else:
            last_ext_path = None
            last_ext = None

        # get default filter
        filters = file_filter.split(';;')
        try:
            last_filter = [f for f in filters if '*{}'.format(last_ext) in f.lower()][0]
        except:
            last_filter = None

        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)

        filename, filter = QFileDialog.getSaveFileName(self, self.tr("Save file"), path,
                                                       file_filter, last_filter)
        if filename:
            self.use_temporary = False
            if not filename.lower().endswith(
                    tuple(re.findall("\\*(\\.[a-z]{1,10})", file_filter))):
                ext = re.search("\\*(\\.[a-z]{1,10})", filter)
                if ext:
                    filename += ext.group(1)
            self.leText.setText(filename)
            settings.setValue('/Processing/LastOutputPath',
                              os.path.dirname(filename))
            if not last_ext_path is None:
                settings.setValue(last_ext_path, os.path.splitext(filename)[1].lower())

    def selectEncoding(self):
        dialog = QgsEncodingSelectionDialog(
            self, self.tr('File encoding'), self.encoding)
        if dialog.exec_() == QDialog.Accepted:
            self.encoding = dialog.encoding()
            settings = QgsSettings()
            settings.setValue('/Processing/encoding', self.encoding)
        dialog.deleteLater()

    def selectDirectory(self):
        lastDir = self.leText.text()
        settings = QgsSettings()
        if not lastDir:
            lastDir = settings.value("/Processing/LastOutputPath", QDir.homePath())

        dirName = QFileDialog.getExistingDirectory(self, self.tr('Select directory'),
                                                   lastDir, QFileDialog.ShowDirsOnly)
        if dirName:
            self.leText.setText(QDir.toNativeSeparators(dirName))
            settings.setValue('/Processing/LastOutputPath', dirName)

    def getValue(self):
        key = None
        if self.use_temporary and isinstance(self.parameter, QgsProcessingParameterFeatureSink):
            key = 'memory:'
        elif self.use_temporary:
            key = self.parameter.generateTemporaryDestination()
        else:
            key = self.leText.text()

        if not key and self.parameter.flags() & QgsProcessingParameterDefinition.FlagOptional:
            return None

        if isinstance(self.parameter, QgsProcessingParameterFolderDestination):
            return self.leText.text()

        if isinstance(self.parameter, QgsProcessingParameterFileDestination):
            return key

        value = QgsProcessingOutputLayerDefinition(key)
        value.createOptions = {'fileEncoding': self.encoding}
        return value
