/***************************************************************************
                              qgswfs3handlers.cpp
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

#include "qgswfs3handlers.h"
#include "qgsserverogcapi.h"
#include "qgsserverapicontext.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include "qgsserverapiutils.h"
#include "qgsserverfeatureid.h"
#include "qgsfeaturerequest.h"
#include "qgsjsonutils.h"
#include "qgsogrutils.h"
#include "qgsvectorlayer.h"
#include "qgsmessagelog.h"
#include "qgsbufferserverrequest.h"
#include "qgsserverprojectutils.h"
#include "qgsserverinterface.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayerutils.h"
#include "qgslogger.h"

#include <QTextCodec>

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsfilterrestorer.h"
#include "qgsaccesscontrol.h"
#endif


QgsWfs3APIHandler::QgsWfs3APIHandler( const QgsServerOgcApi *api ):
  mApi( api )
{
  setContentTypes( { QgsServerOgcApi::ContentType::OPENAPI3, QgsServerOgcApi::ContentType::HTML } );
}

void QgsWfs3APIHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw  QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project not found, please check your server configuration." ) );
  }

  const QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *context.project() );
  const QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *context.project() );
  const QString projectTitle = QgsServerProjectUtils::owsServiceTitle( *context.project() );
  const QString projectDescription = QgsServerProjectUtils::owsServiceAbstract( *context.project() );

  const QgsProjectMetadata metadata { context.project()->metadata() };
  json data
  {
    { "openapi", "3.0.1" },
    {
      "tags", {{
          { "name", "Capabilities" },
          { "description", "Essential characteristics of this API including information about the data." }
        }, {
          { "name", "Features" },
          { "description", "Access to data (features)." }
        }
      }
    },
    {
      "info", {
        { "title", projectTitle.toStdString() },
        { "description", projectDescription.toStdString() },
        {
          "contact",  {
            { "name", contactPerson.toStdString() },
            { "email", contactMail.toStdString() },
            { "url", "" }   // TODO: contact url
          }
        },
        {
          "license", {
            { "name",  "" }  // TODO: license
          }
        },
        { "version", mApi->version().toStdString() }
      }
    },
    {
      "servers", {{
          { "url", parentLink( context.request()->url(), 1 ).toStdString() }
        }
      }
    }
  };

  // Add links only if not OPENAPI3 to avoid validation errors
  if ( QgsServerOgcApiHandler::contentTypeFromRequest( context.request() ) != QgsServerOgcApi::ContentType::OPENAPI3 )
  {
    data["links"] = links( context );
  }

  // Gather path information from handlers
  json paths = json::array();
  for ( const auto &h : mApi->handlers() )
  {
    // Skip null schema
    const json hSchema = h->schema( context );
    if ( ! hSchema.is_null() )
      paths.merge_patch( hSchema );
  }
  data[ "paths" ] = paths;

  // Schema: load common part from file schema.json
  static json schema;

  QFile f( context.serverInterface()->serverSettings()->apiResourcesDirectory() + "/ogc/schema.json" );
  if ( f.open( QFile::ReadOnly | QFile::Text ) )
  {
    QTextStream in( &f );
    schema = json::parse( in.readAll().toStdString() );
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "Could not find schema.json in %1, please check your server configuration" ).arg( f.fileName() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
    throw QgsServerApiInternalServerError( QStringLiteral( "Could not find schema.json" ) );
  }

  // Fill CRSs
  json crss = json::array();
  for ( const QString &crs : QgsServerApiUtils::publishedCrsList( context.project() ) )
  {
    crss.push_back( crs.toStdString() );
  }
  schema[ "components" ][ "parameters" ][ "bbox-crs" ][ "schema" ][ "enum" ] = crss;
  schema[ "components" ][ "parameters" ][ "crs" ][ "schema" ][ "enum" ] = crss;
  data[ "components" ] = schema["components"];

  // Add schema refs
  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 1 ).toStdString() }} ) ;
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", navigation }} );
}

json QgsWfs3APIHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + QStringLiteral( "/api" ), context.request()->url() ).toStdString() };
  data[ path ] =
  {
    {
      "get", {
        { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        {
          "responses", {
            {
              "200", {
                { "description", description() },
                {
                  "content", {
                    {
                      "application/vnd.oai.openapi+json;version=3.0", {
                        {
                          "schema",  {
                            { "type", "object" }
                          }
                        }
                      }
                    },
                    {
                      "text/html", {
                        {
                          "schema",  {
                            { "type", "string" }
                          }
                        }
                      }
                    }
                  }
                }
              }
            },
            { "default", defaultResponse() }
          }
        }
      }
    }
  };
  return data;
}

void QgsWfs3AbstractItemsHandler::checkLayerIsAccessible( QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  const QVector<QgsVectorLayer *> publishedLayers = QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context );
  if ( ! publishedLayers.contains( mapLayer ) )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }
}

QgsFeatureRequest QgsWfs3AbstractItemsHandler::filteredRequest( const QgsVectorLayer *vLayer, const QgsServerApiContext &context, const QStringList &subsetAttributes ) const
{
  QgsFeatureRequest featureRequest;
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope()
                    << QgsExpressionContextUtils::projectScope( context.project() )
                    << QgsExpressionContextUtils::layerScope( vLayer );

  featureRequest.setExpressionContext( expressionContext );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  // Python plugins can make further modifications to the allowed attributes
  QgsAccessControl *accessControl = context.serverInterface()->accessControls();
  if ( accessControl )
  {
    accessControl->filterFeatures( vLayer, featureRequest );
  }
#endif

  QSet<QString> publishedAttrs;
  const QgsFields constFields { publishedFields( vLayer, context ) };
  for ( const QgsField &f : constFields )
  {
    if ( subsetAttributes.isEmpty() || subsetAttributes.contains( f.name( ) ) )
      publishedAttrs.insert( f.name() );
  }
  featureRequest.setSubsetOfAttributes( publishedAttrs, vLayer->fields() );
  return featureRequest;
}

QgsFields QgsWfs3AbstractItemsHandler::publishedFields( const QgsVectorLayer *vLayer, const QgsServerApiContext &context ) const
{

  QStringList publishedAttributes = QStringList();
  // Removed attributes
  // WFS excluded attributes for this layer
  const QgsFields &fields = vLayer->fields();
  for ( const QgsField &field : fields )
  {
    if ( !field.configurationFlags().testFlag( QgsField::ConfigurationFlag::HideFromWfs ) )
    {
      publishedAttributes.push_back( field.name() );
    }
  }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  // Python plugins can make further modifications to the allowed attributes
  QgsAccessControl *accessControl = context.serverInterface()->accessControls();
  if ( accessControl )
  {
    publishedAttributes = accessControl->layerAttributes( vLayer, publishedAttributes );
  }
#else
  ( void )context;
#endif

  QgsFields publishedFields;
  for ( int i = 0; i < fields.count(); ++i )
  {
    if ( publishedAttributes.contains( fields.at( i ).name() ) )
    {
      publishedFields.append( fields.at( i ) );
    }
  }
  return publishedFields;
}

QgsWfs3LandingPageHandler::QgsWfs3LandingPageHandler()
{
}

void QgsWfs3LandingPageHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data
  {
    { "links", links( context ) }
  };
  // Append links to APIs
  data["links"].push_back(
  {
    { "href", href( context, "/collections" )},
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::data ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", "Feature collections" },
  } );
  data["links"].push_back(
  {
    { "href", href( context, "/conformance" )},
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::conformance ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", "Conformance classes" },
  } );
  data["links"].push_back(
  {
    { "href", href( context, "/api" )},
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::service_desc ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::OPENAPI3 ) },
    { "title", "API description" },
  } );
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", json::array() }} );
}

json QgsWfs3LandingPageHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath(), context.request()->url() ).toStdString() };

  data[ path ] =
  {
    {
      "get", {
        { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        {
          "responses", {
            {
              "200", {
                { "description", description() },
                {
                  "content", {
                    {
                      "application/json", {
                        {
                          "schema", {
                            { "$ref", "#/components/schemas/root" }
                          }
                        }
                      }
                    },
                    {
                      "text/html", {
                        {
                          "schema",  {
                            { "type", "string" }
                          }
                        }
                      }
                    }
                  }
                }
              }
            },
            { "default", defaultResponse() }
          }
        }
      }
    }
  };
  return data;
}


QgsWfs3ConformanceHandler::QgsWfs3ConformanceHandler()
{
}

void QgsWfs3ConformanceHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data
  {
    { "links", links( context ) },
    {
      "conformsTo", {
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/core",
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/oas30",
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/html",
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/geojson"
      }
    }
  };
  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 1 ).toStdString() }} ) ;
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", navigation }} );
}

json QgsWfs3ConformanceHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + QStringLiteral( "/conformance" ), context.request()->url() ).toStdString() };
  data[ path ] =
  {
    {
      "get", {
        { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        {
          "responses", {
            {
              "200", {
                { "description", description() },
                {
                  "content", {
                    {
                      "application/json", {
                        {
                          "schema",  {
                            { "$ref", "#/components/schemas/root" }
                          }
                        }
                      }
                    },
                    {
                      "text/html", {
                        {
                          "schema",  {
                            { "type", "string" }
                          }
                        }
                      }
                    }
                  }
                }
              }
            },
            { "default", defaultResponse() }
          }
        }
      }
    }
  };
  return data;
}

QgsWfs3CollectionsHandler::QgsWfs3CollectionsHandler()
{
}

void QgsWfs3CollectionsHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json crss = json::array();

  for ( const QString &crs : QgsServerApiUtils::publishedCrsList( context.project() ) )
  {
    crss.push_back( crs.toStdString() );
  }

  json data
  {
    {
      "links", links( context )
    },  // TODO: add XSD or other schema?
    { "collections", json::array() },
    {
      "crs", crss
    }
  };

  if ( context.project() )
  {
    const QgsProject *project = context.project();
    const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
    for ( const QString &wfsLayerId : wfsLayerIds )
    {
      QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( project->mapLayer( wfsLayerId ) );
      if ( !layer )
      {
        continue;
      }
      if ( layer->type() != QgsMapLayerType::VectorLayer )
      {
        continue;
      }

      // Check if the layer is published, raise not found if it is not
      checkLayerIsAccessible( layer, context );

      const std::string title { layer->title().isEmpty() ? layer->name().toStdString() : layer->title().toStdString() };
      const QString shortName { layer->shortName().isEmpty() ? layer->name() : layer->shortName() };
      data["collections"].push_back(
      {
        // identifier of the collection used, for example, in URIs
        { "id", shortName.toStdString() },
        // human readable title of the collection
        { "title", title },
        // a description of the features in the collection
        { "description", layer->abstract().toStdString() },
        {
          "crs", crss
        },
        // TODO: "relations" ?
        {
          "extent",  {
            {
              "spatial", {
                { "bbox", QgsServerApiUtils::layerExtent( layer ) },
                { "crs", "http://www.opengis.net/def/crs/OGC/1.3/CRS84" },
              },
            },
            {
              "temporal", {
                { "interval", QgsServerApiUtils::temporalExtent( layer ) },
                { "trs", "http://www.opengis.net/def/uom/ISO-8601/0/Gregorian" },
              }
            }
          }
        },
        {
          "links", {
            {
              { "href", href( context, QStringLiteral( "/%1/items" ).arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::JSON ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::GEOJSON ) },
              { "title", title + " as GeoJSON" }
            },
            {
              { "href", href( context, QStringLiteral( "/%1/items" ).arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::HTML ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::HTML )  },
              { "title", title + " as HTML" }
            }/* TODO: not sure what these "concepts" are about, neither if they are mandatory
            {
              { "href", href( api, context.request(), QStringLiteral( "/%1/concepts" ).arg( shortName ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", "text/html" },
              { "title", "Describe " + title }
            }
            */
          }
        },
      } );
    }
  }

  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 1 ).toStdString() }} ) ;
  write( data, context, {{ "pageTitle", linkTitle() }, { "navigation", navigation }} );
}

