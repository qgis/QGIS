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

#include <optional>

#include "qgsattributeeditorcontainer.h"
#include "qgsattributeeditorfield.h"
#include "qgsattributeeditorrelation.h"
#include "qgsbufferserverrequest.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressionnodeimpl.h"
#include "qgsfeaturerequest.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogrutils.h"
#include "qgsreferencedgeometry.h"
#include "qgsserverapicontext.h"
#include "qgsserverapiutils.h"
#include "qgsserverfeatureid.h"
#include "qgsserverinterface.h"
#include "qgsserverogcapi.h"
#include "qgsserverprojectutils.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include "qgsvaluemapfieldformatter.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"

#include <QString>
#include <QTextCodec>

using namespace Qt::StringLiterals;

#ifdef HAVE_SERVER_PYTHON_PLUGINS
#include "qgsfilterrestorer.h"
#include "qgsaccesscontrol.h"
#endif


QgsWfs3APIHandler::QgsWfs3APIHandler( const QgsServerOgcApi *api )
  : mApi( api )
{
  setContentTypes( { QgsServerOgcApi::ContentType::OPENAPI3, QgsServerOgcApi::ContentType::HTML } );
}

void QgsWfs3APIHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( !context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( u"Project not found, please check your server configuration."_s );
  }

  const QString contactPerson = QgsServerProjectUtils::owsServiceContactPerson( *context.project() );
  const QString contactMail = QgsServerProjectUtils::owsServiceContactMail( *context.project() );
  const QString projectTitle = QgsServerProjectUtils::owsServiceTitle( *context.project() );
  const QString projectDescription = QgsServerProjectUtils::owsServiceAbstract( *context.project() );

  const QgsProjectMetadata metadata { context.project()->metadata() };
  json data {
    { "openapi", "3.0.1" },
    { "tags",
      { { { "name", "Capabilities" }, { "description", "Essential characteristics of this API including information about the data." } },
        { { "name", "Features" }, { "description", "Access to data (features)." } } } },
    { "info",
      { { "title", projectTitle.toStdString() },
        { "description", projectDescription.toStdString() },
        { "contact",
          {
            { "name", contactPerson.toStdString() }, { "email", contactMail.toStdString() }, { "url", "" } // TODO: contact url
          } },
        { "license",
          {
            { "name", "" } // TODO: license
          } },
        { "version", mApi->version().toStdString() } } },
    { "servers", { { { "url", parentLink( context.request()->url(), 1 ).toStdString() } } } }
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
    if ( !hSchema.is_null() )
      paths.merge_patch( hSchema );
  }
  data["paths"] = paths;

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
    QgsMessageLog::logMessage( u"Could not find schema.json in %1, please check your server configuration"_s.arg( f.fileName() ), u"Server"_s, Qgis::MessageLevel::Critical );
    throw QgsServerApiInternalServerError( u"Could not find schema.json"_s );
  }

  // Fill CRSs
  json crss = json::array();
  for ( const QString &crs : QgsServerApiUtils::publishedCrsList( context.project() ) )
  {
    crss.push_back( crs.toStdString() );
  }
  schema["components"]["parameters"]["bbox-crs"]["schema"]["enum"] = crss;
  schema["components"]["parameters"]["crs"]["schema"]["enum"] = crss;
  data["components"] = schema["components"];

  // Add schema refs
  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 1 ).toStdString() } } );
  write( data, context, { { "pageTitle", linkTitle() }, { "navigation", navigation } } );
}

