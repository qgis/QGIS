# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'DlgScriptEditor.ui'
#
# Created: Fri Nov 21 13:25:49 2014
#      by: PyQt4 UI code generator 4.11.1
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
    _fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
    def _fromUtf8(s):
        return s

try:
    _encoding = QtGui.QApplication.UnicodeUTF8
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
    def _translate(context, text, disambig):
        return QtGui.QApplication.translate(context, text, disambig)

class Ui_DlgScriptEditor(object):
    def setupUi(self, DlgScriptEditor):
        DlgScriptEditor.setObjectName(_fromUtf8("DlgScriptEditor"))
        DlgScriptEditor.resize(720, 480)
        self.verticalLayout = QtGui.QVBoxLayout(DlgScriptEditor)
        self.verticalLayout.setSpacing(6)
        self.verticalLayout.setMargin(9)
        self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(6)
        self.horizontalLayout.setContentsMargins(3, 3, 3, -1)
        self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
        self.btnOpen = QtGui.QToolButton(DlgScriptEditor)
        self.btnOpen.setAutoRaise(True)
        self.btnOpen.setObjectName(_fromUtf8("btnOpen"))
        self.horizontalLayout.addWidget(self.btnOpen)
        self.btnSave = QtGui.QToolButton(DlgScriptEditor)
        self.btnSave.setAutoRaise(True)
        self.btnSave.setObjectName(_fromUtf8("btnSave"))
        self.horizontalLayout.addWidget(self.btnSave)
        self.btnSaveAs = QtGui.QToolButton(DlgScriptEditor)
        self.btnSaveAs.setAutoRaise(True)
        self.btnSaveAs.setObjectName(_fromUtf8("btnSaveAs"))
        self.horizontalLayout.addWidget(self.btnSaveAs)
        self.line = QtGui.QFrame(DlgScriptEditor)
        self.line.setFrameShape(QtGui.QFrame.VLine)
        self.line.setFrameShadow(QtGui.QFrame.Sunken)
        self.line.setObjectName(_fromUtf8("line"))
        self.horizontalLayout.addWidget(self.line)
        self.btnEditHelp = QtGui.QToolButton(DlgScriptEditor)
        self.btnEditHelp.setAutoRaise(True)
        self.btnEditHelp.setObjectName(_fromUtf8("btnEditHelp"))
        self.horizontalLayout.addWidget(self.btnEditHelp)
        self.line_2 = QtGui.QFrame(DlgScriptEditor)
        self.line_2.setFrameShape(QtGui.QFrame.VLine)
        self.line_2.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_2.setObjectName(_fromUtf8("line_2"))
        self.horizontalLayout.addWidget(self.line_2)
        self.btnRun = QtGui.QToolButton(DlgScriptEditor)
        self.btnRun.setAutoRaise(True)
        self.btnRun.setObjectName(_fromUtf8("btnRun"))
        self.horizontalLayout.addWidget(self.btnRun)
        self.line_3 = QtGui.QFrame(DlgScriptEditor)
        self.line_3.setFrameShape(QtGui.QFrame.VLine)
        self.line_3.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_3.setObjectName(_fromUtf8("line_3"))
        self.horizontalLayout.addWidget(self.line_3)
        self.btnCut = QtGui.QToolButton(DlgScriptEditor)
        self.btnCut.setAutoRaise(True)
        self.btnCut.setObjectName(_fromUtf8("btnCut"))
        self.horizontalLayout.addWidget(self.btnCut)
        self.btnCopy = QtGui.QToolButton(DlgScriptEditor)
        self.btnCopy.setAutoRaise(True)
        self.btnCopy.setObjectName(_fromUtf8("btnCopy"))
        self.horizontalLayout.addWidget(self.btnCopy)
        self.btnPaste = QtGui.QToolButton(DlgScriptEditor)
        self.btnPaste.setAutoRaise(True)
        self.btnPaste.setObjectName(_fromUtf8("btnPaste"))
        self.horizontalLayout.addWidget(self.btnPaste)
        self.line_4 = QtGui.QFrame(DlgScriptEditor)
        self.line_4.setFrameShape(QtGui.QFrame.VLine)
        self.line_4.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_4.setObjectName(_fromUtf8("line_4"))
        self.horizontalLayout.addWidget(self.line_4)
        self.btnUndo = QtGui.QToolButton(DlgScriptEditor)
        self.btnUndo.setAutoRaise(True)
        self.btnUndo.setObjectName(_fromUtf8("btnUndo"))
        self.horizontalLayout.addWidget(self.btnUndo)
        self.btnRedo = QtGui.QToolButton(DlgScriptEditor)
        self.btnRedo.setAutoRaise(True)
        self.btnRedo.setObjectName(_fromUtf8("btnRedo"))
        self.horizontalLayout.addWidget(self.btnRedo)
        self.line_5 = QtGui.QFrame(DlgScriptEditor)
        self.line_5.setFrameShape(QtGui.QFrame.VLine)
        self.line_5.setFrameShadow(QtGui.QFrame.Sunken)
        self.line_5.setObjectName(_fromUtf8("line_5"))
        self.horizontalLayout.addWidget(self.line_5)
        self.btnSnippets = QtGui.QToolButton(DlgScriptEditor)
        self.btnSnippets.setAutoRaise(True)
        self.btnSnippets.setObjectName(_fromUtf8("btnSnippets"))
        self.horizontalLayout.addWidget(self.btnSnippets)
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
        DlgScriptEditor.setWindowTitle(_translate("DlgScriptEditor", "Script editor", None))
        self.btnOpen.setToolTip(_translate("DlgScriptEditor", "Open script", None))
        self.btnOpen.setText(_translate("DlgScriptEditor", "...", None))
        self.btnOpen.setShortcut(_translate("DlgScriptEditor", "Ctrl+O, Return", None))
        self.btnSave.setToolTip(_translate("DlgScriptEditor", "Save", None))
        self.btnSave.setText(_translate("DlgScriptEditor", "...", None))
        self.btnSave.setShortcut(_translate("DlgScriptEditor", "Ctrl+S", None))
        self.btnSaveAs.setToolTip(_translate("DlgScriptEditor", "Save as...", None))
        self.btnSaveAs.setText(_translate("DlgScriptEditor", "...", None))
        self.btnSaveAs.setShortcut(_translate("DlgScriptEditor", "Ctrl+Shift+S", None))
        self.btnEditHelp.setToolTip(_translate("DlgScriptEditor", "Edit script help", None))
        self.btnEditHelp.setText(_translate("DlgScriptEditor", "...", None))
        self.btnRun.setToolTip(_translate("DlgScriptEditor", "Run algorithm", None))
        self.btnRun.setText(_translate("DlgScriptEditor", "...", None))
        self.btnRun.setShortcut(_translate("DlgScriptEditor", "F5", None))
        self.btnCut.setToolTip(_translate("DlgScriptEditor", "Cut", None))
        self.btnCut.setText(_translate("DlgScriptEditor", "...", None))
        self.btnCut.setShortcut(_translate("DlgScriptEditor", "Ctrl+X", None))
        self.btnCopy.setToolTip(_translate("DlgScriptEditor", "Copy", None))
        self.btnCopy.setText(_translate("DlgScriptEditor", "...", None))
        self.btnCopy.setShortcut(_translate("DlgScriptEditor", "Ctrl+C", None))
        self.btnPaste.setToolTip(_translate("DlgScriptEditor", "Paste", None))
        self.btnPaste.setText(_translate("DlgScriptEditor", "...", None))
        self.btnPaste.setShortcut(_translate("DlgScriptEditor", "Ctrl+V", None))
        self.btnUndo.setToolTip(_translate("DlgScriptEditor", "Undo", None))
        self.btnUndo.setText(_translate("DlgScriptEditor", "...", None))
        self.btnUndo.setShortcut(_translate("DlgScriptEditor", "Ctrl+Z", None))
        self.btnRedo.setToolTip(_translate("DlgScriptEditor", "Redo", None))
        self.btnRedo.setText(_translate("DlgScriptEditor", "...", None))
        self.btnRedo.setShortcut(_translate("DlgScriptEditor", "Ctrl+Shift+Z", None))
        self.btnSnippets.setText(_translate("DlgScriptEditor", "...", None))

from processing.gui.ScriptEdit import ScriptEdit
