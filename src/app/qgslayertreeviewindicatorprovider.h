/***************************************************************************
  qgslayertreeviewindicatorprovider.h - QgsLayerTreeViewIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by ale
 email                : [your-email-here]
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYERTREEVIEWINDICATORPROVIDER_H
#define QGSLAYERTREEVIEWINDICATORPROVIDER_H

#include <QObject>
#include <QSet>
#include <memory>

#include "qgslayertreeviewindicator.h"

class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsMapLayer;


/**
 * The QgsLayerTreeViewIndicatorProvider class provides an interface for
 * layer tree indicator providers.
 *
 * Subclasses must override:
 * - iconName()
 * - tooltipText()
 * - acceptLayer() filter function to determine whether the indicator must be added for the layer
 *
 * Subclasses may override:
 * - onIndicatorClicked() default implementation does nothing
 * - connectSignals() default implementation connects vector layers to dataSourceChanged
 * - disconnectSignals() default implementation disconnects vector layers from dataSourceChanged
 */
class QgsLayerTreeViewIndicatorProvider : public QObject
{
    Q_OBJECT
  public:

    explicit QgsLayerTreeViewIndicatorProvider( QgsLayerTreeView *view );

  protected slots:
    //! Connects to signals of layers newly added to the tree
    virtual void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    virtual void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    virtual void onLayerLoaded();
    //! Adds/removes indicator of a layer
    virtual void onLayerChanged();
    //! Action on indicator clicked
    virtual void onIndicatorClicked( const QModelIndex &index ) { Q_UNUSED( index ) }
    //! Connect signals
    virtual void connectSignals( QgsMapLayer *layer );
    //! Disconnect signals
    virtual void disconnectSignals( QgsMapLayer *layer );

  private:
    //! Layer filter
    virtual bool acceptLayer( QgsMapLayer *layer ) = 0;
    virtual QString iconName() = 0;
    virtual QString tooltipText() = 0;
    virtual std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator();
    virtual void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsMapLayer *layer );

  protected:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWINDICATORPROVIDER_H
