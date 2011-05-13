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

class GUI_EXPORT QgsAttributeTableModel: public QAbstractTableModel
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
     * Remove rows
     */
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() );
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
     * get column from field index
     */
    int fieldCol( int idx ) const;
    /**
     * Maps row to feature id
     * @param row row number
     */
    int rowToId( const int row ) const;
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

    /** Execute an action */
    void executeAction( int action, const QModelIndex &idx ) const;

    /** return feature attributes at given index */
    QgsFeature feature( QModelIndex &idx );

    /** In case the table's behaviour is to show only features from current extent,
      this is a method how to let the model know what extent to use without having
      to explicitly ask any canvas */
    static void setCurrentExtent( const QgsRectangle& extent ) { mCurrentExtent = extent; }

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
  protected slots:
    /**
     * Launched when attribute value has been changed
     * @param fid feature id
     * @param idx attribute index
     * @param value new value
     */
    virtual void attributeValueChanged( int fid, int idx, const QVariant &value );
    /**
     * Launched when a feature has been deleted
     * @param fid feature id
     */
    virtual void featureDeleted( int fid );
    /**
     * Launched when a feature has been added
     * @param fid feature id
     * @parem inOperation guard insertion with beginInsertRows() / endInsertRows()
     */
    virtual void featureAdded( int fid, bool inOperation = true );
    /**
     * Launched when layer has been deleted
     */
    virtual void layerDeleted();

  protected:
    QgsVectorLayer *mLayer;
    int mFieldCount;

    mutable QgsFeature mFeat;

    QgsAttributeList mAttributes;
    QMap< int, const QMap<QString, QVariant> * > mValueMaps;

    QList<QgsAttributeTableIdColumnPair> mSortList;
    QHash<int, int> mIdRowMap;
    QHash<int, int> mRowIdMap;

    //! useful when showing only features from a particular extent
    static QgsRectangle mCurrentExtent;

    /**
     * Initializes id <-> row maps
     */
    void initIdMaps();

    /**
     * Loads the layer into the model
     */
    virtual void loadLayer();

    /**
      * Gets mFieldCount, mAttributes and mValueMaps
      */
    virtual void loadAttributes();

    /**
     * load feature fid into mFeat
     * @param fid feature id
     * @return feature exists
     */
    virtual bool featureAtId( int fid ) const;
};


#endif
