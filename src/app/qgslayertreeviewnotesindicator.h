/***************************************************************************
  qgslayertreeviewnotesindicator.h
  --------------------------------------
  Date                 : April 2021
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

#ifndef QGSLAYERTREEVIEWNOTESINDICATOR_H
#define QGSLAYERTREEVIEWNOTESINDICATOR_H

#include "qgslayertreeviewindicatorprovider.h"

//! Adds indicators showing whether layers have notes attached.
class QgsLayerTreeViewNotesIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewNotesIndicatorProvider( QgsLayerTreeView *view );

  protected slots:

    void onIndicatorClicked( const QModelIndex &index ) override;

  protected:
    void connectSignals( QgsMapLayer *layer ) override;
    void disconnectSignals( QgsMapLayer *layer ) override;

  private:
    bool acceptLayer( QgsMapLayer *layer ) override;
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;
};

#endif // QGSLAYERTREEVIEWNOTESINDICATOR_H
