# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ftools_utils
from qgis.core import *
from ui_frmPointsInPolygon import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):

    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inPolygon.addItems(layers)
        self.inPoint.addItems(layers)
        self.updateUI()
        self.connect(self.inPoint, SIGNAL("currentIndexChanged(QString)"), self.updateCheck)
        self.cmbModify.addItems([self.tr("creating new selection"), self.tr("adding to current selection"), self.tr("removing from current selection")])
        
    def updateCheck(self, text):
        vlayer = ftools_utils.getVectorLayerByName(text)
        if vlayer.selectedFeatureCount() > 0:
            self.chkSelected.setChecked(True)
        else:
            self.chkSelected.setChecked(False)

    def updateUI(self):
        self.label_5.setVisible(False)
        self.lnField.setVisible(False)
        self.outShape.setVisible(False)
        self.toolOut.setVisible(False)
        self.label_2.setVisible(False)
        self.setWindowTitle(self.tr("Select by location"))
        self.label_3.setText(self.tr("Select features in:"))
        self.label_4.setText(self.tr("that intersect features in:"))
        self.label_mod = QLabel(self)
        self.label_mod.setObjectName("label_mod")
        self.label_mod.setText(self.tr("Modify current selection by:"))
        self.cmbModify = QComboBox(self)
        self.cmbModify.setObjectName("cmbModify")
        self.chkSelected = QCheckBox(self.tr("Use selected features only"), self)
        self.chkSelected.setObjectName("chkSelected")
        self.gridLayout.addWidget(self.chkSelected,2,0,1,1)
        self.gridLayout.addWidget(self.label_mod,3,0,1,1)
        self.gridLayout.addWidget(self.cmbModify,4,0,1,1)
        self.resize(381, 100)

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inPolygon.currentText() == "":
            QMessageBox.information(self, self.tr("Select by location"), self.tr( "Please specify input layer"))
        elif self.inPoint.currentText() == "":
            QMessageBox.information(self, self.tr("Select by location"), self.tr("Please specify select layer"))
        else:
            inPoly = self.inPolygon.currentText()
            inPts = self.inPoint.currentText()
            self.compute(inPoly, inPts, self.cmbModify.currentText(), self.chkSelected.isChecked())
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def compute(self, inPoly, inPts, modify, selection):
        inputLayer = ftools_utils.getVectorLayerByName(inPoly)
        selectLayer = ftools_utils.getVectorLayerByName(inPts)
        inputProvider = inputLayer.dataProvider()
        allAttrs = inputProvider.attributeIndexes()
        inputProvider.select(allAttrs, QgsRectangle())
        selectProvider = selectLayer.dataProvider()
        allAttrs = selectProvider.attributeIndexes()
        selectProvider.select(allAttrs, QgsRectangle())
        feat = QgsFeature()
        infeat = QgsFeature()
        geom = QgsGeometry()
        selectedSet = []
        index = ftools_utils.createIndex(inputProvider)
        if selection:
            features = selectLayer.selectedFeatures()
            self.progressBar.setMaximum(len(features))
            for feat in features:
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for id in intersects:
                    inputProvider.featureAtId(int(id), infeat, True)
                    tmpGeom = QgsGeometry(infeat.geometry())
                    if geom.intersects(tmpGeom):
                        selectedSet.append(infeat.id())
                self.progressBar.setValue(self.progressBar.value()+1)
        else:        
            self.progressBar.setMaximum(selectProvider.featureCount())
            while selectProvider.nextFeature(feat):
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for id in intersects:
                    inputProvider.featureAtId(int(id), infeat, True)
                    tmpGeom = QgsGeometry( infeat.geometry() )
                    if geom.intersects(tmpGeom):
                        selectedSet.append(infeat.id())
                self.progressBar.setValue(self.progressBar.value()+1)
        if modify == self.tr("adding to current selection"):
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).union(selectedSet))
        elif modify == self.tr("removing from current selection"):
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).difference(selectedSet))
        inputLayer.setSelectedFeatures(selectedSet)