json QgsWfs3APIHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/api"_s, context.request()->url() ).toStdString() };
  data[path] = {
    { "get",
      { { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        { "responses",
          { { "200",
              { { "description", description() },
                { "content", { { "application/vnd.oai.openapi+json;version=3.0", { { "schema", { { "type", "object" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
            { "default", defaultResponse() } } } } }
  };
  return data;
}

QString QgsWfs3AbstractItemsHandler::referencedLayerIdentifier( const QgsVectorLayer *mapLayer, int fieldIdx, const QgsServerApiContext &context, QString *referencedLayerTitle ) const
{
  const QgsEditorWidgetSetup widgetSetup { mapLayer->editorWidgetSetup( fieldIdx ) };
  const QVariantMap widgetConfig = widgetSetup.config();
  const QgsMapLayer *referencedLayer = nullptr;
  QString referencedLayerId;
  if ( widgetSetup.type() == "ValueRelation"_L1 )
  {
    referencedLayerId = widgetConfig.value( u"Layer"_s ).toString();
    referencedLayer = context.project()->mapLayer( referencedLayerId );
  }
  else if ( widgetSetup.type() == "RelationReference"_L1 )
  {
    const QgsRelation relation { context.project()->relationManager()->relation( widgetConfig.value( u"Relation"_s ).toString() ) };
    // NOTE: WMS might be configured to use layerIDs instead of name (see entry: "WMSUseLayerIDs"), WFS doesn't support this,
    //       so we take the layer name as the collectionId in that case
    referencedLayerId = relation.referencedLayerId();
    // Get the layer from the project, don't check if it's published or not (we don't care at this point)
    referencedLayer = context.project()->mapLayer( referencedLayerId );
  }
  else
  {
    // not a relation
    return QString();
  }

  if ( referencedLayer )
  {
    // NOTE: WMS might be configured to use layerIDs instead of name (see entry: "WMSUseLayerIDs"), WFS doesn't support this,
    //       so we take the layer name as the collectionId in that case
    if ( referencedLayerTitle )
      *referencedLayerTitle = referencedLayer->serverProperties()->wfsTitle().isEmpty() ? referencedLayer->name() : referencedLayer->serverProperties()->wfsTitle();
    return referencedLayer->serverProperties()->shortName().isEmpty() ? referencedLayer->name() : referencedLayer->serverProperties()->shortName();
  }
  else
  {
    throw QgsServerApiImproperlyConfiguredException(
      u"Referenced layer with id '%1' not found for %2 field '%3'"_s.arg( referencedLayerId, widgetSetup.type(), mapLayer->fields().field( fieldIdx ).displayName() )
    );
  }
}

json QgsWfs3AbstractItemsHandler::relatedFeatureReference(
  const QVariant &referencedFeatureValue, const ReferencedLayerInfo &referencedInfo, QgsServerOgcApi::Profile relAs, const QgsServerApiContext &context
) const
{
  switch ( relAs )
  {
    case QgsServerOgcApi::Profile::RelAsUri:
    case QgsServerOgcApi::Profile::RelAsLink:
    {
      // Build the URI calling uri()
      const QMap<QString, QVariant> fieldValueMap { { referencedInfo.referencedFieldOapifIdentifier, referencedFeatureValue } };
      const std::string uriStr = uri( referencedInfo.referencedLayerOapifIdentifier, fieldValueMap, context ).toStdString();

      if ( relAs == QgsServerOgcApi::Profile::RelAsLink )
      {
        return { { "title", "Related feature from layer '" + referencedInfo.referencedLayerOapifIdentifier.toStdString() + "'" }, { "href", uriStr } };
      }
      else
      {
        return uriStr;
      }
    }
    case QgsServerOgcApi::Profile::RelAsKey:
    default:
      return QgsJsonUtils::jsonFromVariant( referencedFeatureValue );
  }
  return json();
}


void QgsWfs3AbstractItemsHandler::checkLayerIsAccessible( QgsVectorLayer *mapLayer, const QgsServerApiContext &context )
{
  const QVector<QgsVectorLayer *> publishedLayers = QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context );
  if ( !publishedLayers.contains( mapLayer ) )
  {
    throw QgsServerApiNotFoundError( u"Collection was not found"_s );
  }
}

QgsFeatureRequest QgsWfs3AbstractItemsHandler::filteredRequest( const QgsVectorLayer *vLayer, const QgsServerApiContext &context, const QStringList &subsetAttributes ) const
{
  QgsFeatureRequest featureRequest;
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope() << QgsExpressionContextUtils::projectScope( context.project() ) << QgsExpressionContextUtils::layerScope( vLayer );

  featureRequest.setExpressionContext( expressionContext );

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  // Python plugins can make further modifications to the allowed attributes
  QgsAccessControl *accessControl = context.serverInterface()->accessControls();
  if ( accessControl )
  {
    Q_NOWARN_DEPRECATED_PUSH
    accessControl->filterFeatures( vLayer, featureRequest );
    Q_NOWARN_DEPRECATED_POP
  }
#endif

  QSet<QString> publishedAttrs;
  const QgsFields constFields { publishedFields( vLayer, context ) };
  for ( const QgsField &f : constFields )
  {
    if ( subsetAttributes.isEmpty() || subsetAttributes.contains( f.name() ) )
      publishedAttrs.insert( f.name() );
  }
  featureRequest.setSubsetOfAttributes( publishedAttrs, vLayer->fields() );
  return featureRequest;
}

QgsFields QgsWfs3AbstractItemsHandler::publishedFields( const QgsVectorLayer *vLayer, const QgsServerApiContext &context )
{
  QStringList publishedAttributes = QStringList();
  // Removed attributes
  // WFS excluded attributes for this layer
  const QgsFields &fields = vLayer->fields();
  for ( const QgsField &field : fields )
  {
    if ( !field.configurationFlags().testFlag( Qgis::FieldConfigurationFlag::HideFromWfs ) )
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
  ( void ) context;
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

void QgsWfs3AbstractItemsHandler::gatherLayerFieldsInfo( json &data, const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  struct CodelistInline
  {
      std::optional<std::string> title;
      std::optional<std::string> description;
      std::optional<QVariantMap> values;
  };

  struct FieldInfo
  {
      FieldInfo( int seq, const std::string &identifier )
        : seq( seq )
        , identifier( identifier )
      {}
      int seq;
      std::string identifier;
      std::optional<std::string> type;
      std::optional<bool> readOnly;
      std::optional<bool> nullable;    // unused for now!
      std::optional<std::string> role; // x-ogc-role
      std::optional<std::string> title;
      std::optional<std::string> format;
      std::optional<std::string> unit;
      std::optional<json> minimum; // range (can be int or double)
      std::optional<json> maximum; // range (can be int or double)
      std::optional<CodelistInline> codelist;
      // min length (unused for now)
      // std::optional<int> minLength;  // string length
      std::optional<int> maxLength;            // string length
      std::optional<json> enumValues;          // enum values
      std::optional<std::string> collectionId; // x-ogc-collectionId
      std::optional<std::string> pattern;      // regex pattern for string values
  };

  auto fieldTypeFormat = []( const QgsField &field, FieldInfo &fInfo ) -> void {
    switch ( field.type() )
    {
      // This is usually a JSON field
      case QMetaType::Type::QVariantMap:
      {
        fInfo.type = "string";
        fInfo.format = "json";
        break;
      }
      case QMetaType::Type::QString:
      {
        if ( field.typeName().toLower() == "json" )
          fInfo.format = "json";
        else if ( field.typeName().toLower() == "xml" )
          fInfo.format = "xml";
        else if ( field.typeName().toLower() == "html" )
          fInfo.format = "html";
        else if ( field.typeName().toLower() == "uuid" )
          fInfo.format = "uuid";
        fInfo.type = "string";
        if ( field.length() > 0 )
        {
          fInfo.maxLength = field.length();
        }
        break;
      }
      case QMetaType::Type::Bool:
      {
        fInfo.type = "boolean";
        break;
      }
      case QMetaType::Type::QUuid:
      {
        fInfo.format = "uuid";
        fInfo.type = "string";
        break;
      }
      case QMetaType::Type::Int:
      {
        fInfo.format = "int32";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::UInt:
      {
        fInfo.format = "uint32";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::Short:
      {
        fInfo.format = "int16";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::UShort:
      {
        fInfo.format = "uint16";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::ULong:
      {
        fInfo.format = "uint64";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::ULongLong:
      {
        fInfo.format = "uint64";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::LongLong:
      {
        fInfo.format = "int64";
        fInfo.type = "integer";
        break;
      }
      case QMetaType::Type::Float:
      {
        fInfo.format = "float";
        fInfo.type = "number";
        break;
      }
      case QMetaType::Type::Double:
      {
        fInfo.format = "double";
        fInfo.type = "number";
        break;
      }
      case QMetaType::Type::QDate:
      {
        fInfo.format = "date";
        fInfo.type = "string";
        break;
      }
      case QMetaType::Type::QDateTime:
      {
        fInfo.format = "date-time";
        fInfo.type = "string";
        // TODO: connect with temporal properties
        // fInfo.role = "primary-instant";
        break;
      }
      case QMetaType::Type::QTime:
      {
        fInfo.format = "time";
        fInfo.type = "string";
        break;
      }
      case QMetaType::Type::QByteArray:
      {
        fInfo.type = "binary";
        break;
      }
      case QMetaType::Type::User:
      {
        if ( field.typeName().toLower() == "geometry" )
        {
          // no format: we do not have the information about the geometry type
          fInfo.format = "geometry-any";
          fInfo.type = "string";
        }
        break;
      }
      default:
      {
        QgsDebugMsgLevel( u"Field '%1' has unsupported  meta type '%2', defaulting to string"_s.arg( field.name() ).arg( field.type() ), 2 );
        break;
      }
    }
  };

  const QgsFields constPublishedFields { publishedFields( mapLayer, context ) };
  json fieldsInfo = json::object();
  std::vector<FieldInfo> availableFieldInformation;
  std::vector<std::string> requiredFields;

  int seq = 0;

  // "id" is always present, read-only and must be the first field
  // NOTE: this appears as a top-level field (and not in the properties) in the collection item
  //       response and is used as the unique identifier for the feature but is returned
  //       as a regular field in the collection schema response (with the other attributes)
  {
    FieldInfo fInfo { seq++, "id" };
    fInfo.type = "integer";
    fInfo.readOnly = true;
    fInfo.role = "id";
    availableFieldInformation.push_back( fInfo );
  }

  QList<QString> publishedFieldNames;
  for ( const QgsField &f : std::as_const( constPublishedFields ) )
  {
    publishedFieldNames.push_back( f.name() );
  }

  // Cannot be const :/
  QgsEditFormConfig config { mapLayer->editFormConfig() };

  std::function< void( QgsAttributeEditorElement * )> traverseTab;
  traverseTab = [&]( QgsAttributeEditorElement *element ) -> void {
    switch ( element->type() )
    {
      case Qgis::AttributeEditorType::Field:
      {
        const QgsAttributeEditorField *fieldElement = static_cast<const QgsAttributeEditorField *>( element );
        // It should never happen that the field element references a non-existing field
        Q_ASSERT( mapLayer->fields().exists( fieldElement->idx() ) );
        const QgsField field = mapLayer->fields().field( fieldElement->idx() );
        const QString fieldIdentifier = field.displayName();

        // Early exit to skip non-published fields and avoid unnecessary processing
        if ( !publishedFieldNames.contains( field.name() ) )
        {
          break;
        }

        // Skip primary key
        if ( fieldIdentifier == "id"_L1 || ( mapLayer->isSpatial() && fieldIdentifier == "geometry" ) )
        {
          break;
        }

        if ( field.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintNotNull ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard )
        {
          requiredFields.push_back( fieldIdentifier.toStdString() );
        }

        FieldInfo fInfo { seq++, fieldIdentifier.toStdString() };

        if ( config.readOnly( fieldElement->idx() ) )
        {
          fInfo.readOnly = true;
        }

        if ( !field.comment().isEmpty() )
        {
          fInfo.title = field.comment().toStdString();
        }

        fieldTypeFormat( field, fInfo );

        // Get the widget configuration
        const QgsEditorWidgetSetup widgetSetup { mapLayer->editorWidgetSetup( fieldElement->idx() ) };
        const QString widgetType { widgetSetup.type() };
        const QVariantMap widgetConfig = widgetSetup.config();

        // NOTE: unused for now!
        if ( widgetConfig.contains( u"AllowNull"_s ) )
        {
          fInfo.nullable = widgetConfig.value( u"AllowNull"_s ).toBool();
        }

        // /////////////////////////////////////////////////////////
        // Process specific widget types for additional information

        // Check for value map
        if ( widgetType == "ValueMap"_L1 )
        {
          const QList<QVariant> valueList = widgetConfig.value( u"map"_s ).toList();
          QVariantMap listValues;
          if ( !valueList.empty() )
          {
            for ( const QVariant &value : std::as_const( valueList ) )
            {
              const QVariantMap valueMap = value.toMap();

              if ( valueMap.constBegin().value() == QgsValueMapFieldFormatter::NULL_VALUE )
              {
                listValues.insert( valueMap.constBegin().key(), QVariant() );
              }
              else
              {
                listValues.insert( valueMap.constBegin().key(), valueMap.constBegin().value() );
              }
            }
          }
          else
          {
            const QVariantMap map = widgetConfig.value( u"map"_s ).toMap();
            for ( auto it = map.constBegin(); it != map.constEnd(); ++it )
            {
              if ( it.value() == QgsValueMapFieldFormatter::NULL_VALUE )
              {
                listValues.insert( it.key(), QVariant() );
              }
              else
              {
                listValues.insert( it.key(), it.value() );
              }
            }
          }

          CodelistInline codelist;
          codelist.values = listValues;

          // TODO: take it from label?
          if ( !field.comment().isEmpty() )
          {
            codelist.description = field.comment().toStdString();
          }
          codelist.title = fieldIdentifier.toStdString();
          fInfo.codelist = codelist;
        }
        else if ( widgetType == "Range"_L1 )
        {
          if ( widgetConfig.value( u"Min"_s ).isValid() )
          {
            fInfo.minimum = QgsJsonUtils::jsonFromVariant( widgetConfig.value( u"Min"_s ) );
          }
          if ( widgetConfig.value( u"Max"_s ).isValid() )
          {
            fInfo.maximum = QgsJsonUtils::jsonFromVariant( widgetConfig.value( u"Max"_s ) );
          }
        }
        else if ( widgetType == "UuidGenerator"_L1 )
        {
          fInfo.format = "uuid";
          fInfo.type = "string";
        }
        else if ( widgetType == "CheckBox"_L1 )
        {
          fInfo.format = "boolean";
        }
        else if ( widgetType == "Enumeration" )
        {
          QStringList enumValues;
          mapLayer->dataProvider()->enumValues( fieldElement->idx(), enumValues );
          fInfo.enumValues = json::array();
          for ( const QString &value : std::as_const( enumValues ) )
          {
            fInfo.enumValues->push_back( value.toStdString() );
          }
        }
        else if ( widgetType == "Geometry"_L1 )
        {
          fInfo.format = "geometry-any";
          fInfo.type = "string";
        }
        else if ( widgetType == "ValueRelation"_L1 )
        {
          QString referencedLayerTitle;
          const QString referencedLayerId = referencedLayerIdentifier( mapLayer, fieldElement->idx(), context, &referencedLayerTitle );

          fInfo.role = "reference";
          fInfo.collectionId = referencedLayerId.toStdString();
          if ( !referencedLayerTitle.isEmpty() )
          {
            fInfo.title = referencedLayerTitle.toStdString();
          }
        }
        else if ( widgetType == "RelationReference"_L1 )
        {
          QString referencedLayerTitle;
          const QString referencedLayerId = referencedLayerIdentifier( mapLayer, fieldElement->idx(), context, &referencedLayerTitle );
          fInfo.role = "reference";
          fInfo.collectionId = referencedLayerId.toStdString();
          if ( !referencedLayerTitle.isEmpty() )
          {
            fInfo.title = referencedLayerTitle.toStdString();
          }
        }
        else if ( widgetType == "DateTime"_L1 )
        {
          fInfo.format = "date-time";
        }
        else if ( widgetType == "Date"_L1 )
        {
          fInfo.format = "date";
        }
        else
        {
          // FIXME: if the widget type is unknown, should we skip the field or include it with best effort?
          //         For instance, if a relation is set, we could still include the reference information even
          //         if the widget type is unknown.
          //         For now, we skip and warn (info level) assuming that if the widget is not set this is intentional.
          QgsMessageLog::logMessage( u"Unhandled widget type '%1' for field '%2'"_s.arg( widgetType, fieldIdentifier ), u"Server"_s, Qgis::MessageLevel::Info );
        }
        // ////////////////////////////////////////////////////
        // Generic code that may apply to several widget types

        // Extract the pattern
        const QgsFieldConstraints constraints = field.constraints();
        if ( field.constraints().constraintStrength( QgsFieldConstraints::Constraint::ConstraintExpression ) == QgsFieldConstraints::ConstraintStrength::ConstraintStrengthHard )
        {
          const QgsExpression expression( constraints.constraintExpression() );
          if ( expression.isValid() && expression.rootNode()->dump().startsWith( "regexp_match("_L1 ) )
          {
            if ( const QgsExpressionNodeFunction *functionNode = dynamic_cast<const QgsExpressionNodeFunction *>( expression.rootNode() ) )
            {
              QgsExpressionNode::NodeList *functionArgs = functionNode->args();
              if ( functionArgs->count() == 2 )
              {
                if ( const QgsExpressionNodeColumnRef *columnNode = dynamic_cast<const QgsExpressionNodeColumnRef *>( functionArgs->at( 0 ) ); columnNode->name() == field.name() )
                {
                  if ( const QgsExpressionNodeLiteral *patternNode = dynamic_cast<const QgsExpressionNodeLiteral *>( functionArgs->at( 1 ) ) )
                  {
                    fInfo.pattern = patternNode->value().toString().toStdString();
                  }
                }
              }
            }
          }
        }

        // Check for std::numeric_limits max/min(lowest for double) and reset if found
        if ( fInfo.maximum.has_value() )
        {
          const json &maxValue = fInfo.maximum.value();
          if ( maxValue.is_number_integer() && ( maxValue.get<int64_t>() == std::numeric_limits<int64_t>::min() || maxValue.get<int64_t>() == std::numeric_limits<int64_t>::max() ) )
          {
            fInfo.maximum.reset();
          }
          else if ( maxValue.is_number_float() && ( maxValue.get<double>() == std::numeric_limits<double>::lowest() || maxValue.get<double>() == std::numeric_limits<double>::max() ) )
          {
            fInfo.maximum.reset();
          }
        }
        if ( fInfo.minimum.has_value() )
        {
          const json &minValue = fInfo.minimum.value();
          if ( minValue.is_number_integer() && ( minValue.get<int64_t>() == std::numeric_limits<int64_t>::min() || minValue.get<int64_t>() == std::numeric_limits<int64_t>::max() ) )
          {
            fInfo.minimum.reset();
          }
          else if ( minValue.is_number_float() && ( minValue.get<double>() == std::numeric_limits<double>::lowest() || minValue.get<double>() == std::numeric_limits<double>::max() ) )
          {
            fInfo.minimum.reset();
          }
        }

        if ( const QString suffix = widgetConfig.value( u"Suffix"_s ).toString(); !suffix.isEmpty() )
        {
          fInfo.unit = suffix.toStdString();
        }

        availableFieldInformation.push_back( fInfo );

        break;
      }
      case Qgis::AttributeEditorType::Relation:
      {
        // This is not handled because the field doesn't belong to the referenced layer
        break;
      }
      case Qgis::AttributeEditorType::Container:
      {
        const QgsAttributeEditorContainer *containerElement = qgis::down_cast<const QgsAttributeEditorContainer *>( element );
        const QList<QgsAttributeEditorElement *> children = containerElement->children();
        for ( const auto &child : std::as_const( children ) )
        {
          traverseTab( child );
        }
        break;
      }
      default:
        break;
    }
  };

  traverseTab( config.invisibleRootContainer() );

  // If the layer is spatial add geometry as last field
  if ( mapLayer->isSpatial() )
  {
    requiredFields.push_back( "geometry" );
    FieldInfo fInfo {
      seq++,
      "geometry",
    };
    fInfo.role = "primary-geometry";
    fInfo.format = "geometry-" + QgsWkbTypes::displayString( QgsWkbTypes::flatType( mapLayer->wkbType() ) ).toLower().toStdString();
    availableFieldInformation.push_back( fInfo );
  }

  // END of field processing

  if ( !requiredFields.empty() )
  {
    data["required"] = requiredFields;
  }


  for ( const auto &fInfo : availableFieldInformation )
  {
    const std::string fieldName = fInfo.identifier;
    fieldsInfo[fieldName] = json::object();
    fieldsInfo[fieldName]["x-ogc-propertySeq"] = fInfo.seq;
    fieldsInfo[fieldName]["type"] = fInfo.type;
    // Optional info
    if ( fInfo.title.has_value() )
      fieldsInfo[fieldName]["title"] = fInfo.title.value();
    if ( fInfo.readOnly.has_value() )
      fieldsInfo[fieldName]["readOnly"] = fInfo.readOnly.value();
    if ( fInfo.role.has_value() )
      fieldsInfo[fieldName]["x-ogc-role"] = fInfo.role.value();
    if ( fInfo.format.has_value() )
      fieldsInfo[fieldName]["format"] = fInfo.format.value();
    if ( fInfo.collectionId.has_value() )
      fieldsInfo[fieldName]["x-ogc-collectionId"] = fInfo.collectionId.value();
    if ( fInfo.unit.has_value() )
      fieldsInfo[fieldName]["x-ogc-unit"] = fInfo.unit.value();
    if ( fInfo.minimum.has_value() )
      fieldsInfo[fieldName]["minimum"] = fInfo.minimum.value();
    if ( fInfo.maximum.has_value() )
      fieldsInfo[fieldName]["maximum"] = fInfo.maximum.value();
    if ( fInfo.maxLength.has_value() )
      fieldsInfo[fieldName]["maxLength"] = fInfo.maxLength.value();
    if ( fInfo.enumValues.has_value() )
      fieldsInfo[fieldName]["enum"] = fInfo.enumValues.value();
    if ( fInfo.pattern.has_value() )
      fieldsInfo[fieldName]["pattern"] = fInfo.pattern.value();
    if ( fInfo.codelist.has_value() )
    {
      json codelistJson = json::object();
      const CodelistInline &codelistInfo = fInfo.codelist.value();
      if ( codelistInfo.title.has_value() )
        codelistJson["title"] = codelistInfo.title.value();
      if ( codelistInfo.description.has_value() )
        codelistJson["description"] = codelistInfo.description.value();
      codelistJson["oneOf"] = json::array();
      const QVariantMap values = codelistInfo.values.value();
      for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
      {
        if ( it.value().isValid() )
        {
          codelistJson["oneOf"].push_back( { { "title", it.key().toStdString() }, { "const", QgsJsonUtils::jsonFromVariant( it.value() ) } } );
        }
        else
        {
          codelistJson["oneOf"].push_back( { { "title", it.key().toStdString() }, { "const", json() } } );
          fieldsInfo[fieldName]["x-ogc-nullValues"] = json::array( { json() } );
        }
      }
      fieldsInfo[fieldName]["x-ogc-codelist"] = codelistJson;
    }
  }
  data["properties"] = fieldsInfo;
}

const QString QgsWfs3AbstractItemsHandler::templatePath( const QgsServerApiContext &context ) const
{
  // resources/server/api + /ogc/templates/wfs3 + operationId + .html
  QString path { context.serverInterface()->serverSettings()->apiResourcesDirectory() };
  path += "/ogc/templates/wfs3/"_L1;
  path += QString::fromStdString( operationId() );
  path += ".html"_L1;
  return path;
}

bool QgsWfs3AbstractItemsHandler::canInsertFeatures( const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  const QStringList wfstInsertLayerIds = QgsServerProjectUtils::wfstInsertLayerIds( *context.project() );
  if ( wfstInsertLayerIds.contains( mapLayer->id() ) && mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::AddFeatures ) )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = context.serverInterface()->accessControls();
    if ( accessControl && !accessControl->layerInsertPermission( mapLayer ) )
    {
      return false;
    }
#endif
    return true;
  }
  return false;
}

bool QgsWfs3AbstractItemsHandler::canDeleteFeatures( const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  const QStringList wfstDeleteLayerIds = QgsServerProjectUtils::wfstDeleteLayerIds( *context.project() );
  if ( wfstDeleteLayerIds.contains( mapLayer->id() ) && mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::DeleteFeatures ) )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = context.serverInterface()->accessControls();
    if ( accessControl && !accessControl->layerDeletePermission( mapLayer ) )
    {
      return false;
    }
#endif
    return true;
  }
  return false;
}

bool QgsWfs3AbstractItemsHandler::canUpdateFeatures( const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  const QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstUpdateLayerIds( *context.project() );
  if ( wfstUpdateLayerIds.contains( mapLayer->id() )
       && ( mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::ChangeAttributeValues ) || mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::ChangeGeometries ) ) )
  {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = context.serverInterface()->accessControls();
    if ( accessControl && !accessControl->layerUpdatePermission( mapLayer ) )
    {
      return false;
    }
#endif
    return true;
  }
  return false;
}

QUrlQuery QgsWfs3AbstractItemsHandler::removeOffsetAndLimit( const QUrlQuery &urlQuery, bool removeProfile )
{
  // Since headers are stored in a case sensitive map,
  // make sure we are clearing the correct case if any
  QUrlQuery query( urlQuery );
  const QList<std::pair<QString, QString>> items = query.queryItems();
  for ( const auto &pair : std::as_const( items ) )
  {
    if ( pair.first.compare( "offset"_L1, Qt::CaseInsensitive ) == 0 )
    {
      query.removeAllQueryItems( pair.first );
    }
    else if ( pair.first.compare( "limit"_L1, Qt::CaseInsensitive ) == 0 )
    {
      query.removeAllQueryItems( pair.first );
    }
    else if ( removeProfile && pair.first.compare( "profile"_L1, Qt::CaseInsensitive ) == 0 )
    {
      query.removeAllQueryItems( pair.first );
    }
  }
  return query;
}

bool QgsWfs3AbstractItemsHandler::crsIsPublished( const QgsCoordinateReferenceSystem &crs, const QgsServerApiContext &context )
{
  const QStringList publishedCrsList = QgsServerApiUtils::publishedCrsList( context.project() );
  const QString crsOgcUri = crs.toOgcUri();
  for ( const QString &pubCrs : std::as_const( publishedCrsList ) )
  {
    // Technically we should test case sensitive but let's be tolerant
    if ( pubCrs.compare( crsOgcUri, Qt::CaseInsensitive ) == 0 )
    {
      return true;
    }
  }
  return false;
}

QMap<int, QgsWfs3AbstractItemsHandler::ReferencedLayerInfo> QgsWfs3AbstractItemsHandler::gatherReferencedLayerInfo( const QgsVectorLayer *mapLayer, const QgsServerApiContext &context ) const
{
  QMap<int, ReferencedLayerInfo> refInfo;

  const QVector<QgsVectorLayer *> publishedLayers = QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context );
  const QgsFields published { publishedFields( mapLayer, context ) };
  for ( const QgsField &referencingField : std::as_const( published ) )
  {
    const int referencingFieldIdx = mapLayer->fields().lookupField( referencingField.name() );
    if ( referencingFieldIdx < 0 )
    {
      throw QgsServerApiImproperlyConfiguredException( u"Field '%1' not found in layer '%2'"_s.arg( referencingField.name() ).arg( mapLayer->name() ) );
    }
    const QgsEditorWidgetSetup editorSetup = mapLayer->editorWidgetSetup( referencingFieldIdx );
    if ( editorSetup.type() == "RelationReference"_L1 )
    {
      const QVariantMap widgetConfig = editorSetup.config();
      const QgsRelation relation { context.project()->relationManager()->relation( widgetConfig.value( u"Relation"_s ).toString() ) };
      if ( !relation.isValid() )
      {
        QgsMessageLog::logMessage( u"RelationReference with relation id '%1' is not valid"_s.arg( relation.id() ), u"Server"_s, Qgis::MessageLevel::Warning );
        continue;
      }
      // NOTE: WMS might be configured to use layerIDs instead of name (see entry: "WMSUseLayerIDs"), WFS doesn't support this,
      //       so we take the layer name as the collectionId in that case
      // Get the layer from the project, don't check if it's published or not (we don't care at this point)
      const QgsVectorLayer *referencedLayer = context.project()->mapLayer<const QgsVectorLayer *>( relation.referencedLayerId() );
      if ( referencedLayer && publishedLayers.contains( referencedLayer ) )
      {
        if ( relation.referencedFields().size() > 1 )
        {
          QgsMessageLog::logMessage( u"RelationReference relations with id '%1' has multiple field pairs, this is not supported"_s.arg( relation.id() ), u"Server"_s, Qgis::MessageLevel::Warning );
          continue;
        }
        if ( referencedLayer->type() != Qgis::LayerType::Vector )
        {
          continue;
        }
        if ( referencedLayer->primaryKeyAttributes().size() > 1 )
        {
          QgsMessageLog::
            logMessage( u"RelationReference relations with id '%1' has a referenced layer with composite primary key, this is not supported"_s.arg( relation.id() ), u"Server"_s, Qgis::MessageLevel::Warning );
          continue;
        }
        const int referencedFieldIdx = relation.referencedFields().first();
        const QgsField referencedField = referencedLayer->fields().field( referencedFieldIdx );
        ReferencedLayerInfo info;
        info.referencingFieldIdx = referencingFieldIdx;
        info.referencedLayer = referencedLayer;
        info.referencedLayerOapifIdentifier = referencedLayer->serverProperties()->shortName().isEmpty() ? referencedLayer->name() : referencedLayer->serverProperties()->shortName();
        info.referencedFieldOapifIdentifier = referencedField.displayName();
        info.referencingFieldOapifIdentifier = referencingField.displayName();
        info.referencingFieldComment = referencedField.comment();
        info.valueIsPk = referencedLayer->primaryKeyAttributes().contains( referencedFieldIdx );
        refInfo.insert( referencingFieldIdx, info );
      }
    }
    else if ( editorSetup.type() == "ValueRelation"_L1 )
    {
      const QVariantMap widgetConfig = editorSetup.config();
      const QgsVectorLayer *referencedLayer = context.project()->mapLayer<const QgsVectorLayer *>( widgetConfig.value( u"Layer"_s ).toString() );
      if ( referencedLayer && publishedLayers.contains( referencedLayer ) )
      {
        if ( referencedLayer->type() != Qgis::LayerType::Vector )
        {
          continue;
        }
        if ( referencedLayer->primaryKeyAttributes().size() > 1 )
        {
          const QgsField field = mapLayer->fields().field( referencingFieldIdx );
          QgsMessageLog::
            logMessage( u"ValueRelation %1' has a referenced layer '%2' with composite primary key, this is not supported"_s.arg( field.name(), referencedLayer->id() ), u"Server"_s, Qgis::MessageLevel::Warning );
          continue;
        }
        const QString referencedFieldName = widgetConfig.value( u"Key"_s ).toString();
        const int fieldIdx = referencedLayer->fields().lookupField( referencedFieldName );
        const QgsField referencedField = referencedLayer->fields().field( fieldIdx );
        ReferencedLayerInfo info;
        info.referencingFieldIdx = referencingFieldIdx;
        info.referencedLayer = referencedLayer;
        info.referencedFieldOapifIdentifier = referencedLayer->fields().at( fieldIdx ).displayName();
        info.referencingFieldOapifIdentifier = referencingField.displayName();
        info.referencedLayerOapifIdentifier = referencedLayer->serverProperties()->shortName().isEmpty() ? referencedLayer->name() : referencedLayer->serverProperties()->shortName();
        info.valueIsPk = referencedLayer->primaryKeyAttributes().contains( referencedLayer->fields().lookupField( referencedFieldName ) );
        info.referencingFieldComment = referencedField.comment();
        refInfo.insert( referencingFieldIdx, info );
      }
    }
  }
  return refInfo;
}

QgsWfs3LandingPageHandler::QgsWfs3LandingPageHandler()
{}

void QgsWfs3LandingPageHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data { { "links", links( context ) } };
  // Append links to APIs
  data["links"].push_back( {
    { "href", href( context, "/collections" ) },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::data ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", "Feature collections" },
  } );
  data["links"].push_back( {
    { "href", href( context, "/conformance" ) },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::conformance ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::JSON ) },
    { "title", "Conformance classes" },
  } );
  data["links"].push_back( {
    { "href", href( context, "/api" ) },
    { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::service_desc ) },
    { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::OPENAPI3 ) },
    { "title", "API description" },
  } );
  write( data, context, { { "pageTitle", linkTitle() }, { "navigation", json::array() } } );
}

json QgsWfs3LandingPageHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath(), context.request()->url() ).toStdString() };

  data[path] = {
    { "get",
      { { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        { "responses",
          { { "200",
              { { "description", description() },
                { "content", { { "application/json", { { "schema", { { "$ref", "#/components/schemas/root" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
            { "default", defaultResponse() } } } } }
  };
  return data;
}

const QString QgsWfs3LandingPageHandler::templatePath( const QgsServerApiContext &context ) const
{
  // resources/server/api + /ogc/templates/wfs3/ + operationId() + .html
  QString path { context.serverInterface()->serverSettings()->apiResourcesDirectory() };
  path += "/ogc/templates/wfs3/"_L1;
  path += QString::fromStdString( operationId() );
  path += ".html"_L1;
  return path;
}


QgsWfs3ConformanceHandler::QgsWfs3ConformanceHandler()
{}

void QgsWfs3ConformanceHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json data {
    { "links", links( context ) },
    { "conformsTo",
      {
        // From https://docs.ogc.org/is/19-072/19-072.html
        "http://www.opengis.net/spec/ogcapi-common-1/1.0/req/landing-page",
        "http://www.opengis.net/spec/ogcapi-common-1/1.0/req/oas30",
        "http://www.opengis.net/spec/ogcapi-common-1/1.0/req/html",
        "http://www.opengis.net/spec/ogcapi-common-1/1.0/req/json",

        // From https://docs.ogc.org/is/23-058r2/23-058r2.html
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/schemas",
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/profile-parameter",
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/profile-references",
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/profile-codelists",
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/advanced-property-roles",
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/references",
        "http://www.opengis.net/spec/ogcapi-common-3/1.0/conf/returnables-and-receivables",

        // From: https://docs.ogc.org/is/18-058r1/18-058r1.html
        "http://www.opengis.net/spec/ogcapi-features-2/1.0/conf/crs",

        // From: https://docs.ogc.org/is/17-069r4/17-069r4.html
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/core",
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/oas30",
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/html",
        "http://www.opengis.net/spec/ogcapi-features-1/1.0/conf/geojson",

        // From: https://docs.ogc.org/is/19-079r2/19-079r2.html
        // TODO: queryables and sortables are not supported yet, but we may want to add them in the future:
        // requirement http://www.opengis.net/spec/ogcapi-features-3/1.0/req/queryables-query-parameters
        // filtering has limited supported but we cannot advertise it yet because of the dependency on
        // queryables-query-parameters:
        // http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/features-filter
        // http://www.opengis.net/spec/ogcapi-features-3/1.0/conf/filter

        // Draft? Now approved as part 3 https://docs.ogc.org/is/23-058r2/23-058r2.html
        "http://www.opengis.net/spec/ogcapi-features-5/1.0/conf/schemas",
        "http://www.opengis.net/spec/ogcapi-features-5/1.0/conf/profile-codelists",
        "http://www.opengis.net/spec/ogcapi-features-5/1.0/conf/profile-parameter",
        "http://www.opengis.net/spec/ogcapi-features-5/1.0/conf/feature-references",
      } }
  };
  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 1 ).toStdString() } } );
  write( data, context, { { "pageTitle", linkTitle() }, { "navigation", navigation } } );
}

json QgsWfs3ConformanceHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/conformance"_s, context.request()->url() ).toStdString() };
  data[path] = {
    { "get",
      { { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        { "responses",
          { { "200",
              { { "description", description() },
                { "content", { { "application/json", { { "schema", { { "$ref", "#/components/schemas/root" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
            { "default", defaultResponse() } } } } }
  };
  return data;
}

QgsWfs3CollectionsHandler::QgsWfs3CollectionsHandler()
{}

void QgsWfs3CollectionsHandler::handleRequest( const QgsServerApiContext &context ) const
{
  json crss = json::array();

  for ( const QString &crs : QgsServerApiUtils::publishedCrsList( context.project() ) )
  {
    crss.push_back( crs.toStdString() );
  }

  json data {
    { "links", links( context ) }, // TODO: add XSD or other schema?
    { "collections", json::array() },
    { "crs", crss }
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
      if ( layer->type() != Qgis::LayerType::Vector )
      {
        continue;
      }

      try
      {
        // Check if the layer is published, raise not found if it is not
        checkLayerIsAccessible( layer, context );

        const std::string title { layer->serverProperties()->wfsTitle().isEmpty() ? layer->name().toStdString() : layer->serverProperties()->wfsTitle().toStdString() };
        const QString shortName { layer->serverProperties()->shortName().isEmpty() ? layer->name() : layer->serverProperties()->shortName() };
        data["collections"].push_back( {
          // identifier of the collection used, for example, in URIs
          { "id", shortName.toStdString() },
          // human readable title of the collection
          { "title", title },
          // a description of the features in the collection
          { "description", layer->serverProperties()->abstract().toStdString() },
          { "crs", crss },
          { "extent",
            { {
                "spatial",
                {
                  { "bbox", QgsServerApiUtils::layerExtent( layer ) },
                  { "crs", "http://www.opengis.net/def/crs/OGC/1.3/CRS84" },
                },
              },
              { "temporal",
                {
                  { "interval", QgsServerApiUtils::temporalExtent( layer ) },
                  { "trs", "http://www.opengis.net/def/uom/ISO-8601/0/Gregorian" },
                } } } },
          { "links",
            {
              { { "href", href( context, u"/%1/items"_s.arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) ) },
                { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
                { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::GEOJSON ) },
                { "title", title + " as GeoJSON" } },
              { { "href", href( context, u"/%1/items"_s.arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::HTML ) ) },
                { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
                { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::HTML ) },
                { "title", title + " as HTML" } },
              { { "href", href( context, u"/%1/items"_s.arg( shortName ), QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::FLATGEOBUF ) ) },
                { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
                { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::FLATGEOBUF ) },
                { "title", title + " as FlatGeobuf" } }
              /* TODO: not sure what these "concepts" are about, neither if they are mandatory
            {
              { "href", href( api, context.request(), u"/%1/concepts"_s.arg( shortName ) )  },
              { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::item ) },
              { "type", "text/html" },
              { "title", "Describe " + title }
            }
            */
            } },
        } );
      }
      catch ( QgsServerApiNotFoundError & )
      {
        // Skip non-published layers
      }
    }
  }

  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 1 ).toStdString() } } );
  write( data, context, { { "pageTitle", linkTitle() }, { "navigation", navigation } } );
}

json QgsWfs3CollectionsHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/collections"_s, context.request()->url() ).toStdString() };
  data[path] = {
    { "get",
      { { "tags", jsonTags() },
        { "summary", summary() },
        { "description", description() },
        { "operationId", operationId() },
        { "responses",
          { { "200",
              { { "description", description() },
                { "content", { { "application/json", { { "schema", { { "$ref", "#/components/schemas/content" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
            { "default", defaultResponse() } } } } }
  };
  return data;
}

QgsWfs3DescribeCollectionHandler::QgsWfs3DescribeCollectionHandler()
{}

void QgsWfs3DescribeCollectionHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( !context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( u"Project is invalid or undefined"_s );
  }
  // Check collectionId
  const QRegularExpressionMatch match { path().match( context.request()->url().path() ) };
  if ( !match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( u"Collection was not found"_s );
  }
  const QString collectionId { match.captured( u"collectionId"_s ) };
  // May throw if not found
  QgsVectorLayer *mapLayer { layerFromCollectionId( context, collectionId ) };
  Q_ASSERT( mapLayer );


  const QgsProject *project = context.project();
  const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
  if ( !wfsLayerIds.contains( mapLayer->id() ) )
  {
    throw QgsServerApiNotFoundError( u"Collection was not found"_s );
  }

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

  const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };
  const std::string itemsTitle { title + " items" };
  const QString shortName { mapLayer->serverProperties()->shortName().isEmpty() ? mapLayer->name() : mapLayer->serverProperties()->shortName() };
  json linksList = links( context );

  for ( const auto ct : { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML, QgsServerOgcApi::ContentType::FLATGEOBUF } )
  {
    linksList.push_back(
      { { "href", href( context, u"/items"_s, QgsServerOgcApi::contentTypeToExtension( ct ) ) },
        { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::items ) },
        { "type", QgsServerOgcApi::mimeType( ct ) },
        { "title", itemsTitle + " as " + QgsServerOgcApi::contentTypeToStdString( ct ) } }
    );
  }

  // Add schemas
  for ( const auto ct : { QgsServerOgcApi::ContentType::SCHEMA_JSON, QgsServerOgcApi::ContentType::HTML } )
  {
    linksList.push_back(
      { { "href", href( context, u"/schema"_s, QgsServerOgcApi::contentTypeToExtension( ct ) ) },
        { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::schema ) },
        { "type", QgsServerOgcApi::mimeType( ct ) },
        { "title", "Schema of features in '" + title + "' as " + QgsServerOgcApi::contentTypeToStdString( ct ) } }
    );
  }


#if 0
  const QString typeName { mapLayer->serverProperties()->wfsTypeName() };
  // Unsure we want to keep this after "schema" has been implemented
  linksList.push_back(
    { { "href",
        parentLink( context.request()->url(), 3 ).toStdString() + "?request=DescribeFeatureType&typename=" + QUrlQuery( typeName ).toString( QUrl::EncodeSpaces ).toStdString() + "&service=WFS&version=2.0" },
      { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::describedBy ) },
      { "type", QgsServerOgcApi::mimeType( QgsServerOgcApi::ContentType::XML ) },
      { "title", "Schema for " + title } }
  );
#endif

  // TODO: add JSONFG and JSONFG-PLUS


  json crss = json::array();
  for ( const auto &crs : QgsServerApiUtils::publishedCrsList( context.project() ) )
  {
    crss.push_back( crs.toStdString() );
  }

  json data {
    { "id", shortName.toStdString() },
    { "title", title },
    // TODO: check if we need to expose other advertised CRS here
    { "crs", crss },
    { "storageCrs", mapLayer->crs().toOgcUri().toStdString() },
    { "extent",
      { { "spatial",
          {
            { "bbox", QgsServerApiUtils::layerExtent( mapLayer ) },
            { "crs", "http://www.opengis.net/def/crs/OGC/1.3/CRS84" },
          } },
        { "temporal",
          {
            { "interval", QgsServerApiUtils::temporalExtent( mapLayer ) },
            { "trs", "http://www.opengis.net/def/uom/ISO-8601/0/Gregorian" },
          } } } },
    { "links", linksList }
  };
  // Add storageCrsCoordinateEpoch if not NaN
  if ( !std::isnan( mapLayer->crs().coordinateEpoch() ) )
  {
    data["storageCrsCoordinateEpoch"] = mapLayer->crs().coordinateEpoch();
  }
  json navigation = json::array();
  const QUrl url { context.request()->url() };
  navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 2 ).toStdString() } } );
  navigation.push_back( { { "title", "Collections" }, { "href", parentLink( url, 1 ).toStdString() } } );
  write( data, context, { { "pageTitle", title }, { "navigation", navigation } } );
}

json QgsWfs3DescribeCollectionHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  Q_ASSERT( context.project() );

  const QVector<QgsVectorLayer *> layers { QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context ) };
  // Construct the context with collection id
  for ( const auto &mapLayer : layers )
  {
    const QString shortName { mapLayer->serverProperties()->shortName().isEmpty() ? mapLayer->name() : mapLayer->serverProperties()->shortName() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };
    const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/collections/%1"_s.arg( shortName ), context.request()->url() ).toStdString() };

    data[path] = {
      { "get",
        { { "tags", jsonTags() },
          { "summary", "Describe the '" + title + "' feature collection" },
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() },
          { "responses",
            { { "200",
                { { "description", "Metadata about the collection '" + title + "' shared by this API." },
                  { "content", { { "application/json", { { "schema", { { "$ref", "#/components/schemas/collectionInfo" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
              { "default", defaultResponse() } } } } }
    };
  } // end for loop
  return data;
}

QgsWfs3CollectionsItemsHandler::QgsWfs3CollectionsItemsHandler()
{
  setContentTypes( { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML, QgsServerOgcApi::ContentType::FLATGEOBUF } );
}

QList<QgsServerQueryStringParameter> QgsWfs3CollectionsItemsHandler::parameters( const QgsServerApiContext &context ) const
{
  QList<QgsServerQueryStringParameter> params;

  // Limit
  const qlonglong maxLimit { context.serverInterface()->serverSettings()->apiWfs3MaxLimit() };
  QgsServerQueryStringParameter limit { u"limit"_s, false, QgsServerQueryStringParameter::Type::Integer, u"Number of features to retrieve [0-%1]"_s.arg( maxLimit ), 10 };
  limit.setCustomValidator( [maxLimit]( const QgsServerApiContext &, QVariant &value ) -> bool {
    bool ok = false;
    const qlonglong longVal { value.toLongLong( &ok ) };
    return ok && longVal >= 0 && longVal <= maxLimit;
  } );
  params.push_back( limit );


  // Offset
  QgsServerQueryStringParameter offset { u"offset"_s, false, QgsServerQueryStringParameter::Type::Integer, u"Offset for features to retrieve [0-<number of features in the collection>]"_s, 0 };

  bool offsetValidatorSet = false;

  // I'm not yet sure if we should get here without a project,
  // but parameters() may be called to document the API - better safe than sorry.
  if ( context.project() )
  {
    // Fields filters
    const QgsVectorLayer *mapLayer { layerFromContext( context ) };
    if ( mapLayer )
    {
      offset.setCustomValidator( [mapLayer]( const QgsServerApiContext &, QVariant &value ) -> bool {
        bool ok = false;
        const qlonglong longVal { value.toLongLong( &ok ) };
        return ok && longVal >= 0 && longVal <= mapLayer->featureCount();
      } );
      offset.setDescription( u"Offset for features to retrieve [0-%1]"_s.arg( mapLayer->featureCount() ) );
      offsetValidatorSet = true;
      const QList<QgsServerQueryStringParameter> constFieldParameters { fieldParameters( mapLayer, context ) };
      for ( const auto &p : constFieldParameters )
      {
        params.push_back( p );
      }

      // We want to accept both displayName and name.
      const QgsFields published { publishedFields( mapLayer, context ) };
      QStringList publishedFieldNames;
      QStringList publishedFieldDisplayNames;
      for ( const auto &f : published )
      {
        publishedFieldDisplayNames.push_back( f.displayName() );
        if ( f.name() != f.displayName() )
        {
          publishedFieldNames.push_back( f.name() );
        }
      }

      // Properties (CSV list of properties to return)
      QgsServerQueryStringParameter properties {
        u"properties"_s,
        false,
        QgsServerQueryStringParameter::Type::List,
        u"Comma separated list of feature property names to be added to the result. Valid values: %1"_s.arg( publishedFieldDisplayNames.join( "', '"_L1 ).append( '\'' ).prepend( '\'' ) )
      };

      auto propertiesValidator = [publishedFieldNames, publishedFieldDisplayNames]( const QgsServerApiContext &, QVariant &value ) -> bool {
        const QStringList properties { value.toStringList() };
        for ( const auto &p : properties )
        {
          if ( !publishedFieldNames.contains( p ) && !publishedFieldDisplayNames.contains( p ) )
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
    if ( !QgsServerApiUtils::temporalDimensions( mapLayer ).isEmpty() )
    {
      QgsServerQueryStringParameter datetime {
        u"datetime"_s,
        false,
        QgsServerQueryStringParameter::Type::String,
        u"Datetime filter"_s,
      };
      datetime.setCustomValidator( []( const QgsServerApiContext &, QVariant &value ) -> bool {
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
          if ( !QDate::fromString( stringValue, Qt::DateFormat::ISODate ).isValid() && !QDateTime::fromString( stringValue, Qt::DateFormat::ISODate ).isValid() )
          {
            return false;
          }
        }
        return true;
      } );
      params.push_back( datetime );
    }
  }

  if ( !offsetValidatorSet )
  {
    offset.setCustomValidator( []( const QgsServerApiContext &, QVariant &value ) -> bool {
      bool ok = false;
      const qlonglong longVal { value.toLongLong( &ok ) };
      return ok && longVal >= 0;
    } );
  }

  params.push_back( offset );

  // BBOX
  const QgsServerQueryStringParameter bbox { u"bbox"_s, false, QgsServerQueryStringParameter::Type::String, u"BBOX filter for the features to retrieve"_s };
  params.push_back( bbox );

  auto crsValidator = [context]( const QgsServerApiContext &, QVariant &value ) -> bool { return QgsServerApiUtils::publishedCrsList( context.project() ).contains( value.toString() ); };

  // BBOX CRS
  QgsServerQueryStringParameter bboxCrs { u"bbox-crs"_s, false, QgsServerQueryStringParameter::Type::String, u"CRS for the BBOX filter"_s, u"http://www.opengis.net/def/crs/OGC/1.3/CRS84"_s };
  bboxCrs.setCustomValidator( crsValidator );
  params.push_back( bboxCrs );

  // CRS
  QgsServerQueryStringParameter
    requestedCrs { u"crs"_s, false, QgsServerQueryStringParameter::Type::String, u"The coordinate reference system of the response geometries."_s, u"http://www.opengis.net/def/crs/OGC/1.3/CRS84"_s };
  requestedCrs.setCustomValidator( crsValidator );
  params.push_back( requestedCrs );

  // Result type
  const QgsServerQueryStringParameter resultType { u"resultType"_s, false, QgsServerQueryStringParameter::Type::String, u"Type of returned result: 'results' (default) or 'hits'"_s, u"results"_s };
  params.push_back( resultType );

  // Sortby
  const QgsServerQueryStringParameter sortBy { u"sortby"_s, false, QgsServerQueryStringParameter::Type::String, u"Sort results by the specified field"_s };
  params.push_back( sortBy );

  // Sortdesc
  const QgsServerQueryStringParameter
    sortDesc { u"sortdesc"_s, false, QgsServerQueryStringParameter::Type::Boolean, u"Sort results in descending order, field name must be specified with 'sortby' parameter"_s, false };
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
    const QString shortName { mapLayer->serverProperties()->shortName().isEmpty() ? mapLayer->name() : mapLayer->serverProperties()->shortName() };
    const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const QString path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/collections/%1/items"_s.arg( shortName ), context.request()->url() ) };

    static const QStringList componentNames {
      u"limit"_s,
      u"offset"_s,
      u"resultType"_s,
      u"bbox"_s,
      u"bbox-crs"_s,
      u"crs"_s,
      u"datetime"_s,
      u"sortby"_s,
      u"sortdesc"_s,
    };

    json componentParameters = json::array();
    for ( const QString &name : componentNames )
    {
      componentParameters.push_back( { { "$ref", "#/components/parameters/" + name.toStdString() } } );
    }

    // Add layer specific filters
    QgsServerApiContext layerContext( context );
    QgsBufferServerRequest layerRequest( path );
    layerContext.setRequest( &layerRequest );
    const QList<QgsServerQueryStringParameter> requestParameters { parameters( layerContext ) };
    for ( const auto &p : requestParameters )
    {
      if ( !p.hidden() && !componentNames.contains( p.name() ) )
        componentParameters.push_back( p.data() );
    }

    data[path.toStdString()] = {
      { "get",
        { { "tags", jsonTags() },
          { "summary", "Retrieve features of '" + title + "' feature collection" },
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() },
          { "parameters", componentParameters },
          { "responses",
            { { "200",
                { { "description", "Metadata about the collection '" + title + "' shared by this API." },
                  { "content",
                    { { "application/geo+json", { { "schema", { { "$ref", "#/components/schemas/featureCollectionGeoJSON" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
              { "default", defaultResponse() } } } } },
      { "post",
        { { "summary", "Adds a new feature to the collection {collectionId}" },
          { "tags", { "edit", "insert" } },
          { "description", "Adds a new feature to the collection {collectionId}" },
          { "operationId", operationId() + '_' + layerId.toStdString() + '_' + "POST" },
          { "responses",
            { {
                "201",
                { { "description", "A new feature was successfully added to the collection" } },
              },
              {
                "403",
                { { "description", "Forbidden: the operation requested was not authorized" } },
              },
              { "500", { { "description", "Posted data could not be parsed correctly or another error occurred" } } },
              { "default", defaultResponse() } } } } }
    };

    // If the layer has no insert capabilities, remove the post operation
    if ( !canInsertFeatures( mapLayer, context ) )
    {
      data[path.toStdString()].erase( "post" );
    }


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
      const QString fName { f.displayName() };
      QgsServerQueryStringParameter::Type t;
      switch ( f.type() )
      {
        case QMetaType::Type::Int:
        case QMetaType::Type::LongLong:
          t = QgsServerQueryStringParameter::Type::Integer;
          break;
        case QMetaType::Type::Double:
          t = QgsServerQueryStringParameter::Type::Double;
          break;
        // TODO: date & time
        default:
          t = QgsServerQueryStringParameter::Type::String;
          break;
      }
      const QgsServerQueryStringParameter fieldParam { fName, false, t, u"Retrieve features filtered by: %1 (%2)"_s.arg( fName, QgsServerQueryStringParameter::typeName( t ) ) };
      params.push_back( fieldParam );

      // Add real field name if alias was used but set it as hidden
      if ( fName != f.name() )
      {
        QgsServerQueryStringParameter
          fieldParam { f.name(), false, t, u"Retrieve features filtered by field: %1 (%2), aliased by %3"_s.arg( f.name(), QgsServerQueryStringParameter::typeName( t ), f.alias() ) };
        fieldParam.setHidden( true );
        params.push_back( fieldParam );
      }
    }
  }
  return params;
}

void QgsWfs3CollectionsItemsHandler::writeJsonOutput( const QgsVectorLayer *mapLayer, QgsFeatureRequest &featureRequest, const QgsServerApiContext &context, const ExportContext &exportContext ) const
{
  const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };
  const QList<QgsServerOgcApi::Profile> requestedProfiles = profilesFromRequest( context.request() );

  // If the layer has any related objects, we need to process the related objects in the output
  // if the client requested the a "rel-as-uri" or "rel-as-link" profile

  const QgsAttributeList requestedAttributes = featureRequest.subsetOfAttributes();
  QMap<int, ReferencedLayerInfo> referencedInfo = gatherReferencedLayerInfo( mapLayer, context );
  // remove if attributes are not requested
  referencedInfo.erase(
    std::remove_if( referencedInfo.begin(), referencedInfo.end(), [&requestedAttributes]( const auto &info ) { return !requestedAttributes.contains( info.referencingFieldIdx ); } ), referencedInfo.end()
  );

  QgsServerOgcApi::Profile relAs = QgsServerOgcApi::Profile::Unset;

  bool hasReferencedObjects = !referencedInfo.isEmpty();

  if ( hasReferencedObjects )
  {
    if ( requestedProfiles.contains( QgsServerOgcApi::Profile::RelAsUri ) )
      relAs = QgsServerOgcApi::Profile::RelAsUri;
    else if ( requestedProfiles.contains( QgsServerOgcApi::Profile::RelAsLink ) )
      relAs = QgsServerOgcApi::Profile::RelAsLink;
    else if ( requestedProfiles.contains( QgsServerOgcApi::Profile::RelAsKey ) )
      relAs = QgsServerOgcApi::Profile::RelAsKey;
  }

  // Exporter for JSON output
  QgsJsonExporter exporter { const_cast<QgsVectorLayer *>( mapLayer ) };
  exporter.setAttributes( featureRequest.subsetOfAttributes() );
  exporter.setAttributeDisplayName( true );
  exporter.setTransformGeometries( false );
  QgsFeatureList featureList;
  QgsFeatureIterator features { mapLayer->getFeatures( featureRequest ) };
  QgsFeature feat;
  long i { 0 };
  QMap<QgsFeatureId, QString> fidMap;

  while ( features.nextFeature( feat ) )
  {
    // Ignore records before offset
    if ( i >= exportContext.offset )
    {
      fidMap.insert( feat.id(), QgsServerFeatureId::getServerFid( feat, mapLayer->dataProvider()->pkAttributeIndexes() ) );
      featureList << feat;
    }
    i++;
  }

  // Count features
  long matchedFeaturesCount = 0;
  if ( exportContext.attrFilters.isEmpty() && exportContext.filterRect.isNull() )
  {
    matchedFeaturesCount = mapLayer->featureCount();
  }
  else
  {
    if ( exportContext.filterExpression.isEmpty() )
    {
      featureRequest.setNoAttributes();
    }

    featureRequest.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    featureRequest.setLimit( -1 );
    features = mapLayer->getFeatures( featureRequest );

    while ( features.nextFeature( feat ) )
    {
      matchedFeaturesCount++;
    }
  }

  json data = exporter.exportFeaturesToJsonObject( featureList );

  QMap<int, QString> extraGeomFields;
  const QList<int> requestedAttrs = featureRequest.subsetOfAttributes();
  for ( const int idx : std::as_const( requestedAttrs ) )
  {
    if ( mapLayer->fields().field( idx ).type() == QMetaType::Type::User && mapLayer->fields().field( idx ).typeName().compare( "geometry"_L1 ) == 0 )
    {
      extraGeomFields.insert( idx, mapLayer->fields().field( idx ).displayName() );
    }
  }

  // Patch feature
  //  - IDs with server feature IDs
  //  - add references
  //  - format extra geometries
  for ( int i = 0; i < featureList.length(); i++ )
  {
    data["features"][i]["id"] = fidMap.value( data["features"][i]["id"] ).toStdString();

    // Add referenced objects
    if ( hasReferencedObjects && relAs != QgsServerOgcApi::Profile::Unset )
    {
      for ( const auto &[fieldIdx, referencedInfo] : referencedInfo.toStdMap() )
      {
        const QgsField field = mapLayer->fields().field( fieldIdx );
        const QString fieldIdentifier = field.alias().isEmpty() ? field.name() : field.alias();
        const QVariant rawValue = featureList[i].attribute( fieldIdx );
        data["features"][i]["properties"][fieldIdentifier.toStdString()] = relatedFeatureReference( rawValue, referencedInfo, relAs, context );
      }
    }

    // Add extra geometries as WKT
    for ( const auto &[fieldIdx, fieldIdentifier] : extraGeomFields.toStdMap() )
    {
      const QgsReferencedGeometry &refGeom = featureList[i].attribute( fieldIdx ).value<QgsReferencedGeometry>();
      if ( refGeom.crs() != featureRequest.destinationCrs() )
      {
        QgsReferencedGeometry transformedRefGeom = refGeom;
        const QgsCoordinateTransform ct( refGeom.crs(), featureRequest.destinationCrs(), featureRequest.coordinateTransform().context() );
        transformedRefGeom.transform( ct );
        data["features"][i]["properties"][fieldIdentifier.toStdString()] = transformedRefGeom.asWkt().toStdString();
      }
      else
      {
        data["features"][i]["properties"][fieldIdentifier.toStdString()] = refGeom.asWkt().toStdString();
      }
    }
  }

  // Add some metadata
  data["numberMatched"] = matchedFeaturesCount;
  data["numberReturned"] = featureList.count();
  data["links"] = links( context );

  // Current url
  const QUrl url { context.request()->url() };

  // Url without offset and limit
  QUrl cleanedUrl { url };
  cleanedUrl.setQuery( removeOffsetAndLimit( QUrlQuery( url.query() ) ) );

  QString cleanedUrlAsString { cleanedUrl.toString() };

  if ( !cleanedUrl.hasQuery() )
  {
    cleanedUrlAsString += '?';
  }
  else
  {
    cleanedUrlAsString += '&';
  }

  // Pagesize metadata
  json pagesize = json::array();
  const qlonglong maxLimit { context.serverInterface()->serverSettings()->apiWfs3MaxLimit() };
  if ( matchedFeaturesCount > 1 && maxLimit > 1 )
  {
    const std::string pageSizeOneLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=1"_s.toStdString() };
    pagesize.push_back( { { "title", "1" }, { "href", pageSizeOneLink } } );
    if ( matchedFeaturesCount > 10 && maxLimit > 10 )
    {
      const std::string pageSizeTenLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=10"_s.toStdString() };
      pagesize.push_back( { { "title", "10" }, { "href", pageSizeTenLink } } );
    }
    if ( matchedFeaturesCount > 20 && maxLimit > 20 )
    {
      const std::string pageSizeTwentyLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=20"_s.toStdString() };
      pagesize.push_back( { { "title", "20" }, { "href", pageSizeTwentyLink } } );
    }
    if ( matchedFeaturesCount > 50 && maxLimit > 50 )
    {
      const std::string pageSizeFiftyLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=50"_s.toStdString() };
      pagesize.push_back( { { "title", "50" }, { "href", pageSizeFiftyLink } } );
    }
    if ( matchedFeaturesCount > 100 && maxLimit > 100 )
    {
      const std::string pageSizeHundredLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=100"_s.toStdString() };
      pagesize.push_back( { { "title", "100" }, { "href", pageSizeHundredLink } } );
    }
    if ( matchedFeaturesCount > 1000 && maxLimit > 1000 )
    {
      const std::string pageSizeThousandLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=1000"_s.toStdString() };
      pagesize.push_back( { { "title", "1000" }, { "href", pageSizeThousandLink } } );
    }
    std::string maxTitle = "All";
    if ( maxLimit < matchedFeaturesCount )
    {
      maxTitle = "Maximum";
    }
    const std::string pageSizeMaxLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=%1"_s.arg( maxLimit ).toStdString() };
    pagesize.push_back( { { "title", maxTitle }, { "href", pageSizeMaxLink } } );
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

  // Pagination metadata
  json pagination = json::array();

  if ( exportContext.limit != 0 )
  {
    // Add prev - next links
    json prevLink;
    if ( exportContext.offset != 0 )
    {
      prevLink = selfLink;
      prevLink["href"] = cleanedUrlAsString.toStdString() + u"offset=%1&limit=%2"_s.arg( std::max<long>( 0, exportContext.offset - exportContext.limit ) ).arg( exportContext.limit ).toStdString();
      prevLink["rel"] = "prev";
      prevLink["title"] = "Previous page";
      data["links"].push_back( prevLink );
    }

    json nextLink;
    if ( exportContext.limit + exportContext.offset < matchedFeaturesCount )
    {
      nextLink = selfLink;
      nextLink["href"] = cleanedUrlAsString.toStdString()
                         + u"offset=%1&limit=%2"_s.arg( std::min<long>( matchedFeaturesCount, exportContext.limit + exportContext.offset ) ).arg( exportContext.limit ).toStdString();
      nextLink["rel"] = "next";
      nextLink["title"] = "Next page";
      data["links"].push_back( nextLink );
    }

    // Pagination
    if ( matchedFeaturesCount - exportContext.limit > 0 )
    {
      const int totalPages { static_cast<int>( std::ceil( static_cast<float>( matchedFeaturesCount ) / static_cast<float>( exportContext.limit ) ) ) };
      const int currentPage { static_cast<int>( exportContext.offset / exportContext.limit + 1 ) };
      const std::string currentPageLink { selfLink["href"] };

      std::string prevPageLink;
      if ( prevLink.contains( std::string { "href" } ) )
      {
        prevPageLink = prevLink["href"];
      }

      std::string nextPageLink;
      if ( nextLink.contains( std::string { "href" } ) )
      {
        nextPageLink = nextLink["href"];
      }

      const std::string firstPageLink { cleanedUrlAsString.toStdString() + u"offset=0&limit=%1"_s.arg( exportContext.limit ).toStdString() };
      const std::string lastPageLink { cleanedUrlAsString.toStdString() + u"offset=%1&limit=%2"_s.arg( totalPages * exportContext.limit - exportContext.limit ).arg( exportContext.limit ).toStdString() };

      if ( currentPage != 1 )
      {
        pagination.push_back( { { "title", "1" }, { "href", firstPageLink }, { "class", "page-item" } } );
      }
      if ( currentPage > 3 )
      {
        pagination.push_back( { { "title", "\u2026" }, { "class", "page-item disabled" } } );
      }
      if ( currentPage > 2 )
      {
        pagination.push_back( { { "title", std::to_string( currentPage - 1 ) }, { "href", prevPageLink }, { "class", "page-item" } } );
      }
      pagination.push_back( { { "title", std::to_string( currentPage ) }, { "href", currentPageLink }, { "class", "page-item active" } } );
      if ( currentPage < totalPages - 1 )
      {
        pagination.push_back( { { "title", std::to_string( currentPage + 1 ) }, { "href", nextPageLink }, { "class", "page-item" } } );
      }
      if ( currentPage < totalPages - 2 )
      {
        pagination.push_back( { { "title", "\u2026" }, { "class", "page-item disabled" } } );
      }
      if ( currentPage != totalPages )
      {
        pagination.push_back( { { "title", std::to_string( totalPages ) }, { "href", lastPageLink }, { "class", "page-item" } } );
      }

      // Add first - last links
      // Since we are having them ready, not mandatory by the spec but allowed
      json firstLink = selfLink;
      firstLink["href"] = firstPageLink;
      firstLink["rel"] = "first";
      firstLink["title"] = "First page";
      data["links"].push_back( firstLink );

      json lastLink = selfLink;
      lastLink["href"] = lastPageLink;
      lastLink["rel"] = "last";
      lastLink["title"] = "Last page";
      data["links"].push_back( lastLink );
    }
  }

  // Add rel-as- links based on requested profile
  // Note: for Unset profile we don't add any rel-as- link
  if ( relAs != QgsServerOgcApi::Profile::Unset )
  {
    const std::string profileStr { QgsServerOgcApi::profileToString( relAs ).toStdString() };
    data["links"].push_back(
      { { "rel", QgsServerOgcApi::relToString( QgsServerOgcApi::Rel::profile ) },
        { "href", "http://www.opengis.net/def/profile/ogc/0/" + profileStr },
        { "title", "Profile '" + profileStr + "' is used in the response" } }
    );
  }

  json navigation = json::array();
  navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 3 ).toStdString() } } );
  navigation.push_back( { { "title", "Collections" }, { "href", parentLink( url, 2 ).toStdString() } } );
  navigation.push_back( { { "title", title }, { "href", parentLink( url, 1 ).toStdString() } } );

  const json htmlMetadata {
    { "pageTitle", "Features in layer " + title },
    { "layerTitle", title },
    { "geojsonUrl", href( context, "/", QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) ) },
    { "pagesize", pagesize },
    { "pagination", pagination },
    { "navigation", navigation }
  };

  write( data, context, htmlMetadata );
}

void QgsWfs3CollectionsItemsHandler::writeFlatGeobufOutput( const QgsVectorLayer *mapLayer, QgsFeatureRequest &featureRequest, const QgsServerApiContext &apiContext, const ExportContext &exportContext ) const
{
  const QString destination = VSIMemGenerateHiddenFilename( "data.fgb" );
  // RIIA deleter for generated file
  QObject obj;
  obj.connect( &obj, &QObject::destroyed, [destination]() { VSIUnlink( destination.toStdString().c_str() ); } );

  const auto attributes { featureRequest.subsetOfAttributes() };
  QgsFields exportedFields;
  // Server FID is required if the layer has a compound primary key and doesn't already have a field named "qgs_fid"
  const bool addQgsFid { mapLayer->dataProvider()->pkAttributeIndexes().count() > 1 && mapLayer->fields().lookupField( u"qgs_fid"_s ) == -1 };
  if ( addQgsFid )
  {
    exportedFields.append( QgsField( u"qgs_fid"_s, QMetaType::Type::QString ) );
  }

  for ( int i = 0; i < mapLayer->fields().count(); i++ )
  {
    if ( !attributes.isEmpty() && !attributes.contains( i ) )
    {
      continue;
    }
    exportedFields.append( mapLayer->fields().field( i ) );
  }

  QgsVectorFileWriter::SaveVectorOptions saveOptions;
  saveOptions.driverName = u"FlatGeobuf"_s;
  if ( !mapLayer->isSpatial() )
  {
    saveOptions.layerOptions.append( u"SPATIAL_INDEX=NO"_s );
  }

  if ( featureRequest.destinationCrs() != mapLayer->crs() )
  {
    saveOptions.ct = QgsCoordinateTransform( mapLayer->crs(), featureRequest.destinationCrs(), featureRequest.transformContext() );
  }

  std::unique_ptr<QgsVectorFileWriter> writer(
    QgsVectorFileWriter::create( destination, exportedFields, mapLayer->wkbType(), featureRequest.destinationCrs(), featureRequest.transformContext(), saveOptions )
  );
  if ( writer->hasError() )
  {
    throw QgsServerApiInternalServerError( u"Could not export layer %1 to FlatGeobuf: %2"_s.arg( mapLayer->name(), writer->errorMessage() ) );
  }

  QgsFeatureList featureList;
  QgsFeatureIterator features { mapLayer->getFeatures( featureRequest ) };
  QgsFeature feat;
  qlonglong i { 0 };

  while ( features.nextFeature( feat ) )
  {
    // Ignore records before offset
    if ( i >= exportContext.offset )
    {
      // Patch feature to add the server feature id
      if ( addQgsFid )
      {
        const QString newFid { QgsServerFeatureId::getServerFid( feat, mapLayer->dataProvider()->pkAttributeIndexes() ) };
        QgsAttributes attributes = feat.attributes();
        attributes.insert( 0, newFid );
        feat.setAttributes( attributes );
        feat.setFields( exportedFields );
      }
      featureList << feat;
    }
    i++;
  }

  // Count features
  long matchedFeaturesCount = 0;
  if ( exportContext.attrFilters.isEmpty() && exportContext.filterRect.isNull() )
  {
    matchedFeaturesCount = mapLayer->featureCount();
  }
  else
  {
    if ( exportContext.filterExpression.isEmpty() )
    {
      featureRequest.setNoAttributes();
    }

    featureRequest.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    featureRequest.setLimit( -1 );
    features = mapLayer->getFeatures( featureRequest );

    while ( features.nextFeature( feat ) )
    {
      matchedFeaturesCount++;
    }
  }

  // cannot be const:
  for ( QgsFeature &f : featureList )
  {
    if ( !writer->addFeature( f ) )
    {
      throw QgsServerApiInternalServerError( u"Error adding feature with id %1 to FlatGeobuf export: %2"_s.arg( f.id() ).arg( writer->errorMessage() ) );
    }
  }

  writer->finalize();

  if ( writer->hasError() )
  {
    throw QgsServerApiInternalServerError( u"Error finalizing FlatGeobuf export: %1"_s.arg( writer->errorMessage() ) );
  }

  writer.reset();

  apiContext.response()->setStatusCode( 200 );


  QDateTime time { QDateTime::currentDateTime() };
  time.setTimeSpec( Qt::TimeSpec::UTC );
  apiContext.response()->setHeader( u"Date"_s, time.toString( Qt::DateFormat::ISODate ) );
  apiContext.response()->setHeader( u"Content-Type"_s, u"application/flatgeobuf"_s );
  apiContext.response()->setHeader( u"Content-Disposition"_s, u"inline; filename=\"%1.fgb\""_s.arg( mapLayer->name() ) );
  apiContext.response()->setHeader( u"Content-Crs"_s, featureRequest.destinationCrs().toOgcUri() );
  apiContext.response()->setHeader( u"OGC-NumberReturned"_s, QString::number( featureList.count() ) );

  // Add self link
  apiContext.response()->addHeader( u"Link"_s, headerLink( apiContext, QgsServerOgcApi::Rel::self, QgsServerOgcApi::ContentType::FLATGEOBUF, QgsServerOgcApi::Profile::Unset, u"This document as FlatGeobuf"_s ) );

  // Add alternate links
  apiContext.response()->addHeader( u"Link"_s, headerLink( apiContext, QgsServerOgcApi::Rel::alternate, QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::Profile::Rfc7946, u"This document as GEOJSON"_s ) );
  apiContext.response()->addHeader( u"Link"_s, headerLink( apiContext, QgsServerOgcApi::Rel::alternate, QgsServerOgcApi::ContentType::HTML, QgsServerOgcApi::Profile::Unset, u"This document as HTML"_s ) );
#if 0
    // This not supported yet but I am leaving it here because
    // I am very optimistic that it will be supported soon!
  context.response()->setHeader( u"link"_s, headerLink( context, QgsServerOgcApi::Rel::alternate, QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::Profile::JSONFG, u"This document as JSONFG"_s ) );
  context.response()
    ->setHeader( u"link"_s, headerLink( context, QgsServerOgcApi::Rel::alternate, QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::Profile::JSONFG_PLUS, u"This document as JSONFG-PLUS"_s ) );
#endif

  // Add next link
  if ( exportContext.limit + exportContext.offset < matchedFeaturesCount )
  {
    // Current url
    const QUrl url { apiContext.request()->url() };

    // Url without offset and limit
    QUrl cleanedUrl { url };
    cleanedUrl.setQuery( removeOffsetAndLimit( QUrlQuery( url.query() ), /* removeProfile */ true ) );
    QString cleanedUrlAsString { cleanedUrl.toString() };

    if ( !cleanedUrl.hasQuery() )
    {
      cleanedUrlAsString += '?';
    }
    else
    {
      cleanedUrlAsString += '&';
    }
    const QString nextHref = cleanedUrlAsString + u"offset=%1&limit=%2"_s.arg( std::min<long>( matchedFeaturesCount, exportContext.limit + exportContext.offset ) ).arg( exportContext.limit );
    apiContext.response()->addHeader( u"Link"_s, u"<%1>; rel=\"next\"; title=\"Next page\"; type=\"application/flatgeobuf\""_s.arg( nextHref ) );
  }

  // Retrieve data from the buffer and send it
  vsi_l_offset nDataLength = 0;
  const char *dataPtr = reinterpret_cast<char *>( VSIGetMemFileBuffer( destination.toStdString().c_str(), &nDataLength, false ) );

  Q_ASSERT( nDataLength <= std::numeric_limits<qsizetype>::max() );

  const QByteArray data { QByteArray::fromRawData( dataPtr, static_cast<qsizetype>( nDataLength ) ) };
  apiContext.response()->write( data );
}

void QgsWfs3CollectionsItemsHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( !context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( u"Project is invalid or undefined"_s );
  }
  QgsVectorLayer *mapLayer { layerFromContext( context ) };
  Q_ASSERT( mapLayer );

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

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
      const QString bbox { params[u"bbox"_s].toString() };
      const QgsRectangle filterRect { QgsServerApiUtils::parseBbox( bbox ) };
      if ( !bbox.isEmpty() && filterRect.isNull() )
      {
        throw QgsServerApiBadRequestException( u"bbox is not valid"_s );
      }

      // BBOX CRS
      const QgsCoordinateReferenceSystem bboxCrs { QgsServerApiUtils::parseCrs( params[u"bbox-crs"_s].toString() ) };
      if ( !bboxCrs.isValid() )
      {
        throw QgsServerApiBadRequestException( u"BBOX CRS is not valid"_s );
      }

      // CRS
      const QgsCoordinateReferenceSystem requestedCrs { QgsServerApiUtils::parseCrs( params[u"crs"_s].toString() ) };
      if ( !requestedCrs.isValid() )
      {
        throw QgsServerApiBadRequestException( u"Requested CRS is not valid"_s );
      }

      // resultType
      const QString resultType { params[u"resultType"_s].toString() };
      static const QStringList availableResultTypes { u"results"_s, u"hits"_s };
      if ( !availableResultTypes.contains( resultType ) )
      {
        throw QgsServerApiBadRequestException( u"resultType is not valid [results, hits]"_s );
      }

      // Attribute filters
      QgsStringMap attrFilters;
      const QgsFields constPublishedFields { publishedFields( mapLayer, context ) };
      for ( const QgsField &f : constPublishedFields )
      {
        QString val = params.value( f.name() ).toString();
        // Try alias
        if ( val.isEmpty() && !f.alias().isEmpty() )
        {
          val = params.value( f.alias() ).toString();
        }
        if ( !val.isEmpty() )
        {
          const QString sanitized { QgsServerApiUtils::sanitizedFieldValue( val ) };
          if ( sanitized.isEmpty() )
          {
            throw QgsServerApiBadRequestException( u"Invalid filter field value [%1=%2]"_s.arg( f.name(), val ) );
          }
          attrFilters[f.name()] = sanitized;
        }
      }

      // limit & offset
      // Apparently the standard set limits 0-10000 (and does not implement paging,
      // so we do our own paging with "offset")
      const qlonglong offset { params.value( u"offset"_s ).toLongLong( &ok ) };

      const qlonglong limit { params.value( u"limit"_s ).toLongLong( &ok ) };

      QString filterExpression;
      QStringList expressions;

      //  datetime
      const QString datetime { params.value( u"datetime"_s ).toString() };
      if ( !datetime.isEmpty() )
      {
        const QgsExpression timeExpression { QgsServerApiUtils::temporalFilterExpression( mapLayer, datetime ) };
        if ( !timeExpression.isValid() )
        {
          throw QgsServerApiBadRequestException( u"Invalid datetime filter expression: %1 "_s.arg( datetime ) );
        }
        else
        {
          expressions.push_back( timeExpression.expression() );
        }
      }

      // Properties (subset attributes)
      const QStringList inputRequestedProperties { params.value( u"properties"_s ).toStringList() };

      // Cleanup (may throw)
      QStringList requestedProperties;
      for ( const QString &property : std::as_const( inputRequestedProperties ) )
      {
        requestedProperties.push_back( QgsServerApiUtils::fieldName( QgsServerApiUtils::sanitizedFieldValue( property ), mapLayer ) );
      }

      // Sorting
      const QString sortBy { params.value( u"sortby"_s ).toString() };
      const bool sortDesc { params.value( u"sortdesc"_s ).toBool() };

      if ( !sortBy.isEmpty() )
      {
        // fieldName may throw a different message ...
        try
        {
          if ( !constPublishedFields.names().contains( QgsServerApiUtils::fieldName( QgsServerApiUtils::sanitizedFieldValue( sortBy ), mapLayer ) ) )
          {
            throw QgsServerApiBadRequestException( QString() );
          }
        }
        catch ( const QgsServerApiBadRequestException & )
        {
          throw QgsServerApiBadRequestException( u"Invalid sortBy field '%1'"_s.arg( QgsServerApiUtils::sanitizedFieldValue( sortBy ) ) );
        }
      }

      // ////////////////////////////////////////////////////////////////////////////////////////////////////
      // End of input control: inputs are valid, process the request

      QgsFeatureRequest featureRequest = filteredRequest( mapLayer, context, requestedProperties );

      if ( !sortBy.isEmpty() )
      {
        featureRequest.setOrderBy( { { { sortBy, !sortDesc } } } );
      }

      if ( !filterRect.isNull() )
      {
        const QgsCoordinateTransform ct( bboxCrs, requestedCrs, context.project()->transformContext() );
        try
        {
          featureRequest.setFilterRect( ct.transform( filterRect ) );
        }
        catch ( QgsCsException & )
        {
          throw QgsServerApiInternalServerError( u"BBOX CRS could not be transformed to destination CRS"_s );
        }
      }

      if ( !attrFilters.isEmpty() )
      {
        if ( featureRequest.filterExpression() && !featureRequest.filterExpression()->expression().isEmpty() )
        {
          expressions.push_back( featureRequest.filterExpression()->expression() );
        }
        for ( auto it = attrFilters.constBegin(); it != attrFilters.constEnd(); it++ )
        {
          // Handle star
          const thread_local QRegularExpression re2( R"raw([^\\]\*)raw" );
          if ( re2.match( it.value() ).hasMatch() )
          {
            QString val { it.value() };
            expressions.push_back( u"\"%1\" LIKE '%2'"_s.arg( it.key() ).arg( val.replace( '%', "%%"_L1 ).replace( '*', '%' ) ) );
          }
          else
          {
            expressions.push_back( u"\"%1\" = '%2'"_s.arg( it.key() ).arg( it.value() ) );
          }
        }
      }

      // Join all expression filters
      if ( !expressions.isEmpty() )
      {
        filterExpression = expressions.join( " AND "_L1 );
        featureRequest.setFilterExpression( filterExpression );
        QgsDebugMsgLevel( u"Filter expression: %1"_s.arg( featureRequest.filterExpression()->expression() ), 4 );
      }

      // WFS3 initial core specs only serves CRS84 but we support transformations with new profiles
      featureRequest.setDestinationCrs( requestedCrs, context.project()->transformContext() );
      // Add offset to limit because paging is not supported by QgsFeatureRequest
      featureRequest.setLimit( limit + offset );

      const ExportContext exportContext { limit, offset, attrFilters, filterExpression, filterRect };

      const QgsServerOgcApi::ContentType contentType { contentTypeFromRequest( context.request() ) };
      switch ( contentType )
      {
        case QgsServerOgcApi::ContentType::JSON:
        case QgsServerOgcApi::ContentType::GEOJSON:
        case QgsServerOgcApi::ContentType::HTML:
          writeJsonOutput( mapLayer, featureRequest, context, exportContext );
          break;
        case QgsServerOgcApi::ContentType::FLATGEOBUF:
          writeFlatGeobufOutput( mapLayer, featureRequest, context, exportContext );
          break;
        default:
          throw QgsServerApiInternalServerError( u"Unsupported content type"_s );
      }

      break;
    }
    // //////////////////////////////////////////////////////////////
    // Create a new feature
    case QgsServerRequest::Method::PostMethod:
    {
      // First: check permissions
      const QStringList wfstInsertLayerIds = QgsServerProjectUtils::wfstInsertLayerIds( *context.project() );
      if ( !wfstInsertLayerIds.contains( mapLayer->id() ) || !mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::AddFeatures ) )
      {
        throw QgsServerApiPermissionDeniedException( u"Features cannot be added to layer '%1'"_s.arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerInsertPermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( u"No ACL permissions to insert features on layer '%1'"_s.arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      auto filterRestorer = std::make_unique<QgsOWSServerFilterRestorer>();
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

      // CRS
      const QgsCoordinateReferenceSystem requestedCrs { QgsServerApiUtils::parseCrs( params[u"crs"_s].toString() ) };
      if ( !requestedCrs.isValid() )
      {
        throw QgsServerApiBadRequestException( u"Requested CRS is not valid"_s );
      }

#endif
      try
      {
        // Parse
        json postData = json::parse( context.request()->data().toStdString() );

        // Process data: extract geometry (because we need to process attributes in a much more complex way)
        const QgsFields fields = QgsOgrUtils::stringToFields( context.request()->data(), QTextCodec::codecForName( "UTF-8" ) );
        const QgsFeatureList features = QgsOgrUtils::stringToFeatureList( context.request()->data(), fields, QTextCodec::codecForName( "UTF-8" ) );
        if ( features.isEmpty() )
        {
          throw QgsServerApiBadRequestException( u"Posted data does not contain any feature"_s );
        }

        QgsFeature feat = features.first();
        if ( !feat.isValid() )
        {
          throw QgsServerApiInternalServerError( u"Feature is not valid"_s );
        }

        // FIXME: in JSON-FG we might have a different CRS than CRS84,
        // we should check for a crs member in the JSON and use it if present

        // Transform geometry
        if ( mapLayer->crs() != requestedCrs )
        {
          QgsGeometry geom { feat.geometry() };
          try
          {
            geom.transform( QgsCoordinateTransform( requestedCrs, mapLayer->crs(), context.project()->transformContext() ) );
          }
          catch ( QgsCsException & )
          {
            throw QgsServerApiInternalServerError( u"Geometry could not be transformed to destination CRS"_s );
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
          const QVariantMap properties = QgsJsonUtils::parseJson( postData["properties"].dump() ).toMap();
          const QgsFields fields = mapLayer->fields();
          for ( const auto &field : fields )
          {
            if ( !QgsVariantUtils::isNull( properties.value( field.name() ) ) )
            {
              if ( !authorizedFieldNames.contains( field.name() ) )
              {
                throw QgsServerApiBadRequestException( u"Feature field %1 is not allowed"_s.arg( field.name() ) );
              }
              else
              {
                QVariant value = properties.value( field.name() );
                // Convert blobs
                if ( !QgsVariantUtils::isNull( properties.value( field.name() ) ) && static_cast<QMetaType::Type>( field.type() ) == QMetaType::QByteArray )
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
          throw QgsServerApiBadRequestException( u"Feature properties are not valid"_s );
        }

        // Make sure the first field (id) is null for shapefiles
        if ( mapLayer->providerType() == "ogr"_L1 && mapLayer->storageType() == "ESRI Shapefile"_L1 )
        {
          feat.setAttribute( 0, QVariant() );
        }
        feat.setId( FID_NULL );

        QgsVectorLayerUtils::matchAttributesToFields( feat, mapLayer->fields() );

        QgsFeatureList featuresToAdd( { feat } );
        if ( !mapLayer->dataProvider()->addFeatures( featuresToAdd ) )
        {
          throw QgsServerApiInternalServerError( u"Error adding feature to collection"_s );
        }

        feat = featuresToAdd.first();

        // Send response
        context.response()->setStatusCode( 201 );
        context.response()->setHeader( u"Content-Type"_s, u"application/geo+json"_s );

        QUrl collectionUrl { context.request()->url() };
        // Remove query and fragment
        collectionUrl.setQuery( QString() );
        collectionUrl.setFragment( QString() );

        QString url { collectionUrl.toString( QUrl::EncodeSpaces ) };
        if ( !url.endsWith( '/' ) )
        {
          url.append( '/' );
        }

        context.response()->setHeader( u"Location"_s, url + QString::number( feat.id() ) );
        context.response()->write( "\"string\"" );
      }
      catch ( json::exception &ex )
      {
        throw QgsServerApiBadRequestException( u"JSON parse error: %1"_s.arg( ex.what() ) );
      }
      break;
    }
    case QgsServerRequest::Method::OptionsMethod:
    {
      context.response()->setStatusCode( 200 );
      QStringList methods;
      methods << u"GET"_s << u"OPTIONS"_s;
      if ( canInsertFeatures( mapLayer, context ) )
      {
        methods << u"POST"_s;
      }
      if ( canDeleteFeatures( mapLayer, context ) )
      {
        methods << u"DELETE"_s;
      }
      if ( canUpdateFeatures( mapLayer, context ) )
      {
        methods << u"PUT"_s << u"PATCH"_s;
      }
      context.response()->setHeader( u"Allow"_s, methods.join( ", " ) );
      break;
    }
    // Error
    default:
    {
      throw QgsServerApiNotImplementedException( u"%1 method is not implemented."_s.arg( QgsServerRequest::methodToString( context.request()->method() ) ) );
    }
  } // end switch
}

QgsWfs3CollectionsFeatureHandler::QgsWfs3CollectionsFeatureHandler()
{
  setContentTypes( { QgsServerOgcApi::ContentType::GEOJSON, QgsServerOgcApi::ContentType::HTML, QgsServerOgcApi::ContentType::FLATGEOBUF } );
}

void QgsWfs3CollectionsFeatureHandler::handleRequest( const QgsServerApiContext &context ) const
{
  if ( !context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( u"Project is invalid or undefined"_s );
  }
  // Get parameters
  const QVariantMap params = values( context );

  // Check collectionId
  const QRegularExpressionMatch match { path().match( context.request()->url().path() ) };
  if ( !match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( u"Collection was not found"_s );
  }

  // CRS
  const QgsCoordinateReferenceSystem requestedCrs { QgsServerApiUtils::parseCrs( params[u"crs"_s].toString() ) };
  if ( !requestedCrs.isValid() )
  {
    throw QgsServerApiBadRequestException( u"Requested CRS is not valid"_s );
  }

  const QString collectionId { match.captured( u"collectionId"_s ) };
  // May throw if not found
  QgsVectorLayer *mapLayer { layerFromCollectionId( context, collectionId ) };
  Q_ASSERT( mapLayer );

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

  const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };

  // Retrieve feature from storage
  const QString featureId { match.captured( u"featureId"_s ) };
  QgsFeatureRequest featureRequest = filteredRequest( mapLayer, context );

  const QString fidExpression { QgsServerFeatureId::getExpressionFromServerFid( featureId, mapLayer->dataProvider() ) };
  if ( !fidExpression.isEmpty() )
  {
    QgsExpression *filterExpression { featureRequest.filterExpression() };
    if ( !filterExpression )
    {
      featureRequest.setFilterExpression( fidExpression );
    }
    else
    {
      featureRequest.setFilterExpression( u"(%1) AND (%2)"_s.arg( fidExpression, filterExpression->expression() ) );
    }
  }
  else
  {
    bool ok;
    featureRequest.setFilterFid( featureId.toLongLong( &ok ) );
    if ( !ok )
    {
      throw QgsServerApiInternalServerError( u"Invalid feature ID [%1]"_s.arg( featureId ) );
    }
  }
  QgsFeature feature;
  QgsFeatureIterator it { mapLayer->getFeatures( featureRequest ) };
  if ( !it.nextFeature( feature ) || !feature.isValid() )
  {
    throw QgsServerApiInternalServerError( u"Invalid feature [%1]"_s.arg( featureId ) );
  }

  const QList<QgsServerOgcApi::Profile> requestedProfiles = profilesFromRequest( context.request() );

  auto doGet = [&]() {
#ifdef HAVE_SERVER_PYTHON_PLUGINS
    QgsAccessControl *accessControl = context.serverInterface()->accessControls();
    //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
    //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
    auto filterRestorer = std::make_unique<QgsOWSServerFilterRestorer>();
    if ( accessControl )
    {
      QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
    }
#endif

    const QgsAttributeList requestedAttributes = featureRequest.subsetOfAttributes();

    QgsJsonExporter exporter { mapLayer };
    exporter.setAttributes( requestedAttributes );
    exporter.setAttributeDisplayName( true );
    exporter.setTransformGeometries( true );
    exporter.setSourceCrs( mapLayer->crs() );
    exporter.setDestinationCrs( requestedCrs );

    QMap<int, ReferencedLayerInfo> referencedInfo = gatherReferencedLayerInfo( mapLayer, context );
    // remove if attributes are not requested
    referencedInfo.erase(
      std::remove_if( referencedInfo.begin(), referencedInfo.end(), [&requestedAttributes]( const auto &info ) { return !requestedAttributes.contains( info.referencingFieldIdx ); } ),
      referencedInfo.end()
    );
    QgsServerOgcApi::Profile relAs = QgsServerOgcApi::Profile::Unset;
    bool hasReferencedObjects = !referencedInfo.isEmpty();

    json data = exporter.exportFeatureToJsonObject( feature );

    // Patch feature
    //  - IDs with server feature IDs
    //  - add references
    //  - format extra geometries
    data["id"] = featureId.toStdString();

    if ( hasReferencedObjects )
    {
      if ( requestedProfiles.contains( QgsServerOgcApi::Profile::RelAsUri ) )
        relAs = QgsServerOgcApi::Profile::RelAsUri;
      else if ( requestedProfiles.contains( QgsServerOgcApi::Profile::RelAsLink ) )
        relAs = QgsServerOgcApi::Profile::RelAsLink;
      else if ( requestedProfiles.contains( QgsServerOgcApi::Profile::RelAsKey ) )
        relAs = QgsServerOgcApi::Profile::RelAsKey;

      if ( relAs != QgsServerOgcApi::Profile::Unset )
      {
        for ( const auto &[fieldIdx, referencedInfo] : referencedInfo.toStdMap() )
        {
          const QgsField field = mapLayer->fields().field( fieldIdx );
          const QString fieldIdentifier = field.alias().isEmpty() ? field.name() : field.alias();
          const QVariant rawValue = feature.attribute( fieldIdx );
          data["properties"][fieldIdentifier.toStdString()] = relatedFeatureReference( rawValue, referencedInfo, relAs, context );
        }
      }
    }

    QMap<int, QString> extraGeomFields;
    const QList<int> requestedAttrs = featureRequest.subsetOfAttributes();
    for ( const int idx : std::as_const( requestedAttrs ) )
    {
      if ( mapLayer->fields().field( idx ).type() == QMetaType::Type::User && mapLayer->fields().field( idx ).typeName().compare( "geometry"_L1 ) == 0 )
      {
        extraGeomFields.insert( idx, mapLayer->fields().field( idx ).displayName() );
      }
    }

    // Add extra geometries as WKT
    for ( const auto &[fieldIdx, fieldIdentifier] : extraGeomFields.toStdMap() )
    {
      const QgsReferencedGeometry &refGeom = feature.attribute( fieldIdx ).value<QgsReferencedGeometry>();
      if ( refGeom.crs() != requestedCrs )
      {
        QgsReferencedGeometry transformedRefGeom = refGeom;
        const QgsCoordinateTransform ct( refGeom.crs(), requestedCrs, featureRequest.coordinateTransform().context() );
        transformedRefGeom.transform( ct );
        data["properties"][fieldIdentifier.toStdString()] = transformedRefGeom.asWkt().toStdString();
      }
      else
      {
        data["properties"][fieldIdentifier.toStdString()] = refGeom.asWkt().toStdString();
      }
    }

    // Add links
    data["links"] = links( context );
    json navigation = json::array();
    const QUrl url { context.request()->url() };
    navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 4 ).toStdString() } } );
    navigation.push_back( { { "title", "Collections" }, { "href", parentLink( url, 3 ).toStdString() } } );
    navigation.push_back( { { "title", title }, { "href", parentLink( url, 2 ).toStdString() } } );
    navigation.push_back( { { "title", "Items of " + title }, { "href", parentLink( url ).toStdString() } } );
    const json
      htmlMetadata { { "pageTitle", title + " - feature " + featureId.toStdString() }, { "geojsonUrl", href( context, "", QgsServerOgcApi::contentTypeToExtension( QgsServerOgcApi::ContentType::GEOJSON ) ) }, { "navigation", navigation } };

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
      if ( !wfstUpdateLayerIds.contains( mapLayer->id() )
           || !mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::ChangeGeometries )
           || !mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::ChangeAttributeValues ) )
      {
        throw QgsServerApiPermissionDeniedException( u"Features in layer '%1' cannot be changed"_s.arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerUpdatePermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( u"No ACL permissions to change features on layer '%1'"_s.arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      auto filterRestorer = std::make_unique<QgsOWSServerFilterRestorer>();
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif
      try
      {
        // Parse
        json postData = json::parse( context.request()->data().toStdString() );
        // Process data: extract geometry (because we need to process attributes in a much more complex way)
        const QgsFields fields( QgsOgrUtils::stringToFields( context.request()->data(), QTextCodec::codecForName( "UTF-8" ) ) );
        const QgsFeatureList features = QgsOgrUtils::stringToFeatureList( context.request()->data(), fields, QTextCodec::codecForName( "UTF-8" ) );
        if ( features.isEmpty() )
        {
          throw QgsServerApiBadRequestException( u"Posted data does not contain any feature"_s );
        }

        const QgsFeature feat = features.first();
        if ( !feat.isValid() )
        {
          throw QgsServerApiInternalServerError( u"Feature is not valid"_s );
        }

        QgsChangedAttributesMap changedAttributes;
        QgsAttributeMap changedMap;
        QgsGeometryMap changedGeometries;

        // Transform geometry
        if ( mapLayer->crs() != requestedCrs )
        {
          QgsGeometry geom { feat.geometry() };
          try
          {
            geom.transform( QgsCoordinateTransform( requestedCrs, mapLayer->crs(), context.project()->transformContext() ) );
          }
          catch ( QgsCsException & )
          {
            throw QgsServerApiInternalServerError( u"Geometry could not be transformed to destination CRS"_s );
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
          const QVariantMap properties = QgsJsonUtils::parseJson( postData["properties"].dump() ).toMap();
          const QgsFields fields = mapLayer->fields();
          int fieldIndex = 0;
          for ( const auto &field : fields )
          {
            if ( !QgsVariantUtils::isNull( properties.value( field.name() ) ) )
            {
              if ( !authorizedFieldNames.contains( field.name() ) )
              {
                throw QgsServerApiPermissionDeniedException( u"Feature field '%1' change is not allowed"_s.arg( field.name() ) );
              }
              else
              {
                QVariant value = properties.value( field.name() );
                // Convert blobs
                if ( !QgsVariantUtils::isNull( properties.value( field.name() ) ) && static_cast<QMetaType::Type>( field.type() ) == QMetaType::QByteArray )
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
          if ( !changedMap.isEmpty() )
          {
            changedAttributes.insert( feature.id(), changedMap );
          }
        }
        catch ( json::exception & )
        {
          throw QgsServerApiBadRequestException( u"Feature properties are not valid"_s );
        }

        // TODO: raise if nothing to change?

        if ( !mapLayer->dataProvider()->changeFeatures( changedAttributes, changedGeometries ) )
        {
          throw QgsServerApiInternalServerError( u"Error changing feature"_s );
        }

        // Now we need to send the updated feature to the client
        feature = mapLayer->getFeature( feature.id() );
        doGet();
      }
      catch ( json::exception &ex )
      {
        throw QgsServerApiBadRequestException( u"JSON parse error: %1"_s.arg( ex.what() ) );
      }
      break;
    }
    // //////////////////////////////////////////////////////////////
    // Patch feature
    case QgsServerRequest::Method::PatchMethod:
    {
      // First: check permissions
      const QStringList wfstUpdateLayerIds = QgsServerProjectUtils::wfstUpdateLayerIds( *context.project() );
      if ( !wfstUpdateLayerIds.contains( mapLayer->id() ) || !mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::ChangeAttributeValues ) )
      {
        throw QgsServerApiPermissionDeniedException( u"Feature attributes in layer '%1' cannot be changed"_s.arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerUpdatePermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( u"No ACL permissions to change features on layer '%1'"_s.arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      auto filterRestorer = std::make_unique<QgsOWSServerFilterRestorer>();
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif

      QgsChangedAttributesMap changedAttributes;
      QgsAttributeMap changedMap;
      const QgsGeometryMap changedGeometries; // This will be empty

      try
      {
        // Parse
        json postData = json::parse( context.request()->data().toStdString() );

        // If the request contains "add" we raise
        if ( postData.contains( "add" ) )
        {
          throw QgsServerApiNotImplementedException( u"\"add\" instruction in PATCH method is not implemented"_s, QString::fromStdString( QgsServerOgcApi::mimeType( contentTypeFromRequest( context.request() ) ) ), 400 );
        }

        // If the request does NOT contain "modify" we raise
        if ( !postData.contains( "modify" ) )
        {
          throw QgsServerApiBadRequestException( u"Missing \"modify\" instruction in PATCH method"_s );
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
          const QVariantMap properties = QgsJsonUtils::parseJson( postData["modify"].dump() ).toMap();
          const QgsFields fields = mapLayer->fields();
          int fieldIndex = 0;
          for ( const auto &field : fields )
          {
            if ( !QgsVariantUtils::isNull( properties.value( field.name() ) ) )
            {
              if ( !authorizedFieldNames.contains( field.name() ) )
              {
                throw QgsServerApiPermissionDeniedException( u"Feature field '%1' change is not allowed"_s.arg( field.name() ) );
              }
              else
              {
                QVariant value = properties.value( field.name() );
                // Convert blobs
                if ( !QgsVariantUtils::isNull( properties.value( field.name() ) ) && static_cast<QMetaType::Type>( field.type() ) == QMetaType::QByteArray )
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
          if ( !changedMap.isEmpty() )
          {
            changedAttributes.insert( feature.id(), changedMap );
          }
        }
        catch ( json::exception & )
        {
          throw QgsServerApiBadRequestException( u"Feature properties are not valid"_s );
        }
      }
      catch ( json::exception & )
      {
        throw QgsServerApiBadRequestException( u"Feature properties are not valid"_s );
      }

      if ( changedAttributes.isEmpty() && changedGeometries.isEmpty() )
      {
        QgsMessageLog::logMessage( u"Changeset is empty: no features have been modified"_s, u"Server"_s, Qgis::MessageLevel::Info );
      }

      if ( !mapLayer->dataProvider()->changeFeatures( changedAttributes, changedGeometries ) )
      {
        throw QgsServerApiInternalServerError( u"Error patching feature"_s );
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
      const QStringList wfstDeleteLayerIds = QgsServerProjectUtils::wfstDeleteLayerIds( *context.project() );
      if ( !wfstDeleteLayerIds.contains( mapLayer->id() ) || !mapLayer->dataProvider()->capabilities().testFlag( Qgis::VectorProviderCapability::DeleteFeatures ) )
      {
        throw QgsServerApiPermissionDeniedException( u"Features in layer '%1' cannot be deleted"_s.arg( mapLayer->name() ) );
      }

#ifdef HAVE_SERVER_PYTHON_PLUGINS

      // get access controls
      QgsAccessControl *accessControl = context.serverInterface()->accessControls();
      if ( accessControl && !accessControl->layerDeletePermission( mapLayer ) )
      {
        throw QgsServerApiPermissionDeniedException( u"No ACL permissions to delete features on layer '%1'"_s.arg( mapLayer->name() ) );
      }

      //scoped pointer to restore all original layer filters (subsetStrings) when pointer goes out of scope
      //there's LOTS of potential exit paths here, so we avoid having to restore the filters manually
      auto filterRestorer = std::make_unique<QgsOWSServerFilterRestorer>();
      if ( accessControl )
      {
        QgsOWSServerFilterRestorer::applyAccessControlLayerFilters( accessControl, mapLayer, filterRestorer->originalFilters() );
      }

#endif
      if ( !mapLayer->dataProvider()->deleteFeatures( { feature.id() } ) )
      {
        throw QgsServerApiInternalServerError( u"Error deleting feature '%1' from layer '%2'"_s.arg( featureId ).arg( mapLayer->name() ) );
      }

      // All good, empty response
      json data = nullptr;
      write( data, context );

      break;
    }
    // //////////////////////////////////////////////////////////////
    // Options feature
    case QgsServerRequest::Method::OptionsMethod:
    {
      // In theory we could check permissions for the requested feature here but
      // this method is used by QGIS client to determine the allowed operations on a feature,
      // so we need to return the allowed operations even if the requested feature is not accessible
      // otherwise clients won't be able to know that they can update or delete features in this collection.
      // So we check permissions at collection level, not at feature level.
      context.response()->setStatusCode( 200 );
      QStringList methods;
      methods << u"GET"_s << u"OPTIONS"_s;
      if ( canInsertFeatures( mapLayer, context ) )
      {
        methods << u"POST"_s;
      }
      if ( canDeleteFeatures( mapLayer, context ) )
      {
        methods << u"DELETE"_s;
      }
      if ( canUpdateFeatures( mapLayer, context ) )
      {
        methods << u"PUT"_s << u"PATCH"_s;
      }
      context.response()->setHeader( u"Allow"_s, methods.join( ", " ) );
      break;
    }
    default:
    {
      throw QgsServerApiNotImplementedException( u"%1 method is not implemented."_s.arg( QgsServerRequest::methodToString( context.request()->method() ) ) );
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
    const QString shortName { mapLayer->serverProperties()->shortName().isEmpty() ? mapLayer->name() : mapLayer->serverProperties()->shortName() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };
    const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/collections/%1/items/{featureId}"_s.arg( shortName ), context.request()->url() ).toStdString() };

    data[path] = {
      { "get",
        { { "tags", jsonTags() },
          { "summary", "Retrieve a single feature from the '" + title + "' feature collection" },
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() + '_' + "GET" },
          { "parameters",
            { { // array of objects
                { "$ref", "#/components/parameters/featureId" }
            } } },
          { "responses",
            { { "200",
                { { "description", "Retrieve a '" + title + "' feature by 'featureId'." },
                  { "content", { { "application/geo+json", { { "schema", { { "$ref", "#/components/schemas/featureGeoJSON" } } } } }, { "text/html", { { "schema", { { "type", "string" } } } } } } } } },
              { "default", defaultResponse() } } } } },
      { "put",
        { { "summary", "Replaces the feature with ID {featureId} in the collection {collectionId}" },
          { "tags", { "edit", "replace" } },
          { "description", "Replaces the feature with ID {featureId} in the collection {collectionId}" },
          { "operationId", operationId() + "PUT" },
          { "responses",
            { {
                "200",
                { { "description", "The feature was successfully updated" } },
              },
              {
                "403",
                { { "description", "Forbidden: the operation requested was not authorized" } },
              },
              {
                "500",
                { { "description", "Posted data could not be parsed correctly or another error occurred" } },
              },
              { "default", defaultResponse() } } } } },
      { "patch",
        { { "summary", "Changes attributes of feature with ID {featureId} in the collection {collectionId}" },
          { "tags", { "edit" } },
          { "description", "Changes attributes of feature with ID {featureId} in the collection {collectionId}" },
          { "operationId", operationId() + "PATCH" },
          { "responses",
            { {
                "200",
                { { "description", "The feature was successfully updated" } },
              },
              {
                "403",
                { { "description", "Forbidden: the operation requested was not authorized" } },
              },
              {
                "500",
                { { "description", "Posted data could not be parsed correctly or another error occurred" } },
              },
              { "default", defaultResponse() } } } } },
      { "delete",
        { { "summary", "Deletes the feature with ID {featureId} in the collection {collectionId}" },
          { "tags", { "edit", "delete" } },
          { "description", "Deletes the feature with ID {featureId} in the collection {collectionId}" },
          { "operationId", operationId() + "DELETE" },
          { "responses",
            { {
                "201",
                { { "description", "The feature was successfully deleted from the collection" } },
              },
              {
                "403",
                { { "description", "Forbidden: the operation requested was not authorized" } },
              },
              { "500", { { "description", "Posted data could not be parsed correctly or another error occurred" } } },
              { "default", defaultResponse() } } } } }
    };


    // If the layer has no delete capabilities, remove the delete operation
    if ( !canDeleteFeatures( mapLayer, context ) )
    {
      data[path].erase( "delete" );
    }
    // If the layer has no update capabilities, remove the put and patch operation
    if ( !canUpdateFeatures( mapLayer, context ) )
    {
      data[path].erase( "put" );
      data[path].erase( "patch" );
    }

  } // end for loop
  return data;
}

QString QgsWfs3AbstractItemsHandler::uri( const QString &collectionId, const QMap<QString, QVariant> &fieldValueMap, const QgsServerApiContext &context, QgsServerOgcApi::ContentType contentType )
{
  // Retrieve the layer from the collection ID
  if ( !context.project() )
  {
    throw QgsServerApiImproperlyConfiguredException( u"Project is invalid or undefined"_s );
  }
  // May throw not found exception if the collectionId is invalid
  QgsVectorLayer *mapLayer { layerFromCollectionId( context, collectionId ) };

  // This will never happen (it's just a hint for reviewers)
  Q_ASSERT( mapLayer );

  // Check if the layer is published, raise not found if it is not
  checkLayerIsAccessible( mapLayer, context );

  // Collect info about pks from the layer and the fieldValueMap
  const QList<int> pkList = mapLayer->primaryKeyAttributes();
  QSet<QString> pkFieldNames;
  QString firstPkFieldName;
  for ( const int &pkIdx : std::as_const( pkList ) )
  {
    const QgsField field = mapLayer->fields().field( pkIdx );
    const QString fieldNameOrAlias = field.alias().isEmpty() ? field.name() : field.alias();
    pkFieldNames.insert( fieldNameOrAlias );
    if ( firstPkFieldName.isEmpty() )
    {
      firstPkFieldName = fieldNameOrAlias;
    }
  }

  const bool providerHasPk { pkList.size() > 0 };

  // Note: if "id" is the only field in the fieldValueMap and it is not a pk field of the provider,
  // we can still use it as featureId as long as the provider does not have any pk field.
  // In that case, we will assume that "id" is a unique identifier for features in that layer.
  const bool valueMapHasSinglePkId = ( fieldValueMap.size() == 1 )
                                     && ( ( providerHasPk && fieldValueMap.contains( firstPkFieldName ) ) || ( !providerHasPk && fieldValueMap.firstKey() == "id"_L1 && !pkFieldNames.contains( "id"_L1 ) ) );

  const QStringList fieldValueMapKeys = fieldValueMap.keys();
  const bool valueMapsHasMultiplePkIds = pkFieldNames == QSet<QString>( fieldValueMapKeys.constBegin(), fieldValueMapKeys.constEnd() );

  QUrl cleanedUrl { context.request()->url() };
  cleanedUrl.setQuery( removeOffsetAndLimit( QUrlQuery( cleanedUrl.query() ) ) );

  // 1 - Simple case: fieldValueMap contains a single entry with the field that is used as featureId (e.g. primary key)
  if ( valueMapHasSinglePkId )
  {
    const QString featureId { fieldValueMap.first().toString() };
    cleanedUrl.setPath( context.matchedPath() + u"/collections/%1/items/%2"_s.arg( collectionId ).arg( featureId ) );
  }
  // 2 - Complex case: fieldValueMap contains multiple entries that are only and all PKs
  else if ( valueMapsHasMultiplePkIds )
  {
    // Build the ID
    QgsFeature feature( mapLayer->fields() );
    for ( auto it = fieldValueMap.constBegin(); it != fieldValueMap.constEnd(); ++it )
    {
      const QString fieldName { it.key() };
      // Retrieve the field index from the exposed name/alias
      int fieldIdx = -1;
      for ( int idx = 0; idx < mapLayer->fields().count(); ++idx )
      {
        if ( mapLayer->fields().field( idx ).displayName() == fieldName )
        {
          fieldIdx = idx;
          break;
        }
      }
      if ( fieldIdx == -1 )
      {
        throw QgsServerApiImproperlyConfiguredException( u"Field '%1' does not exist in layer '%2'"_s.arg( fieldName ).arg( mapLayer->name() ) );
      }
      feature.setAttribute( fieldIdx, it.value() );
    }
    const QString featureId { QgsServerFeatureId::getServerFid( feature, pkList ) };
    cleanedUrl.setPath( context.matchedPath() + u"/collections/%1/items/%2"_s.arg( collectionId ).arg( featureId ) );
  }
  // 3 - Wild case: fieldValueMap contains multiple entries that are not all and only PKs, we cannot be sure about the featureId
  //     so we build an URI that retrieve the features by filtering by all the field values, but we cannot be sure that it will
  //     return a single feature, so this is a fallback solution
  else if ( !fieldValueMap.isEmpty() )
  {
    const QgsFields availableFields { publishedFields( mapLayer, context ) };
    cleanedUrl.setPath( context.matchedPath() + u"/collections/%1/items.json"_s.arg( collectionId ) );
    QUrlQuery query { cleanedUrl.query() };
    for ( auto it = fieldValueMap.constBegin(); it != fieldValueMap.constEnd(); ++it )
    {
      const QString fieldName { it.key() };
      // Throw if the field is not part of the published fields, as it means that we cannot use it for filtering
      if ( availableFields.lookupField( fieldName ) < 0 )
      {
        throw QgsServerApiImproperlyConfiguredException( u"Field '%1' is not part of the published fields for layer '%2', cannot be used for filtering"_s.arg( fieldName ).arg( mapLayer->name() ) );
      }
      const QVariant fieldValue( it.value() );
      query.addQueryItem( fieldName, fieldValue.toString() );
    }
    cleanedUrl.setQuery( query );
  }

  // Add extension from contentType if not already present in the url path
  // Remove any existing extension
  const auto suffixLength { QFileInfo( cleanedUrl.path() ).suffix().length() };
  if ( suffixLength > 0 )
  {
    auto path { cleanedUrl.path() };
    path.truncate( path.length() - ( suffixLength + 1 ) );
    cleanedUrl.setPath( path );
  }

  const QString extension { QgsServerOgcApi::contentTypeToExtension( contentType ) };

  // (re-)add extension
  // JSON is the default anyway so we don't need to add it
  if ( !extension.isEmpty() )
  {
    // Remove trailing slashes if any.
    QString path { cleanedUrl.path() };
    while ( path.endsWith( '/' ) )
    {
      path.chop( 1 );
    }
    cleanedUrl.setPath( path + '.' + extension );
  }
  return QgsServerOgcApi::sanitizeUrl( cleanedUrl ).toString( QUrl::FullyEncoded );
}

const QString QgsWfs3ConformanceHandler::templatePath( const QgsServerApiContext &context ) const
{
  // resources/server/api + /ogc/templates/wfs3/ + operationId() + .html
  QString path { context.serverInterface()->serverSettings()->apiResourcesDirectory() };
  path += "/ogc/templates/wfs3/"_L1;
  path += QString::fromStdString( operationId() );
  path += ".html"_L1;
  return path;
}

QgsWfs3CollectionsSchemaHandler::QgsWfs3CollectionsSchemaHandler()
{
  setContentTypes( { QgsServerOgcApi::ContentType::SCHEMA_JSON, QgsServerOgcApi::ContentType::HTML } );
}

void QgsWfs3CollectionsSchemaHandler::handleRequest( const QgsServerApiContext &context ) const
{
  switch ( context.request()->method() )
  {
    case QgsServerRequest::Method::GetMethod:
    {
      if ( !context.project() )
      {
        throw QgsServerApiImproperlyConfiguredException( u"Project is invalid or undefined"_s );
      }
      // Check collectionId
      const QRegularExpressionMatch match { path().match( context.request()->url().path() ) };
      if ( !match.hasMatch() )
      {
        throw QgsServerApiNotFoundError( u"Collection was not found"_s );
      }
      const QString collectionId { match.captured( u"collectionId"_s ) };
      // May throw if not found
      QgsVectorLayer *mapLayer { layerFromCollectionId( context, collectionId ) };
      Q_ASSERT( mapLayer );

      const QgsProject *project = context.project();
      const QStringList wfsLayerIds = QgsServerProjectUtils::wfsLayerIds( *project );
      if ( !wfsLayerIds.contains( mapLayer->id() ) )
      {
        throw QgsServerApiNotFoundError( u"Collection was not found"_s );
      }

      // Check if the layer is published, raise not found if it is not
      checkLayerIsAccessible( mapLayer, context );

      const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };

      QUrl idUrl { context.request()->baseUrl() };
      // Remove query and fragment from URL
      idUrl.setQuery( QString() );
      idUrl.setFragment( QString() );

      json data {
        { "$schema", "https://json-schema.org/draft/2020-12/schema" },
        { "$id", idUrl.toDisplayString().toStdString() },
        { "title", title },
        { "type", "object" },
        // List of required fields
        { "required", json::array() },
        // Object of fields with title, type x-ogc-role and x-ogc-propertySeq
        { "properties", {} }
      };

      if ( !mapLayer->serverProperties()->abstract().isEmpty() )
      {
        data["description"] = mapLayer->serverProperties()->abstract().toStdString();
      }

      gatherLayerFieldsInfo( data, mapLayer, context );

      // Add links if content type is not schema json to avoid validation issues with the schema
      if ( QgsServerOgcApiHandler::contentTypeFromRequest( context.request() ) != QgsServerOgcApi::ContentType::SCHEMA_JSON )
      {
        data["links"] = links( context );
      }

      json navigation = json::array();
      const QUrl url { context.request()->url() };
      navigation.push_back( { { "title", "Landing page" }, { "href", parentLink( url, 3 ).toStdString() } } );
      navigation.push_back( { { "title", "Collections" }, { "href", parentLink( url, 2 ).toStdString() } } );
      navigation.push_back( { { "title", title }, { "href", parentLink( url, 1 ).toStdString() } } );
      write( data, context, { { "pageTitle", linkTitle() + " of feature in '" + mapLayer->name().toStdString() + "'" }, { "navigation", navigation } } );
      break;
    }
    default:
      throw QgsServerApiNotImplementedException( u"%1 method is not implemented."_s.arg( QgsServerRequest::methodToString( context.request()->method() ) ) );
  }
}

json QgsWfs3CollectionsSchemaHandler::schema( const QgsServerApiContext &context ) const
{
  json data;
  Q_ASSERT( context.project() );

  const QVector<QgsVectorLayer *> layers { QgsServerApiUtils::publishedWfsLayers<QgsVectorLayer *>( context ) };
  for ( const auto &mapLayer : layers )
  {
    const QString shortName { mapLayer->serverProperties()->shortName().isEmpty() ? mapLayer->name() : mapLayer->serverProperties()->shortName() };
    // Use layer id for operationId
    const QString layerId { mapLayer->id() };
    const std::string title { mapLayer->serverProperties()->wfsTitle().isEmpty() ? mapLayer->name().toStdString() : mapLayer->serverProperties()->wfsTitle().toStdString() };
    const std::string path { QgsServerApiUtils::appendMapParameter( context.apiRootPath() + u"/collections/%1/schema"_s.arg( shortName ), context.request()->url() ).toStdString() };

    data[path] = {
      { "get",
        { { "tags", jsonTags() },
          { "summary", "Retrieve the schema of the '" + title + "' feature collection" },
          { "description", description() },
          { "operationId", operationId() + '_' + layerId.toStdString() + '_' + "GET" },
          { "responses",
            { { "200",
                { { "description", "Retrieve the schema of the '" + title + "' feature collection." },
                  { "content", { { "application/schema+json", { { "schema", { { "$ref", "#/components/schemas/anyObject" } } } } } } } } },
              { "default", defaultResponse() } } } } }
    };
  }
  return data;
}


QList<QgsServerQueryStringParameter> QgsWfs3CollectionsFeatureHandler::parameters( const QgsServerApiContext &context ) const
{
  QList<QgsServerQueryStringParameter> params;
  // CRS
  QgsServerQueryStringParameter
    requestedCrs { u"crs"_s, false, QgsServerQueryStringParameter::Type::String, u"The coordinate reference system of the response geometries."_s, u"http://www.opengis.net/def/crs/OGC/1.3/CRS84"_s };
  auto crsValidator = [context]( const QgsServerApiContext &, QVariant &value ) -> bool { return QgsServerApiUtils::publishedCrsList( context.project() ).contains( value.toString() ); };
  requestedCrs.setCustomValidator( crsValidator );
  params.push_back( requestedCrs );
  return params;
}
