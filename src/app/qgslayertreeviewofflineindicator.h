/***************************************************************************
  qgslayertreeviewofflineindicator.h
  --------------------------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by David Signer
  Email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEVIEWOFFLINEINDICATOR_H
#define QGSLAYERTREEVIEWOFFLINEINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"

#include <QSet>
#include <memory>

class QgsLayerTreeNode;
class QgsLayerTreeView;

//! Adds indicators showing whether layers are offline.
class QgsLayerTreeViewOfflineIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewOfflineIndicatorProvider( QgsLayerTreeView *view );

  protected:
    void connectSignals( QgsMapLayer *layer ) override;
    void disconnectSignals( QgsMapLayer *layer ) override;

  private:
    bool acceptLayer( QgsMapLayer *layer ) override;
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
};

#endif // QGSLAYERTREEVIEWOFFLINEINDICATOR_H
