/**
 * This class allows to include a set of layers in a database-side transaction,
 * provided the layer data providers support transactions and are compatible
 * with each other.
 *
 * Only layers which are not in edit mode can be included in a transaction,
 * and all layers need to be in read-only mode for a transaction to be committed
 * or rolled back.
 *
 * Layers cannot only be included in one transaction at a time.
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
class QgsTransaction : QObject /Abstract/
{
%TypeHeaderCode
#include <qgstransaction.h>
%End
  public:
    /** Creates a transaction for the specified connection string and provider */
    static QgsTransaction* create( const QString& connString, const QString& providerKey ) /Factory/;

    /** Creates a transaction which includes the specified layers. Connection string
     *  and data provider are taken from the first layer */
    static QgsTransaction* create( const QStringList& layerIds ) /Factory/;

    virtual ~QgsTransaction();

    /** Add layer to the transaction. The layer must not be in edit mode.*/
    bool addLayer( const QString& layerId );

    /** Add layer to the transaction. The layer must not be in edit mode.*/
    bool addLayer( QgsVectorLayer* layer );

    /** Begin transaction
     *  The statement timeout, in seconds, specifies how long an sql statement
     *  is allowed to block QGIS before it is aborted. Statements can block,
     *  depending on the provider, if multiple transactions are active and a
     *  statement would produce a conflicting state. In these cases, the
     *  statements block until the conflicting transaction is committed or
     *  rolled back.
     *  Some providers might not honour the statement timeout. */
    bool begin( QString& errorMsg /Out/, int statementTimeout = 20 );

    /** Commit transaction. All layers need to be in read-only mode. */
    bool commit( QString& errorMsg /Out/ );

    /** Roll back transaction. All layers need to be in read-only mode. */
    bool rollback( QString& errorMsg /Out/ );

    /** Executes sql */
    virtual bool executeSql( const QString& sql, QString& error /Out/ ) = 0;

    /**
     * Checks if a the provider of a give layer supports transactions.
     */
    static bool supportsTransaction( const QgsVectorLayer* layer );

  signals:
    /**
     * Emitted after a rollback
     */
    void afterRollback();
};
