/***************************************************************************
     qgstransformsettingsdialog.h
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTRANSFORMSETTINGSDIALOG_H
#define QGSTRANSFORMSETTINGSDIALOG_H

#include <QDialog>

#include "qgsgeorefmainwindow.h"

#include "ui_qgstransformsettingsdialogbase.h"

class QgsTransformSettingsDialog : public QDialog, private Ui::QgsTransformSettingsDialog
{
    Q_OBJECT

  public:
    QgsTransformSettingsDialog( const QString &raster, const QString &output, QWidget *parent = nullptr );

    /**
     * Sets the selected target \a crs.
     */
    void setTargetCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the selected target CRS.
     */
    QgsCoordinateReferenceSystem targetCrs() const;

    /**
     * Returns TRUE if the create world file only option is set.
     */
    bool createWorldFileOnly() const;

    /**
     * Sets whether the create world file only option should be set.
     */
    void setCreateWorldFileOnly( bool enabled );

    void getTransformSettings( QgsGeorefTransform::TransformMethod &tp,
                               QgsImageWarper::ResamplingMethod &rm, QString &comprMethod,
                               QString &raster, QString &pdfMapFile, QString &pdfReportFile, bool &saveGcpPoints, bool &zt, bool &loadInQgis,
                               double &resX, double &resY );

  protected:
    void accept() override;

  private slots:
    void cmbTransformType_currentIndexChanged( const QString &text );
    void mWorldFileCheckBox_stateChanged( int state );
    void showHelp();

  private:
    QString generateModifiedRasterFileName( const QString &raster );

    QString mSourceRasterFile;
};

#endif // QGSTRANSFORMSETTINGSDIALOG_H
