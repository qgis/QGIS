/***************************************************************************
                             qgsmaplayerutils.h
                             -------------------
    begin                : May 2021
    copyright            : (C) 2021 Nyall Dawson
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
#ifndef QGSMAPLAYERUTILS_H
#define QGSMAPLAYERUTILS_H

#define MAXIMUM_OPENSTREETMAP_TILES_FETCH 5000

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayer.h"

class QgsRectangle;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransformContext;
class QgsAbstractDatabaseProviderConnection;
class QgsGeometry;

/**
 * \ingroup core
 * \brief Contains utility functions for working with map layers.
 * \since QGIS 3.20
*/
class CORE_EXPORT QgsMapLayerUtils
{
  public:
    /**
     * Returns the combined extent of a list of \a layers.
     *
     * The \a crs argument specifies the desired coordinate reference system for the combined extent.
     */
    static QgsRectangle combinedExtent( const QList<QgsMapLayer *> &layers, const QgsCoordinateReferenceSystem &crs, const QgsCoordinateTransformContext &transformContext );

    /**
     * Creates and returns the (possibly NULLPTR) database connection for a \a layer.
     * Ownership is transferred to the caller.
     * \since QGIS 3.22
     */
    static QgsAbstractDatabaseProviderConnection *databaseConnection( const QgsMapLayer *layer ) SIP_FACTORY;

    /**
     * Returns TRUE if the source of the specified \a layer matches the given \a path.
     *
     * This method can be used to test whether a layer is associated with a file path.
     *
     * \since QGIS 3.22
     */
    static bool layerSourceMatchesPath( const QgsMapLayer *layer, const QString &path );

    /**
     * Returns TRUE if a \a layer and \a uri point to the same resource at the specified hierarchy \a level.
     *
     * This method parses the underlying connection parameters of the \a layer and \a uri
     * to check if they share the same scope defined by \a level.
     *
     * \note This method is only valid for a \a uri from the same data provider as \a layer.
     *
     * \warning Not all providers implement this functionality. Check whether the provider's metadata capabilities returns the
     * QgsProviderMetadata::ProviderMetadataCapability::UrisReferToSame to determine whether a specific provider metadata object
     * supports this method.
     *
     * \throws QgsNotSupportedException if the layer's provider does not implement the QgsProviderMetadata::ProviderMetadataCapability::UrisReferToSame capability.
     *
     * \since QGIS 4.0
     */
    static bool layerRefersToUri( const QgsMapLayer *layer, const QString &uri, Qgis::SourceHierarchyLevel level = Qgis::SourceHierarchyLevel::Object ) SIP_THROW( QgsNotSupportedException );

    /**
     * Updates a \a layer's data source, replacing its data source with a path referring to \a newPath.
     *
     * Returns TRUE if the layer was updated, or FALSE if the layer was not updated (e.g. it
     * uses a data provider which does not specify paths in a layer URI.
     *
     * \since QGIS 3.22
     */
    static bool updateLayerSourcePath( QgsMapLayer *layer, const QString &newPath );

    /**
     * Sorts a list of map \a layers by their layer type, respecting the \a order of types specified.
     *
     * Layer types which appear earlier in the \a order list will result in matching layers appearing earlier in the
     * result list.
     *
     * \since QGIS 3.26
     */
    static QList< QgsMapLayer * > sortLayersByType( const QList< QgsMapLayer * > &layers, const QList< Qgis::LayerType > &order );

    /**
     * Launders a layer's name, converting it into a format which is general suitable for
     * file names or database layer names.
     *
     * Specifically this method:
     *
     * - Converts the name to lowercase
     * - Replaces spaces by underscore characters
     * - Removes any characters which are not alphanumeric or '_'.
     *
     * \since QGIS 3.28
     */
    static QString launderLayerName( const QString &name );

    /**
     * Returns TRUE if the \a layer is served by OpenStreetMap server.
     *
     * \since QGIS 3.40
     */
    static bool isOpenStreetMapLayer( QgsMapLayer *layer );

    /**
     * Returns TRUE if the \a uri for a given \a provider is served by OpenStreetMap server.
     *
     * \since QGIS 4.2
     */
    static bool isOpenStreetMapUri( const QString &uri, const QString &providerType );

    /**
     * Returns the translated name of the type for a given layer type.
     *
     * \since QGIS 4.0
     */
    static QString layerTypeToString( Qgis::LayerType type );

    /**
     * Returns the consistent tooltip for the given layer.
     *
     * \since QGIS 4.2
     */
    static QString layerToolTip( const QgsMapLayer *layer );

    /**
     * Saves QML and SLD representations of \a layer's style to a table in the database
     * identified by \a providerKey and associates it with \a targetDataSource which can
     * be different from the layer's own data source.
     *
     * Unlike QgsMapLayer::saveStyleToDatabaseV2(), this method allows saving the style
     * from one layer to a different layer in a database. The target layer is identified
     *by the \a targetDataSource argument, which is a URI that specifies the layer in
     * the database to which the style should be associated.
     *
     * \param layer Source layer whose style will be exported.
     * \param providerKey Provider key for the target database.
     * \param targetDataSource URI of the target layer in the database.
     * \param name Style name.
     * \param description A description of the style.
     * \param useAsDefault Set to TRUE if style should be used as the default style for the layer.
     * \param uiFileContent Optional UI file content associated with the style.
     * \param formats Flag specifying which style formats to save.
     * \param categories The style categories to be saved.
     * \returns Flags representing whether QML or SLD storing was successful.
     *
     * \since QGIS 4.2
     */
    static QgsSaveStyleResult saveLayerStyleToDatabase(
      QgsMapLayer *layer,
      const QString &providerKey,
      const QString &targetDataSource,
      const QString &name,
      const QString &description,
      bool useAsDefault,
      const QString &uiFileContent,
      const Qgis::SaveStyleFormats formats = Qgis::SaveStyleFormats( Qgis::SaveStyleFormat::QML | Qgis::SaveStyleFormat::SLD ),
      QgsMapLayer::StyleCategories categories = QgsMapLayer::AllStyleCategories
    );
};

/**
 * \ingroup core
 * \brief Represents the result of saving a style.
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsSaveStyleResult
{
  public:
    QgsMapLayer::SaveStyleResults saveResult;
    QStringList sldErrorMessages;
    QStringList sldWarningMessages;
    QString providerSaveStyleError;
    QString qmlError;
};
#endif // QGSMAPLAYERUTILS_H
