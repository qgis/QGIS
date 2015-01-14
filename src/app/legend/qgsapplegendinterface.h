/***************************************************************************
    qgsapplegendinterface.h
     --------------------------------------
    Date                 : 23-Nov-2009
    Copyright            : (C) 2009 by Andres Manz
    Email                : manz dot andres at gmail dot com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLEGENDAPPIFACE_H
#define QGSLEGENDAPPIFACE_H

#include "qgslegendinterface.h"

#include <QModelIndex>

class QgsLayerTreeGroup;
class QgsLayerTreeNode;
class QgsLayerTreeView;
class QgsMapLayer;

/** \ingroup gui
 * QgsLegendInterface
 * Abstract base class to make QgsLegend available to plugins.
 */
Q_NOWARN_DEPRECATED_PUSH
class QgsAppLegendInterface : public QgsLegendInterface
{
    Q_OBJECT

  public:

    /** Constructor */
    explicit QgsAppLegendInterface( QgsLayerTreeView * layerTreeView );

    /** Destructor */
    ~QgsAppLegendInterface();

    //! Return a string list of groups
    QStringList groups() OVERRIDE;

    //! Return the relationship between groups and layers in the legend
    QList< GroupLayerInfo > groupLayerRelationship() OVERRIDE;

    //! Returns the currently selected layers of QgsLegendLayers.
    QList<QgsMapLayer *> selectedLayers( bool inDrawOrder = false ) const OVERRIDE;

    //! Return all layers in the project in drawing order
    QList< QgsMapLayer * > layers() const OVERRIDE;

    //! Check if a group exists
    bool groupExists( int groupIndex ) OVERRIDE;

    //! Check if a group is expanded
    bool isGroupExpanded( int groupIndex ) OVERRIDE;

    //! Check if a group is visible
    bool isGroupVisible( int groupIndex ) OVERRIDE;

    //! Check if a layer is expanded
    bool isLayerExpanded( QgsMapLayer * ml ) OVERRIDE;

    //! Check if a layer is visible
    bool isLayerVisible( QgsMapLayer * ml ) OVERRIDE;

    void addLegendLayerAction( QAction* action, QString menu, QString id,
                               QgsMapLayer::LayerType type, bool allLayers ) OVERRIDE;
    void addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer ) OVERRIDE;
    bool removeLegendLayerAction( QAction* action ) OVERRIDE;

    QgsMapLayer* currentLayer() OVERRIDE;
    bool setCurrentLayer( QgsMapLayer *layer ) OVERRIDE;

  public slots:

    //! Add a new group
    int addGroup( QString name, bool expand = true, QTreeWidgetItem* parent = 0 ) OVERRIDE;

    //! Add a new group at a specified index
    int addGroup( QString name, bool expand, int groupIndex ) OVERRIDE;

    //! Remove all groups with the given name
    void removeGroup( int groupIndex ) OVERRIDE;

    //! Move a layer to a group
    void moveLayer( QgsMapLayer *ml, int groupIndex ) OVERRIDE;

    //! Collapse or expand a group
    virtual void setGroupExpanded( int groupIndex, bool expand ) OVERRIDE;

    //! Collapse or expand a layer
    virtual void setLayerExpanded( QgsMapLayer * ml, bool expand ) OVERRIDE;

    //! Set the visibility of a group
    virtual void setGroupVisible( int groupIndex, bool visible ) OVERRIDE;

    //! Set the visibility of a layer
    virtual void setLayerVisible( QgsMapLayer * ml, bool visible ) OVERRIDE;

    //! refresh layer symbology
    void refreshLayerSymbology( QgsMapLayer *ml ) OVERRIDE;

  protected slots:
    void onAddedChildren( QgsLayerTreeNode* node, int indexFrom, int indexTo );
    void onRemovedChildren();

  private:
    //! Pointer to QgsLegend object
    QgsLayerTreeView* mLayerTreeView;
    QgsLayerTreeGroup* groupIndexToNode( int itemIndex );
    int groupNodeToIndex( QgsLayerTreeGroup* group );
    void setExpanded( QgsLayerTreeNode *node, bool expand );
};
Q_NOWARN_DEPRECATED_POP

#endif //QGSLEGENDAPPIFACE_H
