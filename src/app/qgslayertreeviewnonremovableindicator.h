/***************************************************************************
  qgslayertreeviewnonremovableindicator.h
  --------------------------------------
  Date                 : Sep 2018
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

#ifndef QGSLAYERTREEVIEWNONREMOVABLEINDICATOR_H
#define QGSLAYERTREEVIEWNONREMOVABLEINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"

#include <QSet>
#include <memory>

class QgsLayerTreeViewNonRemovableIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewNonRemovableIndicatorProvider( QgsLayerTreeView *view );

  private:
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
    bool acceptLayer( QgsMapLayer *layer ) override;

  protected:
    void connectSignals( QgsMapLayer *layer ) override;
    void disconnectSignals( QgsMapLayer *layer ) override;
};


#endif // QGSLAYERTREEVIEWNONREMOVABLEINDICATOR_H
