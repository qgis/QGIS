# -*- coding: utf-8 -*-
#-----------------------------------------------------------
#
# fTools
# Copyright (C) 2008-2011  Carson Farmer, edited and improved by Giovanni Allegri (2014)
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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ftools_utils
from qgis.core import *
from ui_frmSelectByLocation import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    TOUCH = 1
    OVERLAP = 2
    WITHIN = 4

    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.opFlags = 0
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        self.buttonOk = self.buttonBox.button( QDialogButtonBox.Ok )
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inPolygon.addItems(layers)
        self.inPoint.addItems(layers)
        self.connect(self.inPoint, SIGNAL("currentIndexChanged(QString)"), self.updateCheck)
        self.cmbModify.addItems([self.tr("creating new selection"), self.tr("adding to current selection"), self.tr("removing from current selection")])

    def updateCheck(self, text):
        vlayer = ftools_utils.getVectorLayerByName(text)
        if vlayer.selectedFeatureCount() > 0:
            self.chkSelected.setChecked(True)
        else:
            self.chkSelected.setChecked(False)

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inPolygon.currentText() == "":
            QMessageBox.information(self, self.tr("Select by location"), self.tr( "Please specify input layer"))
        elif self.inPoint.currentText() == "":
            QMessageBox.information(self, self.tr("Select by location"), self.tr("Please specify select layer"))
        else:
            inLayer = self.inPolygon.currentText()
            selLayer = self.inPoint.currentText()
            self.compute(inLayer, selLayer, self.cmbModify.currentText(), self.chkSelected.isChecked())
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def compute(self, inLayer, selLayer, modify, selection):
        inputLayer = ftools_utils.getVectorLayerByName(inLayer)
        selectLayer = ftools_utils.getVectorLayerByName(selLayer)
        inputProvider = inputLayer.dataProvider()
        selectProvider = selectLayer.dataProvider()
        feat = QgsFeature()
        infeat = QgsFeature()
        geom = QgsGeometry()
        selectedSet = []
        index = ftools_utils.createIndex(inputProvider)

        def _points_op(geomA,geomB):
            return geomA.intersects(geomB)

        def _poly_lines_op(geomA,geomB):
            if geomA.disjoint(geomB):
                return False
            intersects = False
            if self.opFlags & self.TOUCH:
                intersects |= geomA.touches(geomB)
            if not intersects and (self.opFlags & self.OVERLAP):
                if geomB.type() == QGis.Line or geomA.type() == QGis.Line:
                    intersects |= geomA.crosses(geomB)
                else:
                    intersects |= geomA.overlaps(geomB)
            if not intersects and (self.opFlags & self.WITHIN):
                intersects |= geomA.contains(geomB)
            return intersects

        def _sp_operator():
            if inputLayer.geometryType() == QGis.Point:
                return _points_op
            else:
                return _poly_lines_op

        self.opFlags = 0
        if self.chkTouches.checkState() == Qt.Checked:
            self.opFlags |= self.TOUCH
        if self.chkOverlaps.checkState() == Qt.Checked:
            self.opFlags |= self.OVERLAP
        if self.chkContains.checkState() == Qt.Checked:
            self.opFlags |= self.WITHIN

        sp_operator = _sp_operator()

        if selection:
            features = selectLayer.selectedFeatures()
            self.progressBar.setMaximum(len(features))
            for feat in features:
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for id in intersects:
                    inputProvider.getFeatures( QgsFeatureRequest().setFilterFid( int(id) ) ).nextFeature( infeat )
                    tmpGeom = QgsGeometry(infeat.geometry())
                    if sp_operator(geom,tmpGeom):
                        selectedSet.append(infeat.id())
                self.progressBar.setValue(self.progressBar.value()+1)
        else:
            self.progressBar.setMaximum(selectProvider.featureCount())
	    selectFit = selectProvider.getFeatures()
            while selectFit.nextFeature(feat):
                geom = QgsGeometry(feat.geometry())
                intersects = index.intersects(geom.boundingBox())
                for id in intersects:
                    inputProvider.getFeatures( QgsFeatureRequest().setFilterFid( int(id) ) ).nextFeature( infeat )
                    tmpGeom = QgsGeometry( infeat.geometry() )
                    if sp_operator(geom,tmpGeom):
                        selectedSet.append(infeat.id())
                self.progressBar.setValue(self.progressBar.value()+1)
        if modify == self.tr("adding to current selection"):
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).union(selectedSet))
        elif modify == self.tr("removing from current selection"):
            selectedSet = list(set(inputLayer.selectedFeaturesIds()).difference(selectedSet))
        inputLayer.setSelectedFeatures(selectedSet)
