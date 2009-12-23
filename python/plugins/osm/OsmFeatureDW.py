# -*- coding: utf-8 -*-99
"""@package OsmFeatureDW
This module is descendant of "OSM Feature" dockable widget and makes user able
to view and edit information on selected OSM feature.

OsmFeatureDW module shows details of selected feature - its basic info, tags and relations.
It provides methods for editing features' tags so that user can edit them directly on the widget.

There are also some identify and edit buttons on "OsmFeatureDW" - this modul implements all the methods that are called
after clicking on these buttons. Such methods creates (and set) map tool that coresponds to specified button.
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_OsmFeatureDW import Ui_OsmFeatureDW
from OsmAddRelationDlg import OsmAddRelationDlg
import OsmTags

# include all available osm map tools
from map_tools.OsmCreatePointMT import OsmCreatePointMT
from map_tools.OsmCreateLineMT import OsmCreateLineMT
from map_tools.OsmCreatePolygonMT import OsmCreatePolygonMT
from map_tools.OsmMoveMT import OsmMoveMT
from map_tools.OsmIdentifyMT import OsmIdentifyMT
import unicodedata


class OsmFeatureDW(QDockWidget, Ui_OsmFeatureDW,  object):
    """This class shows details of selected feature - its basic info, tags and relations.

    It provides methods for editing features' tags so that user can edit them directly on the widget.

    There are also some identify and edit buttons on "OsmFeatureDW" - this modul implements all the methods that are called
    after clicking them. Such methods creates (and set) map tool that coresponds to specified button.
    """


    def __init__(self, plugin):
        """The constructor."""

        QDockWidget.__init__(self, None)
        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self.plugin=plugin
        self.mapTool=None
        self.__dlgAddRel=None

        # set icons for tool buttons (identify,move,createPoint,createLine,createPolygon)
        self.identifyButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_identify.png"))
        self.moveButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_move.png"))
        self.createPointButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createPoint.png"))
        self.createLineButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createLine.png"))
        self.createPolygonButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createPolygon.png"))
        self.createRelationButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_createRelation.png"))
        self.removeButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_removeFeat.png"))
        self.deleteTagsButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_removeTag.png"))
        self.undoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_undo.png"))
        self.redoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_redo.png"))
        self.addRelationButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_addRelation.png"))
        self.removeRelationButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_removeRelation.png"))
        self.editRelationButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_editRelation.png"))
        self.urDetailsButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_urDetails.png"))

        # initializing group of edit buttons
        self.toolButtons=QButtonGroup(self)
        self.dummyButton.setVisible(False)
        self.toolButtons.addButton(self.dummyButton)
        self.toolButtons.addButton(self.identifyButton)
        self.toolButtons.addButton(self.moveButton)
        self.toolButtons.addButton(self.createPointButton)
        self.toolButtons.addButton(self.createLineButton)
        self.toolButtons.addButton(self.createPolygonButton)
        self.toolButtons.setExclusive(True)

        # initializing table of feature tags
        self.tagTable.setColumnCount(2)
        self.tagTable.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.tagTable.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))
        self.tagTable.setSelectionMode(QAbstractItemView.ExtendedSelection)
        self.tagTable.setSelectionBehavior(QAbstractItemView.SelectRows)
        self.newTagLabel="<new tag here>"
        self.relTagsTree.setSelectionMode(QAbstractItemView.NoSelection)

        # initializing rubberbands/vertexmarkers; getting qgis settings of line width and color for rubberbands
        settings=QSettings()
        qgsLineWidth=2 # use fixed width
        qgsLineRed=settings.value( "/qgis/digitizing/line_color_red", QVariant(255) ).toInt()
        qgsLineGreen=settings.value( "/qgis/digitizing/line_color_green", QVariant(0) ).toInt()
        qgsLineBlue=settings.value( "/qgis/digitizing/line_color_blue", QVariant(0) ).toInt()

        self.rubBandPol=QgsRubberBand(plugin.canvas,True)
        self.rubBandPol.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        self.rubBandPol.setWidth(qgsLineWidth)

        self.rubBand=QgsRubberBand(plugin.canvas,False)
        self.rubBand.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        self.rubBand.setWidth(qgsLineWidth)

        self.verMarker=QgsVertexMarker(plugin.canvas)
        self.verMarker.setIconType(2)
        self.verMarker.setIconSize(13)
        self.verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        self.verMarker.setPenWidth(qgsLineWidth)
        self.verMarkers=[]

        self.relRubBandPol=QgsRubberBand(plugin.canvas,True)
        self.relRubBandPol.setColor(QColor(qgsLineRed[0],50,50))
        self.relRubBandPol.setWidth(qgsLineWidth+4)

        self.relRubBand=QgsRubberBand(plugin.canvas,False)
        self.relRubBand.setColor(QColor(qgsLineRed[0],50,50))
        self.relRubBand.setWidth(qgsLineWidth+4)

        self.relVerMarker=QgsVertexMarker(plugin.canvas)
        self.relVerMarker.setIconType(2)
        self.relVerMarker.setIconSize(13)
        self.relVerMarker.setColor(QColor(qgsLineRed[0],50,50))
        self.relVerMarker.setPenWidth(qgsLineWidth)

        # initializing inner variables
        self.activeEditButton=self.dummyButton
        self.__tagsLoaded=False
        self.__relTagsLoaded=False
        self.feature=None
        self.featureType=None
        self.featRels=[]
        self.featRelTags=[]
        self.featRelMembers=[]

        # clear all widget items
        self.clear()

        self.__connectWidgetSignals()

        self.removeButton.setEnabled(False)
        self.createRelationButton.setCheckable(False)

        # set current tab to "Properties"
        self.propRelBox.setCurrentIndex(0)
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)

        # init coordinate transform
        self.projectionChanged()

        renderer=self.plugin.canvas.mapRenderer()
        self.connect(renderer, SIGNAL("hasCrsTransformEnabled(bool)"), self.projectionChanged)
        self.connect(renderer, SIGNAL("destinationSrsChanged()"), self.projectionChanged)


    def setContentEnabled(self,flag):

        self.featInfoBox.setEnabled(flag)
        self.propRelBox.setEnabled(flag)
        self.identifyButton.setEnabled(flag)
        self.moveButton.setEnabled(flag)
        self.createPointButton.setEnabled(flag)
        self.createLineButton.setEnabled(flag)
        self.createPolygonButton.setEnabled(flag)
        self.createRelationButton.setEnabled(flag)

        if flag:
            if self.plugin.undoredo.undoCounter>0:
                self.undoButton.setEnabled(True)
            if self.plugin.undoredo.redoCounter>0:
                self.redoButton.setEnabled(True)
        else:
            self.undoButton.setEnabled(False)
            self.redoButton.setEnabled(False)

        self.urDetailsButton.setEnabled(flag)


    def projectionChanged(self):
        """Function is connected to signals from QgsMapRenderer.
        It updates coordinate transforms.
        """

        renderer = self.plugin.canvas.mapRenderer()
        if renderer.hasCrsTransformEnabled():
          self.coordXform = QgsCoordinateTransform(renderer.destinationSrs(), QgsCoordinateReferenceSystem(4326))
        else:
          self.coordXform = None


    def canvasToOsmCoords(self, point):
        """Performs conversion from canvas to map coordinates.

        @param point canvas coordinates to convert
        """
        point = self.plugin.canvas.getCoordinateTransform().toMapCoordinates( point )

        # optional conversion from map to layer coordinates
        if self.coordXform is not None:
            point = self.coordXform.transform( point )

        return point


    def __connectWidgetSignals(self):
        """Function connects all necessary signals to appropriate slots.
        """

        # signals emitted on clicking with tag and member tables
        QObject.connect(self.identifyButton,  SIGNAL("clicked()"), self.__startIdentifyingFeature)
        QObject.connect(self.moveButton,  SIGNAL("clicked()"), self.__startMovingFeature)
        QObject.connect(self.createPointButton,  SIGNAL("clicked()"), self.__startPointCreation)
        QObject.connect(self.createLineButton,  SIGNAL("clicked()"), self.__startLineCreation)
        QObject.connect(self.createPolygonButton,  SIGNAL("clicked()"), self.__startPolygonCreation)
        QObject.connect(self.createRelationButton,  SIGNAL("clicked()"), self.__createRelation)
        QObject.connect(self.removeButton, SIGNAL("clicked()"), self.removeFeature)
        QObject.connect(self.relListWidget, SIGNAL("currentRowChanged(int)"), self.loadRelationStuff)
        QObject.connect(self.relMembersList, SIGNAL("currentRowChanged(int)"), self.__showRelMemberOnMap)
        QObject.connect(self.addRelationButton,  SIGNAL("clicked()"), self.createRelationWithMember)
        QObject.connect(self.editRelationButton,  SIGNAL("clicked()"), self.editSelectedRelation)
        QObject.connect(self.removeRelationButton,  SIGNAL("clicked()"), self.removeSelectedRelation)
        QObject.connect(self.deleteTagsButton, SIGNAL("clicked()"), self.removeSelectedTags)
        QObject.connect(self.tagTable, SIGNAL("cellChanged(int,int)"), self.__onTagsCellChanged)
        QObject.connect(self.tagTable, SIGNAL("currentCellChanged(int,int,int,int)"), self.__onCurrentCellChanged)
        QObject.connect(self.tagTable, SIGNAL("itemDoubleClicked(QTableWidgetItem*)"), self.__onTagsItemDoubleClicked)
        QObject.connect(self.undoButton,  SIGNAL("clicked()"), self.__undo)
        QObject.connect(self.redoButton, SIGNAL("clicked()"), self.__redo)
        QObject.connect(self.urDetailsButton, SIGNAL("clicked()"), self.__urDetailsChecked)


    def databaseChanged(self,dbKey):
        """This function is called when current OSM database of plugin changes.
        The OsmFeatureDW performs necessary actions and tells current map tool about the change.

        @param dbKey key (name) of new current database
        """

        if self.__dlgAddRel and dbKey:

            self.__dlgAddRel.close()
            self.__dlgAddRel=None
            self.setContentEnabled(True)
            self.plugin.undoredo.setContentEnabled(True)

            QMessageBox.information(self, self.tr("OSM Plugin")
                ,self.tr("The 'Create OSM Relation' dialog was closed automatically because current OSM database was changed."))

        # clear the whole OSM Feature dockwidget as well as all related rubberbands and vertex markers
        self.clear()

        # if some mapTool is currently set tell it about database changing
        if self.mapTool:
            self.mapTool.databaseChanged(dbKey)
            self.activeEditButton=self.dummyButton

        # and if new database is None, disable the whole dockwidget
        if not dbKey:
            self.setContentEnabled(False)
            if self.mapTool:
                self.plugin.canvas.unsetMapTool(self.mapTool)
                self.mapTool=None
                self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
            return

        self.setContentEnabled(True)


    def clear(self):
        """Function clears all widget items.
        It resets rubberbands, vertexmarkers, re-initializes OsmFeatureDW inner structures.
        """

        # clear common feature infos
        self.typeIdLabel.setText("")
        self.userLabel.setText("")
        self.createdLabel.setText("")

        # clear table with information about feature's tags
        self.tagTable.clear()
        self.tagTable.setEnabled(False)
        self.tagTable.setRowCount(0)
        self.tagTable.setColumnCount(0)

        # clear table with info about feature's relations
        self.relListWidget.clear()
        self.relTagsTree.clear()
        self.relMembersList.clear()
        self.relTagsTree.setColumnCount(0)

        self.relListWidget.setEnabled(False)
        self.relTagsTree.setEnabled(False)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.deleteTagsButton.setEnabled(False)
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)
        self.addRelationButton.setEnabled(False)
        self.removeButton.setEnabled(False)

        # remove previous rubber bands
        self.rubBand.reset(False)
        self.rubBandPol.reset(True)
        self.verMarker.setCenter(QgsPoint(-1000,-1000))
        self.verMarker.hide()
        self.__removeMemberMarkers()
        self.verMarkers=[]

        self.relRubBand.reset(False)
        self.relRubBandPol.reset(True)
        self.relVerMarker.setCenter(QgsPoint(-1000,-1000))
        self.relVerMarker.hide()

        # clear member variables
        self.__tagsLoaded=False
        self.__relTagsLoaded=False
        self.feature=None
        self.featureType=None
        self.featRels=[]
        self.featRelTags=[]
        self.featRelMembers=[]


    def __createRelation(self):
        """Function calls relation creating process.
        Creation is started by displaying appropriate dialog.
        """

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.plugin.canvas.unsetMapTool(self.mapTool)
        del self.mapTool
        self.mapTool=None

        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.dummyButton

        self.setContentEnabled(False)
        self.plugin.undoredo.setContentEnabled(False)
        self.plugin.toolBar.setEnabled(False)

        # OsmAddRelationDlg parameters: plugin, newRelationFirstMember, relationToEdit
        self.__dlgAddRel=OsmAddRelationDlg(self.plugin, None, None)
        self.__dlgAddRel.setWindowModality(Qt.WindowModal)
        self.__dlgAddRel.exec_()
        self.__dlgAddRel=None

        self.setContentEnabled(True)
        self.plugin.undoredo.setContentEnabled(True)
        self.plugin.toolBar.setEnabled(True)


    def createRelationWithMember(self):
        """Function calls relation creating process. Creation is started by displaying appropriate dialog.
        Function pre-fills dialog with information on currently loaded feature.
        """

        self.__dlgAddRel=OsmAddRelationDlg(self.plugin, QString(self.featureType+" %1").arg(self.feature.id()), None)

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.plugin.canvas.unsetMapTool(self.mapTool)
        del self.mapTool
        self.mapTool=None

        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.dummyButton

        self.setContentEnabled(False)
        self.plugin.undoredo.setContentEnabled(False)
        self.plugin.toolBar.setEnabled(False)

        self.__dlgAddRel.setWindowModality(Qt.WindowModal)
        self.__dlgAddRel.exec_()
        self.__dlgAddRel=None

        self.setContentEnabled(True)
        self.plugin.undoredo.setContentEnabled(True)
        self.plugin.toolBar.setEnabled(True)


    def editSelectedRelation(self):
        """Function calls editing of a relation. Editing is started by displaying appropriate dialog.
        Relation identifier is not passed to this function. Function has to find it out from current row
        of appropriate list widget.
        """

        # show modal dialog "Edit relation"
        if not self.feature:
            QMessageBox.information(self, self.tr("OSM Feature Dock Widget"), self.tr("Choose OSM feature first."))
            return

        item=self.relListWidget.item(self.relListWidget.currentRow())
        if not item:
            QMessageBox.information(self, self.tr("OSM Feature Dock Widget"), self.tr("Choose relation for editing first."))
            return

        relId=self.featRels[self.relListWidget.currentRow()]

        self.setContentEnabled(False)
        self.plugin.undoredo.setContentEnabled(False)
        self.plugin.toolBar.setEnabled(False)

        self.__dlgAddRel=OsmAddRelationDlg(self.plugin, None, relId)
        self.__dlgAddRel.setWindowModality(Qt.WindowModal)
        self.__dlgAddRel.exec_()
        self.__dlgAddRel=None

        self.setContentEnabled(True)
        self.plugin.undoredo.setContentEnabled(True)
        self.plugin.toolBar.setEnabled(True)


    def __onTagsCellChanged(self,row,column):
        """Function is called after cellChanged(int,int) signal is emitted on table of all features' relations.
        It means that user is changed key or value of some existing tag.

        @param row index of row in table of tags
        @param column index of column in table of tags
        """

        if not self.__tagsLoaded:
            # this signal was emitted during table initialization,
            # but we are interested in user actions only
            return

        if row<self.tagTable.rowCount()-1:

            # changing value of tag that already exists
            key=self.tagTable.item(row,0).text()
            value=self.tagTable.item(row,1).text()

            # store tag's change into database
            self.plugin.undoredo.startAction("Change tag value.")
            self.plugin.dbm.changeTagValue(self.feature.id(),self.featureType,key.toAscii().data(),value.toUtf8())
        else:
            key = self.tagTable.item(row,0).text()
            if key=="" or key==self.newTagLabel:
                return

            # adding new tag and setting its key
            if column==0:

                # only ascii keys are allowed
                nkfd_form=unicodedata.normalize('NFKD', unicode(key.toUtf8(),'utf-8'))
                key_only_ascii=nkfd_form.encode('ASCII', 'ignore')

                # store it into database
                isAlreadyDef=self.plugin.dbm.isTagDefined(self.feature.id(),self.featureType,key_only_ascii)
                if isAlreadyDef:
                    # such a key already exists for this relation
                    self.tagTable.setItem(row,0,QTableWidgetItem(self.newTagLabel))
                    QMessageBox.information(self, self.tr("OSM Feature Dock Widget")
                        ,self.tr(QString("Property '").append(key_only_ascii).append("' cannot be added twice.")))
                    return

                # well, insert new tag into database
                self.plugin.undoredo.startAction("Insert new tag.")
                self.plugin.dbm.insertTag(self.feature.id(),self.featureType,key_only_ascii,None)

                self.__tagsLoaded=False

                self.tagTable.item(row,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
                self.tagTable.item(row,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.tagTable.item(row,0).setText(QString(key_only_ascii))

                newLastRow = row+1
                self.tagTable.setRowCount(row+2)
                self.tagTable.setItem(newLastRow,0,QTableWidgetItem(self.newTagLabel))
                self.tagTable.setItem(newLastRow,1,QTableWidgetItem(""))
                self.tagTable.item(newLastRow,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
                self.tagTable.item(newLastRow,1).setFlags(Qt.ItemIsEnabled)

                self.__tagsLoaded=True

        # updating feature status (from Normal to Updated)
        if self.featureType=='Point':
            self.plugin.dbm.changePointStatus(self.feature.id(),'N','U')

        elif self.featureType=='Line':
            self.plugin.dbm.changeLineStatus(self.feature.id(),'N','U')

        elif self.featureType=='Polygon':
            self.plugin.dbm.changePolygonStatus(self.feature.id(),'N','U')

        elif self.featureType=='Relation':
            self.plugin.dbm.changeRelationStatus(self.feature.id(),'N','U')

        self.plugin.dbm.commit()

        affected=[(self.feature.id(),self.featureType)]
        self.plugin.undoredo.stopAction(affected)
        self.plugin.dbm.recacheAffectedNow(affected)

        # refresh map canvas so that changes take effect
        if column==1:
            self.plugin.canvas.refresh()


    def __onTagsItemDoubleClicked(self,item):
        """Function is called after itemDoubleClicked(...) signal is emitted on table of feature tags.

        It shows combobox with possible values for given item of table.

        @param item item of table of feature tags
        """

        if item.column()==0:

            if item.row()<self.tagTable.rowCount()-1:
                return

            tagValues=OsmTags.suitableTagKeys(self.featureType)
            tagValues.sort()
            if len(tagValues)>0:
                valCombo=QComboBox()
                valCombo.setEditable(True)
                valCombo.addItems(tagValues)
                currentComboText=self.tagTable.item(item.row(),0).text()
                ix=valCombo.findText(currentComboText)
                valCombo.setCurrentIndex(ix)
                if ix==-1:
                    valCombo.setEditText(currentComboText)

                self.tagTable.setCellWidget(item.row(),0,valCombo)
                QObject.connect(valCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onTagKeySelectionChanged)
            return

        key=str(self.tagTable.item(item.row(),0).text())
        tagValues=OsmTags.suitableTagValues(self.featureType,key)
        tagValues.sort()

        if len(tagValues)>0:
            valCombo=QComboBox()
            valCombo.setEditable(True)
            valCombo.addItems(tagValues)
            currentComboText=self.tagTable.item(item.row(),1).text()
            ix=valCombo.findText(currentComboText)
            valCombo.setCurrentIndex(ix)
            if ix==-1:
                valCombo.setEditText(currentComboText)

            self.tagTable.setCellWidget(item.row(),1,valCombo)
            QObject.connect(valCombo, SIGNAL("currentIndexChanged(const QString &)"), self.__onTagValueSelectionChanged)


    def __onCurrentCellChanged(self,curRow,curCol,prevRow,prevCol):
        """Function is called after currentCellChanged(...) signal is emitted on table of feature tags.

        @param curRow current row index to tags table
        @param curCol current column index to tags table
        @param prevRow previous row index to tags table
        @param prevCol previous column index to tags table
        """

        cellWidget=self.tagTable.cellWidget(prevRow,prevCol)
        if cellWidget==None:
            return

        self.tagTable.removeCellWidget(prevRow,prevCol)


    def __onTagKeySelectionChanged(self,key):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox in 1st column of tags table.

        @param key key selected in combobox
        """

        row=self.tagTable.currentRow()
        col=self.tagTable.currentColumn()

        self.tagTable.item(row,col).setText(key)
        self.tagTable.removeCellWidget(row,col)


    def __onTagValueSelectionChanged(self,value):
        """Function is called after currentIndexChanged(...) signal is emitted on combobox in 2nd column of tags table.

        @param value value selected in combobox
        """

        row=self.tagTable.currentRow()
        col=self.tagTable.currentColumn()

        self.tagTable.item(row,col).setText(value)
        self.tagTable.removeCellWidget(row,col)



    def __startIdentifyingFeature(self):
        """Function prepares feature identification.
        The appropriate map tool (OsmIdentifyMT) is set to map canvas.
        """

        if self.activeEditButton==self.identifyButton:
            return

        self.plugin.canvas.unsetMapTool(self.plugin.canvas.mapTool())

        # clear dockwidget
        self.clear()

        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.mapTool=OsmIdentifyMT(self.plugin.canvas, self, self.plugin.dbm)
        self.plugin.canvas.setMapTool(self.mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.identifyButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startMovingFeature(self):
        """Function prepares feature moving.
        The appropriate map tool (OsmMoveMT) is set to map canvas.
        """

        if self.activeEditButton==self.moveButton:
            return

        # clear dockwidget
        self.clear()
        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.mapTool=OsmMoveMT(self.plugin)
        self.plugin.canvas.setMapTool(self.mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.CrossCursor))
        self.activeEditButton=self.moveButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startPointCreation(self):
        """Function prepares point creating operation.
        The appropriate map tool (OsmCreatePointMT) is set to map canvas.
        """

        if self.activeEditButton==self.createPointButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.mapTool=OsmCreatePointMT(self.plugin)
        self.plugin.canvas.setMapTool(self.mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.createPointButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startLineCreation(self):
        """Function prepares line creating operation.
        The appropriate map tool (OsmCreateLineMT) is set to map canvas.
        """

        if self.activeEditButton==self.createLineButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.mapTool=OsmCreateLineMT(self.plugin)
        self.plugin.canvas.setMapTool(self.mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.createLineButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def __startPolygonCreation(self):
        """Function prepares polygon creating operation.
        The appropriate map tool (OsmCreatePolygonMT) is set to map canvas.
        """

        if self.activeEditButton==self.createPolygonButton:
            return

        self.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")

        self.mapTool=OsmCreatePolygonMT(self.plugin)
        self.plugin.canvas.setMapTool(self.mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.activeEditButton=self.createPolygonButton
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)


    def removeFeature(self):
        """Function completely removes feature that is currently loaded on "OSM Feature" widget.
        """

        self.removeButton.setDown(False)
        self.removeButton.setChecked(False)
        self.plugin.iface.mainWindow().statusBar().showMessage("")

        # remove object that was identified by "identify tool"
        featId=self.feature.id()
        featType=self.featureType
        affected=[]

        if featType=='Point':
            self.plugin.undoredo.startAction("Remove point.")
            affected=self.plugin.dbm.removePoint(featId)

        elif featType=='Line':
            self.plugin.undoredo.startAction("Remove line.")
            affected=self.plugin.dbm.removeLine(featId,True)    # todo: False when Ctrl pressed

        elif featType=='Polygon':
            self.plugin.undoredo.startAction("Remove polygon.")
            affected=self.plugin.dbm.removePolygon(featId,True)    # todo: False when Ctrl pressed

        elif featType=='Relation':
            self.plugin.undoredo.startAction("Remove relation.")
            self.plugin.dbm.removeRelation(featId)
        else:
            return    # strange situation

        self.plugin.undoredo.stopAction(affected)
        self.plugin.dbm.recacheAffectedNow(affected)

        # refresh map canvas so that changes take effect
        self.plugin.canvas.refresh()

        # clear dockwidget
        self.clear()

        self.plugin.iface.mainWindow().statusBar().showMessage("")

        self.plugin.canvas.unsetMapTool(self.mapTool)
        del self.mapTool
        self.mapTool=OsmIdentifyMT(self.plugin.canvas, self, self.plugin.dbm)
        self.plugin.canvas.setMapTool(self.mapTool)
        self.plugin.canvas.setCursor(QCursor(Qt.ArrowCursor))
        self.plugin.canvas.setFocus(Qt.OtherFocusReason)
        self.activeEditButton=self.identifyButton
        self.activeEditButton.setChecked(True)


    def removeSelectedTags(self):
        """Function completely removes all tags that are currently selected in the appropriate
        list of the "OSM Feature" widget. More than one tag can be selected using Ctrl and clicking.
        """

        # remove selected tags (rows)
        selectedItems=self.tagTable.selectedItems()
        selectedRowsIndexes=[]
        lastRowIndex=self.tagTable.rowCount()-1
        self.tagTable.setCurrentCell(lastRowIndex,0)

        for i in selectedItems:
            if i.column()==0 and not i.row()==lastRowIndex:
                selectedRowsIndexes.append(i.row())

        self.plugin.undoredo.startAction("Remove tags.")

        selectedRowsIndexes.sort()
        selectedRowsIndexes.reverse()

        for ix in selectedRowsIndexes:

            key=self.tagTable.item(ix,0).text()
            # updating feature status (from Normal to Updated)
            if self.featureType=='Point':
                self.plugin.dbm.changePointStatus(self.feature.id(),'N','U')

            elif self.featureType=='Line':
                self.plugin.dbm.changeLineStatus(self.feature.id(),'N','U')

            elif self.featureType=='Polygon':
                self.plugin.dbm.changePolygonStatus(self.feature.id(),'N','U')

            elif self.featureType=='Relation':
                self.plugin.dbm.changeRelationStatus(self.feature.id(),'N','U')

            # perform tag removing 
            self.plugin.dbm.removeTag(self.feature.id(),self.featureType,key.toAscii().data())

            self.tagTable.removeRow(ix)

        # make this action permanent
        self.plugin.dbm.commit()
        affected=[(self.feature.id(),self.featureType)]
        self.plugin.undoredo.stopAction(affected)
        self.plugin.dbm.recacheAffectedNow(affected)

        # refresh map canvas so that changes take effect
        self.plugin.canvas.refresh()


    def removeSelectedRelation(self):
        """Function completely removes all relations that are selected in the appropriate list of the "OSM Feature" widget.
        More than one relation can be selected using Ctrl and clicking.
        """

        # find out id of relation which is selected
        item=self.relListWidget.item(self.relListWidget.currentRow())
        if not item or not self.plugin.dbm.currentKey:
            return

        relId=self.featRels[self.relListWidget.currentRow()]

        self.plugin.undoredo.startAction("Remove relation.")
        self.plugin.dbm.removeRelation(relId)
        self.plugin.dbm.commit()
        self.plugin.undoredo.stopAction()

        # reload list of feature's relations
        self.__loadFeatureRelations(self.feature.id(),self.featureType)


    def __showRelMemberOnMap(self,ixRow):
        """Functions marks relation member on the map.

        Marking is realized with simple rubberBand (in case of line/polygon) or vertexMarker (for point).
        Relation member is given by its index to the list of all members of currently loaded relation.

        @param ixRow index to the list of all relations on mentioned widget
        """

        # move rubberband to selected feature
        self.relRubBand.reset(False)
        self.relRubBandPol.reset(True)
        self.relVerMarker.hide()

        if ixRow==-1 or not self.plugin.dbm.currentKey:
            return
        mem=self.featRelMembers[ixRow]
        self.showFeatureOnMap(mem[0],mem[1])    # id & type


    def loadRelationStuff(self,ixRow):
        """Functions loads information on specified relation into "OSM Feature" widget.

        Relation is given by its index to the list of all relations on mentioned widget.
        Relation "stuff" means its basic info, tags and members.

        @param ixRow index to the list of all relations on mentioned widget
        """

        if ixRow==-1 or not self.plugin.dbm.currentKey:
            return
        relId=self.featRels[ixRow]

        # show all tags connected to selected relation
        self.__loadRelationTags(relId)

        # show all relation members on osm dock widget
        self.__loadRelationMembers(relId)

        # enable list of members and buttons for relation removing and editing
        self.relMembersList.setEnabled(True)
        self.editRelationButton.setEnabled(True)
        self.removeRelationButton.setEnabled(True)


    def __loadRelationMembers(self,relId):
        """Functions loads the list of members of specified relation.
        Relation is given by its identifier.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        Resulting list contains info on member's identifier, type and its role in specified relation.

        @param relId identifier of relation
        """

        self.relMembersList.clear()
        # ask database manager for all relation members
        self.featRelMembers=self.plugin.dbm.getRelationMembers(relId)

        # printing members
        for i in range(0,len(self.featRelMembers)):
            memId=self.featRelMembers[i][0]
            memType=self.featRelMembers[i][1]
            memRole=self.featRelMembers[i][2]

            listRow=QString("(%1) - %2").arg(memId).arg(memType)
            if memRole and memRole<>"":
                listRow=listRow.append(QString(", role:%3").arg(memRole))

            self.relMembersList.addItem(listRow)


    def __loadRelationTags(self,relId):
        """Functions loads the list of tags of specified relation.
        Relation is given by its identifier.

        Loading is realized into the appropriate QTreeWidget of "OSM Feature" dockable widget.
        Resulting tags table has two columns "Key","Value" for easy representing of tag pairs.

        @param relId identifier of relation
        """

        self.relTagsTree.clear()
        self.__relTagsLoaded=False
        # ask database manager for all relation tags
        self.featRelTags=self.plugin.dbm.getFeatureTags(relId,"Relation")

        self.relTagsTree.setColumnCount(2)
        self.relTagsTree.setHeaderLabels(["Key","Value"])

        for i in range(0,len(self.featRelTags)):
            self.relTagsTree.addTopLevelItem(QTreeWidgetItem([self.featRelTags[i][0],self.featRelTags[i][1]]))

        self.__relTagsLoaded=True


    def __loadFeatureInformation(self,featId,featType):
        """Functions shows up the basic information on feature.
        Feature is given by its identifier and its type.

        Info is loaded to appropriate place of the "OSM Feature" widget.
        Basic info consists (mainly) of feature's identifier, type, owner and timestamp.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        # asking OsmDatabaseManager for missing information
        featUser=self.plugin.dbm.getFeatureOwner(featId,featType)
        featCreated=self.plugin.dbm.getFeatureCreated(featId,featType)    # returned as string
            # timestamp example: "2008-02-18T15:34:14Z"

        self.typeIdLabel.setText("")
        self.userLabel.setText("")
        self.createdLabel.setText("")

        # put non-tags feature information on dock widget
        if featType:
            self.typeIdLabel.setText(QString("%1  %2").arg(featType).arg(str(featId)))
        if featUser:
            self.userLabel.setText(QString("%1").arg(featUser))
        if featCreated:
            # format timestamp
            DT_format_osm=Qt.ISODate
                # "yyyy-MM-ddThh:mm:ssZ"
            DT_format_plugin="yy/MM/dd - h:mm"
            self.createdLabel.setText(QDateTime.fromString(featCreated,DT_format_osm).toString(DT_format_plugin))


    def __loadFeatureTags(self,featId,featType):
        """Functions loads the list of tags of specified feature.
        Feature is given by its identifier and its type.

        Loading is realized into the appropriate QTableWidget of "OSM Feature" dockable widget.
        Resulting tags table has two columns "Key","Value" for easy representing of tag pairs.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        # clear table with information about feature's tags
        self.tagTable.clear()

        # fill tableWidget with tags of selected feature
        tableData=self.plugin.dbm.getFeatureTags(featId,featType)
        rowCount=len(tableData)
        self.__tagsLoaded=False

        self.tagTable.setRowCount(rowCount+1)
        self.tagTable.setColumnCount(2)
        self.tagTable.setHorizontalHeaderItem(0,QTableWidgetItem("Key"))
        self.tagTable.setHorizontalHeaderItem(1,QTableWidgetItem("Value"))

        for i in range(0,rowCount):
            self.tagTable.setItem(i,0,QTableWidgetItem(QString.fromUtf8(tableData[i][0])))
            self.tagTable.setItem(i,1,QTableWidgetItem(QString.fromUtf8(tableData[i][1])))
            self.tagTable.item(i,0).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled)
            self.tagTable.item(i,1).setFlags(Qt.ItemIsSelectable | Qt.ItemIsEnabled | Qt.ItemIsEditable)

        self.tagTable.setItem(rowCount,0,QTableWidgetItem(self.newTagLabel))
        self.tagTable.setItem(rowCount,1,QTableWidgetItem(""))
        self.tagTable.item(rowCount,0).setFlags(Qt.ItemIsEnabled | Qt.ItemIsEditable)
        self.tagTable.item(rowCount,1).setFlags(Qt.ItemIsEnabled)
        self.__tagsLoaded=True

        # enable tags table for editing
        self.tagTable.setEnabled(True)
        self.deleteTagsButton.setEnabled(True)


    def __loadFeatureRelations(self,featId,featType):
        """Functions loads the list of relations of specified feature.
        Feature is given by its identifier and its type.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        If no relation exists for specified feature, listWidget is filled with the only row with text: "<no relation>".

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        self.relTagsTree.setColumnCount(0)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)

        # clear all tables connected to relations
        self.relListWidget.clear()
        self.relTagsTree.clear()
        self.relMembersList.clear()

        # load relations for selected feature
        self.featRels=self.plugin.dbm.getFeatureRelations(featId,featType)

        for i in range(0,len(self.featRels)):
            self.relListWidget.addItem(self.__getRelationInfo(self.featRels[i]))

        if len(self.featRels)==0:
            self.relListWidget.addItem("<no relation>")
            self.addRelationButton.setEnabled(True)
            return

        # enable relation tables and button for relation addition
        self.addRelationButton.setEnabled(True)
        self.relListWidget.setEnabled(True)
        self.relTagsTree.setEnabled(True)


    def reloadFeatureRelations(self):
        """Functions reloads the list of relations for currently loaded feature.

        Loading is realized into the appropriate QListWidget of "OSM Feature" dockable widget.
        If no relation exists for specified feature, listWidget is filled with the only row with text: "<no relation>".
        """

        self.relTagsTree.setColumnCount(0)
        self.relMembersList.setEnabled(False)

        # disable widget buttons
        self.editRelationButton.setEnabled(False)
        self.removeRelationButton.setEnabled(False)

        # clear all tables connected to relations
        self.relListWidget.clear()
        self.relTagsTree.clear()
        self.relMembersList.clear()

        # load relations for selected feature
        self.featRels=self.plugin.dbm.getFeatureRelations(self.feature.id(),self.featureType)

        for i in range(0,len(self.featRels)):
            self.relListWidget.addItem(self.__getRelationInfo(self.featRels[i]))

        if len(self.featRels)==0:
            self.relListWidget.addItem("<no relation>")
            self.addRelationButton.setEnabled(True)
            return

        # enable relation tables and button for relation addition
        self.addRelationButton.setEnabled(True)
        self.relListWidget.setEnabled(True)
        self.relTagsTree.setEnabled(True)


    def putMarkersOnMembers(self,feat,featType):
        """Function adds additional vertexMarkers to the map.

        Additional vertexMarker are used to provide better marking of line/polygon.
        In that case line/polygon geometry is marked with rubberband first, and second one vertexMarker is put
        on each its vertex. Such extended marking of line/polygon is used (for example) when calling loadFeature() method with
        "markingMode" parameter set to 2.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        if featType=='Point':
            return

        pline=None
        if featType=='Line':
            pline=feat.geometry().asPolyline()

        elif featType=='Polygon':
            pline=feat.geometry().asPolygon()[0]

        # get qgis settings of line width and color for rubberband
        settings=QSettings()
        qgsLineWidth=2 # use fixed width
        qgsLineRed=settings.value("/qgis/digitizing/line_color_red",QVariant(255)).toInt()
        qgsLineGreen=settings.value("/qgis/digitizing/line_color_green",QVariant(0)).toInt()
        qgsLineBlue=settings.value("/qgis/digitizing/line_color_blue",QVariant(0)).toInt()

        for i in range(0,len(pline)):
            verMarker=QgsVertexMarker(self.plugin.canvas)
            verMarker.setIconType(3)
            verMarker.setIconSize(6)
            verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
            verMarker.setPenWidth(qgsLineWidth)
            verMarker.setCenter(pline[i])
            verMarker.show()
            self.verMarkers.append(verMarker)


    def __removeMemberMarkers(self):
        """Function removes additional vertexMarkers from the map.

        Additional vertexMarker are used to provide better marking of line/polygon.
        In that case line/polygon geometry is marked with rubberband first, and second one vertexMarker is put
        on each its vertex. Such extended marking of line/polygon is used (for example) when calling loadFeature() method with
        "markingMode" parameter set to 2.
        """

        for verMarker in self.verMarkers:
            self.plugin.canvas.scene().removeItem(verMarker)
            del verMarker
        self.verMarkers=[]


    def showFeatureOnMap(self,featId,featType):
        """Function just shows up specified feature on the map canvas.

        Showing feature up is realized by putting rubberBand (vertexMarker) on its whole geometry.
        This rubberBand (vM) is the same as when using loadFeature() method with "markingMode" parameter set to 1.

        Feature is given by its identifier and its type. Feature data are not loaded into "OSM Feature" widget.
        Rubberbands (vertexMarkers) of other features are not removed from the map canvas before this action.

        @param featId identifier of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        """

        # we have to know feature's geometry to be able to display it on map
        featGeom=self.plugin.dbm.getFeatureGeometry(featId,featType)
        if not featGeom:
           return    # nothing to show :-/

        if featType=='Polygon':
            self.relRubBandPol.setToGeometry(featGeom,self.plugin.canvas.currentLayer())
        elif featType=='Point':
            self.relVerMarker.setCenter(featGeom.asPoint())
            self.relVerMarker.show()
        elif featType=='Line':
            self.relRubBand.setToGeometry(featGeom,self.plugin.canvas.currentLayer())


    def loadFeature(self,feat,featType,markingMode=1):
        """Function loads information on specified feature into "OSM Feature" widget elements.
        It shows up features identifier, type, osm user, timestamp, all its tags and related OSM relations.

        According to the value of parameter "markingMode" is marks feature on the map canvas. If "markingMode" equals to zero
        feature is not marked on the map. If it equals to 1, simple rubberband (or vertexMarker) is put on the feature (this is
        default behavior). If "markingMode" equals to 2, extended rubberband is shown on the map, especially for non-point features.

        @param feat QgsFeature object of feature to load
        @param featType type of feature to load - one of 'Point','Line','Polygon'
        @param markingMode not compulsory; defines quality of feature's rubberband on map
        """

        if not feat or not featType:
            self.clear()
            return

        # remember which feature is loaded
        self.featureType=featType
        self.feature=feat

        # move rubberband to selected feature
        self.rubBandPol.reset(True)
        self.rubBand.reset(False)
        self.verMarker.hide()
        self.__removeMemberMarkers()

        if markingMode>0:
            if self.featureType=='Polygon':
                self.rubBandPol.setToGeometry(feat.geometry(),self.plugin.canvas.currentLayer())
                if markingMode>1:
                    self.putMarkersOnMembers(feat,featType)
            elif self.featureType=='Point':
                self.verMarker.setCenter(feat.geometry().asPoint())
                self.verMarker.show()
            elif self.featureType=='Line':
                self.rubBand.setToGeometry(feat.geometry(),self.plugin.canvas.currentLayer())
                if markingMode>1:
                    self.putMarkersOnMembers(feat,featType)

        # show common feature information (id,feature originator,created,feature type,...)
        self.__loadFeatureInformation(feat.id(),featType)

        # show all tags connected to feature onto osm widget dialog
        self.__loadFeatureTags(feat.id(),featType)

        # show all relations connected to feature onto osm widget dialog
        self.__loadFeatureRelations(feat.id(),featType)

        # feature has been loaded; enable "remove feature" button
        self.removeButton.setEnabled(True)


    def __getRelationInfo(self,relId):
        """Function returns brief info on specified relation. Information consists of relation identifier,
        its type and other important relation properties and it is returned simply in QString() object.

        Information are good to be shown in some list (of relations).

        @param relId identifier of relation
        @return brief info on relation concatenated in string
        """

        relInfo=relType=relRoute=relBoundary=relRef=relRestr=""
        tags=self.plugin.dbm.getFeatureTags(relId,"Relation")

        for i in range(0,len(tags)):
            key=tags[i][0]
            value=tags[i][1]

            if key=="type":
                relType=value        # type: route / boundary, ...
            elif key=="route":
                relRoute=value       # route: road / bicycle / foot / hiking / bus / railway / tram, ...
            elif key=="boundary":
                relBoundary=value    # boundary: administrative, ...
            elif key=="ref":
                relRef=value
            elif key=="restriction":
                relRestr=value

        if relType=="route":
            relInfo = QString("(%1) "+relRoute+" "+relType+" "+relRef).arg(relId)

        elif relType=="boundary":
            relInfo = QString("(%1) "+relBoundary+" "+relType).arg(relId)

        elif relType=="restriction":
            relInfo = QString("(%1) "+relRestr+" "+relType).arg(relId)

        elif relType=="":
            relInfo = QString("(%1) relation of unknown type").arg(relId)

        else:
            relInfo = QString("(%1) "+relType).arg(relId)

        return relInfo


    def __undo(self):
        """Function performs exactly one undo operation.
        """

        self.plugin.undoredo.undo()


    def __redo(self):
        """Function performs exactly one redo operation.
        """

        self.plugin.undoredo.redo()


    def __urDetailsChecked(self):
        """Function is called after clicking on urDetailsButton checkbox.
        It shows or hides OSM Edit History widget.
        """

        if self.urDetailsButton.isChecked():
            self.plugin.undoredo.show()
            self.urDetailsButton.setToolTip("Hide OSM Edit History")
        else:
            self.plugin.undoredo.hide()
            self.urDetailsButton.setToolTip("Show OSM Edit History")


