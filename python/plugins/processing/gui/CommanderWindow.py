# -*- coding: utf-8 -*-

"""
***************************************************************************
    CommanderWindow.py
    ---------------------
    Date                 : April 2013
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
__date__ = 'April 2013'
__copyright__ = '(C) 2013, Victor Olaya'
# This will get replaced with a git SHA1 when you do a git archive
__revision__ = '$Format:%H$'

import types
import os
import imp
from PyQt4 import QtCore, QtGui
from PyQt4.QtCore import *
from PyQt4.QtGui import *
from qgis.utils import iface
from processing.core.Processing import Processing
from processing.gui.MissingDependencyDialog import MissingDependencyDialog
from processing.gui.ParametersDialog import ParametersDialog
from processing.tools import dataobjects
from processing.tools.system import *

ITEMHEIGHT = 30
OFFSET = 20
HEIGHT = 60


class CommanderWindow(QtGui.QDialog):
    def __init__(self, parent, canvas):
        self.canvas = canvas
        QtGui.QDialog.__init__(self, parent, Qt.FramelessWindowHint)
        self.commands = imp.load_source('commands', self.commandsFile())
        self.initGui()

    def commandsFolder(self):
        folder = unicode(os.path.join(userFolder(), 'commander'))
        mkdir(folder)
        return os.path.abspath(folder)

    def commandsFile(self):
        f = os.path.join(self.commandsFolder(), 'commands.py')
        if not os.path.exists(f):
            out = open(f, 'w')
            out.write('from qgis.core import *\n')
            out.write('import processing\n\n')
            out.write('def removeall():\n')
            out.write('\tmapreg = QgsMapLayerRegistry.instance()\n')
            out.write('\tmapreg.removeAllMapLayers()\n\n')
            out.write('def load(*args):\n')
            out.write('\tprocessing.load(args[0])\n')
            out.close()
        return f

    def algsListHasChanged(self):
        self.fillCombo()

    def initGui(self):
        self.combo = ExtendedComboBox()
        self.fillCombo()

        self.combo.setEditable(True)
        self.label = QtGui.QLabel('Enter command:')
        self.errorLabel = QtGui.QLabel('Enter command:')
        self.vlayout = QtGui.QVBoxLayout()
        self.vlayout.setSpacing(2)
        self.vlayout.setMargin(0)
        self.vlayout.addSpacerItem(QtGui.QSpacerItem(0, OFFSET,
                QSizePolicy.Maximum, QSizePolicy.Expanding))
        self.hlayout = QtGui.QHBoxLayout()
        self.hlayout.addWidget(self.label)
        self.vlayout.addLayout(self.hlayout)
        self.hlayout2 = QtGui.QHBoxLayout()
        self.hlayout2.addWidget(self.combo)
        self.vlayout.addLayout(self.hlayout2)
        self.vlayout.addSpacerItem(QtGui.QSpacerItem(0, OFFSET,
                QSizePolicy.Maximum, QSizePolicy.Expanding))
        self.setLayout(self.vlayout)
        self.combo.lineEdit().returnPressed.connect(self.run)
        self.prepareGui()

    def fillCombo(self):
        self.combo.clear()

        # Add algorithms
        for providerName in Processing.algs.keys():
            provider = Processing.algs[providerName]
            algs = provider.values()
            for alg in algs:
                self.combo.addItem('Processing algorithm: ' + alg.name)

        # Add functions
        for command in dir(self.commands):
            if isinstance(self.commands.__dict__.get(command),
                    types.FunctionType):
                self.combo.addItem('Command: ' + command)

        #Add menu entries
        menuActions = []
        actions = iface.mainWindow().menuBar().actions()
        for action in actions:
            menuActions.extend(self.getActions(action))
        for action in menuActions:
            self.combo.addItem('Menu action: ' + unicode(action.text()))

    def prepareGui(self):
        self.combo.setEditText('')
        self.combo.setMaximumSize(QtCore.QSize(
                self.canvas.rect().width() - 2 * OFFSET, ITEMHEIGHT))
        self.combo.view().setStyleSheet('min-height: 150px')
        self.combo.setFocus(Qt.OtherFocusReason)
        self.label.setMaximumSize(self.combo.maximumSize())
        self.label.setVisible(False)
        self.adjustSize()
        pt = self.canvas.rect().topLeft()
        absolutePt = self.canvas.mapToGlobal(pt)
        self.move(absolutePt)
        self.resize(self.canvas.rect().width(), HEIGHT)
        self.setStyleSheet('CommanderWindow {background-color: #e7f5fe; \
                            border: 1px solid #b9cfe4;}')

    def getActions(self, action):
        menuActions = []
        menu = action.menu()
        if menu is None:
            menuActions.append(action)
            return menuActions
        else:
            actions = menu.actions()
            for subaction in actions:
                if subaction.menu() is not None:
                    menuActions.extend(self.getActions(subaction))
                elif not subaction.isSeparator():
                    menuActions.append(subaction)

        return menuActions

    def run(self):
        s = unicode(self.combo.currentText())
        if s.startswith('Processing algorithm: '):
            algName = s[len('Processing algorithm: '):]
            alg = Processing.getAlgorithmFromFullName(algName)
            if alg is not None:
                self.close()
                self.runAlgorithm(alg)
        elif s.startswith("Command: "):
            command = s[len("Command: "):]
            try:
                self.runCommand(command)
                self.close()
            except Exception, e:
                self.label.setVisible(True)
                self.label.setText('Error:' + unicode(e))

        elif s.startswith('Menu action: '):
            actionName = s[len('Menu action: '):]
            menuActions = []
            actions = iface.mainWindow().menuBar().actions()
            for action in actions:
                menuActions.extend(self.getActions(action))
            for action in menuActions:
                if action.text() == actionName:
                    self.close()
                    action.trigger()
                    return
        else:
            try:
                self.runCommand(s)
                self.close()
            except Exception, e:
                self.label.setVisible(True)
                self.label.setText('Error:' + unicode(e))

    def runCommand(self, command):
        tokens = command.split(' ')
        if len(tokens) == 1:
            method = self.commands.__dict__.get(command)
            if method is not None:
                method()
            else:
                raise Exception('Wrong command')
        else:
            method = self.commands.__dict__.get(tokens[0])
            if method is not None:
                method(*tokens[1:])
            else:
                raise Exception('Wrong command')

    def runAlgorithm(self, alg):
        alg = alg.getCopy()
        message = alg.checkBeforeOpeningParametersDialog()
        if message:
            dlg = MissingDependencyDialog(message)
            dlg.exec_()
            return
        dlg = alg.getCustomParametersDialog()
        if not dlg:
            dlg = ParametersDialog(alg)
        canvas = iface.mapCanvas()
        prevMapTool = canvas.mapTool()
        dlg.show()
        dlg.exec_()
        if canvas.mapTool() != prevMapTool:
            try:
                canvas.mapTool().reset()
            except:
                pass
            canvas.setMapTool(prevMapTool)


class ExtendedComboBox(QComboBox):
    def __init__(self, parent=None):
        super(ExtendedComboBox, self).__init__(parent)

        self.setFocusPolicy(Qt.StrongFocus)
        self.setEditable(True)
        self.pFilterModel = QSortFilterProxyModel(self)
        self.pFilterModel.setFilterCaseSensitivity(Qt.CaseInsensitive)
        self.pFilterModel.setSourceModel(self.model())
        self.completer = QCompleter(self.pFilterModel, self)
        self.completer.setCompletionMode(QCompleter.UnfilteredPopupCompletion)
        self.completer.popup().setStyleSheet('min-height: 150px')
        self.completer.popup().setAlternatingRowColors(True)
        self.setCompleter(self.completer)
        self.lineEdit().textEdited[unicode].connect(
                self.pFilterModel.setFilterFixedString)
