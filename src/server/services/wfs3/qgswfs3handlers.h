/***************************************************************************
                              qgswfs3handlers.h
                              -------------------------
  begin                : May 3, 2019
  copyright            : (C) 2019 by Alessandro Pasotti
  email                : elpaso at itopen dot it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_WFS3_HANDLERS_H
#define QGS_WFS3_HANDLERS_H

#include "qgsfields.h"
#include "qgsserverogcapihandler.h"

class QgsFeatureRequest;
class QgsServerOgcApi;
class QgsFeature;

/**
 * The QgsWfs3AbstractItemsHandler class provides some
 * functionality which is common to the handlers that
 * return or process items.
 */
class QgsWfs3AbstractItemsHandler : public QgsServerOgcApiHandler
{
  public:
    /**
     * Checks if the layer is published in WFS (and perform additional checks for access
     * control if plugins are enabled)
     * and throws an exception if it is not.
     * \param layer the map layer
     * \param context the server api context
     * \throws QgsServerApiNotFoundException if the layer is NOT published
     */
    void checkLayerIsAccessible( QgsVectorLayer *layer, const QgsServerApiContext &context ) const;

    /**
     * Creates a filtered QgsFeatureRequest containing only fields published for WFS and plugin filters applied.
     * \param layer the vector layer
     * \param context the server api context
     * \param subsetAttributes optional list of field names requested by the client, if empty all published attributes will be added to the request
     * \return QgsFeatureRequest with filters applied
     */
    QgsFeatureRequest filteredRequest( const QgsVectorLayer *layer, const QgsServerApiContext &context, const QStringList &subsetAttributes = QStringList() ) const;

    /**
     * Returns a filtered list of fields containing only fields published for WFS and plugin filters applied.
     * \param layer the vector layer
     * \param context the server api context
     * \return QgsFields list with filters applied
     */
    QgsFields publishedFields( const QgsVectorLayer *layer, const QgsServerApiContext &context ) const;

    /**
     * Returns the HTML template path for the handler in the given \a context
     *
     * The template path is calculated from QgsServerSettings's apiResourcesDirectory() as follow:
     * apiResourcesDirectory() + "/ogc/templates/wfs3/" + operationId + ".html"
     * e.g. for an handler with operationId "collectionItems", the path
     * will be apiResourcesDirectory() + "/ogc/templates/wfs3/collectionItems.html"
     */
    [[nodiscard]] const QString templatePath( const QgsServerApiContext &context ) const override;
};

/**
 * The APIHandler class Wfs3handles the API definition
 */
class QgsWfs3APIHandler : public QgsWfs3AbstractItemsHandler
{
  public:
    QgsWfs3APIHandler( const QgsServerOgcApi *api );

