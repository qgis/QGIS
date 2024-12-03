/***************************************************************************
  qgslayertreeviewlowaccuracyindicator.h
  --------------------------------------
  Date                 : May 2021
  Copyright            : (C) 2021 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERTREEVIEWLOWACCURACYINDICATOR_H
#define QGSLAYERTREEVIEWLOWACCURACYINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"
#include "qgsmaplayer.h"

#include <QObject>
#include <QPointer>

//! Indicators for low accuracy layers
class QgsLayerTreeViewLowAccuracyIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT

  public:
    explicit QgsLayerTreeViewLowAccuracyIndicatorProvider( QgsLayerTreeView *view );

  protected:
    void connectSignals( QgsMapLayer *layer ) override;
    void disconnectSignals( QgsMapLayer *layer ) override;

  protected slots:
    void onIndicatorClicked( const QModelIndex &index ) override;

  private:
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
    bool acceptLayer( QgsMapLayer *layer ) override;
};

#endif // QGSLAYERTREEVIEWLOWACCURACYINDICATOR_H
