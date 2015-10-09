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

from math import pow, log
from PyQt4.QtCore import Qt, QObject, SIGNAL, QThread
from PyQt4.QtCore import QLineF, QRectF, QPyNullVariant
from PyQt4.QtGui import QDialog, QApplication, QDialogButtonBox, QMessageBox, QTableWidgetItem, QHeaderView
from PyQt4.QtGui import QGraphicsLineItem, QGraphicsRectItem
from PyQt4.QtGui import QGraphicsTextItem, QGraphicsScene, QBrush
from PyQt4.QtGui import QPen, QColor, QGraphicsView
from qgis.core import QGis, QgsFeature, QgsDistanceArea, QgsFeatureRequest

from ui_frmVisual import Ui_Dialog

import ftools_utils
import math


class VisualDialog(QDialog, Ui_Dialog):

    def __init__(self, iface, function):
        QDialog.__init__(self, iface.mainWindow())
        self.iface = iface
        self.setupUi(self)
        self.myFunction = function

        ## Set object visibility to False if tool is not Check geometry
        self.ckBoxShpError.hide()
        self.browseShpError.hide()
        self.lineEditShpError.hide()
        self.label_6.hide()
        self.line.hide()
        self.addToCanvasCheck.hide()
        self.buttonBox_2.setOrientation(Qt.Horizontal)
        ## Hide the histogram by default
        self.histogramGroupBox.hide()

        if self.myFunction == 2 or self.myFunction == 3:
            QObject.connect(self.inShape, SIGNAL("currentIndexChanged(QString)"), self.update)
            QObject.connect(self.cmbField,
                            SIGNAL("currentIndexChanged(QString)"),
                            self.updatefield)
        self.manageGui()
        self.cancel_close = self.buttonBox_2.button(QDialogButtonBox.Close)
        self.buttonOk = self.buttonBox_2.button(QDialogButtonBox.Ok)
        self.progressBar.setValue(0)
        self.partProgressBar.setValue(0)
        self.partProgressBar.setVisible(False)

    def keyPressEvent(self, e):
        '''
        Reimplemented key press event:
        '''
        if (e.modifiers() == Qt.ControlModifier or e.modifiers() == Qt.MetaModifier) and e.key() == Qt.Key_C:
            #selection = self.tblUnique.selectedItems()
            items = ""
            if self.myFunction in (1, 2):
                for rec in range(self.tblUnique.rowCount()):
                    items += self.tblUnique.item(rec, 0).text() + "\n"
            else:
                for rec in range(self.tblUnique.rowCount()):
                    items += self.tblUnique.item(rec, 0).text() + ":" + self.tblUnique.item(rec, 1).text() + "\n"
            if items:
                clip_board = QApplication.clipboard()
                clip_board.setText(items)
        else:
            QDialog.keyPressEvent(self, e)

    def update(self):
        self.cmbField.clear()
        inputLayer = unicode(self.inShape.currentText())
        if inputLayer != "":
            changedLayer = ftools_utils.getVectorLayerByName(inputLayer)
            changedField = changedLayer.dataProvider().fields()
            # for Basic statistics (with or without selection)
            if self.myFunction == 3:
                if changedLayer.selectedFeatureCount() != 0:
                    self.useSelected.setCheckState(Qt.Checked)
                else:
                    self.useSelected.setCheckState(Qt.Unchecked)
            # add all fields in combobox because now we can work with text fields too
            for f in changedField:
                self.cmbField.addItem(unicode(f.name()))

    def updatefield(self):
        if self.myFunction == 3:
            self.minSpinBox.setValue(0.0)
            self.maxSpinBox.setValue(0.0)
            self.scene.clear()

    def accept(self):
        if self.inShape.currentText() == "":
            QMessageBox.information(self, self.tr("Error!"), self.tr("Please specify input vector layer"))
        elif self.cmbField.isVisible() and self.cmbField.currentText() == "":
            QMessageBox.information(self, self.tr("Error!"), self.tr("Please specify input field"))
        else:
            numberOfBins = self.binsSpinBox.value()
            histMin = self.minSpinBox.value()
            histMax = self.maxSpinBox.value()
            self.visual(self.inShape.currentText(), 
                        self.cmbField.currentText(),
                        self.useSelected.checkState(),
                        [numberOfBins, histMin, histMax])

    def manageGui(self):
        if self.myFunction == 2: # List unique values
            self.setWindowTitle(self.tr("List unique values"))
            self.label_2.setText(self.tr("Unique values"))
            self.label_4.setText(self.tr("Total unique values"))
            self.useSelected.setVisible(False)
        elif self.myFunction == 3: # Basic statistics
            self.setWindowTitle(self.tr("Basics statistics"))
            self.label_2.setText(self.tr("Statistics output"))
            self.histogramGroupBox.setVisible(True)
            self.scene = QGraphicsScene(self)
            self.graphicsView.setScene(self.scene)
            self.label_4.setVisible(False)
            self.lstCount.setVisible(False)
            self.resize(381, 650)
        elif self.myFunction == 4: # Nearest neighbour analysis
            self.setWindowTitle(self.tr("Nearest neighbour analysis"))
            self.cmbField.setVisible(False)
            self.label.setVisible(False)
            self.useSelected.setVisible(False)
            self.label_2.setText(self.tr("Nearest neighbour statistics"))
            self.label_4.setVisible(False)
            self.lstCount.setVisible(False)
            self.resize(381, 200)
        self.inShape.clear()
        if self.myFunction == 4:
            myList = ftools_utils.getLayerNames([QGis.Point])
        else:
            myList = ftools_utils.getLayerNames([QGis.Point, QGis.Line, QGis.Polygon])
        self.inShape.addItems(myList)
        return