json QgsWfs3CollectionsHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + QStringLiteral( "/collections" ), context.request()->url() ).toStdString() };
  data[ path ] =
  {
    {
      "get", {
        { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        {
          "responses", {
            {
              "200", {
                { "description", description() },
                {
                  "content", {
                    {
                      "application/json", {
                        {
                          "schema",  {
                            { "$ref", "#/components/schemas/content" }
                          }
                        }
                      }
                    },
                    {
                      "text/html", {
                        {
                          "schema",  {
                            { "type", "string" }
                          }
                        }
                      }
                    }
                  }
                }
              }
            },
            { "default", defaultResponse() }
          }
        }
      }
    }
  };
  return data;
}

QgsWfs3DescribeCollectionHandler::QgsWfs3DescribeCollectionHandler()
{
}

void QgsWfs3DescribeCollectionHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project is invalid or undefined" ) );
  }
  // Check collectionId
  const QRegularExpressionMatch match { path().match( context.request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }
  const QString collectionId { match.captured( QStringLiteral( "collectionId" ) ) };
  // May throw if not found
  QgsVectorLayer *mapLayer { layerFromCollectionId( context, collectionId ) };
  Q_ASSERT( mapLayer );


  const QgsProject *project = context.project();
  const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
  if ( ! wfsLayerIds.contains( mapLayer->id() ) )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

  const std::string title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };
  const std::string itemsTitle { title + " items" };
  const QString shortName { mapLayer->shortName().isEmpty() ? mapLayer->name() : mapLayer->shortName() };
  json linksList = links( context );
  linksList.push_back(
  {
    { "href", href( context, QStringLiteral( "/items" ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::JSON ) )  },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::GEOJSON ) },
    { "title", itemsTitle + " as " + QgsServerOgcApi::contentTypeToStdString( QgsServerOgcApi::ContentType::GEOJSON ) }
  } );

  linksList.push_back(
  {
    { "href", href( context, QStringLiteral( "/items" ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::HTML ) )  },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::HTML ) },
    { "title",  itemsTitle + " as " + QgsServerOgcApi::contentTypeToStdString( QgsServerOgcApi::ContentType::HTML ) }
  } );

  linksList.push_back(
  {
    {
      "href", parentLink( context.request()->url(), 3 ).toStdString() +
      "?request=DescribeFeatureType&typenames=" +
      QUrlQuery( shortName ).toString( QUrl::EncodeSpaces ).toStdString() +
      "&service=WFS&version=2.0"
    },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::describedBy ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::XML ) },
    { "title", "Schema for " + title }
  } );

  json crss = json::array();
  for ( const auto &crs : QgsServerApiUtils::publishedCrsList( context.project() ) )
  {
    crss.push_back( crs.toStdString() );
  }
  json data
  {
    { "id", mapLayer->name().toStdString() },
    { "title", title },
    // TODO: check if we need to expose other advertised CRS here
    {
      "crs", crss
    },
    // TODO: "relations" ?
    {
      "extent",  {
        {
          "spatial", {
            { "bbox", QgsServerApiUtils::layerExtent( mapLayer ) },
            { "crs", "http://www.opengis.net/def/crs/OGC/1.3/CRS84" },
          }
        },
        {
          "temporal", {
            { "interval", QgsServerApiUtils::temporalExtent( mapLayer ) },
            { "trs", "http://www.opengis.net/def/uom/ISO-8601/0/Gregorian" },
          }
        }
      }
    },
    {
      "links", linksList
    }
  };
  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 2 ).toStdString() }} ) ;
  navigation.push_back( {{ "title",  "Collections" }, { "href", parentLink( url, 1 ).toStdString() }} ) ;
  write( data, context, {{ "pageTitle", title }, { "navigation", navigation }} );
}

