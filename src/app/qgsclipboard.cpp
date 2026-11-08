/***************************************************************************
     qgsclipboard.cpp  -  QGIS internal clipboard for storage of features
     --------------------------------------------------------------------
    begin                : 20 May, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsclipboard.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgsjsonutils.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmemoryproviderutils.h"
#include "qgsmessagebar.h"
#include "qgsogrutils.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsvectortilelayer.h"

#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTextCodec>

#include "moc_qgsclipboard.cpp"

using namespace Qt::StringLiterals;

const QString QgsClipboard::FEATURES_MIME_TYPE = u"application/qgis.featurelist"_s;

QgsClipboard::QgsClipboard()
{
  connect( QApplication::clipboard(), &QClipboard::dataChanged, this, &QgsClipboard::systemClipboardChanged );
}

void QgsClipboard::replaceWithCopyOf( QgsVectorLayer *src )
{
  if ( !src )
    return;

  setSystemClipboard( src, src->crs(), src->fields(), src->selectedFeatures() );
  emit changed();
}

void QgsClipboard::replaceWithCopyOf( QgsVectorTileLayer *src )
{
  if ( !src )
    return;

  // things are a bit tricky for vector tile features, as each will have different fields
  // so we build a "super set" of fields first, and then make sure each feature has that superset present

  const QList<QgsFeature> selectedFeatures = src->selectedFeatures();
  QgsFields supersetFields;
  for ( const QgsFeature &feature : selectedFeatures )
  {
    const QgsFields fields = feature.fields();
    for ( const QgsField &field : fields )
    {
      if ( supersetFields.lookupField( field.name() ) == -1 )
      {
        supersetFields.append( field );
      }
    }
  }

  QgsFeatureList featuresToCopy;
  for ( const QgsFeature &feature : selectedFeatures )
  {
    QgsFeature superSetFeature = feature;
    QgsVectorLayerUtils::matchAttributesToFields( superSetFeature, supersetFields );
    featuresToCopy.append( superSetFeature );
  }

  QgsDebugMsgLevel( u"replaced QGIS clipboard."_s, 2 );

  setSystemClipboard( src, src->crs(), supersetFields, featuresToCopy );
  emit changed();
}

void QgsClipboard::replaceWithCopyOf( QgsFeatureStore &featureStore, QgsVectorLayer *src )
{
  QgsDebugMsgLevel( u"features count = %1"_s.arg( featureStore.features().size() ), 2 );
  setSystemClipboard( src, featureStore.crs(), featureStore.fields(), featureStore.features() );
  emit changed();
}

void QgsClipboard::setSystemClipboard( QgsMapLayer *sourceLayer, const QgsCoordinateReferenceSystem &crs, const QgsFields &fields, const QgsFeatureList &features )
{
  QClipboard *cb = QApplication::clipboard();

  QgsFeatureListMimeData *m = new QgsFeatureListMimeData( sourceLayer, crs, fields, features );
  cb->setMimeData( m, QClipboard::Clipboard );
}

QgsFeatureList QgsClipboard::stringToFeatureList( const QString &string, const QgsFields &fields ) const
{
  //first try using OGR to read string
  QgsFeatureList features = QgsOgrUtils::stringToFeatureList( string, fields, QTextCodec::codecForName( "System" ) );

  if ( !features.isEmpty() )
    return features;

  // otherwise try to read in as WKT
  if ( string.isEmpty() || string.split( '\n' ).count() == 0 )
    return features;

  // Poor man's csv parser
  bool isInsideQuotes { false };
  QgsAttributes attrs;
  QgsGeometry geom;
  QString attrVal;
  bool isFirstLine { string.startsWith( "wkt_geom"_L1 ) };
  // it seems there is no other way to check for header
  const bool hasHeader { string.startsWith( "wkt_geom"_L1 ) };
  QgsGeometry geometry;
  bool setFields { fields.isEmpty() };
  QgsFields fieldsFromClipboard;

  auto parseFunc = [&]( const QChar &c ) {
    // parse geom only if it wasn't successfully set before
    if ( geometry.isNull() )
    {
      geometry = QgsGeometry::fromWkt( attrVal );
    }

    if ( isFirstLine ) // ... name
    {
      if ( attrVal != "wkt_geom"_L1 ) // ignore this one
      {
        fieldsFromClipboard.append( QgsField { attrVal, QMetaType::Type::QString } );
      }
    }
    else // ... or value
    {
      attrs.append( attrVal );
    }

    // end of record, create a new feature if it's not the header
    if ( c == QChar( '\n' ) )
    {
      if ( isFirstLine )
      {
        isFirstLine = false;
      }
      else
      {
        QgsFeature feature { setFields ? fieldsFromClipboard : fields };
        feature.setGeometry( geometry );
        if ( hasHeader || !geometry.isNull() )
        {
          attrs.pop_front();
        }
        feature.setAttributes( attrs );
        features.append( feature );
        geometry = QgsGeometry();
        attrs.clear();
      }
    }
    attrVal.clear();
  };

  for ( auto c = string.constBegin(); c < string.constEnd(); ++c )
  {
    if ( *c == QChar( '\n' ) || *c == QChar( '\t' ) )
    {
      if ( isInsideQuotes )
      {
        attrVal.append( *c );
      }
      else
      {
        parseFunc( *c );
      }
    }
    else if ( *c == QChar( '\"' ) )
    {
      isInsideQuotes = !isInsideQuotes;
    }
    else
    {
      attrVal.append( *c );
    }
  }

  // handle missing newline
  if ( !string.endsWith( QChar( '\n' ) ) )
  {
    parseFunc( QChar( '\n' ) );
  }

  return features;
}

QgsFields QgsClipboard::retrieveFields() const
{
  QClipboard *cb = QApplication::clipboard();

  const QString string = cb->text( QClipboard::Clipboard );

  QgsFields f = QgsOgrUtils::stringToFields( string, QTextCodec::codecForName( "System" ) );
  if ( f.size() < 1 )
  {
    if ( string.isEmpty() )
    {
      return f;
    }

    //wkt?
    const QString firstLine = string.section( '\n', 0, 0 );
    if ( !firstLine.isEmpty() )
    {
      const QStringList fieldNames = firstLine.split( '\t' );
      //wkt / text always has wkt_geom as first attribute (however values can be NULL)
      if ( fieldNames.at( 0 ) != "wkt_geom"_L1 )
      {
        return f;
      }

      for ( int i = 0; i < fieldNames.size(); ++i )
      {
        const QString fieldName = fieldNames.at( i );
        if ( fieldName == "wkt_geom"_L1 )
        {
          continue;
        }

        f.append( QgsField( fieldName, QMetaType::Type::QString ) );
      }
    }
  }
  return f;
}

bool QgsClipboard::getFeaturesFromClipboard( QgsCoordinateReferenceSystem &crs, QgsFields &fields, QgsFeatureList &features ) const
{
  if ( const QgsFeatureListMimeData *featureListData = featureListDataFromClipboard() )
  {
    crs = featureListData->mCRS;
    fields = featureListData->mFields;
    features = featureListData->mFeatures;
    return true;
  }

  const QMimeData *data = QApplication::clipboard()->mimeData();
  if ( !data )
    return false;

  if ( data->hasFormat( FEATURES_MIME_TYPE ) )
  {
    const QByteArray clipboardData = data->data( FEATURES_MIME_TYPE );
    QDataStream stream( clipboardData );
    QString crsWkt;
    stream >> crsWkt >> fields >> features;
    crs = QgsCoordinateReferenceSystem::fromWkt( crsWkt );
    return true;
  }
  else
  {
    fields = retrieveFields();
    features = getFeaturesFromClipboardText( fields );
    // unknown
    crs = QgsCoordinateReferenceSystem();
    return true;
  }
}

QgsFeatureList QgsClipboard::getFeaturesFromClipboardText( const QgsFields &fields ) const
{
  QgsDebugMsgLevel( u"returning clipboard."_s, 2 );
  QClipboard *cb = QApplication::clipboard();
  const QMimeData *mimeData = cb->mimeData( QClipboard::Clipboard );
  if ( !mimeData )
    return {};

  QString text = mimeData->text();

  if ( text.endsWith( '\n' ) )
  {
    text.chop( 1 );
    // In case Windows <EOL> marker (CRLF) makes it into the variable "text"
    if ( text.endsWith( '\r' ) )
    {
      text.chop( 1 );
    }
  }

  return stringToFeatureList( text, fields );
}

bool QgsClipboard::isEmpty() const
{
  if ( const QgsFeatureListMimeData *featureListData = featureListDataFromClipboard() )
  {
    return featureListData->mFeatures.isEmpty();
  }

  QClipboard *cb = QApplication::clipboard();
  const QMimeData *data = cb->mimeData( QClipboard::Clipboard );
  if ( !data )
    return true;

  if ( data->hasFormat( FEATURES_MIME_TYPE ) )
  {
    // can't be empty if it has this mime type
    return false;
  }

  return !data->hasText() || data->text().isEmpty();
}

QgsFeatureList QgsClipboard::transformedCopyOf( QgsVectorLayer *destinationLayer ) const
{
  if ( !destinationLayer )
    return {};

  QgsCoordinateReferenceSystem clipboardCrs;
  QgsFields clipboardFields;
  QgsFeatureList featureList;
  getFeaturesFromClipboard( clipboardCrs, clipboardFields, featureList );

  if ( clipboardCrs.isValid() && destinationLayer->crs().isValid() && clipboardCrs != destinationLayer->crs() )
  {
    QgisApp::instance()->askUserForDatumTransform( clipboardCrs, destinationLayer->crs() );

    const QgsCoordinateTransform ct = QgsCoordinateTransform( clipboardCrs, destinationLayer->crs(), QgsProject::instance() );

    QgsDebugMsgLevel( u"transforming clipboard."_s, 2 );
    if ( !ct.isShortCircuited() )
    {
      for ( QgsFeatureList::iterator iter = featureList.begin(); iter != featureList.end(); ++iter )
      {
        QgsGeometry g = iter->geometry();
        g.transform( ct );
        iter->setGeometry( g );
      }
    }
  }

  return QgsVectorLayerUtils::makeFeaturesCompatible( featureList, destinationLayer );
}

void QgsClipboard::setData( const QString &mimeType, const QByteArray &data, const QString &text )
{
  QMimeData *mdata = new QMimeData();
  mdata->setData( mimeType, data );
  if ( !text.isEmpty() )
  {
    mdata->setText( text );
  }
  // Transfers ownership to the clipboard object
  QApplication::clipboard()->setMimeData( mdata, QClipboard::Clipboard );
}

void QgsClipboard::setText( const QString &text )
{
  QApplication::clipboard()->setText( text, QClipboard::Clipboard );
}

bool QgsClipboard::hasFormat( const QString &mimeType ) const
{
  if ( const QMimeData *data = QApplication::clipboard()->mimeData() )
    return data->hasFormat( mimeType );
  return false;
}

QByteArray QgsClipboard::data( const QString &mimeType ) const
{
  if ( const QMimeData *data = QApplication::clipboard()->mimeData() )
    return data->data( mimeType );
  return QByteArray();
}

QgsMapLayer *QgsClipboard::layer() const
{
  if ( const QgsFeatureListMimeData *featureListData = featureListDataFromClipboard() )
  {
    return featureListData->mSourceLayer;
  }
  else
  {
    return nullptr;
  }
}

std::unique_ptr<QgsVectorLayer> QgsClipboard::pasteToNewMemoryVector( QgsMessageBar *messageBar )
{
  QgsCoordinateReferenceSystem crs;
  QgsFields fields;
  QgsFeatureList features;
  getFeaturesFromClipboard( crs, fields, features );

  // Decide geometry type from features, switch to multi type if at least one multi is found
  QMap<Qgis::WkbType, int> typeCounts;
  for ( const QgsFeature &feature : std::as_const( features ) )
  {
    if ( !feature.hasGeometry() )
      continue;

    const Qgis::WkbType type = feature.geometry().wkbType();

    if ( type == Qgis::WkbType::Unknown || type == Qgis::WkbType::NoGeometry )
      continue;

    if ( QgsWkbTypes::isSingleType( type ) )
    {
      if ( typeCounts.contains( QgsWkbTypes::multiType( type ) ) )
      {
        typeCounts[QgsWkbTypes::multiType( type )] = typeCounts[QgsWkbTypes::multiType( type )] + 1;
      }
      else
      {
        typeCounts[type] = typeCounts[type] + 1;
      }
    }
    else if ( QgsWkbTypes::isMultiType( type ) )
    {
      if ( typeCounts.contains( QgsWkbTypes::singleType( type ) ) )
      {
        // switch to multi type
        typeCounts[type] = typeCounts[QgsWkbTypes::singleType( type )];
        typeCounts.remove( QgsWkbTypes::singleType( type ) );
      }
      typeCounts[type] = typeCounts[type] + 1;
    }
  }

  const Qgis::WkbType wkbType = !typeCounts.isEmpty() ? typeCounts.constBegin().key() : Qgis::WkbType::NoGeometry;

  if ( features.isEmpty() )
  {
    // should not happen
    if ( messageBar )
      messageBar->pushMessage( tr( "Paste features" ), tr( "No features in clipboard." ), Qgis::MessageLevel::Info );
    return nullptr;
  }
  else if ( typeCounts.size() > 1 )
  {
    QString typeName = wkbType != Qgis::WkbType::NoGeometry ? QgsWkbTypes::displayString( wkbType ) : u"none"_s;
    if ( messageBar )
      messageBar->pushMessage( tr( "Paste features" ), tr( "Multiple geometry types found, features with geometry different from %1 will be created without geometry." ).arg( typeName ), Qgis::MessageLevel::Info );
  }

  std::unique_ptr<QgsVectorLayer> layer( QgsMemoryProviderUtils::createMemoryLayer( u"pasted_features"_s, QgsFields(), wkbType, crs ) );

  if ( !layer->isValid() || !layer->dataProvider() )
  {
    if ( messageBar )
      messageBar->pushMessage( tr( "Paste features" ), tr( "Cannot create new layer." ), Qgis::MessageLevel::Warning );
    return nullptr;
  }

  layer->startEditing();
  for ( const QgsField &f : fields )
  {
    QgsDebugMsgLevel( u"field %1 (%2)"_s.arg( f.name(), QVariant::typeToName( f.type() ) ), 2 );
    if ( !layer->addAttribute( f ) )
    {
      if ( messageBar )
        messageBar->pushMessage( tr( "Paste features" ), tr( "Cannot create field %1 (%2,%3), falling back to string type" ).arg( f.name(), f.typeName(), QVariant::typeToName( f.type() ) ), Qgis::MessageLevel::Warning );

      // Fallback to string
      QgsField strField { f };
      strField.setType( QMetaType::Type::QString );
      if ( !layer->addAttribute( strField ) )
      {
        if ( messageBar )
          messageBar->pushMessage( tr( "Paste features" ), tr( "Cannot create field %1 (%2,%3)" ).arg( strField.name(), strField.typeName(), QVariant::typeToName( strField.type() ) ), Qgis::MessageLevel::Critical );
        return nullptr;
      }
    }
  }

  QgsFeatureList convertedFeatures { QgsVectorLayerUtils::makeFeaturesCompatible( features, layer.get() ) };

  // Convert attributes
  for ( auto it = convertedFeatures.begin(); it != convertedFeatures.end(); ++it )
  {
    for ( int idx = 0; idx < layer->fields().count() && idx < it->attributeCount(); ++idx )
    {
      QVariant attr = it->attribute( idx );
      //if convertCompatible fails, it will replace attr with an appropriate null QVariant
      //so we're calling setAttribute regardless, covering cases like 'Autogenerate' or 'nextVal()' on int fields.
      layer->fields().at( idx ).convertCompatible( attr );
      it->setAttribute( idx, attr );
    }
  }

  if ( !layer->addFeatures( convertedFeatures ) || !layer->commitChanges() )
  {
    QgsDebugError( u"Cannot add features or commit changes"_s );
    return nullptr;
  }

  QgsDebugMsgLevel( u"%1 features pasted to temporary scratch layer"_s.arg( layer->featureCount() ), 2 );
  return layer;
}

void QgsClipboard::systemClipboardChanged()
{
  if ( QApplication::clipboard()->ownsClipboard() )
  {
    // on wayland the system clipboard will always emit dataChanged on an application defocus/refocus cycle,
    // so we'll ignore this if we know we set the clipboard originally ourselves
    return;
  }

  emit changed();
}

const QgsFeatureListMimeData *QgsClipboard::featureListDataFromClipboard() const
{
  QClipboard *cb = QApplication::clipboard();
  const QMimeData *data = cb->mimeData( QClipboard::Clipboard );

  return qobject_cast< const QgsFeatureListMimeData * >( data );
}

//
// QgsFeatureListMimeData
//

QgsFeatureListMimeData::QgsFeatureListMimeData( QgsMapLayer *sourceLayer, const QgsCoordinateReferenceSystem &crs, const QgsFields &fields, const QgsFeatureList &features )
  : mFeatures( features )
  , mFields( fields )
  , mCRS( crs )
  , mSourceLayer( sourceLayer )
{}

QStringList QgsFeatureListMimeData::formats() const
{
  return { u"text/plain"_s, u"text/plain;charset=utf-8"_s, u"text/html"_s, QgsClipboard::FEATURES_MIME_TYPE };
}

QVariant QgsFeatureListMimeData::retrieveData( const QString &mimeType, QMetaType type ) const
{
  if ( mimeType == "text/plain"_L1 || mimeType == "text/plain;charset=utf-8"_L1 )
  {
    return toText( false );
  }
  else if ( mimeType == "text/html"_L1 )
  {
    return toText( true );
  }
  else if ( mimeType == QgsClipboard::FEATURES_MIME_TYPE )
  {
    QByteArray data;
    QDataStream stream( &data, QIODevice::WriteOnly );
    stream << mCRS.toWkt() << mFields << mFeatures;
    return data;
  }
  return QMimeData::retrieveData( mimeType, type );
}

QString QgsFeatureListMimeData::toText( bool asHtml ) const
{
  QgsClipboard::CopyFormat format = QgsSettings().enumValue( u"qgis/copyFeatureFormat"_s, QgsClipboard::AttributesWithWKT );

  if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( mSourceLayer.data() ) )
  {
    if ( vectorLayer->geometryType() == Qgis::GeometryType::Null )
    {
      format = QgsClipboard::AttributesOnly;
    }
  }

  QString content;
  switch ( format )
  {
    case QgsClipboard::AttributesOnly:
    case QgsClipboard::AttributesWithWKT:
    case QgsClipboard::AttributesWithWKB:
    {
      QStringList lines;
      QStringList fields;

      // first do the field names
      if ( ( format == QgsClipboard::AttributesWithWKB ) || ( format == QgsClipboard::AttributesWithWKT ) )
      {
        const QLatin1String geometryHeading = format == QgsClipboard::AttributesWithWKB ? "wkb_geom"_L1 : "wkt_geom"_L1;
        // only include the "wkx_geom" field IF we have other fields -- otherwise it's redundant and we should just set the clipboard to WKT/WKB text directly
        if ( !asHtml )
        {
          if ( !mFields.isEmpty() )
          {
            fields += geometryHeading;
          }
        }
        else
        {
          fields += "<td>"_L1 + geometryHeading + "</td>"_L1;
        }
      }

      fields.reserve( mFields.size() );
      for ( const QgsField &field : mFields )
      {
        if ( !asHtml )
        {
          fields += field.name();
        }
        else
        {
          fields += u"<td>%1</td>"_s.arg( field.name() );
        }
      }
      if ( !asHtml )
      {
        if ( !fields.empty() )
          lines += fields.join( QLatin1Char( '\t' ) );
      }
      else
      {
        lines += fields.join( QString() );
      }
      fields.clear();

      // then the field contents
      for ( QgsFeatureList::const_iterator it = mFeatures.constBegin(); it != mFeatures.constEnd(); ++it )
      {
        const QgsAttributes attributes = it->attributes();

        // TODO: Set up Paste Transformations to specify the order in which fields are added.
        if ( ( format == QgsClipboard::AttributesWithWKB ) || ( format == QgsClipboard::AttributesWithWKT ) )
        {
          if ( it->hasGeometry() )
          {
            if ( format == QgsClipboard::AttributesWithWKT )
            {
              const QString wkt = it->geometry().asWkt();
              if ( !asHtml )
              {
                fields += wkt;
              }
              else
              {
                fields += u"<td>%1</td>"_s.arg( wkt );
              }
            }
            else if ( format == QgsClipboard::AttributesWithWKB )
            {
              const QString wkb = it->geometry().asWkb().toHex();
              if ( !asHtml )
              {
                fields += wkb;
              }
              else
              {
                fields += u"<td>%1</td>"_s.arg( wkb );
              }
            }
          }
          else
          {
            if ( !asHtml )
            {
              fields += QgsApplication::nullRepresentation();
            }
            else
            {
              fields += u"<td>%1</td>"_s.arg( QgsApplication::nullRepresentation() );
            }
          }
        }

        for ( int idx = 0; idx < attributes.count(); ++idx )
        {
          QString value;
          QVariant variant = attributes.at( idx );
          const bool useJSONFromVariant = variant.userType() == QMetaType::Type::QStringList || variant.userType() == QMetaType::Type::QVariantList || variant.userType() == QMetaType::Type::QVariantMap;

          if ( useJSONFromVariant )
          {
            value = QString::fromStdString( QgsJsonUtils::jsonFromVariant( variant ).dump() );
          }
          else
          {
            if ( QgsVariantUtils::isNull( variant ) && variant.isValid() )
              value = "";
            else
              value = variant.toString();
          }

          if ( !asHtml )
          {
            if ( value.contains( '\n' ) || value.contains( '\t' ) )
              fields += '"' + value.replace( '"', "\"\""_L1 ) + '\"';
            else
            {
              fields += value;
            }
          }
          else
          {
            if ( useJSONFromVariant )
            {
              value = QString::fromStdString( QgsJsonUtils::jsonFromVariant( variant ).dump() );
            }
            else
            {
              if ( QgsVariantUtils::isNull( variant ) && variant.isValid() )
                value = "";
              else
                value = variant.toString();
            }
            value.replace( '\n', "<br>"_L1 ).replace( '\t', "&emsp;"_L1 );
            fields += u"<td>%1</td>"_s.arg( value );
          }
        }

        if ( !asHtml )
        {
          lines += fields.join( QLatin1Char( '\t' ) );
        }
        else
        {
          lines += fields.join( QString() );
        }
        fields.clear();
      }

      if ( !asHtml )
      {
        content = lines.join( '\n' );
      }
      else
      {
        content = u"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr>"_s
                  + lines.join( "</tr><tr>"_L1 )
                  + u"</tr></table></body></html>"_s;
      }
      break;
    }
    case QgsClipboard::GeoJSON:
    {
      QgsJsonExporter exporter;
      exporter.setSourceCrs( mCRS );

      content = exporter.exportFeatures( mFeatures );
    }
  }
  return content;
}