#1:  Check geometry (disabled)
#2:  List unique values
#3:  Basic statistics
#4:  Nearest neighbour analysis
    def visual(self, myLayer, myField, mySelection, myParameters):
        vlayer = ftools_utils.getVectorLayerByName(myLayer)
        self.tblUnique.clearContents()
        self.tblUnique.setRowCount(0)
        self.lstCount.clear()
        self.buttonOk.setEnabled(False)

        self.testThread = visualThread(self.iface.mainWindow(), self,
                                       self.myFunction, vlayer, myField,
                                       mySelection, myParameters)
        QObject.connect(self.testThread, SIGNAL("runFinished(PyQt_PyObject)"), self.runFinishedFromThread)
        QObject.connect(self.testThread, SIGNAL("runStatus(PyQt_PyObject)"), self.runStatusFromThread)
        QObject.connect(self.testThread, SIGNAL("runRange(PyQt_PyObject)"), self.runRangeFromThread)
        QObject.connect(self.testThread, SIGNAL("runPartRange(PyQt_PyObject)"), self.runPartRangeFromThread)
        QObject.connect(self.testThread, SIGNAL("runPartStatus(PyQt_PyObject)"), self.runPartStatusFromThread)
        self.cancel_close.setText(self.tr("Cancel"))
        QObject.connect(self.cancel_close, SIGNAL("clicked()"), self.cancelThread)

        QApplication.setOverrideCursor(Qt.WaitCursor)
        self.testThread.start()
        return True

    def cancelThread(self):
        self.testThread.stop()
        QApplication.restoreOverrideCursor()
        self.buttonOk.setEnabled(True)

    def runFinishedFromThread(self, output):
        self.testThread.stop()
        QApplication.restoreOverrideCursor()
        self.buttonOk.setEnabled(True)
        result = output[0]
        numRows = len(result)
        self.tblUnique.setRowCount(numRows)
        if self.myFunction in (1, 2):
            self.tblUnique.setColumnCount(1)
            for rec in range(numRows):
                item = QTableWidgetItem(result[rec])
                self.tblUnique.setItem(rec, 0, item)
        else:
            self.tblUnique.setColumnCount(2)
            for rec in range(numRows):
                tmp = result[rec].split(":")
                item = QTableWidgetItem(tmp[0])
                self.tblUnique.setItem(rec, 0, item)
                item = QTableWidgetItem(tmp[1])
                self.tblUnique.setItem(rec, 1, item)
            self.tblUnique.setHorizontalHeaderLabels([self.tr("Parameter"), self.tr("Value")])
            self.tblUnique.horizontalHeader().setResizeMode(1, QHeaderView.ResizeToContents)
            self.tblUnique.horizontalHeader().setResizeMode(1, QHeaderView.ResizeToContents)
            self.tblUnique.horizontalHeader().show()
        self.tblUnique.horizontalHeader().setResizeMode(0, QHeaderView.Stretch)
        self.tblUnique.resizeRowsToContents()
        if self.myFunction == 3:
            self.histogramGroupBox.setEnabled(False)
            if output[1] is not None and len(output[1]) > 1:
                self.minSpinBox.setValue(output[1][0][0])
                self.histogramGroupBox.setEnabled(True)
                self.maxSpinBox.setValue(output[1][len(output[1])-1][0])
                self.drawHistogram(output[1])
        else:
            self.lstCount.insert(unicode(output[1]))
        self.cancel_close.setText("Close")
        QObject.disconnect(self.cancel_close, SIGNAL("clicked()"), self.cancelThread)
        return True

    def runStatusFromThread(self, status):
        self.progressBar.setValue(status)

    def runRangeFromThread(self, range_vals):
        self.progressBar.setRange(range_vals[0], range_vals[1])

    def runPartStatusFromThread(self, status):
        self.partProgressBar.setValue(status)
        if status >= self.part_max:
            self.partProgressBar.setVisible(False)

    def runPartRangeFromThread(self, range_vals):
        self.part_max = range_vals[1]
        self.partProgressBar.setVisible(True)
        self.partProgressBar.setRange(range_vals[0], range_vals[1])

    def drawHistogram(self, theBins):
        histBins = theBins
        if histBins is None:
            return
        # Find the maximum y axis value for scaling
        maxyvalue = 0
        for i in range(len(histBins)-1):
            if self.cumulativeCheckBox.checkState():
                maxyvalue = maxyvalue + histBins[i][1]
            else:
                if histBins[i][1] > maxyvalue:
                    maxyvalue = histBins[i][1]
        cutoffvalue = maxyvalue
        #if (self.frequencyRangeSpinBox.value() > 0):
        #    cutoffvalue = self.frequencyRangeSpinBox.value()
        if maxyvalue == 0:
            return
        self.scene.clear()
        viewprect = QRectF(self.graphicsView.viewport().rect())
        self.graphicsView.setSceneRect(viewprect)
        bottom = self.graphicsView.sceneRect().bottom()
        top = self.graphicsView.sceneRect().top()
        left = self.graphicsView.sceneRect().left()
        right = self.graphicsView.sceneRect().right()
        height = bottom - top - 1
        width = right - left - 1
        padding = 3
        toppadding = 3
        minValue = histBins[0][0]
        minvaltext = QGraphicsTextItem(str(minValue))
        minvaltextheight = minvaltext.boundingRect().height()
        bottompadding = minvaltextheight
        # Determine the width of the left margin (depends on the y range)
        clog = log(cutoffvalue, 10)
        clogint = int(clog)
        yincr = pow(10, clogint)
        ylabeltext = QGraphicsTextItem(str(int(yincr)))
        # The left padding must accomodate the y labels
        leftpadding = ylabeltext.boundingRect().width()
        # Find the width of the maximium frequency label
        maxfreqtext = QGraphicsTextItem(str(cutoffvalue))
        maxfreqtextwidth = maxfreqtext.boundingRect().width()
        rightpadding = maxfreqtextwidth
        width = width - (leftpadding + rightpadding)
        height = height - (toppadding + bottompadding)
        if len(histBins) <= 1:
            barwidth = width
        else:
            barwidth = width / (len(histBins)-1)
        # Create the histogram
        binsize = 0
        for i in range(len(histBins) - 1):
            if self.cumulativeCheckBox.checkState():
                binsize = binsize + histBins[i][1]
            else:
                binsize = histBins[i][1]
            barheight =  binsize * height / cutoffvalue
            barrect = QGraphicsRectItem(QRectF(leftpadding + barwidth * i,
                        height - barheight + toppadding, barwidth, barheight))
            barbrush = QBrush(QColor(255, 153, 102))
            barrect.setBrush(barbrush)
            self.scene.addItem(barrect)
        # Determine the increments for the horizontal lines
        if (cutoffvalue // yincr <= 5 and yincr > 1):
            yincr = yincr / 2
            if (cutoffvalue // yincr < 5 and yincr > 10):
                yincr = yincr / 2
        # Draw horizontal lines with labels
        yval = yincr
        while (yval <= cutoffvalue):
            scval = height + toppadding - yval * height / cutoffvalue
            hline = QGraphicsLineItem(QLineF(leftpadding - 3, scval,
                                             width + leftpadding, scval))
            hlinepen = QPen(QColor(153, 153, 153))
            hlinepen.setStyle(Qt.DotLine)
            hline.setPen(hlinepen)
            self.scene.addItem(hline)
            ylabtext = QGraphicsTextItem(str(int(yval)))
            ylabtextheight = ylabtext.boundingRect().height()
            ylabtextwidth = ylabtext.boundingRect().width()
            ylabtext.setPos(leftpadding - ylabtextwidth,
                            scval - ylabtextheight / 2)
            if (scval - ylabtextheight / 2 > 0):
                self.scene.addItem(ylabtext)
            yval = yval + yincr
        # Draw frame
        vline1 = QGraphicsLineItem(QLineF(leftpadding - 1, toppadding,
                                 leftpadding - 1, toppadding + height))
        vlinepen = QPen(QColor(153, 153, 153))
        vline1.setPen(vlinepen)
        self.scene.addItem(vline1)
        vline2 = QGraphicsLineItem(QLineF(leftpadding + width + 1, toppadding,
                               leftpadding + width + 1, toppadding + height))
        vline2.setPen(vlinepen)
        self.scene.addItem(vline2)
        hline2 = QGraphicsLineItem(QLineF(leftpadding - 1, toppadding - 1,
                               leftpadding + width + 1, toppadding - 1))
        hline2.setPen(vlinepen)
        self.scene.addItem(hline2)
        # Label the x axis
        minvaltextwidth = minvaltext.boundingRect().width()
        minvaltext.setPos(leftpadding - minvaltextwidth / 2,
                          height + toppadding + bottompadding
                          - minvaltextheight)
        self.scene.addItem(minvaltext)
        maxValue = histBins[len(histBins) - 1][0]
        maxvaltext = QGraphicsTextItem(str(maxValue))
        maxvaltextwidth = maxvaltext.boundingRect().width()
        maxvaltext.setPos(leftpadding + width - maxvaltextwidth / 2,
                          height + toppadding + bottompadding
                          - minvaltextheight)
        self.scene.addItem(maxvaltext)
        maxfreqtext.setPos(leftpadding + width, 0)
        self.scene.addItem(maxfreqtext)

        
        
class visualThread(QThread):

    def __init__(self, parentThread, parentObject, function, vlayer, myField, mySelection, myParameters):
        QThread.__init__(self, parentThread)
        self.parent = parentObject
        self.running = False
        self.myFunction = function
        self.vlayer = vlayer
        self.myField = myField
        self.mySelection = mySelection
        self.myParameters = myParameters

    def run(self):
        self.running = True
        # note that 1 used to be associated with check_geometry
        if self.myFunction == 2: # List unique values
            (lst, cnt) = self.list_unique_values(self.vlayer, self.myField)
        elif self.myFunction == 3: # Basic statistics
            (lst, cnt) = self.basic_statistics(self.vlayer, self.myField, self.myParameters)
        elif self.myFunction == 4: # Nearest neighbour analysis
            (lst, cnt) = self.nearest_neighbour_analysis(self.vlayer)
        self.emit(SIGNAL("runFinished(PyQt_PyObject)"), (lst, cnt))
        self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)

    def stop(self):
        self.running = False

    def list_unique_values(self, vlayer, myField):
        vprovider = vlayer.dataProvider()
        index = vprovider.fieldNameIndex(myField)
        unique = ftools_utils.getUniqueValues(vprovider, int(index))
        lstUnique = []
        nFeat = len(unique)
        nElement = 0
        if nFeat > 0:
            self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)
            self.emit(SIGNAL("runRange(PyQt_PyObject)"), (0, nFeat))
        for item in unique:
            nElement += 1
            self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
            lstUnique.append(unicode(item).strip())
        lstCount = len(unique)
        return (lstUnique, lstCount)

    def basic_statistics(self, vlayer, myField, myParameters):
        vprovider = vlayer.dataProvider()
        index = vprovider.fieldNameIndex(myField)
        feat = QgsFeature()
        sumVal = 0.0
        meanVal = 0.0
        nVal = 0.0
        values = []
        first = True
        nElement = 0
        # determine selected field type
        if ftools_utils.getFieldType(vlayer, myField) in (
            'String', 'varchar', 'char', 'text'
        ):
            fillVal = 0
            emptyVal = 0
            if self.mySelection: # only selected features
                selection = vlayer.selectedFeatures()
                nFeat = vlayer.selectedFeatureCount()
                if nFeat > 0:
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)
                    self.emit(SIGNAL("runRange(PyQt_PyObject)"), (0, nFeat))
                for f in selection:
                    try:
                        lenVal = float(len(f[index]))
                    except TypeError:
                        lenVal = 0
                    if first:
                        minVal = lenVal
                        maxVal = lenVal
                        first = False
                    else:
                        if lenVal < minVal:
                            minVal = lenVal
                        if lenVal > maxVal:
                            maxVal = lenVal
                    if lenVal != 0.00:
                        fillVal += 1
                    else:
                        emptyVal += 1
                    values.append(lenVal)
                    sumVal = sumVal + lenVal
                    nElement += 1
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
            else: # there is no selection, process the whole layer
                nFeat = vprovider.featureCount()
                if nFeat > 0:
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)
                    self.emit(SIGNAL("runRange(PyQt_PyObject)"), (0, nFeat))
                fit = vprovider.getFeatures()
                while fit.nextFeature(feat):
                    try:
                        lenVal = float(len(feat[index]))
                    except TypeError:
                        lenVal = 0
                    if first:
                        minVal = lenVal
                        maxVal = lenVal
                        first = False
                    else:
                        if lenVal < minVal:
                            minVal = lenVal
                        if lenVal > maxVal:
                            maxVal = lenVal
                    if lenVal != 0.00:
                        fillVal += 1
                    else:
                        emptyVal += 1
                    values.append(lenVal)
                    sumVal = sumVal + lenVal
                    nElement += 1
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
            nVal = float(len(values))
            if nVal > 0:
                meanVal = sumVal / nVal
                lstStats = []
                lstStats.append(self.tr("Max. len:") + unicode(maxVal))
                lstStats.append(self.tr("Min. len:") + unicode(minVal))
                lstStats.append(self.tr("Mean. len:") + unicode(meanVal))
                lstStats.append(self.tr("Filled:") + unicode(fillVal))
                lstStats.append(self.tr("Empty:") + unicode(emptyVal))
                lstStats.append(self.tr("N:") + unicode(nVal))
                return (lstStats, [])
            else:
                return (["Error:No features selected!"], [])
        else: # numeric field
            stdVal = 0.00
            cvVal = 0.00
            rangeVal = 0.00
            medianVal = 0.00
            maxVal = 0.00
            minVal = 0.00
            numberOfHistBins = myParameters[0]
            histMin = myParameters[1]
            histMax = myParameters[2]
            if histMin == 0 and histMax == 0:
                histMin = vlayer.minimumValue(index)
                histMax = vlayer.maximumValue(index)
            if isinstance(histMin, QPyNullVariant):
                histMin = 0.0
            if isinstance(histMax, QPyNullVariant):
                histMax = 0.0
            histBinSize = float(histMax - histMin) / float(numberOfHistBins)
            histStatistics = []
            for i in range(numberOfHistBins + 1):
                histStatistics.append([histMin + i * histBinSize, 0])
            if self.mySelection: # only selected features
                selection = vlayer.selectedFeatures()
                nFeat = vlayer.selectedFeatureCount()
                uniqueVal = ftools_utils.getUniqueValuesCount(vlayer, index, True)
                if nFeat > 0:
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)
                    self.emit(SIGNAL("runRange(PyQt_PyObject)"), (0, nFeat))
                for f in selection:
                    if isinstance(f[index], QPyNullVariant):
                        continue
                    value = float(f[index])
                    if first:
                        minVal = value
                        maxVal = value
                        first = False
                    else:
                        if value < minVal:
                            minVal = value
                        if value > maxVal:
                            maxVal = value
                    values.append(value)
                    sumVal = sumVal + value
                    nElement += 1
                    if (value >= histMin) and (value <= histMax):
                        if histBinSize == 0:
                            fittingbin = 0
                        else:
                            fittingbin = int((value - histMin) / histBinSize)
                        if fittingbin >= numberOfHistBins:
                            fittingbin = numberOfHistBins - 1
                        histStatistics[fittingbin][1] = histStatistics[fittingbin][1] + 1
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
            else: # there is no selection, process the whole layer
                nFeat = vprovider.featureCount()
                uniqueVal = ftools_utils.getUniqueValuesCount(vlayer, index, False)
                if nFeat > 0:
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)
                    self.emit(SIGNAL("runRange(PyQt_PyObject)"), (0, nFeat))
                fit = vprovider.getFeatures()
                while fit.nextFeature(feat):
                    if isinstance(feat[index], QPyNullVariant):
                        continue
                    value = float(feat[index])
                    if first:
                        minVal = value
                        maxVal = value
                        first = False
                    else:
                        if value < minVal:
                            minVal = value
                        if value > maxVal:
                            maxVal = value
                    values.append(value)
                    sumVal = sumVal + value
                    nElement += 1
                    if (value >= histMin) and (value <= histMax):
                        if histBinSize == 0:
                            fittingbin = 0
                        else:
                            fittingbin = int((value - histMin) / histBinSize)
                        if fittingbin >= numberOfHistBins:
                            fittingbin = numberOfHistBins - 1
                        histStatistics[fittingbin][1] = histStatistics[fittingbin][1] + 1
                    self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
            nVal = float(len(values))
            if nVal > 0.00:
                rangeVal = maxVal - minVal
                meanVal = sumVal / nVal
                if meanVal != 0.00:
                    for val in values:
                        stdVal += ((val - meanVal) * (val - meanVal))
                    stdVal = math.sqrt(stdVal / nVal)
                    cvVal = stdVal / meanVal
                if nVal > 1:
                    lstVal = sorted(values)
                    if (nVal % 2) == 0:
                        medianVal = 0.5 * (lstVal[int((nVal - 1) / 2)] + lstVal[int((nVal) / 2)])
                    else:
                        medianVal = lstVal[int((nVal + 1) / 2 - 1)]
                lstStats = []
                lstStats.append(self.tr("Mean:") + unicode(meanVal))
                lstStats.append(self.tr("StdDev:") + unicode(stdVal))
                lstStats.append(self.tr("Sum:") + unicode(sumVal))
                lstStats.append(self.tr("Min:") + unicode(minVal))
                lstStats.append(self.tr("Max:") + unicode(maxVal))
                lstStats.append(self.tr("N:") + unicode(nVal))
                lstStats.append(self.tr("CV:") + unicode(cvVal))
                lstStats.append(self.tr("Number of unique values:") + unicode(uniqueVal))
                lstStats.append(self.tr("Range:") + unicode(rangeVal))
                lstStats.append(self.tr("Median:") + unicode(medianVal))
                return (lstStats, histStatistics)
            else:
                return (["Error: No features with non-empty value selected!"], [])

    def nearest_neighbour_analysis(self, vlayer):
        vprovider = vlayer.dataProvider()
        sumDist = 0.00
        distance = QgsDistanceArea()
        A = vlayer.extent()
        A = float(A.width() * A.height())
        index = ftools_utils.createIndex(vprovider)
        nFeat = vprovider.featureCount()
        nElement = 0
        if nFeat > 0:
            self.emit(SIGNAL("runStatus(PyQt_PyObject)"), 0)
            self.emit(SIGNAL("runRange(PyQt_PyObject)"), (0, nFeat))
        feat = QgsFeature()
        neighbour = QgsFeature()
        fit = vprovider.getFeatures()
        while fit.nextFeature(feat):
            neighbourID = index.nearestNeighbor(feat.geometry().asPoint(), 2)[1]
            vprovider.getFeatures(QgsFeatureRequest().setFilterFid(neighbourID).setSubsetOfAttributes([])).nextFeature(neighbour)
            nearDist = distance.measureLine(neighbour.geometry().asPoint(), feat.geometry().asPoint())
            sumDist += nearDist
            nElement += 1
            self.emit(SIGNAL("runStatus(PyQt_PyObject)"), nElement)
        nVal = vprovider.featureCount()
        do = float(sumDist) / nVal
        de = float(0.5 / math.sqrt(nVal / A))
        d = float(do / de)
        SE = float(0.26136 / math.sqrt((nVal * nVal) / A))
        zscore = float((do - de) / SE)
        lstStats = []
        lstStats.append(self.tr("Observed mean distance:") + unicode(do))
        lstStats.append(self.tr("Expected mean distance:") + unicode(de))
        lstStats.append(self.tr("Nearest neighbour index:") + unicode(d))
        lstStats.append(self.tr("N:") + unicode(nVal))
        lstStats.append(self.tr("Z-Score:") + unicode(zscore))
        return (lstStats, [])
