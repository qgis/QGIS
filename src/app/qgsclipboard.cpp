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

#include <fstream>

#include <QApplication>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QClipboard>
#include <QMimeData>
#include <QTextCodec>

#include "qgsclipboard.h"
#include "moc_qgsclipboard.cpp"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsogrutils.h"
#include "qgsjsonutils.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgsvectortilelayer.h"
#include "qgsvectorlayerutils.h"
#include "qgsmemoryproviderutils.h"
#include "qgsmessagebar.h"

#include <nlohmann/json.hpp>

QgsClipboard::QgsClipboard()
{
  connect( QApplication::clipboard(), &QClipboard::dataChanged, this, &QgsClipboard::systemClipboardChanged );
}

void QgsClipboard::replaceWithCopyOf( QgsVectorLayer *src )
{
  if ( !src )
    return;

  // Replace the QGIS clipboard.
  mFeatureFields = src->fields();
  mFeatureClipboard = src->selectedFeatures();
  mCRS = src->crs();
  mFeatureLayer = src;
  QgsDebugMsgLevel( QStringLiteral( "replaced QGIS clipboard." ), 2 );

  setSystemClipboard();
  mUseSystemClipboard = false;
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

  mFeatureFields = supersetFields;

  mFeatureClipboard.clear();
  for ( const QgsFeature &feature : selectedFeatures )
  {
    QgsFeature superSetFeature = feature;
    QgsVectorLayerUtils::matchAttributesToFields( superSetFeature, mFeatureFields );
    mFeatureClipboard.append( superSetFeature );
  }

  mCRS = src->crs();
  mFeatureLayer = src;
  QgsDebugMsgLevel( QStringLiteral( "replaced QGIS clipboard." ), 2 );

  setSystemClipboard();
  mUseSystemClipboard = false;
  emit changed();
}

void QgsClipboard::replaceWithCopyOf( QgsFeatureStore &featureStore, QgsVectorLayer *src )
{
  QgsDebugMsgLevel( QStringLiteral( "features count = %1" ).arg( featureStore.features().size() ), 2 );
  mFeatureFields = featureStore.fields();
  mFeatureClipboard = featureStore.features();
  mCRS = featureStore.crs();
  mFeatureLayer = src;
  setSystemClipboard();
  mUseSystemClipboard = false;
  emit changed();
}

