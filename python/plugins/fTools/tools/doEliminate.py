# -*- coding: utf-8 -*-
#-----------------------------------------------------------
#
# Eliminate for fTools
# Copyright (C) 2011  Bernhard Ströbl
# EMAIL: bernhard.stroebl@jena.de
#
# Eliminate sliver polygons
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

from PyQt4 import QtCore, QtGui
from qgis.core import *

import ftools_utils
from ui_frmEliminate import Ui_Dialog

class Dialog(QtGui.QDialog, Ui_Dialog):
    def __init__(self, iface):
        QtGui.QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QtCore.QObject.connect(self.toolOut, QtCore.SIGNAL("clicked()"), self.outFile)
        QtCore.QObject.connect(self.inShape, QtCore.SIGNAL("currentIndexChanged(QString)"), self.update)
        QtCore.QObject.connect(self.writeShapefileCheck, QtCore.SIGNAL("stateChanged(int)"), self.on_writeShapefileCheck_stateChanged)
        self.setWindowTitle(self.tr("Eliminate sliver polygons"))
        self.buttonOk = self.buttonBox_2.button(QtGui.QDialogButtonBox.Ok)
        # populate layer list
        self.progressBar.setValue(0)
        self.area.setChecked(True)
        layers = ftools_utils.getLayerNames([QGis.Polygon])
        self.inShape.addItems(layers)

        if len(layers) > 0:
            self.update(layers[0])

        self.on_writeShapefileCheck_stateChanged(self.writeShapefileCheck.checkState())

    def update(self, inputLayer):
        changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        selFeatures = changedLayer.selectedFeatureCount()
        self.selected.setText( self.tr("Selected features: %1").arg(selFeatures))

    def on_writeShapefileCheck_stateChanged(self, newState):
        doEnable = (newState == 2)
        self.outShape.setEnabled(doEnable)
        self.toolOut.setEnabled(doEnable)
        self.addToCanvasCheck.setEnabled(doEnable)

    def accept(self):
        self.buttonOk.setEnabled(False)

        if self.inShape.currentText() == "":
            QtGui.QMessageBox.information(self, self.tr("Eliminate"), self.tr("No input shapefile specified"))
        else:

            if self.writeShapefileCheck.isChecked():
                outFileName = self.outShape.text()

                if outFileName == "":
                    QtGui.MessageBox.information(self, self.tr("Eliminate"), self.tr("Please specify output shapefile"))
                else:
                    outFile = QtCore.QFile(outFileName)

                    if outFile.exists():
                        if not QgsVectorFileWriter.deleteShapeFile(outFileName):
                            QtGui.QMessageBox.warning(self, self.tr("Delete error"),
                                self.tr("Can't delete file %1").arg(outFileName))
                            return

                    outFileName = unicode(outFileName)
            else:
                outFileName = None

            inLayer = ftools_utils.getVectorLayerByName(unicode(self.inShape.currentText()))

            if inLayer.selectedFeatureCount() == 0:
                QtGui.QMessageBox.information(self, self.tr("Eliminate"), self.tr("No selection in input layer"))
            else:
                self.progressBar.setValue(5)
                boundary = self.boundary.isChecked()
                self.eliminate(inLayer, boundary, self.progressBar, outFileName)
                self.progressBar.setValue(100)
                self.outShape.clear()

        self.progressBar.setValue(0)
        self.buttonOk.setEnabled(True)

    def outFile(self):
        self.outShape.clear()
        (outFileName, self.encoding) = ftools_utils.saveDialog(self)
        if outFileName is None or self.encoding is None:
          return
        self.outShape.setText(outFileName)

    def eliminate(self, inLayer, boundary, progressBar, outFileName = None):
        # keep references to the features to eliminate
        fidsToEliminate = inLayer.selectedFeaturesIds()
        fidsToProcess = inLayer.selectedFeaturesIds()

        if outFileName: # user wants a new shape file to be created as result
            provider = inLayer.dataProvider()
            error = QgsVectorFileWriter.writeAsVectorFormat(inLayer, outFileName, provider.encoding(), inLayer.crs(), "ESRI Shapefile")

            if error != QgsVectorFileWriter.NoError:
                QtGui.QMessageBox.warning(self, self.tr("Eliminate"), self.tr("Error creating output file"))
                return None

            outLayer = QgsVectorLayer(outFileName, QtCore.QFileInfo(outFileName).completeBaseName(), "ogr")

        else:
            outLayer = inLayer
            outLayer.removeSelection(False)

        outLayer.startEditing()
        doCommit = True

        # ANALYZE
        start = 20.00
        progressBar.setValue(start)
        add = 80.00 / len(fidsToEliminate)

        lastLen = 0
        geomsToMerge = dict()

        # we go through the list and see if we find any polygons we can merge the selected with
        # if we have no success with some we merge and then restart the whole story
        while (lastLen != len(fidsToProcess)): #check if we made any progress
            lastLen = len(fidsToProcess)
            fidsNotEliminated = []
            fidsToDelete = []

            #iterate over the polygons to eliminate
            for fid in fidsToProcess:
                feat = QgsFeature()

                if outLayer.featureAtId(fid, feat, True, False):
                    geom = feat.geometry()
                    bbox = geom.boundingBox()
                    outLayer.select(bbox, False) # make a new selection
                    mergeWithFid = None
                    mergeWithGeom = None
                    max = 0

                    for selFid in outLayer.selectedFeaturesIds():
                        if fid != selFid:
                            #check if this feature is to be eliminated, too
                            try:
                                found = fidsToEliminate.index(selFid)
                            except ValueError: #selFid is not in fidsToEliminate
                                # check wether the geometry to eliminate and the other geometry intersect
                                selFeat = QgsFeature()

                                if outLayer.featureAtId(selFid, selFeat, True, False):
                                    selGeom = selFeat.geometry()

                                    if geom.intersects(selGeom): # we have a candidate
                                        iGeom = geom.intersection(selGeom)

                                        if boundary:
                                            selValue = iGeom.length()
                                        else:
                                            # we need a common boundary
                                            if 0 < iGeom.length():
                                                selValue = selGeom.area()
                                            else:
                                                selValue = 0

                                        if selValue > max:
                                            max = selValue
                                            mergeWithFid = selFid
                                            mergeWithGeom = QgsGeometry(selGeom) # deep copy of the geometry

                    if mergeWithFid: # a successful candidate
                        try:
                            geomList = geomsToMerge[mergeWithFid]
                        except KeyError:
                            geomList = [mergeWithGeom]

                        geomList.append(QgsGeometry(geom)) # deep copy of the geom
                        geomsToMerge[mergeWithFid] = geomList
                        fidsToDelete.append(fid)

                        start = start + add
                        progressBar.setValue(start)
                    else:
                        fidsNotEliminated.append(fid)

            # PROCESS
            for aFid in geomsToMerge.iterkeys():
                geomList = geomsToMerge[aFid]

                if len(geomList) > 1:
                    for i in range(len(geomList)):
                        aGeom = geomList[i]

                        if i == 0:
                            newGeom = aGeom
                        else:
                            newGeom = newGeom.combine(aGeom)

                    # replace geometry in outLayer
                    if not outLayer.changeGeometry(aFid, newGeom):
                        QtGui.QMessageBox.warning(self, self.tr("Eliminate"),
                            self.tr("Could not replace geometry of feature with id ") + str(aFid))
                        doCommit = False
                        break

            # delete eliminated features
            for aFid in fidsToDelete:
                if not outLayer.deleteFeature(aFid):
                    QtGui.QMessageBox.warning(self, self.tr("Eliminate"),
                        self.tr("Could not delete feature with id ") + str(aFid))
                    doCommit = False
                    break
            # prepare array for the next loop
            fidsToProcess = fidsNotEliminated

        # SAVE CHANGES
        if doCommit:
            if not outLayer.commitChanges():
                QtGui.QMessageBox.warning(self, self.tr("Commit error"), self.tr("Commit Error"))
        else:
            outLayer.rollBack()

        if outFileName:
            if self.addToCanvasCheck.isChecked():
                ftools_utils.addShapeToCanvas(outFileName)
            else:
                QtGui.QMessageBox.information(self, self.tr("Eliminate"),
                    self.tr("Created output shapefile:\n%1").arg(outFileName))

        # inform user
        if len(fidsNotEliminated) > 0:
            fidList = QtCore.QString()

            for fid in fidsNotEliminated:
                if not fidList.isEmpty():
                    fidList.append(", ")

                fidList.append(str(fid))

            QtGui.QMessageBox.information(self, self.tr("Eliminate"),
                    self.tr("Could not eliminate features with these ids:\n%1").arg(fidList))

        self.iface.mapCanvas().refresh()
