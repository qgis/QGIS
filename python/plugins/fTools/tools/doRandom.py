# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *

from qgis.core import *
import ftools_utils
from ui_frmRandom import Ui_Dialog
import random
class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.changed)
        self.setWindowTitle(self.tr("Random selection"))
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
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