void QgsClipboard::generateClipboardText( QString &textContent, QString &htmlContent ) const
{
  CopyFormat format = QgsSettings().enumValue( QStringLiteral( "qgis/copyFeatureFormat" ), AttributesWithWKT );

  textContent.clear();
  htmlContent.clear();

  if ( QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( mFeatureLayer.data() ) )
  {
    if ( vectorLayer->geometryType() == Qgis::GeometryType::Null )
    {
      format = AttributesOnly;
    }
  }

  switch ( format )
  {
    case AttributesOnly:
    case AttributesWithWKT:
    case AttributesWithWKB:
    {
      QStringList textLines, htmlLines;
      QStringList textFields, htmlFields;

      // first do the field names
      if ( ( format == AttributesWithWKB ) || ( format == AttributesWithWKT ) )
      {
        const QLatin1String geometryHeading = format == AttributesWithWKB ? QLatin1String( "wkb_geom" ) : QLatin1String( "wkt_geom" );
        // only include the "wkx_geom" field IF we have other fields -- otherwise it's redundant and we should just set the clipboard to WKT/WKB text directly
        if ( !mFeatureFields.isEmpty() )
        {
          textFields += geometryHeading;
        }

        htmlFields += QLatin1String( "<td>" ) + geometryHeading + QLatin1String( "</td>" );
      }

      textFields.reserve( mFeatureFields.size() );
      htmlFields.reserve( mFeatureFields.size() );
      for ( const QgsField &field : mFeatureFields )
      {
        textFields += field.name();
        htmlFields += QStringLiteral( "<td>%1</td>" ).arg( field.name() );
      }
      if ( !textFields.empty() )
        textLines += textFields.join( QLatin1Char( '\t' ) );
      htmlLines += htmlFields.join( QString() );
      textFields.clear();
      htmlFields.clear();

      // then the field contents
      for ( QgsFeatureList::const_iterator it = mFeatureClipboard.constBegin(); it != mFeatureClipboard.constEnd(); ++it )
      {
        const QgsAttributes attributes = it->attributes();

        // TODO: Set up Paste Transformations to specify the order in which fields are added.
        if ( ( format == AttributesWithWKB ) || ( format == AttributesWithWKT ) )
        {
          if ( it->hasGeometry() )
          {
            if ( format == AttributesWithWKT )
            {
              const QString wkt = it->geometry().asWkt();
              textFields += wkt;
              htmlFields += QStringLiteral( "<td>%1</td>" ).arg( wkt );
            }
            else if ( format == AttributesWithWKB )
            {
              const QString wkb = it->geometry().asWkb().toHex();
              textFields += wkb;
              htmlFields += QStringLiteral( "<td>%1</td>" ).arg( wkb );
            }
          }
          else
          {
            textFields += QgsApplication::nullRepresentation();
            htmlFields += QStringLiteral( "<td>%1</td>" ).arg( QgsApplication::nullRepresentation() );
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

          if ( value.contains( '\n' ) || value.contains( '\t' ) )
            textFields += '"' + value.replace( '"', QLatin1String( "\"\"" ) ) + '\"';
          else
          {
            textFields += value;
          }
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
          value.replace( '\n', QLatin1String( "<br>" ) ).replace( '\t', QLatin1String( "&emsp;" ) );
          htmlFields += QStringLiteral( "<td>%1</td>" ).arg( value );
        }

        textLines += textFields.join( QLatin1Char( '\t' ) );
        htmlLines += htmlFields.join( QString() );
        textFields.clear();
        htmlFields.clear();
      }

      textContent = textLines.join( '\n' );
      htmlContent = QStringLiteral( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr>" ) + htmlLines.join( QLatin1String( "</tr><tr>" ) ) + QStringLiteral( "</tr></table></body></html>" );
      break;
    }
    case GeoJSON:
    {
      QgsJsonExporter exporter;
      exporter.setSourceCrs( mCRS );

      textContent = exporter.exportFeatures( mFeatureClipboard );
    }
  }
}

void QgsClipboard::setSystemClipboard()
{
  // avoid overwriting internal clipboard - note that on Windows, the call to QClipboard::setText
  // below doesn't immediately trigger QClipboard::dataChanged, and accordingly the call to
  // systemClipboardChanged() is delayed. So by setting mIgnoreNextSystemClipboardChange we indicate
  // that just the next call to systemClipboardChanged() should be ignored
  mIgnoreNextSystemClipboardChange = true;

  QClipboard *cb = QApplication::clipboard();

  // Copy text into the clipboard
  QString textCopy, htmlCopy;
  generateClipboardText( textCopy, htmlCopy );
  QMimeData *m = new QMimeData();
  m->setText( textCopy );

  if ( mFeatureClipboard.count() < 1000 && !htmlCopy.isEmpty() )
  {
    m->setHtml( htmlCopy );
  }

  // With qgis running under Linux, but with a Windows based X
  // server (Xwin32), ::Selection was necessary to get the data into
  // the Windows clipboard (which seems contrary to the Qt
  // docs). With a Linux X server, ::Clipboard was required.
  // The simple solution was to put the text into both clipboards.
#ifdef Q_OS_LINUX
  cb->setMimeData( m, QClipboard::Selection );
#endif
  cb->setMimeData( m, QClipboard::Clipboard );

  QgsDebugMsgLevel( QStringLiteral( "replaced system clipboard with: %1." ).arg( textCopy ), 4 );
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
  bool isFirstLine { string.startsWith( QLatin1String( "wkt_geom" ) ) };
  // it seems there is no other way to check for header
  const bool hasHeader { string.startsWith( QLatin1String( "wkt_geom" ) ) };
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
      if ( attrVal != QLatin1String( "wkt_geom" ) ) // ignore this one
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

#ifdef Q_OS_LINUX
  const QString string = cb->text( QClipboard::Selection );
#else
  QString string = cb->text( QClipboard::Clipboard );
#endif

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
      if ( fieldNames.at( 0 ) != QLatin1String( "wkt_geom" ) )
      {
        return f;
      }

      for ( int i = 0; i < fieldNames.size(); ++i )
      {
        const QString fieldName = fieldNames.at( i );
        if ( fieldName == QLatin1String( "wkt_geom" ) )
        {
          continue;
        }

        f.append( QgsField( fieldName, QMetaType::Type::QString ) );
      }
    }
  }
  return f;
}

