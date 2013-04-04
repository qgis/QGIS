/***************************************************************************
  dockModel.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
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

#ifndef DOCKMODEL_H
#define DOCKMODEL_H

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QObject>

#include "topolError.h"

class DockModel: public QAbstractTableModel
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param theErrorList reference to the ErrorList where errors will be stored
     * @param parent parent object
     */
    DockModel( ErrorList& theErrorList, QObject *parent );
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
     * Returns the number of rows
     * @param parent parent index
     */
    int rowCount( const QModelIndex &parent ) const;
    /**
     * Returns the number of columns
     * @param parent parent index
     */
    int columnCount( const QModelIndex &parent ) const;

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

  private:
    ErrorList& mErrorlist;
    QList<QString> mHeader;
};

#endif
