/***************************************************************************
   qgsdatabasetablemodel.h
    --------------------------------------
   Date                 : March 2020
   Copyright            : (C) 2020 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSDATABASETABLEMODEL_H
#define QGSDATABASETABLEMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "qgsabstractdatabaseproviderconnection.h"
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

/**
 * \ingroup core
 * \class QgsDatabaseTableModel
 * \brief A model containing tables from a database connection.
 *
 * This class does not automatically subscribe to database updates. Tables are queried
 * from the database initially upon model construction. In order
 * to update the listed tbales, QgsDatabaseTableModel::refresh() must be manually
 * called.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsDatabaseTableModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Model roles
    enum Role
    {
      RoleTableName = Qt::UserRole, //!< Table name
      RoleSchema, //!< Table schema
      RoleTableFlags, //!< Table flags role
      RoleComment, //!< Comment role
      RoleCustomInfo, //!< Custom info variant map role
      RoleWkbType, //!< WKB type for primary (first) geometry column in table
      RoleCrs, //!< CRS for primary (first) geometry column in table
      RoleEmpty, //!< Entry is an empty entry
    };

    /**
     * Constructor for QgsDatabaseTableModel, for the specified \a provider and \a connection name.
     *
     * The optional \a schema argument can be used to restrict the tables to those from a specific schema.
     *
     * \warning The \a provider must support the connection API methods in its QgsProviderMetadata implementation
     * in order for the model to work correctly.
     */
    explicit QgsDatabaseTableModel( const QString &provider, const QString &connection, const QString &schema = QString(), QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsDatabaseTableModel, for the specified \a connection.
     *
     * The optional \a schema argument can be used to restrict the tables to those from a specific schema.
     *
     * Ownership of \a connection is transferred to the model.
     */
    explicit QgsDatabaseTableModel( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, const QString &schema = QString(), QObject *parent SIP_TRANSFERTHIS = nullptr );

    // QAbstractItemModel interface
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;

    /**
     * Sets whether an optional empty table ("not set") option is present in the model.
     * \see allowEmptyTable()
     */
    void setAllowEmptyTable( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty table ("not set") choice.
     * \see setAllowEmptyTable()
     */
    bool allowEmptyTable() const { return mAllowEmpty; }

  public slots:

    /**
     * Refreshes the table list by querying the underlying connection.
     */
    void refresh();

  private:
    void init();
    std::unique_ptr< QgsAbstractDatabaseProviderConnection > mConnection;
    QString mSchema;
    QList<QgsAbstractDatabaseProviderConnection::TableProperty> mTables;
    bool mAllowEmpty = false;
};

#endif // QGSDATABASETABLEMODEL_H
