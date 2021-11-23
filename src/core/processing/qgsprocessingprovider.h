/***************************************************************************
                         qgsprocessingprovider.h
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

#ifndef QGSPROCESSINGPROVIDER_H
#define QGSPROCESSINGPROVIDER_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include <QIcon>

/**
 * \class QgsProcessingProvider
 * \ingroup core
 * \brief Abstract base class for processing providers.
 *
 * An algorithm provider is a set of related algorithms, typically from the same external application or related
 * to a common area of analysis.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingProvider : public QObject
{
    Q_OBJECT

  public:

    /**
     * Flags indicating how and when an provider operates and should be exposed to users
     * \since QGIS 3.14
     */
    enum Flag
    {
      FlagDeemphasiseSearchResults = 1 << 1, //!< Algorithms should be de-emphasised in the search results when searching for algorithms. Use for low-priority providers or those with substantial known issues.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsProcessingProvider.
     */
    QgsProcessingProvider( QObject *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsProcessingProvider() override;

    //! Providers cannot be copied
    QgsProcessingProvider( const QgsProcessingProvider &other ) = delete;
    //! Providers cannot be copied
    QgsProcessingProvider &operator=( const QgsProcessingProvider &other ) = delete;

    /**
     * Returns an icon for the provider.
     * \see svgIconPath()
     */
    virtual QIcon icon() const;

    /**
     * Returns a path to an SVG version of the provider's icon.
     * \see icon()
     */
    virtual QString svgIconPath() const;

    /**
     * Returns the flags indicating how and when the provider operates and should be exposed to users.
     * Default is no flags.
     * \since QGIS 3.14
     */
    virtual Flags flags() const;

    /**
     * Returns the unique provider id, used for identifying the provider. This string
     * should be a unique, short, character only string, eg "qgis" or "gdal". This
     * string should not be localised.
     * \see name()
     * \see helpId()
     */
    virtual QString id() const = 0;

    /**
     * Returns the provider help id string, used for creating QgsHelp urls for algorithms
     * belong to this provider. By default, this returns an empty string, meaning that
     * no QgsHelp url should be created for the provider's algorithms.
     * \see id()
     */
    virtual QString helpId() const;

    /**
     * Returns the provider name, which is used to describe the provider within the GUI.
     * This string should be short (e.g. "Lastools") and localised.
     * \see longName()
     * \see id()
     */
    virtual QString name() const = 0;

    /**
     * Returns the longer version of the provider name, which can include extra details
     * such as version numbers. E.g. "Lastools LIDAR tools (version 2.2.1)".
     * This string should be localised.
     *
     * The default implementation returns the same string as name().
     *
     * \see name()
     * \see id()
     */
    virtual QString longName() const;

    /**
     * Returns a version information string for the provider, or an empty string if this
     * is not applicable (e.g. for inbuilt Processing providers).
     *
     * For plugin based providers, this should return the plugin's version identifier.
     *
     * \since QGIS 3.8
     */
    virtual QString versionInfo() const;

    /**
     * Returns TRUE if the provider can be activated, or FALSE if it cannot be activated (e.g. due to
     * missing external dependencies).
     * \see isActive()
     */
    virtual bool canBeActivated() const { return true; }

    /**
     * Returns an optional warning message to show users when running algorithms from this provider.
     *
     * This can be used to return a translated warning message which should be shown to users
     * of this provider. It's intended for use in cases such as a provider which relies on a 3rd-party
     * backend, where the version of the backend software is not officially supported, or for
     * alerting users to providers in a "beta" or "untrustworthy" state.
     *
     * \since QGIS 3.10.1
     */
    virtual QString warningMessage() const { return QString(); }

    /**
     * Returns TRUE if the provider is active and able to run algorithms.
     */
    virtual bool isActive() const { return true; }

    /**
     * Returns a list of the raster format file extensions supported by this provider.
     * \see supportedOutputVectorLayerExtensions()
     */
    virtual QStringList supportedOutputRasterLayerExtensions() const;

    /**
     * Returns a list of the vector format file extensions supported by this provider.
     * \see supportedOutputTableExtensions()
     * \see defaultVectorFileExtension()
     * \see supportedOutputRasterLayerExtensions()
     * \see supportsNonFileBasedOutput()
     * \see supportedOutputPointCloudLayerExtensions()
     */
    virtual QStringList supportedOutputVectorLayerExtensions() const;

    /**
     * Returns a list of the point cloud format file extensions supported by this provider.
     * \see supportedOutputVectorLayerExtensions()
     * \see supportedOutputRasterLayerExtensions()
     * \see supportedOutputTableExtensions()
     * \see defaultVectorFileExtension()
     * \see supportsNonFileBasedOutput()
     *
     * \since QGIS 3.24
     */
    virtual QStringList supportedOutputPointCloudLayerExtensions() const;

    /**
     * Returns a list of the table (geometry-less vector layers) file extensions supported by this provider.
     *
     * By default this is the same as supportedOutputVectorLayerExtensions(). Providers which utilize different
     * formats for geometry-less layers can override this method to return a different list of supported formats.
     *
     * \see supportedOutputVectorLayerExtensions()
     * \see defaultVectorFileExtension()
     * \see supportedOutputRasterLayerExtensions()
     * \see supportsNonFileBasedOutput()
     * \see supportedOutputPointCloudLayerExtensions()
     *
     * \since QGIS 3.4.3
     */
    virtual QStringList supportedOutputTableExtensions() const;

    /**
     * Returns TRUE if the specified \a outputValue is of a supported file format for the given destination \a parameter.
     *
     * If the output value is not supported, \a error will be set to a descriptive message explaining why.
     *
     * \since QGIS 3.6
    */
    virtual bool isSupportedOutputValue( const QVariant &outputValue, const QgsProcessingDestinationParameter *parameter, QgsProcessingContext &context, QString &error SIP_OUT ) const;

    /**
     * Returns the default file extension to use for vector outputs created by the
     * provider.
     *
     * If \a hasGeometry is TRUE then the output file format must have support for
     * geometry. If \a hasGeometry is FALSE then non-spatial formats can be used.
     *
     * The default implementation returns the user's default Processing vector output format
     * setting, if it's supported by the provider (see supportedOutputVectorLayerExtensions()).
     * Otherwise the first reported supported vector format will be used.
     *
     * \see supportedOutputVectorLayerExtensions()
     * \see defaultRasterFileExtension()
     * \see defaultPointCloudFileExtension()
     */
    virtual QString defaultVectorFileExtension( bool hasGeometry = true ) const;

    /**
     * Returns the default file extension to use for raster outputs created by the
     * provider.
     *
     * The default implementation returns the user's default Processing raster output format
     * setting, if it's supported by the provider (see supportedOutputRasterLayerExtensions()).
     * Otherwise the first reported supported raster format will be used.
     *
     * \see supportedOutputRasterLayerExtensions()
     * \see defaultVectorFileExtension()
     * \see defaultPointCloudFileExtension()
     */
    virtual QString defaultRasterFileExtension() const;

    /**
     * Returns the default file extension to use for point cloud outputs created by the
     * provider.
     *
     * The default implementation returns the user's default Processing point cloud output format
     * setting, if it's supported by the provider (see supportedOutputPointCloudLayerExtensions()).
     * Otherwise the first reported supported point cloud format will be used.
     *
     * \see supportedOutputPointCloudLayerExtensions()
     * \see defaultVectorFileExtension()
     * \see defaultRasterFileExtension()
     *
     * \since QGIS 3.24
     */
    virtual QString defaultPointCloudFileExtension() const;

    /**
     * Returns TRUE if the provider supports non-file based outputs (such as memory layers
     * or direct database outputs). If a provider returns FALSE for this method than it
     * indicates that none of the outputs from any of the provider's algorithms have
     * support for non-file based outputs. Returning TRUE indicates that the algorithm's
     * parameters will each individually declare their non-file based support.
     *
     * The default behavior for providers is to support non-file based outputs, and most
     * providers which rely solely on QGIS API (and which do not depend on third-party scripts
     * or external dependencies) will automatically support this.
     *
     * \see supportedOutputVectorLayerExtensions()
     */
    virtual bool supportsNonFileBasedOutput() const;

    /**
     * Loads the provider. This will be called when the plugin is being loaded, and any general
     * setup actions should occur in an overridden version of this method.
     * Subclasses should not individually load any algorithms in their load() implementations, as that must
     * occur within the loadAlgorithms() method. Instead, subclasses should call refreshAlgorithms()
     * from any overloaded load() method to trigger an initial load of the provider's algorithms.
     * \returns TRUE if provider could be successfully loaded
     * \see unload()
     */
    virtual bool load() { refreshAlgorithms(); return true; }

    /**
     * Unloads the provider. Any tear-down steps required by the provider should be implemented here.
     * \see load()
     */
    virtual void unload() {}

    /**
     * Refreshes the algorithms available from the provider, causing it to re-populate with all associated algorithms.
     */
    void refreshAlgorithms();

    /**
     * Returns a list of algorithms supplied by this provider.
     * \see algorithm()
     */
    QList< const QgsProcessingAlgorithm * > algorithms() const;

    /**
     * Returns the matching algorithm by \a name, or NULLPTR if no matching
     * algorithm is contained by this provider.
     * \see algorithms()
     */
    const QgsProcessingAlgorithm *algorithm( const QString &name ) const;

  signals:

    /**
     * Emitted when the provider has loaded (or refreshed) its list of available
     * algorithms.
     * \see refreshAlgorithms()
     */
    void algorithmsLoaded();

  protected:

    /**
     * Loads all algorithms belonging to this provider. Subclasses should implement this, calling
     * addAlgorithm() to register all their associated algorithms.
     */
    virtual void loadAlgorithms() = 0;

    /**
     * Adds an \a algorithm to the provider. Ownership of the algorithm is transferred to the provider.
     */
    bool addAlgorithm( QgsProcessingAlgorithm *algorithm SIP_TRANSFER );

  private:

    QMap< QString, const QgsProcessingAlgorithm * > mAlgorithms;

#ifdef SIP_RUN
    QgsProcessingProvider( const QgsProcessingProvider &other );
#endif
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingProvider::Flags )

#endif // QGSPROCESSINGPROVIDER_H