json QgsWfs3DescribeCollectionHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  Q_ASSERT( context.project() );

  const QVector<QgsVectorLayer *> layers { QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context ) };
  // Construct the context with collection id
  for ( const auto &mapLayer : layers )
  {
    const QString shortName { mapLayer->shortName().isEmpty() ? mapLayer->name() : mapLayer->shortName() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const std::string title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };
    const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + QStringLiteral( "/collections/%1" ).arg( shortName ), context.request()->url() ).toStdString() };

    data[ path ] =
    {
      {
        "get", {
          { "tags", jsonTags() },
          { "summary", "Describe the '" + title + "' feature collection"},
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() },
          {
            "responses", {
              {
                "200", {
                  { "description", "Metadata about the collection '" + title + "' shared by this API." },
                  {
                    "content", {
                      {
                        "application/json", {
                          {
                            "schema",  {
                              { "$ref", "#/components/schemas/collectionInfo" }
                            }
                          }
                        }
                      },
                      {
                        "text/html", {
                          {
                            "schema",  {
                              { "type", "string" }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              },
              { "default", defaultResponse() }
            }
          }
        }
      }
    };
  } // end for loop
  return data;
}

QgsWfs3CollectionsItemsHandler::QgsWfs3CollectionsItemsHandler()
{
  setContentTypes( { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML } );
}

QList<QgsServerQueryStringParameter> QgsWfs3CollectionsItemsHandler::parameters( const QgsServerApiContext &context ) const
{
  QList<QgsServerQueryStringParameter> params;

  // Limit
  const qlonglong maxLimit { context.serverInterface()->serverSettings()->apiWfs3MaxLimit() };
  QgsServerQueryStringParameter limit { QStringLiteral( "limit" ), false,
                                        QgsServerQueryStringParameter::Type::Integer,
                                        QStringLiteral( "Number of features to retrieve [0-%1]" ).arg( maxLimit ),
                                        10 };
  limit.setCustomValidator( [ = ]( const QgsServerApiContext &, QVariant & value ) -> bool
  {
    return value.toInt() >= 0 && value.toInt() <= maxLimit;   // TODO: make this configurable!
  } );
  params.push_back( limit );


  // Offset
  QgsServerQueryStringParameter offset { QStringLiteral( "offset" ), false,
                                         QgsServerQueryStringParameter::Type::Integer,
                                         QStringLiteral( "Offset for features to retrieve [0-<number of features in the collection>]" ),
                                         0 };

  bool offsetValidatorSet = false;

  // I'm not yet sure if we should get here without a project,
  // but parameters() may be called to document the API - better safe than sorry.
  if ( context.project() )
  {
    // Fields filters
    const QgsVectorLayer *mapLayer { layerFromContext( context ) };
    if ( mapLayer )
    {
      offset.setCustomValidator( [ = ]( const QgsServerApiContext &, QVariant & value ) -> bool
      {
        const qlonglong longVal { value.toLongLong( ) };
        return longVal >= 0 && longVal <= mapLayer->featureCount( );
      } );
      offset.setDescription( QStringLiteral( "Offset for features to retrieve [0-%1]" ).arg( mapLayer->featureCount( ) ) );
      offsetValidatorSet = true;
      const QList<QgsServerQueryStringParameter> constFieldParameters { fieldParameters( mapLayer, context ) };
      for ( const auto &p : constFieldParameters )
      {
        params.push_back( p );
      }

      const QgsFields published { publishedFields( mapLayer, context ) };
      QStringList publishedFieldNames;
      for ( const auto &f : published )
      {
        publishedFieldNames.push_back( f.name() );
      }

      // Properties (CSV list of properties to return)
      QgsServerQueryStringParameter properties { QStringLiteral( "properties" ), false,
          QgsServerQueryStringParameter::Type::List,
          QStringLiteral( "Comma separated list of feature property names to be added to the result. Valid values: %1" )
          .arg( publishedFieldNames.join( QLatin1String( "', '" ) )
                .append( '\'' )
                .prepend( '\'' ) ) };

      auto propertiesValidator = [ = ]( const QgsServerApiContext &, QVariant & value ) -> bool
      {
        const QStringList properties { value.toStringList() };
        for ( const auto &p : properties )
        {
          if ( ! publishedFieldNames.contains( p ) )
          {
            return false;
          }
        }
        return true;
      };

      properties.setCustomValidator( propertiesValidator );
      params.push_back( properties );

    }

    // Check if is there any suitable datetime fields
    if ( ! QgsServerApiUtils::temporalDimensions( mapLayer ).isEmpty() )
    {
      QgsServerQueryStringParameter datetime { QStringLiteral( "datetime" ), false,
          QgsServerQueryStringParameter::Type::String,
          QStringLiteral( "Datetime filter" ),
                                             };
      datetime.setCustomValidator( [ ]( const QgsServerApiContext &, QVariant & value ) -> bool
      {
        const QString stringValue { value.toString() };
        if ( stringValue.contains( '/' ) )
        {
          try
          {
            QgsServerApiUtils::parseTemporalDateInterval( stringValue );
          }
          catch ( QgsServerException & )
          {
            try
            {
              QgsServerApiUtils::parseTemporalDateTimeInterval( stringValue );
            }
            catch ( QgsServerException & )
            {
              return false;
            }
          }
        }
        else
        {
          if ( ! QDate::fromString( stringValue, Qt::DateFormat::ISODate ).isValid( ) &&
               ! QDateTime::fromString( stringValue, Qt::DateFormat::ISODate ).isValid( ) )
          {
            return false;
          }
        }
        return true;
      } );
      params.push_back( datetime );
    }
  }

  if ( ! offsetValidatorSet )
  {
    offset.setCustomValidator( [ ]( const QgsServerApiContext &, QVariant & value ) -> bool
    {
      const qlonglong longVal { value.toLongLong( ) };
      return longVal >= 0 ;
    } );
  }

  params.push_back( offset );

  // BBOX
  const QgsServerQueryStringParameter bbox { QStringLiteral( "bbox" ), false,
      QgsServerQueryStringParameter::Type::String,
      QStringLiteral( "BBOX filter for the features to retrieve" ) };
  params.push_back( bbox );

  auto crsValidator = [ = ]( const QgsServerApiContext &, QVariant & value ) -> bool
  {
    return QgsServerApiUtils::publishedCrsList( context.project() ).contains( value.toString() );
  };

  // BBOX CRS
  QgsServerQueryStringParameter bboxCrs { QStringLiteral( "bbox-crs" ), false,
                                          QgsServerQueryStringParameter::Type::String,
                                          QStringLiteral( "CRS for the BBOX filter" ),
                                          QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" ) };
  bboxCrs.setCustomValidator( crsValidator );
  params.push_back( bboxCrs );

  // CRS
  QgsServerQueryStringParameter crs { QStringLiteral( "crs" ), false,
                                      QgsServerQueryStringParameter::Type::String,
                                      QStringLiteral( "The coordinate reference system of the response geometries." ),
                                      QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" ) };
  crs.setCustomValidator( crsValidator );
  params.push_back( crs );

  // Result type
  const QgsServerQueryStringParameter resultType { QStringLiteral( "resultType" ), false,
      QgsServerQueryStringParameter::Type::String,
      QStringLiteral( "Type of returned result: 'results' (default) or 'hits'" ),
      QStringLiteral( "results" ) };
  params.push_back( resultType );

  // Sortby
  const QgsServerQueryStringParameter sortBy { QStringLiteral( "sortby" ), false,
      QgsServerQueryStringParameter::Type::String,
      QStringLiteral( "Sort results by the specified field" )
                                             };
  params.push_back( sortBy );

  // Sortdesc
  const QgsServerQueryStringParameter sortDesc { QStringLiteral( "sortdesc" ), false,
      QgsServerQueryStringParameter::Type::Boolean,
      QStringLiteral( "Sort results in descending order, field name must be specified with 'sortby' parameter" ),
      false };
  params.push_back( sortDesc );

  return params;
}

json QgsWfs3CollectionsItemsHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  Q_ASSERT( context.project() );

  const QVector<QgsVectorLayer *> layers { QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context ) };
  // Construct the context with collection id
  for ( const auto &mapLayer : layers )
  {
    const QString shortName { mapLayer->shortName().isEmpty() ? mapLayer->name() : mapLayer->shortName() };
    const std::string title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const QString path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + QStringLiteral( "/collections/%1/items" ).arg( shortName ), context.request()->url() ) };

    static const QStringList componentNames
    {
      QStringLiteral( "limit" ),
      QStringLiteral( "offset" ),
      QStringLiteral( "resultType" ),
      QStringLiteral( "bbox" ),
      QStringLiteral( "bbox-crs" ),
      QStringLiteral( "crs" ),
      QStringLiteral( "datetime" ),
      QStringLiteral( "sortby" ),
      QStringLiteral( "sortdesc" ),
    };

    json componentParameters = json::array();
    for ( const QString &name : componentNames )
    {
      componentParameters.push_back( {{ "$ref", "#/components/parameters/" + name.toStdString() }} );
    }

    // Add layer specific filters
    QgsServerApiContext layerContext( context );
    QgsBufferServerRequest layerRequest( path );
    layerContext.setRequest( &layerRequest );
    const QList<QgsServerQueryStringParameter> requestParameters { parameters( layerContext ) };
    for ( const auto &p : requestParameters )
    {
      if ( ! componentNames.contains( p.name() ) )
        componentParameters.push_back( p.data() );
    }

    data[ path.toStdString() ] =
    {
      {
        "get", {
          { "tags", jsonTags() },
          { "summary", "Retrieve features of '" + title + "' feature collection" },
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() },
          { "parameters", componentParameters },
          {
            "responses", {
              {
                "200", {
                  { "description", "Metadata about the collection '" + title + "' shared by this API." },
                  {
                    "content", {
                      {
                        "application/geo+json", {
                          {
                            "schema",  {
                              { "$ref", "#/components/schemas/featureCollectionGeoJSON" }
                            }
                          }
                        }
                      },
                      {
                        "text/html", {
                          {
                            "schema",  {
                              { "type", "string" }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              },
              { "default", defaultResponse() }
            }
          }
        }
      },
      {
        "post", {
          { "summary", "Adds a new feature to the collection {collectionId}" },
          { "tags", { "edit", "insert" } },
          { "description", "Adds a new feature to the collection {collectionId}" },
          { "operationId", operationId() + '_' + layerId.toStdString() + '_' + "POST" },
          {
            "responses", {
              {
                "201", {
                  { "description", "A new feature was successfully added to the collection" }
                },
              },
              {
                "403", {
                  { "description", "Forbidden: the operation requested was not authorized" }
                },
              },
              {
                "500", {
                  { "description", "Posted data could not be parsed correctly or another error occurred" }
                }
              },
              { "default", defaultResponse() }
            }
          }
        }
      }
    };
  } // end for loop
  return data;
}

const QList<QgsServerQueryStringParameter> QgsWfs3CollectionsItemsHandler::fieldParameters( const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  QList<QgsServerQueryStringParameter> params;
  if ( mapLayer )
  {
    const QgsFields constFields { publishedFields( mapLayer, context ) };
    for ( const auto &f : constFields )
    {
      const QString fName { f.alias().isEmpty() ? f.name() : f.alias() };
      QgsServerQueryStringParameter::Type t;
      switch ( f.type() )
      {
        case QVariant::Int:
        case QVariant::LongLong:
          t = QgsServerQueryStringParameter::Type::Integer;
          break;
        case QVariant::Double:
          t = QgsServerQueryStringParameter::Type::Double;
          break;
        // TODO: date & time
        default:
          t = QgsServerQueryStringParameter::Type::String;
          break;
      }
      const QgsServerQueryStringParameter fieldParam { fName, false,
          t, QStringLiteral( "Retrieve features filtered by: %1 (%2)" ).arg( fName )
          .arg( QgsServerQueryStringParameter::typeName( t ) ) };
      params.push_back( fieldParam );
    }
  }
  return params;
}

void QgsWfs3CollectionsItemsHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project is invalid or undefined" ) );
  }
  QgsVectorLayer *mapLayer { layerFromContext( context ) };
  Q_ASSERT( mapLayer );

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

  const std::string title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };

  // Get parameters
  QVariantMap params = values( context );

  switch ( context.request()->method() )
  {
    // //////////////////////////////////////////////////////////////
    // Retrieve features
    case QgsServerRequest::Method::GetMethod:
    {
      // Validate inputs
      bool ok { false };

      // BBOX
      const QString bbox { params[ QStringLiteral( "bbox" )].toString()  };
      const QgsRectangle filterRect { QgsServerApiUtils::parseBbox( bbox ) };
      if ( ! bbox.isEmpty() && filterRect.isNull() )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "bbox is not valid" ) );
      }

      // BBOX CRS
      const QgsCoordinateReferenceSystem bboxCrs { QgsServerApiUtils::parseCrs( params[ QStringLiteral( "bbox-crs" ) ].toString() ) };
      if ( ! bboxCrs.isValid() )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "BBOX CRS is not valid" ) );
      }

      // CRS
      const QgsCoordinateReferenceSystem crs { QgsServerApiUtils::parseCrs( params[ QStringLiteral( "crs" ) ].toString() ) };
      if ( ! crs.isValid() )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "CRS is not valid" ) );
      }

      // resultType
      const QString resultType { params[ QStringLiteral( "resultType" ) ].toString() };
      static const QStringList availableResultTypes { QStringLiteral( "results" ), QStringLiteral( "hits" )};
      if ( ! availableResultTypes.contains( resultType ) )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "resultType is not valid [results, hits]" ) );
      }

      // Attribute filters
      QgsStringMap attrFilters;
      const QgsFields constPublishedFields { publishedFields( mapLayer, context ) };
      for ( const QgsField &f : constPublishedFields )
      {
        const QString fName { f.alias().isEmpty() ? f.name() : f.alias() };
        const QString val = params.value( fName ).toString() ;
        if ( ! val.isEmpty() )
        {
          const QString sanitized { QgsServerApiUtils::sanitizedFieldValue( val ) };
          if ( sanitized.isEmpty() )
          {
            throw QgsServerApiBadRequestException( QStringLiteral( "Invalid filter field value [%1=%2]" ).arg( f.name() ).arg( val ) );
          }
          attrFilters[fName] = sanitized;
        }
      }

      // limit & offset
      // Apparently the standard set limits 0-10000 (and does not implement paging,
      // so we do our own paging with "offset")
      const qlonglong offset { params.value( QStringLiteral( "offset" ) ).toLongLong( &ok ) };

      const qlonglong limit {  params.value( QStringLiteral( "limit" ) ).toLongLong( &ok ) };

      QString filterExpression;
      QStringList expressions;

      //  datetime
      const QString datetime { params.value( QStringLiteral( "datetime" ) ).toString() };
      if ( ! datetime.isEmpty() )
      {
        const QgsExpression timeExpression { QgsServerApiUtils::temporalFilterExpression( mapLayer, datetime ) };
        if ( ! timeExpression.isValid() )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Invalid datetime filter expression: %1 " ).arg( datetime ) );
        }
        else
        {
          expressions.push_back( timeExpression.expression() );
        }
      }

      // Properties (subset attributes)
      const QStringList requestedProperties { params.value( QStringLiteral( "properties" ) ).toStringList( ) };

      // Sorting
      const QString sortBy { params.value( QStringLiteral( "sortby" ) ).toString( ) };
      const bool sortDesc { params.value( QStringLiteral( "sortdesc" ) ).toBool( ) };

      if ( !sortBy.isEmpty() )
      {
        if ( ! constPublishedFields.names().contains( QgsServerApiUtils::sanitizedFieldValue( sortBy ) ) )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Invalid sortBy field '%1'" ).arg( QgsServerApiUtils::sanitizedFieldValue( sortBy ) ) );
        }
      }


      // ////////////////////////////////////////////////////////////////////////////////////////////////////
      // End of input control: inputs are valid, process the request

      QgsFeatureRequest featureRequest = filteredRequest( mapLayer, context, requestedProperties );

      if ( ! sortBy.isEmpty() )
      {
        featureRequest.setOrderBy( { { { sortBy, ! sortDesc } } } );
      }

      if ( ! filterRect.isNull() )
      {
        const QgsCoordinateTransform ct( bboxCrs, crs, context.project()->transformContext() );
        try
        {
          featureRequest.setFilterRect( ct.transform( filterRect ) );
        }
        catch ( QgsCsException & )
        {
          throw QgsServerApiInternalServerError( QStringLiteral( "BBOX CRS could not be transformed to destination CRS" ) );
        }
      }

      if ( ! attrFilters.isEmpty() )
      {

        if ( featureRequest.filterExpression() && ! featureRequest.filterExpression()->expression().isEmpty() )
        {
          expressions.push_back( featureRequest.filterExpression()->expression() );
        }
        for ( auto it = attrFilters.constBegin(); it != attrFilters.constEnd(); it++ )
        {
          // Handle star
          static const QRegularExpression re2( R"raw([^\\]\*)raw" );
          if ( re2.match( it.value() ).hasMatch() )
          {
            QString val { it.value() };
            expressions.push_back( QStringLiteral( "\"%1\" LIKE '%2'" ).arg( it.key() ).arg( val.replace( '%', QLatin1String( "%%" ) ).replace( '*', '%' ) ) );
          }
          else
          {
            expressions.push_back( QStringLiteral( "\"%1\" = '%2'" ).arg( it.key() ).arg( it.value() ) );
          }
        }
      }

      // Join all expression filters
      if ( ! expressions.isEmpty() )
      {
        filterExpression = expressions.join( QLatin1String( " AND " ) );
        featureRequest.setFilterExpression( filterExpression );
        QgsDebugMsgLevel( QStringLiteral( "Filter expression: %1" ).arg( featureRequest.filterExpression()->expression() ), 4 );
      }

      // WFS3 core specs only serves 4326
      featureRequest.setDestinationCrs( crs, context.project()->transformContext() );
      // Add offset to limit because paging is not supported by QgsFeatureRequest
      featureRequest.setLimit( limit + offset );
      QgsJsonExporter exporter { mapLayer };
      exporter.setAttributes( featureRequest.subsetOfAttributes() );
      exporter.setAttributeDisplayName( true );
      exporter.setSourceCrs( mapLayer->crs() );
      exporter.setTransformGeometries( false );
      QgsFeatureList featureList;
      QgsFeatureIterator features { mapLayer->getFeatures( featureRequest ) };
      QgsFeature feat;
      long i { 0 };
      QMap<QgsFeatureId, QString> fidMap;

      while ( features.nextFeature( feat ) )
      {
        // Ignore records before offset
        if ( i >= offset )
        {
          fidMap.insert( feat.id(), QgsServerFeatureId::getServerFid( feat, mapLayer->dataProvider()->pkAttributeIndexes() ) );
          featureList << feat;
        }
        i++;
      }

      // Count features
      long matchedFeaturesCount = 0;
      if ( attrFilters.isEmpty() && filterRect.isNull() )
      {
        matchedFeaturesCount = mapLayer->featureCount();
      }
      else
      {
        if ( filterExpression.isEmpty() )
        {
          featureRequest.setNoAttributes();
        }

        featureRequest.setFlags( QgsFeatureRequest::Flag::NoGeometry );
        featureRequest.setLimit( -1 );
        features = mapLayer->getFeatures( featureRequest );

        while ( features.nextFeature( feat ) )
        {
          matchedFeaturesCount++;
        }
      }

      json data = exporter.exportFeaturesToJsonObject( featureList );

      // Patch feature IDs with server feature IDs
      for ( int i = 0; i < featureList.length(); i++ )
      {
        data[ "features" ][ i ]["id"] = fidMap.value( data[ "features" ][ i ]["id"] ).toStdString();
      }

      // Add some metadata
      data["numberMatched"] = matchedFeaturesCount;
      data["numberReturned"] = featureList.count();
      data["links"] = links( context );

      // Current url
      const QUrl url { context.request()->url() };

      // Url without offset and limit
      QUrl cleanedUrl { url };
      QUrlQuery query( cleanedUrl );
      query.removeQueryItem( QStringLiteral( "limit" ) );
      query.removeQueryItem( QStringLiteral( "offset" ) );
      cleanedUrl.setQuery( query );

      QString cleanedUrlAsString { cleanedUrl.toString() };

      if ( ! cleanedUrl.hasQuery() )
      {
        cleanedUrlAsString += '?';
      }

      // Get the self link
      json selfLink;
      for ( const auto &l : data["links"] )
      {
        if ( l["rel"] == "self" )
        {
          selfLink = l;
          break;
        }
      }

      // Add prev - next links
      if ( offset != 0 )
      {
        json prevLink = selfLink;
        prevLink["href"] = QStringLiteral( "%1&offset=%2&limit=%3" ).arg( cleanedUrlAsString ).arg( std::max<long>( 0, limit - offset ) ).arg( limit ).toStdString();
        prevLink["rel"] = "prev";
        prevLink["name"] = "Previous page";
        data["links"].push_back( prevLink );
      }

      if ( limit + offset < matchedFeaturesCount )
      {
        json nextLink = selfLink;
        nextLink["href"] = QStringLiteral( "%1&offset=%2&limit=%3" ).arg( cleanedUrlAsString ).arg( std::min<long>( matchedFeaturesCount, limit + offset ) ).arg( limit ).toStdString();
        nextLink["rel"] = "next";
        nextLink["name"] = "Next page";
        data["links"].push_back( nextLink );
      }

      json navigation = json::array();
      navigation.push_back( {{ "title",  "Landing page" }, { "href", parentLink( url, 3 ).toStdString() }} ) ;
      navigation.push_back( {{ "title",  "Collections" }, { "href", parentLink( url, 2 ).toStdString() }} ) ;
      navigation.push_back( {{ "title",   title }, { "href", parentLink( url, 1 ).toStdString()  }} ) ;

      const json htmlMetadata
      {
        { "pageTitle", "Features in layer " + title },
        { "layerTitle", title },
        {
          "geojsonUrl", href( context, "/",
                              QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) )
        },
        { "navigation", navigation }
      };

      write( data, context, htmlMetadata );
      break;
    }
    // //////////////////////////////////////////////////////////////
    // Create a new feature
    case QgsServerRequest::Method::PostMethod:
    {
      // First: check permissions
      const QStringList wfstInsertLayerIds = QgsServerProjectUtils::wfstInsertLayerIds( *context.project() );
      if ( ! wfstInsertLayerIds.contains( mapLayer->id() ) || ! mapLayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::AddFeatures ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "Features cannot be added to layer '%1'" ).arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerInsertPermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "No ACL permissions to insert features on layer '%1'" ).arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif
      try
      {
        // Parse
        json postData = json::parse( context.request()->data() );

        // Process data: extract geometry (because we need to process attributes in a much more complex way)
        const QgsFields fields = QgsOgrUtils::stringToFields( context.request()->data(), QTextCodec::codecForName( "UTF-8" ) );
        const QgsFeatureList features = QgsOgrUtils::stringToFeatureList( context.request()->data(), fields, QTextCodec::codecForName( "UTF-8" ) );
        if ( features.isEmpty() )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Posted data does not contain any feature" ) );
        }

        QgsFeature feat = features.first();
        if ( ! feat.isValid() )
        {
          throw QgsServerApiInternalServerError( QStringLiteral( "Feature is not valid" ) );
        }

        // Transform geometry
        if ( mapLayer->crs() != QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) )
        {
          QgsGeometry geom { feat.geometry() };
          try
          {
            geom.transform( QgsCoordinateTransform( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), mapLayer->crs(), context.project()->transformContext() ) );
          }
          catch ( QgsCsException & )
          {
            throw QgsServerApiInternalServerError( QStringLiteral( "Geometry could not be transformed to destination CRS" ) );
          }
          feat.setGeometry( geom );
        }

        // Process attributes
        try
        {
          const QgsFields authorizedFields { publishedFields( mapLayer, context ) };
          QStringList authorizedFieldNames;
          for ( const auto &f : authorizedFields )
          {
            authorizedFieldNames.push_back( f.name() );
          }
          const QVariantMap properties = QgsJsonUtils::parseJson( postData["properties"].dump( ) ).toMap( );
          const QgsFields fields = mapLayer->fields();
          for ( const auto &field : fields )
          {
            if ( ! properties.value( field.name() ).isNull() )
            {
              if ( ! authorizedFieldNames.contains( field.name() ) )
              {
                throw QgsServerApiBadRequestException( QStringLiteral( "Feature field %1 is not allowed" ).arg( field.name() ) );
              }
              else
              {
                QVariant value = properties.value( field.name() );
                // Convert blobs
                if ( ! properties.value( field.name() ).isNull() && static_cast<QMetaType::Type>( field.type() ) == QMetaType::QByteArray )
                {
                  value = QByteArray::fromBase64( value.toByteArray() );
                }
                feat.setAttribute( field.name(), value );
              }
            }
            else
            {
              feat.setAttribute( field.name(), QVariant() );
            }
          }
        }
        catch ( json::exception & )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Feature properties are not valid" ) );
        }

        // Make sure the first field (id) is null for shapefiles
        if ( mapLayer->providerType() == QLatin1String( "ogr" ) && mapLayer->storageType() == QLatin1String( "ESRI Shapefile" ) )
        {
          feat.setAttribute( 0, QVariant() );
        }
        feat.setId( FID_NULL );

        QgsVectorLayerUtils::matchAttributesToFields( feat, mapLayer->fields( ) );

        QgsFeatureList featuresToAdd( { feat } );
        if ( ! mapLayer->dataProvider()->addFeatures( featuresToAdd ) )
        {
          throw QgsServerApiInternalServerError( QStringLiteral( "Error adding feature to collection" ) );
        }

        feat = featuresToAdd.first();

        // Send response
        context.response()->setStatusCode( 201 );
        context.response()->setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "application/geo+json" ) );

        QString url { context.request()->url().toString( QUrl::EncodeSpaces ) };
        if ( ! url.endsWith( '/' ) )
        {
          url.append( '/' );
        }

        context.response()->setHeader( QStringLiteral( "Location" ), url + QString::number( feat.id() ) );
        context.response()->write( "\"string\"" );

      }
      catch ( json::exception &ex )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "JSON parse error: %1" ).arg( ex.what( ) ) );
      }
      break;
    }
    // Error
    default:
    {
      throw QgsServerApiNotImplementedException( QStringLiteral( "%1 method is not implemented." )
          .arg( QgsServerRequest::methodToString( context.request()->method() ) ) );
    }
  } // end switch
}

