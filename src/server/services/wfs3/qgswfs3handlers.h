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

#include "qgsserverogcapihandler.h"


/**
 * The APIHandler class Wfs3handles the API definition
 */
class Wfs3APIHandler: public QgsServerOgcApiHandler
{
  public:

    Wfs3APIHandler( );

    // QgsServerOgcApiHandler interface
    void handleRequest( const QgsServerApiContext &context ) const override;
    QRegularExpression path() const override { return QRegularExpression( R"re(/api)re" ); }
    std::string operationId() const override { return "api"; }
    std::string summary() const override { return "The API definition"; }
    std::string description() const override { return "API definition"; }
    std::string linkTitle() const override { return "API definition"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::OPENAPI3, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::service_desc; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::OPENAPI3; }

};


/**
 * The StaticHandler class Wfs3serves static files from the static path (resources/server/api/wfs3/static)
 * \see staticPath()
 */
class Wfs3StaticHandler: public QgsServerOgcApiHandler
{
  public:

    Wfs3StaticHandler( );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return QRegularExpression( R"re(/static/(?<staticFilePath>.*)$)re" ); }
    std::string operationId() const override { return "static"; }
    std::string summary() const override { return "Serves static files"; }
    std::string description() const override { return "Serves static files"; }
    std::string linkTitle() const override { return "Serves static files"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::HTML; }

};


class Wfs3LandingPageHandler: public QgsServerOgcApiHandler
{
  public:

    Wfs3LandingPageHandler( );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return QRegularExpression( R"re($)re" ); }
    std::string operationId() const override { return "getLandingPage"; }
    std::string summary() const override
    {
      return "The landing page provides links to the API definition, the Conformance "
             "statements and the metadata about the feature data in this dataset.";
    }
    std::string description() const override { return "Landing page of this API"; }
    std::string linkTitle() const override { return "Landing page"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::self; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::JSON; }
};


class Wfs3ConformanceHandler: public QgsServerOgcApiHandler
{
  public:

    Wfs3ConformanceHandler( );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return QRegularExpression( R"re(/conformance)re" ); }
    std::string operationId() const override { return "getRequirementClasses"; }
    std::string summary() const override { return "Information about standards that this API conforms to"; }
    std::string description() const override
    {
      return "List all requirements classes specified in a standard (e.g., WFS 3.0 "
             "Part 1: Core) that the server conforms to";
    }
    std::string linkTitle() const override { return "WFS 3.0 conformance classes"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::conformance; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::JSON; }
};


/**
 * The CollectionsHandler lists all available collections for the current project
 * Path: /collections
 */
class Wfs3CollectionsHandler: public QgsServerOgcApiHandler
{
  public:

    Wfs3CollectionsHandler( );

    void handleRequest( const QgsServerApiContext &context ) const override;

    // QgsServerOgcApiHandler interface
    QRegularExpression path() const override { return QRegularExpression( R"re(/collections(\.json|\.html)?$)re" ); }
    std::string operationId() const override { return "describeCollections"; }
    std::string summary() const override
    {
      return "describe the feature collections in the dataset"
             "statements and the metadata about the feature data in this dataset.";
    }
    std::string description() const override { return "Metadata about the feature collections shared by this API."; }
    std::string linkTitle() const override { return "Feature collections"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::JSON; }
};

/**
 * The DescribeCollectionHandler describes a single collection
 * Path: /collections/{collectionId}
 */
class Wfs3DescribeCollectionHandler: public QgsServerOgcApiHandler
{
  public:
    Wfs3DescribeCollectionHandler( );
    void handleRequest( const QgsServerApiContext &context ) const override;

    QRegularExpression path() const override { return QRegularExpression( R"re(/collections/(?<collectionId>[^/]+?)(\.json|\.html)?$)re" ); }
    std::string operationId() const override { return "describeCollection"; }
    std::string summary() const override { return "describe the feature collection"; }
    std::string description() const override { return "Metadata about a feature collection."; }
    std::string linkTitle() const override { return "Feature collection"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::JSON, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::JSON; }


};

/**
 * The CollectionsItemsHandler list all items in the collection
 * Path: /collections/{collectionId}
 */
class Wfs3CollectionsItemsHandler: public QgsServerOgcApiHandler
{
  public:
    Wfs3CollectionsItemsHandler( );
    void handleRequest( const QgsServerApiContext &context ) const override;
    QRegularExpression path() const override { return QRegularExpression( R"re(/collections/(?<collectionId>[^/]+)/items(\.geojson|\.json|\.html)?$)re" ); }
    std::string operationId() const override { return "getFeatures"; }
    std::string summary() const override { return "retrieve features of feature collection collectionId"; }
    std::string description() const override
    {
      return "Every feature in a dataset belongs to a collection. A dataset may "
             "consist of multiple feature collections. A feature collection is often a "
             "collection of features of a similar type, based on a common schema. "
             "Use content negotiation or specify a file extension to request HTML (.html) "
             "or GeoJSON (.json).";
    }
    std::string linkTitle() const override { return "Retrieve the features of the collection"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::GEOJSON; }

};


class Wfs3CollectionsFeatureHandler: public QgsServerOgcApiHandler
{
  public:
    Wfs3CollectionsFeatureHandler( );
    void handleRequest( const QgsServerApiContext &context ) const override;
    QRegularExpression path() const override { return QRegularExpression( R"re(/collections/(?<collectionId>[^/]+)/items/(?<featureId>[^/]+?)(\.json|\.geojson|\.html)?$)re" ); }
    std::string operationId() const override { return "getFeature"; }
    std::string summary() const override { return "retrieve a feature; use content negotiation or specify a file extension to request HTML (.html or GeoJSON (.json)"; }
    std::string description() const override { return "retrieve a single feature"; }
    std::string linkTitle() const override { return "Retrieve a feature"; }
    QList<QgsServerOgcApi::ContentType> contentTypes() const override { return { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML }; }
    QgsServerOgcApi::Rel linkType() const override { return QgsServerOgcApi::Rel::data; }
    QgsServerOgcApi::ContentType defaultContentType() const override { return QgsServerOgcApi::ContentType::GEOJSON; }

};


#endif // QGS_WFS3_HANDLERS_H
