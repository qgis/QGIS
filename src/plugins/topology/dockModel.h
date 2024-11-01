/***************************************************************************
  dockModel.h
  TOPOLogy checker
  -------------------
         date                 : May 2009
         copyright            : (C) 2009 by Vita Cizek
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
#include <QSortFilterProxyModel>
#include <QModelIndex>
#include <QObject>

#include "topolError.h"

class DockModel : public QAbstractTableModel
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param parent parent object
     */
    DockModel( QObject *parent = nullptr );

    /**
     * \param errorList reference to the ErrorList where errors will be stored
     * \since QGIS 3.38
     */
    void setErrors( const ErrorList &errorList );

    /**
     * Returns header data
     * \param section required section
     * \param orientation horizontal or vertical orientation
     * \param role data role
     */
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

    /**
     * Returns data on the given index
     * \param index model index
     * \param role data role
     */
    QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * Updates data on given index
     * \param index model index
     * \param value new data value
     * \param role data role
     */
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;

    /**
     * Returns item flags for the index
     * \param index model index
     */
    Qt::ItemFlags flags( const QModelIndex &index ) const override;

    /**
     * Returns the number of rows
     * \param parent parent index
     */
    int rowCount( const QModelIndex &parent ) const override;

    /**
     * Returns the number of columns
     * \param parent parent index
     */
    int columnCount( const QModelIndex &parent ) const override;

    /**
     * Reloads the model data between indices
     * \param index1 start index
     * \param index2 end index
     */
    void reload( const QModelIndex &index1, const QModelIndex &index2 );

  private:
    ErrorList mErrorlist;
    QList<QString> mHeader;
};

class DockFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param parent parent object
     */
    DockFilterModel( QObject *parent = nullptr );

    ~DockFilterModel() = default;

    /**
     * \param errorList reference to the ErrorList where errors will be stored
     */
    void setErrors( const ErrorList &errorList );

    /**
     * Reloads the model data between indices
     * \param index1 start index
     * \param index2 end index
     */
    void reload( const QModelIndex &index1, const QModelIndex &index2 );

  private:
    DockModel *mDockModel = nullptr;
};

#endif
