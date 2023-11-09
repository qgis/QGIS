/***************************************************************************
                          qgsrasterformatsaveoptionswidget.h
                             -------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERFORMATSAVEOPTIONSWIDGET_H
#define QGSRASTERFORMATSAVEOPTIONSWIDGET_H

#include "ui_qgsrasterformatsaveoptionswidgetbase.h"
#include "qgsraster.h"
#include "qgis_gui.h"

class QgsRasterLayer;

/**
 * \ingroup gui
 * \brief A widget to select format-specific raster saving options
 */
class GUI_EXPORT QgsRasterFormatSaveOptionsWidget: public QWidget, private Ui::QgsRasterFormatSaveOptionsWidgetBase
{
    Q_OBJECT

  public:

    enum Type
    {
      Default, // everything except profile buttons (save as dlg)
      Full, // everything (options dlg)
      Table, // just table
      LineEdit, // just the line edit
      ProfileLineEdit // Profile + LineEdit
    };

    QgsRasterFormatSaveOptionsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                      const QString &format = "GTiff",
                                      QgsRasterFormatSaveOptionsWidget::Type type = Default,
                                      const QString &provider = "gdal" );

    /**
     * Set output raster format, it is used to determine list
     * of available options
     */
    void setFormat( const QString &format );

    /**
     * Set provider key, , it is used to determine list
     * of available options
     */
    void setProvider( const QString &provider );

    /**
     * Set output raster layer
     */
    void setRasterLayer( QgsRasterLayer *rasterLayer ) { mRasterLayer = rasterLayer; mRasterFileName = QString(); }

    /**
     * Set output raster file name
     */
    void setRasterFileName( const QString &file ) { mRasterLayer = nullptr; mRasterFileName = file; }

    /**
     * Returns list of selected options
     * \see setOptions()
     */
    QStringList options() const;

    /**
     * Populate widget with user-defined options. String should contain
     * key=value pairs separated by spaces, e.g. "TILED=YES TFW=YES"
     * \see options()
     * \since QGIS 3.0
     */
    void setOptions( const QString &options );

    /**
     * Set widget look and feel
     */
    void setType( QgsRasterFormatSaveOptionsWidget::Type type = Default );

    /**
     * Set pyramids format to use
     */
    void setPyramidsFormat( Qgis::RasterPyramidFormat format )
    {
      mPyramids = true;
      mPyramidsFormat = format;
    }

  public slots:

    void apply();

    /**
     * Opens window with options description for given provider
     * and output format
     */
    void helpOptions();

    /**
     * Validates options correctness
     */
    QString validateOptions( bool gui = true, bool reportOk = true );

    /**
     * Reloads profiles list from QGIS settings
     */
    void updateProfiles();

  private slots:

    void mProfileNewButton_clicked() SIP_FORCE;
    void mProfileDeleteButton_clicked() SIP_FORCE;
    void mProfileResetButton_clicked() SIP_FORCE;
    void mOptionsAddButton_clicked() SIP_FORCE;
    void mOptionsDeleteButton_clicked() SIP_FORCE;
    void mOptionsLineEdit_editingFinished() SIP_FORCE;
    void optionsTableChanged() SIP_FORCE;
    void optionsTableEnableDeleteButton() SIP_FORCE;
    void updateOptions() SIP_FORCE;
    void swapOptionsUI( int newIndex = -1 ) SIP_FORCE;
    void updateControls() SIP_FORCE;

  protected:
    void showEvent( QShowEvent *event ) override;

  signals:
    void optionsChanged();

  private:

    QString mFormat;
    QString mProvider;
    QgsRasterLayer *mRasterLayer = nullptr;
    QString mRasterFileName;
    QMap< QString, QString> mOptionsMap;
    static QMap< QString, QStringList > sBuiltinProfiles;
    bool mPyramids = false;
    Qgis::RasterPyramidFormat mPyramidsFormat = Qgis::RasterPyramidFormat::GeoTiff;
    int mBlockOptionUpdates = 0;

    QString settingsKey( QString profile ) const SIP_FORCE;
    QString currentProfileKey() const SIP_FORCE;
    QString createOptions( const QString &profile ) const SIP_FORCE;
    void deleteCreateOptions( const QString &profile ) SIP_FORCE;
    void setCreateOptions() SIP_FORCE;
    void setCreateOptions( const QString &profile, const QString &options ) SIP_FORCE;
    void setCreateOptions( const QString &profile, const QStringList &list ) SIP_FORCE;
    QStringList profiles() const SIP_FORCE;
    bool eventFilter( QObject *obj, QEvent *event ) override SIP_FORCE;
    QString pseudoFormat() const SIP_FORCE;

};

// clazy:excludeall=qstring-allocations

#endif // QGSRASTERLAYERSAVEASDIALOG_H
