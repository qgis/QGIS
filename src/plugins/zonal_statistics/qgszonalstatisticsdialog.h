/***************************************************************************
                          qgszonalstatisticsdialog.h  -  description
                             -----------------------
    begin                : September 1st, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSZONALSTATISTICSDIALOG_H
#define QGSZONALSTATISTICSDIALOG_H

#include "ui_qgszonalstatisticsdialogbase.h"
#include "qgszonalstatistics.h"

class QgisInterface;
class QgsVectorLayer;
class QgsRasterLayer;

class QgsZonalStatisticsDialog: public QDialog, private Ui::QgsZonalStatisticsDialogBase
{
    Q_OBJECT
  public:
    explicit QgsZonalStatisticsDialog( QgisInterface* iface );
    ~QgsZonalStatisticsDialog();

    QString rasterFilePath() const;
    int rasterBand() const;
    QgsVectorLayer* polygonLayer() const;
    QgsRasterLayer* rasterLayer() const;

    QString attributePrefix() const;
    QgsZonalStatistics::Statistics selectedStats() const;

  private:
    QgsZonalStatisticsDialog();
    /** Fills the available raster and polygon layers into the combo boxes*/
    void insertAvailableLayers();
    /** Propose a valid prefix for the attributes*/
    QString proposeAttributePrefix() const;
    /** Check if a prefix can be used for the count, sum and mean attribute*/
    bool prefixIsValid( const QString& prefix ) const;

    QgisInterface* mIface;

  private slots:

    void on_mRasterLayerComboBox_currentIndexChanged( int index );

};

#endif // QGSZONALSTATISTICSDIALOG_H
