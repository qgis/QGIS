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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import os
import webbrowser

from qgis.PyQt import uic
from qgis.PyQt.QtCore import QCoreApplication, QSettings, QByteArray, QUrl
from qgis.PyQt.QtWidgets import QApplication, QDialogButtonBox, QDesktopWidget
from qgis.PyQt.QtNetwork import QNetworkRequest, QNetworkReply

from qgis.utils import iface
from qgis.core import QgsNetworkAccessManager

from processing.core.ProcessingConfig import ProcessingConfig

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgAlgorithmBase.ui'))


class AlgorithmDialogBase(BASE, WIDGET):

    def __init__(self, alg):
        super(AlgorithmDialogBase, self).__init__(iface.mainWindow())
        self.setupUi(self)

        self.settings = QSettings()
        self.restoreGeometry(self.settings.value("/Processing/dialogBase", QByteArray()))

        self.executed = False
        self.mainWidget = None
        self.alg = alg

        # Rename OK button to Run
        self.btnRun = self.buttonBox.button(QDialogButtonBox.Ok)
        self.btnRun.setText(self.tr('Run'))

        self.btnClose = self.buttonBox.button(QDialogButtonBox.Close)

        self.setWindowTitle(self.alg.displayName())

        #~ desktop = QDesktopWidget()
        #~ if desktop.physicalDpiX() > 96:
        #~ self.txtHelp.setZoomFactor(desktop.physicalDpiX() / 96)

        algHelp = self.alg.shortHelp()
        if algHelp is None:
            self.textShortHelp.setVisible(False)
        else:
            self.textShortHelp.document().setDefaultStyleSheet('''.summary { margin-left: 10px; margin-right: 10px; }
                                                    h2 { color: #555555; padding-bottom: 15px; }
                                                    a { text-decoration: none; color: #3498db; font-weight: bold; }
                                                    p { color: #666666; }
                                                    b { color: #333333; }
                                                    dl dd { margin-bottom: 5px; }''')
            self.textShortHelp.setHtml(algHelp)

        self.textShortHelp.setOpenLinks(False)

        def linkClicked(url):
            webbrowser.open(url.toString())

        self.textShortHelp.anchorClicked.connect(linkClicked)

        isText, algHelp = self.alg.help()
        if algHelp is not None:
            algHelp = algHelp if isText else QUrl(algHelp)
            try:
                if isText:
                    self.txtHelp.setHtml(algHelp)
                else:
                    html = self.tr('<p>Downloading algorithm help... Please wait.</p>')
                    self.txtHelp.setHtml(html)
                    rq = QNetworkRequest(algHelp)
                    self.reply = QgsNetworkAccessManager.instance().get(rq)
                    self.reply.finished.connect(self.requestFinished)
            except Exception, e:
                self.tabWidget.removeTab(2)
        else:
            self.tabWidget.removeTab(2)

        self.showDebug = ProcessingConfig.getSetting(
            ProcessingConfig.SHOW_DEBUG_IN_DIALOG)

    def requestFinished(self):
        """Change the webview HTML content"""
        reply = self.sender()
        if reply.error() != QNetworkReply.NoError:
            html = self.tr('<h2>No help available for this algorithm</h2><p>{}</p>'.format(reply.errorString()))
        else:
            html = unicode(reply.readAll())
        reply.deleteLater()
        self.txtHelp.setHtml(html)

    def closeEvent(self, evt):
        self.settings.setValue("/Processing/dialogBase", self.saveGeometry())
        super(AlgorithmDialogBase, self).closeEvent(evt)

    def setMainWidget(self):
        self.tabWidget.widget(0).layout().addWidget(self.mainWidget)

    def error(self, msg):
        QApplication.restoreOverrideCursor()
        self.setInfo(msg, True)
        self.resetGUI()
        self.tabWidget.setCurrentIndex(1)

    def resetGUI(self):
        QApplication.restoreOverrideCursor()
        self.lblProgress.setText('')
        self.progressBar.setMaximum(100)
        self.progressBar.setValue(0)
        self.btnRun.setEnabled(True)
        self.btnClose.setEnabled(True)

    def setInfo(self, msg, error=False):
        if error:
            self.txtLog.append('<span style="color:red"><br>%s<br></span>' % msg)
        else:
            self.txtLog.append(msg)
        QCoreApplication.processEvents()

    def setCommand(self, cmd):
        if self.showDebug:
            self.setInfo('<code>%s<code>' % cmd)
        QCoreApplication.processEvents()

    def setDebugInfo(self, msg):
        if self.showDebug:
            self.setInfo('<span style="color:blue">%s</span>' % msg)
        QCoreApplication.processEvents()

    def setConsoleInfo(self, msg):
        if self.showDebug:
            self.setCommand('<span style="color:darkgray">%s</span>' % msg)
        QCoreApplication.processEvents()

    def setPercentage(self, value):
        if self.progressBar.maximum() == 0:
            self.progressBar.setMaximum(100)
        self.progressBar.setValue(value)
        QCoreApplication.processEvents()

    def setText(self, text):
        self.lblProgress.setText(text)
        self.setInfo(text, False)
        QCoreApplication.processEvents()

    def setParamValues(self):
        pass

    def setParamValue(self, param, widget, alg=None):
        pass

    def accept(self):
        pass

    def finish(self):
        pass

    class InvalidParameterValue(Exception):

        def __init__(self, param, widget):
            (self.parameter, self.widget) = (param, widget)