QgsWfs3CollectionsFeatureHandler::QgsWfs3CollectionsFeatureHandler()
{
  setContentTypes( { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML } );
}

void QgsWfs3CollectionsFeatureHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( ! context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( QStringLiteral( "Project is invalid or undefined" ) );
  }

  // Check collectionId
  const QRegularExpressionMatch match { path().match( context.request()->url().path( ) ) };
  if ( ! match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Collection was not found" ) );
  }

  const QString collectionId { match.captured( QStringLiteral( "collectionId" ) ) };
  // May throw if not found
  QgsVectorLayer *mapLayer { layerFromCollectionId( context, collectionId ) };
  Q_ASSERT( mapLayer );

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

  const std::string title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };

  // Retrieve feature from storage
  const QString featureId { match.captured( QStringLiteral( "featureId" ) ) };
  QgsFeatureRequest featureRequest = filteredRequest( mapLayer, context );
  const QString fidExpression { QgsServerFeatureId::getExpressionFromServerFid( featureId, mapLayer->dataProvider() ) };
  if ( ! fidExpression.isEmpty() )
  {
    QgsExpression *filterExpression { featureRequest.filterExpression() };
    if ( ! filterExpression )
    {
      featureRequest.setFilterExpression( fidExpression );
    }
    else
    {
      featureRequest.setFilterExpression( QStringLiteral( "(%1) AND (%2)" ).arg( fidExpression, filterExpression->expression() ) );
    }
  }
  else
  {
    bool ok;
    featureRequest.setFilterFid( featureId.toLongLong( &ok ) );
    if ( ! ok )
    {
      throw QgsServerApiInternalServerError( QStringLiteral( "Invalid feature ID [%1]" ).arg( featureId ) );
    }
  }
  QgsFeature feature;
  QgsFeatureIterator it { mapLayer->getFeatures( featureRequest ) };
  if ( ! it.nextFeature( feature ) || ! feature.isValid() )
  {
    throw QgsServerApiInternalServerError( QStringLiteral( "Invalid feature [%1]" ).arg( featureId ) );
  }

  auto doGet = [ & ]( )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = context.serverInterface()->accessControls();
    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );
    if ( accessControl )
    {
      QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
    }
