/***************************************************************************
   qgsdatabaseschemamodel.h
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

#ifndef QGSDATABASESCHEMAMODEL_H
#define QGSDATABASESCHEMAMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>
#include <memory>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsProviderMetadata;
class QgsAbstractDatabaseProviderConnection;

/**
 * \ingroup core
 * \class QgsDatabaseSchemaModel
 * \brief A model containing schemas from a database connection.
 *
 * This class does not automatically subscribe to database updates. Schemas are queried
 * from the database initially upon model construction. In order
 * to update the listed schemas, QgsDatabaseSchemaModel::refresh() must be manually
 * called.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsDatabaseSchemaModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsDatabaseSchemaModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsDatabaseSchemaModel, Role ) : int
    {
      Empty SIP_MONKEYPATCH_COMPAT_NAME(RoleEmpty) = Qt::UserRole, //!< Entry is an empty entry
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsDatabaseSchemaModel, for the specified \a provider and \a connection name.
     *
     * \warning The \a provider must support the connection API methods in its QgsProviderMetadata implementation
     * in order for the model to work correctly.
     */
    explicit QgsDatabaseSchemaModel( const QString &provider, const QString &connection, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsDatabaseSchemaModel, for the specified \a connection.
     *
     * Ownership of \a connection is transferred to the model.
     */
    explicit QgsDatabaseSchemaModel( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QObject *parent SIP_TRANSFERTHIS = nullptr );

    // QAbstractItemModel interface
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;

    /**
     * Sets whether an optional empty schema ("not set") option is present in the model.
     * \see allowEmptySchema()
     */
    void setAllowEmptySchema( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty schema ("not set") choice.
     * \see setAllowEmptySchema()
     */
    bool allowEmptySchema() const { return mAllowEmpty; }

  public slots:

    /**
     * Refreshes the schema list by querying the underlying connection.
     */
    void refresh();

  private:
    void init();
    std::unique_ptr< QgsAbstractDatabaseProviderConnection > mConnection;
    QStringList mSchemas;
    bool mAllowEmpty = false;
};

#endif // QGSDATABASESCHEMAMODEL_H
