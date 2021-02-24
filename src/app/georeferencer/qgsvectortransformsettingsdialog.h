/***************************************************************************
     QgsVectorTransformSettingsDialog.h
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

#ifndef QGSVECTORTRANSFORMSETTINGSDIALOG_H
#define QGSVECTORTRANSFORMSETTINGSDIALOG_H

#include <QDialog>

#include "qgsgeorefmainwindow.h"

#include "ui_qgsvectortransformsettingsdialogbase.h"

class QgsVectorTransformSettingsDialog : public QDialog, private Ui::QgsVectorTransformSettingsDialog
{
    Q_OBJECT

  public:
    QgsVectorTransformSettingsDialog( const QString &input, const QString &output,
                                      int countGCPpoints, const QString &outputFilters,
                                      QWidget *parent = nullptr );

    void getTransformSettings( QgsGeorefTransform::TransformMethod &tp,
                               QString &output, QgsCoordinateReferenceSystem &proj, QString &pdfMapFile,
                               QString &pdfReportFile, QString &gcpPoints, bool &loadInQgis );
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
    QString generateModifiedOutputFileName( const QString &output );

    QString mSourceFile;

    int mCountGCPpoints;

};

#endif // QGSVECTORTRANSFORMSETTINGSDIALOG_H
