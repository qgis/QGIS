/***************************************************************************
    qgslegendinterface.h
     --------------------------------------
    Date                 : 19-Nov-2009
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

#ifndef QGSLEGENDINTERFACE_H
#define QGSLEGENDINTERFACE_H

#include <QObject>
#include <QPair>
#include <QStringList>
#include <QModelIndex>

class QgsMapLayer;
class QTreeWidgetItem;
class QAction;

#include "qgsmaplayer.h"

//Information about relationship between groups and layers
//key: group name (or null strings for single layers without groups)
//value: containter with layer ids contained in the group
typedef QPair< QString, QList<QString> > GroupLayerInfo;

/** \ingroup gui
 * QgsLegendInterface
 * Abstract base class to make QgsLegend available to plugins.
 */
class GUI_EXPORT QgsLegendInterface : public QObject
{
    Q_OBJECT

  public:

    /** Constructor */
    QgsLegendInterface();

    /** Virtual destructor */
    virtual ~QgsLegendInterface();

    //! Return a string list of groups
    virtual QStringList groups() = 0;

    //! Return the relationship between groups and layers in the legend
    virtual QList< GroupLayerInfo > groupLayerRelationship() { return QList< GroupLayerInfo >(); }

    //! Returns the currently selected layers of QgsLegendLayers.
    //! @param inDrawOrder return layers in drawing order
    //! @returns list of layers, else an empty list
    virtual QList<QgsMapLayer *> selectedLayers( bool inDrawOrder = false ) const = 0;

    //! Return all layers in the project in drawing order
    virtual QList< QgsMapLayer * > layers() const = 0;

    //! Check if a group exists
    virtual bool groupExists( int groupIndex ) = 0;

    //! Check if a group is expanded
    virtual bool isGroupExpanded( int groupIndex ) = 0;

    //! Check if a group is visible
    virtual bool isGroupVisible( int groupIndex ) = 0;

    //! Check if a layer is expanded
    virtual bool isLayerExpanded( QgsMapLayer * ml ) = 0;

    //! Check if a layer is visible
    virtual bool isLayerVisible( QgsMapLayer * ml ) = 0;

    /** Add action for layers in the legend
     */
    virtual void addLegendLayerAction( QAction* action, QString menu, QString id,
                                       QgsMapLayer::LayerType type, bool allLayers ) = 0;

    /** Add action for a specific layers in the legend.
     * Use this in combination with addLegendLayerAction( allLayers = False )
     */
    virtual void addLegendLayerActionForLayer( QAction* action, QgsMapLayer* layer ) = 0;

    /** Remove action for layers in the legend */
    virtual bool removeLegendLayerAction( QAction* action ) = 0;

    //! Returns the current layer if the current item is a QgsLegendLayer.
    //! If the current item is a QgsLegendLayer, its first maplayer is returned.
    //! Else, 0 is returned.
    virtual QgsMapLayer* currentLayer() = 0;

    //! set the current layer
    //! returns true if the layer exists, false otherwise
    virtual bool setCurrentLayer( QgsMapLayer *layer ) = 0;

  signals:

    //! emitted when a group index has changed
    void groupIndexChanged( int oldIndex, int newIndex );

    /* //! emitted when group relations have changed */
    void groupRelationsChanged();

    /* //! emitted when an item (group/layer) is added */
    void itemAdded( QModelIndex index );

    /* //! emitted when an item (group/layer) is removed */
    void itemRemoved();

    //! Emitted whenever current (selected) layer changes
    //  the pointer to layer can be null if no layer is selected
    void currentLayerChanged( QgsMapLayer * layer );

  public slots:

    //! Add a new group
    //! a parent group can be given to nest the new group in it
    virtual int addGroup( QString name, bool expand = true, QTreeWidgetItem* parent = 0 ) = 0;

    //! Add a new group
    //! a parent group index has to be given to nest the new group in it
    virtual int addGroup( QString name, bool expand, int parentIndex ) = 0;

    //! Remove group on index
    virtual void removeGroup( int groupIndex ) = 0;

    //! Move a layer to a group
    virtual void moveLayer( QgsMapLayer * ml, int groupIndex ) = 0;

    //! Collapse or expand a group
    virtual void setGroupExpanded( int groupIndex, bool expand ) = 0;

    //! Collapse or expand a layer
    virtual void setLayerExpanded( QgsMapLayer * ml, bool expand ) = 0;

    //! Set the visibility of a group
    virtual void setGroupVisible( int groupIndex, bool visible ) = 0;

    //! Set the visibility of a layer
    virtual void setLayerVisible( QgsMapLayer * ml, bool visible ) = 0;

    //! Refresh layer symbology

    virtual void refreshLayerSymbology( QgsMapLayer *ml ) = 0;
};

#endif
