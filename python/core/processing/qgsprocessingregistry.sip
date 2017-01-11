/**
 * \class QgsProcessingRegistry
 * \ingroup core
 * Registry for various processing components, including providers, algorithms
 * and various parameters and outputs.
 *
 * QgsProcessingRegistry is not usually directly created, but rather accessed through
 * QgsApplication::processingRegistry().
 * \note added in QGIS 3.0
 */
class QgsProcessingRegistry : QObject
{
%TypeHeaderCode
#include <qgsprocessingregistry.h>
%End

  public:

    /**
     * Constructor for QgsProcessingRegistry.
     */
    QgsProcessingRegistry( QObject* parent /TransferThis/ = nullptr );

    ~QgsProcessingRegistry();

    /**
     * Get list of available providers.
     */
    QList<QgsProcessingProvider*> providers() const;

    /**
     * Add a processing provider to the registry. Ownership of the provider is transferred to the registry.
     * Returns false if the provider could not be added (eg if a provider with a duplicate ID already exists
     * in the registry).
     * @see removeProvider()
     */
    bool addProvider( QgsProcessingProvider* provider /Transfer/ );

    /**
     * Removes a provider implementation from the registry (the provider object is deleted).
     * Returns false if the provider could not be removed (eg provider does not exist in the registry).
     * @see addProvider()
     */
    bool removeProvider( QgsProcessingProvider* provider );

    /**
     * Removes a provider implementation from the registry (the provider object is deleted).
     * Returns false if the provider could not be removed (eg provider does not exist in the registry).
     * @see addProvider()
     */
    bool removeProvider( const QString& providerId );

    /**
     * Returns a matching provider by provider ID.
     */
    QgsProcessingProvider* providerById( const QString& id );

  signals:

    //! Emitted when a provider has been added to the registry.
    void providerAdded( const QString& id );

    //! Emitted when a provider is removed from the registry
    void providerRemoved( const QString& id );

  private:

    //! Registry cannot be copied
    QgsProcessingRegistry( const QgsProcessingRegistry& other );
    //! Registry cannot be copied
    //QgsProcessingRegistry& operator=( const QgsProcessingRegistry& other );
};
