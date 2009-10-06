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
#include <QHash>

#include "qgsfeature.h" // QgsAttributeMap
#include "qgsvectorlayer.h" // QgsAttributeList
#include "qgsattributetableidcolumnpair.h"

class QgsAttributeTableModel: public QAbstractTableModel
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param theLayer layer pointer
     * @param parent parent pointer
     */
    QgsAttributeTableModel( QgsVectorLayer *theLayer, QObject *parent = 0 );

    /**
     * Returns the number of rows
     * @param parent parent index
     */
    int rowCount( const QModelIndex &parent = QModelIndex() ) const;
    /**
     * Returns the number of columns
     * @param parent parent index
     */
    int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    /**
     * Returns header data
     * @param section required section
     * @param orientation horizontal or vertical orientation
     * @param role data role
     */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
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
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole );
    /**
     * Returns item flags for the index
     * @param index model index
     */
    Qt::ItemFlags flags( const QModelIndex &index ) const;

    /**
     * Reloads the model data between indices
     * @param index1 start index
     * @param index2 end index
     */
    void reload( const QModelIndex &index1, const QModelIndex &index2 );
    /**
     * Resets the model
     */
    void resetModel();
    /**
     * Layout has been changed
     */
    void changeLayout();
    /**
     * Layout will be changed
     */
    void incomingChangeLayout();
    /**
     * Maps feature id to table row
     * @param id feature id
     */
    int idToRow( const int id ) const;
    /**
     * get field index from column
     */
    int fieldIdx( int col ) const;
    /**
     * Maps row to feature id
     * @param id row id
     */
    int rowToId( const int id ) const;
    /**
     * Sorts the model
     * @param column column to sort by
     * @param order sorting order
     */
    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder );
    /**
     * Swaps two rows
     * @param a first row
     * @param b second row
     */
    void swapRows( int a, int b );

    /**
     * Returns layer pointer
     */
    QgsVectorLayer* layer() const { return mLayer; }

  signals:
    /**
     * Model has been changed
     */
    void modelChanged();
    /**
     * Sets new number of rows
     * @param oldNum old row number
     * @param newNum new row number
     */
    void setNumRows( int oldNum, int newNum );

  private slots:
    /**
     * Launched when attribute has been added
     * @param idx attribute index
     */
    virtual void attributeAdded( int idx );
    /**
     * Launched when attribute has been deleted
     * @param idx attribute index
     */
    virtual void attributeDeleted( int idx );
    /**
     * Launched when attribute value has been changed
     * @param fid feature id
     * @param idx attribute index
     * @param value new value
     */
    virtual void attributeValueChanged( int fid, int idx, const QVariant &value );
    /**
     * Launched when layer has been modified
     * Rebuilds the model
     * @param onlyGeometry true if only geometry has changed
     */
    virtual void layerModified( bool onlyGeometry );

  protected slots:
    /**
     * Launched when a feature has been deleted
     * @param fid feature id
     */
    virtual void featureDeleted( int fid );
    /**
     * Launched when a feature has been added
     * @param fid feature id
     */
    virtual void featureAdded( int fid );
    /**
     * Launched when layer has been deleted
     */
    virtual void layerDeleted();

  protected:
    QgsVectorLayer *mLayer;
    int mFeatureCount;
    int mFieldCount;
    mutable int mLastRowId;
    mutable QgsFeature mFeat;

    mutable QgsAttributeMap *mLastRow;
    QgsAttributeList mAttributes;
    QMap< int, const QMap<QString, QVariant> * > mValueMaps;

    QList<QgsAttributeTableIdColumnPair> mSortList;
    QHash<int, int> mIdRowMap;
    QHash<int, int> mRowIdMap;

    /**
     * Initializes id <-> row maps
     */
    void initIdMaps();
    /**
     * Loads the layer into the model
     */
    virtual void loadLayer();

    /**
     * load feature fid into mFeat
     * @param fid feature id
     * @return feature exists
     */
    virtual bool featureAtId( int fid );

    QVariant data( const QModelIndex &index, int role );
};


#endif
