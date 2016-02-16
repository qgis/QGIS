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
    QStringList groups() override;

    //! Return the relationship between groups and layers in the legend
    QList< GroupLayerInfo > groupLayerRelationship() override;

    //! Returns the currently selected layers of QgsLegendLayers.
    QList<QgsMapLayer *> selectedLayers( bool inDrawOrder = false ) const override;

    //! Return all layers in the project in drawing order
    QList< QgsMapLayer * > layers() const override;

    //! Check if a group exists
    bool groupExists( int groupIndex ) override;

    //! Check if a group is expanded
    bool isGroupExpanded( int groupIndex ) override;

    //! Check if a group is visible
    bool isGroupVisible( int groupIndex ) override;

    //! Check if a layer is expanded
    bool isLayerExpanded( QgsMapLayer * ml ) override;

    //! Check if a layer is visible
    bool isLayerVisible( QgsMapLayer * ml ) override;

    void addLegendLayerAction( QAction* action, QString menu, QString id,
                               QgsMapLayer::LayerType type, bool allLayers ) override;
    void addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer ) override;
    bool removeLegendLayerAction( QAction* action ) override;

    QgsMapLayer* currentLayer() override;
    bool setCurrentLayer( QgsMapLayer *layer ) override;

  public slots:

    //! Add a new group
    int addGroup( const QString& name, bool expand = true, QTreeWidgetItem* parent = nullptr ) override;

    //! Add a new group at a specified index
    int addGroup( const QString& name, bool expand, int groupIndex ) override;

    //! Remove all groups with the given name
    void removeGroup( int groupIndex ) override;

    //! Move a layer to a group
    void moveLayer( QgsMapLayer *ml, int groupIndex ) override;

    //! Collapse or expand a group
    virtual void setGroupExpanded( int groupIndex, bool expand ) override;

    //! Collapse or expand a layer
    virtual void setLayerExpanded( QgsMapLayer * ml, bool expand ) override;

    //! Set the visibility of a group
    virtual void setGroupVisible( int groupIndex, bool visible ) override;

    //! Set the visibility of a layer
    virtual void setLayerVisible( QgsMapLayer * ml, bool visible ) override;

    //! refresh layer symbology
    void refreshLayerSymbology( QgsMapLayer *ml ) override;

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
