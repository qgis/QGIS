"""@package OsmUndoRedoDW
This module holds evidence of user edit actions.

Such evidence exists for each loaded OSM data.

Module provides easy way how to call undo/redo actions.
"""

from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

from ui_OsmUndoRedoDW import Ui_OsmUndoRedoDW
from OsmDatabaseManager import OsmDatabaseManager

import sqlite3
from math import *
from time import *



class OsmUndoRedoDW(QDockWidget, Ui_OsmUndoRedoDW, object):
    """This class extends functionality of Ui_OsmUndoRedoDW dialog which displays history of user edit actions.
    Such history exists for each loaded OSM data.

    This class provides easy way how to call undo/redo actions.
    """

    def __init__(self, plugin):
        """The constructor.

        Does basic initialization, connecting dialog signals to appropriate slots
        and setting icons to its buttons.

        @param plugin pointer to OSM Plugin instance
        """

        QDockWidget.__init__(self, None)

        self.setupUi(self)
        self.setAllowedAreas(Qt.LeftDockWidgetArea | Qt.RightDockWidgetArea)

        self.undoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_undo.png"))
        self.redoButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_redo.png"))
        self.clearButton.setIcon(QIcon(":/plugins/osm_plugin/images/osm_remove.png"))

        self.canvas=plugin.canvas
        self.iface=plugin.iface
        self.dbm=plugin.dbm
        self.plugin=plugin

        self.actionList.setDragDropMode(QAbstractItemView.NoDragDrop)

        QObject.connect(self.undoButton,SIGNAL("clicked()"),self.undo)
        QObject.connect(self.redoButton,SIGNAL("clicked()"),self.redo)
        QObject.connect(self.clearButton,SIGNAL("clicked()"),self.clear)
        QObject.connect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)

        # structures for evidence of user edit changes (required by undo and redo operations)
        self.mapActions={}
        self.mapIxAction={}
        self.currentMapKey=None

        self.actStartId=self.actStopId=self.actNote=None
        self.actionInProgress=False

        self.redoCounter=0
        self.undoCounter=0
        self.affected=set()
        self.urIsBusy=False

        self.actionList.addItem(QString("<empty>"))
        self.actionList.setCurrentRow(0)


    def currRowChanged(self,row):
        """This function is called after currentRowChanged(...) signal is emmited on dialog's list of edit actions.
        Functions calls as many undo actions (as many redo actions) as needed to jump in editing history to selected row.

        @param row row number from the list of edit actions
        """

        if row<0:
            return
        self.goToAction(row)


    def setContentEnabled(self,flag):

        if flag:
            if self.undoCounter>0:
                self.undoButton.setEnabled(True)
            if self.redoCounter>0:
                self.redoButton.setEnabled(True)
        else:
            self.undoButton.setEnabled(False)
            self.redoButton.setEnabled(False)

        self.clearButton.setEnabled(flag)
        self.actionList.setEnabled(flag)


    def clear(self):
        """Function clears (re-initializes) the whole undo/redo dialog.
        """

        if self.dbm and self.dbm.currentKey:
            self.dbm.removeAllChangeSteps()

        self.actionList.clear()
        self.actStartId=self.actStopId=self.actNote=None
        self.actionInProgress=False
        self.urIsBusy=False

        self.actionList.addItem(QString("<empty>"))
        self.actionList.setCurrentRow(0)

        self.redoCounter=0
        self.redoButton.setEnabled(False)
        self.undoCounter=0
        self.undoButton.setEnabled(False)

        if self.plugin:
            if self.plugin.dockWidget:
                self.plugin.dockWidget.redoButton.setEnabled(False)
                self.plugin.dockWidget.undoButton.setEnabled(False)

        if self.currentMapKey:
            self.mapActions[self.currentMapKey]=[]
            self.mapIxAction[self.currentMapKey]=-1


    def databaseChanged(self,dbKey):
        """Functions is called when current database of OSM Plugin changed.

        OSM Undo/Redo module clears its list of actions and loads the one for new current database.
        If dbKey parameter is None, there is no current database. In this case function just clears the list
        and reinitialize its inner structures.

        @param dbKey key/name of new current database file
        """

        # clear the list widget
        self.actionList.clear()

        # clear inner structures
        self.actStartId=self.actStopId=self.actNote=None
        self.actionInProgress=False
        self.currentMapKey=dbKey
        self.urIsBusy=False

        if not dbKey:
            self.setContentEnabled(False)
            return

        self.setContentEnabled(True)
        if dbKey in self.mapActions.keys():

            # load the list widget
            self.redoCounter=0
            self.redoButton.setEnabled(False)
            self.plugin.dockWidget.redoButton.setEnabled(False)
            self.undoCounter=0
            self.undoButton.setEnabled(False)
            self.plugin.dockWidget.undoButton.setEnabled(False)

            self.actionList.addItem(QString("<empty>"))

            for action in self.mapActions[self.currentMapKey]:
                self.actionList.addItem(action[2])    # 2 ~ actionNote!

            ixAction=self.mapIxAction[self.currentMapKey]
            self.undoCounter=ixAction+1
            self.redoCounter=len(self.mapActions[self.currentMapKey])-self.undoCounter
            self.actionList.setCurrentRow(ixAction+1)

            if self.undoCounter>0:
                self.undoButton.setEnabled(True)
                self.plugin.dockWidget.undoButton.setEnabled(True)
            if self.redoCounter>0:
                self.redoButton.setEnabled(True)
                self.plugin.dockWidget.redoButton.setEnabled(True)
            return

        # new dbKey has no undo/redo history yet
        self.actionList.addItem(QString("<empty>"))
        self.actionList.setCurrentRow(0)

        self.redoCounter=0
        self.redoButton.setEnabled(False)
        self.plugin.dockWidget.redoButton.setEnabled(False)
        self.undoCounter=0
        self.undoButton.setEnabled(False)
        self.plugin.dockWidget.undoButton.setEnabled(False)

        self.mapActions[dbKey]=[]
        self.mapIxAction[dbKey]=-1


    def goToAction(self,row):
        """Functions goes to the selected row in history of edit actions.
        It calls as many undo/redo operations, as needed.

        @param row row index to list of edit actions history
        """
        curr=self.actionList.currentRow()
        self.actionList.setEnabled(False)

        if not self.currentMapKey in self.mapIxAction:
            return

        ixGoto=row                   # ix of row which was clicked
        ixCurrent=self.mapIxAction[self.currentMapKey]+1    # current action index

        # how many undo/redo actions are necessary?
        howFar=0
        self.affected=set()

        if ixCurrent<ixGoto:
            # redo actions; we move "whitespace item" behind the row which was clicked
            howFar=ixGoto-ixCurrent
            for ix in range(0,howFar):
                self.redo(False)
        elif ixCurrent>ixGoto:
            # undo actions; we move "whitespace item" before the row which was clicked
            howFar=ixCurrent-ixGoto
            for ix in range(0,howFar):
                self.undo(False)
        else:
            self.actionList.setEnabled(True)
            QObject.disconnect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)
            self.actionList.setCurrentRow(curr)
            QObject.connect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)
            return

        self.dbm.commit()
        self.dbm.recacheAffectedNow(self.affected)
        self.affected=set()
        self.canvas.refresh()

        if self.plugin.dockWidget:
            lFeat=self.plugin.dockWidget.feature
            lFeatType=self.plugin.dockWidget.featureType
            self.plugin.dockWidget.loadFeature(lFeat,lFeatType)

        self.actionList.setEnabled(True)
        QObject.disconnect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)
        self.actionList.setCurrentRow(curr)
        QObject.connect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)


    def startAction(self,actNote):
        """Function remembers current state of system.

        This function is called before performing an action that should be put into editing history.
        It's expected that you call stopAction() function after edit actions finishes.
        Then new record in history will be created.

        @param actNote action description (in brief)
        """

        if not self.dbm.currentKey:
            # there is no current database
            return

        if self.actionInProgress:
            print "failed! change in progress!"
            return

        self.actionInProgress=True
        self.actStartId=self.dbm.getCurrentActionNumber()
        self.actStopId=None
        self.actNote=actNote


    def stopAction(self,affected=set()):
        """Function is called after an edit action. It stores current state of system
        and the state from last calling of startAction(). This two states of system
        are considered new edit history record together and are stored in database.

        @param affected list of all OSM features that was affected with just finished action"""

        if not self.dbm.currentKey:
            # there is no current database
            return

        if affected==None:
            affected=set()

        self.actStopId=self.dbm.getCurrentActionNumber()
        if self.actStopId<=self.actStartId:
            self.actionInProgress=False
            return

        for ix in range(len(self.mapActions[self.currentMapKey]),self.actionList.currentRow(),-1):
            self.actionList.takeItem(ix)
            self.redoCounter=self.redoCounter-1

        ixAction=self.mapIxAction[self.currentMapKey]
        cntActions=len(self.mapActions[self.currentMapKey])
        if ixAction>-1:
            fromId=self.mapActions[self.currentMapKey][ixAction][1]+1
            self.dbm.removeChangeStepsBetween(fromId,self.actStartId)

        del self.mapActions[self.currentMapKey][ixAction+1:cntActions]

        if self.redoCounter==0:
            self.redoButton.setEnabled(False)
            self.plugin.dockWidget.redoButton.setEnabled(False)

        self.mapIxAction[self.currentMapKey]=ixAction+1
        ixAction=ixAction+1

        # increase undo counter
        self.undoCounter=self.undoCounter+1
        self.undoButton.setEnabled(True)
        self.plugin.dockWidget.undoButton.setEnabled(True)

        self.mapActions[self.currentMapKey].append((self.actStartId,self.actStopId,self.actNote,affected))
        self.actionList.addItem(self.actNote)
        self.actionList.setCurrentRow(ixAction+1)

        self.actStartId=self.actStopId=self.actNote=None
        self.actionInProgress=False


    def undo(self,standAlone=True):
        """Functions performs exactly one undo operation in system.

        Last edit action is reverted, list of editing history on undo/redo dialog
        shifts its current row up.

        @param refresh if False, no canvas refresh will be performed after reverting an action; default is True
        """

        self.undoButton.setEnabled(False)
        self.plugin.dockWidget.undoButton.setEnabled(False)
        self.redoButton.setEnabled(False)
        self.plugin.dockWidget.redoButton.setEnabled(False)

        (startId,stopId,note,affected)=self.mapActions[self.currentMapKey][self.mapIxAction[self.currentMapKey]]

        # shift up in the list widget
        ixCurrent=self.actionList.currentRow()
        QObject.disconnect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)
        self.actionList.setCurrentRow(ixCurrent-1)
        QObject.connect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)

        self.mapIxAction[self.currentMapKey]=self.mapIxAction[self.currentMapKey]-1

        changeSteps=self.dbm.getChangeSteps(startId,stopId)

        for (change_type,tab_name,row_id,col_name,old_value,new_value) in changeSteps:

            if change_type=='I':
                self.dbm.setRowDeleted(tab_name,row_id)
            elif change_type=='D':
                self.dbm.setRowNotDeleted(tab_name,row_id)
            elif change_type=='U':
                self.dbm.setRowColumnValue(tab_name,col_name,old_value,row_id)

        # increase redo counter
        self.redoCounter=self.redoCounter+1
        # decrease undo counter
        self.undoCounter=self.undoCounter-1

        # refresh
        if standAlone:
            self.dbm.commit()
            self.dbm.recacheAffectedNow(affected)
            self.canvas.refresh()
            self.plugin.dockWidget.loadFeature(self.plugin.dockWidget.feature,self.plugin.dockWidget.featureType)
        else:
            self.affected.update(affected)

        self.redoButton.setEnabled(True)
        self.plugin.dockWidget.redoButton.setEnabled(True)
        if self.undoCounter>0:
            self.undoButton.setEnabled(True)
            self.plugin.dockWidget.undoButton.setEnabled(True)


    def redo(self,standAlone=True):
        """Functions performs exactly one redo operation in system.

        Last reverted edit action is redone again. List of editing history on undo/redo dialog
        shifts its current row down.

        @param refresh if False, no canvas refresh will be performed after redo action; default is True
        """

        self.undoButton.setEnabled(False)
        self.plugin.dockWidget.undoButton.setEnabled(False)
        self.redoButton.setEnabled(False)
        self.plugin.dockWidget.redoButton.setEnabled(False)

        (startId,stopId,note,affected)=self.mapActions[self.currentMapKey][self.mapIxAction[self.currentMapKey]+1]

        # shift down in the list widget
        ixCurrent=self.actionList.currentRow()
        QObject.disconnect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)
        self.actionList.setCurrentRow(ixCurrent+1)
        QObject.connect(self.actionList,SIGNAL("currentRowChanged(int)"),self.currRowChanged)

        self.mapIxAction[self.currentMapKey]=self.mapIxAction[self.currentMapKey]+1
        if self.mapIxAction[self.currentMapKey]==len(self.mapActions[self.currentMapKey]):
            self.redoButton.setEnabled(False)
            self.plugin.dockWidget.redoButton.setEnabled(False)

        changeSteps=self.dbm.getChangeSteps(startId,stopId)
        for (change_type,tab_name,row_id,col_name,old_value,new_value) in changeSteps:

            if change_type=='I':
                self.dbm.setRowNotDeleted(tab_name,row_id)
            elif change_type=='D':
                self.dbm.setRowDeleted(tab_name,row_id)
            elif change_type=='U':
                self.dbm.setRowColumnValue(tab_name,col_name,new_value,row_id)

        # decrease redo counter
        self.redoCounter=self.redoCounter-1
        # increase undo counter
        self.undoCounter=self.undoCounter+1

        # refresh
        if standAlone:
            self.dbm.commit()
            self.dbm.recacheAffectedNow(affected)
            self.canvas.refresh()
            self.plugin.dockWidget.loadFeature(self.plugin.dockWidget.feature,self.plugin.dockWidget.featureType)
        else:
            self.affected.update(affected)

        self.undoButton.setEnabled(True)
        self.plugin.dockWidget.undoButton.setEnabled(True)
        if self.redoCounter>0:
            self.redoButton.setEnabled(True)
            self.plugin.dockWidget.redoButton.setEnabled(True)


