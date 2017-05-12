/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstransaction.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsTransaction : QObject /Abstract/
{
%Docstring
 This class allows including a set of layers in a database-side transaction,
 provided the layer data providers support transactions and are compatible
 with each other.

 Only layers which are not in edit mode can be included in a transaction,
 and all layers need to be in read-only mode for a transaction to be committed
 or rolled back.

 Layers cannot only be included in one transaction at a time.

 When editing layers which are part of a transaction group, all changes are
 sent directly to the data provider (bypassing the undo/redo stack), and the
 changes can either be committed or rolled back on the database side via the
 QgsTransaction.commit and QgsTransaction.rollback methods.

 As long as the transaction is active, the state of all layer features reflects
 the current state in the transaction.

 Edits on features can get rejected if another conflicting transaction is active.
%End

%TypeHeaderCode
#include "qgstransaction.h"
%End
  public:

    static QgsTransaction *create( const QString &connString, const QString &providerKey ) /Factory/;
%Docstring
 Create a transaction for the specified connection string ``connString``
 and provider with ``providerKey``.
 :rtype: QgsTransaction
%End

    static QgsTransaction *create( const QStringList &layerIds ) /Factory/;
%Docstring
 Create a transaction which includes the layers specified with
 ``layerIds``.
 All layers are expected to have the same connection string and data
 provider.
 :rtype: QgsTransaction
%End

    virtual ~QgsTransaction();

    bool addLayer( const QString &layerId );
%Docstring
 Add the layer with ``layerId`` to the transaction. The layer must not be
 in edit mode and the connection string must match.
 :rtype: bool
%End

    bool addLayer( QgsVectorLayer *layer );
%Docstring
 Add the ``layer`` to the transaction. The layer must not be
 in edit mode and the connection string must match.
 :rtype: bool
%End

    bool begin( QString &errorMsg /Out/, int statementTimeout = 20 );
%Docstring
 Begin transaction
 The ``statementTimeout`` (in seconds) specifies how long an sql statement
 is allowed to block QGIS before it is aborted.
 Statements can block, if multiple transactions are active and a
 statement would produce a conflicting state. In these cases, the
 statements block until the conflicting transaction is committed or
 rolled back.
 Some providers might not honour the statement timeout.
 :rtype: bool
%End

    bool commit( QString &errorMsg /Out/ );
%Docstring
 Commit transaction.
 :rtype: bool
%End

    bool rollback( QString &errorMsg /Out/ );
%Docstring
 Roll back transaction.
 :rtype: bool
%End

    virtual bool executeSql( const QString &sql, QString &error /Out/ ) = 0;
%Docstring
 Execute the ``sql`` string. The result must not be a tuple, so running a
 ``SELECT`` query will return an error.
 :rtype: bool
%End

    static bool supportsTransaction( const QgsVectorLayer *layer );
%Docstring
 Checks if a the provider of a given ``layer`` supports transactions.
 :rtype: bool
%End

  signals:

    void afterRollback();
%Docstring
 Emitted after a rollback
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgstransaction.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
