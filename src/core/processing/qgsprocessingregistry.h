/***************************************************************************
                         qgsprocessingregistry.h
                         ------------------------
    begin                : December 2016
    copyright            : (C) 2016 by Nyall Dawson
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

#ifndef QGSPROCESSINGREGISTRY_H
#define QGSPROCESSINGREGISTRY_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingprovider.h"
#include <QMap>

class QgsProcessingParameterType;
class QgsProcessingAlgorithmConfigurationWidgetFactory;

/**
 * \class QgsProcessingAlgorithmInformation
 * \ingroup core
 * \brief Contains basic properties for a Processing algorithm.
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsProcessingAlgorithmInformation
{
  public:

    //! Algorithm display name
    QString displayName;

    //! Algorithm icon
    QIcon icon;
};


/**
 * \class QgsProcessingRegistry
 * \ingroup core
 * \brief Registry for various processing components, including providers, algorithms
 * and various parameters and outputs.
 *
 * QgsProcessingRegistry is not usually directly created, but rather accessed through
 * QgsApplication::processingRegistry().
 */
class CORE_EXPORT QgsProcessingRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingRegistry.
     */
    QgsProcessingRegistry( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsProcessingRegistry() override;

    QgsProcessingRegistry( const QgsProcessingRegistry &other ) = delete;
    QgsProcessingRegistry &operator=( const QgsProcessingRegistry &other ) = delete;

    /**
     * Gets list of available providers.
     */
    QList<QgsProcessingProvider *> providers() const { return mProviders.values(); }

    /**
     * Add a processing provider to the registry. Ownership of the provider is transferred to the registry,
     * and the provider's parent will be set to the registry.
     * Returns FALSE if the provider could not be added (eg if a provider with a duplicate ID already exists
     * in the registry).
     * Adding a provider to the registry automatically triggers the providers QgsProcessingProvider::load()
     * method to populate the provider with algorithms.
     * \see removeProvider()
     */
    bool addProvider( QgsProcessingProvider *provider SIP_TRANSFER );

    /**
     * Removes a provider implementation from the registry (the provider object is deleted).
     * Returns FALSE if the provider could not be removed (eg provider does not exist in the registry).
     * \see addProvider()
     */
    bool removeProvider( QgsProcessingProvider *provider );

    /**
     * Removes a provider implementation from the registry (the provider object is deleted).
     * Returns FALSE if the provider could not be removed (eg provider does not exist in the registry).
     * \see addProvider()
     */
    bool removeProvider( const QString &providerId );

    /**
     * Returns a matching provider by provider ID.
     */
    QgsProcessingProvider *providerById( const QString &id ) const SIP_HOLDGIL;

    /**
     * Returns a list of all available algorithms from registered providers.
     * \see algorithmById()
     */
    QList< const QgsProcessingAlgorithm *> algorithms() const;

    /**
     * Returns basic algorithm information for the algorithm with matching ID.
     *
     * This method uses an internal cache to ensure that information is quickly
     * returned and is suitable for calling many times.
     *
     * \since QGIS 3.32
     */
    QgsProcessingAlgorithmInformation algorithmInformation( const QString &id ) const;

    /**
     * Finds an algorithm by its ID. If no matching algorithm is found, NULLPTR
     * is returned.
     * \see algorithms()
     * \see createAlgorithmById()
     */
    const QgsProcessingAlgorithm *algorithmById( const QString &id ) const;

    /*
     * IMPORTANT: While it seems like /Factory/ would be the correct annotation here, that's not
     * the case.
     * As per Phil Thomson's advice on https://www.riverbankcomputing.com/pipermail/pyqt/2017-July/039450.html:
     *
     * "
     * /Factory/ is used when the instance returned is guaranteed to be new to Python.
     * In this case it isn't because it has already been seen when being returned by QgsProcessingAlgorithm::createInstance()
     * (However for a different sub-class implemented in C++ then it would be the first time it was seen
     * by Python so the /Factory/ on create() would be correct.)
     *
     * You might try using /TransferBack/ on create() instead - that might be the best compromise.
     * "
     */

    /**
     * Creates a new instance of an algorithm by its ID. If no matching algorithm is found, NULLPTR
     * is returned. Callers take responsibility for deleting the returned object.
     *
     * The \a configuration argument allows passing of a map of configuration settings
     * to the algorithm, allowing it to dynamically adjust its initialized parameters
     * and outputs according to this configuration. This is generally used only for
     * algorithms in a model, allowing them to adjust their behavior at run time
     * according to some user configuration.
     *
     * \see algorithms()
     * \see algorithmById()
     */
    QgsProcessingAlgorithm *createAlgorithmById( const QString &id, const QVariantMap &configuration = QVariantMap() ) const SIP_TRANSFERBACK;

    /**
     * Adds a new alias to an existing algorithm.
     *
     * This allows algorithms to be referred to by a different provider ID and algorithm name to their actual underlying provider and algorithm name.
     * It provides a mechanism to allow algorithms to be moved between providers without breaking existing scripts or plugins.
     *
     * The \a aliasId argument specifies the "fake" algorithm id (eg "fake_provider:fake_alg") by which the algorithm can
     * be referred to, and the \a actualId argument specifies the real algorithm ID for the algorithm.
     *
     * \since QGIS 3.10
     */
    void addAlgorithmAlias( const QString &aliasId, const QString &actualId );

    /**
     * Register a new parameter type for processing.
     * Ownership is transferred to the registry.
     * Will emit parameterTypeAdded.
     *
     * \see removeParameterType
     *
     * \since QGIS 3.2
     */
    bool addParameterType( QgsProcessingParameterType *type SIP_TRANSFER );

    /**
     * Unregister a custom parameter type from processing.
     * The type will be deleted.
     * Will emit parameterTypeRemoved.
     *
     * \see addParameterType
     *
     * \since QGIS 3.2
     */
    void removeParameterType( QgsProcessingParameterType *type );

    /**
     * Returns the parameter type registered for \a id.
     *
     * \since QGIS 3.2
     */
    QgsProcessingParameterType *parameterType( const QString &id ) const;

    /**
     * Returns a list with all known parameter types.
     *
     * \since QGIS 3.2
     */
    QList<QgsProcessingParameterType *> parameterTypes() const;

  signals:

    //! Emitted when a provider has been added to the registry.
    void providerAdded( const QString &id );

    //! Emitted when a provider is removed from the registry
    void providerRemoved( const QString &id );

    /**
     * Emitted when a new parameter type has been added to the registry.
     *
     * \since QGIS 3.2
     */
    void parameterTypeAdded( QgsProcessingParameterType *type );

    /**
     * Emitted when a parameter type has been removed from the
     * registry and is about to be deleted.
     *
     * \since QGIS 3.2
     */
    void parameterTypeRemoved( QgsProcessingParameterType *type );

  private:

    //! Map of available providers by id. This class owns the pointers
    QMap<QString, QgsProcessingProvider *> mProviders;

    //! Hash of available parameter types by id. This object owns the pointers.
    QMap<QString, QgsProcessingParameterType *> mParameterTypes;

    QMap< QString, QString > mAlgorithmAliases;

    mutable QMap< QString, QgsProcessingAlgorithmInformation > mCachedInformation;

#ifdef SIP_RUN
    QgsProcessingRegistry( const QgsProcessingRegistry &other );
#endif

    friend class TestQgsProcessing;
};

#endif // QGSPROCESSINGREGISTRY_H


