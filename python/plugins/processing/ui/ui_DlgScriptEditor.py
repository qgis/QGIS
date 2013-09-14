# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgScriptEditor.ui'
#
# Created: Sat Sep 14 12:47:57 2013
#      by: PyQt4 UI code generator 4.9.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    _fromUtf8 = lambda s: s

class Ui_DlgScriptEditor(object):
    def setupUi(self, DlgScriptEditor):
        DlgScriptEditor.setObjectName(_fromUtf8("DlgScriptEditor"))
        DlgScriptEditor.resize(720, 480)
        self.verticalLayout = QtGui.QVBoxLayout(DlgScriptEditor)
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(-1)
        self.horizontalLayout.setContentsMargins(3, 3, 3, -1)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.btnSave = QtGui.QToolButton(DlgScriptEditor)
        self.btnSave.setObjectName(_fromUtf8("btnSave"))
        self.horizontalLayout.addWidget(self.btnSave)
        self.btnSaveAs = QtGui.QToolButton(DlgScriptEditor)
        self.btnSaveAs.setObjectName(_fromUtf8("btnSaveAs"))
        self.horizontalLayout.addWidget(self.btnSaveAs)
        self.line = QtGui.QFrame(DlgScriptEditor)
        self.line.setFrameShape(QtGui.QFrame.VLine)
        self.line.setFrameShadow(QtGui.QFrame.Sunken)
        self.line.setObjectName(_fromUtf8("line"))
        self.horizontalLayout.addWidget(self.line)
        self.btnEditHelp = QtGui.QToolButton(DlgScriptEditor)
        self.btnEditHelp.setObjectName(_fromUtf8("btnEditHelp"))
        self.horizontalLayout.addWidget(self.btnEditHelp)
        self.line_2 = QtGui.QFrame(DlgScriptEditor)
        self.line_2.setFrameShape(QtGui.QFrame.VLine)
        self.line_2.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_2.setObjectName(_fromUtf8("line_2"))
        self.horizontalLayout.addWidget(self.line_2)
        self.btnRun = QtGui.QToolButton(DlgScriptEditor)
        self.btnRun.setObjectName(_fromUtf8("btnRun"))
        self.horizontalLayout.addWidget(self.btnRun)
        self.line_3 = QtGui.QFrame(DlgScriptEditor)
        self.line_3.setFrameShape(QtGui.QFrame.VLine)
        self.line_3.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_3.setObjectName(_fromUtf8("line_3"))
        self.horizontalLayout.addWidget(self.line_3)
        self.btnCut = QtGui.QToolButton(DlgScriptEditor)
        self.btnCut.setObjectName(_fromUtf8("btnCut"))
        self.horizontalLayout.addWidget(self.btnCut)
        self.btnCopy = QtGui.QToolButton(DlgScriptEditor)
        self.btnCopy.setObjectName(_fromUtf8("btnCopy"))
        self.horizontalLayout.addWidget(self.btnCopy)
        self.btnPaste = QtGui.QToolButton(DlgScriptEditor)
        self.btnPaste.setObjectName(_fromUtf8("btnPaste"))
        self.horizontalLayout.addWidget(self.btnPaste)
        self.line_4 = QtGui.QFrame(DlgScriptEditor)
        self.line_4.setFrameShape(QtGui.QFrame.VLine)
        self.line_4.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_4.setObjectName(_fromUtf8("line_4"))
        self.horizontalLayout.addWidget(self.line_4)
        self.btnUndo = QtGui.QToolButton(DlgScriptEditor)
        self.btnUndo.setObjectName(_fromUtf8("btnUndo"))
        self.horizontalLayout.addWidget(self.btnUndo)
        self.btnRedo = QtGui.QToolButton(DlgScriptEditor)
        self.btnRedo.setObjectName(_fromUtf8("btnRedo"))
        self.horizontalLayout.addWidget(self.btnRedo)
        spacerItem = QtGui.QSpacerItem(40, 20, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Minimum)
        self.horizontalLayout.addItem(spacerItem)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.editor = ScriptEdit(DlgScriptEditor)
        self.editor.setObjectName(_fromUtf8("editor"))
        self.verticalLayout.addWidget(self.editor)

        self.retranslateUi(DlgScriptEditor)
        QtCore.QMetaObject.connectSlotsByName(DlgScriptEditor)
        DlgScriptEditor.setTabOrder(self.btnSave, self.btnSaveAs)
        DlgScriptEditor.setTabOrder(self.btnSaveAs, self.btnEditHelp)
        DlgScriptEditor.setTabOrder(self.btnEditHelp, self.btnRun)

    def retranslateUi(self, DlgScriptEditor):
        DlgScriptEditor.setWindowTitle(QtGui.QApplication.translate("DlgScriptEditor", "Script editor", None, QtGui.QApplication.UnicodeUTF8))
        self.btnSave.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Save", None, QtGui.QApplication.UnicodeUTF8))
        self.btnSave.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnSaveAs.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Save as...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnSaveAs.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnEditHelp.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Edit script help", None, QtGui.QApplication.UnicodeUTF8))
        self.btnEditHelp.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnRun.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Run algorithm", None, QtGui.QApplication.UnicodeUTF8))
        self.btnRun.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnCut.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Cut", None, QtGui.QApplication.UnicodeUTF8))
        self.btnCut.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnCopy.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Copy", None, QtGui.QApplication.UnicodeUTF8))
        self.btnCopy.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnPaste.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Paste", None, QtGui.QApplication.UnicodeUTF8))
        self.btnPaste.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnUndo.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Undo", None, QtGui.QApplication.UnicodeUTF8))
        self.btnUndo.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))
        self.btnRedo.setToolTip(QtGui.QApplication.translate("DlgScriptEditor", "Redo", None, QtGui.QApplication.UnicodeUTF8))
        self.btnRedo.setText(QtGui.QApplication.translate("DlgScriptEditor", "...", None, QtGui.QApplication.UnicodeUTF8))

from processing.gui.ScriptEdit import ScriptEdit
