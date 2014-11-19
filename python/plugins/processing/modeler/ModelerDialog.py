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
import json
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from processing.core.ProcessingConfig import ProcessingConfig
from processing.core.GeoAlgorithm import GeoAlgorithm
from processing.core.ProcessingLog import ProcessingLog
from processing.gui.HelpEditionDialog import HelpEditionDialog
from processing.gui.AlgorithmDialog import AlgorithmDialog
from processing.gui.AlgorithmClassification import AlgorithmDecorator
from processing.modeler.ModelerParameterDefinitionDialog import ModelerParameterDefinitionDialog
from processing.modeler.ModelerAlgorithm import ModelerAlgorithm, ModelerParameter
from processing.modeler.ModelerParametersDialog import ModelerParametersDialog
from processing.modeler.ModelerUtils import ModelerUtils
from processing.modeler.ModelerScene import ModelerScene
from processing.modeler.WrongModelException import WrongModelException
from processing.tools.system import *

from processing.ui.ui_DlgModeler import Ui_DlgModeler


class ModelerDialog(QDialog, Ui_DlgModeler):

    USE_CATEGORIES = '/Processing/UseSimplifiedInterface'
    CANVAS_SIZE = 4000

    def __init__(self, alg=None):
        QDialog.__init__(self)

        self.setupUi(self)

        self.zoom = 1

        self.setWindowFlags(Qt.WindowMinimizeButtonHint |
                            Qt.WindowMaximizeButtonHint |
                            Qt.WindowCloseButtonHint)

        self.tabWidget.setCurrentIndex(0)
        self.scene = ModelerScene(self)
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
                    alg = ModelerUtils.getAlgorithm(text);
                    if alg is not None:
                        self._addAlgorithm(alg.getCopy(), event.pos())
                event.accept()
            else:
                event.ignore()

        def _dragMoveEvent(event):
            if event.mimeData().hasText():
                event.accept()
            else:
                event.ignore()

        def _wheelEvent(event):
            self.view.setTransformationAnchor(QGraphicsView.AnchorUnderMouse);
            factor = 1.05
            if event.delta() > 0:
                factor = 1/factor
            self.view.scale(factor, factor)
            self.view.centerOn(event.pos().x(), event.pos().y())
            self.repaintModel()

        def _enterEvent(e):
            QGraphicsView.enterEvent(self.view, e)
            self.view.viewport().setCursor(Qt.ArrowCursor)
        def _mousePressEvent(e):
            QGraphicsView.mousePressEvent(self.view, e)
            self.view.viewport().setCursor(Qt.ArrowCursor)
        def _mouseReleaseEvent(e):
            QGraphicsView.mouseReleaseEvent(self.view, e)
            self.view.viewport().setCursor(Qt.ArrowCursor)

        self.view.setDragMode(QGraphicsView.ScrollHandDrag);
        self.view.dragEnterEvent = _dragEnterEvent
        self.view.dropEvent = _dropEvent
        self.view.dragMoveEvent = _dragMoveEvent
        self.view.wheelEvent = _wheelEvent
        self.view.enterEvent = _enterEvent
        self.view.mousePressEvent = _mousePressEvent
        self.view.mouseReleaseEvent = _mouseReleaseEvent


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
                mimeData.setText(item.alg.commandLineName())
            return mimeData

        self.algorithmTree.mimeData = _mimeDataAlgorithm

        self.algorithmTree.setDragDropMode(QTreeWidget.DragOnly)
        self.algorithmTree.setDropIndicatorShown(True)

        # Set icons
        self.btnOpen.setIcon(
                QgsApplication.getThemeIcon('/mActionFileOpen.svg'))
        self.btnSave.setIcon(
                QgsApplication.getThemeIcon('/mActionFileSave.svg'))
        self.btnSaveAs.setIcon(
                QgsApplication.getThemeIcon('/mActionFileSaveAs.svg'))
        self.btnExportImage.setIcon(
                QgsApplication.getThemeIcon('/mActionSaveMapAsImage.png'))
        self.btnEditHelp.setIcon(QIcon(':/processing/images/edithelp.png'))
        self.btnRun.setIcon(QIcon(':/processing/images/runalgorithm.png'))

        # Fill trees with inputs and algorithms
        self.fillInputsTree()
        self.fillAlgorithmTree()

        if hasattr(self.searchBox, 'setPlaceholderText'):
            self.searchBox.setPlaceholderText(self.tr('Search...'))
        if hasattr(self.textName, 'setPlaceholderText'):
            self.textName.setPlaceholderText(self.tr('[Enter model name here]'))
        if hasattr(self.textGroup, 'setPlaceholderText'):
            self.textGroup.setPlaceholderText(self.tr('[Enter group name here]'))

        # Connect signals and slots
        self.inputsTree.doubleClicked.connect(self.addInput)
        self.searchBox.textChanged.connect(self.fillAlgorithmTree)
        self.algorithmTree.doubleClicked.connect(self.addAlgorithm)

        self.btnOpen.clicked.connect(self.openModel)
        self.btnSave.clicked.connect(self.save)
        self.btnSaveAs.clicked.connect(self.saveAs)
        self.btnExportImage.clicked.connect(self.exportAsImage)
        self.btnEditHelp.clicked.connect(self.editHelp)
        self.btnRun.clicked.connect(self.runModel)

        if alg is not None:
            self.alg = alg
            self.textGroup.setText(alg.group)
            self.textName.setText(alg.name)
            self.repaintModel()

        else:
            self.alg = ModelerAlgorithm()
            self.alg.modelerdialog = self

        self.view.centerOn(0, 0)
        self.alg.setModelerView(self)
        self.help = None
        # Indicates whether to update or not the toolbox after
        # closing this dialog
        self.update = False

        self.hasChanged = False

    def closeEvent(self, evt):
        if self.hasChanged:
            ret = QMessageBox.question(self, self.tr('Unsaved changes'),
                    self.tr('There are unsaved changes in model. Continue?'),
                    QMessageBox.Yes | QMessageBox.No, QMessageBox.No)

            if ret == QMessageBox.Yes:
                evt.accept()
            else:
                evt.ignore()
        else:
            evt.accept()

    def editHelp(self):
        dlg = HelpEditionDialog(self.alg.getCopy())
        dlg.exec_()

    def runModel(self):
        if len(self.alg.algs) == 0:
            QMessageBox.warning(self, self.tr('Empty model'),
                    self.tr("Model doesn't contains any algorithms and/or "
                            "parameters and can't be executed"))
            return

        if self.alg.provider is None:
            # Might happen if model is opened from modeler dialog
            self.alg.provider = ModelerUtils.providers['model']
        alg = self.alg.getCopy()
        dlg = AlgorithmDialog(alg)
        dlg.exec_()

    def save(self):
        self.saveModel(False)

    def saveAs(self):
        self.saveModel(True)

    def exportAsImage(self):
        filename = unicode(QFileDialog.getSaveFileName(self,
                           self.tr('Save Model As Image'), '',
                           self.tr('PNG files (*.png *.PNG)')))
        if not filename:
            return

        if not filename.lower().endswith('.png'):
            filename += '.png'

        totalRect = QRectF(0, 0, 1, 1)
        for item in self.scene.items():
            totalRect = totalRect.united(item.sceneBoundingRect())
        totalRect.adjust(-10, -10, 10, 10)

        img = QImage(totalRect.width(), totalRect.height(),
                     QImage.Format_ARGB32_Premultiplied)
        img.fill(Qt.white)
        painter = QPainter()
        painter.setRenderHint(QPainter.Antialiasing)
        painter.begin(img)
        self.scene.render(painter, totalRect, totalRect)
        painter.end()

        img.save(filename)

    def saveModel(self, saveAs):
        if unicode(self.textGroup.text()).strip() == '' \
                or unicode(self.textName.text()).strip() == '':
            QMessageBox.warning(self, self.tr('Warning'),
                    self.tr('Please enter group and model names before saving'
                    ))
            return
        self.alg.name = unicode(self.textName.text())
        self.alg.group = unicode(self.textGroup.text())
        if self.alg.descriptionFile is not None and not saveAs:
            filename = self.alg.descriptionFile
        else:
            filename = unicode(QFileDialog.getSaveFileName(self,
                               self.tr('Save Model'),
                               ModelerUtils.modelsFolder(),
                               self.tr('Processing models (*.model)')))
            if filename:
                if not filename.endswith('.model'):
                    filename += '.model'
                self.alg.descriptionFile = filename
        if filename:
            text = self.alg.toJson()
            try:
                fout = codecs.open(filename, 'w', encoding='utf-8')
            except:
                if saveAs:
                    QMessageBox.warning(self, self.tr('I/O error'),
                            self.tr('Unable to save edits. Reason:\n %s')
                            % unicode(sys.exc_info()[1]))
                else:
                    QMessageBox.warning(self, self.tr("Can't save model"),
                            self.tr("This model can't be saved in its "
                                    "original location (probably you do not "
                                    "have permission to do it). Please, use "
                                    "the 'Save as...' option."))
                return
            fout.write(text)
            fout.close()
            self.update = True
            QMessageBox.information(self, self.tr('Model saved'),
                                    self.tr('Model was correctly saved.'))

            self.hasChanged = False

    def openModel(self):
        filename = unicode(QFileDialog.getOpenFileName(self,
                           self.tr('Open Model'), ModelerUtils.modelsFolder(),
                           self.tr('Processing models (*.model *.MODEL)')))
        if filename:
            try:
                alg = ModelerAlgorithm.fromFile(filename)
                self.alg = alg
                self.alg.setModelerView(self)
                self.textGroup.setText(alg.group)
                self.textName.setText(alg.name)
                self.repaintModel()

                self.view.centerOn(0, 0)
                self.hasChanged = False
            except WrongModelException, e:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                    self.tr('Could not load model %s\n%s') % (filename, e.msg))
                QMessageBox.critical(self, self.tr('Could not open model'),
                        self.tr('The selected model could not be loaded.\n'
                                 'See the log for more information.'))
            except Exception, e:
                ProcessingLog.addToLog(ProcessingLog.LOG_ERROR,
                    self.tr('Could not load model %s\n%s') % (filename, e.args[0]))
                QMessageBox.critical(self, self.tr('Could not open model'),
                    self.tr('The selected model could not be loaded.\n'
                            'See the log for more information.'))

    def repaintModel(self):
        self.scene = ModelerScene()
        self.scene.setSceneRect(QRectF(0, 0, ModelerAlgorithm.CANVAS_SIZE,
                                ModelerAlgorithm.CANVAS_SIZE))
        self.scene.paintModel(self.alg)
        self.view.setScene(self.scene)


    def addInput(self):
        item = self.inputsTree.currentItem()
        paramType = str(item.text(0))
        self.addInputOfType(paramType)

    def addInputOfType(self, paramType, pos=None):
        if paramType in ModelerParameterDefinitionDialog.paramTypes:
            dlg = ModelerParameterDefinitionDialog(self.alg, paramType)
            dlg.exec_()
            if dlg.param is not None:
                if pos is None:
                    pos = self.getPositionForParameterItem()
                if isinstance(pos, QPoint):
                    pos =  QPointF(pos)
                self.alg.addParameter(ModelerParameter(dlg.param, pos))
                self.repaintModel()
                #self.view.ensureVisible(self.scene.getLastParameterItem())
                self.hasChanged = True

    def getPositionForParameterItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if self.alg.inputs:
            maxX = max([i.pos.x() for i in self.alg.inputs.values()])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
        else:
            newX = MARGIN + BOX_WIDTH / 2
        return QPointF(newX, MARGIN + BOX_HEIGHT / 2)

    def fillInputsTree(self):
        icon = QIcon(os.path.dirname(__file__) + '/../images/input.png')
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
            alg = ModelerUtils.getAlgorithm(item.alg.commandLineName())
            self._addAlgorithm(alg.getCopy())

    def _addAlgorithm(self, alg, pos=None):
            dlg = alg.getCustomModelerParametersDialog(self.alg)
            if not dlg:
                dlg = ModelerParametersDialog(alg, self.alg)
            dlg.exec_()
            if dlg.alg is not None:
                if pos is None:
                    dlg.alg.pos = self.getPositionForAlgorithmItem()
                else:
                    dlg.alg.pos = pos
                if isinstance(dlg.alg.pos, QPoint):
                    dlg.alg.pos =  QPointF(pos)
                from processing.modeler.ModelerGraphicItem import ModelerGraphicItem
                for i, out in enumerate(dlg.alg.outputs):
                    dlg.alg.outputs[out].pos = dlg.alg.pos + QPointF(ModelerGraphicItem.BOX_WIDTH, (i + 1.5)
                            * ModelerGraphicItem.BOX_HEIGHT)
                self.alg.addAlgorithm(dlg.alg)
                self.repaintModel()
                #self.view.ensureVisible(self.scene.getLastAlgorithmItem())
                self.hasChanged = True

    def getPositionForAlgorithmItem(self):
        MARGIN = 20
        BOX_WIDTH = 200
        BOX_HEIGHT = 80
        if self.alg.algs:
            maxX = max([alg.pos.x() for alg in self.alg.algs.values()])
            maxY = max([alg.pos.y() for alg in self.alg.algs.values()])
            newX = min(MARGIN + BOX_WIDTH + maxX, self.CANVAS_SIZE - BOX_WIDTH)
            newY = min(MARGIN + BOX_HEIGHT + maxY, self.CANVAS_SIZE
                       - BOX_HEIGHT)
        else:
            newX = MARGIN + BOX_WIDTH / 2
            newY = MARGIN * 2 + BOX_HEIGHT + BOX_HEIGHT / 2
        return QPointF(newX, newY)


    def fillAlgorithmTree(self):
        settings = QSettings()
        useCategories = settings.value(self.USE_CATEGORIES, type=bool)
        if useCategories:
            self.fillAlgorithmTreeUsingCategories()
        else:
            self.fillAlgorithmTreeUsingProviders()

        self.algorithmTree.sortItems(0, Qt.AscendingOrder)

        text = unicode(self.searchBox.text())
        if text != '':
            self.algorithmTree.expandAll()

    def fillAlgorithmTreeUsingCategories(self):
        providersToExclude = ['model', 'script']
        self.algorithmTree.clear()
        text = unicode(self.searchBox.text())
        groups = {}
        allAlgs = ModelerUtils.allAlgs
        for providerName in allAlgs.keys():
            provider = allAlgs[providerName]
            name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
            if not ProcessingConfig.getSetting(name):
                continue
            if providerName in providersToExclude \
                    or len(ModelerUtils.providers[providerName].actions) != 0:
                continue
            algs = provider.values()

            # Add algorithms
            for alg in algs:
                if not alg.showInModeler or alg.allowOnlyOpenedLayers:
                    continue
                (altgroup, altsubgroup, altname) = \
                    AlgorithmDecorator.getGroupsAndName(alg)
                if altgroup is None:
                    continue
                if text == '' or text.lower() in altname.lower():
                    if altgroup not in groups:
                        groups[altgroup] = {}
                    group = groups[altgroup]
                    if altsubgroup not in group:
                        groups[altgroup][altsubgroup] = []
                    subgroup = groups[altgroup][altsubgroup]
                    subgroup.append(alg)

        if len(groups) > 0:
            mainItem = QTreeWidgetItem()
            mainItem.setText(0, self.tr('Geoalgorithms'))
            mainItem.setIcon(0, GeoAlgorithm.getDefaultIcon())
            mainItem.setToolTip(0, mainItem.text(0))
            for (groupname, group) in groups.items():
                groupItem = QTreeWidgetItem()
                groupItem.setText(0, groupname)
                groupItem.setIcon(0, GeoAlgorithm.getDefaultIcon())
                groupItem.setToolTip(0, groupItem.text(0))
                mainItem.addChild(groupItem)
                for (subgroupname, subgroup) in group.items():
                    subgroupItem = QTreeWidgetItem()
                    subgroupItem.setText(0, subgroupname)
                    subgroupItem.setIcon(0, GeoAlgorithm.getDefaultIcon())
                    subgroupItem.setToolTip(0, subgroupItem.text(0))
                    groupItem.addChild(subgroupItem)
                    for alg in subgroup:
                        algItem = TreeAlgorithmItem(alg)
                        subgroupItem.addChild(algItem)
            self.algorithmTree.addTopLevelItem(mainItem)

        for providerName in allAlgs.keys():
            groups = {}
            provider = allAlgs[providerName]
            name = 'ACTIVATE_' + providerName.upper().replace(' ', '_')
            if not ProcessingConfig.getSetting(name):
                continue
            if providerName not in providersToExclude:
                continue
            algs = provider.values()

            # Add algorithms
            for alg in algs:
                if not alg.showInModeler or alg.allowOnlyOpenedLayers:
                    continue
                if text == '' or text.lower() in alg.name.lower():
                    if alg.group in groups:
                        groupItem = groups[alg.group]
                    else:
                        groupItem = QTreeWidgetItem()
                        groupItem.setText(0, alg.group)
                        groupItem.setToolTip(0, alg.group)
                        groups[alg.group] = groupItem
                    algItem = TreeAlgorithmItem(alg)
                    groupItem.addChild(algItem)

            if len(groups) > 0:
                providerItem = QTreeWidgetItem()
                providerItem.setText(0,
                        ModelerUtils.providers[providerName].getDescription())
                providerItem.setIcon(0,
                        ModelerUtils.providers[providerName].getIcon())
                providerItem.setToolTip(0, providerItem.text(0))
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)
                providerItem.setExpanded(text != '')
                for groupItem in groups.values():
                    if text != '':
                        groupItem.setExpanded(True)

    def fillAlgorithmTreeUsingProviders(self):
        self.algorithmTree.clear()
        text = unicode(self.searchBox.text())
        allAlgs = ModelerUtils.allAlgs
        for providerName in allAlgs.keys():
            groups = {}
            provider = allAlgs[providerName]
            algs = provider.values()

            # Add algorithms
            for alg in algs:
                if not alg.showInModeler or alg.allowOnlyOpenedLayers:
                    continue
                if text == '' or text.lower() in alg.name.lower():
                    if alg.group in groups:
                        groupItem = groups[alg.group]
                    else:
                        groupItem = QTreeWidgetItem()
                        groupItem.setText(0, alg.group)
                        groupItem.setToolTip(0, alg.group)
                        groups[alg.group] = groupItem
                    algItem = TreeAlgorithmItem(alg)
                    groupItem.addChild(algItem)

            if len(groups) > 0:
                providerItem = QTreeWidgetItem()
                providerItem.setText(0,
                        ModelerUtils.providers[providerName].getDescription())
                providerItem.setToolTip(0,
                        ModelerUtils.providers[providerName].getDescription())
                providerItem.setIcon(0,
                        ModelerUtils.providers[providerName].getIcon())
                for groupItem in groups.values():
                    providerItem.addChild(groupItem)
                self.algorithmTree.addTopLevelItem(providerItem)
                providerItem.setExpanded(text != '')
                for groupItem in groups.values():
                    if text != '':
                        groupItem.setExpanded(True)

        self.algorithmTree.sortItems(0, Qt.AscendingOrder)


class TreeAlgorithmItem(QTreeWidgetItem):

    def __init__(self, alg):
        settings = QSettings()
        useCategories = settings.value(ModelerDialog.USE_CATEGORIES, type=bool)
        QTreeWidgetItem.__init__(self)
        self.alg = alg
        icon = alg.getIcon()
        name = alg.name
        if useCategories:
            icon = GeoAlgorithm.getDefaultIcon()
            (group, subgroup, name) = AlgorithmDecorator.getGroupsAndName(alg)
        self.setIcon(0, icon)
        self.setToolTip(0, name)
        self.setText(0, name)
