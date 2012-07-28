/***************************************************************************
                         qgsrasterminmaxwidget.h
                         ---------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERMINMAXWIDGET_H
#define QGSRASTERMINMAXWIDGET_H

#include "ui_qgsrasterminmaxwidgetbase.h"
#include "qgsrasterlayer.h"
#include "qgsrectangle.h"

class GUI_EXPORT QgsRasterMinMaxWidget: public QWidget, private Ui::QgsRasterMinMaxWidgetBase
{
    Q_OBJECT
  public:
    QgsRasterMinMaxWidget( QgsRasterLayer* theLayer, QWidget *parent = 0 );
    ~QgsRasterMinMaxWidget();

    void setExtent ( const QgsRectangle & theExtent ) { mExtent = theExtent; }

    void setBands ( const QList<int> & theBands ) { mBands = theBands; }

  signals:
    void load( int theBandNo, double theMin, double theMax );

  private slots:
    void on_mLoadPushButton_clicked();

  private:
    QgsRasterLayer* mLayer; 
    QList<int> mBands;
    QgsRectangle mExtent;
};

#endif // QGSRASTERMINMAXWIDGET_H
