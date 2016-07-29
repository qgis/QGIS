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

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, QSettings
from qgis.PyQt.QtWidgets import QDialog, QMenu, QAction, QFileDialog
from qgis.PyQt.QtGui import QCursor
from qgis.gui import QgsEncodingFileDialog, QgsExpressionBuilderDialog
from qgis.core import (QgsDataSourceURI,
                       QgsCredentials,
                       QgsExpressionContext,
                       QgsExpressionContextUtils,
                       QgsExpression,
                       QgsExpressionContextScope)

from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.outputs import OutputVector
from processing.core.outputs import OutputDirectory
from processing.gui.PostgisTableSelector import PostgisTableSelector

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'widgetBaseSelector.ui'))


class OutputSelectionPanel(BASE, WIDGET):

    SAVE_TO_TEMP_FILE = QCoreApplication.translate(
        'OutputSelectionPanel', '[Save to temporary file]')

    def __init__(self, output, alg):
        super(OutputSelectionPanel, self).__init__(None)
        self.setupUi(self)

        self.output = output
        self.alg = alg

        if hasattr(self.leText, 'setPlaceholderText'):
            self.leText.setPlaceholderText(self.SAVE_TO_TEMP_FILE)

        self.btnSelect.clicked.connect(self.selectOutput)

    def selectOutput(self):
        if isinstance(self.output, OutputDirectory):
            self.selectDirectory()
        else:
            popupMenu = QMenu()

            actionSaveToTempFile = QAction(
                self.tr('Save to a temporary file'), self.btnSelect)
            actionSaveToTempFile.triggered.connect(self.saveToTemporaryFile)
            popupMenu.addAction(actionSaveToTempFile)

            actionSaveToFile = QAction(
                self.tr('Save to file...'), self.btnSelect)
            actionSaveToFile.triggered.connect(self.selectFile)
            popupMenu.addAction(actionSaveToFile)

            actionShowExpressionsBuilder = QAction(
                self.tr('Use expression...'), self.btnSelect)
            actionShowExpressionsBuilder.triggered.connect(self.showExpressionsBuilder)
            popupMenu.addAction(actionShowExpressionsBuilder)

            if isinstance(self.output, OutputVector) \
                    and self.alg.provider.supportsNonFileBasedOutput():
                actionSaveToMemory = QAction(
                    self.tr('Save to memory layer'), self.btnSelect)
                actionSaveToMemory.triggered.connect(self.saveToMemory)
                popupMenu.addAction(actionSaveToMemory)
                actionSaveToSpatialite = QAction(
                    self.tr('Save to Spatialite table...'), self.btnSelect)
                actionSaveToSpatialite.triggered.connect(self.saveToSpatialite)
                popupMenu.addAction(actionSaveToSpatialite)
                actionSaveToPostGIS = QAction(
                    self.tr('Save to PostGIS table...'), self.btnSelect)
                actionSaveToPostGIS.triggered.connect(self.saveToPostGIS)
                settings = QSettings()
                settings.beginGroup('/PostgreSQL/connections/')
                names = settings.childGroups()
                settings.endGroup()
                actionSaveToPostGIS.setEnabled(bool(names))
                popupMenu.addAction(actionSaveToPostGIS)

            popupMenu.exec_(QCursor.pos())

    def showExpressionsBuilder(self):
        dlg = QgsExpressionBuilderDialog(None, self.leText.text(), self, 'generic', self.expressionContext())
        dlg.setWindowTitle(self.tr('Expression based output'))
        if dlg.exec_() == QDialog.Accepted:
            self.leText.setText(dlg.expressionText())

    def expressionContext(self):
        context = QgsExpressionContext()
        context.appendScope(QgsExpressionContextUtils.globalScope())
        context.appendScope(QgsExpressionContextUtils.projectScope())
        processingScope = QgsExpressionContextScope()
        for param in self.alg.parameters:
            processingScope.setVariable('%s_value' % param.name, '')
        context.appendScope(processingScope)
        return context

    def saveToTemporaryFile(self):
        self.leText.setText('')

    def saveToPostGIS(self):
        dlg = PostgisTableSelector(self, self.output.name.lower())
        dlg.exec_()
        if dlg.connection:
            settings = QSettings()
            mySettings = '/PostgreSQL/connections/' + dlg.connection
            dbname = settings.value(mySettings + '/database')
            user = settings.value(mySettings + '/username')
            host = settings.value(mySettings + '/host')
            port = settings.value(mySettings + '/port')
            password = settings.value(mySettings + '/password')
            uri = QgsDataSourceURI()
            uri.setConnection(host, str(port), dbname, user, password)
            uri.setDataSource(dlg.schema, dlg.table,
                              "the_geom" if self.output.hasGeometry() else None)

            connInfo = uri.connectionInfo()
            (success, user, passwd) = QgsCredentials.instance().get(connInfo, None, None)
            if success:
                QgsCredentials.instance().put(connInfo, user, passwd)
            self.leText.setText("postgis:" + uri.uri())

    def saveToSpatialite(self):
        fileFilter = self.output.tr('Spatialite files(*.sqlite)', 'OutputFile')

        settings = QSettings()
        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)

        encoding = settings.value('/Processing/encoding', 'System')
        fileDialog = QgsEncodingFileDialog(
            self, self.tr('Save Spatialite'), path, fileFilter, encoding)
        fileDialog.setFileMode(QFileDialog.AnyFile)
        fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        fileDialog.setConfirmOverwrite(False)

        if fileDialog.exec_() == QDialog.Accepted:
            files = fileDialog.selectedFiles()
            encoding = unicode(fileDialog.encoding())
            self.output.encoding = encoding
            fileName = unicode(files[0])
            selectedFileFilter = unicode(fileDialog.selectedNameFilter())
            if not fileName.lower().endswith(
                    tuple(re.findall("\*(\.[a-z]{1,10})", fileFilter))):
                ext = re.search("\*(\.[a-z]{1,10})", selectedFileFilter)
                if ext:
                    fileName += ext.group(1)
            settings.setValue('/Processing/LastOutputPath',
                              os.path.dirname(fileName))
            settings.setValue('/Processing/encoding', encoding)

            uri = QgsDataSourceURI()
            uri.setDatabase(fileName)
            uri.setDataSource('', self.output.name.lower(),
                              'the_geom' if self.output.hasGeometry() else None)
            self.leText.setText("spatialite:" + uri.uri())

    def saveToMemory(self):
        self.leText.setText('memory:')

    def selectFile(self):
        fileFilter = self.output.getFileFilter(self.alg)

        settings = QSettings()
        if settings.contains('/Processing/LastOutputPath'):
            path = settings.value('/Processing/LastOutputPath')
        else:
            path = ProcessingConfig.getSetting(ProcessingConfig.OUTPUT_FOLDER)

        encoding = settings.value('/Processing/encoding', 'System')
        fileDialog = QgsEncodingFileDialog(
            self, self.tr('Save file'), path, fileFilter, encoding)
        fileDialog.setFileMode(QFileDialog.AnyFile)
        fileDialog.setAcceptMode(QFileDialog.AcceptSave)
        fileDialog.setConfirmOverwrite(True)

        if fileDialog.exec_() == QDialog.Accepted:
            files = fileDialog.selectedFiles()
            encoding = unicode(fileDialog.encoding())
            self.output.encoding = encoding
            fileName = unicode(files[0])
            selectedFileFilter = unicode(fileDialog.selectedNameFilter())
            if not fileName.lower().endswith(
                    tuple(re.findall("\*(\.[a-z]{1,10})", fileFilter))):
                ext = re.search("\*(\.[a-z]{1,10})", selectedFileFilter)
                if ext:
                    fileName += ext.group(1)
            self.leText.setText(fileName)
            settings.setValue('/Processing/LastOutputPath',
                              os.path.dirname(fileName))
            settings.setValue('/Processing/encoding', encoding)

    def selectDirectory(self):
        lastDir = ''
        dirName = QFileDialog.getExistingDirectory(self, self.tr('Select directory'),
                                                   lastDir, QFileDialog.ShowDirsOnly)
        self.leText.setText(dirName)

    def getValue(self):
        fileName = unicode(self.leText.text())
        context = self.expressionContext()
        exp = QgsExpression(fileName)
        if not exp.hasParserError():
            result = exp.evaluate(context)
            if not exp.hasEvalError():
                fileName = result
                if fileName.startswith("[") and fileName.endswith("]"):
                    fileName = fileName[1:-1]
        if fileName.strip() in ['', self.SAVE_TO_TEMP_FILE]:
            value = None
        elif fileName.startswith('memory:'):
            value = fileName
        elif fileName.startswith('postgis:'):
            value = fileName
        elif fileName.startswith('spatialite:'):
            value = fileName
        elif not os.path.isabs(fileName):
            value = ProcessingConfig.getSetting(
                ProcessingConfig.OUTPUT_FOLDER) + os.sep + fileName
        else:
            value = fileName

        return value
