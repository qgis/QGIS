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

from PyQt4.QtCore import QObject, SIGNAL, QFile, QFileInfo
from PyQt4.QtGui import QDialog, QDialogButtonBox, QMessageBox
from qgis.core import QGis, QgsVectorFileWriter, QgsVectorLayer, QgsFeature, QgsFeatureRequest, QgsGeometry

import ftools_utils
from ui_frmEliminate import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface):
        QDialog.__init__(self)
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.setWindowTitle(self.tr("Eliminate sliver polygons"))
        self.buttonOk = self.buttonBox_2.button(QDialogButtonBox.Ok)
        # populate layer list
        self.progressBar.setValue(0)
        self.area.setChecked(True)
        layers = ftools_utils.getLayerNames([QGis.Polygon])
        self.inShape.addItems(layers)

        if len(layers) > 0:
            self.update(layers[0])

    def update(self, inputLayer):
        changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        selFeatures = changedLayer.selectedFeatureCount()
        self.selected.setText( self.tr("Selected features: %s") % (selFeatures))

    def accept(self):
        self.buttonOk.setEnabled(False)

        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Eliminate"), self.tr("No input shapefile specified"))
        else:
            outFileName = self.outShape.text()

            if outFileName == "":
                QMessageBox.information(self, self.tr("Eliminate"), self.tr("Please specify output shapefile"))
                self.buttonOk.setEnabled(True)
                return None
            else:
                outFile = QFile(outFileName)

                if outFile.exists():
                    if not QgsVectorFileWriter.deleteShapeFile(outFileName):
                        QMessageBox.warning(
                            self,
                            self.tr("Delete error"),
                            self.tr("Can't delete file %s") % (outFileName))
                        self.buttonOk.setEnabled(True)
                        return None

                outFileName = unicode(outFileName)

            inLayer = ftools_utils.getVectorLayerByName(unicode(self.inShape.currentText()))

            if inLayer.selectedFeatureCount() == 0:
                QMessageBox.information(self, self.tr("Eliminate"), self.tr("No selection in input layer"))
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
            return None
        self.outShape.setText(outFileName)

    def saveChanges(self, outLayer):
        if outLayer.commitChanges():
            return True
        else:
            msg = ""
            for aStrm in outLayer.commitErrors():
                msg = msg + "\n" + aStrm
            QMessageBox.warning(self, self.tr("Eliminate"), self.tr("Commit error:\n%s") % (msg))
            outLayer.rollBack()
            return False

    def eliminate(self, inLayer, boundary, progressBar, outFileName):
        # keep references to the features to eliminate
        fidsToEliminate = inLayer.selectedFeaturesIds()

        if outFileName: # user wants a new shape file to be created as result
            provider = inLayer.dataProvider()
            error = QgsVectorFileWriter.writeAsVectorFormat(inLayer, outFileName, provider.encoding(), inLayer.crs(), "ESRI Shapefile")

            if error != QgsVectorFileWriter.NoError:
                QMessageBox.warning(self, self.tr("Eliminate"), self.tr("Error creating output file"))
                return None

            outLayer = QgsVectorLayer(outFileName, QFileInfo(outFileName).completeBaseName(), "ogr")

        else:
            QMessageBox.information(self, self.tr("Eliminate"), self.tr("Please specify output shapefile"))
            return None

        # delete features to be eliminated in outLayer
        outLayer.setSelectedFeatures(fidsToEliminate)
        outLayer.startEditing()

        if outLayer.deleteSelectedFeatures():
            if self.saveChanges(outLayer):
                outLayer.startEditing()
        else:
            QMessageBox.warning(self, self.tr("Eliminate"), self.tr("Could not delete features"))
            return None

        # ANALYZE
        start = 20.00
        progressBar.setValue(start)
        add = 80.00 / len(fidsToEliminate)

        lastLen = 0

        # we go through the list and see if we find any polygons we can merge the selected with
        # if we have no success with some we merge and then restart the whole story
        while (lastLen != inLayer.selectedFeatureCount()):  # check if we made any progress
            lastLen = inLayer.selectedFeatureCount()
            fidsToDeselect = []

            #iterate over the polygons to eliminate
            for fid2Eliminate in inLayer.selectedFeaturesIds():
                feat = QgsFeature()

                if inLayer.getFeatures( QgsFeatureRequest().setFilterFid( fid2Eliminate ).setSubsetOfAttributes([]) ).nextFeature( feat ):
                    geom2Eliminate = feat.geometry()
                    bbox = geom2Eliminate.boundingBox()
                    fit = outLayer.getFeatures( QgsFeatureRequest().setFilterRect( bbox ) )
                    mergeWithFid = None
                    mergeWithGeom = None
                    max = 0

                    selFeat = QgsFeature()
                    while fit.nextFeature(selFeat):
                            selGeom = selFeat.geometry()

                            if geom2Eliminate.intersects(selGeom): # we have a candidate
                                iGeom = geom2Eliminate.intersection(selGeom)

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
                                    mergeWithFid = selFeat.id()
                                    mergeWithGeom = QgsGeometry(selGeom) # deep copy of the geometry

                    if mergeWithFid is not None:  # a successful candidate
                        newGeom = mergeWithGeom.combine(geom2Eliminate)

                        if outLayer.changeGeometry(mergeWithFid, newGeom):
                            # write change back to disc
                            if self.saveChanges(outLayer):
                                outLayer.startEditing()
                            else:
                                return None

                            # mark feature as eliminated in inLayer
                            fidsToDeselect.append(fid2Eliminate)
                        else:
                            QMessageBox.warning(
                                self,
                                self.tr("Eliminate"),
                                self.tr("Could not replace geometry of feature with id %s") % (mergeWithFid))
                            return None

                        start = start + add
                        progressBar.setValue(start)
            # end for fid2Eliminate

            # deselect features that are already eliminated in inLayer
            inLayer.deselect(fidsToDeselect)

        #end while

        if inLayer.selectedFeatureCount() > 0:
            # copy all features that could not be eliminated to outLayer
            if outLayer.addFeatures(inLayer.selectedFeatures()):
                # inform user
                fidList = ""

                for fid in inLayer.selectedFeaturesIds():
                    if not fidList == "":
                        fidList += ", "

                    fidList += str(fid)

                QMessageBox.information(
                    self,
                    self.tr("Eliminate"),
                    self.tr("Could not eliminate features with these ids:\n%s") % (fidList))
            else:
                QMessageBox.warning(self, self.tr("Eliminate"), self.tr("Could not add features"))

        # stop editing outLayer and commit any pending changes
        if not self.saveChanges(outLayer):
            return None

        if outFileName:
            if self.addToCanvasCheck.isChecked():
                ftools_utils.addShapeToCanvas(outFileName)
            else:
                QMessageBox.information(
                    self,
                    self.tr("Eliminate"),
                    self.tr("Created output shapefile:\n%s") % (outFileName))

        self.iface.mapCanvas().refresh()
