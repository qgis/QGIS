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
/* $Id$ */

#ifndef QGSLEGENDINTERFACE_H
#define QGSLEGENDINTERFACE_H

#include <QObject>
#include <QPair>
#include <QStringList>

class QgsMapLayer;
class QTreeWidgetItem;

//Information about relationship between groups and layers
//key: group name (or null strings for single layers without groups)
//value: containter with layer ids contained in the group
typedef QPair< QString, QList<QString> > GroupLayerInfo;

/** \ingroup gui
 * QgsLegendInterface
 * Abstract base class to make QgsLegend available to plugins.
 *
 * \note added in 1.4
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

    //! Return all layers in the project in legend order
    //! @note added in 1.5
    virtual QList< QgsMapLayer * > layers() const = 0;

    //! Check if a group exists
    //! @note added in 1.5
    virtual bool groupExists( int groupIndex ) = 0;

    //! Check if a group is expanded
    //! @note added in 1.5
    virtual bool isGroupExpanded( int groupIndex ) = 0;

    //! Check if a group is visible
    //! @note added in 1.5
    virtual bool isGroupVisible( int groupIndex ) = 0;

    //! Check if a layer is visible
    //! @note added in 1.5
    virtual bool isLayerVisible( QgsMapLayer * ml ) = 0;

  signals:
    //! emitted when a group index has changed
    void groupIndexChanged( int oldIndex, int newIndex );

  public slots:

    //! Add a new group
    //! forceAtEnd forces the new group to be created at the end of the legend
    virtual int addGroup( QString name, bool expand = true, QTreeWidgetItem* parent = 0 ) = 0;

    //! Remove group on index
    virtual void removeGroup( int groupIndex ) = 0;

    //! Move a layer to a group
    virtual void moveLayer( QgsMapLayer * ml, int groupIndex ) = 0;

    //! Collapse or expand a group
    //! @note added in 1.5
    virtual void setGroupExpanded( int groupIndex, bool expand ) = 0;

    //! Set the visibility of a group
    //! @note added in 1.5
    virtual void setGroupVisible( int groupIndex, bool visible ) = 0;

    //! Set the visibility of a layer
    //! @note added in 1.5
    virtual void setLayerVisible( QgsMapLayer * ml, bool visible ) = 0;

    //! Refresh layer symbology
    //! @note added in 1.5
    virtual void refreshLayerSymbology( QgsMapLayer *ml ) = 0;
};

#endif
