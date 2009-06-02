/***************************************************************************
  QgsAttributeTableModel.h - Models for attribute table
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

#ifndef QGSATTRIBUTETABLEMODEL_H
#define QGSATTRIBUTETABLEMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>

//QGIS Includes
#include "qgsfeature.h" //QgsAttributeMap
#include "qgsvectorlayer.h" //QgsAttributeList
#include "qgsattributetableidcolumnpair.h"

class QgsAttributeTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:
    QgsAttributeTableModel( QgsVectorLayer *theLayer, QObject *parent = 0 );

    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    virtual QVariant data( const QModelIndex &index, int role ) const;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    void reload( const QModelIndex &index1, const QModelIndex &index2 );
    void resetModel();
    void changeLayout();
    void incomingChangeLayout();
    int idToRow( const int id ) const;
    int rowToId( const int id ) const;
    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
    void swapRows( int a, int b );

    QgsVectorLayer* layer() const { return mLayer; }

  signals:
    void modelChanged();
    void setNumRows( int oldNum, int newNum );

  private slots:
    virtual void attributeAdded( int idx );
    virtual void attributeDeleted( int idx );
    virtual void attributeValueChanged( int fid, int idx, const QVariant &value );
    virtual void layerModified( bool onlyGeometry );

  protected slots:
    virtual void featureDeleted( int fid );
    virtual void featureAdded( int fid );
    virtual void layerDeleted();

  protected:
    QgsVectorLayer *mLayer;
    int mFeatureCount;
    int mFieldCount;
    mutable int mLastRowId;
    mutable QgsFeature mFeat;

    mutable QgsAttributeMap *mLastRow;
    QgsAttributeList mAttributes;

    QList<QgsAttributeTableIdColumnPair> mSortList;
    QMap<int, int> mIdRowMap;
    QMap<int, int> mRowIdMap;

    void initIdMaps();
    virtual void loadLayer();

};


#endif
