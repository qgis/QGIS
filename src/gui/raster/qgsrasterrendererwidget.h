/***************************************************************************
                         qgsrasterrendererwidget.h
                         ---------------------------
    begin                : February 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERRENDERERWIDGET_H
#define QGSRASTERRENDERERWIDGET_H

#include <QWidget>

class QgsRasterLayer;
class QgsRasterRenderer;

class GUI_EXPORT QgsRasterRendererWidget: public QWidget
{
  public:
    QgsRasterRendererWidget( QgsRasterLayer* layer ) { mRasterLayer = layer; }
    virtual ~QgsRasterRendererWidget() {}

    enum LoadMinMaxAlgo
    {
      Estimate,
      Actual,
      CurrentExtent
    };

    virtual QgsRasterRenderer* renderer() = 0;

    void setRasterLayer( QgsRasterLayer* layer ) { mRasterLayer = layer; }
    const QgsRasterLayer* rasterLayer() const { return mRasterLayer; }

    virtual QString min( int index = 0 ) { Q_UNUSED( index ); return QString( ); }
    virtual QString max( int index = 0 ) { Q_UNUSED( index ); return QString( ); }
    virtual void setMin( QString value, int index = 0 ) { Q_UNUSED( index ); Q_UNUSED( value ); }
    virtual void setMax( QString value, int index = 0 ) { Q_UNUSED( index ); Q_UNUSED( value ); }
    virtual QString stdDev( ) { return QString( ); }
    virtual void setStdDev( QString value ) { Q_UNUSED( value ); }
    virtual int selectedBand( int index = 0 ) { Q_UNUSED( index ); return -1; }

    bool bandMinMax( LoadMinMaxAlgo loadAlgo, int band, double *values );
    bool bandMinMaxFromStdDev( double stdDev, int band, double *values );

  protected:
    QgsRasterLayer* mRasterLayer;
    /**Returns a band name for display. First choice is color name, otherwise band number*/
    QString displayBandName( int band ) const;
};

#endif // QGSRASTERRENDERERWIDGET_H
