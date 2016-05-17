/***************************************************************************
    qgsgcplistmodel.h - Model implementation of GCPList Model/View
     --------------------------------------
    Date                 : 27-Feb-2009
    Copyright            : (c) 2009 by Manuel Massing
    Email                : m.massing at warped-space.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGCP_LIST_TABLE_VIEW_H
#define QGSGCP_LIST_TABLE_VIEW_H

#include <QTreeView>
#include <QStandardItemModel>

class QgsGeorefDataPoint;
class QgsGeorefTransform;
class QgsGCPList;

class QgsGCPListModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    explicit QgsGCPListModel( QObject *parent = nullptr );

    void setGCPList( QgsGCPList *theGCPList );
    void setGeorefTransform( QgsGeorefTransform *theGeorefTransform );
    void updateModel();

  public slots:
    void replaceDataPoint( QgsGeorefDataPoint *newDataPoint, int i );

    void onGCPListModified();
    void onTransformationModified();

  private:
    QgsGCPList         *mGCPList;
    QgsGeorefTransform *mGeorefTransform;
};

#endif
