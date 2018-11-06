/***************************************************************************
  qgslayertreeviewindicatorprovider.h - QgsLayerTreeViewIndicatorProvider

 ---------------------
 begin                : 17.10.2018
 copyright            : (C) 2018 by Alessandro Pasotti
 email                : elpaso@itopen.it
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
 * - connectSignals() default implementation connects layers to dataSourceChanged()
 * - disconnectSignals() default implementation disconnects layers from dataSourceChanged()
 */
class QgsLayerTreeViewIndicatorProvider : public QObject
{
    Q_OBJECT
  public:

    explicit QgsLayerTreeViewIndicatorProvider( QgsLayerTreeView *view );

  protected:

    // Subclasses MAY override:
    //! Connect signals, default implementation connects layers to dataSourceChanged()
    virtual void connectSignals( QgsMapLayer *layer );
    //! Disconnect signals, default implementation disconnects layers from dataSourceChanged()
    virtual void disconnectSignals( QgsMapLayer *layer );

  protected slots:

    //! Action on indicator clicked, default implementation does nothing
    virtual void onIndicatorClicked( const QModelIndex &index ) { Q_UNUSED( index ) }
    // End MAY overrides

    //! Connects to signals of layers newly added to the tree
    void onAddedChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    //! Disconnects from layers about to be removed from the tree
    void onWillRemoveChildren( QgsLayerTreeNode *node, int indexFrom, int indexTo );
    void onLayerLoaded();
    //! Adds/removes indicator of a layer
    void onLayerChanged();

  private:

    // Subclasses MUST override:
    //! Layer filter: layers that pass the test will get the indicator
    virtual bool acceptLayer( QgsMapLayer *layer ) = 0;
    //! Returns the icon name for the given \a layer, icon name is passed to QgsApplication::getThemeIcon()
    virtual QString iconName( QgsMapLayer *layer ) = 0;
    //! Returns the tooltip text for the given \a layer
    virtual QString tooltipText( QgsMapLayer *layer ) = 0;
    // End MUST overrides

    //! Indicator factory
    std::unique_ptr< QgsLayerTreeViewIndicator > newIndicator( QgsMapLayer *layer );
    //! Add or remove the indicator to the given node
    void addOrRemoveIndicator( QgsLayerTreeNode *node, QgsMapLayer *layer );

  protected:
    QgsLayerTreeView *mLayerTreeView = nullptr;
    QSet<QgsLayerTreeViewIndicator *> mIndicators;
};

#endif // QGSLAYERTREEVIEWINDICATORPROVIDER_H
