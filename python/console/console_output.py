# -*- coding:utf-8 -*-
"""
/***************************************************************************
Python Console for QGIS
                             -------------------
begin                : 2012-09-10
copyright            : (C) 2012 by Salvatore Larosa
email                : lrssvtml (at) gmail (dot) com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
Some portions of code were taken from https://code.google.com/p/pydee/
"""

from qgis.PyQt import sip
from qgis.PyQt.QtCore import Qt, QCoreApplication, QThread, QMetaObject, Q_ARG, QObject, pyqtSlot
from qgis.PyQt.QtGui import QColor, QKeySequence
from qgis.PyQt.QtWidgets import QAction, QGridLayout, QSpacerItem, QSizePolicy, QShortcut, QMenu, QApplication
from qgis.PyQt.Qsci import QsciScintilla
from qgis.core import Qgis, QgsApplication, QgsSettings
from qgis.gui import QgsMessageBar, QgsCodeEditorPython
import sys


class writeOut(QObject):
    ERROR_COLOR = "#e31a1c"

    def __init__(self, shellOut, out=None, style=None):
        """
        This class allows writing to stdout and stderr
        """
        super().__init__()
        self.sO = shellOut
        self.out = None
        self.style = style
        self.fire_keyboard_interrupt = False

    @pyqtSlot(str)
    def write(self, m):
        if sip.isdeleted(self.sO):
            return

        # This manage the case when console is called from another thread
        if QThread.currentThread() != QCoreApplication.instance().thread():
            QMetaObject.invokeMethod(self, "write", Qt.ConnectionType.QueuedConnection, Q_ARG(str, m))
            return

        if self.style == "_traceback":
            # Show errors in red
            stderrColor = QColor(QgsSettings().value("pythonConsole/stderrFontColor", QColor(self.ERROR_COLOR)))
            self.sO.SendScintilla(QsciScintilla.SCI_STYLESETFORE, 0o01, stderrColor)
            self.sO.SendScintilla(QsciScintilla.SCI_STYLESETITALIC, 0o01, True)
            self.sO.SendScintilla(QsciScintilla.SCI_STYLESETBOLD, 0o01, True)
            pos = self.sO.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS)
            self.sO.SendScintilla(QsciScintilla.SCI_STARTSTYLING, pos, 31)
            self.sO.append(m)
            self.sO.SendScintilla(QsciScintilla.SCI_SETSTYLING, len(m), 0o01)
        else:
            self.sO.append(m)

        if self.out:
            self.out.write(m)

        self.sO.moveCursorToEnd()

        if self.style != "_traceback":
            self.sO.repaint()

        if self.fire_keyboard_interrupt:
            self.fire_keyboard_interrupt = False
            raise KeyboardInterrupt

    def flush(self):
        pass

    def isatty(self):
        return False


FULL_HELP_TEXT = QCoreApplication.translate("PythonConsole", """QGIS Python Console
======================================

The console is a Python interpreter that allows you to execute python commands.
Modules from QGIS (analysis, core, gui, 3d) and Qt (QtCore, QtGui, QtNetwork,
QtWidgets, QtXml) as well as Python's math, os, re and sys modules are already
imported and can be used directly.

Useful variables:

- iface will return the current QGIS interface, class 'QgisInterface'
- iface.mainWindow() will return the Qt Main Window
- iface.mapCanvas() will return the map canvas
- iface.layerTreeView() will return the layer tree
- iface.activeLayer() will return the active layer
- QgsProject.instance() will return the current project

From the console, you can type the following special commands:

    - _pyqgis, _pyqgis(object): Open the QGIS Python API (or the Qt documentation) in a web browser
    - _api, _api(object): Open the QGIS C++ API (or the Qt documentation) in a web browser
    - _cookbook: Open the PyQGIS Developer Cookbook in a web browser
    - System commands: Any command starting with an exclamation mark (!) will be executed by the system shell. Examples:
        !gdalinfo --formats: List all available GDAL drivers
        !ogr2ogr --help: Show help for the ogr2ogr command
        !ping www.qgis.org: Ping the QGIS website
        !pip install black: install black python formatter using pip (if available)
    - ?: Show this help
""")


