/***************************************************************************
   qgsproviderconnectionmodel.h
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

#ifndef QGSPROVIDERCONNECTIONMODEL_H
#define QGSPROVIDERCONNECTIONMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsProviderMetadata;

/**
 * \ingroup core
 * \class QgsProviderConnectionModel
 * \brief A model containing registered connection names for a specific data provider.
 *
 * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
 * in order for the model to work correctly.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsProviderConnectionModel : public QAbstractItemModel
{
    Q_OBJECT

  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProviderConnectionModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProviderConnectionModel, Role ) : int
    {
      ConnectionName SIP_MONKEYPATCH_COMPAT_NAME(RoleConnectionName) = Qt::UserRole, //!< Connection name
      Uri SIP_MONKEYPATCH_COMPAT_NAME(RoleUri), //!< Connection URI string
      Configuration SIP_MONKEYPATCH_COMPAT_NAME(RoleConfiguration), //!< Connection configuration variant map
      Empty SIP_MONKEYPATCH_COMPAT_NAME(RoleEmpty), //!< Entry is an empty entry
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsProviderConnectionModel, for the specified \a provider.
     *
     * \warning The \a provider must support the connection API methods in its QgsProviderMetadata implementation
     * in order for the model to work correctly.
     */
    explicit QgsProviderConnectionModel( const QString &provider, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets whether an optional empty connection ("not set") option is present in the model.
     * \see allowEmptyConnection()
     */
    void setAllowEmptyConnection( bool allowEmpty );

    /**
     * Returns TRUE if the model allows the empty connection ("not set") choice.
     * \see setAllowEmptyConnection()
     */
    bool allowEmptyConnection() const { return mAllowEmpty; }

    // QAbstractItemModel interface
    QModelIndex parent( const QModelIndex &child ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column, const QModelIndex &parent ) const override;
  private slots:
    void removeConnection( const QString &connection );
    void addConnection( const QString &connection );

  private:
    QString mProvider;
    QgsProviderMetadata *mMetadata = nullptr;
    QStringList mConnections;
    bool mAllowEmpty = false;
};

#endif // QGSPROVIDERCONNECTIONMODEL_H
