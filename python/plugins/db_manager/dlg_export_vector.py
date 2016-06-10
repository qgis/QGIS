# -*- coding: utf-8 -*-

"""
/***************************************************************************
Name                 : DB Manager
Description          : Database manager plugin for QGIS
Date                 : Oct 13, 2011
copyright            : (C) 2011 by Giuseppe Sucameli
email                : brush.tyler@gmail.com

The content of this file is based on
- PG_Manager by Martin Dobias (GPLv2 license)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
"""

from qgis.PyQt.QtCore import Qt, QSettings, QFileInfo
from qgis.PyQt.QtWidgets import QDialog, QFileDialog, QMessageBox, QApplication
from qgis.PyQt.QtGui import QCursor

from qgis.core import QgsVectorFileWriter, QgsVectorDataProvider, QgsCoordinateReferenceSystem, QgsVectorLayerImport

from .ui.ui_DlgExportVector import Ui_DbManagerDlgExportVector as Ui_Dialog


class DlgExportVector(QDialog, Ui_Dialog):

    def __init__(self, inLayer, inDb, parent=None):
        QDialog.__init__(self, parent)
        self.inLayer = inLayer
        self.db = inDb
        self.setupUi(self)

        vectorFilterName = "lastVectorFileFilter"  # "lastRasterFileFilter"
        self.lastUsedVectorFilterSettingsKey = u"/UI/{0}".format(vectorFilterName)
        self.lastUsedVectorDirSettingsKey = u"/UI/{0}Dir".format(vectorFilterName)

        # update UI
        self.setupWorkingMode()
        self.populateFileFilters()
        self.populateEncodings()

    def setupWorkingMode(self):
        # set default values
        inCrs = self.inLayer.crs()
        srid = inCrs.postgisSrid() if inCrs.isValid() else 4236
        self.editSourceSrid.setText("%s" % srid)
        self.editTargetSrid.setText("%s" % srid)

        self.btnChooseOutputFile.clicked.connect(self.chooseOutputFile)
        self.checkSupports()

    def checkSupports(self):
        """ update options available for the current input layer """
        allowSpatial = self.db.connector.hasSpatialSupport()
        hasGeomType = self.inLayer and self.inLayer.hasGeometryType()
        self.chkSourceSrid.setEnabled(allowSpatial and hasGeomType)
        self.chkTargetSrid.setEnabled(allowSpatial and hasGeomType)
        # self.chkSpatialIndex.setEnabled(allowSpatial and hasGeomType)

    def chooseOutputFile(self):
        # get last used dir
        settings = QSettings()
        lastUsedDir = settings.value(self.lastUsedVectorDirSettingsKey, ".")

        # get selected filter
        selectedFilter = self.cboFileFormat.itemData(self.cboFileFormat.currentIndex())

        # ask for a filename
        filename = QFileDialog.getSaveFileName(self, self.tr("Choose where to save the file"), lastUsedDir,
                                               selectedFilter)
        if filename == "":
            return

        filterString = QgsVectorFileWriter.filterForDriver(selectedFilter)
        ext = filterString[filterString.find('.'):]
        ext = ext[:ext.find(' ')]

        if not filename.lower().endswith(ext):
            filename += ext

        # store the last used dir
        settings.setValue(self.lastUsedVectorDirSettingsKey, QFileInfo(filename).filePath())

        self.editOutputFile.setText(filename)

    def populateEncodings(self):
        # populate the combo with supported encodings
        self.cboEncoding.addItems(QgsVectorDataProvider.availableEncodings())

        # set the last used encoding
        enc = self.inLayer.dataProvider().encoding()
        idx = self.cboEncoding.findText(enc)
        if idx < 0:
            self.cboEncoding.insertItem(0, enc)
            idx = 0
        self.cboEncoding.setCurrentIndex(idx)

    def populateFileFilters(self):
        # populate the combo with supported vector file formats
        for name, filt in QgsVectorFileWriter.ogrDriverList().items():
            self.cboFileFormat.addItem(name, filt)

        # set the last used filter
        settings = QSettings()
        filt = settings.value(self.lastUsedVectorFilterSettingsKey, "ESRI Shapefile")

        idx = self.cboFileFormat.findText(filt)
        if idx < 0:
            idx = 0
        self.cboFileFormat.setCurrentIndex(idx)

    def accept(self):
        # sanity checks
        if self.editOutputFile.text() == "":
            QMessageBox.information(self, self.tr("Export to file"), self.tr("Output file name is required"))
            return

        if self.chkSourceSrid.isEnabled() and self.chkSourceSrid.isChecked():
            try:
                sourceSrid = int(self.editSourceSrid.text())
            except ValueError:
                QMessageBox.information(self, self.tr("Export to file"),
                                        self.tr("Invalid source srid: must be an integer"))
                return

        if self.chkTargetSrid.isEnabled() and self.chkTargetSrid.isChecked():
            try:
                targetSrid = int(self.editTargetSrid.text())
            except ValueError:
                QMessageBox.information(self, self.tr("Export to file"),
                                        self.tr("Invalid target srid: must be an integer"))
                return

        # override cursor
        QApplication.setOverrideCursor(QCursor(Qt.WaitCursor))
        # store current input layer crs, so I can restore it later
        prevInCrs = self.inLayer.crs()
        try:
            uri = self.editOutputFile.text()
            providerName = "ogr"

            options = {}

            # set the OGR driver will be used
            driverName = self.cboFileFormat.itemData(self.cboFileFormat.currentIndex())
            options['driverName'] = driverName

            # set the output file encoding
            if self.chkEncoding.isEnabled() and self.chkEncoding.isChecked():
                enc = self.cboEncoding.currentText()
                options['fileEncoding'] = enc

            if self.chkDropTable.isChecked():
                options['overwrite'] = True

            outCrs = None
            if self.chkTargetSrid.isEnabled() and self.chkTargetSrid.isChecked():
                targetSrid = int(self.editTargetSrid.text())
                outCrs = QgsCoordinateReferenceSystem(targetSrid)

            # update input layer crs
            if self.chkSourceSrid.isEnabled() and self.chkSourceSrid.isChecked():
                sourceSrid = int(self.editSourceSrid.text())
                inCrs = QgsCoordinateReferenceSystem(sourceSrid)
                self.inLayer.setCrs(inCrs)

            # do the export!
            ret, errMsg = QgsVectorLayerImport.importLayer(self.inLayer, uri, providerName, outCrs, False,
                                                           False, options)
        except Exception as e:
            ret = -1
            errMsg = unicode(e)

        finally:
            # restore input layer crs and encoding
            self.inLayer.setCrs(prevInCrs)
            # restore cursor
            QApplication.restoreOverrideCursor()

        if ret != 0:
            QMessageBox.warning(self, self.tr("Export to file"), self.tr("Error %d\n%s") % (ret, errMsg))
            return

        # create spatial index
        # if self.chkSpatialIndex.isEnabled() and self.chkSpatialIndex.isChecked():
        #       self.db.connector.createSpatialIndex( (schema, table), geom )

        QMessageBox.information(self, self.tr("Export to file"), self.tr("Export finished."))
        return QDialog.accept(self)


if __name__ == '__main__':
    import sys

    a = QApplication(sys.argv)
    dlg = DlgExportVector()
    dlg.show()
    sys.exit(a.exec_())
