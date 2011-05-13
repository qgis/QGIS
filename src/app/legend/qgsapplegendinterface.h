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
/* $Id$ */

#ifndef QGSLEGENDAPPIFACE_H
#define QGSLEGENDAPPIFACE_H

#include "qgslegendinterface.h"

#include <QModelIndex>

class QgsLegend;
class QgsMapLayer;

/** \ingroup gui
 * QgsLegendInterface
 * Abstract base class to make QgsLegend available to plugins.
 */
class QgsAppLegendInterface : public QgsLegendInterface
{
    Q_OBJECT

  public:

    /** Constructor */
    explicit QgsAppLegendInterface( QgsLegend * legend );

    /** Destructor */
    ~QgsAppLegendInterface();

    //! Return a string list of groups
    QStringList groups();

    //! Return the relationship between groups and layers in the legend
    QList< GroupLayerInfo > groupLayerRelationship();

    //! Return all layers in the project in legend order
    QList< QgsMapLayer * > layers() const;

    //! Check if a group exists
    bool groupExists( int groupIndex );

    //! Check if a group is expanded
    bool isGroupExpanded( int groupIndex );

    //! Check if a group is visible
    bool isGroupVisible( int groupIndex );

    //! Check if a layer is visible
    bool isLayerVisible( QgsMapLayer * ml );

  public slots:

    //! Add a new group
    int addGroup( QString name, bool expand = true, QTreeWidgetItem* parent = 0 );

    //! Add a new group at a specified index
    int addGroup( QString name, bool expand, int groupIndex );

    //! Remove all groups with the given name
    void removeGroup( int groupIndex );

    //! Move a layer to a group
    void moveLayer( QgsMapLayer *ml, int groupIndex );

    //! Update an index
    void updateIndex( QModelIndex oldIndex, QModelIndex newIndex );

    //! Collapse or expand a group
    virtual void setGroupExpanded( int groupIndex, bool expand );

    //! Set the visibility of a group
    virtual void setGroupVisible( int groupIndex, bool visible );

    //! Set the visibility of a layer
    virtual void setLayerVisible( QgsMapLayer * ml, bool visible );

    //! refresh layer symbology
    void refreshLayerSymbology( QgsMapLayer *ml );

  private:

    //! Pointer to QgsLegend object
    QgsLegend *mLegend;
};

#endif //QGSLEGENDAPPIFACE_H
