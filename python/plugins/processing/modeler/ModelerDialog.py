# -*- coding: utf-8 -*-

"""
***************************************************************************
    ModelerDialog.py
    ---------------------
    Date                 : August 2012
    Copyright            : (C) 2012 by Victor Olaya
    Email                : volayaf at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************
"""
from builtins import str

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import codecs
import sys
import os
import math

from qgis.PyQt import uic
from qgis.PyQt.QtCore import Qt, QRectF, QMimeData, QPoint, QPointF, QByteArray, QSize, QSizeF, pyqtSignal
from qgis.PyQt.QtWidgets import QGraphicsView, QTreeWidget, QMessageBox, QFileDialog, QTreeWidgetItem, QSizePolicy, QMainWindow, QShortcut
from qgis.PyQt.QtGui import QIcon, QImage, QPainter, QKeySequence
from qgis.PyQt.QtSvg import QSvgGenerator
from qgis.PyQt.QtPrintSupport import QPrinter
from qgis.core import (QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsSettings,
                       QgsMessageLog,
                       QgsProcessingUtils,
                       QgsProcessingModelAlgorithm,
                       QgsProcessingModelParameter,
                       QgsXmlUtils)
from qgis.gui import QgsMessageBar
from processing.gui.HelpEditionDialog import HelpEditionDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.modeler.ModelerUtils import ModelerUtils
from processing.modeler.ModelerScene import ModelerScene
from qgis.utils import iface

from processing.modeler.WrongModelException import WrongModelException
from qgis.PyQt.QtXml import QDomDocument

pluginPath = os.path.split(os.path.dirname(__file__))[0]
WIDGET, BASE = uic.loadUiType(
    os.path.join(pluginPath, 'ui', 'DlgModeler.ui'))