QgsFeatureList QgsClipboard::copyOf( const QgsFields &fields ) const
{
  QgsDebugMsgLevel( QStringLiteral( "returning clipboard." ), 2 );
  if ( !mUseSystemClipboard )
    return mFeatureClipboard;

  QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
  QString text = cb->text( QClipboard::Selection );
#else
  QString text = cb->text( QClipboard::Clipboard );
#endif

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

void QgsClipboard::clear()
{
  mFeatureClipboard.clear();

  QgsDebugMsgLevel( QStringLiteral( "cleared clipboard." ), 2 );
  emit changed();
}

void QgsClipboard::insert( const QgsFeature &feature )
{
  mFeatureClipboard.push_back( feature );

  QgsDebugMsgLevel( "inserted " + feature.geometry().asWkt(), 4 );
  mUseSystemClipboard = false;
  emit changed();
}

bool QgsClipboard::isEmpty() const
{
  QClipboard *cb = QApplication::clipboard();
#ifdef Q_OS_LINUX
  const QString text = cb->text( QClipboard::Selection );
#else
  QString text = cb->text( QClipboard::Clipboard );
#endif
  return text.isEmpty() && mFeatureClipboard.empty();
}

QgsFeatureList QgsClipboard::transformedCopyOf( const QgsCoordinateReferenceSystem &destCRS, const QgsFields &fields ) const
{
  QgsFeatureList featureList = copyOf( fields );

  QgisApp::instance()->askUserForDatumTransform( crs(), destCRS );
  const QgsCoordinateTransform ct = QgsCoordinateTransform( crs(), destCRS, QgsProject::instance() );

  QgsDebugMsgLevel( QStringLiteral( "transforming clipboard." ), 2 );
  for ( QgsFeatureList::iterator iter = featureList.begin(); iter != featureList.end(); ++iter )
  {
    QgsGeometry g = iter->geometry();
    g.transform( ct );
    iter->setGeometry( g );
  }

  return featureList;
}

QgsCoordinateReferenceSystem QgsClipboard::crs() const
{
  return mCRS;
}

void QgsClipboard::setData( const QString &mimeType, const QByteArray &data, const QString &text )
{
  mUseSystemClipboard = true;
  QMimeData *mdata = new QMimeData();
  mdata->setData( mimeType, data );
  if ( !text.isEmpty() )
  {
    mdata->setText( text );
  }
  // Transfers ownership to the clipboard object
#ifdef Q_OS_LINUX
  QApplication::clipboard()->setMimeData( mdata, QClipboard::Selection );
#endif
  QApplication::clipboard()->setMimeData( mdata, QClipboard::Clipboard );
}

void QgsClipboard::setText( const QString &text )
{
#ifdef Q_OS_LINUX
  QApplication::clipboard()->setText( text, QClipboard::Selection );
#endif
  QApplication::clipboard()->setText( text, QClipboard::Clipboard );
}

bool QgsClipboard::hasFormat( const QString &mimeType ) const
{
  return QApplication::clipboard()->mimeData()->hasFormat( mimeType );
}

QByteArray QgsClipboard::data( const QString &mimeType ) const
{
  return QApplication::clipboard()->mimeData()->data( mimeType );
}

QgsFields QgsClipboard::fields() const
{
  if ( !mUseSystemClipboard )
    return mFeatureFields;
  else
    return retrieveFields();
}

QgsMapLayer *QgsClipboard::layer() const
{
  if ( !mUseSystemClipboard )
    return mFeatureLayer.data();
  else
    return nullptr;
}

std::unique_ptr<QgsVectorLayer> QgsClipboard::pasteToNewMemoryVector( QgsMessageBar *messageBar )
{
  const QgsFields fields = QgsClipboard::fields();

  // Decide geometry type from features, switch to multi type if at least one multi is found
  QMap<Qgis::WkbType, int> typeCounts;
  const QgsFeatureList features = copyOf( fields );
  for ( const QgsFeature &feature : features )
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
    QString typeName = wkbType != Qgis::WkbType::NoGeometry ? QgsWkbTypes::displayString( wkbType ) : QStringLiteral( "none" );
    if ( messageBar )
      messageBar->pushMessage( tr( "Paste features" ), tr( "Multiple geometry types found, features with geometry different from %1 will be created without geometry." ).arg( typeName ), Qgis::MessageLevel::Info );
  }

  std::unique_ptr<QgsVectorLayer> layer( QgsMemoryProviderUtils::createMemoryLayer( QStringLiteral( "pasted_features" ), QgsFields(), wkbType, crs() ) );

  if ( !layer->isValid() || !layer->dataProvider() )
  {
    if ( messageBar )
      messageBar->pushMessage( tr( "Paste features" ), tr( "Cannot create new layer." ), Qgis::MessageLevel::Warning );
    return nullptr;
  }

  layer->startEditing();
  for ( const QgsField &f : QgsClipboard::fields() )
  {
    QgsDebugMsgLevel( QStringLiteral( "field %1 (%2)" ).arg( f.name(), QVariant::typeToName( f.type() ) ), 2 );
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
    QgsDebugError( QStringLiteral( "Cannot add features or commit changes" ) );
    return nullptr;
  }

  QgsDebugMsgLevel( QStringLiteral( "%1 features pasted to temporary scratch layer" ).arg( layer->featureCount() ), 2 );
  return layer;
}

void QgsClipboard::systemClipboardChanged()
{
  if ( mIgnoreNextSystemClipboardChange )
  {
    mIgnoreNextSystemClipboardChange = false;
    return;
  }

  mUseSystemClipboard = true;
  emit changed();
}
