# -*- coding:utf-8 -*-
"""
/***************************************************************************
Module to generate prepared APIs for calltips and auto-completion.
                             -------------------
begin                : 2012-09-10
copyright            : (C) 2012 Larry Shaffer
email                : larrys (at) dakotacarto (dot) com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Portions of this file contain code from Eric4 APIsManager module.
"""

import os

from PyQt4.Qsci import QsciAPIs
from PyQt4.QtGui import QDialog, QDialogButtonBox
from PyQt4.QtCore import QCoreApplication

from ui_console_compile_apis import Ui_APIsDialogPythonConsole

class PrepareAPIDialog(QDialog):
    def __init__(self, api_lexer, api_files, pap_file, parent=None):
        QDialog.__init__(self, parent)
        self.ui = Ui_APIsDialogPythonConsole()
        self.ui.setupUi(self)
        self.setWindowTitle(QCoreApplication.translate("PythonConsole","Compile APIs"))
        self.ui.plainTextEdit.setVisible(False)
        self.ui.textEdit_Qsci.setVisible(False)
        self.adjustSize()
        self._api = None
        self.ui.buttonBox.rejected.connect(self._stopPreparation)
        self._api_files = api_files
        self._api_lexer = api_lexer
        self._pap_file = pap_file

    def _clearLexer(self):
        # self.ui.textEdit_Qsci.setLexer(0)
        self.qlexer = None

    def _stopPreparation(self):
        if self._api is not None:
            self._api.cancelPreparation()
        self._api = None
        self._clearLexer()
        self.close()

    def _preparationFinished(self):
        self._clearLexer()
        if os.path.exists(self._pap_file):
            os.remove(self._pap_file)
        self.ui.label.setText(QCoreApplication.translate("PythonConsole","Saving prepared file..."))
        prepd = self._api.savePrepared(unicode(self._pap_file))
        rslt = self.trUtf8("Error")
        if prepd:
            rslt = QCoreApplication.translate("PythonConsole","Saved")
        self.ui.label.setText(u'{0} {1}'.format(self.ui.label.text(), rslt))
        self._api = None
        self.ui.progressBar.setVisible(False)
        self.ui.buttonBox.button(QDialogButtonBox.Cancel).setText(
            QCoreApplication.translate("PythonConsole","Done"))
        self.adjustSize()

    def prepareAPI(self):
        # self.ui.textEdit_Qsci.setLexer(0)
        exec u'self.qlexer = {0}(self.ui.textEdit_Qsci)'.format(self._api_lexer)
        # self.ui.textEdit_Qsci.setLexer(self.qlexer)
        self._api = QsciAPIs(self.qlexer)
        self._api.apiPreparationFinished.connect(self._preparationFinished)
        for api_file in self._api_files:
            self._api.load(unicode(api_file))
        try:
            self._api.prepare()
        except Exception, err:
            self._api = None
            self._clearLexer()
            self.ui.label.setText(QCoreApplication.translate("PythonConsole","Error preparing file..."))
            self.ui.progressBar.setVisible(False)
            self.ui.plainTextEdit.setVisible(True)
            self.ui.plainTextEdit.insertPlainText(err)
            self.ui.buttonBox.button(QDialogButtonBox.Cancel).setText(
                self.trUtf8("Done"))
            self.adjustSize()