class ModelerDialog(BASE, WIDGET):

    CANVAS_SIZE = 4000

    update_model = pyqtSignal()

    def __init__(self, model=None):
        super(ModelerDialog, self).__init__(None)
        self.setupUi(self)

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.centralWidget().layout().insertWidget(0, self.bar)

        try:
            self.setDockOptions(self.dockOptions() | QMainWindow.GroupedDragging)
        except:
            pass

        self.mToolbar.setIconSize(iface.iconSize())
        self.mActionOpen.setIcon(
            QgsApplication.getThemeIcon('/mActionFileOpen.svg'))
        self.mActionSave.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.mActionSaveAs.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSaveAs.svg'))
        self.mActionZoomActual.setIcon(
            QgsApplication.getThemeIcon('/mActionZoomActual.svg'))
        self.mActionZoomIn.setIcon(
            QgsApplication.getThemeIcon('/mActionZoomIn.svg'))
        self.mActionZoomOut.setIcon(
            QgsApplication.getThemeIcon('/mActionZoomOut.svg'))
        self.mActionExportImage.setIcon(
            QgsApplication.getThemeIcon('/mActionSaveMapAsImage.svg'))
        self.mActionZoomToItems.setIcon(
            QgsApplication.getThemeIcon('/mActionZoomFullExtent.svg'))
        self.mActionExportPdf.setIcon(
            QgsApplication.getThemeIcon('/mActionSaveAsPDF.svg'))
        self.mActionExportSvg.setIcon(
            QgsApplication.getThemeIcon('/mActionSaveAsSVG.svg'))
        self.mActionExportPython.setIcon(
            QgsApplication.getThemeIcon('/mActionSaveAsPython.svg'))
        self.mActionEditHelp.setIcon(
            QgsApplication.getThemeIcon('/mActionEditHelpContent.svg'))
        self.mActionRun.setIcon(
            QgsApplication.getThemeIcon('/mActionStart.svg'))

        self.addDockWidget(Qt.LeftDockWidgetArea, self.propertiesDock)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.inputsDock)
        self.addDockWidget(Qt.LeftDockWidgetArea, self.algorithmsDock)
        self.tabifyDockWidget(self.inputsDock, self.algorithmsDock)
        self.inputsDock.raise_()

        self.zoom = 1

        self.setWindowFlags(Qt.WindowMinimizeButtonHint |
                            Qt.WindowMaximizeButtonHint |
                            Qt.WindowCloseButtonHint)

        settings = QgsSettings()
        self.restoreState(settings.value("/Processing/stateModeler", QByteArray()))
        self.restoreGeometry(settings.value("/Processing/geometryModeler", QByteArray()))

        self.scene = ModelerScene(self, dialog=self)
        self.scene.setSceneRect(QRectF(0, 0, self.CANVAS_SIZE, self.CANVAS_SIZE))

        self.view.setScene(self.scene)
        self.view.setAcceptDrops(True)
        self.view.ensureVisible(0, 0, 10, 10)

        def _dragEnterEvent(event):
            if event.mimeData().hasText():
                event.acceptProposedAction()
            else:
                event.ignore()

        def _dropEvent(event):
            if event.mimeData().hasText():
                text = event.mimeData().text()
                if text in ModelerParameterDefinitionDialog.paramTypes:
                    self.addInputOfType(text, event.pos())
                else:
                    alg = QgsApplication.processingRegistry().createAlgorithmById(text)
                    if alg is not None:
                        self._addAlgorithm(alg, event.pos())
                event.accept()
            else:
                event.ignore()

        def _dragMoveEvent(event):
            if event.mimeData().hasText():
                event.accept()
            else:
                event.ignore()

        def _wheelEvent(event):
            self.view.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)

            settings = QgsSettings()
            factor = settings.value('/qgis/zoom_favor', 2.0)

            # "Normal" mouse has an angle delta of 120, precision mouses provide data
            # faster, in smaller steps
            factor = 1.0 + (factor - 1.0) / 120.0 * abs(event.angleDelta().y())

            if (event.modifiers() == Qt.ControlModifier):
                factor = 1.0 + (factor - 1.0) / 20.0

            if event.angleDelta().y() < 0:
                factor = 1 / factor

            self.view.scale(factor, factor)

        def _enterEvent(e):
            QGraphicsView.enterEvent(self.view, e)
            self.view.viewport().setCursor(Qt.ArrowCursor)

        def _mouseReleaseEvent(e):
            QGraphicsView.mouseReleaseEvent(self.view, e)
            self.view.viewport().setCursor(Qt.ArrowCursor)

        def _mousePressEvent(e):
            if e.button() == Qt.MidButton:
                self.previousMousePos = e.pos()
            else:
                QGraphicsView.mousePressEvent(self.view, e)

        def _mouseMoveEvent(e):
            if e.buttons() == Qt.MidButton:
                offset = self.previousMousePos - e.pos()
                self.previousMousePos = e.pos()

                self.view.verticalScrollBar().setValue(self.view.verticalScrollBar().value() + offset.y())
                self.view.horizontalScrollBar().setValue(self.view.horizontalScrollBar().value() + offset.x())
            else:
                QGraphicsView.mouseMoveEvent(self.view, e)

        self.view.setDragMode(QGraphicsView.ScrollHandDrag)
        self.view.dragEnterEvent = _dragEnterEvent
        self.view.dropEvent = _dropEvent
        self.view.dragMoveEvent = _dragMoveEvent
        self.view.wheelEvent = _wheelEvent
        self.view.enterEvent = _enterEvent
        self.view.mousePressEvent = _mousePressEvent
        self.view.mouseMoveEvent = _mouseMoveEvent

        def _mimeDataInput(items):
            mimeData = QMimeData()
            text = items[0].text(0)
            mimeData.setText(text)
            return mimeData

        self.inputsTree.mimeData = _mimeDataInput

        self.inputsTree.setDragDropMode(QTreeWidget.DragOnly)
        self.inputsTree.setDropIndicatorShown(True)

        def _mimeDataAlgorithm(items):
            item = items[0]
            if isinstance(item, TreeAlgorithmItem):
                mimeData = QMimeData()
                mimeData.setText(item.alg.id())
            return mimeData

        self.algorithmTree.mimeData = _mimeDataAlgorithm

        self.algorithmTree.setDragDropMode(QTreeWidget.DragOnly)
        self.algorithmTree.setDropIndicatorShown(True)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr('Search...'))
        if hasattr(self.textName, 'setPlaceholderText'):
            self.textName.setPlaceholderText(self.tr('Enter model name here'))
        if hasattr(self.textGroup, 'setPlaceholderText'):
            self.textGroup.setPlaceholderText(self.tr('Enter group name here'))

        # Connect signals and slots
        self.inputsTree.doubleClicked.connect(self.addInput)
        self.searchBox.textChanged.connect(self.fillAlgorithmTree)
        self.algorithmTree.doubleClicked.connect(self.addAlgorithm)

        # Ctrl+= should also trigger a zoom in action
        ctrlEquals = QShortcut(QKeySequence("Ctrl+="), self)
        ctrlEquals.activated.connect(self.zoomIn)

        self.mActionOpen.triggered.connect(self.openModel)
        self.mActionSave.triggered.connect(self.save)
        self.mActionSaveAs.triggered.connect(self.saveAs)
        self.mActionZoomIn.triggered.connect(self.zoomIn)
        self.mActionZoomOut.triggered.connect(self.zoomOut)
        self.mActionZoomActual.triggered.connect(self.zoomActual)
        self.mActionZoomToItems.triggered.connect(self.zoomToItems)
        self.mActionExportImage.triggered.connect(self.exportAsImage)
        self.mActionExportPdf.triggered.connect(self.exportAsPdf)
        self.mActionExportSvg.triggered.connect(self.exportAsSvg)
        self.mActionExportPython.triggered.connect(self.exportAsPython)
        self.mActionEditHelp.triggered.connect(self.editHelp)
        self.mActionRun.triggered.connect(self.runModel)

        if model is not None:
            self.model = model.create()
            self.model.setSourceFilePath(model.sourceFilePath())
            self.textGroup.setText(self.model.group())
            self.textName.setText(self.model.displayName())
            self.repaintModel()

        else:
            self.model = QgsProcessingModelAlgorithm()
            self.model.setProvider(QgsApplication.processingRegistry().providerById('model'))

        self.fillInputsTree()
        self.fillAlgorithmTree()

        self.view.centerOn(0, 0)
        self.help = None

        self.hasChanged = False

    def closeEvent(self, evt):
        settings = QgsSettings()
        settings.setValue("/Processing/stateModeler", self.saveState())
        settings.setValue("/Processing/geometryModeler", self.saveGeometry())

        if self.hasChanged:
            ret = QMessageBox.question(
                self, self.tr('Save?'),
                self.tr('There are unsaved changes in this model, do you want to keep those?'),
                QMessageBox.Save | QMessageBox.Cancel | QMessageBox.Discard, QMessageBox.Cancel)

            if ret == QMessageBox.Save:
                self.saveModel(False)
                evt.accept()
            elif ret == QMessageBox.Discard:
                evt.accept()
            else:
                evt.ignore()
        else:
            evt.accept()

    def editHelp(self):
        alg = self.model
        dlg = HelpEditionDialog(alg)
        dlg.exec_()
        if dlg.descriptions:
            self.model.setHelpContent(dlg.descriptions)
            self.hasChanged = True

    def runModel(self):
        if len(self.model.childAlgorithms()) == 0:
            self.bar.pushMessage("", "Model doesn't contain any algorithm and/or parameter and can't be executed", level=QgsMessageBar.WARNING, duration=5)
            return

        dlg = AlgorithmDialog(self.model)
        dlg.exec_()
        # have to manually delete the dialog - otherwise it's owned by the
        # iface mainWindow and never deleted
        dlg.deleteLater()

    def save(self):
        self.saveModel(False)

    def saveAs(self):
        self.saveModel(True)

    def zoomIn(self):
        self.view.setTransformationAnchor(QGraphicsView.NoAnchor)
        point = self.view.mapToScene(QPoint(self.view.viewport().width() / 2, self.view.viewport().height() / 2))

        settings = QgsSettings()
        factor = settings.value('/qgis/zoom_favor', 2.0)

        self.view.scale(factor, factor)
        self.view.centerOn(point)
        self.repaintModel()

    def zoomOut(self):
        self.view.setTransformationAnchor(QGraphicsView.NoAnchor)
        point = self.view.mapToScene(QPoint(self.view.viewport().width() / 2, self.view.viewport().height() / 2))

        settings = QgsSettings()
        factor = settings.value('/qgis/zoom_favor', 2.0)
        factor = 1 / factor

        self.view.scale(factor, factor)
        self.view.centerOn(point)
        self.repaintModel()

    def zoomActual(self):
        point = self.view.mapToScene(QPoint(self.view.viewport().width() / 2, self.view.viewport().height() / 2))
        self.view.resetTransform()
        self.view.centerOn(point)

    def zoomToItems(self):
        totalRect = self.scene.itemsBoundingRect()
        totalRect.adjust(-10, -10, 10, 10)
        self.view.fitInView(totalRect, Qt.KeepAspectRatio)

    def exportAsImage(self):
        self.repaintModel(controls=False)
        filename, fileFilter = QFileDialog.getSaveFileName(self,
                                                           self.tr('Save Model As Image'), '',
                                                           self.tr('PNG files (*.png *.PNG)'))
        if not filename:
            return

        if not filename.lower().endswith('.png'):
            filename += '.png'

        totalRect = self.scene.itemsBoundingRect()
        totalRect.adjust(-10, -10, 10, 10)
        imgRect = QRectF(0, 0, totalRect.width(), totalRect.height())

        img = QImage(totalRect.width(), totalRect.height(),
                     QImage.Format_ARGB32_Premultiplied)
        img.fill(Qt.white)
        painter = QPainter()
        painter.setRenderHint(QPainter.Antialiasing)
        painter.begin(img)
        self.scene.render(painter, imgRect, totalRect)
        painter.end()

        img.save(filename)

        self.bar.pushMessage("", "Model was correctly exported as image", level=QgsMessageBar.SUCCESS, duration=5)
        self.repaintModel(controls=True)

    def exportAsPdf(self):
        self.repaintModel(controls=False)
        filename, fileFilter = QFileDialog.getSaveFileName(self,
                                                           self.tr('Save Model As PDF'), '',
                                                           self.tr('PDF files (*.pdf *.PDF)'))
        if not filename:
            return

        if not filename.lower().endswith('.pdf'):
            filename += '.pdf'

        totalRect = self.scene.itemsBoundingRect()
        totalRect.adjust(-10, -10, 10, 10)
        printerRect = QRectF(0, 0, totalRect.width(), totalRect.height())

        printer = QPrinter()
        printer.setOutputFormat(QPrinter.PdfFormat)
        printer.setOutputFileName(filename)
        printer.setPaperSize(QSizeF(printerRect.width(), printerRect.height()), QPrinter.DevicePixel)
        printer.setFullPage(True)

        painter = QPainter(printer)
        self.scene.render(painter, printerRect, totalRect)
        painter.end()

        self.bar.pushMessage("", "Model was correctly exported as PDF", level=QgsMessageBar.SUCCESS, duration=5)
        self.repaintModel(controls=True)

    def exportAsSvg(self):
        self.repaintModel(controls=False)
        filename, fileFilter = QFileDialog.getSaveFileName(self,
                                                           self.tr('Save Model As SVG'), '',
                                                           self.tr('SVG files (*.svg *.SVG)'))
        if not filename:
            return

        if not filename.lower().endswith('.svg'):
            filename += '.svg'

        totalRect = self.scene.itemsBoundingRect()
        totalRect.adjust(-10, -10, 10, 10)
        svgRect = QRectF(0, 0, totalRect.width(), totalRect.height())

        svg = QSvgGenerator()
        svg.setFileName(filename)
        svg.setSize(QSize(totalRect.width(), totalRect.height()))
        svg.setViewBox(svgRect)
        svg.setTitle(self.model.displayName())

        painter = QPainter(svg)
        self.scene.render(painter, svgRect, totalRect)
        painter.end()

        self.bar.pushMessage("", "Model was correctly exported as SVG", level=QgsMessageBar.SUCCESS, duration=5)
        self.repaintModel(controls=True)

    def exportAsPython(self):
        filename, filter = QFileDialog.getSaveFileName(self,
                                                       self.tr('Save Model As Python Script'), '',
                                                       self.tr('Python files (*.py *.PY)'))
        if not filename:
            return

        if not filename.lower().endswith('.py'):
            filename += '.py'

        text = self.model.asPythonCode()
        with codecs.open(filename, 'w', encoding='utf-8') as fout:
            fout.write(text)

        self.bar.pushMessage("", "Model was correctly exported as python script", level=QgsMessageBar.SUCCESS, duration=5)

    def saveModel(self, saveAs):
        if str(self.textGroup.text()).strip() == '' \
                or str(self.textName.text()).strip() == '':
            QMessageBox.warning(
                self, self.tr('Warning'), self.tr('Please enter group and model names before saving')
            )
            return
        self.model.setName(str(self.textName.text()))
        self.model.setGroup(str(self.textGroup.text()))
        if self.model.sourceFilePath() and not saveAs:
            filename = self.model.sourceFilePath()
        else:
            filename, filter = QFileDialog.getSaveFileName(self,
                                                           self.tr('Save Model'),
                                                           ModelerUtils.modelsFolders()[0],
                                                           self.tr('Processing models (*.model3)'))
            if filename:
                if not filename.endswith('.model3'):
                    filename += '.model3'
                self.model.setSourceFilePath(filename)
        if filename:
            if not self.model.toFile(filename):
                if saveAs:
                    QMessageBox.warning(self, self.tr('I/O error'),
                                        self.tr('Unable to save edits. Reason:\n {0}').format(str(sys.exc_info()[1])))
                else:
                    QMessageBox.warning(self, self.tr("Can't save model"),
                                        self.tr("This model can't be saved in its "
                                                "original location (probably you do not "
                                                "have permission to do it). Please, use "
                                                "the 'Save as...' option."))
                return
            self.update_model.emit()
            self.bar.pushMessage("", "Model was correctly saved", level=QgsMessageBar.SUCCESS, duration=5)

            self.hasChanged = False

    def openModel(self):
        filename, selected_filter = QFileDialog.getOpenFileName(self,
                                                                self.tr('Open Model'),
                                                                ModelerUtils.modelsFolders()[0],
                                                                self.tr('Processing models (*.model3 *.MODEL3)'))
        if filename:
            self.loadModel(filename)

    def loadModel(self, filename):
        alg = QgsProcessingModelAlgorithm()
        if alg.fromFile(filename):
            self.model = alg
            self.model.setProvider(QgsApplication.processingRegistry().providerById('model'))
            self.textGroup.setText(alg.group())
            self.textName.setText(alg.name())
            self.repaintModel()

            self.view.centerOn(0, 0)
            self.hasChanged = False
        else:
            QgsMessageLog.logMessage(self.tr('Could not load model {0}').format(filename),
                                     self.tr('Processing'),
                                     QgsMessageLog.CRITICAL)
            QMessageBox.critical(self, self.tr('Could not open model'),
                                 self.tr('The selected model could not be loaded.\n'
                                         'See the log for more information.'))

    def repaintModel(self, controls=True):
        self.scene = ModelerScene(self, dialog=self)
        self.scene.setSceneRect(QRectF(0, 0, self.CANVAS_SIZE,
                                       self.CANVAS_SIZE))
        self.scene.paintModel(self.model, controls)
        self.view.setScene(self.scene)

    def addInput(self):
        item = self.inputsTree.currentItem()
        paramType = str(item.text(0))
        self.addInputOfType(paramType)

    def addInputOfType(self, paramType, pos=None):
        if paramType in ModelerParameterDefinitionDialog.paramTypes:
            dlg = ModelerParameterDefinitionDialog(self.model, paramType)
            dlg.exec_()
            if dlg.param is not None:
                if pos is None:
                    pos = self.getPositionForParameterItem()
                if isinstance(pos, QPoint):
                    pos = QPointF(pos)
                component = QgsProcessingModelParameter(dlg.param.name())
                component.setDescription(dlg.param.name())
                component.setPosition(pos)
                self.model.addModelParameter(dlg.param, component)
                self.repaintModel()
                # self.view.ensureVisible(self.scene.getLastParameterItem())
                self.hasChanged = True

    def getPositionForParameterItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if len(self.model.parameterComponents()) > 0:
            maxX = max([i.position().x() for i in list(self.model.parameterComponents().values())])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
        else:
            newX = MARGIN + BOX_WIDTH / 2
        return QPointF(newX, MARGIN + BOX_HEIGHT / 2)

    def fillInputsTree(self):
        icon = QIcon(os.path.join(pluginPath, 'images', 'input.svg'))
        parametersItem = QTreeWidgetItem()
        parametersItem.setText(0, self.tr('Parameters'))
        for paramType in ModelerParameterDefinitionDialog.paramTypes:
            paramItem = QTreeWidgetItem()
            paramItem.setText(0, paramType)
            paramItem.setIcon(0, icon)
            paramItem.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled)
            parametersItem.addChild(paramItem)
        self.inputsTree.addTopLevelItem(parametersItem)
        parametersItem.setExpanded(True)

    def addAlgorithm(self):
        item = self.algorithmTree.currentItem()
        if isinstance(item, TreeAlgorithmItem):
            alg = QgsApplication.processingRegistry().createAlgorithmById(item.alg.id())
            self._addAlgorithm(alg)

    def _addAlgorithm(self, alg, pos=None):
        dlg = None
        try:
            dlg = alg.getCustomModelerParametersDialog(self.model)
        except:
            pass
        if not dlg:
            dlg = ModelerParametersDialog(alg, self.model)
        if dlg.exec_():
            alg = dlg.createAlgorithm()
            if pos is None:
                alg.setPosition(self.getPositionForAlgorithmItem())
            else:
                alg.setPosition(pos)
            from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
            for i, out in enumerate(alg.modelOutputs()):
                alg.modelOutput(out).setPosition(alg.position() + QPointF(ModelerGraphicItem.BOX_WIDTH, (i + 1.5) *
                                                                          ModelerGraphicItem.BOX_HEIGHT))
            self.model.addChildAlgorithm(alg)
            self.repaintModel()
            self.hasChanged = True

    def getPositionForAlgorithmItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if self.model.childAlgorithms():
            maxX = max([alg.position().x() for alg in list(self.model.childAlgorithms().values())])
            maxY = max([alg.position().y() for alg in list(self.model.childAlgorithms().values())])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
            newY = min(MARGIN + BOX_HEIGHT + maxY, self.CANVAS_SIZE -
                       BOX_HEIGHT)
        else:
            newX = MARGIN + BOX_WIDTH / 2
            newY = MARGIN * 2 + BOX_HEIGHT + BOX_HEIGHT / 2
        return QPointF(newX, newY)

    def fillAlgorithmTree(self):
        self.fillAlgorithmTreeUsingProviders()
        self.algorithmTree.sortItems(0, Qt.AscendingOrder)

        text = str(self.searchBox.text())
        if text != '':
            self.algorithmTree.expandAll()

    def fillAlgorithmTreeUsingProviders(self):
        self.algorithmTree.clear()
        text = str(self.searchBox.text())
        search_strings = text.split(' ')
        qgis_groups = {}
        for provider in QgsApplication.processingRegistry().providers():
            if not provider.isActive():
                continue
            groups = {}

            # Add algorithms
            for alg in provider.algorithms():
                if alg.flags() & QgsProcessingAlgorithm.FlagHideFromModeler:
                    continue
                if alg.id() == self.model.id():
                    continue

                item_text = [alg.displayName().lower()]
                item_text.extend(alg.tags())

                show = not search_strings or all(
                    any(part in t for t in item_text)
                    for part in search_strings)

                if show:
                    if alg.group() in groups:
                        groupItem = groups[alg.group()]
                    elif provider.id() in ('qgis', 'native') and alg.group() in qgis_groups:
                        groupItem = qgis_groups[alg.group()]
                    else:
                        groupItem = QTreeWidgetItem()
                        name = alg.group()
                        groupItem.setText(0, name)
                        groupItem.setToolTip(0, name)
                        groupItem.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
                        if provider.id() in ('qgis', 'native'):
                            groupItem.setIcon(0, provider.icon())
                            qgis_groups[alg.group()] = groupItem
                        else:
                            groups[alg.group()] = groupItem
                    algItem = TreeAlgorithmItem(alg)
                    groupItem.addChild(algItem)

            if len(groups) > 0:
                providerItem = QTreeWidgetItem()
                providerItem.setText(0, provider.name())
                providerItem.setToolTip(0, provider.name())
                providerItem.setIcon(0, provider.icon())
                providerItem.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable)
                for groupItem in list(groups.values()):
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)
                providerItem.setExpanded(text != '')
                for groupItem in list(groups.values()):
                    if text != '':
                        groupItem.setExpanded(True)

        if len(qgis_groups) > 0:
            for groupItem in list(qgis_groups.values()):
                self.algorithmTree.addTopLevelItem(groupItem)
            for groupItem in list(qgis_groups.values()):
                if text != '':
                    groupItem.setExpanded(True)

        self.algorithmTree.sortItems(0, Qt.AscendingOrder)


class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        icon = alg.icon()
        name = alg.displayName()
        self.setIcon(0, icon)
        self.setToolTip(0, name)
        self.setText(0, name)
