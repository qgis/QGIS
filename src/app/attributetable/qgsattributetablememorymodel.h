/***************************************************************************
  QgsAttributeTableMemoryModel.h - Memory Model for attribute table
  -------------------
         date                 : Feb 2009
         copyright            : Vita Cizek
         email                : weetya (at) gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTETABLEMEMORYMODEL_H
#define QGSATTRIBUTETABLEMEMORYMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>

//QGIS Includes
#include "qgsfeature.h" //QgsAttributeMap
#include "qgsvectorlayer.h" //QgsAttributeList
#include "qgsattributetablemodel.h"
#include "qgsattributetableidcolumnpair.h"

class QgsAttributeTableMemoryModel: public QgsAttributeTableModel
{
    Q_OBJECT;

  public:
    QgsAttributeTableMemoryModel( QgsVectorLayer *theLayer );//, QObject *parent = 0);

  protected slots:
    virtual void featureDeleted( int fid );
    virtual void featureAdded( int fid );
    virtual void layerDeleted();

  private slots:
    //virtual void attributeAdded (int idx);
    //virtual void attributeDeleted (int idx);
    virtual void attributeValueChanged( int fid, int idx, const QVariant &value );
    //virtual void layerModified(bool onlyGeometry);

  private:
    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role );
    virtual void loadLayer();

    QMap<int, QgsFeature> mFeatureMap;
};

#endif //QGSATTRIBUTETABLEMEMORYMODEL_H
