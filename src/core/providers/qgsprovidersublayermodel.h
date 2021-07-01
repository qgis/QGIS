/***************************************************************************
                             qgsprovidersublayermodel.h
                             ----------------------
    begin                : June 2021
    copyright            : (C) 2021 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROVIDERSUBLAYERMODEL_H
#define QGSPROVIDERSUBLAYERMODEL_H

#include "qgstaskmanager.h"
#include <QAbstractItemModel>

class QgsProviderSublayerDetails;

/**
 * \ingroup core
 *
 * \brief

 * \since QGIS 3.22
 */
class CORE_EXPORT QgsProviderSublayerModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Custom model roles
    enum class Role : int
    {
      ProviderKey = Qt::UserRole + 1, //!< Provider key
      LayerType, //!< Layer type
      Uri, //!< Layer URI
      Name, //!< Layer name
      Description, //!< Layer description
      Path, //!< Layer path
      FeatureCount, //!< Feature count (for vector sublayers)
      WkbType, //!< WKB geometry type (for vector sublayers)
      GeometryColumnName, //!< Geometry column name (for vector sublayers)
      LayerNumber, //!< Layer number
    };

    //! Model columns
    enum class Column : int
    {
      Name = 0, //!< Layer name
      Description = 1, //!< Layer description
    };

    /**
     * Constructor for QgsProviderSublayerModel, with the specified \a parent object.
     */
    QgsProviderSublayerModel( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the sublayer \a details to show in the model.
     *
     * \see sublayerDetails()
     */
    void setSublayerDetails( const QList< QgsProviderSublayerDetails > &details );

    /**
     * Returns the sublayer details shown in the model.
     *
     * \see setSublayerDetails()
     */
    QList< QgsProviderSublayerDetails > sublayerDetails() const;
    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

  private:

    QList<QgsProviderSublayerDetails> mSublayers;

};

#endif // QGSPROVIDERSUBLAYERMODEL_H
