/***************************************************************************
  qgslayertreeviewnocrsindicator.h
  --------------------------------------
  Date                 : October 2019
  Copyright            : (C) 2019 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWNOCRSINDICATOR_H
#define QGSLAYERTREEVIEWNOCRSINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"

//! Adds indicators showing whether layers have an unknown/not set CRS
class QgsLayerTreeViewNoCrsIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewNoCrsIndicatorProvider( QgsLayerTreeView *view );

  protected:
    void connectSignals( QgsMapLayer *layer ) override;
    void disconnectSignals( QgsMapLayer *layer ) override;

  protected slots:

    void onIndicatorClicked( const QModelIndex &index ) override;

  private:
    bool acceptLayer( QgsMapLayer *layer ) override;
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
};

#endif // QGSLAYERTREEVIEWNOCRSINDICATOR_H
