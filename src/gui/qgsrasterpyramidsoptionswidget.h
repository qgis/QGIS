/***************************************************************************
                          qgsrasterpyramidsoptionswidget.h
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

#ifndef QGSRASTERPYRAMIDSOPTIONSWIDGET_H
#define QGSRASTERPYRAMIDSOPTIONSWIDGET_H

#include "ui_qgsrasterpyramidsoptionswidgetbase.h"
#include "qgis.h"
#include "qgis_gui.h"

class QCheckBox;

/**
 * \ingroup gui
 * A widget to select format-specific raster saving options
 */
class GUI_EXPORT QgsRasterPyramidsOptionsWidget: public QWidget, private Ui::QgsRasterPyramidsOptionsWidgetBase
{
    Q_OBJECT

  public:

    //! Constructor for QgsRasterPyramidsOptionsWidget
    QgsRasterPyramidsOptionsWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &provider = "gdal" );

    QStringList configOptions() const { return mSaveOptionsWidget->options(); }
    QgsRasterFormatSaveOptionsWidget *createOptionsWidget() SIP_FACTORY { return mSaveOptionsWidget; }
    const QList<int> overviewList() const { return mOverviewList; }
    QgsRaster::RasterPyramidsFormat pyramidsFormat() const
    { return static_cast< QgsRaster::RasterPyramidsFormat >( cbxPyramidsFormat->currentIndex() ); }
    QString resamplingMethod() const;
    void setRasterLayer( QgsRasterLayer *rasterLayer ) { mSaveOptionsWidget->setRasterLayer( rasterLayer ); }
    void setRasterFileName( const QString &file ) { mSaveOptionsWidget->setRasterFileName( file ); }

  public slots:

    void apply();
    void checkAllLevels( bool checked );

  private slots:

    void cbxPyramidsLevelsCustom_toggled( bool toggled ) SIP_FORCE;
    void cbxPyramidsFormat_currentIndexChanged( int index ) SIP_FORCE;
    void setOverviewList() SIP_FORCE;
    void updateUi() SIP_FORCE;

  signals:
    void overviewListChanged();
    void someValueChanged(); /* emitted when any other setting changes */

  private:

    // Must be in the same order as in the .ui file
    typedef enum
    {
      GTIFF = 0,
      INTERNAL = 1,
      ERDAS = 2
    } Format;


    QString mProvider;
    QList< int > mOverviewList;
    QMap< int, QCheckBox * > mOverviewCheckBoxes;
};

// clazy:excludeall=qstring-allocations

#endif // QGSRASTERLAYERSAVEASDIALOG_H