    // QgsServerOgcApiHandler interface
    void handleRequest( const QgsServerApiContext &context ) const override;
    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re(/api)re" ); }
    [[nodiscard]] std::string operationId() const override { return "getApiDescription"; }
    [[nodiscard]] std::string summary() const override { return "The API description"; }
    [[nodiscard]] std::string description() const override { return "The formal documentation of this API according to the OpenAPI specification, version 3.0. I.e., this document."; }
    [[nodiscard]] std::string linkTitle() const override { return "API description"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Capabilities" ) }; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::service_desc; }
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;

  private:
    const QgsServerOgcApi *mApi = nullptr;
};


/**
 * The QgsWfs3LandingPageHandler is the landing page handler.
 */
class QgsWfs3LandingPageHandler : public QgsServerOgcApiHandler
{
  public:
    QgsWfs3LandingPageHandler();

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re((.html|.json)?$)re" ); }
    [[nodiscard]] std::string operationId() const override { return "getLandingPage"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Capabilities" ) }; }
    [[nodiscard]] std::string summary() const override
    {
      return "WFS 3.0 Landing Page";
    }
    [[nodiscard]] std::string description() const override
    {
      return "The landing page provides links to the API definition, the Conformance "
             "statements and the metadata about the feature data in this dataset.";
    }
    [[nodiscard]] std::string linkTitle() const override { return "Landing page"; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::self; }
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;
};

/**
 * The QgsWfs3ConformanceHandler class shows the conformance links.
 */
class QgsWfs3ConformanceHandler : public QgsServerOgcApiHandler
{
  public:
    QgsWfs3ConformanceHandler();

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re(/conformance)re" ); }
    [[nodiscard]] std::string operationId() const override { return "getRequirementClasses"; }
    [[nodiscard]] std::string summary() const override { return "Information about standards that this API conforms to."; }
    [[nodiscard]] std::string description() const override
    {
      return "List all requirements classes specified in a standard (e.g., WFS 3.0 "
             "Part 1: Core) that the server conforms to.";
    }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Capabilities" ) }; }
    [[nodiscard]] std::string linkTitle() const override { return "WFS 3.0 conformance classes"; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::conformance; }
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;
};


/**
 * The CollectionsHandler lists all available collections for the current project
 * Path: /collections
 */
class QgsWfs3CollectionsHandler : public QgsWfs3AbstractItemsHandler
{
  public:
    QgsWfs3CollectionsHandler();

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re(/collections(\.json|\.html|/)?$)re" ); }
    [[nodiscard]] std::string operationId() const override { return "describeCollections"; }
    [[nodiscard]] std::string summary() const override
    {
      return "Metadata about the feature collections shared by this API.";
    }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Capabilities" ) }; }
    [[nodiscard]] std::string description() const override
    {
      return "Describe the feature collections in the dataset "
             "statements and the metadata about the feature data in this dataset.";
    }
    [[nodiscard]] std::string linkTitle() const override { return "Feature collections"; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;
};

/**
 * The DescribeCollectionHandler describes a single collection
 * Path: /collections/{collectionId}
 */
class QgsWfs3DescribeCollectionHandler : public QgsWfs3AbstractItemsHandler
{
  public:
    QgsWfs3DescribeCollectionHandler();
    void handleRequest( const QgsServerApiContext &context ) const override;

    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re(/collections/(?<collectionId>[^/]+?)(\.json|\.html|/)?$)re" ); }
    [[nodiscard]] std::string operationId() const override { return "describeCollection"; }
    [[nodiscard]] std::string summary() const override { return "Describe the feature collection with ID {collectionId}."; }
    [[nodiscard]] std::string description() const override { return "Metadata about a feature collection."; }
    [[nodiscard]] std::string linkTitle() const override { return "Feature collection"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Capabilities" ) }; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;
};

/**
 * The CollectionsItemsHandler list all items in the collection
 * Path: /collections/{collectionId}
 */
class QgsWfs3CollectionsItemsHandler : public QgsWfs3AbstractItemsHandler
{
  public:
    QgsWfs3CollectionsItemsHandler();
    void handleRequest( const QgsServerApiContext &context ) const override;
    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re(/collections/(?<collectionId>[^/]+)/items(\.geojson|\.json|\.html|/)?$)re" ); }
    [[nodiscard]] std::string operationId() const override { return "getFeatures"; }
    [[nodiscard]] std::string summary() const override { return "Retrieve features of feature collection {collectionId}."; }
    [[nodiscard]] std::string description() const override
    {
      return "Every feature in a dataset belongs to a collection. A dataset may "
             "consist of multiple feature collections. A feature collection is often a "
             "collection of features of a similar type, based on a common schema. "
             "Use content negotiation or specify a file extension to request HTML (.html) "
             "or GeoJSON (.json).";
    }
    [[nodiscard]] std::string linkTitle() const override { return "Retrieve the features of the collection"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Features" ) }; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    [[nodiscard]] QList<QgsServerQueryStringParameter> parameters( const QgsServerApiContext &context ) const override;
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;

  private:
    // Retrieve the fields filter parameters
    const QList<QgsServerQueryStringParameter> fieldParameters( const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const;
};


class QgsWfs3CollectionsFeatureHandler : public QgsWfs3AbstractItemsHandler
{
  public:
    QgsWfs3CollectionsFeatureHandler();
    void handleRequest( const QgsServerApiContext &context ) const override;
    [[nodiscard]] QRegularExpression path() const override { return QRegularExpression( R"re(/collections/(?<collectionId>[^/]+)/items/(?<featureId>[^/]+?)(\.json|\.geojson|\.html|/)?$)re" ); }
    [[nodiscard]] std::string operationId() const override { return "getFeature"; }
    [[nodiscard]] std::string description() const override { return "Retrieve a feature with ID {featureId} from the collection with ID {collectionId}; use content negotiation or specify a file extension to request HTML (.html or GeoJSON (.json)."; }
    [[nodiscard]] std::string summary() const override { return "Retrieve a single feature with ID {featureId} from the collection with ID {collectionId}."; }
    [[nodiscard]] std::string linkTitle() const override { return "Retrieve a feature"; }
    [[nodiscard]] QStringList tags() const override { return { QStringLiteral( "Features" ) }; }
    [[nodiscard]] QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    [[nodiscard]] json schema( const QgsServerApiContext &context ) const override;
};


#endif // QGS_WFS3_HANDLERS_H
