/***************************************************************************
  qgslayertreeviewfilterindicator.h
  --------------------------------------
  Date                 : January 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWFILTERINDICATOR_H
#define QGSLAYERTREEVIEWFILTERINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"

#include <QObject>

//! Adds indicators showing whether vector layers have a filter applied.
class QgsLayerTreeViewFilterIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewFilterIndicatorProvider( QgsLayerTreeView *view );

  private:
    bool acceptLayer( QgsMapLayer *layer ) override;
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;

  protected slots:
    void onIndicatorClicked( const QModelIndex &index ) override;

  protected:
    void connectSignals( QgsMapLayer *layer ) override ;
    void disconnectSignals( QgsMapLayer *layer ) override;
};

#endif // QGSLAYERTREEVIEWFILTERINDICATOR_H
