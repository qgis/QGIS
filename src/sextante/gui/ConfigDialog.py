from PyQt4.QtCore import *
from PyQt4.QtGui import *
from PyQt4 import QtCore, QtGui
from sextante.core.SextanteConfig import SextanteConfig


class ConfigDialog(QtGui.QDialog):
    def __init__(self, toolbox):
        QtGui.QDialog.__init__(self)
        self.toolbox = toolbox
        self.setupUi()

    def setupUi(self):
        self.resize(700, 500)
        self.setWindowTitle("SEXTANTE options")
        self.verticalLayout = QtGui.QVBoxLayout()
        self.verticalLayout.setSpacing(2)
        self.verticalLayout.setMargin(0)
        self.searchBox = QtGui.QLineEdit()
        self.searchBox.textChanged.connect(self.fillTree)
        self.verticalLayout.addWidget(self.searchBox)
        self.groupIcon = QtGui.QIcon()
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirClosedIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.Off)
        self.groupIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_DirOpenIcon),
                QtGui.QIcon.Normal, QtGui.QIcon.On)
        self.keyIcon = QtGui.QIcon()
        self.keyIcon.addPixmap(self.style().standardPixmap(QtGui.QStyle.SP_FileIcon))
        self.tree = QtGui.QTreeWidget()
        self.tree.setHeaderLabels(("Setting", "Value"))
        self.tree.header().setResizeMode(0, QtGui.QHeaderView.Stretch)
        self.tree.header().setResizeMode(1, QtGui.QHeaderView.Stretch)
        self.fillTree()
        self.verticalLayout.addWidget(self.tree)
        self.horizontalLayout = QtGui.QHBoxLayout()
        self.horizontalLayout.setSpacing(2)
        self.horizontalLayout.setMargin(0)
        self.buttonBox = QtGui.QDialogButtonBox()
        self.buttonBox.setOrientation(QtCore.Qt.Horizontal)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Ok)
        self.horizontalLayout.addSpacing(100)
        self.horizontalLayout.addWidget(self.buttonBox)
        self.verticalLayout.addLayout(self.horizontalLayout)
        self.setLayout(self.verticalLayout)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("accepted()"), self.okPressed)
        QtCore.QObject.connect(self.buttonBox, QtCore.SIGNAL("rejected()"), self.cancelPressed)
        QtCore.QMetaObject.connectSlotsByName(self)


    def fillTree(self):
        self.items = {}
        self.tree.clear()
        text = str(self.searchBox.text())
        settings = SextanteConfig.getSettings()
        for group in settings.keys():
            groupItem = QtGui.QTreeWidgetItem()
            groupItem.setText(0,group)
            icon = SextanteConfig.getGroupIcon(group)
            groupItem.setIcon(0, icon)
            #groupItem.setIcon(0,self.groupIcon)
            for setting in settings[group]:
                if setting.hidden:
                    continue
                if text =="" or text.lower() in setting.description.lower():
                    settingItem = TreeSettingItem(setting, icon)
                    self.items[setting]=settingItem
                    groupItem.addChild(settingItem)
            self.tree.addTopLevelItem(groupItem)
            if text != "":
                groupItem.setExpanded(True)
        self.tree.sortItems(0, Qt.AscendingOrder)

    def okPressed(self):
        for setting in self.items.keys():
            if isinstance(setting.value,bool):
                setting.value = (self.items[setting].checkState(1) == QtCore.Qt.Checked)
            elif isinstance(setting.value, (float,int, long)):
                value = str(self.items[setting].text(1))
                try:
                    value = float(value)
                    setting.value = value
                except ValueError:
                    QtGui.QMessageBox.critical(self, "Wrong value","Wrong parameter value:\n" + value)
                    return
            else:
                setting.value = str(self.items[setting].text(1))
            SextanteConfig.addSetting(setting)
        SextanteConfig.saveSettings()
        self.toolbox.updateTree()
        self.close()


    def cancelPressed(self):
        self.close()


class TreeSettingItem(QtGui.QTreeWidgetItem):

    def __init__(self, setting, icon):
        QTreeWidgetItem.__init__(self)
        self.setting = setting
        self.setText(0, setting.description)
        self.setFlags(self.flags() | QtCore.Qt.ItemIsEditable)
        if isinstance(setting.value,bool):
            if setting.value:
                self.setCheckState(1, QtCore.Qt.Checked)
            else:
                self.setCheckState(1, QtCore.Qt.Unchecked)
        else:
            self.setText(1, str(setting.value))
        self.setIcon(0, icon)
