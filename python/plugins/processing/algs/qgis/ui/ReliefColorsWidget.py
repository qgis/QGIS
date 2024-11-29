"""
***************************************************************************
    ReliefColorsWidget.py
    ---------------------
    Date                 : December 2016
    Copyright            : (C) 2016 by Alexander Bruy
    Email                : alexander dot bruy at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""

__author__ = "Alexander Bruy"
__date__ = "December 2016"
__copyright__ = "(C) 2016, Alexander Bruy"

import os
import codecs

from qgis.PyQt import uic
from qgis.PyQt.QtCore import pyqtSlot, QDir
from qgis.PyQt.QtGui import QColor, QBrush
from qgis.PyQt.QtWidgets import (
    QTreeWidgetItem,
    QFileDialog,
    QMessageBox,
    QInputDialog,
    QColorDialog,
)
from qgis.PyQt.QtXml import QDomDocument

from qgis.core import QgsApplication, QgsMapLayer
from qgis.analysis import QgsRelief

from processing.gui.wrappers import WidgetWrapper
from processing.tools import system

pluginPath = os.path.dirname(__file__)
WIDGET, BASE = uic.loadUiType(os.path.join(pluginPath, "reliefcolorswidgetbase.ui"))


class ReliefColorsWidget(BASE, WIDGET):

    def __init__(self):
        super().__init__(None)
        self.setupUi(self)

        self.btnAdd.setIcon(QgsApplication.getThemeIcon("/symbologyAdd.svg"))
        self.btnRemove.setIcon(QgsApplication.getThemeIcon("/symbologyRemove.svg"))
        self.btnUp.setIcon(QgsApplication.getThemeIcon("/mActionArrowUp.svg"))
        self.btnDown.setIcon(QgsApplication.getThemeIcon("/mActionArrowDown.svg"))
        self.btnLoad.setIcon(QgsApplication.getThemeIcon("/mActionFileOpen.svg"))
        self.btnSave.setIcon(QgsApplication.getThemeIcon("/mActionFileSave.svg"))
        self.btnAuto.setIcon(QgsApplication.getThemeIcon("/mActionReload.svg"))

        self.layer = None

    @pyqtSlot()
    def on_btnAdd_clicked(self):
        item = QTreeWidgetItem()
        item.setText(0, "0.00")
        item.setText(1, "0.00")
        item.setBackground(2, QBrush(QColor(127, 127, 127)))
        self.reliefClassTree.addTopLevelItem(item)

    @pyqtSlot()
    def on_btnRemove_clicked(self):
        selectedItems = self.reliefClassTree.selectedItems()
        for item in selectedItems:
            self.reliefClassTree.invisibleRootItem().removeChild(item)
            item = None

    @pyqtSlot()
    def on_btnDown_clicked(self):
        selectedItems = self.reliefClassTree.selectedItems()
        for item in selectedItems:
            currentIndex = self.reliefClassTree.indexOfTopLevelItem(item)
            if currentIndex < self.reliefClassTree.topLevelItemCount() - 1:
                self.reliefClassTree.takeTopLevelItem(currentIndex)
                self.reliefClassTree.insertTopLevelItem(currentIndex + 1, item)
                self.reliefClassTree.setCurrentItem(item)

    @pyqtSlot()
    def on_btnUp_clicked(self):
        selectedItems = self.reliefClassTree.selectedItems()
        for item in selectedItems:
            currentIndex = self.reliefClassTree.indexOfTopLevelItem(item)
            if currentIndex > 0:
                self.reliefClassTree.takeTopLevelItem(currentIndex)
                self.reliefClassTree.insertTopLevelItem(currentIndex - 1, item)
                self.reliefClassTree.setCurrentItem(item)

    @pyqtSlot()
    def on_btnLoad_clicked(self):
        fileName, _ = QFileDialog.getOpenFileName(
            None,
            self.tr("Import Colors and elevations from XML"),
            QDir.homePath(),
            self.tr("XML files (*.xml *.XML)"),
        )
        if fileName == "":
            return

        doc = QDomDocument()
        with codecs.open(fileName, "r", encoding="utf-8") as f:
            content = f.read()

        if not doc.setContent(content):
            QMessageBox.critical(
                None,
                self.tr("Error parsing XML"),
                self.tr("The XML file could not be loaded"),
            )
            return

        self.reliefClassTree.clear()
        reliefColorList = doc.elementsByTagName("ReliefColor")
        for i in range(reliefColorList.length()):
            elem = reliefColorList.at(i).toElement()
            item = QTreeWidgetItem()
            item.setText(0, elem.attribute("MinElevation"))
            item.setText(1, elem.attribute("MaxElevation"))
            item.setBackground(
                2,
                QBrush(
                    QColor(
                        int(elem.attribute("red")),
                        int(elem.attribute("green")),
                        int(elem.attribute("blue")),
                    )
                ),
            )
            self.reliefClassTree.addTopLevelItem(item)

    @pyqtSlot()
    def on_btnSave_clicked(self):
        fileName, _ = QFileDialog.getSaveFileName(
            None,
            self.tr("Export Colors and elevations as XML"),
            QDir.homePath(),
            self.tr("XML files (*.xml *.XML)"),
        )

        if fileName == "":
            return

        if not fileName.lower().endswith(".xml"):
            fileName += ".xml"

        doc = QDomDocument()
        colorsElem = doc.createElement("ReliefColors")
        doc.appendChild(colorsElem)

        colors = self.reliefColors()
        for c in colors:
            elem = doc.createElement("ReliefColor")
            elem.setAttribute("MinElevation", str(c.minElevation))
            elem.setAttribute("MaxElevation", str(c.maxElevation))
            elem.setAttribute("red", str(c.color.red()))
            elem.setAttribute("green", str(c.color.green()))
            elem.setAttribute("blue", str(c.color.blue()))
            colorsElem.appendChild(elem)

        with codecs.open(fileName, "w", encoding="utf-8") as f:
            f.write(doc.toString(2))

    @pyqtSlot()
    def on_btnAuto_clicked(self):
        if self.layer is None:
            return

        relief = QgsRelief(self.layer, system.getTempFilename(), "GTiff")
        colors = relief.calculateOptimizedReliefClasses()
        self.populateColors(colors)

    @pyqtSlot(QTreeWidgetItem, int)
    def on_reliefClassTree_itemDoubleClicked(self, item, column):
        if not item:
            return

        if column == 0:
            d, ok = QInputDialog.getDouble(
                None,
                self.tr("Enter lower elevation class bound"),
                self.tr("Elevation"),
                float(item.text(0)),
                decimals=2,
            )
            if ok:
                item.setText(0, str(d))
        elif column == 1:
            d, ok = QInputDialog.getDouble(
                None,
                self.tr("Enter upper elevation class bound"),
                self.tr("Elevation"),
                float(item.text(1)),
                decimals=2,
            )
            if ok:
                item.setText(1, str(d))
        elif column == 2:
            c = QColorDialog.getColor(
                item.background(2).color(),
                None,
                self.tr("Select color for relief class"),
            )
            if c.isValid():
                item.setBackground(2, QBrush(c))

    def reliefColors(self):
        colors = []
        for i in range(self.reliefClassTree.topLevelItemCount()):
            item = self.reliefClassTree.topLevelItem(i)
            if item:
                c = QgsRelief.ReliefColor(
                    item.background(2).color(), float(item.text(0)), float(item.text(1))
                )
                colors.append(c)
        return colors

    def populateColors(self, colors):
        self.reliefClassTree.clear()
        for c in colors:
            item = QTreeWidgetItem()
            item.setText(0, str(c.minElevation))
            item.setText(1, str(c.maxElevation))
            item.setBackground(2, QBrush(c.color))
            self.reliefClassTree.addTopLevelItem(item)

    def setLayer(self, layer):
        self.layer = layer

    def setValue(self, value):
        self.reliefClassTree.clear()
        rows = value.split(";")
        for r in rows:
            v = r.split(",")
            item = QTreeWidgetItem()
            item.setText(0, v[0])
            item.setText(1, v[1])
            color = QColor(int(v[2]), int(v[3]), int(v[4]))
            item.setBackground(2, QBrush(color))
            self.reliefClassTree.addTopLevelItem(item)

    def value(self):
        rColors = self.reliefColors()
        colors = ""
        for c in rColors:
            colors += "{:f}, {:f}, {:d}, {:d}, {:d};".format(
                c.minElevation,
                c.maxElevation,
                c.color.red(),
                c.color.green(),
                c.color.blue(),
            )
        return colors[:-1]


class ReliefColorsWidgetWrapper(WidgetWrapper):

    def createWidget(self):
        return ReliefColorsWidget()

    def postInitialize(self, wrappers):
        for wrapper in wrappers:
            if wrapper.param.name == self.param.parent:
                self.setLayer(wrapper.value())
                wrapper.widgetValueHasChanged.connect(self.parentValueChanged)
                break

    def parentValueChanged(self, wrapper):
        self.setLayer(wrapper.parameterValue())

    def setLayer(self, layer):
        if isinstance(layer, QgsMapLayer):
            layer = layer.source()
        self.widget.setLayer(layer)

    def setValue(self, value):
        self.widget.setValue(value)

    def value(self):
        return self.widget.value()
