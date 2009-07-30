# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'ui_files/DockUndoRedo.ui'
#
# Created: Wed Jul 29 12:14:34 2009
#      by: PyQt4 UI code generator 4.4.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_OsmUndoRedoDockWidget(object):
    def setupUi(self, OsmUndoRedoDockWidget):
        OsmUndoRedoDockWidget.setObjectName("OsmUndoRedoDockWidget")
        OsmUndoRedoDockWidget.resize(227, 374)
        OsmUndoRedoDockWidget.setAllowedAreas(QtCore.Qt.AllDockWidgetAreas)
        self.dockWidgetContents = QtGui.QWidget()
        self.dockWidgetContents.setObjectName("dockWidgetContents")
        self.vboxlayout = QtGui.QVBoxLayout(self.dockWidgetContents)
        self.vboxlayout.setObjectName("vboxlayout")
        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setObjectName("hboxlayout")
        self.clearButton = QtGui.QToolButton(self.dockWidgetContents)
        self.clearButton.setObjectName("clearButton")
        self.hboxlayout.addWidget(self.clearButton)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.hboxlayout.addItem(spacerItem)
        self.undoButton = QtGui.QToolButton(self.dockWidgetContents)
        self.undoButton.setEnabled(False)
        self.undoButton.setObjectName("undoButton")
        self.hboxlayout.addWidget(self.undoButton)
        self.redoButton = QtGui.QToolButton(self.dockWidgetContents)
        self.redoButton.setEnabled(False)
        self.redoButton.setObjectName("redoButton")
        self.hboxlayout.addWidget(self.redoButton)
        self.vboxlayout.addLayout(self.hboxlayout)
        self.actionList = QtGui.QListWidget(self.dockWidgetContents)
        self.actionList.setObjectName("actionList")
        self.vboxlayout.addWidget(self.actionList)
        OsmUndoRedoDockWidget.setWidget(self.dockWidgetContents)

        self.retranslateUi(OsmUndoRedoDockWidget)
        QtCore.QMetaObject.connectSlotsByName(OsmUndoRedoDockWidget)

    def retranslateUi(self, OsmUndoRedoDockWidget):
        OsmUndoRedoDockWidget.setWindowTitle(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "OSM Edit History", None, QtGui.QApplication.UnicodeUTF8))
        self.clearButton.setToolTip(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Clear all", None, QtGui.QApplication.UnicodeUTF8))
        self.clearButton.setStatusTip(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Clear all", None, QtGui.QApplication.UnicodeUTF8))
        self.clearButton.setWhatsThis(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Clear all", None, QtGui.QApplication.UnicodeUTF8))
        self.clearButton.setText(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.undoButton.setToolTip(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Undo", None, QtGui.QApplication.UnicodeUTF8))
        self.undoButton.setStatusTip(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Undo", None, QtGui.QApplication.UnicodeUTF8))
        self.undoButton.setWhatsThis(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Undo", None, QtGui.QApplication.UnicodeUTF8))
        self.undoButton.setText(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.redoButton.setToolTip(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Redo", None, QtGui.QApplication.UnicodeUTF8))
        self.redoButton.setStatusTip(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Redo", None, QtGui.QApplication.UnicodeUTF8))
        self.redoButton.setWhatsThis(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "Redo", None, QtGui.QApplication.UnicodeUTF8))
        self.redoButton.setText(QtGui.QApplication.translate("OsmUndoRedoDockWidget", "...", None, QtGui.QApplication.UnicodeUTF8))

