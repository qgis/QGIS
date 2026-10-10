#include "qgsagentplugin.h"

#include "qgsexpression.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature.h"
#include "qgsfeaturerequest.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsvectorlayer.h"

using namespace Qt::StringLiterals;

namespace
{
  QMetaType::Type fieldType( const QString &name )
  {
    const QString normalized = name.trimmed().toLower();
    if ( normalized == u"int"_s || normalized == u"integer"_s )
      return QMetaType::Type::Int;
    if ( normalized == u"long"_s || normalized == u"int64"_s )
      return QMetaType::Type::LongLong;
    if ( normalized == u"double"_s || normalized == u"float"_s || normalized == u"number"_s )
      return QMetaType::Type::Double;
    if ( normalized == u"bool"_s || normalized == u"boolean"_s )
      return QMetaType::Type::Bool;
    if ( normalized == u"date"_s )
      return QMetaType::Type::QDate;
    if ( normalized == u"datetime"_s )
      return QMetaType::Type::QDateTime;
    return QMetaType::Type::QString;
  }

  bool prepareFeatureRequest( QgsVectorLayer *layer, const QString &expressionText, bool needsGeometry, QgsFeatureRequest &request, QString &error )
  {
    QgsExpression expression( expressionText );
    if ( expressionText.isEmpty() || expression.hasParserError() )
    {
      error = expressionText.isEmpty() ? QObject::tr( "An expression is required." ) : expression.parserErrorString();
      return false;
    }
    request.setFilterExpression( expressionText );
    request.setExpressionContext( QgsExpressionContext( QgsExpressionContextUtils::globalProjectLayerScopes( layer ) ) );
    if ( !needsGeometry )
      request.setFlags( Qgis::FeatureRequestFlag::NoGeometry );
    return true;
  }
}

QJsonObject QgsAgentServer::editVectorLayer( const QJsonObject &arguments )
{
  QString error;
  QgsVectorLayer *layer = qobject_cast<QgsVectorLayer *>( findLayer( arguments.value( u"layer"_s ).toString(), error ) );
  if ( !layer )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, error.isEmpty() ? tr( "The requested layer is not a vector layer." ) : error } };
  const QString action = arguments.value( u"action"_s ).toString();
  const QString reason = arguments.value( u"reason"_s ).toString();
  if ( !confirmWriteAction( tr( "Edit QGIS layer?" ), tr( "Layer: %1\nAction: %2\nReason: %3" ).arg( layer->name(), action, reason ) ) )
    return QJsonObject{ { u"ok"_s, false }, { u"cancelled"_s, true } };
  if ( !layer->isEditable() && !layer->startEditing() )
    return QJsonObject{ { u"ok"_s, false }, { u"error"_s, tr( "Could not start editing the layer." ) } };

  layer->beginEditCommand( tr( "QGIS Agent: %1" ).arg( action ) );
  bool ok = true;
  qint64 changedCount = 0;
  const QString fieldName = arguments.value( u"field"_s ).toString();

  if ( action == u"add_field"_s )
  {
    ok = !fieldName.isEmpty() && layer->fields().indexFromName( fieldName ) < 0
         && layer->addAttribute( QgsField( fieldName, fieldType( arguments.value( u"field_type"_s ).toString() ) ) );
    changedCount = ok ? 1 : 0;
  }
  else if ( action == u"rename_field"_s )
  {
    const int index = layer->fields().indexFromName( fieldName );
    const QString newName = arguments.value( u"new_name"_s ).toString();
    ok = index >= 0 && !newName.isEmpty() && layer->fields().indexFromName( newName ) < 0 && layer->renameAttribute( index, newName );
    changedCount = ok ? 1 : 0;
  }
  else if ( action == u"delete_field"_s )
  {
    const int index = layer->fields().indexFromName( fieldName );
    ok = index >= 0 && layer->deleteAttribute( index );
    changedCount = ok ? 1 : 0;
  }
  else if ( action == u"update_attributes"_s || action == u"update_geometry"_s || action == u"delete_features"_s )
  {
    QgsFeatureRequest request;
    if ( !prepareFeatureRequest( layer, arguments.value( u"expression"_s ).toString(), action == u"update_geometry"_s, request, error ) )
      ok = false;
    const QJsonObject values = arguments.value( u"values"_s ).toObject();
    const QString geometryWkt = arguments.value( u"geometry_wkt"_s ).toString();
    QgsGeometry replacementGeometry = QgsGeometry::fromWkt( geometryWkt );
    if ( action == u"update_attributes"_s && values.isEmpty() )
      ok = false;
    if ( action == u"update_geometry"_s && ( geometryWkt.isEmpty() || replacementGeometry.isNull() ) )
      ok = false;

    if ( ok )
    {
      QgsFeature feature;
      QgsFeatureIterator iterator = layer->getFeatures( request );
      while ( iterator.nextFeature( feature ) )
      {
        bool changed = true;
        if ( action == u"delete_features"_s )
        {
          changed = layer->deleteFeature( feature.id() );
        }
        else if ( action == u"update_geometry"_s )
        {
          QgsGeometry geometry = replacementGeometry;
          changed = layer->changeGeometry( feature.id(), geometry );
        }
        else
        {
          for ( auto it = values.constBegin(); it != values.constEnd(); ++it )
          {
            const int fieldIndex = layer->fields().indexFromName( it.key() );
            if ( fieldIndex < 0 || !layer->changeAttributeValue( feature.id(), fieldIndex, it.value().toVariant() ) )
            {
              changed = false;
              break;
            }
          }
        }
        ok = ok && changed;
        if ( changed )
          ++changedCount;
      }
    }
  }
  else if ( action == u"add_feature"_s )
  {
    QgsFeature feature( layer->fields() );
    const QString geometryWkt = arguments.value( u"geometry_wkt"_s ).toString();
    if ( !geometryWkt.isEmpty() )
      feature.setGeometry( QgsGeometry::fromWkt( geometryWkt ) );
    const QJsonObject attributes = arguments.value( u"attributes"_s ).toObject();
    for ( auto it = attributes.constBegin(); it != attributes.constEnd(); ++it )
    {
      if ( layer->fields().indexFromName( it.key() ) >= 0 )
        feature.setAttribute( it.key(), it.value().toVariant() );
    }
    ok = layer->addFeature( feature );
    changedCount = ok ? 1 : 0;
  }
  else
  {
    ok = false;
  }

  if ( ok )
  {
    layer->endEditCommand();
    layer->updateFields();
    layer->triggerRepaint();
    emit layerEdited( layer->id() );
  }
  else
  {
    layer->destroyEditCommand();
  }
  return QJsonObject{
    { u"ok"_s, ok },
    { u"layer_id"_s, layer->id() },
    { u"action"_s, action },
    { u"changed_count"_s, changedCount },
    { u"error"_s, ok ? QString() : error.isEmpty() ? tr( "The edit action failed or its arguments were invalid." ) : error },
  };
}
