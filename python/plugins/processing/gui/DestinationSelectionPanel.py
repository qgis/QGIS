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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import re
import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, QDir, pyqtSignal, QFileInfo
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

with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class DestinationSelectionPanel(BASE, WIDGET):

    SAVE_TO_TEMP_FILE = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Save to temporary file]')
    SAVE_TO_TEMP_FOLDER = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Save to temporary folder]')
    SAVE_TO_TEMP_LAYER = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Create temporary layer]')
    SKIP_OUTPUT = QCoreApplication.translate(
        'DestinationSelectionPanel', '[Skip output]')

    skipOutputChanged = pyqtSignal(bool)
    destinationChanged = pyqtSignal()

    def __init__(self, parameter, alg, default_selection=False):
        super(DestinationSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.parameter = parameter
        self.alg = alg
        self.default_selection = default_selection
        settings = QgsSettings()
        self.encoding = settings.value('/Processing/encoding', 'System')
        self.use_temporary = True

        self.setValue(self.parameter.defaultValue())

        self.btnSelect.clicked.connect(self.selectOutput)
        self.leText.textEdited.connect(self.textChanged)

    def textChanged(self):
        self.use_temporary = not self.leText.text()
        self.destinationChanged.emit()

    def outputIsSkipped(self):
        """
        Returns true if output is set to be skipped
        """
        return not self.leText.text() and not self.use_temporary

    def skipOutput(self):
        self.leText.setPlaceholderText(self.SKIP_OUTPUT)
        self.leText.setText('')
        self.use_temporary = False
        self.skipOutputChanged.emit(True)
        self.destinationChanged.emit()

    def selectOutput(self):
        if isinstance(self.parameter, QgsProcessingParameterFolderDestination):
            self.selectDirectory()
        else:
            popupMenu = QMenu()

            if not self.default_selection:
                if self.parameter.flags() & QgsProcessingParameterDefinition.FlagOptional:
                    actionSkipOutput = QAction(
                        self.tr('Skip Output'), self.btnSelect)
                    actionSkipOutput.triggered.connect(self.skipOutput)
                    popupMenu.addAction(actionSkipOutput)

                if isinstance(self.parameter, QgsProcessingParameterFeatureSink) \
                        and self.parameter.supportsNonFileBasedOutput():
                    # use memory layers for temporary layers if supported
                    actionSaveToTemp = QAction(
                        self.tr('Create Temporary Layer'), self.btnSelect)
                else:
                    actionSaveToTemp = QAction(
                        self.tr('Save to a Temporary File'), self.btnSelect)
                actionSaveToTemp.triggered.connect(self.saveToTemporary)
                popupMenu.addAction(actionSaveToTemp)

            actionSaveToFile = QAction(
                QCoreApplication.translate('DestinationSelectionPanel', 'Save to File…'), self.btnSelect)
            actionSaveToFile.triggered.connect(self.selectFile)
            popupMenu.addAction(actionSaveToFile)

            if isinstance(self.parameter, QgsProcessingParameterFeatureSink) \
                    and self.parameter.supportsNonFileBasedOutput():
                actionSaveToGpkg = QAction(
                    QCoreApplication.translate('DestinationSelectionPanel', 'Save to GeoPackage…'), self.btnSelect)
                actionSaveToGpkg.triggered.connect(self.saveToGeopackage)
                popupMenu.addAction(actionSaveToGpkg)
                actionSaveToPostGIS = QAction(
                    QCoreApplication.translate('DestinationSelectionPanel', 'Save to PostGIS Table…'), self.btnSelect)
                actionSaveToPostGIS.triggered.connect(self.saveToPostGIS)
                settings = QgsSettings()
                settings.beginGroup('/PostgreSQL/connections/')
                names = settings.childGroups()
                settings.endGroup()
                actionSaveToPostGIS.setEnabled(bool(names))
                popupMenu.addAction(actionSaveToPostGIS)

            actionSetEncoding = QAction(
                QCoreApplication.translate('DestinationSelectionPanel', 'Change File Encoding ({})…').format(self.encoding), self.btnSelect)
            actionSetEncoding.triggered.connect(self.selectEncoding)
            popupMenu.addAction(actionSetEncoding)

            popupMenu.exec_(QCursor.pos())

    def saveToTemporary(self):
        if isinstance(self.parameter, QgsProcessingParameterFeatureSink) and self.parameter.supportsNonFileBasedOutput():
            self.leText.setPlaceholderText(self.SAVE_TO_TEMP_LAYER)
        elif isinstance(self.parameter, QgsProcessingParameterFolderDestination):
            self.leText.setPlaceholderText(self.SAVE_TO_TEMP_FOLDER)
        else:
            self.leText.setPlaceholderText(self.SAVE_TO_TEMP_FILE)
        self.leText.setText('')
        self.use_temporary = True
        self.skipOutputChanged.emit(False)
        self.destinationChanged.emit()

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

            self.skipOutputChanged.emit(False)
            self.destinationChanged.emit()

    def saveToGeopackage(self):
        file_filter = self.tr('GeoPackage files (*.gpkg);;All files (*.*)', 'OutputFile')

        settings = QgsSettings()
        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)

        filename, filter = QFileDialog.getSaveFileName(self, self.tr("Save to GeoPackage"), path,
                                                       file_filter, options=QFileDialog.DontConfirmOverwrite)

        if not filename:
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

            self.skipOutputChanged.emit(False)
            self.destinationChanged.emit()

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

            self.skipOutputChanged.emit(False)
            self.destinationChanged.emit()

    def selectEncoding(self):
        dialog = QgsEncodingSelectionDialog(
            self, self.tr('File encoding'), self.encoding)
        if dialog.exec_() == QDialog.Accepted:
            self.encoding = dialog.encoding()
            settings = QgsSettings()
            settings.setValue('/Processing/encoding', self.encoding)
            self.destinationChanged.emit()
        dialog.deleteLater()

    def selectDirectory(self):
        lastDir = self.leText.text()
        settings = QgsSettings()
        if not lastDir:
            lastDir = settings.value("/Processing/LastOutputPath", QDir.homePath())

        dirName = QFileDialog.getExistingDirectory(self, self.tr('Select Directory'),
                                                   lastDir, QFileDialog.ShowDirsOnly)
        if dirName:
            self.leText.setText(QDir.toNativeSeparators(dirName))
            settings.setValue('/Processing/LastOutputPath', dirName)
            self.use_temporary = False
            self.skipOutputChanged.emit(False)
            self.destinationChanged.emit()

    def setValue(self, value):
        if not value:
            if self.parameter.flags() & QgsProcessingParameterDefinition.FlagOptional and \
                    not self.parameter.createByDefault():
                self.skipOutput()
            else:
                self.saveToTemporary()
        else:
            if value == 'memory:':
                self.saveToTemporary()
            elif isinstance(value, QgsProcessingOutputLayerDefinition):
                if value.sink.staticValue() in ('memory:', ''):
                    self.saveToTemporary()
                else:
                    self.leText.setText(value.sink.staticValue())
                    self.use_temporary = False
                    self.skipOutputChanged.emit(False)
                    self.destinationChanged.emit()
                self.encoding = value.createOptions['fileEncoding']
            else:
                self.leText.setText(value)
                self.use_temporary = False
                self.skipOutputChanged.emit(False)
                self.destinationChanged.emit()

    def getValue(self):
        key = None
        if self.use_temporary and isinstance(self.parameter, QgsProcessingParameterFeatureSink):
            key = 'memory:'
        elif self.use_temporary and not self.default_selection:
            key = self.parameter.generateTemporaryDestination()
        else:
            key = self.leText.text()

        if not key and self.parameter.flags() & QgsProcessingParameterDefinition.FlagOptional:
            return None

        if key and not key.startswith('memory:') \
                and not key.startswith('ogr:') \
                and not key.startswith('postgres:') \
                and not key.startswith('postgis:'):
            # output should be a file path
            folder = QFileInfo(key).path()
            if folder == '.':
                # output name does not include a folder - use default
                default_folder = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)
                key = QDir(default_folder).filePath(key)

        if isinstance(self.parameter, QgsProcessingParameterFolderDestination):
            return key

        if isinstance(self.parameter, QgsProcessingParameterFileDestination):
            return key

        value = QgsProcessingOutputLayerDefinition(key)
        value.createOptions = {'fileEncoding': self.encoding}
        return value
