/***************************************************************************
    qgslegendgroup.h
    ---------------------
    begin                : January 2007
    copyright            : (C) 2007 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLEGENDGROUP_H
#define QGSLEGENDGROUP_H

#include <list>
#include <qgslegenditem.h>


/**
This is a specialised version of QLegendItem that specifies that the items below this point will be treated as a group. For example hiding this node will hide all layers below that are members of the group.

@author Tim Sutton
*/
class QgsLegendGroup : public QgsLegendItem
{
  public:
    QgsLegendGroup( QTreeWidgetItem *, QString );
    QgsLegendGroup( QTreeWidget*, QString );
    QgsLegendGroup( QString name );
    ~QgsLegendGroup();

    /** Helper method to set font characteristics.
     *  Not to be confused with setFont() which is inherited
     *  from the QTreeWidgetItem base class.
     *  @note added in QGIS 1.9
     */
    void setupFont();

    bool insert( QgsLegendItem* theItem );
    /**Returns all legend layers under this group (including those of subgroups by default)*/
    QList<QgsLegendLayer*> legendLayers( bool recurse = true );

    Qt::CheckState pendingCheckState();

    bool isEmbedded() const { return mEmbedded; }
    void setEmbedded( bool b ) { mEmbedded = b; }

    QString projectPath() const { return mProjectPath; }
    void setProjectPath( const QString& path ) { mProjectPath = path; }

    int drawingOrder() const { return mDrawingOrder; }
    void setDrawingOrder( int i ) { mDrawingOrder = i; }

  private:
    bool mEmbedded;
    /**Path to project from which the group is embedded. Empty for not-embedded groups*/
    QString mProjectPath;
    int mDrawingOrder;

};

#endif
