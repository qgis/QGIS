# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : January 15, 2018
copyright            : (C) 2018 by Paul Blottiere
email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from qgis.PyQt.QtWidgets import QDialog, QLabel, QHBoxLayout
from qgis.PyQt.QtGui import QMovie
from qgis.PyQt.QtCore import QSize, Qt, pyqtSignal

from qgis.core import QgsApplication

from .ui.ui_DlgCancelTaskQuery import Ui_DlgCancelTaskQuery as Ui_Dialog


class DlgCancelTaskQuery(QDialog, Ui_Dialog):

    canceled = pyqtSignal()

    def __init__(self, parent=None):
        QDialog.__init__(self, parent)
        self.setupUi(self)

        gif = QgsApplication.iconPath("/mIconLoading.gif")
        self.mGif = QMovie(gif)
        self.mGif.setScaledSize(QSize(16, 16))

        self.mMovie.setMovie(self.mGif)
        self.setWindowModality(Qt.ApplicationModal)

        self.mCancelButton.clicked.connect(self.cancel)

        self.cancelStatus = False

    def cancel(self):
        self.mLabel.setText("Stopping SQL...")
        self.cancelStatus = True
        self.mCancelButton.setEnabled(False)
        self.canceled.emit()

    def show(self):
        self.cancelStatus = False
        self.mGif.start()
        self.mCancelButton.setEnabled(True)
        self.mLabel.setText("Executing SQL...")
        super(QDialog, self).show()

    def hide(self):
        self.mGif.stop()
        super(QDialog, self).hide()
