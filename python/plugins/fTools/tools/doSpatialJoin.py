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

from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ftools_utils
from qgis.core import *
from ui_frmSpatialJoin import Ui_Dialog

def myself(L):
    #median computation
    nVal = len(L)
    if nVal == 1:
        return L[0]
    L.sort()
    #test for list length
    medianVal = 0
    if nVal > 1:
        if ( nVal % 2 ) == 0:
            #index begin at 0
            #remove 1 to index in standard median computation
            medianVal = 0.5 * ( (L[ (nVal) / 2  - 1]) + (L[ (nVal) / 2 ] ))
        else:
            medianVal = L[ (nVal + 1) / 2 - 1]
    return medianVal

class Dialog(QDialog, Ui_Dialog):

    def __init__(self, iface):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        # Set up the user interface from Designer.
        self.setupUi(self)
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        self.setWindowTitle( self.tr("Join attributes by location") )
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )
        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)
        self.joinShape.addItems(layers)

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Spatial Join"), self.tr("Please specify target vector layer") )
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Spatial Join"), self.tr("Please specify output shapefile") )
        elif self.joinShape.currentText() == "":
            QMessageBox.information(self, self.tr("Spatial Join"), self.tr("Please specify join vector layer") )
        elif self.rdoSummary.isChecked() and not (self.chkMean.isChecked() or self.chkSum.isChecked() or self.chkMin.isChecked() or self.chkMax.isChecked() or self.chkMean.isChecked() or self.chkMedian.isChecked()):
            QMessageBox.information(self, self.tr("Spatial Join"), self.tr("Please specify at least one summary statistic") )
        else:
            inName = self.inShape.currentText()
            joinName = self.joinShape.currentText()
            outPath = self.outShape.text()
            if self.rdoSummary.isChecked():
                summary = True
                sumList = []
                if self.chkSum.isChecked(): sumList.append("SUM")
                if self.chkMean.isChecked(): sumList.append("MEAN")
                if self.chkMin.isChecked(): sumList.append("MIN")
                if self.chkMax.isChecked(): sumList.append("MAX")
                if self.chkMedian.isChecked(): sumList.append("MED")
            else:
                summary = False
                sumList = ["all"]
            if self.rdoKeep.isChecked(): keep = True
            else: keep = False
            outName = ftools_utils.getShapefileName( outPath )
            res = self.compute(inName, joinName, outPath, summary, sumList, keep, self.progressBar)
            self.outShape.clear()
            if res:
              addToTOC = QMessageBox.question(self, self.tr("Spatial Join"),
                      self.tr("Created output shapefile:\n%s\n\nWould you like to add the new layer to the TOC?") % (unicode(outPath)), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
              if addToTOC == QMessageBox.Yes:
                self.vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
                QgsMapLayerRegistry.instance().addMapLayers([self.vlayer])
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( self.shapefileName )

    def compute(self, inName, joinName, outName, summary, sumList, keep, progressBar):
        layer1 = ftools_utils.getVectorLayerByName(inName)
        provider1 = layer1.dataProvider()
        fieldList1 = ftools_utils.getFieldList(layer1)

        layer2 = ftools_utils.getVectorLayerByName(joinName)
        provider2 = layer2.dataProvider()

        fieldList2 = ftools_utils.getFieldList(layer2)
        fieldList = QgsFields()
        if provider1.crs() != provider2.crs():
            QMessageBox.warning(self, self.tr("CRS warning!"), self.tr("Warning: Input layers have non-matching CRS.\nThis may cause unexpected results."))
        if not summary:
            fieldList2 = ftools_utils.testForUniqueness(fieldList1, fieldList2)
            seq = range(0, len(fieldList1) + len(fieldList2))
            fieldList1.extend(fieldList2)
            fieldList1 = dict(zip(seq, fieldList1))
        else:
            numFields = {}
            for j in xrange(len(fieldList2)):
                if fieldList2[j].type() == QVariant.Int or fieldList2[j].type() == QVariant.Double:
                    numFields[j] = []
                    for i in sumList:
                        field = QgsField(i + unicode(fieldList2[j].name()), QVariant.Double, "real", 24, 16, self.tr("Summary field") )
                        fieldList.append(field)
            field = QgsField("COUNT", QVariant.Double, "real", 24, 16, self.tr("Summary field") )
            fieldList.append(field)
            fieldList2 = ftools_utils.testForUniqueness(fieldList1, fieldList)
            fieldList1.extend(fieldList)
            seq = range(0, len(fieldList1))
            fieldList1 = dict(zip(seq, fieldList1))

        # check for correct field names
        print fieldList1
        longNames = ftools_utils.checkFieldNameLength( fieldList1.values() )
        if len( longNames ) > 0:
            QMessageBox.warning( self, self.tr( 'Incorrect field names' ),
                        self.tr( 'No output will be created.\nFollowing field names are longer than 10 characters:\n%s' ) % ( "\n".join(longNames) ) )
            return False

        sRs = provider1.crs()
        progressBar.setValue(13)
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                QMessageBox.warning( self, self.tr( 'Error deleting shapefile' ),
                            self.tr( "Can't delete existing shapefile\n%s" ) % ( self.shapefileName ) )
                return False
        fields = QgsFields()
        for f in fieldList1.values():
          fields.append(f)
        writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fields, provider1.geometryType(), sRs)
        #writer = QgsVectorFileWriter(outName, "UTF-8", fieldList1, provider1.geometryType(), sRs)
        inFeat = QgsFeature()
        outFeat = QgsFeature()
        inFeatB = QgsFeature()
        inGeom = QgsGeometry()
        progressBar.setValue(15)
        start = 15.00
        add = 85.00 / provider1.featureCount()

        index = ftools_utils.createIndex(provider2)
        fit1 = provider1.getFeatures()
        while fit1.nextFeature(inFeat):
            inGeom = inFeat.geometry()
            atMap1 = inFeat.attributes()
            outFeat.setGeometry(inGeom)
            none = True
            joinList = []
            if inGeom.type() == QGis.Point:
                #(check, joinList) = layer2.featuresInRectangle(inGeom.buffer(10,2).boundingBox(), True, True)
                #layer2.select(inGeom.buffer(10,2).boundingBox(), False)
                #joinList = layer2.selectedFeatures()
                joinList = index.intersects( inGeom.buffer(10,2).boundingBox() )
                if len(joinList) > 0: check = 0
                else: check = 1
            else:
                #(check, joinList) = layer2.featuresInRectangle(inGeom.boundingBox(), True, True)
                #layer2.select(inGeom.boundingBox(), False)
                #joinList = layer2.selectedFeatures()
                joinList = index.intersects( inGeom.boundingBox() )
                if len(joinList) > 0: check = 0
                else: check = 1
            if check == 0:
                count = 0
                for i in joinList:
                    #tempGeom = i.geometry()
                    provider2.getFeatures( QgsFeatureRequest().setFilterFid( int(i) ) ).nextFeature( inFeatB )
                    if inGeom.intersects(inFeatB.geometry()):
                        count = count + 1
                        none = False
                        atMap2 = inFeatB.attributes()
                        if not summary:
                            atMap = atMap1
                            atMap2 = atMap2
                            atMap.extend(atMap2)
                            atMap = dict(zip(seq, atMap))
                            break
                        else:
                            for j in numFields.keys():
                                numFields[j].append(atMap2[j])
                if summary and not none:
                    atMap = atMap1
                    for j in numFields.keys():
                        for k in sumList:
                            if k == "SUM": atMap.append(sum(numFields[j]))
                            elif k == "MEAN": atMap.append(sum(numFields[j]) / count)
                            elif k == "MIN": atMap.append(min(numFields[j]))
                            elif k == "MED": atMap.append(myself(numFields[j]))
                            else: atMap.append(max(numFields[j]))
                        numFields[j] = []
                    atMap.append(count)
                    atMap = dict(zip(seq, atMap))
            if none:
                outFeat.setAttributes(atMap1)
            else:
                outFeat.setAttributes(atMap.values())
            if keep: # keep all records
                writer.addFeature(outFeat)
            else: # keep only matching records
                if not none:
                    writer.addFeature(outFeat)
            start = start + add
            progressBar.setValue(start)
        del writer
        return True
