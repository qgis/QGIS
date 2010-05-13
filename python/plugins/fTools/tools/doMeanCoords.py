# -*- coding: utf-8 -*-
from PyQt4.QtCore import *
from PyQt4.QtGui import *
import ftools_utils
from qgis.core import *
from math import *
from ui_frmMeanCoords import Ui_Dialog

class Dialog(QDialog, Ui_Dialog):
    def __init__(self, iface, function):
        QDialog.__init__(self)
        self.iface = iface
        self.function = function
        self.setupUi(self)
        self.updateUi()
        QObject.connect(self.toolOut, SIGNAL("clicked()"), self.outFile)
        QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
        self.buttonOk = self.buttonBox_2.button( QDialogButtonBox.Ok )

        # populate layer list
        self.progressBar.setValue(0)
        mapCanvas = self.iface.mapCanvas()
        layers = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(layers)

    def updateUi(self):
        if self.function == 1:
            self.setWindowTitle( self.tr("Mean coordinates") )
            self.sizeValue.setVisible(False)
            self.label_size.setVisible(False)
        elif self.function == 2:
            self.setWindowTitle( self.tr("Standard distance") )
        self.resize(381, 100)

    def update(self, inputLayer):
        self.weightField.clear()
        self.uniqueField.clear()
        self.weightField.addItem( self.tr("(Optional) Weight field") )
        self.uniqueField.addItem( self.tr("(Optional) Unique ID field") )
        self.changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
        changedField = ftools_utils.getFieldList(self.changedLayer)
        for i in changedField:
            if changedField[i].type() == QVariant.Int or changedField[i].type() == QVariant.Double:
                self.weightField.addItem(unicode(changedField[i].name()))
            self.uniqueField.addItem(unicode(changedField[i].name()))

    def accept(self):
        self.buttonOk.setEnabled( False )
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Coordinate statistics"), self.tr("No input vector layer specified"))
        elif self.outShape.text() == "":
            QMessageBox.information(self, self.tr("Coordinate statistics"), self.tr("Please specify output shapefile"))
        else:
            inName = self.inShape.currentText()
            outPath = self.outShape.text()

            if outPath.contains("\\"):
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("\\")) - 1)
            else:
                outName = outPath.right((outPath.length() - outPath.lastIndexOf("/")) - 1)
            if outName.endsWith(".shp"):
                outName = outName.left(outName.length() - 4)
            self.compute(inName, outPath, self.weightField.currentText(), self.sizeValue.value(), self.uniqueField.currentText())
            self.progressBar.setValue(100)
            self.outShape.clear()
            addToTOC = QMessageBox.question(self, self.tr("Coordinate statistics"), self.tr("Created output point shapefile:\n%1\n\nWould you like to add the new layer to the TOC?").arg( outPath ), QMessageBox.Yes, QMessageBox.No, QMessageBox.NoButton)
            if addToTOC == QMessageBox.Yes:
                self.vlayer = QgsVectorLayer(outPath, unicode(outName), "ogr")
                if self.vlayer.geometryType() == QGis.Point:
                    render = QgsSingleSymbolRenderer(QGis.Point)
                    symbol = QgsSymbol(QGis.Point)
                    symbol.setFillColor(Qt.red)
                    symbol.setFillStyle(Qt.SolidPattern)
                    symbol.setColor(Qt.red)
                    symbol.setPointSize(5)
                    render.addSymbol(symbol)
                    self.vlayer.setRenderer(render)
                QgsMapLayerRegistry.instance().addMapLayer(self.vlayer)
        self.progressBar.setValue(0)
        self.buttonOk.setEnabled( True )

    def outFile(self):
        self.outShape.clear()
        ( self.shapefileName, self.encoding ) = ftools_utils.saveDialog( self )
        if self.shapefileName is None or self.encoding is None:
            return
        self.outShape.setText( QString( self.shapefileName ) )

    def compute(self, inName, outName, weightField="", times=1, uniqueField=""):
        vlayer = ftools_utils.getVectorLayerByName(inName)
        provider = vlayer.dataProvider()
        weightIndex = provider.fieldNameIndex(weightField)
        uniqueIndex = provider.fieldNameIndex(uniqueField)
        feat = QgsFeature()
        allAttrs = provider.attributeIndexes()
        provider.select(allAttrs)
        sRs = provider.crs()
        check = QFile(self.shapefileName)
        if check.exists():
            if not QgsVectorFileWriter.deleteShapeFile(self.shapefileName):
                return
        if uniqueIndex <> -1:
            uniqueValues = ftools_utils.getUniqueValues(provider, int( uniqueIndex ) )
            single = False
        else:
            uniqueValues = [QVariant(1)]
            single = True
        if self.function == 2:
            fieldList = { 0 : QgsField("STD_DIST", QVariant.Double), 1 : QgsField("UID", QVariant.String) }
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, QGis.WKBPolygon, sRs)
        else:
            fieldList = { 0 : QgsField("MEAN_X", QVariant.Double), 1 : QgsField("MEAN_Y", QVariant.Double), 2 : QgsField("UID", QVariant.String)  }
            writer = QgsVectorFileWriter(self.shapefileName, self.encoding, fieldList, QGis.WKBPoint, sRs)
        outfeat = QgsFeature()
        points = []
        weights = []
        nFeat = provider.featureCount() * len(uniqueValues)
        nElement = 0
        self.progressBar.setValue(0)
        self.progressBar.setRange(0, nFeat)
        for j in uniqueValues:
            provider.rewind()
            provider.select(allAttrs)
            cx = 0.00
            cy = 0.00
            points = []
            weights = []
            while provider.nextFeature(feat):
                nElement += 1
                self.progressBar.setValue(nElement)
                if single:
                    check = j.toString().trimmed()
                else:
                    check = feat.attributeMap()[uniqueIndex].toString().trimmed()
                if check == j.toString().trimmed():
                    cx = 0.00
                    cy = 0.00
                    if weightIndex == -1:
                        weight = 1.00
                    else:
                        weight = float(feat.attributeMap()[weightIndex].toDouble()[0])
                    geom = QgsGeometry(feat.geometry())
                    geom = ftools_utils.extractPoints(geom)
                    for i in geom:
                        cx += i.x()
                        cy += i.y()
                    points.append(QgsPoint((cx / len(geom)), (cy / len(geom))))
                    weights.append(weight)
            sumWeight = sum(weights)
            cx = 0.00
            cy = 0.00
            item = 0
            for item, i in enumerate(points):
                cx += i.x() * weights[item]
                cy += i.y() * weights[item]
            cx = cx / sumWeight
            cy = cy / sumWeight
            meanPoint = QgsPoint(cx, cy)
            if self.function == 2:
                values = []
                md = 0.00
                sd = 0.00
                dist = QgsDistanceArea()
                item = 0
                for i in points:
                    tempDist = dist.measureLine(i, meanPoint)
                    values.append(tempDist)
                    item += 1
                    md += tempDist
                md = md / item
                for i in values:
                    sd += (i-md)*(i-md)
                sd = sqrt(sd/item)
                outfeat.setGeometry(QgsGeometry.fromPoint(meanPoint).buffer(sd * times, 10))
                outfeat.addAttribute(0, QVariant(sd))
                outfeat.addAttribute(1, QVariant(j))
            else:
                outfeat.setGeometry(QgsGeometry.fromPoint(meanPoint))
                outfeat.addAttribute(0, QVariant(cx))
                outfeat.addAttribute(1, QVariant(cy))
                outfeat.addAttribute(2, QVariant(j))
            writer.addFeature(outfeat)
            if single:
                break
        del writer
