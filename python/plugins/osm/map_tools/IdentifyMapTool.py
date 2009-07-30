"""@package IdentifyMapTool
This module holds all structures and methods required to perform
"identify feature" operation on current OSM data.

When feature is identified its id, type, timestamp, owner, properties/tags, relations are loaded
into OSM Feature widget. Feature is also marked with rubberband (or vertexmarker for points) on map.

If you want to identify some feature, just left-click on it.

If OSM Plugin marked wrong feature after that, repeat RIGHT-clicking til the right one is marked.
(Right-clicking gives you one by one each feature that is in the place where left-click was done.)

If no feature is marked after your left-clicking, you missed the feature :-) Try again.

If you are not able to hit any feature, be sure that map data you are trying to identify are the current OSM data.

If they are, maybe there is something wrong in your QGIS settings. Be sure that there aren't too small values
in QGIS Settings -> Digitalization -> Tolerance/Snapping.

If you've just identified the wrong feature or want to identify a new one,
just left-click to continue the identification process.
"""


from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.core import *
from qgis.gui import *


class IdentifyMapTool(QgsMapTool):
    """This class holds all structures and methods required to perform
    "identify feature" operation on current OSM data.

    When feature is identified its id, type, timestamp, owner, properties/tags, relations are loaded
    into OSM Feature widget. Feature is also marked with rubberband (or vertexmarker for points) on map.

    If you want to identify some feature, just left-click on it.

    If OSM Plugin marked wrong feature after that, repeat RIGHT-clicking til the right one is marked.
    (Right-clicking gives you one by one each feature that is in the place where left-click was done.)

    If no feature is marked after your left-clicking, you missed the feature :-) Try again.

    If you are not able to hit any feature, be sure that map data you are trying to identify are the current OSM data.

    If they are, maybe there is something wrong in your QGIS settings. Be sure that there aren't too small values
    in QGIS Settings -> Digitalization -> Tolerance/Snapping.

    If you've just identified the wrong feature or want to identify a new one,
    just left-click to continue the identification process.
    """


    def __init__(self, canvas, dockWidget, dbManager):
        """The constructor.
        Initializes the map tool.

        @param canvas map canvas
        @param dockWidget pointer to the main widget (OSM Feature widget) of OSM Plugin
        @param dbManager pointer to instance of DatabaseManager; for communication with sqlite3 database
        """

        QgsMapTool.__init__(self,canvas)

        self.canvas=canvas
        self.dockWidget=dockWidget
        self.dbm=dbManager
        self.moves=0
        self.pause=False
        self.doubleclick=False
        self.featuresFound=[]
        self.ixFeature=0

        self.dlgSelect=QDialog(self.dockWidget)
        self.dlgSelect.setWindowTitle("Feature identification")
        butBox=QDialogButtonBox(QDialogButtonBox.Ok|QDialogButtonBox.Cancel,Qt.Horizontal,self.dlgSelect)

        self.lw=QListWidget(self.dlgSelect)

        layout=QVBoxLayout(self.dlgSelect)
        layout.addWidget(self.lw)
        layout.addWidget(butBox)
        self.dlgSelect.setLayout(layout)

        QObject.connect(butBox,SIGNAL("accepted()"),self.onSelectDlgOK)
        QObject.connect(butBox,SIGNAL("rejected()"),self.onSelectDlgCancel)

        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")


    def databaseChanged(self,dbKey):
        """This function is called automatically when current OSM database has changed.

        Function does re-initialization of maptool.

        @param dbKey key of database with new current OSM data
        """

        # re-initialization
        self.pause=False
        self.doubleclick=False
        self.featuresFound=[]
        self.ixFeature=0

        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")


    def canvasDoubleClickEvent(self,event):
        """This function is called after doubleclick is done on map when using this map tool.
        Function is interested into left-doubleclicking only.

        It finds out all features that are currently at the place where doubleclick was done.
        Then it shows simple dialog with the list of all these features. User can select the required one
        and close dialog.

        Selected feature is then loaded into OSM Feature widget.

        @param event event that occured when double clicking
        """

        if event.button()<>Qt.LeftButton:
            return

        self.dockWidget.clear()

        # find out map coordinates from mouse click
        mapPoint=self.dockWidget.canvasToOsmCoords(event.pos())

        # display modal dialog with features selection
        self.featuresFound=self.dbm.findAllFeatures(mapPoint)
        self.ixFeature=0

        lwItems=[]
        for f in self.featuresFound:
            feat=f[0]
            featType=f[1]
            name=self.dbm.getTagValue(feat.id(),featType,"name")
            lwItems.append(QString("[%1] ").arg(feat.id()).append(featType).append(QString(" ")).append(name))

        self.lw.clear()
        self.lw.addItems(lwItems)

        self.pause=False
        self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")
        self.doubleclick=True

        # continue only if OK button was clicked
        if self.dlgSelect.exec_()==0:
            return


    def canvasReleaseEvent(self,event):
        """This function is called after mouse button is released on map when using this map tool.

        It finds out all features that are currently at place where releasing was done.

        OSM Plugin then marks the first of them. User can repeat right-clicking to mark
        the next one, the next one, the next one... periodically...
        Note that only one feature is marked at a time.

        Each marked feature is also loaded into OSM Feature widget.

        @param event event that occured when button releasing
        """

        if self.doubleclick:
            self.doubleclick=False
            return

        # we are interested only in left/right button clicking
        if event.button() not in (Qt.LeftButton,Qt.RightButton):
            return

        # find out map coordinates from mouse click
        mapPoint=self.dockWidget.canvasToOsmCoords(event.pos())

        if event.button()==Qt.LeftButton:

            if self.pause:
                self.pause=False
                self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("")
                self.featuresFound=[]
                self.ixFeature=0
                return

            # start identification
            self.featuresFound=self.dbm.findAllFeatures(mapPoint)
            self.ixFeature=0

            if len(self.featuresFound)>0:

                self.pause=True
                self.dockWidget.plugin.iface.mainWindow().statusBar().showMessage("PAUSED. Left-click to continue.")

                (feat,featType)=self.featuresFound[self.ixFeature]

                self.dockWidget.loadFeature(feat,featType,2)

            elif self.dockWidget.feature:
                self.dockWidget.clear()

        elif event.button()==Qt.RightButton:

            if len(self.featuresFound)<1:
                return

            self.ixFeature=self.ixFeature+1
            if self.ixFeature>=len(self.featuresFound):
                self.ixFeature=0

            (feat,featType)=self.featuresFound[self.ixFeature]
            self.dockWidget.loadFeature(feat,featType,2)


    def canvasMoveEvent(self,event):
        """This function is called after mouse moving.

        Feature are marked and loaded dynamically when going over them.

        @param event event that occured when mouse moving.
        """

        if self.pause:
            return

        if self.moves<>1:
            self.moves=self.moves+1
            return

        self.moves=0

        # find out map coordinates from mouse click
        mapPoint=self.dockWidget.canvasToOsmCoords(event.pos())

        # start identification
        feature=self.dbm.findFeature(mapPoint)

        if feature:
            (feat,featType)=feature

            if not self.dockWidget.feature or feat.id()<>self.dockWidget.feature.id():
                self.dockWidget.loadFeature(feat,featType,1)

        elif self.dockWidget.feature:

            self.dockWidget.clear()


    def onSelectDlgOK(self):
        """This function handles clicking on OK button of selection dialog.
        """

        self.dlgSelect.close()

        if not self.lw.currentItem():
            return

        self.ixFeature=self.lw.currentRow()
        (feat,featType)=self.featuresFound[self.ixFeature]

        if feat:
            self.dockWidget.loadFeature(feat,featType,2)
            self.pause=True


    def onSelectDlgCancel(self):
        """This function handles clicking on Cancel button of selection dialog.
        """

        self.dlgSelect.close()


    def deactivate(self):
        """Functions is called when identify-map-tool is being deactivated.

        It performs standard cleaning;
        re-initialization etc.
        """

        self.dockWidget.toolButtons.setExclusive(False)
        self.dockWidget.identifyButton.setChecked(False)
        self.dockWidget.toolButtons.setExclusive(True)
        self.dockWidget.activeEditButton=self.dockWidget.dummyButton
        self.pause=False
        self.doubleclick=False

        self.dockWidget.clear()
        self.featuresFound=[]
        self.ixFeature=0


