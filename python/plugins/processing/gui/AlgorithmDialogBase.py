# -*- coding: utf-8 -*-

"""
***************************************************************************
    AlgorithmDialogBase.py
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

import os
import webbrowser
import html

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSignal, Qt, QCoreApplication, QByteArray, QUrl
from qgis.PyQt.QtWidgets import QApplication, QDialogButtonBox, QVBoxLayout, QToolButton

from qgis.utils import iface
from qgis.core import (QgsProject,
                       QgsProcessingFeedback,
                       QgsSettings)
from qgis.gui import QgsHelp

from processing.core.ProcessingConfig import ProcessingConfig

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgAlgorithmBase.ui'))


class AlgorithmDialogFeedback(QgsProcessingFeedback):
    """
    Directs algorithm feedback to an algorithm dialog
    """

    error = pyqtSignal(str)
    progress_text = pyqtSignal(str)
    info = pyqtSignal(str)
    command_info = pyqtSignal(str)
    debug_info = pyqtSignal(str)
    console_info = pyqtSignal(str)

    def __init__(self, dialog):
        QgsProcessingFeedback.__init__(self)
        self.dialog = dialog

    def reportError(self, msg):
        self.error.emit(msg)

    def setProgressText(self, text):
        self.progress_text.emit(text)

    def pushInfo(self, msg):
        self.info.emit(msg)

    def pushCommandInfo(self, msg):
        self.command_info.emit(msg)

    def pushDebugInfo(self, msg):
        self.debug_info.emit(msg)

    def pushConsoleInfo(self, msg):
        self.console_info.emit(msg)


class AlgorithmDialogBase(BASE, WIDGET):

    def __init__(self, alg):
        super(AlgorithmDialogBase, self).__init__(iface.mainWindow() if iface else None)
        self.setupUi(self)

        # don't collapse parameters panel
        self.splitter.setCollapsible(0, False)

        # add collapse button to splitter
        splitterHandle = self.splitter.handle(1)
        handleLayout = QVBoxLayout()
        handleLayout.setContentsMargins(0, 0, 0, 0)
        self.btnCollapse = QToolButton(splitterHandle)
        self.btnCollapse.setAutoRaise(True)
        self.btnCollapse.setFixedSize(12, 12)
        self.btnCollapse.setCursor(Qt.ArrowCursor)
        handleLayout.addWidget(self.btnCollapse)
        handleLayout.addStretch()
        splitterHandle.setLayout(handleLayout)

        self.settings = QgsSettings()
        self.splitter.restoreState(self.settings.value("/Processing/dialogBaseSplitter", QByteArray()))
        self.restoreGeometry(self.settings.value("/Processing/dialogBase", QByteArray()))
        self.splitterState = self.splitter.saveState()
        self.splitterChanged(0, 0)

        self.executed = False
        self.mainWidget = None
        self.alg = alg

        self.setWindowTitle(self.alg.displayName())

        self.buttonBox.rejected.connect(self.reject)
        self.buttonBox.accepted.connect(self.accept)

        # Rename OK button to Run
        self.btnRun = self.buttonBox.button(QDialogButtonBox.Ok)
        self.btnRun.setText(self.tr('Run'))

        self.buttonCancel.setEnabled(False)

        self.btnClose = self.buttonBox.button(QDialogButtonBox.Close)

        self.buttonBox.helpRequested.connect(self.openHelp)

        self.btnCollapse.clicked.connect(self.toggleCollapsed)
        self.splitter.splitterMoved.connect(self.splitterChanged)

        # desktop = QDesktopWidget()
        # if desktop.physicalDpiX() > 96:
        # self.txtHelp.setZoomFactor(desktop.physicalDpiX() / 96)

        algHelp = self.formatHelp(self.alg)
        if algHelp is None:
            self.textShortHelp.hide()
        else:
            self.textShortHelp.document().setDefaultStyleSheet('''.summary { margin-left: 10px; margin-right: 10px; }
                                                    h2 { color: #555555; padding-bottom: 15px; }
                                                    a { text-decoration: none; color: #3498db; font-weight: bold; }
                                                    p { color: #666666; }
                                                    b { color: #333333; }
                                                    dl dd { margin-bottom: 5px; }''')
            self.textShortHelp.setHtml(algHelp)

        def linkClicked(url):
            webbrowser.open(url.toString())

        self.textShortHelp.anchorClicked.connect(linkClicked)

        self.showDebug = ProcessingConfig.getSetting(
            ProcessingConfig.SHOW_DEBUG_IN_DIALOG)

    def createFeedback(self):
        feedback = AlgorithmDialogFeedback(self)
        feedback.progressChanged.connect(self.setPercentage)
        feedback.error.connect(self.error)
        feedback.progress_text.connect(self.setText)
        feedback.info.connect(self.setInfo)
        feedback.command_info.connect(self.setCommand)
        feedback.debug_info.connect(self.setDebugInfo)
        feedback.console_info.connect(self.setConsoleInfo)

        self.buttonCancel.clicked.connect(feedback.cancel)
        return feedback

    def formatHelp(self, alg):
        text = alg.shortHelpString()
        if not text:
            return None
        return "<h2>%s</h2>%s" % (alg.displayName(), "".join(["<p>%s</p>" % s for s in text.split("\n")]))

    def closeEvent(self, event):
        self._saveGeometry()
        super(AlgorithmDialogBase, self).closeEvent(event)

    def setMainWidget(self, widget):
        if self.mainWidget is not None:
            QgsProject.instance().layerWasAdded.disconnect(self.mainWidget.layerRegistryChanged)
            QgsProject.instance().layersWillBeRemoved.disconnect(self.mainWidget.layerRegistryChanged)
        self.mainWidget = widget
        self.tabWidget.widget(0).layout().addWidget(self.mainWidget)
        QgsProject.instance().layerWasAdded.connect(self.mainWidget.layerRegistryChanged)
        QgsProject.instance().layersWillBeRemoved.connect(self.mainWidget.layerRegistryChanged)

    def error(self, msg):
        self.setInfo(msg, True)
        self.resetGUI()
        self.tabWidget.setCurrentIndex(1)

    def resetGUI(self):
        self.lblProgress.setText('')
        self.progressBar.setMaximum(100)
        self.progressBar.setValue(0)
        self.btnRun.setEnabled(True)
        self.btnClose.setEnabled(True)

    def setInfo(self, msg, error=False, escape_html=True):
        if error:
            self.txtLog.append('<span style="color:red">{}</span><br />'.format(msg, quote=False))
        elif escape_html:
            self.txtLog.append(html.escape(msg))
        else:
            self.txtLog.append(msg)

    def setCommand(self, cmd):
        if self.showDebug:
            self.txtLog.append('<code>{}<code>'.format(html.escape(cmd, quote=False)))

    def setDebugInfo(self, msg):
        if self.showDebug:
            self.txtLog.append('<span style="color:blue">{}</span>'.format(html.escape(msg, quote=False)))

    def setConsoleInfo(self, msg):
        if self.showDebug:
            self.txtLog.append('<code><span style="color:darkgray">{}</span></code>'.format(html.escape(msg, quote=False)))

    def setPercentage(self, value):
        if self.progressBar.maximum() == 0:
            self.progressBar.setMaximum(100)
        self.progressBar.setValue(value)

    def setText(self, text):
        self.lblProgress.setText(text)
        self.setInfo(text, False)

    def getParamValues(self):
        return {}

    def accept(self):
        pass

    def reject(self):
        self._saveGeometry()
        super(AlgorithmDialogBase, self).reject()

    def finish(self, successful, result, context, feedback):
        pass

    def toggleCollapsed(self):
        if self.helpCollapsed:
            self.splitter.restoreState(self.splitterState)
            self.btnCollapse.setArrowType(Qt.RightArrow)
        else:
            self.splitterState = self.splitter.saveState()
            self.splitter.setSizes([1, 0])
            self.btnCollapse.setArrowType(Qt.LeftArrow)
        self.helpCollapsed = not self.helpCollapsed

    def splitterChanged(self, pos, index):
        if self.splitter.sizes()[1] == 0:
            self.helpCollapsed = True
            self.btnCollapse.setArrowType(Qt.LeftArrow)
        else:
            self.helpCollapsed = False
            self.btnCollapse.setArrowType(Qt.RightArrow)

    def openHelp(self):
        algHelp = self.alg.helpUrl()
        if not algHelp:
            algHelp = QgsHelp.helpUrl("processing_algs/{}/{}".format(
                self.alg.provider().id(), self.alg.id())).toString()

        if algHelp not in [None, ""]:
            webbrowser.open(algHelp)

    def _saveGeometry(self):
        self.settings.setValue("/Processing/dialogBaseSplitter", self.splitter.saveState())
        self.settings.setValue("/Processing/dialogBase", self.saveGeometry())

    class InvalidParameterValue(Exception):

        def __init__(self, param, widget):
            (self.parameter, self.widget) = (param, widget)