#endif

    QgsJsonExporter exporter { mapLayer };
    exporter.setAttributes( featureRequest.subsetOfAttributes() );
    exporter.setAttributeDisplayName( true );
    json data = exporter.exportFeatureToJsonObject( feature );
    // Patch feature ID
    data["id"] = featureId.toStdString();
    data["links"] = links( context );
    json navigation = json::array();
    const QUrl url { context.request()->url() };
    navigation.push_back( {{ "title", "Landing page" }, { "href", parentLink( url, 4 ).toStdString() }} ) ;
    navigation.push_back( {{ "title", "Collections" }, { "href", parentLink( url, 3 ).toStdString() }} ) ;
    navigation.push_back( {{ "title", title }, { "href", parentLink( url, 2 ).toStdString()  }} ) ;
    navigation.push_back( {{ "title", "Items of " + title }, { "href", parentLink( url ).toStdString() }} ) ;
    const json htmlMetadata
    {
      { "pageTitle", title + " - feature " + featureId.toStdString() },
      {
        "geojsonUrl", href( context, "",
                            QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) )
      },
      { "navigation", navigation }
    };
    write( data, context, htmlMetadata );
  };

  switch ( context.request()->method() )
  {
    // //////////////////////////////////////////////////////////////
    //  Retrieve a single feature
    case QgsServerRequest::Method::GetMethod:
    {
      doGet();
      break;
    }
    // //////////////////////////////////////////////////////////////
    // Replace feature, PATCH should be used for partial updates but we allow partial updates here too
    // because according to the specs PATCH does not allow changes to the geometry.
    // TODO: factor with items handler POST, that uses mostly the same code
    // QUESTION: do we want make things easier for clients and also allow POST here?
    case QgsServerRequest::Method::PostMethod:
    case QgsServerRequest::Method::PutMethod:
    {
      // First: check permissions
      const QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstUpdateLayerIds( *context.project() );
      if ( ! wfstUpdateLayerIds.contains( mapLayer->id() ) ||
           ! mapLayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::ChangeGeometries ) ||
           ! mapLayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::ChangeAttributeValues ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "Features in layer '%1' cannot be changed" ).arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerUpdatePermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "No ACL permissions to change features on layer '%1'" ).arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif
      try
      {
        // Parse
        json postData = json::parse( context.request()->data() );
        // Process data: extract geometry (because we need to process attributes in a much more complex way)
        const QgsFields fields( QgsOgrUtils::stringToFields( context.request()->data(), QTextCodec::codecForName( "UTF-8" ) ) );
        const QgsFeatureList features = QgsOgrUtils::stringToFeatureList( context.request()->data(), fields, QTextCodec::codecForName( "UTF-8" ) );
        if ( features.isEmpty() )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Posted data does not contain any feature" ) );
        }

        const QgsFeature feat = features.first();
        if ( ! feat.isValid() )
        {
          throw QgsServerApiInternalServerError( QStringLiteral( "Feature is not valid" ) );
        }

        QgsChangedAttributesMap changedAttributes;
        QgsAttributeMap changedMap;
        QgsGeometryMap changedGeometries;

        // Transform geometry
        if ( mapLayer->crs() != QgsCoordinateReferenceSystem::fromEpsgId( 4326 ) )
        {
          QgsGeometry geom { feat.geometry() };
          try
          {
            geom.transform( QgsCoordinateTransform( QgsCoordinateReferenceSystem::fromEpsgId( 4326 ), mapLayer->crs(), context.project()->transformContext() ) );
          }
          catch ( QgsCsException & )
          {
            throw QgsServerApiInternalServerError( QStringLiteral( "Geometry could not be transformed to destination CRS" ) );
          }
          changedGeometries.insert( feature.id(), geom );
        }

        // Process attributes
        try
        {
          const QgsFields authorizedFields { publishedFields( mapLayer, context ) };
          QStringList authorizedFieldNames;
          for ( const auto &f : authorizedFields )
          {
            authorizedFieldNames.push_back( f.name() );
          }
          const QVariantMap properties = QgsJsonUtils::parseJson( postData["properties"].dump( ) ).toMap( );
          const QgsFields fields = mapLayer->fields();
          int fieldIndex = 0;
          for ( const auto &field : fields )
          {
            if ( ! properties.value( field.name() ).isNull() )
            {
              if ( ! authorizedFieldNames.contains( field.name() ) )
              {
                throw QgsServerApiPermissionDeniedException( QStringLiteral( "Feature field '%1' change is not allowed" ).arg( field.name() ) );
              }
              else
              {
                QVariant value = properties.value( field.name() );
                // Convert blobs
                if ( ! properties.value( field.name() ).isNull() && static_cast<QMetaType::Type>( field.type() ) == QMetaType::QByteArray )
                {
                  value = QByteArray::fromBase64( value.toByteArray() );
                }
                changedMap.insert( fieldIndex, value );
              }
            }
            else
            {
              // We don't want to set NULL here, in case of partial updates (not sure yet about what the specs will say about this case)
              // changedMap.insert( fieldIndex, QVariant( ) );
            }
            fieldIndex++;
          }
          if ( ! changedMap.isEmpty() )
          {
            changedAttributes.insert( feature.id(), changedMap );
          }
        }
        catch ( json::exception & )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Feature properties are not valid" ) );
        }

        // TODO: raise if nothing to change?

        if ( ! mapLayer->dataProvider()->changeFeatures( changedAttributes, changedGeometries ) )
        {
          throw QgsServerApiInternalServerError( QStringLiteral( "Error changing feature" ) );
        }

        // Now we need to send the updated feature to the client
        feature = mapLayer->getFeature( feature.id() );
        doGet();

      }
      catch ( json::exception &ex )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "JSON parse error: %1" ).arg( ex.what( ) ) );
      }
      break;
    }
    // //////////////////////////////////////////////////////////////
    // Patch feature
    case QgsServerRequest::Method::PatchMethod:
    {
      // First: check permissions
      const QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstUpdateLayerIds( *context.project() );
      if ( ! wfstUpdateLayerIds.contains( mapLayer->id() ) ||
           ! mapLayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::ChangeAttributeValues ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "Feature attributes in layer '%1' cannot be changed" ).arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerUpdatePermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "No ACL permissions to change features on layer '%1'" ).arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif

      QgsChangedAttributesMap changedAttributes;
      QgsAttributeMap changedMap;
      const QgsGeometryMap changedGeometries;  // This will be empty

      try
      {
        // Parse
        json postData = json::parse( context.request()->data() );

        // If the request contains "add" we raise
        if ( postData.contains( "add" ) )
        {
          throw QgsServerApiNotImplementedException( QStringLiteral( "\"add\" instruction in PATCH method is not implemented" ),
              QString::fromStdString( QgsServerOgcApi::mimeType( contentTypeFromRequest( context.request() ) ) ),
              400 );
        }

        // If the request does NOT contain "modify" we raise
        if ( ! postData.contains( "modify" ) )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Missing \"modify\" instruction in PATCH method" ) );
        }

        // Process attributes
        try
        {
          const QgsFields authorizedFields { publishedFields( mapLayer, context ) };
          QStringList authorizedFieldNames;
          for ( const auto &f : authorizedFields )
          {
            authorizedFieldNames.push_back( f.name() );
          }
          const QVariantMap properties = QgsJsonUtils::parseJson( postData["modify"].dump( ) ).toMap( );
          const QgsFields fields = mapLayer->fields();
          int fieldIndex = 0;
          for ( const auto &field : fields )
          {
            if ( ! properties.value( field.name() ).isNull() )
            {
              if ( ! authorizedFieldNames.contains( field.name() ) )
              {
                throw QgsServerApiPermissionDeniedException( QStringLiteral( "Feature field '%1' change is not allowed" ).arg( field.name() ) );
              }
              else
              {
                QVariant value = properties.value( field.name() );
                // Convert blobs
                if ( ! properties.value( field.name() ).isNull() && static_cast<QMetaType::Type>( field.type() ) == QMetaType::QByteArray )
                {
                  value = QByteArray::fromBase64( value.toByteArray() );
                }
                changedMap.insert( fieldIndex, value );
              }
            }
            else
            {
              // Do nothing
            }
            fieldIndex++;
          }
          if ( ! changedMap.isEmpty() )
          {
            changedAttributes.insert( feature.id(), changedMap );
          }
        }
        catch ( json::exception & )
        {
          throw QgsServerApiBadRequestException( QStringLiteral( "Feature properties are not valid" ) );
        }

      }
      catch ( json::exception & )
      {
        throw QgsServerApiBadRequestException( QStringLiteral( "Feature properties are not valid" ) );
      }

      if ( changedAttributes.isEmpty() && changedGeometries.isEmpty() )
      {
        QgsMessageLog::logMessage( QStringLiteral( "Changeset is empty: no features have been modified" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
      }

      if ( ! mapLayer->dataProvider()->changeFeatures( changedAttributes, changedGeometries ) )
      {
        throw QgsServerApiInternalServerError( QStringLiteral( "Error patching feature" ) );
      }

      // Now we need to send the updated feature to the client
      feature = mapLayer->getFeature( feature.id() );
      doGet();

      break;
    }
    // //////////////////////////////////////////////////////////////
    // Delete feature
    case QgsServerRequest::Method::DeleteMethod:
    {
      // First: check permissions
      const QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstDeleteLayerIds( *context.project() );
      if ( ! wfstUpdateLayerIds.contains( mapLayer->id() ) ||
           ! mapLayer->dataProvider()->capabilities().testFlag( QgsVectorDataProvider::Capability::DeleteFeatures ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "Features in layer '%1' cannot be deleted" ).arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerDeletePermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( QStringLiteral( "No ACL permissions to delete features on layer '%1'" ).arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      std::unique_ptr< QgsOWSServerFilterRestorer > filterRestorer( new QgsOWSServerFilterRestorer() );
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif
      if ( ! mapLayer->dataProvider()->deleteFeatures( { feature.id() } ) )
      {
        throw QgsServerApiInternalServerError( QStringLiteral( "Error deleting feature '%1' from layer '%2'" )
                                               .arg( featureId )
                                               .arg( mapLayer->name() ) );
      }

      // All good, empty response
      json data = nullptr;
      write( data, context );

      break;
    }
    default:
    {
      throw QgsServerApiNotImplementedException( QStringLiteral( "%1 method is not implemented." )
          .arg( QgsServerRequest::methodToString( context.request()->method() ) ) );
    }
  } // end switch
}

json QgsWfs3CollectionsFeatureHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  Q_ASSERT( context.project() );

  const QVector<QgsVectorLayer *> layers { QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context ) };
  // Construct the context with collection id
  for ( const auto &mapLayer : layers )
  {
    const QString shortName { mapLayer->shortName().isEmpty() ? mapLayer->name() : mapLayer->shortName() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const std::string title { mapLayer->title().isEmpty() ? mapLayer->name().toStdString() : mapLayer->title().toStdString() };
    const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + QStringLiteral( "/collections/%1/items/{featureId}" ).arg( shortName ), context.request()->url() ).toStdString() };

    data[ path ] =
    {
      {
        "get", {
          { "tags", jsonTags() },
          { "summary", "Retrieve a single feature from the '" + title + "' feature collection"},
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() + '_' + "GET" },
          {
            "parameters", {{ // array of objects
                { "$ref", "#/components/parameters/featureId" }
              }
            }
          },
          {
            "responses", {
              {
                "200", {
                  { "description", "Retrieve a '" + title + "' feature by 'featureId'." },
                  {
                    "content", {
                      {
                        "application/geo+json", {
                          {
                            "schema",  {
                              { "$ref", "#/components/schemas/featureGeoJSON" }
                            }
                          }
                        }
                      },
                      {
                        "text/html", {
                          {
                            "schema",  {
                              { "type", "string" }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              },
              { "default", defaultResponse() }
            }
          }
        }
      },
      {
        "put", {
          { "summary", "Replaces the feature with ID {featureId} in the collection {collectionId}" },
          { "tags", { "edit", "replace" } },
          { "description", "Replaces the feature with ID {featureId} in the collection {collectionId}" },
          { "operationId", operationId() + "PUT" },
          {
            "responses", {
              {
                "200", {
                  { "description", "The feature was successfully updated" }
                },
              },
              {
                "403", {
                  { "description", "Forbidden: the operation requested was not authorized" }
                },
              },
              {
                "500", {
                  { "description", "Posted data could not be parsed correctly or another error occurred" }
                },
              },
              { "default", defaultResponse() }
            }
          }
        }
      },
      {
        "patch", {
          { "summary", "Changes attributes of feature with ID {featureId} in the collection {collectionId}" },
          { "tags", { "edit" } },
          { "description", "Changes attributes of feature with ID {featureId} in the collection {collectionId}" },
          { "operationId", operationId() + "PUT" },
          {
            "responses", {
              {
                "200", {
                  { "description", "The feature was successfully updated" }
                },
              },
              {
                "403", {
                  { "description", "Forbidden: the operation requested was not authorized" }
                },
              },
              {
                "500", {
                  { "description", "Posted data could not be parsed correctly or another error occurred" }
                },
              },
              { "default", defaultResponse() }
            }
          }
        }
      },
      {
        "delete", {
          { "summary", "Deletes the feature with ID {featureId} in the collection {collectionId}" },
          { "tags", { "edit", "delete" } },
          { "description", "Deletes the feature with ID {featureId} in the collection {collectionId}" },
          { "operationId", operationId() + "DELETE" },
          {
            "responses", {
              {
                "201", {
                  { "description", "The feature was successfully deleted from the collection" }
                },
              },
              {
                "403", {
                  { "description", "Forbidden: the operation requested was not authorized" }
                },
              },
              {
                "500", {
                  { "description", "Posted data could not be parsed correctly or another error occurred" }
                }
              },
              { "default", defaultResponse() }
            }
          }
        }
      }
    };
  } // end for loop
  return data;
}
