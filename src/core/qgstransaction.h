/***************************************************************************
                              qgstransaction.h
                              ----------------
  begin                : May 5, 2014
  copyright            : (C) 2014 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTRANSACTION_H
#define QGSTRANSACTION_H

#include <QSet>
#include "qgis_sip.h"
#include <QString>
#include <QObject>
#include <QStack>

#include "qgis_core.h"
#include "qgis_sip.h"

class QgsVectorDataProvider;
class QgsVectorLayer;

/**
 * \ingroup core
 * \brief This class allows including a set of layers in a database-side transaction,
 * provided the layer data providers support transactions and are compatible
 * with each other.
 *
 * Only layers which are not in edit mode can be included in a transaction,
 * and all layers need to be in read-only mode for a transaction to be committed
 * or rolled back.
 *
 * Layers can only be included in one transaction at a time.
 *
 * When editing layers which are part of a transaction group, all changes are
 * sent directly to the data provider (bypassing the undo/redo stack), and the
 * changes can either be committed or rolled back on the database side via the
 * QgsTransaction::commit and QgsTransaction::rollback methods.
 *
 * As long as the transaction is active, the state of all layer features reflects
 * the current state in the transaction.
 *
 * Edits on features can get rejected if another conflicting transaction is active.
 */

class CORE_EXPORT QgsTransaction : public QObject SIP_ABSTRACT
{
    Q_OBJECT

  public:

    /**
     * Create a transaction for the specified connection string \a connString
     * and provider with \a providerKey.
     */
    static QgsTransaction *create( const QString &connString, const QString &providerKey ) SIP_FACTORY;

    /**
     * Create a transaction which includes the \a layers.
     * All layers are expected to have the same connection string and data
     * provider.
     */
    static QgsTransaction *create( const QSet<QgsVectorLayer *> &layers ) SIP_FACTORY;

    ~QgsTransaction() override;

    /**
     * Returns the connection string of the transaction
     * \since QGIS 3.26
     */
    QString connectionString() const;

    /**
     * Add the \a layer to the transaction. The connection string
     * must match.
     * \param layer that will be added to the transaction
     * \param addLayersInEditMode if set layers that are already
     * in edit mode can be added to the transaction \since QGIS 3.26
     */
    bool addLayer( QgsVectorLayer *layer, bool addLayersInEditMode = false );

    /**
     * Begin transaction
     * The \a statementTimeout (in seconds) specifies how long an sql statement
     * is allowed to block QGIS before it is aborted.
     * Statements can block, if multiple transactions are active and a
     * statement would produce a conflicting state. In these cases, the
     * statements block until the conflicting transaction is committed or
     * rolled back.
     * Some providers might not honour the statement timeout.
     */
    bool begin( QString &errorMsg SIP_OUT, int statementTimeout = 20 );

    /**
     * Commit transaction.
     */
    bool commit( QString &errorMsg SIP_OUT );

    /**
     * Roll back transaction.
     */
    bool rollback( QString &errorMsg SIP_OUT );

    /**
     * Execute the \a sql string.
     *
     * \param sql The sql query to execute
     * \param error The error message
     * \param isDirty Flag to indicate if the underlying data will be modified
     * \param name Name of the transaction ( only used if `isDirty` is TRUE)
     *
     * \returns TRUE if everything is OK, FALSE otherwise
     */
    virtual bool executeSql( const QString &sql, QString &error SIP_OUT, bool isDirty = false, const QString &name = QString() ) = 0;

    /**
     * Checks if the provider of a given \a layer supports transactions.
     */
    static bool supportsTransaction( const QgsVectorLayer *layer );

    /**
     * creates a save point
     * returns empty string on error
     * returns the last created savepoint if it's not dirty
     * \since QGIS 3.0
     */
    QString createSavepoint( QString &error SIP_OUT );

    /**
     * creates a save point
     * returns empty string on error
     * \since QGIS 3.0
     */
    virtual QString createSavepoint( const QString &savePointId, QString &error SIP_OUT );

    /**
     * rollback to save point, the save point is maintained and is "undertied"
     * \since QGIS 3.0
     */
    virtual bool rollbackToSavepoint( const QString &name, QString &error SIP_OUT );

    /**
     * dirty save point such that next call to createSavepoint will create a new one
     * \since QGIS 3.0
     */
    void dirtyLastSavePoint();

    /**
     * returns savepoints
     * \since QGIS 3.0
     */
    QList< QString > savePoints() const { return QList< QString >::fromVector( mSavepoints ); }

    /**
     * returns the last created savepoint
     * \since QGIS 3.0
     */
    bool lastSavePointIsDirty() const { return mLastSavePointIsDirty; }

///@cond PRIVATE
    // For internal use only, or by QgsTransactionGroup
    static QString connectionString( const QString &layerUri ) SIP_SKIP;
///@endcond

  signals:

    /**
     * Emitted after a rollback
     */
    void afterRollback();

    /**
     * Emitted if a sql query is executed and the underlying data is modified
     */
    void dirtied( const QString &sql, const QString &name );

  protected:
    QgsTransaction( const QString &connString ) SIP_SKIP;

    QString mConnString;
    bool mTransactionActive;
    QStack< QString > mSavepoints;
    bool mLastSavePointIsDirty;

  private slots:
    void onLayerDeleted();

  private:

    QSet<QgsVectorLayer *> mLayers;

    void setLayerTransactionIds( QgsTransaction *transaction );

    static QString removeLayerIdOrName( const QString &str );

    virtual bool beginTransaction( QString &error, int statementTimeout ) = 0;
    virtual bool commitTransaction( QString &error ) = 0;
    virtual bool rollbackTransaction( QString &error ) = 0;
};

#endif // QGSTRANSACTION_H
