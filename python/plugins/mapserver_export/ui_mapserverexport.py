# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'qgsmapserverexportbase.ui'
#
# Created: Thu Jan 17 14:02:57 2008
#      by: PyQt4 UI code generator 4.3.3
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

class Ui_QgsMapserverExportBase(object):
    def setupUi(self, QgsMapserverExportBase):
        QgsMapserverExportBase.setObjectName("QgsMapserverExportBase")
        QgsMapserverExportBase.resize(QtCore.QSize(QtCore.QRect(0,0,416,480).size()).expandedTo(QgsMapserverExportBase.minimumSizeHint()))
        QgsMapserverExportBase.setSizeGripEnabled(True)

        self.gridlayout = QtGui.QGridLayout(QgsMapserverExportBase)
        self.gridlayout.setMargin(9)
        self.gridlayout.setSpacing(6)
        self.gridlayout.setObjectName("gridlayout")

        self.grpWeb = QtGui.QGroupBox(QgsMapserverExportBase)
        self.grpWeb.setObjectName("grpWeb")

        self.gridlayout1 = QtGui.QGridLayout(self.grpWeb)
        self.gridlayout1.setMargin(0)
        self.gridlayout1.setSpacing(0)
        self.gridlayout1.setObjectName("gridlayout1")

        self.hboxlayout = QtGui.QHBoxLayout()
        self.hboxlayout.setSpacing(6)
        self.hboxlayout.setMargin(4)
        self.hboxlayout.setObjectName("hboxlayout")

        self.textLabel5_2 = QtGui.QLabel(self.grpWeb)
        self.textLabel5_2.setMinimumSize(QtCore.QSize(60,0))
        self.textLabel5_2.setObjectName("textLabel5_2")
        self.hboxlayout.addWidget(self.textLabel5_2)

        self.txtWebTemplate = QtGui.QLineEdit(self.grpWeb)
        self.txtWebTemplate.setObjectName("txtWebTemplate")
        self.hboxlayout.addWidget(self.txtWebTemplate)

        self.btnChooseTemplateFile = QtGui.QPushButton(self.grpWeb)
        self.btnChooseTemplateFile.setObjectName("btnChooseTemplateFile")
        self.hboxlayout.addWidget(self.btnChooseTemplateFile)
        self.gridlayout1.addLayout(self.hboxlayout,0,0,1,1)

        self.hboxlayout1 = QtGui.QHBoxLayout()
        self.hboxlayout1.setSpacing(6)
        self.hboxlayout1.setMargin(4)
        self.hboxlayout1.setObjectName("hboxlayout1")

        self.textLabel5 = QtGui.QLabel(self.grpWeb)
        self.textLabel5.setMinimumSize(QtCore.QSize(60,0))
        self.textLabel5.setObjectName("textLabel5")
        self.hboxlayout1.addWidget(self.textLabel5)

        self.txtWebHeader = QtGui.QLineEdit(self.grpWeb)
        self.txtWebHeader.setObjectName("txtWebHeader")
        self.hboxlayout1.addWidget(self.txtWebHeader)

        self.btnChooseHeaderFile = QtGui.QPushButton(self.grpWeb)
        self.btnChooseHeaderFile.setObjectName("btnChooseHeaderFile")
        self.hboxlayout1.addWidget(self.btnChooseHeaderFile)
        self.gridlayout1.addLayout(self.hboxlayout1,1,0,1,1)

        self.hboxlayout2 = QtGui.QHBoxLayout()
        self.hboxlayout2.setSpacing(6)
        self.hboxlayout2.setMargin(4)
        self.hboxlayout2.setObjectName("hboxlayout2")

        self.textLabel5_3 = QtGui.QLabel(self.grpWeb)
        self.textLabel5_3.setMinimumSize(QtCore.QSize(60,0))
        self.textLabel5_3.setObjectName("textLabel5_3")
        self.hboxlayout2.addWidget(self.textLabel5_3)

        self.txtWebFooter = QtGui.QLineEdit(self.grpWeb)
        self.txtWebFooter.setObjectName("txtWebFooter")
        self.hboxlayout2.addWidget(self.txtWebFooter)

        self.btnChooseFooterFile = QtGui.QPushButton(self.grpWeb)
        self.btnChooseFooterFile.setObjectName("btnChooseFooterFile")
        self.hboxlayout2.addWidget(self.btnChooseFooterFile)
        self.gridlayout1.addLayout(self.hboxlayout2,2,0,1,1)
        self.gridlayout.addWidget(self.grpWeb,4,0,1,3)

        self.grpMap = QtGui.QGroupBox(QgsMapserverExportBase)
        self.grpMap.setObjectName("grpMap")

        self.gridlayout2 = QtGui.QGridLayout(self.grpMap)
        self.gridlayout2.setMargin(0)
        self.gridlayout2.setSpacing(0)
        self.gridlayout2.setObjectName("gridlayout2")

        self.hboxlayout3 = QtGui.QHBoxLayout()
        self.hboxlayout3.setSpacing(6)
        self.hboxlayout3.setMargin(4)
        self.hboxlayout3.setObjectName("hboxlayout3")

        self.textLabel3 = QtGui.QLabel(self.grpMap)
        self.textLabel3.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.textLabel3.setObjectName("textLabel3")
        self.hboxlayout3.addWidget(self.textLabel3)

        self.cmbMapUnits = QtGui.QComboBox(self.grpMap)
        self.cmbMapUnits.setObjectName("cmbMapUnits")
        self.hboxlayout3.addWidget(self.cmbMapUnits)

        self.textLabel4 = QtGui.QLabel(self.grpMap)
        self.textLabel4.setAlignment(QtCore.Qt.AlignRight|QtCore.Qt.AlignTrailing|QtCore.Qt.AlignVCenter)
        self.textLabel4.setObjectName("textLabel4")
        self.hboxlayout3.addWidget(self.textLabel4)

        self.cmbMapImageType = QtGui.QComboBox(self.grpMap)
        self.cmbMapImageType.setObjectName("cmbMapImageType")
        self.hboxlayout3.addWidget(self.cmbMapImageType)
        self.gridlayout2.addLayout(self.hboxlayout3,2,0,1,1)

        self.hboxlayout4 = QtGui.QHBoxLayout()
        self.hboxlayout4.setSpacing(6)
        self.hboxlayout4.setMargin(4)
        self.hboxlayout4.setObjectName("hboxlayout4")

        self.textLabel2 = QtGui.QLabel(self.grpMap)
        self.textLabel2.setObjectName("textLabel2")
        self.hboxlayout4.addWidget(self.textLabel2)

        self.txtMapWidth = QtGui.QLineEdit(self.grpMap)
        self.txtMapWidth.setObjectName("txtMapWidth")
        self.hboxlayout4.addWidget(self.txtMapWidth)

        self.textLabel2_2 = QtGui.QLabel(self.grpMap)
        self.textLabel2_2.setObjectName("textLabel2_2")
        self.hboxlayout4.addWidget(self.textLabel2_2)

        self.txtMapHeight = QtGui.QLineEdit(self.grpMap)
        self.txtMapHeight.setObjectName("txtMapHeight")
        self.hboxlayout4.addWidget(self.txtMapHeight)
        self.gridlayout2.addLayout(self.hboxlayout4,1,0,1,1)

        self.hboxlayout5 = QtGui.QHBoxLayout()
        self.hboxlayout5.setSpacing(6)
        self.hboxlayout5.setMargin(4)
        self.hboxlayout5.setObjectName("hboxlayout5")

        self.textLabel1 = QtGui.QLabel(self.grpMap)
        self.textLabel1.setObjectName("textLabel1")
        self.hboxlayout5.addWidget(self.textLabel1)

        self.txtMapName = QtGui.QLineEdit(self.grpMap)
        self.txtMapName.setObjectName("txtMapName")
        self.hboxlayout5.addWidget(self.txtMapName)
        self.gridlayout2.addLayout(self.hboxlayout5,0,0,1,1)
        self.gridlayout.addWidget(self.grpMap,3,0,1,3)

        self.textLabel7 = QtGui.QLabel(QgsMapserverExportBase)
        self.textLabel7.setObjectName("textLabel7")
        self.gridlayout.addWidget(self.textLabel7,0,0,1,1)

        self.txtMapFilePath = QtGui.QLineEdit(QgsMapserverExportBase)
        self.txtMapFilePath.setObjectName("txtMapFilePath")
        self.gridlayout.addWidget(self.txtMapFilePath,0,1,1,1)

        self.btnChooseProjectFile = QtGui.QPushButton(QgsMapserverExportBase)
        self.btnChooseProjectFile.setObjectName("btnChooseProjectFile")
        self.gridlayout.addWidget(self.btnChooseProjectFile,1,2,1,1)

        self.txtQgisFilePath = QtGui.QLineEdit(QgsMapserverExportBase)
        self.txtQgisFilePath.setObjectName("txtQgisFilePath")
        self.gridlayout.addWidget(self.txtQgisFilePath,1,1,1,1)

        self.textLabel7_2 = QtGui.QLabel(QgsMapserverExportBase)
        self.textLabel7_2.setObjectName("textLabel7_2")
        self.gridlayout.addWidget(self.textLabel7_2,1,0,1,1)

        self.btnChooseFile = QtGui.QPushButton(QgsMapserverExportBase)
        self.btnChooseFile.setObjectName("btnChooseFile")
        self.gridlayout.addWidget(self.btnChooseFile,0,2,1,1)

        self.chkExpLayersOnly = QtGui.QCheckBox(QgsMapserverExportBase)
        self.chkExpLayersOnly.setObjectName("chkExpLayersOnly")
        self.gridlayout.addWidget(self.chkExpLayersOnly,2,1,1,2)

        self.buttonBox = QtGui.QDialogButtonBox(QgsMapserverExportBase)
        self.buttonBox.setStandardButtons(QtGui.QDialogButtonBox.Cancel|QtGui.QDialogButtonBox.Help|QtGui.QDialogButtonBox.NoButton|QtGui.QDialogButtonBox.Ok)
        self.buttonBox.setObjectName("buttonBox")
        self.gridlayout.addWidget(self.buttonBox,5,0,1,3)
        self.textLabel5_2.setBuddy(self.txtWebTemplate)
        self.textLabel5.setBuddy(self.txtWebHeader)
        self.textLabel5_3.setBuddy(self.txtWebFooter)
        self.textLabel3.setBuddy(self.cmbMapUnits)
        self.textLabel4.setBuddy(self.cmbMapImageType)
        self.textLabel2.setBuddy(self.txtMapWidth)
        self.textLabel2_2.setBuddy(self.txtMapHeight)
        self.textLabel1.setBuddy(self.txtMapName)
        self.textLabel7.setBuddy(self.txtMapFilePath)
        self.textLabel7_2.setBuddy(self.txtQgisFilePath)

        self.retranslateUi(QgsMapserverExportBase)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("accepted()"),QgsMapserverExportBase.accept)
        QtCore.QObject.connect(self.buttonBox,QtCore.SIGNAL("rejected()"),QgsMapserverExportBase.reject)
        QtCore.QMetaObject.connectSlotsByName(QgsMapserverExportBase)
        QgsMapserverExportBase.setTabOrder(self.txtMapFilePath,self.btnChooseFile)
        QgsMapserverExportBase.setTabOrder(self.btnChooseFile,self.txtQgisFilePath)
        QgsMapserverExportBase.setTabOrder(self.txtQgisFilePath,self.btnChooseProjectFile)
        QgsMapserverExportBase.setTabOrder(self.btnChooseProjectFile,self.chkExpLayersOnly)
        QgsMapserverExportBase.setTabOrder(self.chkExpLayersOnly,self.txtMapName)
        QgsMapserverExportBase.setTabOrder(self.txtMapName,self.txtMapWidth)
        QgsMapserverExportBase.setTabOrder(self.txtMapWidth,self.txtMapHeight)
        QgsMapserverExportBase.setTabOrder(self.txtMapHeight,self.cmbMapUnits)
        QgsMapserverExportBase.setTabOrder(self.cmbMapUnits,self.cmbMapImageType)
        QgsMapserverExportBase.setTabOrder(self.cmbMapImageType,self.txtWebTemplate)
        QgsMapserverExportBase.setTabOrder(self.txtWebTemplate,self.btnChooseTemplateFile)
        QgsMapserverExportBase.setTabOrder(self.btnChooseTemplateFile,self.txtWebHeader)
        QgsMapserverExportBase.setTabOrder(self.txtWebHeader,self.btnChooseHeaderFile)
        QgsMapserverExportBase.setTabOrder(self.btnChooseHeaderFile,self.txtWebFooter)
        QgsMapserverExportBase.setTabOrder(self.txtWebFooter,self.btnChooseFooterFile)

    def retranslateUi(self, QgsMapserverExportBase):
        QgsMapserverExportBase.setWindowTitle(QtGui.QApplication.translate("QgsMapserverExportBase", "Export to Mapserver", None, QtGui.QApplication.UnicodeUTF8))
        self.grpWeb.setTitle(QtGui.QApplication.translate("QgsMapserverExportBase", "Web Interface Definition", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel5_2.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Template", None, QtGui.QApplication.UnicodeUTF8))
        self.txtWebTemplate.setToolTip(QtGui.QApplication.translate("QgsMapserverExportBase", "Path to the MapServer template file", None, QtGui.QApplication.UnicodeUTF8))
        self.btnChooseTemplateFile.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel5.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Header", None, QtGui.QApplication.UnicodeUTF8))
        self.btnChooseHeaderFile.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel5_3.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Footer", None, QtGui.QApplication.UnicodeUTF8))
        self.btnChooseFooterFile.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.grpMap.setTitle(QtGui.QApplication.translate("QgsMapserverExportBase", "Map", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel3.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Units", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel4.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Image type", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "gif", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "gtiff", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "jpeg", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "png", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "swf", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "userdefined", None, QtGui.QApplication.UnicodeUTF8))
        self.cmbMapImageType.addItem(QtGui.QApplication.translate("QgsMapserverExportBase", "wbmp", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel2.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Width", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel2_2.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Height", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel1.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Name", None, QtGui.QApplication.UnicodeUTF8))
        self.txtMapName.setToolTip(QtGui.QApplication.translate("QgsMapserverExportBase", "Prefix attached to map, scalebar and legend GIF filenames created using this MapFile", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel7.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Map file", None, QtGui.QApplication.UnicodeUTF8))
        self.txtMapFilePath.setToolTip(QtGui.QApplication.translate("QgsMapserverExportBase", "Name for the map file to be created from the QGIS project file", None, QtGui.QApplication.UnicodeUTF8))
        self.btnChooseProjectFile.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Browse...", None, QtGui.QApplication.UnicodeUTF8))
        self.txtQgisFilePath.setToolTip(QtGui.QApplication.translate("QgsMapserverExportBase", "Full path to the QGIS project file to export to MapServer map format", None, QtGui.QApplication.UnicodeUTF8))
        self.textLabel7_2.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "QGIS project file", None, QtGui.QApplication.UnicodeUTF8))
        self.btnChooseFile.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Save As...", None, QtGui.QApplication.UnicodeUTF8))
        self.chkExpLayersOnly.setToolTip(QtGui.QApplication.translate("QgsMapserverExportBase", "If checked, only the layer information will be processed", None, QtGui.QApplication.UnicodeUTF8))
        self.chkExpLayersOnly.setText(QtGui.QApplication.translate("QgsMapserverExportBase", "Export LAYER information only", None, QtGui.QApplication.UnicodeUTF8))

