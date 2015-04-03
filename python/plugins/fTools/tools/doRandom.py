# -*- coding: utf-8 -*-
#-----------------------------------------------------------
#
# fTools
# Copyright (C) 2008-2011  Carson Farmer
# EMAIL: carson.farmer (at) gmail.com
# WEB  : http://www.ftools.ca/fTools.html
#
# A collection of data management and analysis tools for vector data
#
#-----------------------------------------------------------
#
# licensed under the terms of GNU GPL 2
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#---------------------------------------------------------------------

from PyQt4.QtCore import QObject, SIGNAL
from PyQt4.QtGui import QDialog, QDialogButtonBox, QMessageBox
from qgis.core import QGis

import ftools_utils
from ui_frmRandom import Ui_Dialog
import random
class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.changed)
        self.setWindowTitle(self.tr("Random selection"))
        # populate layer list
        self.progressBar.setValue(0)
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)

    def changed(self, inputLayer):
        changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        changedProvider = changedLayer.dataProvider()
        upperVal = changedProvider.featureCount()
        self.spnNumber.setMaximum(upperVal)

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Random Selection Tool"), self.tr("No input shapefile specified"))
            return
        else:
            self.progressBar.setValue(10)
            inName = self.inShape.currentText()
            self.progressBar.setValue(20)
            layer = ftools_utils.getVectorLayerByName(inName)
            self.progressBar.setValue(30)
            if self.rdoNumber.isChecked():
                value = self.spnNumber.value()
                self.progressBar.setValue(60)
            else:
                value = self.spnPercent.value()
                self.progressBar.setValue(50)
                value = int(round((value / 100.0000), 4) * layer.featureCount())
                self.progressBar.setValue(60)
        selran = random.sample(xrange(0, layer.featureCount()), value)
        self.progressBar.setValue(70)
        self.progressBar.setValue(80)
        self.progressBar.setValue(90)
        self.progressBar.setValue(100)
        layer.setSelectedFeatures(selran)
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )
