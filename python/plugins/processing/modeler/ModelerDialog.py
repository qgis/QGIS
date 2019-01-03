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

__author__ = 'Victor Olaya'
__date__ = 'August 2012'
__copyright__ = '(C) 2012, Victor Olaya'

# This will get replaced with a git SHA1 when you do a git archive

__revision__ = '$Format:%H$'

import codecs
import sys
import operator
import os
import warnings

from qgis.PyQt import uic
from qgis.PyQt.QtCore import (
    Qt,
    QCoreApplication,
    QDir,
    QRectF,
    QMimeData,
    QPoint,
    QPointF,
    QByteArray,
    QSize,
    QSizeF,
    pyqtSignal,
    QDataStream,
    QIODevice,
    QUrl)
from qgis.PyQt.QtWidgets import (QGraphicsView,
                                 QTreeWidget,
                                 QMessageBox,
                                 QFileDialog,
                                 QTreeWidgetItem,
                                 QSizePolicy,
                                 QMainWindow,
                                 QShortcut,
                                 QLabel,
                                 QDockWidget,
                                 QWidget,
                                 QVBoxLayout,
                                 QGridLayout,
                                 QFrame,
                                 QLineEdit)
from qgis.PyQt.QtGui import (QIcon,
                             QImage,
                             QPainter,
                             QKeySequence)
from qgis.PyQt.QtSvg import QSvgGenerator
from qgis.PyQt.QtPrintSupport import QPrinter
from qgis.core import (Qgis,
                       QgsApplication,
                       QgsProcessingAlgorithm,
                       QgsProject,
                       QgsSettings,
                       QgsMessageLog,
                       QgsProcessingUtils,
                       QgsProcessingModelAlgorithm,
                       QgsProcessingModelParameter,
                       QgsProcessingParameterType
                       )
from qgis.gui import (QgsMessageBar,
                      QgsDockWidget,
                      QgsScrollArea,
                      QgsFilterLineEdit,
                      QgsProcessingToolboxTreeView,
                      QgsProcessingToolboxProxyModel)
from processing.gui.HelpEditionDialog import HelpEditionDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.modeler.ModelerUtils import ModelerUtils
from processing.modeler.ModelerScene import ModelerScene
from processing.modeler.ProjectProvider import PROJECT_PROVIDER_ID
from qgis.utils import iface


pluginPath = os.path.split(os.path.dirname(__file__))[0]
with warnings.catch_warnings():
    warnings.filterwarnings("ignore", category=DeprecationWarning)
    WIDGET, BASE = uic.loadUiType(
        os.path.join(pluginPath, 'ui', 'DlgModeler.ui'))


class ModelerToolboxModel(QgsProcessingToolboxProxyModel):

    def __init__(self, parent=None, registry=None, recentLog=None):
        super().__init__(parent, registry, recentLog)

    def flags(self, index):
        f = super().flags(index)
        source_index = self.mapToSource(index)
        if self.toolboxModel().isAlgorithm(source_index):
            f = f | Qt.ItemIsDragEnabled
        return f

    def supportedDragActions(self):
        return Qt.CopyAction


class ModelerDialog(BASE, WIDGET):
    ALG_ITEM = 'ALG_ITEM'
    PROVIDER_ITEM = 'PROVIDER_ITEM'
    GROUP_ITEM = 'GROUP_ITEM'

    NAME_ROLE = Qt.UserRole
    TAG_ROLE = Qt.UserRole + 1
    TYPE_ROLE = Qt.UserRole + 2

    CANVAS_SIZE = 4000

    update_model = pyqtSignal()

    def __init__(self, model=None):
        super().__init__(None)
        self.setAttribute(Qt.WA_DeleteOnClose)

        self.setupUi(self)

        # LOTS of bug reports when we include the dock creation in the UI file
        # see e.g. #16428, #19068
        # So just roll it all by hand......!
        self.propertiesDock = QgsDockWidget(self)
        self.propertiesDock.setFeatures(
            QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.propertiesDock.setObjectName("propertiesDock")
        propertiesDockContents = QWidget()
        self.verticalDockLayout_1 = QVBoxLayout(propertiesDockContents)
        self.verticalDockLayout_1.setContentsMargins(0, 0, 0, 0)
        self.verticalDockLayout_1.setSpacing(0)
        self.scrollArea_1 = QgsScrollArea(propertiesDockContents)
        sizePolicy = QSizePolicy(QSizePolicy.MinimumExpanding,
                                 QSizePolicy.MinimumExpanding)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.scrollArea_1.sizePolicy().hasHeightForWidth())
        self.scrollArea_1.setSizePolicy(sizePolicy)
        self.scrollArea_1.setFocusPolicy(Qt.WheelFocus)
        self.scrollArea_1.setFrameShape(QFrame.NoFrame)
        self.scrollArea_1.setFrameShadow(QFrame.Plain)
        self.scrollArea_1.setWidgetResizable(True)
        self.scrollAreaWidgetContents_1 = QWidget()
        self.gridLayout = QGridLayout(self.scrollAreaWidgetContents_1)
        self.gridLayout.setContentsMargins(6, 6, 6, 6)
        self.gridLayout.setSpacing(4)
        self.label_1 = QLabel(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.label_1, 0, 0, 1, 1)
        self.textName = QLineEdit(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.textName, 0, 1, 1, 1)
        self.label_2 = QLabel(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.label_2, 1, 0, 1, 1)
        self.textGroup = QLineEdit(self.scrollAreaWidgetContents_1)
        self.gridLayout.addWidget(self.textGroup, 1, 1, 1, 1)
        self.label_1.setText(self.tr("Name"))
        self.textName.setToolTip(self.tr("Enter model name here"))
        self.label_2.setText(self.tr("Group"))
        self.textGroup.setToolTip(self.tr("Enter group name here"))
        self.scrollArea_1.setWidget(self.scrollAreaWidgetContents_1)
        self.verticalDockLayout_1.addWidget(self.scrollArea_1)
        self.propertiesDock.setWidget(propertiesDockContents)
        self.addDockWidget(Qt.DockWidgetArea(1), self.propertiesDock)
        self.propertiesDock.setWindowTitle(self.tr("Model properties"))

        self.inputsDock = QgsDockWidget(self)
        self.inputsDock.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.inputsDock.setObjectName("inputsDock")
        self.inputsDockContents = QWidget()
        self.verticalLayout_3 = QVBoxLayout(self.inputsDockContents)
        self.verticalLayout_3.setContentsMargins(0, 0, 0, 0)
        self.scrollArea_2 = QgsScrollArea(self.inputsDockContents)
        sizePolicy.setHeightForWidth(self.scrollArea_2.sizePolicy().hasHeightForWidth())
        self.scrollArea_2.setSizePolicy(sizePolicy)
        self.scrollArea_2.setFocusPolicy(Qt.WheelFocus)
        self.scrollArea_2.setFrameShape(QFrame.NoFrame)
        self.scrollArea_2.setFrameShadow(QFrame.Plain)
        self.scrollArea_2.setWidgetResizable(True)
        self.scrollAreaWidgetContents_2 = QWidget()
        self.verticalLayout = QVBoxLayout(self.scrollAreaWidgetContents_2)
        self.verticalLayout.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout.setSpacing(0)
        self.inputsTree = QTreeWidget(self.scrollAreaWidgetContents_2)
        self.inputsTree.setAlternatingRowColors(True)
        self.inputsTree.header().setVisible(False)
        self.verticalLayout.addWidget(self.inputsTree)
        self.scrollArea_2.setWidget(self.scrollAreaWidgetContents_2)
        self.verticalLayout_3.addWidget(self.scrollArea_2)
        self.inputsDock.setWidget(self.inputsDockContents)
        self.addDockWidget(Qt.DockWidgetArea(1), self.inputsDock)
        self.inputsDock.setWindowTitle(self.tr("Inputs"))

        self.algorithmsDock = QgsDockWidget(self)
        self.algorithmsDock.setFeatures(QDockWidget.DockWidgetFloatable | QDockWidget.DockWidgetMovable)
        self.algorithmsDock.setObjectName("algorithmsDock")
        self.algorithmsDockContents = QWidget()
        self.verticalLayout_4 = QVBoxLayout(self.algorithmsDockContents)
        self.verticalLayout_4.setContentsMargins(0, 0, 0, 0)
        self.scrollArea_3 = QgsScrollArea(self.algorithmsDockContents)
        sizePolicy.setHeightForWidth(self.scrollArea_3.sizePolicy().hasHeightForWidth())
        self.scrollArea_3.setSizePolicy(sizePolicy)
        self.scrollArea_3.setFocusPolicy(Qt.WheelFocus)
        self.scrollArea_3.setFrameShape(QFrame.NoFrame)
        self.scrollArea_3.setFrameShadow(QFrame.Plain)
        self.scrollArea_3.setWidgetResizable(True)
        self.scrollAreaWidgetContents_3 = QWidget()
        self.verticalLayout_2 = QVBoxLayout(self.scrollAreaWidgetContents_3)
        self.verticalLayout_2.setContentsMargins(0, 0, 0, 0)
        self.verticalLayout_2.setSpacing(4)
        self.searchBox = QgsFilterLineEdit(self.scrollAreaWidgetContents_3)
        self.verticalLayout_2.addWidget(self.searchBox)
        self.algorithmTree = QgsProcessingToolboxTreeView(None,
                                                          QgsApplication.processingRegistry())
        self.algorithmTree.setAlternatingRowColors(True)
        self.algorithmTree.header().setVisible(False)
        self.verticalLayout_2.addWidget(self.algorithmTree)
        self.scrollArea_3.setWidget(self.scrollAreaWidgetContents_3)
        self.verticalLayout_4.addWidget(self.scrollArea_3)
        self.algorithmsDock.setWidget(self.algorithmsDockContents)
        self.addDockWidget(Qt.DockWidgetArea(1), self.algorithmsDock)
        self.algorithmsDock.setWindowTitle(self.tr("Algorithms"))
        self.searchBox.setToolTip(self.tr("Enter algorithm name to filter list"))
        self.searchBox.setShowSearchIcon(True)

        self.bar = QgsMessageBar()
        self.bar.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Fixed)
        self.centralWidget().layout().insertWidget(0, self.bar)

        try:
            self.setDockOptions(self.dockOptions() | QMainWindow.GroupedDragging)
        except:
            pass

        if iface is not None:
            self.mToolbar.setIconSize(iface.iconSize())
            self.setStyleSheet(iface.mainWindow().styleSheet())

        self.mActionOpen.setIcon(
            QgsApplication.getThemeIcon('/mActionFileOpen.svg'))
        self.mActionSave.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.mActionSaveAs.setIcon(
            QgsApplication.getThemeIcon('/mActionFileSaveAs.svg'))
        self.mActionSaveInProject.setIcon(
            QgsApplication.getThemeIcon('/mAddToProject.svg'))
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
        #self.mActionExportPython.setIcon(
        #    QgsApplication.getThemeIcon('/mActionSaveAsPython.svg'))
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
            if event.mimeData().hasText() or event.mimeData().hasFormat('application/x-vnd.qgis.qgis.algorithmid'):
                event.acceptProposedAction()
            else:
                event.ignore()

        def _dropEvent(event):
            if event.mimeData().hasFormat('application/x-vnd.qgis.qgis.algorithmid'):
                data = event.mimeData().data('application/x-vnd.qgis.qgis.algorithmid')
                stream = QDataStream(data, QIODevice.ReadOnly)
                algorithm_id = stream.readQString()
                alg = QgsApplication.processingRegistry().createAlgorithmById(algorithm_id)
                if alg is not None:
                    self._addAlgorithm(alg, event.pos())
                else:
                    assert False, algorithm_id
            elif event.mimeData().hasText():
                itemId = event.mimeData().text()
                if itemId in [param.id() for param in QgsApplication.instance().processingRegistry().parameterTypes()]:
                    self.addInputOfType(itemId, event.pos())
                event.accept()
            else:
                event.ignore()

        def _dragMoveEvent(event):
            if event.mimeData().hasText() or event.mimeData().hasFormat('application/x-vnd.qgis.qgis.algorithmid'):
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
            text = items[0].data(0, Qt.UserRole)
            mimeData.setText(text)
            return mimeData

        self.inputsTree.mimeData = _mimeDataInput

        self.inputsTree.setDragDropMode(QTreeWidget.DragOnly)
        self.inputsTree.setDropIndicatorShown(True)

        self.algorithms_model = ModelerToolboxModel(self, QgsApplication.processingRegistry())
        self.algorithmTree.setToolboxProxyModel(self.algorithms_model)
        self.algorithmTree.setDragDropMode(QTreeWidget.DragOnly)
        self.algorithmTree.setDropIndicatorShown(True)

        self.algorithmTree.setFilters(QgsProcessingToolboxProxyModel.FilterModeler)

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(QCoreApplication.translate('ModelerDialog', 'Search…'))
        if hasattr(self.textName, 'setPlaceholderText'):
            self.textName.setPlaceholderText(self.tr('Enter model name here'))
        if hasattr(self.textGroup, 'setPlaceholderText'):
            self.textGroup.setPlaceholderText(self.tr('Enter group name here'))

        # Connect signals and slots
        self.inputsTree.doubleClicked.connect(self.addInput)
        self.searchBox.textChanged.connect(self.algorithmTree.setFilterString)
        self.algorithmTree.doubleClicked.connect(self.addAlgorithm)

        # Ctrl+= should also trigger a zoom in action
        ctrlEquals = QShortcut(QKeySequence("Ctrl+="), self)
        ctrlEquals.activated.connect(self.zoomIn)

        self.mActionOpen.triggered.connect(self.openModel)
        self.mActionSave.triggered.connect(self.save)
        self.mActionSaveAs.triggered.connect(self.saveAs)
        self.mActionSaveInProject.triggered.connect(self.saveInProject)
        self.mActionZoomIn.triggered.connect(self.zoomIn)
        self.mActionZoomOut.triggered.connect(self.zoomOut)
        self.mActionZoomActual.triggered.connect(self.zoomActual)
        self.mActionZoomToItems.triggered.connect(self.zoomToItems)
        self.mActionExportImage.triggered.connect(self.exportAsImage)
        self.mActionExportPdf.triggered.connect(self.exportAsPdf)
        self.mActionExportSvg.triggered.connect(self.exportAsSvg)
        #self.mActionExportPython.triggered.connect(self.exportAsPython)
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

        self.view.centerOn(0, 0)
        self.help = None

        self.hasChanged = False

    def closeEvent(self, evt):
        settings = QgsSettings()
        settings.setValue("/Processing/stateModeler", self.saveState())
        settings.setValue("/Processing/geometryModeler", self.saveGeometry())

        if self.hasChanged:
            ret = QMessageBox.question(
                self, self.tr('Save Model?'),
                self.tr('There are unsaved changes in this model. Do you want to keep those?'),
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
            self.bar.pushMessage("", self.tr("Model doesn't contain any algorithm and/or parameter and can't be executed"), level=Qgis.Warning, duration=5)
            return

        dlg = AlgorithmDialog(self.model.create(), parent=iface.mainWindow())
        dlg.exec_()

    def save(self):
        self.saveModel(False)

    def saveAs(self):
        self.saveModel(True)

    def saveInProject(self):
        if not self.can_save():
            return

        self.model.setName(str(self.textName.text()))
        self.model.setGroup(str(self.textGroup.text()))
        self.model.setSourceFilePath(None)

        project_provider = QgsApplication.processingRegistry().providerById(PROJECT_PROVIDER_ID)
        project_provider.add_model(self.model)

        self.update_model.emit()
        self.bar.pushMessage("", self.tr("Model was saved inside current project"), level=Qgis.Success, duration=5)

        self.hasChanged = False
        QgsProject.instance().setDirty(True)

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

        self.bar.pushMessage("", self.tr("Successfully exported model as image to <a href=\"{}\">{}</a>").format(QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success, duration=5)
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

        self.bar.pushMessage("", self.tr("Successfully exported model as PDF to <a href=\"{}\">{}</a>").format(QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success, duration=5)
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

        self.bar.pushMessage("", self.tr("Successfully exported model as SVG to <a href=\"{}\">{}</a>").format(QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success, duration=5)
        self.repaintModel(controls=True)

    def exportAsPython(self):
        filename, filter = QFileDialog.getSaveFileName(self,
                                                       self.tr('Save Model As Python Script'), '',
                                                       self.tr('Processing scripts (*.py *.PY)'))
        if not filename:
            return

        if not filename.lower().endswith('.py'):
            filename += '.py'

        text = self.model.asPythonCode()
        with codecs.open(filename, 'w', encoding='utf-8') as fout:
            fout.write(text)

        self.bar.pushMessage("", self.tr("Successfully exported model as python script to <a href=\"{}\">{}</a>").format(QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success, duration=5)

    def can_save(self):
        """
        Tests whether a model can be saved, or if it is not yet valid
        :return: bool
        """
        if str(self.textName.text()).strip() == '':
            self.bar.pushWarning(
                "", self.tr('Please a enter model name before saving')
            )
            return False

        return True

    def saveModel(self, saveAs):
        if not self.can_save():
            return
        self.model.setName(str(self.textName.text()))
        self.model.setGroup(str(self.textGroup.text()))
        if self.model.sourceFilePath() and not saveAs:
            filename = self.model.sourceFilePath()
        else:
            filename, filter = QFileDialog.getSaveFileName(self,
                                                           self.tr('Save Model'),
                                                           ModelerUtils.modelsFolders()[0],
                                                           self.tr('Processing models (*.model3 *.MODEL3)'))
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
                    QMessageBox.warning(self, self.tr("Can't save model"), QCoreApplication.translate('QgsPluginInstallerInstallingDialog', (
                        "This model can't be saved in its original location (probably you do not "
                        "have permission to do it). Please, use the 'Save as…' option."))
                    )
                return
            self.update_model.emit()
            if saveAs:
                self.bar.pushMessage("", self.tr("Model was correctly saved to <a href=\"{}\">{}</a>").format(QUrl.fromLocalFile(filename).toString(), QDir.toNativeSeparators(filename)), level=Qgis.Success, duration=5)
            else:
                self.bar.pushMessage("", self.tr("Model was correctly saved"), level=Qgis.Success, duration=5)

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
                                     Qgis.Critical)
            QMessageBox.critical(self, self.tr('Open Model'),
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
        param = item.data(0, Qt.UserRole)
        self.addInputOfType(param)

    def addInputOfType(self, paramType, pos=None):
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
        sortedParams = sorted(QgsApplication.instance().processingRegistry().parameterTypes(), key=lambda pt: pt.name())
        for param in sortedParams:
            if param.flags() & QgsProcessingParameterType.ExposeToModeler:
                paramItem = QTreeWidgetItem()
                paramItem.setText(0, param.name())
                paramItem.setData(0, Qt.UserRole, param.id())
                paramItem.setIcon(0, icon)
                paramItem.setFlags(Qt.ItemIsEnabled | Qt.ItemIsSelectable | Qt.ItemIsDragEnabled)
                paramItem.setToolTip(0, param.description())
                parametersItem.addChild(paramItem)
        self.inputsTree.addTopLevelItem(parametersItem)
        parametersItem.setExpanded(True)

    def addAlgorithm(self):
        algorithm = self.algorithmTree.selectedAlgorithm()
        if algorithm is not None:
            alg = QgsApplication.processingRegistry().createAlgorithmById(algorithm.id())
            self._addAlgorithm(alg)

    def _addAlgorithm(self, alg, pos=None):
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
