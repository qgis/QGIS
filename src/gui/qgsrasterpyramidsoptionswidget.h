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
#include "qgsrasterdataprovider.h"

class QCheckBox;

/** \ingroup gui
 * A widget to select format-specific raster saving options
 */
class GUI_EXPORT QgsRasterPyramidsOptionsWidget: public QWidget,
      private Ui::QgsRasterPyramidsOptionsWidgetBase
{
    Q_OBJECT

  public:

    QgsRasterPyramidsOptionsWidget( QWidget* parent = 0, QString provider = "gdal" );
    ~QgsRasterPyramidsOptionsWidget();

    void setProvider( QString provider );
    QStringList createOptions() const { return mPyramidsOptionsWidget->options(); }
    QgsRasterFormatSaveOptionsWidget* createOptionsWidget() { return mPyramidsOptionsWidget; }
    const QList<int> overviewList() const { return mOverviewList; }
    QgsRasterDataProvider::RasterPyramidsFormat pyramidsFormat() const
    { return ( QgsRasterDataProvider::RasterPyramidsFormat ) cbxPyramidsFormat->currentIndex(); }
    QString resamplingMethod() const;

  public slots:

    void apply();
    void checkAllLevels( bool checked );

  private slots:

    void on_cbxPyramidsLevelsCustom_toggled( bool toggled );
    void on_cbxPyramidsFormat_currentIndexChanged( int index )
    { mPyramidsOptionsWidget->setEnabled( index != 2 ); }
    void setOverviewList();

  signals:
    void overviewListChanged();

  private:

    void updateUi();

    QString mProvider;
    QList< int > mOverviewList;
    QMap< int, QCheckBox* > mOverviewCheckBoxes;
};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
