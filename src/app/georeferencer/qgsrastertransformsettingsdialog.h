/***************************************************************************
     qgsrastertransformsettingsdialog.h
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

#ifndef QGSRASTERTRANSFORMSETTINGSDIALOG_H
#define QGSRASTERTRANSFORMSETTINGSDIALOG_H

#include <QDialog>

#include "qgsgeorefmainwindow.h"

#include "ui_qgsrastertransformsettingsdialogbase.h"

class QgsRasterTransformSettingsDialog : public QDialog, private Ui::QgsRasterTransformSettingsDialog
{
    Q_OBJECT

  public:
    QgsRasterTransformSettingsDialog( const QString &raster, const QString &output,
                                int countGCPpoints, QWidget *parent = nullptr );

    void getTransformSettings( QgsGeorefTransform::TransformMethod &tp,
                               QgsImageWarper::ResamplingMethod &rm, QString &comprMethod,
                               QString &raster, QgsCoordinateReferenceSystem &proj, QString &pdfMapFile, QString &pdfReportFile, QString &gcpPoints, bool &zt, bool &loadInQgis,
                               double &resX, double &resY );
    static void resetSettings();

  protected:
    void changeEvent( QEvent *e ) override;
    void accept() override;

  private slots:
    void cmbTransformType_currentIndexChanged( const QString &text );
    void mWorldFileCheckBox_stateChanged( int state );
    QIcon getThemeIcon( const QString &name );
    void showHelp();

  private:
    bool checkGCPpoints( int count, int &minGCPpoints );
    QString generateModifiedRasterFileName( const QString &raster );

    QString mSourceRasterFile;

    int mCountGCPpoints;

    QStringList mListCompression;
};

#endif // QGSRASTERTRANSFORMSETTINGSDIALOG_H
