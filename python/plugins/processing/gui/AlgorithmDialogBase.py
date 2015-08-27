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

from PyQt4 import uic
from PyQt4.QtCore import QCoreApplication, QUrl
from PyQt4.QtGui import QApplication, QDialogButtonBox

from qgis.utils import iface

from processing.core.ProcessingConfig import ProcessingConfig

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgAlgorithmBase.ui'))


class AlgorithmDialogBase(BASE, WIDGET):

    class InvalidParameterValue(Exception):

        def __init__(self, param, widget):
            (self.parameter, self.widget) = (param, widget)

    def __init__(self, alg):
        super(AlgorithmDialogBase, self).__init__(iface.mainWindow())
        self.setupUi(self)

        self.executed = False
        self.mainWidget = None
        self.alg = alg

        # Rename OK button to Run
        self.btnRun = self.buttonBox.button(QDialogButtonBox.Ok)
        self.btnRun.setText(self.tr('Run'))

        self.btnClose = self.buttonBox.button(QDialogButtonBox.Close)

        self.setWindowTitle(self.alg.name)

        # load algorithm help if available
        isText, algHelp = self.alg.help()
        if algHelp is not None:
            algHelp = algHelp if isText else QUrl(algHelp)
        else:
            algHelp = self.tr('<h2>Sorry, no help is available for this '
                              'algorithm.</h2>')
        try:
            if isText:
                self.txtHelp.setHtml(algHelp)
            else:
                self.txtHelp.load(algHelp)
        except:
            self.txtHelp.setHtml(
                self.tr('<h2>Could not open help file :-( </h2>'))

        self.showDebug = ProcessingConfig.getSetting(
            ProcessingConfig.SHOW_DEBUG_IN_DIALOG)

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
            self.txtLog.append('<span style="color:red">%s</span>' % msg)
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
