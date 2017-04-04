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
 * Abstract base class for processing providers. An algorithm provider is a set of
 * related algorithms, typically from the same external application or related
 * to a common area of analysis.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsProcessingProvider : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingProvider.
     */
    QgsProcessingProvider( QObject *parent SIP_TRANSFERTHIS = nullptr );

    virtual ~QgsProcessingProvider();

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
     * Returns the unique provider id, used for identifying the provider. This string
     * should be a unique, short, character only string, eg "qgis" or "gdal". This
     * string should not be localised.
     * \see name()
     */
    virtual QString id() const = 0;

    /**
     * Returns the full provider name, which is used to describe the provider within the GUI.
     * This string should be localised.
     * \see id()
     */
    virtual QString name() const = 0;

    /**
     * Returns true if the provider can be activated, or false if it cannot be activated (e.g. due to
     * missing external dependencies).
     */
    virtual bool canBeActivated() const { return true; }

    /**
     * Returns a list of the raster format file extensions supported by this provider.
     * \see supportedOutputVectorLayerExtensions()
     * \see supportedOutputTableExtensions()
     */
    virtual QStringList supportedOutputRasterLayerExtensions() const { return QStringList() << QStringLiteral( "tif" ); }

    /**
     * Returns a list of the vector format file extensions supported by this provider.
     * \see supportedOutputRasterLayerExtensions()
     * \see supportedOutputTableExtensions()
     * \see supportsNonFileBasedOutput()
     */
    virtual QStringList supportedOutputVectorLayerExtensions() const;

    /**
     * Returns a list of the table format file extensions supported by this provider.
     * \see supportedOutputRasterLayerExtensions()
     * \see supportedOutputVectorLayerExtensions()
     */
    virtual QStringList supportedOutputTableExtensions() const { return QStringList() << QStringLiteral( "csv" ); }

    /**
     * Returns true if the provider supports non-file based outputs (such as memory layers
     * or direct database outputs).
     * \see supportedOutputVectorLayerExtensions()
     */
    virtual bool supportsNonFileBasedOutput() const { return false; }

    /**
     * Loads the provider. This will be called when the plugin is being loaded, and any general
     * setup actions should occur in an overridden version of this method.
     * Subclasses should not load any algorithms in their load() implementations, as that must
     * occur within the loadAlgorithms() method.
     * \returns true if provider could be successfully loaded
     * \see unload()
     */
    virtual bool load() { return true; }

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
    QList< QgsProcessingAlgorithm * > algorithms() const;

    /**
     * Returns the matching algorithm by \a name, or a nullptr if no matching
     * algorithm is contained by this provider.
     * \see algorithms()
     */
    QgsProcessingAlgorithm *algorithm( const QString &name ) const;

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

    QMap< QString, QgsProcessingAlgorithm * > mAlgorithms;

#ifdef SIP_RUN
    QgsProcessingProvider( const QgsProcessingProvider &other );
#endif
};

#endif // QGSPROCESSINGPROVIDER_H


