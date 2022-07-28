/***************************************************************************
    qgsapplayerhandling.h
    -------------------------
    begin                : July 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPLAYERHANDLING_H
#define QGSAPPLAYERHANDLING_H

#include "qgis.h"
#include "qgsconfig.h"

class QgsMapLayer;
class QgsProviderSublayerDetails;
class QgsPointCloudLayer;
class QgsRasterLayer;

/**
 * Contains logic related to general layer handling in QGIS app.
 */
class QgsAppLayerHandling
{

  public:

    enum class SublayerHandling
    {
      AskUser,
      LoadAll,
      AbortLoading
    };

    static void postProcessAddedLayer( QgsMapLayer *layer );



    static bool addVectorLayers( const QStringList &layers, const QString &enc, const QString &dataSourceType, bool guiWarning = true );


    //! Open a point cloud layer - this is the generic function which takes all parameters
    static QgsPointCloudLayer *addPointCloudLayer( const QString &uri,
        const QString &baseName,
        const QString &providerKey,
        bool guiWarning = true );

    /**
     * This method will open a dialog so the user can select GDAL sublayers to load
     * \returns TRUE if any items were loaded
     */
    static bool askUserForZipItemLayers( const QString &path, const QList< QgsMapLayerType > &acceptableTypes );


    static SublayerHandling shouldAskUserForSublayers( const QList< QgsProviderSublayerDetails > &layers, bool hasNonLayerItems = false );

    static QList< QgsMapLayer * > addSublayers( const QList< QgsProviderSublayerDetails> &layers, const QString &baseName, const QString &groupName );


    /**
     * Open a raster or vector file; ignore other files.
     * Used to process a commandline argument, FileOpen or Drop event.
     * Set \a allowInteractive to TRUE if it is OK to ask the user for information (mostly for
     * when a vector layer has sublayers and we want to ask which sublayers to use).
     * \returns TRUE if the file is successfully opened
     */
    static bool openLayer( const QString &fileName, bool allowInteractive = false );

    template<typename T> static T *addLayerPrivate( QgsMapLayerType type, const QString &uri, const QString &baseName, const QString &providerKey, bool guiWarnings = true );

    /**
     * Add a raster layer directly without prompting user for location
     * The caller must provide information compatible with the provider plugin
     * using the \a uri and \a baseName. The provider can use these
     * parameters in any way necessary to initialize the layer. The \a baseName
     * parameter is used in the Map Legend so it should be formed in a meaningful
     * way.
     */
    static QgsRasterLayer *addRasterLayer( QString const &uri, QString const &baseName, QString const &providerKey = QLatin1String( "gdal" ) );

    /**
     * Overloaded version of the private addRasterLayer()
     * Method that takes a list of file names instead of prompting
     * user with a dialog.
     * \returns TRUE if successfully added layer(s)
     */
    static bool addRasterLayers( const QStringList &files, bool guiWarning = true );

};

#endif // QGSAPPLAYERHANDLING_H
