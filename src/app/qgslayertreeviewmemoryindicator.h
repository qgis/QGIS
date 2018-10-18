/***************************************************************************
  qgslayertreeviewmemoryindicator.h
  --------------------------------------
  Date                 : July 2018
  Copyright            : (C) 2018 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWMEMORYINDICATOR_H
#define QGSLAYERTREEVIEWMEMORYINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"

//! Adds indicators showing whether layers are memory layers.
class QgsLayerTreeViewMemoryIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewMemoryIndicatorProvider( QgsLayerTreeView *view );

  protected slots:

    void onIndicatorClicked( const QModelIndex &index );

  private:
    bool acceptLayer( QgsMapLayer *layer ) override;
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
};

#endif // QGSLAYERTREEVIEWMEMORYINDICATOR_H