class ShellOutputScintilla(QgsCodeEditorPython):

    def __init__(self, parent=None):
        super().__init__(parent)
        self.parent = parent
        self.shell = self.parent.shell

        # Creates layout for message bar
        self.layout = QGridLayout(self)
        self.layout.setContentsMargins(0, 0, 0, 0)
        spacerItem = QSpacerItem(20, 40, QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Expanding)
        self.layout.addItem(spacerItem, 1, 0, 1, 1)
        # messageBar instance
        self.infoBar = QgsMessageBar()
        sizePolicy = QSizePolicy(QSizePolicy.Policy.Minimum, QSizePolicy.Policy.Fixed)
        self.infoBar.setSizePolicy(sizePolicy)
        self.layout.addWidget(self.infoBar, 0, 0, 1, 1)

        self._old_stdout = sys.stdout
        self._old_stderr = sys.stderr

        sys.stdout = writeOut(self, sys.stdout)
        sys.stderr = writeOut(self, sys.stderr, "_traceback")

        QgsApplication.instance().aboutToQuit.connect(self.on_app_exit)

        self.insertInitText()
        self.refreshSettingsOutput()

        self.setMinimumHeight(120)

        self.setWrapMode(QsciScintilla.WrapMode.WrapCharacter)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)

        self.runScut = QShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_E), self)
        self.runScut.setContext(Qt.ShortcutContext.WidgetShortcut)
        self.runScut.activated.connect(self.enteredSelected)
        # Reimplemented copy action to prevent paste prompt (>>>,...) in command view
        self.copyShortcut = QShortcut(QKeySequence.StandardKey.Copy, self)
        self.copyShortcut.setContext(Qt.ShortcutContext.WidgetWithChildrenShortcut)
        self.copyShortcut.activated.connect(self.copy)
        self.selectAllShortcut = QShortcut(QKeySequence.StandardKey.SelectAll, self)
        self.selectAllShortcut.setContext(Qt.ShortcutContext.WidgetWithChildrenShortcut)
        self.selectAllShortcut.activated.connect(self.selectAll)

    def on_app_exit(self):
        """
        Prepares the console for a graceful close
        """
        sys.stdout = self._old_stdout
        sys.stderr = self._old_stderr

    def insertInitText(self):
        txtInit = QCoreApplication.translate("PythonConsole",
                                             "Python Console\n"
                                             "Use iface to access QGIS API interface or type '?' for more info\n"
                                             "Security warning: typing commands from an untrusted source can harm your computer")

        txtInit = '\n'.join(['# ' + line for line in txtInit.split('\n')])

        # some translation string for the console header ends without '\n'
        # and the first command in console will be appended at the header text.
        # The following code add a '\n' at the end of the string if not present.
        if txtInit.endswith('\n'):
            self.setText(txtInit)
        else:
            self.setText(txtInit + '\n')

    def insertHelp(self):
        self.append(FULL_HELP_TEXT)
        self.moveCursorToEnd()

    def initializeLexer(self):
        super().initializeLexer()
        self.setFoldingVisible(False)
        self.setEdgeMode(QsciScintilla.EdgeMode.EdgeNone)

    def refreshSettingsOutput(self):
        # Set Python lexer
        self.initializeLexer()
        self.setReadOnly(True)

        self.setCaretWidth(0)  # NO (blinking) caret in the output

    def clearConsole(self):
        self.setText('')
        self.insertInitText()
        self.shell.setFocus()

    def contextMenuEvent(self, e):
        menu = QMenu(self)
        menu.addAction(QgsApplication.getThemeIcon("console/iconHideToolConsole.svg"),
                       QCoreApplication.translate("PythonConsole", "Hide/Show Toolbar"),
                       self.hideToolBar)
        menu.addSeparator()
        showEditorAction = menu.addAction(
            QgsApplication.getThemeIcon("console/iconShowEditorConsole.svg"),
            QCoreApplication.translate("PythonConsole", "Show Editor"),
            self.showEditor)
        menu.addSeparator()
        runAction = QAction(QgsApplication.getThemeIcon("console/mIconRunConsole.svg"),
                            QCoreApplication.translate("PythonConsole", "Enter Selected"),
                            menu)
        runAction.triggered.connect(self.enteredSelected)
        runAction.setShortcut(QKeySequence(Qt.Modifier.CTRL | Qt.Key.Key_E))
        menu.addAction(runAction)

        clearAction = QAction(QgsApplication.getThemeIcon("console/iconClearConsole.svg"),
                              QCoreApplication.translate("PythonConsole", "Clear Console"),
                              menu)
        clearAction.triggered.connect(self.clearConsole)
        menu.addAction(clearAction)

        pyQGISHelpAction = QAction(QgsApplication.getThemeIcon("console/iconHelpConsole.svg"),
                                   QCoreApplication.translate("PythonConsole", "Search Selection in PyQGIS Documentation"),
                                   menu)
        pyQGISHelpAction.triggered.connect(self.searchSelectedTextInPyQGISDocs)
        menu.addAction(pyQGISHelpAction)

        menu.addSeparator()
        copyAction = QAction(
            QgsApplication.getThemeIcon("mActionEditCopy.svg"),
            QCoreApplication.translate("PythonConsole", "Copy"),
            menu)
        copyAction.triggered.connect(self.copy)
        copyAction.setShortcut(QKeySequence.StandardKey.Copy)
        menu.addAction(copyAction)

        selectAllAction = QAction(
            QCoreApplication.translate("PythonConsole", "Select All"),
            menu)
        selectAllAction.triggered.connect(self.selectAll)
        selectAllAction.setShortcut(QKeySequence.StandardKey.SelectAll)
        menu.addAction(selectAllAction)

        menu.addSeparator()
        settings_action = QAction(QgsApplication.getThemeIcon("console/iconSettingsConsole.svg"),
                                  QCoreApplication.translate("PythonConsole", "Options…"),
                                  menu)
        settings_action.triggered.connect(self.parent.openSettings)
        menu.addAction(settings_action)

        runAction.setEnabled(False)
        clearAction.setEnabled(False)
        copyAction.setEnabled(False)
        pyQGISHelpAction.setEnabled(False)
        selectAllAction.setEnabled(False)
        showEditorAction.setEnabled(True)
        if self.hasSelectedText():
            runAction.setEnabled(True)
            copyAction.setEnabled(True)
            pyQGISHelpAction.setEnabled(True)
        if not self.text(3) == '':
            selectAllAction.setEnabled(True)
            clearAction.setEnabled(True)
        if self.parent.tabEditorWidget.isVisible():
            showEditorAction.setEnabled(False)
        menu.exec(self.mapToGlobal(e.pos()))

    def hideToolBar(self):
        tB = self.parent.toolBar
        tB.hide() if tB.isVisible() else tB.show()
        self.shell.setFocus()

    def showEditor(self):
        Ed = self.parent.splitterObj
        if not Ed.isVisible():
            Ed.show()
            self.parent.showEditorButton.setChecked(True)
        self.shell.setFocus()

    def copy(self):
        """Copy text to clipboard... or keyboard interrupt"""
        if self.hasSelectedText():
            text = self.selectedText()
            text = text.replace('>>> ', '').replace('... ', '').strip()  # removing prompts
            QApplication.clipboard().setText(text)
        else:
            raise KeyboardInterrupt

    def enteredSelected(self):
        cmd = self.selectedText()
        self.shell.insertFromDropPaste(cmd)
        self.shell.entered()

    def keyPressEvent(self, e):
        # empty text indicates possible shortcut key sequence so stay in output
        txt = e.text()
        if len(txt) and txt >= " ":
            self.shell.append(txt)
            self.shell.moveCursorToEnd()
            self.shell.setFocus()
            e.ignore()
        else:
            # possible shortcut key sequence, accept it
            e.accept()

    def widgetMessageBar(self, iface, text):
        timeout = iface.messageTimeout()
        self.infoBar.pushMessage(text, Qgis.MessageLevel.Info, timeout)
