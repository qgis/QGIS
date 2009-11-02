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
#include <QHash>

//QGIS Includes
#include "qgsfeature.h" //QgsAttributeMap
#include "qgsvectorlayer.h" //QgsAttributeList
#include "qgsattributetablemodel.h"
#include "qgsattributetableidcolumnpair.h"

class QgsAttributeTableMemoryModel: public QgsAttributeTableModel
{
    Q_OBJECT;

  public:
    /**
     * Constructor
     * @param theLayer layer pointer
     */
    QgsAttributeTableMemoryModel( QgsVectorLayer *theLayer );

  protected slots:
    /**
     * Launched when a feature has been deleted
     * @param fid feature id
     */
    virtual void featureDeleted( int fid );
    /**
     * Launched when a feature has been deleted
     * @param fid feature id
     */
    virtual void featureAdded( int fid );
    /**
     * Launched when layer has been deleted
     */
    virtual void layerDeleted();

  private slots:
    /**
     * Launched when attribute has been added
     * @param idx attribute index
     */
    //virtual void attributeAdded (int idx);
    /**
     * Launched when attribute has been deleted
     * @param idx attribute index
     */
    //virtual void attributeDeleted (int idx);
    /**
     * Launched when layer has been modified
     * Rebuilds the model
     * @param onlyGeometry true if only geometry has changed
     */
    //virtual void layerModified(bool onlyGeometry);
    /**
     * Launched when attribute value has been changed
     * @param fid feature id
     * @param idx attribute index
     * @param value new value
     */
    virtual void attributeValueChanged( int fid, int idx, const QVariant &value );

  private:
    /**
     * Returns data on the given index
     * @param index model index
     * @param role data role
     */
    virtual QVariant data( const QModelIndex &index, int role ) const;
    /**
     * Updates data on given index
     * @param index model index
     * @param value new data value
     * @param role data role
     */
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role );
    /**
     * Loads the layer into the model
     */
    virtual void loadLayer();

    QHash<int, QgsFeature> mFeatureMap;
};

#endif //QGSATTRIBUTETABLEMEMORYMODEL_H
