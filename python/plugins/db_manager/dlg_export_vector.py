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

from PyQt4.QtCore import Qt, QObject, SIGNAL, QSettings, QFileInfo
from PyQt4.QtGui import QDialog, QFileDialog, QMessageBox, QApplication, QCursor

import qgis.core

from .ui.ui_DlgExportVector import Ui_DbManagerDlgExportVector as Ui_Dialog


class DlgExportVector(QDialog, Ui_Dialog):
    def __init__(self, inLayer, inDb, parent=None):
        QDialog.__init__(self, parent)
        self.inLayer = inLayer
        self.db = inDb
        self.setupUi(self)

        # update UI
        self.setupWorkingMode()
        self.populateEncodings()

    def setupWorkingMode(self):
        # set default values
        inCrs = self.inLayer.crs()
        srid = inCrs.postgisSrid() if inCrs.isValid() else 4236
        self.editSourceSrid.setText("%s" % srid)
        self.editTargetSrid.setText("%s" % srid)

        QObject.connect(self.btnChooseOutputFile, SIGNAL("clicked()"), self.chooseOutputFile)
        self.checkSupports()

    def checkSupports(self):
        """ update options available for the current input layer """
        allowSpatial = self.db.connector.hasSpatialSupport()
        hasGeomType = self.inLayer and self.inLayer.hasGeometryType()
        self.chkSourceSrid.setEnabled(allowSpatial and hasGeomType)
        self.chkTargetSrid.setEnabled(allowSpatial and hasGeomType)
        # self.chkSpatialIndex.setEnabled(allowSpatial and hasGeomType)


    def chooseOutputFile(self):
        # get last used dir and format
        settings = QSettings()
        lastDir = settings.value("/db_manager/lastUsedDir", "")
        # ask for a filename
        filename = QFileDialog.getSaveFileName(self, self.tr("Choose where to save the file"), lastDir,
                                               self.tr("Shapefiles") + " (*.shp)")
        if filename == "":
            return
        if filename[-4:] != ".shp":
            filename += ".shp"
        # store the last used dir and format
        settings.setValue("/db_manager/lastUsedDir", QFileInfo(filename).filePath())

        self.editOutputFile.setText(filename)

    def populateEncodings(self):
        # populate the combo with supported encodings
        self.cboEncoding.addItems(qgis.core.QgsVectorDataProvider.availableEncodings())

        # set the last used encoding
        enc = self.inLayer.dataProvider().encoding()
        idx = self.cboEncoding.findText(enc)
        if idx < 0:
            self.cboEncoding.insertItem(0, enc)
            idx = 0
        self.cboEncoding.setCurrentIndex(idx)

    def accept(self):
        # sanity checks
        if self.editOutputFile.text() == "":
            QMessageBox.information(self, self.tr("Export to file"), self.tr("Output table name is required"))
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
            driverName = "ESRI Shapefile"

            options = {}

            # set the OGR driver will be used
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
                outCrs = qgis.core.QgsCoordinateReferenceSystem(targetSrid)

            # update input layer crs
            if self.chkSourceSrid.isEnabled() and self.chkSourceSrid.isChecked():
                sourceSrid = int(self.editSourceSrid.text())
                inCrs = qgis.core.QgsCoordinateReferenceSystem(sourceSrid)
                self.inLayer.setCrs(inCrs)

            # do the export!
            ret, errMsg = qgis.core.QgsVectorLayerImport.importLayer(self.inLayer, uri, providerName, outCrs, False,
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
