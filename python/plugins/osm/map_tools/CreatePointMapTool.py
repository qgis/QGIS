"""@package CreatePointMapTool
This module holds all structures and methods required to perform
"create point" operation on current OSM data.

Snapping to existing segments of lines/polygons is supported when creating new point.

Process generates vertexMarkers so that user can watch results
of the operation on the map in a nice way.

There is also an interaction with plugin's "OSM Feature" dialog.
New points are loaded to it dynamically.
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *

import sqlite3
from math import *



class CreatePointMapTool(QgsMapTool):
    """This class holds all structures and methods required to perform
    "create point" operation on current OSM data.

    Snapping to existing segments of lines/polygons is supported when creating new point.

    Process generates vertexMarkers so that user can watch results
    of the operation on the map in a nice way.

    There is also an interaction with plugin's "OSM Feature" dialog.
    New points are loaded to it dynamically.
    """


    def __init__(self,plugin):
        """The constructor.

        Initializes the map tool, creates necessary snappers.

        @param plugin pointer to OSM Plugin instance
        """

        QgsMapTool.__init__(self,plugin.canvas)
        self.canvas=plugin.canvas
        self.dockWidget=plugin.dockWidget
        self.dbm=plugin.dbm
        self.ur=plugin.undoredo
        self.snappingEnabled=True
        self.snapFeat=None
        self.snapFeatType=None

        # creating vertex marker
        self.verMarker=self.createVertexMarker()

        # creating snapper to this map tool
        self.snapper=self.createSnapper(self.canvas.mapRenderer())


    def databaseChanged(self,dbKey):
        """This function is called automatically when current OSM database has changed.

        Function does re-initialization of maptool and create new snapper again (if necessary).

        @param dbKey key of database with new current OSM data
        """

        # re-initialization
        self.snappingEnabled=True
        self.snapFeat=None
        self.snapFeatType=None
        self.verMarker.setCenter(QgsPoint(-1000,-1000))
        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")

        if self.verMarker:
            self.canvas.scene().removeItem(self.verMarker)
            del self.verMarker
            self.verMarker=None

        if not dbKey:
            return

        self.verMarker=self.createVertexMarker()
        del self.snapper
        self.snapper=self.createSnapper(self.canvas.mapRenderer())


    def createVertexMarker(self):
        """Function creates vertexMarker that is used for marking new point on map.

        @return vertex marker - QgsVertexMarker object
        """

        # get qgis settings
        settings=QSettings()
        qgsLineWidth=settings.value("/qgis/digitizing/line_width",QVariant(10)).toInt()
        qgsLineRed=settings.value("/qgis/digitizing/line_color_red",QVariant(255)).toInt()
        qgsLineGreen=settings.value("/qgis/digitizing/line_color_green",QVariant(0)).toInt()
        qgsLineBlue=settings.value("/qgis/digitizing/line_color_blue",QVariant(0)).toInt()

        verMarker=QgsVertexMarker(self.canvas)
        verMarker.setIconType(2)
        verMarker.setIconSize(13)
        verMarker.setColor(QColor(qgsLineRed[0],qgsLineGreen[0],qgsLineBlue[0]))
        verMarker.setPenWidth(qgsLineWidth[0])
        verMarker.setCenter(QgsPoint(-1000,-1000))

        return verMarker


    def createSnapper(self,canvasRenderer):
        """Function creates snapper that snaps within standard qgis tolerance.

        Snapping is done to all segments of both line and polygon layer.

        @param canvasRenderer renderer of current map canvas
        @return instance of segment QgsSnapper
        """

        if not self.dbm.currentKey:
            # there is no current database -> no layer for snapping
            return QgsSnapper(self.canvas.mapRenderer())

        snapper=QgsSnapper(self.canvas.mapRenderer())
        snapLayers=[]

        # snap to line and polygon layer from current database only
        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.lineLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,self.canvas.mapRenderer())
        sLayer.mSnapTo=QgsSnapper.SnapToSegment
        snapLayers.append(sLayer)

        sLayer=QgsSnapper.SnapLayer()
        sLayer.mLayer=self.dbm.polygonLayers[self.dbm.currentKey]
        sLayer.mTolerance=QgsTolerance.vertexSearchRadius(sLayer.mLayer,self.canvas.mapRenderer())
        sLayer.mSnapTo=QgsSnapper.SnapToSegment
        snapLayers.append(sLayer)

        snapper.setSnapLayers(snapLayers)
        return snapper


    def deactivate(self):
        """Functions is called when create point map-tool is being deactivated.

        Function performs standard cleaning; re-initialization etc.
        """

        self.dockWidget.toolButtons.setExclusive(False)
        self.dockWidget.createPointButton.setChecked(False)
        self.dockWidget.toolButtons.setExclusive(True)
        self.dockWidget.activeEditButton=self.dockWidget.dummyButton

        if self.verMarker:
            self.verMarker.setCenter(QgsPoint(-1000,-1000))
            self.canvas.scene().removeItem(self.verMarker)
            del self.verMarker
            self.verMarker=None

        self.dockWidget.clear()


    def keyPressEvent(self, event):
        """This function is called after keyPressEvent(QKeyEvent *) signal
        is emmited when using this map tool.

        If Control key was pressed, function disables snapping til key is released again.

        @param event event that occured when key pressing
        """

        if (event.key()==Qt.Key_Control):
            self.snappingEnabled=False
            if self.verMarker:
                self.verMarker.setCenter(QgsPoint(-1000,-1000))
            self.snappedPoint=None
            self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("Snapping OFF.")


    def keyReleaseEvent(self, event):
        """This function is called after keyReleaseEvent(QKeyEvent *) signal
        is emmited when using this map tool.

        If Control key was released, function enables snapping again.

        @param event event that occured when key releasing
        """

        if (event.key()==Qt.Key_Control):
            self.snappingEnabled=True
            self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("Snapping ON. Hold Ctrl to disable it.")


    def canvasReleaseEvent(self,event):
        """This function is called after mouse button releasing when using this map tool.

        If left button is released new point is created.
        Right (and other) clicking does nothing.

        @param event event that occured when button releasing
        """

        if event.button()<>Qt.LeftButton:
            return    # nothing to do

        if not self.snappedPoint:
            newPoint = self.dockWidget.canvasToOsmCoords(event.pos())
        else:
            newPoint=self.snappedPoint

        self.ur.startAction("Create a point.")
        (node,affected)=self.dbm.createPoint(newPoint,self.snapFeat,self.snapFeatType)
        self.ur.stopAction(affected)
        self.dbm.recacheAffectedNow(affected)

        if node:
            self.dockWidget.loadFeature(node,"Point",2)

        self.canvas.refresh()


    def canvasMoveEvent(self,event):
        """This function is called when mouse moving.

        @param event event that occured when mouse moving.
        """

        self.mapPoint = self.dockWidget.canvasToOsmCoords(event.pos())

        # try snapping to the closest vertex/segment
        if not self.snappingEnabled:
            self.verMarker.setCenter(self.mapPoint)
            return

        # snapping! first reset old snapping vertexMarker
        self.verMarker.setCenter(QgsPoint(-1000,-1000))

        (retval,snappingResults)=self.snapper.snapPoint(event.pos(),[])

        if len(snappingResults)==0:
            self.verMarker.setCenter(self.mapPoint)
            self.snappedPoint=None
            self.snapFeat=None
            self.snapFeatType=None

            if self.dockWidget.feature:
                self.dockWidget.clear()
            return

        self.snappedPoint=QgsPoint(snappingResults[0].snappedVertex)
        self.verMarker.setCenter(self.snappedPoint)

        # start identification
        feature=self.dbm.findFeature(self.snappedPoint)
        if feature:
            (self.snapFeat,self.snapFeatType)=feature
            if not self.dockWidget.feature or self.snapFeat.id()<>self.dockWidget.feature.id():
                self.dockWidget.loadFeature(self.snapFeat,self.snapFeatType)



