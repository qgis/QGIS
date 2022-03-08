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

    static const inline QgsSettingsEntryString settingLastDestinationFolder = QgsSettingsEntryString( QStringLiteral( "last-destination-folder" ), QgsSettings::Prefix::APP_GEOREFERENCER, QString(), QObject::tr( "Last used folder for georeferencer destination files" ) );
    static const inline QgsSettingsEntryString settingLastPdfFolder = QgsSettingsEntryString( QStringLiteral( "last-pdf-folder" ), QgsSettings::Prefix::APP_GEOREFERENCER, QString(), QObject::tr( "Last used folder for georeferencer PDF report files" ) );

    QgsTransformSettingsDialog( QgsMapLayerType type, const QString &source, const QString &output, QWidget *parent = nullptr );

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

    /**
     * Returns the selected transform method.
     */
    QgsGcpTransformerInterface::TransformMethod transformMethod() const;

    /**
     * Sets the selected transform method.
     */
    void setTransformMethod( QgsGcpTransformerInterface::TransformMethod method );

    /**
     * Returns the selected resampling method.
     */
    QgsImageWarper::ResamplingMethod resamplingMethod() const;

    /**
     * Sets the selected resampling method.
     */
    void setResamplingMethod( QgsImageWarper::ResamplingMethod method );

    /**
     * Returns the selected compression method.
     */
    QString compressionMethod() const;

    /**
     * Sets the selected compression \a method.
     */
    void setCompressionMethod( const QString &method );

    /**
     * Returns the destination filename.
     */
    QString destinationFilename() const;

    /**
     * Returns the filename for the PDF report.
     */
    QString pdfReportFilename() const;

    /**
     * Sets the \a filename for the PDF report.
     */
    void setPdfReportFilename( const QString &filename );

    /**
     * Returns the filename for the PDF map.
     */
    QString pdfMapFilename() const;

    /**
     * Sets the \a filename for the PDF map.
     */
    void setPdfMapFilename( const QString &filename );

    /**
     * Returns TRUE if GCP points should be automatically saved.
     */
    bool saveGcpPoints() const;

    /**
     * Sets whether GCP points should be automatically saved.
     */
    void setSaveGcpPoints( bool save );

    /**
     * Returns TRUE if the use zero for transparent option is checked.
     */
    bool useZeroForTransparent() const;

    /**
     * Sets whether the use zero for transparent option is checked.
     */
    void setUseZeroForTransparent( bool enabled );

    /**
     * Returns TRUE if the load result in project option is checked.
     */
    bool loadInProject() const;

    /**
     * Sets whether the load result in project option is checked.
     */
    void setLoadInProject( bool enabled );

    /**
     * Retrieves the output resolution set in the dialog.
     */
    void outputResolution( double &resX, double &resY );

    /**
     * Sets the output resolution shown in the dialog.
     */
    void setOutputResolution( double resX, double resY );

  protected:
    void accept() override;

  private slots:
    void cmbTransformType_currentIndexChanged( const QString &text );
    void mWorldFileCheckBox_stateChanged( int state );
    void showHelp();

  private:
    QString generateModifiedFileName( const QString &filename );

    QgsMapLayerType mType = QgsMapLayerType::RasterLayer;
    QString mSourceFile;
};

#endif // QGSTRANSFORMSETTINGSDIALOG_H
