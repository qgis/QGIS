/***************************************************************************
                         qgslayertreeviewprojecttimeindicator.h
                         ---------------
    begin                : February 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSLAYERTREEVIEWPROJECTTIMEINDICATOR_H
#define QGSLAYERTREEVIEWPROJECTTIMEINDICATOR_H


#include "qgslayertreeviewindicatorprovider.h"

/**
 * Adds indicators for showing temporal layers that are using project time to set their
 * current active temporal range.
 */
class QgsLayerTreeViewProjectTimeIndicatorProvider : public QgsLayerTreeViewIndicatorProvider
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeViewProjectTimeIndicatorProvider( QgsLayerTreeView *view );

  protected:
    void connectSignals( QgsMapLayer *layer ) override;

  protected slots:

    void onIndicatorClicked( const QModelIndex &index ) override;

    //! Adds/removes indicator of a layer
    void onLayerChanged();

  private:
    bool acceptLayer( QgsMapLayer *layer ) override;
    QString iconName( QgsMapLayer *layer ) override;
    QString tooltipText( QgsMapLayer *layer ) override;

    QgsMapLayer *mLayer = nullptr;
};

#endif // QGSLAYERTREEVIEWPROJECTTIMEINDICATOR_H
