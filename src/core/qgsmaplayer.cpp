/***************************************************************************
                          qgsmaplayer.cpp  -  description
                             -------------------
    begin                : Fri Jun 28 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomImplementation>
#include <QDomNode>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QUrl>

#include <sqlite3.h>

#include "qgssqliteutils.h"

#include "qgssqliteutils.h"
#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgslogger.h"
#include "qgsauthmanager.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmeshlayer.h"
#include "qgspathresolver.h"
#include "qgsprojectfiletransform.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrectangle.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsxmlutils.h"
#include "qgsstringutils.h"

QString QgsMapLayer::extensionPropertyType( QgsMapLayer::PropertyType type )
{
  switch ( type )
  {
    case Metadata:
      return QStringLiteral( ".qmd" );

    case Style:
      return QStringLiteral( ".qml" );
  }
  return QString();
}

QgsMapLayer::QgsMapLayer( QgsMapLayer::LayerType type,
                          const QString &lyrname,
                          const QString &source )
  : mDataSource( source )
  , mLayerName( lyrname )
  , mLayerType( type )
  , mUndoStack( new QUndoStack( this ) )
  , mUndoStackStyles( new QUndoStack( this ) )
  , mStyleManager( new QgsMapLayerStyleManager( this ) )
  , mRefreshTimer( new QTimer( this ) )
{
  //mShortName.replace( QRegExp( "[\\W]" ), "_" );

  // Generate the unique ID of this layer
  QString uuid = QUuid::createUuid().toString();
  // trim { } from uuid
  mID = lyrname + '_' + uuid.mid( 1, uuid.length() - 2 );

  // Tidy the ID up to avoid characters that may cause problems
  // elsewhere (e.g in some parts of XML). Replaces every non-word
  // character (word characters are the alphabet, numbers and
  // underscore) with an underscore.
  // Note that the first backslashe in the regular expression is
  // there for the compiler, so the pattern is actually \W
  mID.replace( QRegExp( "[\\W]" ), QStringLiteral( "_" ) );

  connect( this, &QgsMapLayer::crsChanged, this, &QgsMapLayer::configChanged );
  connect( this, &QgsMapLayer::nameChanged, this, &QgsMapLayer::configChanged );

  connect( mRefreshTimer, &QTimer::timeout, this, [ = ] { triggerRepaint( true ); } );
}

QgsMapLayer::~QgsMapLayer()
{
  delete m3DRenderer;
  delete mLegend;
  delete mStyleManager;
}

void QgsMapLayer::clone( QgsMapLayer *layer ) const
{
  layer->setBlendMode( blendMode() );

  Q_FOREACH ( const QString &s, styleManager()->styles() )
  {
    layer->styleManager()->addStyle( s, styleManager()->style( s ) );
  }

  layer->setName( name() );
  layer->setShortName( shortName() );
  layer->setExtent( extent() );
  layer->setMaximumScale( maximumScale() );
  layer->setMinimumScale( minimumScale() );
  layer->setScaleBasedVisibility( hasScaleBasedVisibility() );
  layer->setTitle( title() );
  layer->setAbstract( abstract() );
  layer->setKeywordList( keywordList() );
  layer->setDataUrl( dataUrl() );
  layer->setDataUrlFormat( dataUrlFormat() );
  layer->setAttribution( attribution() );
  layer->setAttributionUrl( attributionUrl() );
  layer->setMetadataUrl( metadataUrl() );
  layer->setMetadataUrlType( metadataUrlType() );
  layer->setMetadataUrlFormat( metadataUrlFormat() );
  layer->setLegendUrl( legendUrl() );
  layer->setLegendUrlFormat( legendUrlFormat() );
  layer->setDependencies( dependencies() );
  layer->setCrs( crs() );
  layer->setCustomProperties( mCustomProperties );
}

QgsMapLayer::LayerType QgsMapLayer::type() const
{
  return mLayerType;
}

QgsMapLayer::LayerFlags QgsMapLayer::flags() const
{
  return mFlags;
}

void QgsMapLayer::setFlags( QgsMapLayer::LayerFlags flags )
{
  if ( flags == mFlags )
    return;

  mFlags = flags;
  emit flagsChanged();
}

QString QgsMapLayer::id() const
{
  return mID;
}

void QgsMapLayer::setName( const QString &name )
{
  if ( name == mLayerName )
    return;

  mLayerName = name;

  emit nameChanged();
}

QString QgsMapLayer::name() const
{
  QgsDebugMsgLevel( "returning name '" + mLayerName + '\'', 4 );
  return mLayerName;
}

QgsDataProvider *QgsMapLayer::dataProvider()
{
  return nullptr;
}

const QgsDataProvider *QgsMapLayer::dataProvider() const
{
  return nullptr;
}

QString QgsMapLayer::publicSource() const
{
  // Redo this every time we're asked for it, as we don't know if
  // dataSource has changed.
  QString safeName = QgsDataSourceUri::removePassword( mDataSource );
  return safeName;
}

QString QgsMapLayer::source() const
{
  return mDataSource;
}

QgsRectangle QgsMapLayer::extent() const
{
  return mExtent;
}

void QgsMapLayer::setBlendMode( const QPainter::CompositionMode blendMode )
{
  if ( mBlendMode == blendMode )
    return;

  mBlendMode = blendMode;
  emit blendModeChanged( blendMode );
  emit styleChanged();
}

QPainter::CompositionMode QgsMapLayer::blendMode() const
{
  return mBlendMode;
}


bool QgsMapLayer::readLayerXml( const QDomElement &layerElement, QgsReadWriteContext &context )
{
  bool layerError;

  QDomNode mnl;
  QDomElement mne;

  // read provider
  QString provider;
  mnl = layerElement.namedItem( QStringLiteral( "provider" ) );
  mne = mnl.toElement();
  provider = mne.text();

  // set data source
  mnl = layerElement.namedItem( QStringLiteral( "datasource" ) );
  mne = mnl.toElement();
  mDataSource = mne.text();

  // if the layer needs authentication, ensure the master password is set
  QRegExp rx( "authcfg=([a-z]|[A-Z]|[0-9]){7}" );
  if ( ( rx.indexIn( mDataSource ) != -1 )
       && !QgsApplication::authManager()->setMasterPassword( true ) )
  {
    return false;
  }

  mDataSource = decodedSource( mDataSource, provider, context );

  // Set the CRS from project file, asking the user if necessary.
  // Make it the saved CRS to have WMS layer projected correctly.
  // We will still overwrite whatever GDAL etc picks up anyway
  // further down this function.
  mnl = layerElement.namedItem( QStringLiteral( "layername" ) );
  mne = mnl.toElement();

  QgsCoordinateReferenceSystem savedCRS;
  CUSTOM_CRS_VALIDATION savedValidation;

  QDomNode srsNode = layerElement.namedItem( QStringLiteral( "srs" ) );
  mCRS.readXml( srsNode );
  mCRS.setValidationHint( tr( "Specify CRS for layer %1" ).arg( mne.text() ) );
  if ( isSpatial() )
    mCRS.validate();
  savedCRS = mCRS;

  // Do not validate any projections in children, they will be overwritten anyway.
  // No need to ask the user for a projections when it is overwritten, is there?
  savedValidation = QgsCoordinateReferenceSystem::customCrsValidation();
  QgsCoordinateReferenceSystem::setCustomCrsValidation( nullptr );

  QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Layer" ), mne.text() );

  // now let the children grab what they need from the Dom node.
  layerError = !readXml( layerElement, context );

  // overwrite CRS with what we read from project file before the raster/vector
  // file reading functions changed it. They will if projections is specified in the file.
  // FIXME: is this necessary?
  QgsCoordinateReferenceSystem::setCustomCrsValidation( savedValidation );
  mCRS = savedCRS;

  // the internal name is just the data source basename
  //QFileInfo dataSourceFileInfo( mDataSource );
  //internalName = dataSourceFileInfo.baseName();

  // set ID
  mnl = layerElement.namedItem( QStringLiteral( "id" ) );
  if ( ! mnl.isNull() )
  {
    mne = mnl.toElement();
    if ( ! mne.isNull() && mne.text().length() > 10 ) // should be at least 17 (yyyyMMddhhmmsszzz)
    {
      mID = mne.text();
    }
  }

  setAutoRefreshInterval( layerElement.attribute( QStringLiteral( "autoRefreshTime" ), QStringLiteral( "0" ) ).toInt() );
  setAutoRefreshEnabled( layerElement.attribute( QStringLiteral( "autoRefreshEnabled" ), QStringLiteral( "0" ) ).toInt() );
  setRefreshOnNofifyMessage( layerElement.attribute( QStringLiteral( "refreshOnNotifyMessage" ), QString() ) );
  setRefreshOnNotifyEnabled( layerElement.attribute( QStringLiteral( "refreshOnNotifyEnabled" ), QStringLiteral( "0" ) ).toInt() );


  // set name
  mnl = layerElement.namedItem( QStringLiteral( "layername" ) );
  mne = mnl.toElement();

  //name can be translated
  setName( context.projectTranslator()->translate( QStringLiteral( "project:layers:%1" ).arg( layerElement.namedItem( QStringLiteral( "id" ) ).toElement().text() ), mne.text() ) );

  //short name
  QDomElement shortNameElem = layerElement.firstChildElement( QStringLiteral( "shortname" ) );
  if ( !shortNameElem.isNull() )
  {
    mShortName = shortNameElem.text();
  }

  //title
  QDomElement titleElem = layerElement.firstChildElement( QStringLiteral( "title" ) );
  if ( !titleElem.isNull() )
  {
    mTitle = titleElem.text();
  }

  //abstract
  QDomElement abstractElem = layerElement.firstChildElement( QStringLiteral( "abstract" ) );
  if ( !abstractElem.isNull() )
  {
    mAbstract = abstractElem.text();
  }

  //keywordList
  QDomElement keywordListElem = layerElement.firstChildElement( QStringLiteral( "keywordList" ) );
  if ( !keywordListElem.isNull() )
  {
    QStringList kwdList;
    for ( QDomNode n = keywordListElem.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
      kwdList << n.toElement().text();
    }
    mKeywordList = kwdList.join( QStringLiteral( ", " ) );
  }

  //metadataUrl
  QDomElement dataUrlElem = layerElement.firstChildElement( QStringLiteral( "dataUrl" ) );
  if ( !dataUrlElem.isNull() )
  {
    mDataUrl = dataUrlElem.text();
    mDataUrlFormat = dataUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }

  //legendUrl
  QDomElement legendUrlElem = layerElement.firstChildElement( QStringLiteral( "legendUrl" ) );
  if ( !legendUrlElem.isNull() )
  {
    mLegendUrl = legendUrlElem.text();
    mLegendUrlFormat = legendUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }

  //attribution
  QDomElement attribElem = layerElement.firstChildElement( QStringLiteral( "attribution" ) );
  if ( !attribElem.isNull() )
  {
    mAttribution = attribElem.text();
    mAttributionUrl = attribElem.attribute( QStringLiteral( "href" ), QString() );
  }

  //metadataUrl
  QDomElement metaUrlElem = layerElement.firstChildElement( QStringLiteral( "metadataUrl" ) );
  if ( !metaUrlElem.isNull() )
  {
    mMetadataUrl = metaUrlElem.text();
    mMetadataUrlType = metaUrlElem.attribute( QStringLiteral( "type" ), QString() );
    mMetadataUrlFormat = metaUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }

  // mMetadata.readFromLayer( this );
  QDomElement metadataElem = layerElement.firstChildElement( QStringLiteral( "resourceMetadata" ) );
  mMetadata.readMetadataXml( metadataElem );

  return ! layerError;
} // bool QgsMapLayer::readLayerXML


bool QgsMapLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  Q_UNUSED( layer_node );
  Q_UNUSED( context );
  // NOP by default; children will over-ride with behavior specific to them

  return true;
} // void QgsMapLayer::readXml


bool QgsMapLayer::writeLayerXml( QDomElement &layerElement, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  if ( !extent().isNull() )
  {
    layerElement.appendChild( QgsXmlUtils::writeRectangle( mExtent, document ) );
  }

  layerElement.setAttribute( QStringLiteral( "autoRefreshTime" ), QString::number( mRefreshTimer->interval() ) );
  layerElement.setAttribute( QStringLiteral( "autoRefreshEnabled" ), mRefreshTimer->isActive() ? 1 : 0 );
  layerElement.setAttribute( QStringLiteral( "refreshOnNotifyEnabled" ),  mIsRefreshOnNofifyEnabled ? 1 : 0 );
  layerElement.setAttribute( QStringLiteral( "refreshOnNotifyMessage" ),  mRefreshOnNofifyMessage );


  // ID
  QDomElement layerId = document.createElement( QStringLiteral( "id" ) );
  QDomText layerIdText = document.createTextNode( id() );
  layerId.appendChild( layerIdText );

  layerElement.appendChild( layerId );

  // data source
  QDomElement dataSource = document.createElement( QStringLiteral( "datasource" ) );
  QString src = encodedSource( source(), context );
  QDomText dataSourceText = document.createTextNode( src );
  dataSource.appendChild( dataSourceText );
  layerElement.appendChild( dataSource );

  // layer name
  QDomElement layerName = document.createElement( QStringLiteral( "layername" ) );
  QDomText layerNameText = document.createTextNode( name() );
  layerName.appendChild( layerNameText );
  layerElement.appendChild( layerName );

  // layer short name
  if ( !mShortName.isEmpty() )
  {
    QDomElement layerShortName = document.createElement( QStringLiteral( "shortname" ) );
    QDomText layerShortNameText = document.createTextNode( mShortName );
    layerShortName.appendChild( layerShortNameText );
    layerElement.appendChild( layerShortName );
  }

  // layer title
  if ( !mTitle.isEmpty() )
  {
    QDomElement layerTitle = document.createElement( QStringLiteral( "title" ) );
    QDomText layerTitleText = document.createTextNode( mTitle );
    layerTitle.appendChild( layerTitleText );
    layerElement.appendChild( layerTitle );
  }

  // layer abstract
  if ( !mAbstract.isEmpty() )
  {
    QDomElement layerAbstract = document.createElement( QStringLiteral( "abstract" ) );
    QDomText layerAbstractText = document.createTextNode( mAbstract );
    layerAbstract.appendChild( layerAbstractText );
    layerElement.appendChild( layerAbstract );
  }

  // layer keyword list
  QStringList keywordStringList = keywordList().split( ',' );
  if ( !keywordStringList.isEmpty() )
  {
    QDomElement layerKeywordList = document.createElement( QStringLiteral( "keywordList" ) );
    for ( int i = 0; i < keywordStringList.size(); ++i )
    {
      QDomElement layerKeywordValue = document.createElement( QStringLiteral( "value" ) );
      QDomText layerKeywordText = document.createTextNode( keywordStringList.at( i ).trimmed() );
      layerKeywordValue.appendChild( layerKeywordText );
      layerKeywordList.appendChild( layerKeywordValue );
    }
    layerElement.appendChild( layerKeywordList );
  }

  // layer metadataUrl
  QString aDataUrl = dataUrl();
  if ( !aDataUrl.isEmpty() )
  {
    QDomElement layerDataUrl = document.createElement( QStringLiteral( "dataUrl" ) );
    QDomText layerDataUrlText = document.createTextNode( aDataUrl );
    layerDataUrl.appendChild( layerDataUrlText );
    layerDataUrl.setAttribute( QStringLiteral( "format" ), dataUrlFormat() );
    layerElement.appendChild( layerDataUrl );
  }

  // layer legendUrl
  QString aLegendUrl = legendUrl();
  if ( !aLegendUrl.isEmpty() )
  {
    QDomElement layerLegendUrl = document.createElement( QStringLiteral( "legendUrl" ) );
    QDomText layerLegendUrlText = document.createTextNode( aLegendUrl );
    layerLegendUrl.appendChild( layerLegendUrlText );
    layerLegendUrl.setAttribute( QStringLiteral( "format" ), legendUrlFormat() );
    layerElement.appendChild( layerLegendUrl );
  }

  // layer attribution
  QString aAttribution = attribution();
  if ( !aAttribution.isEmpty() )
  {
    QDomElement layerAttribution = document.createElement( QStringLiteral( "attribution" ) );
    QDomText layerAttributionText = document.createTextNode( aAttribution );
    layerAttribution.appendChild( layerAttributionText );
    layerAttribution.setAttribute( QStringLiteral( "href" ), attributionUrl() );
    layerElement.appendChild( layerAttribution );
  }

  // layer metadataUrl
  QString aMetadataUrl = metadataUrl();
  if ( !aMetadataUrl.isEmpty() )
  {
    QDomElement layerMetadataUrl = document.createElement( QStringLiteral( "metadataUrl" ) );
    QDomText layerMetadataUrlText = document.createTextNode( aMetadataUrl );
    layerMetadataUrl.appendChild( layerMetadataUrlText );
    layerMetadataUrl.setAttribute( QStringLiteral( "type" ), metadataUrlType() );
    layerMetadataUrl.setAttribute( QStringLiteral( "format" ), metadataUrlFormat() );
    layerElement.appendChild( layerMetadataUrl );
  }

  // timestamp if supported
  if ( timestamp() > QDateTime() )
  {
    QDomElement stamp = document.createElement( QStringLiteral( "timestamp" ) );
    QDomText stampText = document.createTextNode( timestamp().toString( Qt::ISODate ) );
    stamp.appendChild( stampText );
    layerElement.appendChild( stamp );
  }

  layerElement.appendChild( layerName );

  // zorder
  // This is no longer stored in the project file. It is superfluous since the layers
  // are written and read in the proper order.

  // spatial reference system id
  QDomElement mySrsElement = document.createElement( QStringLiteral( "srs" ) );
  mCRS.writeXml( mySrsElement, document );
  layerElement.appendChild( mySrsElement );

  // layer metadata
  QDomElement myMetadataElem = document.createElement( QStringLiteral( "resourceMetadata" ) );
  mMetadata.writeMetadataXml( myMetadataElem, document );
  layerElement.appendChild( myMetadataElem );

  // now append layer node to map layer node
  return writeXml( layerElement, document, context );
}

void QgsMapLayer::writeCommonStyle( QDomElement &layerElement, QDomDocument &document,
                                    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  // save categories
  QMetaEnum metaEnum = QMetaEnum::fromType<QgsMapLayer::StyleCategories>();
  QString categoriesKeys( metaEnum.valueToKeys( static_cast<int>( categories ) ) );
  layerElement.setAttribute( QStringLiteral( "styleCategories" ), categoriesKeys );

  if ( categories.testFlag( Rendering ) )
  {
    // use scale dependent visibility flag
    layerElement.setAttribute( QStringLiteral( "hasScaleBasedVisibilityFlag" ), hasScaleBasedVisibility() ? 1 : 0 );
    layerElement.setAttribute( QStringLiteral( "maxScale" ), QString::number( maximumScale() ) );
    layerElement.setAttribute( QStringLiteral( "minScale" ), QString::number( minimumScale() ) );
  }

  if ( categories.testFlag( Symbology3D ) )
  {
    if ( m3DRenderer )
    {
      QDomElement renderer3DElem = document.createElement( QStringLiteral( "renderer-3d" ) );
      renderer3DElem.setAttribute( QStringLiteral( "type" ), m3DRenderer->type() );
      m3DRenderer->writeXml( renderer3DElem, context );
      layerElement.appendChild( renderer3DElem );
    }
  }

  if ( categories.testFlag( LayerConfiguration ) )
  {
    // flags
    // this code is saving automatically all the flags entries
    QDomElement layerFlagsElem = document.createElement( QStringLiteral( "flags" ) );
    auto enumMap = qgsEnumMap<QgsMapLayer::LayerFlag>();
    for ( auto it = enumMap.constBegin(); it != enumMap.constEnd(); ++it )
    {
      bool flagValue = mFlags.testFlag( it.key() );
      QDomElement flagElem = document.createElement( it.value() );
      flagElem.appendChild( document.createTextNode( QString::number( flagValue ) ) );
      layerFlagsElem.appendChild( flagElem );
    }
    layerElement.appendChild( layerFlagsElem );
  }

  // custom properties
  if ( categories.testFlag( CustomProperties ) )
  {
    writeCustomProperties( layerElement, document );
  }
}


bool QgsMapLayer::writeXml( QDomNode &layer_node, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( layer_node );
  Q_UNUSED( document );
  Q_UNUSED( context );
  // NOP by default; children will over-ride with behavior specific to them

  return true;
}

QString QgsMapLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );
  return source;
}

QString QgsMapLayer::decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context );
  Q_UNUSED( dataProvider );
  return source;
}

void QgsMapLayer::resolveReferences( QgsProject *project )
{
  if ( m3DRenderer )
    m3DRenderer->resolveReferences( *project );
}


void QgsMapLayer::readCustomProperties( const QDomNode &layerNode, const QString &keyStartsWith )
{
  mCustomProperties.readXml( layerNode, keyStartsWith );
}

void QgsMapLayer::writeCustomProperties( QDomNode &layerNode, QDomDocument &doc ) const
{
  mCustomProperties.writeXml( layerNode, doc );
}

void QgsMapLayer::readStyleManager( const QDomNode &layerNode )
{
  QDomElement styleMgrElem = layerNode.firstChildElement( QStringLiteral( "map-layer-style-manager" ) );
  if ( !styleMgrElem.isNull() )
    mStyleManager->readXml( styleMgrElem );
  else
    mStyleManager->reset();
}

void QgsMapLayer::writeStyleManager( QDomNode &layerNode, QDomDocument &doc ) const
{
  if ( mStyleManager )
  {
    QDomElement styleMgrElem = doc.createElement( QStringLiteral( "map-layer-style-manager" ) );
    mStyleManager->writeXml( styleMgrElem );
    layerNode.appendChild( styleMgrElem );
  }
}

bool QgsMapLayer::isValid() const
{
  return mValid;
}

#if 0
void QgsMapLayer::connectNotify( const char *signal )
{
  Q_UNUSED( signal );
  QgsDebugMsgLevel( "QgsMapLayer connected to " + QString( signal ), 3 );
} //  QgsMapLayer::connectNotify
#endif

bool QgsMapLayer::isInScaleRange( double scale ) const
{
  return !mScaleBasedVisibility ||
         ( ( mMinScale == 0 || mMinScale * Qgis::SCALE_PRECISION < scale )
           && ( mMaxScale == 0 || scale < mMaxScale ) );
}

bool QgsMapLayer::hasScaleBasedVisibility() const
{
  return mScaleBasedVisibility;
}

bool QgsMapLayer::hasAutoRefreshEnabled() const
{
  return mRefreshTimer->isActive();
}

int QgsMapLayer::autoRefreshInterval() const
{
  return mRefreshTimer->interval();
}

void QgsMapLayer::setAutoRefreshInterval( int interval )
{
  if ( interval <= 0 )
  {
    mRefreshTimer->stop();
    mRefreshTimer->setInterval( 0 );
  }
  else
  {
    mRefreshTimer->setInterval( interval );
  }
  emit autoRefreshIntervalChanged( mRefreshTimer->isActive() ? mRefreshTimer->interval() : 0 );
}

void QgsMapLayer::setAutoRefreshEnabled( bool enabled )
{
  if ( !enabled )
    mRefreshTimer->stop();
  else if ( mRefreshTimer->interval() > 0 )
    mRefreshTimer->start();

  emit autoRefreshIntervalChanged( mRefreshTimer->isActive() ? mRefreshTimer->interval() : 0 );
}

const QgsLayerMetadata &QgsMapLayer::metadata() const
{
  return mMetadata;
}

void QgsMapLayer::setMaximumScale( double scale )
{
  mMinScale = scale;
}

double QgsMapLayer::maximumScale() const
{
  return mMinScale;
}


void QgsMapLayer::setMinimumScale( double scale )
{
  mMaxScale = scale;
}

void QgsMapLayer::setScaleBasedVisibility( const bool enabled )
{
  mScaleBasedVisibility = enabled;
}

double QgsMapLayer::minimumScale() const
{
  return mMaxScale;
}

QStringList QgsMapLayer::subLayers() const
{
  return QStringList();  // Empty
}

void QgsMapLayer::setLayerOrder( const QStringList &layers )
{
  Q_UNUSED( layers );
  // NOOP
}

void QgsMapLayer::setSubLayerVisibility( const QString &name, bool vis )
{
  Q_UNUSED( name );
  Q_UNUSED( vis );
  // NOOP
}

QgsCoordinateReferenceSystem QgsMapLayer::crs() const
{
  return mCRS;
}

void QgsMapLayer::setCrs( const QgsCoordinateReferenceSystem &srs, bool emitSignal )
{
  mCRS = srs;

  if ( isSpatial() && !mCRS.isValid() )
  {
    mCRS.setValidationHint( tr( "Specify CRS for layer %1" ).arg( name() ) );
    mCRS.validate();
  }

  if ( emitSignal )
    emit crsChanged();
}

QString QgsMapLayer::formatLayerName( const QString &name )
{
  QString layerName( name );
  layerName.replace( '_', ' ' );
  layerName = QgsStringUtils::capitalize( layerName, QgsStringUtils::ForceFirstLetterToCapital );
  return layerName;
}

QString QgsMapLayer::baseURI( PropertyType type ) const
{
  QString myURI = publicSource();

  // if file is using the VSIFILE mechanism, remove the prefix
  if ( myURI.startsWith( QLatin1String( "/vsigzip/" ), Qt::CaseInsensitive ) )
  {
    myURI.remove( 0, 9 );
  }
  else if ( myURI.startsWith( QLatin1String( "/vsizip/" ), Qt::CaseInsensitive ) &&
            myURI.endsWith( QLatin1String( ".zip" ), Qt::CaseInsensitive ) )
  {
    // ideally we should look for .qml file inside zip file
    myURI.remove( 0, 8 );
  }
  else if ( myURI.startsWith( QLatin1String( "/vsitar/" ), Qt::CaseInsensitive ) &&
            ( myURI.endsWith( QLatin1String( ".tar" ), Qt::CaseInsensitive ) ||
              myURI.endsWith( QLatin1String( ".tar.gz" ), Qt::CaseInsensitive ) ||
              myURI.endsWith( QLatin1String( ".tgz" ), Qt::CaseInsensitive ) ) )
  {
    // ideally we should look for .qml file inside tar file
    myURI.remove( 0, 8 );
  }

  QFileInfo myFileInfo( myURI );
  QString key;

  if ( myFileInfo.exists() )
  {
    // if file is using the /vsizip/ or /vsigzip/ mechanism, cleanup the name
    if ( myURI.endsWith( QLatin1String( ".gz" ), Qt::CaseInsensitive ) )
      myURI.chop( 3 );
    else if ( myURI.endsWith( QLatin1String( ".zip" ), Qt::CaseInsensitive ) )
      myURI.chop( 4 );
    else if ( myURI.endsWith( QLatin1String( ".tar" ), Qt::CaseInsensitive ) )
      myURI.chop( 4 );
    else if ( myURI.endsWith( QLatin1String( ".tar.gz" ), Qt::CaseInsensitive ) )
      myURI.chop( 7 );
    else if ( myURI.endsWith( QLatin1String( ".tgz" ), Qt::CaseInsensitive ) )
      myURI.chop( 4 );
    myFileInfo.setFile( myURI );
    // get the file name for our .qml style file
    key = myFileInfo.path() + QDir::separator() + myFileInfo.completeBaseName() + QgsMapLayer::extensionPropertyType( type );
  }
  else
  {
    key = publicSource();
  }

  return key;
}

QString QgsMapLayer::metadataUri() const
{
  return baseURI( PropertyType::Metadata );
}

QString QgsMapLayer::saveDefaultMetadata( bool &resultFlag )
{
  return saveNamedMetadata( metadataUri(), resultFlag );
}

QString QgsMapLayer::loadDefaultMetadata( bool &resultFlag )
{
  return loadNamedMetadata( metadataUri(), resultFlag );
}

QString QgsMapLayer::styleURI() const
{
  return baseURI( PropertyType::Style );
}

QString QgsMapLayer::loadDefaultStyle( bool &resultFlag )
{
  return loadNamedStyle( styleURI(), resultFlag );
}

bool QgsMapLayer::loadNamedMetadataFromDatabase( const QString &db, const QString &uri, QString &qmd )
{
  return loadNamedPropertyFromDatabase( db, uri, qmd, PropertyType::Metadata );
}

bool QgsMapLayer::loadNamedStyleFromDatabase( const QString &db, const QString &uri, QString &qml )
{
  return loadNamedPropertyFromDatabase( db, uri, qml, PropertyType::Style );
}

bool QgsMapLayer::loadNamedPropertyFromDatabase( const QString &db, const QString &uri, QString &xml, QgsMapLayer::PropertyType type )
{
  QgsDebugMsgLevel( QStringLiteral( "db = %1 uri = %2" ).arg( db, uri ), 4 );

  bool resultFlag = false;

  // read from database
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr statement;

  int myResult;

  QgsDebugMsgLevel( QStringLiteral( "Trying to load style or metadata for \"%1\" from \"%2\"" ).arg( uri, db ), 4 );

  if ( db.isEmpty() || !QFile( db ).exists() )
    return false;

  myResult = database.open_v2( db, SQLITE_OPEN_READONLY, nullptr );
  if ( myResult != SQLITE_OK )
  {
    return false;
  }

  QString mySql;
  switch ( type )
  {
    case Metadata:
      mySql = QStringLiteral( "select qmd from tbl_metadata where metadata=?" );
      break;

    case Style:
      mySql = QStringLiteral( "select qml from tbl_styles where style=?" );
      break;
  }

  statement = database.prepare( mySql, myResult );
  if ( myResult == SQLITE_OK )
  {
    QByteArray param = uri.toUtf8();

    if ( sqlite3_bind_text( statement.get(), 1, param.data(), param.length(), SQLITE_STATIC ) == SQLITE_OK &&
         sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      xml = QString::fromUtf8( reinterpret_cast< const char * >( sqlite3_column_text( statement.get(), 0 ) ) );
      resultFlag = true;
    }
  }
  return resultFlag;
}


QString QgsMapLayer::loadNamedStyle( const QString &uri, bool &resultFlag, QgsMapLayer::StyleCategories categories )
{
  return loadNamedProperty( uri, PropertyType::Style, resultFlag, categories );
}

QString QgsMapLayer::loadNamedProperty( const QString &uri, QgsMapLayer::PropertyType type, bool &resultFlag, StyleCategories categories )
{
  QgsDebugMsgLevel( QStringLiteral( "uri = %1 myURI = %2" ).arg( uri, publicSource() ), 4 );

  resultFlag = false;

  QDomDocument myDocument( QStringLiteral( "qgis" ) );

  // location of problem associated with errorMsg
  int line, column;
  QString myErrorMessage;

  QFile myFile( uri );
  if ( myFile.open( QFile::ReadOnly ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "file found %1" ).arg( uri ), 2 );
    // read file
    resultFlag = myDocument.setContent( &myFile, &myErrorMessage, &line, &column );
    if ( !resultFlag )
      myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
    myFile.close();
  }
  else
  {
    QFileInfo project( QgsProject::instance()->fileName() );
    QgsDebugMsgLevel( QStringLiteral( "project fileName: %1" ).arg( project.absoluteFilePath() ), 4 );

    QString xml;
    switch ( type )
    {
      case QgsMapLayer::Style:
      {
        if ( loadNamedStyleFromDatabase( QDir( QgsApplication::qgisSettingsDirPath() ).absoluteFilePath( QStringLiteral( "qgis.qmldb" ) ), uri, xml ) ||
             ( project.exists() && loadNamedStyleFromDatabase( project.absoluteDir().absoluteFilePath( project.baseName() + ".qmldb" ), uri, xml ) ) ||
             loadNamedStyleFromDatabase( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/qgis.qmldb" ) ), uri, xml ) )
        {
          resultFlag = myDocument.setContent( xml, &myErrorMessage, &line, &column );
          if ( !resultFlag )
          {
            myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
          }
        }
        else
        {
          myErrorMessage = tr( "Style not found in database" );
          resultFlag = false;
        }
        break;
      }
      case QgsMapLayer::Metadata:
      {
        if ( loadNamedMetadataFromDatabase( QDir( QgsApplication::qgisSettingsDirPath() ).absoluteFilePath( QStringLiteral( "qgis.qmldb" ) ), uri, xml ) ||
             ( project.exists() && loadNamedMetadataFromDatabase( project.absoluteDir().absoluteFilePath( project.baseName() + ".qmldb" ), uri, xml ) ) ||
             loadNamedMetadataFromDatabase( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/qgis.qmldb" ) ), uri, xml ) )
        {
          resultFlag = myDocument.setContent( xml, &myErrorMessage, &line, &column );
          if ( !resultFlag )
          {
            myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
          }
        }
        else
        {
          myErrorMessage = tr( "Metadata not found in database" );
          resultFlag = false;
        }
        break;
      }
    }
  }

  if ( !resultFlag )
  {
    return myErrorMessage;
  }

  switch ( type )
  {
    case QgsMapLayer::Style:
      resultFlag = importNamedStyle( myDocument, myErrorMessage, categories );
      if ( !resultFlag )
        myErrorMessage = tr( "Loading style file %1 failed because:\n%2" ).arg( uri, myErrorMessage );
      break;
    case QgsMapLayer::Metadata:
      resultFlag = importNamedMetadata( myDocument, myErrorMessage );
      if ( !resultFlag )
        myErrorMessage = tr( "Loading metadata file %1 failed because:\n%2" ).arg( uri, myErrorMessage );
      break;
  }
  return myErrorMessage;
}

bool QgsMapLayer::importNamedMetadata( QDomDocument &document, QString &errorMessage )
{
  QDomElement myRoot = document.firstChildElement( QStringLiteral( "qgis" ) );
  if ( myRoot.isNull() )
  {
    errorMessage = tr( "Root <qgis> element could not be found" );
    return false;
  }

  return mMetadata.readMetadataXml( myRoot );
}

bool QgsMapLayer::importNamedStyle( QDomDocument &myDocument, QString &myErrorMessage, QgsMapLayer::StyleCategories categories )
{
  QDomElement myRoot = myDocument.firstChildElement( QStringLiteral( "qgis" ) );
  if ( myRoot.isNull() )
  {
    myErrorMessage = tr( "Root <qgis> element could not be found" );
    return false;
  }

  // get style file version string, if any
  QgsProjectVersion fileVersion( myRoot.attribute( QStringLiteral( "version" ) ) );
  QgsProjectVersion thisVersion( Qgis::QGIS_VERSION );

  if ( thisVersion > fileVersion )
  {
    QgsProjectFileTransform styleFile( myDocument, fileVersion );
    styleFile.updateRevision( thisVersion );
  }

  // Get source categories
  QgsMapLayer::StyleCategories sourceCategories = QgsXmlUtils::readFlagAttribute( myRoot, QStringLiteral( "styleCategories" ), QgsMapLayer::AllStyleCategories );

  //Test for matching geometry type on vector layers when applying, if geometry type is given in the style
  if ( ( sourceCategories.testFlag( QgsMapLayer::Symbology ) || sourceCategories.testFlag( QgsMapLayer::Symbology3D ) ) &&
       ( categories.testFlag( QgsMapLayer::Symbology ) || categories.testFlag( QgsMapLayer::Symbology3D ) ) )
  {
    if ( type() == QgsMapLayer::VectorLayer && !myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).isNull() )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( this );
      QgsWkbTypes::GeometryType importLayerGeometryType = static_cast<QgsWkbTypes::GeometryType>( myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).text().toInt() );
      if ( vl->geometryType() != importLayerGeometryType )
      {
        myErrorMessage = tr( "Cannot apply style with symbology to layer with a different geometry type" );
        return false;
      }
    }
  }

  QgsReadWriteContext context = QgsReadWriteContext();
  return readSymbology( myRoot, myErrorMessage, context, categories ); // TODO: support relative paths in QML?
}

void QgsMapLayer::exportNamedMetadata( QDomDocument &doc, QString &errorMsg ) const
{
  QDomImplementation DomImplementation;
  QDomDocumentType documentType = DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument myDocument( documentType );

  QDomElement myRootNode = myDocument.createElement( QStringLiteral( "qgis" ) );
  myRootNode.setAttribute( QStringLiteral( "version" ), Qgis::QGIS_VERSION );
  myDocument.appendChild( myRootNode );

  if ( !mMetadata.writeMetadataXml( myRootNode, myDocument ) )
  {
    errorMsg = QObject::tr( "Could not save metadata" );
    return;
  }

  doc = myDocument;
}

void QgsMapLayer::exportNamedStyle( QDomDocument &doc, QString &errorMsg, const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QDomImplementation DomImplementation;
  QDomDocumentType documentType = DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument myDocument( documentType );

  QDomElement myRootNode = myDocument.createElement( QStringLiteral( "qgis" ) );
  myRootNode.setAttribute( QStringLiteral( "version" ), Qgis::QGIS_VERSION );
  myDocument.appendChild( myRootNode );

  if ( !writeSymbology( myRootNode, myDocument, errorMsg, context, categories ) )  // TODO: support relative paths in QML?
  {
    errorMsg = QObject::tr( "Could not save symbology because:\n%1" ).arg( errorMsg );
    return;
  }

  /*
   * Check to see if the layer is vector - in which case we should also export its geometryType
   * to avoid eventually pasting to a layer with a different geometry
  */
  if ( type() == QgsMapLayer::VectorLayer )
  {
    //Getting the selectionLayer geometry
    const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( this );
    QString geoType = QString::number( vl->geometryType() );

    //Adding geometryinformation
    QDomElement layerGeometryType = myDocument.createElement( QStringLiteral( "layerGeometryType" ) );
    QDomText type = myDocument.createTextNode( geoType );

    layerGeometryType.appendChild( type );
    myRootNode.appendChild( layerGeometryType );
  }

  doc = myDocument;
}

QString QgsMapLayer::saveDefaultStyle( bool &resultFlag )
{
  return saveNamedStyle( styleURI(), resultFlag );
}

QString QgsMapLayer::saveNamedMetadata( const QString &uri, bool &resultFlag )
{
  return saveNamedProperty( uri, QgsMapLayer::Metadata, resultFlag );
}

QString QgsMapLayer::loadNamedMetadata( const QString &uri, bool &resultFlag )
{
  return loadNamedProperty( uri, QgsMapLayer::Metadata, resultFlag );
}

QString QgsMapLayer::saveNamedProperty( const QString &uri, QgsMapLayer::PropertyType type, bool &resultFlag, StyleCategories categories )
{
  // check if the uri is a file or ends with .qml/.qmd,
  // which indicates that it should become one
  // everything else goes to the database
  QString filename;

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( this );
  if ( vlayer && vlayer->providerType() == QLatin1String( "ogr" ) )
  {
    QStringList theURIParts = uri.split( '|' );
    filename = theURIParts[0];
  }
  else if ( vlayer && vlayer->providerType() == QLatin1String( "gpx" ) )
  {
    QStringList theURIParts = uri.split( '?' );
    filename = theURIParts[0];
  }
  else if ( vlayer && vlayer->providerType() == QLatin1String( "delimitedtext" ) )
  {
    filename = QUrl::fromEncoded( uri.toLatin1() ).toLocalFile();
    // toLocalFile() returns an empty string if theURI is a plain Windows-path, e.g. "C:/style.qml"
    if ( filename.isEmpty() )
      filename = uri;
  }
  else
  {
    filename = uri;
  }

  QString myErrorMessage;
  QDomDocument myDocument;
  switch ( type )
  {
    case Metadata:
      exportNamedMetadata( myDocument, myErrorMessage );
      break;

    case Style:
      QgsReadWriteContext context;
      exportNamedStyle( myDocument, myErrorMessage, context, categories );
      break;
  }

  QFileInfo myFileInfo( filename );
  if ( myFileInfo.exists() || filename.endsWith( QgsMapLayer::extensionPropertyType( type ), Qt::CaseInsensitive ) )
  {
    QFileInfo myDirInfo( myFileInfo.path() );  //excludes file name
    if ( !myDirInfo.isWritable() )
    {
      return tr( "The directory containing your dataset needs to be writable!" );
    }

    // now construct the file name for our .qml or .qmd file
    QString myFileName = myFileInfo.path() + QDir::separator() + myFileInfo.completeBaseName() + QgsMapLayer::extensionPropertyType( type );

    QFile myFile( myFileName );
    if ( myFile.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      QTextStream myFileStream( &myFile );
      // save as utf-8 with 2 spaces for indents
      myDocument.save( myFileStream, 2 );
      myFile.close();
      resultFlag = true;
      switch ( type )
      {
        case Metadata:
          return tr( "Created default metadata file as %1" ).arg( myFileName );

        case Style:
          return tr( "Created default style file as %1" ).arg( myFileName );
      }

    }
    else
    {
      resultFlag = false;
      switch ( type )
      {
        case Metadata:
          return tr( "ERROR: Failed to created default metadata file as %1. Check file permissions and retry." ).arg( myFileName );

        case Style:
          return tr( "ERROR: Failed to created default style file as %1. Check file permissions and retry." ).arg( myFileName );
      }
    }
  }
  else
  {
    QString qml = myDocument.toString();

    // read from database
    sqlite3_database_unique_ptr database;
    sqlite3_statement_unique_ptr statement;

    int myResult = database.open( QDir( QgsApplication::qgisSettingsDirPath() ).absoluteFilePath( QStringLiteral( "qgis.qmldb" ) ) );
    if ( myResult != SQLITE_OK )
    {
      return tr( "User database could not be opened." );
    }

    QByteArray param0 = uri.toUtf8();
    QByteArray param1 = qml.toUtf8();

    QString mySql;
    switch ( type )
    {
      case Metadata:
        mySql = QStringLiteral( "create table if not exists tbl_metadata(metadata varchar primary key,qmd varchar)" );
        break;

      case Style:
        mySql = QStringLiteral( "create table if not exists tbl_styles(style varchar primary key,qml varchar)" );
        break;
    }

    statement = database.prepare( mySql, myResult );
    if ( myResult == SQLITE_OK )
    {
      if ( sqlite3_step( statement.get() ) != SQLITE_DONE )
      {
        resultFlag = false;
        switch ( type )
        {
          case Metadata:
            return tr( "The metadata table could not be created." );

          case Style:
            return tr( "The style table could not be created." );
        }
      }
    }

    switch ( type )
    {
      case Metadata:
        mySql = QStringLiteral( "insert into tbl_metadata(metadata,qmd) values (?,?)" );
        break;

      case Style:
        mySql = QStringLiteral( "insert into tbl_styles(style,qml) values (?,?)" );
        break;
    }
    statement = database.prepare( mySql, myResult );
    if ( myResult == SQLITE_OK )
    {
      if ( sqlite3_bind_text( statement.get(), 1, param0.data(), param0.length(), SQLITE_STATIC ) == SQLITE_OK &&
           sqlite3_bind_text( statement.get(), 2, param1.data(), param1.length(), SQLITE_STATIC ) == SQLITE_OK &&
           sqlite3_step( statement.get() ) == SQLITE_DONE )
      {
        resultFlag = true;
        switch ( type )
        {
          case Metadata:
            myErrorMessage = tr( "The metadata %1 was saved to database" ).arg( uri );
            break;

          case Style:
            myErrorMessage = tr( "The style %1 was saved to database" ).arg( uri );
            break;
        }
      }
    }

    if ( !resultFlag )
    {
      QString mySql;
      switch ( type )
      {
        case Metadata:
          mySql = QStringLiteral( "update tbl_metadata set qmd=? where metadata=?" );
          break;

        case Style:
          mySql = QStringLiteral( "update tbl_styles set qml=? where style=?" );
          break;
      }
      statement = database.prepare( mySql, myResult );
      if ( myResult == SQLITE_OK )
      {
        if ( sqlite3_bind_text( statement.get(), 2, param0.data(), param0.length(), SQLITE_STATIC ) == SQLITE_OK &&
             sqlite3_bind_text( statement.get(), 1, param1.data(), param1.length(), SQLITE_STATIC ) == SQLITE_OK &&
             sqlite3_step( statement.get() ) == SQLITE_DONE )
        {
          resultFlag = true;
          switch ( type )
          {
            case Metadata:
              myErrorMessage = tr( "The metadata %1 was updated in the database." ).arg( uri );
              break;

            case Style:
              myErrorMessage = tr( "The style %1 was updated in the database." ).arg( uri );
              break;
          }
        }
        else
        {
          resultFlag = false;
          switch ( type )
          {
            case Metadata:
              myErrorMessage = tr( "The metadata %1 could not be updated in the database." ).arg( uri );
              break;

            case Style:
              myErrorMessage = tr( "The style %1 could not be updated in the database." ).arg( uri );
              break;
          }
        }
      }
      else
      {
        resultFlag = false;
        switch ( type )
        {
          case Metadata:
            myErrorMessage = tr( "The metadata %1 could not be inserted into database." ).arg( uri );
            break;

          case Style:
            myErrorMessage = tr( "The style %1 could not be inserted into database." ).arg( uri );
            break;
        }
      }
    }
  }

  return myErrorMessage;
}

QString QgsMapLayer::saveNamedStyle( const QString &uri, bool &resultFlag, StyleCategories categories )
{
  return saveNamedProperty( uri, QgsMapLayer::Style, resultFlag, categories );
}

void QgsMapLayer::exportSldStyle( QDomDocument &doc, QString &errorMsg ) const
{
  QDomDocument myDocument = QDomDocument();

  QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
  myDocument.appendChild( header );

  const QgsVectorLayer *vlayer = qobject_cast<const QgsVectorLayer *>( this );
  const QgsRasterLayer *rlayer = qobject_cast<const QgsRasterLayer *>( this );
  if ( !vlayer && !rlayer )
  {
    errorMsg = tr( "Could not save symbology because:\n%1" )
               .arg( tr( "Only vector and raster layers are supported" ) );
    return;
  }

  // Create the root element
  QDomElement root = myDocument.createElementNS( QStringLiteral( "http://www.opengis.net/sld" ), QStringLiteral( "StyledLayerDescriptor" ) );
  QDomElement layerNode;
  if ( vlayer )
  {
    root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.1.0" ) );
    root.setAttribute( QStringLiteral( "xsi:schemaLocation" ), QStringLiteral( "http://www.opengis.net/sld http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd" ) );
    root.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
    root.setAttribute( QStringLiteral( "xmlns:se" ), QStringLiteral( "http://www.opengis.net/se" ) );
    root.setAttribute( QStringLiteral( "xmlns:xlink" ), QStringLiteral( "http://www.w3.org/1999/xlink" ) );
    root.setAttribute( QStringLiteral( "xmlns:xsi" ), QStringLiteral( "http://www.w3.org/2001/XMLSchema-instance" ) );
    myDocument.appendChild( root );

    // Create the NamedLayer element
    layerNode = myDocument.createElement( QStringLiteral( "NamedLayer" ) );
    root.appendChild( layerNode );
  }

  // note: Only SLD 1.0 version is generated because seems none is using SE1.1.0 at least for rasters
  if ( rlayer )
  {
    // Create the root element
    root.setAttribute( QStringLiteral( "version" ), QStringLiteral( "1.0.0" ) );
    root.setAttribute( QStringLiteral( "xmlns:gml" ), QStringLiteral( "http://www.opengis.net/gml" ) );
    root.setAttribute( QStringLiteral( "xmlns:ogc" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
    root.setAttribute( QStringLiteral( "xmlns:sld" ), QStringLiteral( "http://www.opengis.net/sld" ) );
    myDocument.appendChild( root );

    // Create the NamedLayer element
    layerNode = myDocument.createElement( QStringLiteral( "UserLayer" ) );
    root.appendChild( layerNode );
  }

  QgsStringMap props;
  if ( hasScaleBasedVisibility() )
  {
    props[ QStringLiteral( "scaleMinDenom" ) ] = QString::number( mMinScale );
    props[ QStringLiteral( "scaleMaxDenom" ) ] = QString::number( mMaxScale );
  }

  if ( vlayer )
  {
    if ( !vlayer->writeSld( layerNode, myDocument, errorMsg, props ) )
    {
      errorMsg = tr( "Could not save symbology because:\n%1" ).arg( errorMsg );
      return;
    }
  }

  if ( rlayer )
  {
    if ( !rlayer->writeSld( layerNode, myDocument, errorMsg, props ) )
    {
      errorMsg = tr( "Could not save symbology because:\n%1" ).arg( errorMsg );
      return;
    }
  }

  doc = myDocument;
}

QString QgsMapLayer::saveSldStyle( const QString &uri, bool &resultFlag ) const
{
  const QgsMapLayer *mlayer = qobject_cast<const QgsMapLayer *>( this );

  QString errorMsg;
  QDomDocument myDocument;
  mlayer->exportSldStyle( myDocument, errorMsg );
  if ( !errorMsg.isNull() )
  {
    resultFlag = false;
    return errorMsg;
  }
  // check if the uri is a file or ends with .sld,
  // which indicates that it should become one
  QString filename;
  if ( mlayer->providerType() == QLatin1String( "ogr" ) )
  {
    QStringList theURIParts = uri.split( '|' );
    filename = theURIParts[0];
  }
  else if ( mlayer->providerType() == QLatin1String( "gpx" ) )
  {
    QStringList theURIParts = uri.split( '?' );
    filename = theURIParts[0];
  }
  else if ( mlayer->providerType() == QLatin1String( "delimitedtext" ) )
  {
    filename = QUrl::fromEncoded( uri.toLatin1() ).toLocalFile();
    // toLocalFile() returns an empty string if theURI is a plain Windows-path, e.g. "C:/style.qml"
    if ( filename.isEmpty() )
      filename = uri;
  }
  else
  {
    filename = uri;
  }

  QFileInfo myFileInfo( filename );
  if ( myFileInfo.exists() || filename.endsWith( QLatin1String( ".sld" ), Qt::CaseInsensitive ) )
  {
    QFileInfo myDirInfo( myFileInfo.path() );  //excludes file name
    if ( !myDirInfo.isWritable() )
    {
      return tr( "The directory containing your dataset needs to be writable!" );
    }

    // now construct the file name for our .sld style file
    QString myFileName = myFileInfo.path() + QDir::separator() + myFileInfo.completeBaseName() + ".sld";

    QFile myFile( myFileName );
    if ( myFile.open( QFile::WriteOnly | QFile::Truncate ) )
    {
      QTextStream myFileStream( &myFile );
      // save as utf-8 with 2 spaces for indents
      myDocument.save( myFileStream, 2 );
      myFile.close();
      resultFlag = true;
      return tr( "Created default style file as %1" ).arg( myFileName );
    }
  }

  resultFlag = false;
  return tr( "ERROR: Failed to created SLD style file as %1. Check file permissions and retry." ).arg( filename );
}

QString QgsMapLayer::loadSldStyle( const QString &uri, bool &resultFlag )
{
  resultFlag = false;

  QDomDocument myDocument;

  // location of problem associated with errorMsg
  int line, column;
  QString myErrorMessage;

  QFile myFile( uri );
  if ( myFile.open( QFile::ReadOnly ) )
  {
    // read file
    resultFlag = myDocument.setContent( &myFile, true, &myErrorMessage, &line, &column );
    if ( !resultFlag )
      myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
    myFile.close();
  }
  else
  {
    myErrorMessage = tr( "Unable to open file %1" ).arg( uri );
  }

  if ( !resultFlag )
  {
    return myErrorMessage;
  }

  // check for root SLD element
  QDomElement myRoot = myDocument.firstChildElement( QStringLiteral( "StyledLayerDescriptor" ) );
  if ( myRoot.isNull() )
  {
    myErrorMessage = QStringLiteral( "Error: StyledLayerDescriptor element not found in %1" ).arg( uri );
    resultFlag = false;
    return myErrorMessage;
  }

  // now get the style node out and pass it over to the layer
  // to deserialise...
  QDomElement namedLayerElem = myRoot.firstChildElement( QStringLiteral( "NamedLayer" ) );
  if ( namedLayerElem.isNull() )
  {
    myErrorMessage = QStringLiteral( "Info: NamedLayer element not found." );
    resultFlag = false;
    return myErrorMessage;
  }

  QString errorMsg;
  resultFlag = readSld( namedLayerElem, errorMsg );
  if ( !resultFlag )
  {
    myErrorMessage = tr( "Loading style file %1 failed because:\n%2" ).arg( uri, errorMsg );
    return myErrorMessage;
  }

  return QString();
}

bool QgsMapLayer::readStyle( const QDomNode &node, QString &errorMessage, QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories )
{
  Q_UNUSED( node );
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );
  Q_UNUSED( categories );
  return false;
}

bool QgsMapLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                              const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( node );
  Q_UNUSED( doc );
  Q_UNUSED( errorMessage );
  Q_UNUSED( context );
  Q_UNUSED( categories );
  return false;
}

void QgsMapLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider, const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag )
{
  Q_UNUSED( dataSource );
  Q_UNUSED( baseName );
  Q_UNUSED( provider );
  Q_UNUSED( options );
  Q_UNUSED( loadDefaultStyleFlag );
}


QString QgsMapLayer::providerType() const
{
  return mProviderKey;
}

void QgsMapLayer::readCommonStyle( const QDomElement &layerElement, const QgsReadWriteContext &context,
                                   QgsMapLayer::StyleCategories categories )
{
  if ( categories.testFlag( Symbology3D ) )
  {
    QgsAbstract3DRenderer *r3D = nullptr;
    QDomElement renderer3DElem = layerElement.firstChildElement( QStringLiteral( "renderer-3d" ) );
    if ( !renderer3DElem.isNull() )
    {
      QString type3D = renderer3DElem.attribute( QStringLiteral( "type" ) );
      Qgs3DRendererAbstractMetadata *meta3D = QgsApplication::renderer3DRegistry()->rendererMetadata( type3D );
      if ( meta3D )
      {
        r3D = meta3D->createRenderer( renderer3DElem, context );
      }
    }
    setRenderer3D( r3D );
  }

  if ( categories.testFlag( CustomProperties ) )
  {
    // read custom properties before passing reading further to a subclass, so that
    // the subclass can also read custom properties
    readCustomProperties( layerElement );
  }

  // use scale dependent visibility flag
  if ( categories.testFlag( Rendering ) )
  {
    setScaleBasedVisibility( layerElement.attribute( QStringLiteral( "hasScaleBasedVisibilityFlag" ) ).toInt() == 1 );
    if ( layerElement.hasAttribute( QStringLiteral( "minimumScale" ) ) )
    {
      // older element, when scales were reversed
      setMaximumScale( layerElement.attribute( QStringLiteral( "minimumScale" ) ).toDouble() );
      setMinimumScale( layerElement.attribute( QStringLiteral( "maximumScale" ) ).toDouble() );
    }
    else
    {
      setMaximumScale( layerElement.attribute( QStringLiteral( "maxScale" ) ).toDouble() );
      setMinimumScale( layerElement.attribute( QStringLiteral( "minScale" ) ).toDouble() );
    }
  }

  if ( categories.testFlag( LayerConfiguration ) )
  {
    // flags
    QDomElement flagsElem = layerElement.firstChildElement( QStringLiteral( "flags" ) );
    LayerFlags flags = mFlags;
    auto enumMap = qgsEnumMap<QgsMapLayer::LayerFlag>();
    for ( auto it = enumMap.constBegin(); it != enumMap.constEnd(); ++it )
    {
      QDomNode flagNode = flagsElem.namedItem( it.value() );
      if ( flagNode.isNull() )
        continue;
      bool flagValue = flagNode.toElement().text() == "1" ? true : false;
      if ( flags.testFlag( it.key() ) && !flagValue )
        flags &= ~it.key();
      else if ( !flags.testFlag( it.key() ) && flagValue )
        flags |= it.key();
    }
    setFlags( flags );
  }
}

QUndoStack *QgsMapLayer::undoStack()
{
  return mUndoStack;
}

QUndoStack *QgsMapLayer::undoStackStyles()
{
  return mUndoStackStyles;
}

QStringList QgsMapLayer::customPropertyKeys() const
{
  return mCustomProperties.keys();
}

void QgsMapLayer::setCustomProperty( const QString &key, const QVariant &value )
{
  mCustomProperties.setValue( key, value );
}

void QgsMapLayer::setCustomProperties( const QgsObjectCustomProperties &properties )
{
  mCustomProperties = properties;
}

QVariant QgsMapLayer::customProperty( const QString &value, const QVariant &defaultValue ) const
{
  return mCustomProperties.value( value, defaultValue );
}

void QgsMapLayer::removeCustomProperty( const QString &key )
{
  mCustomProperties.remove( key );
}

QgsError QgsMapLayer::error() const
{
  return mError;
}



bool QgsMapLayer::isEditable() const
{
  return false;
}

bool QgsMapLayer::isSpatial() const
{
  return true;
}

void QgsMapLayer::setValid( bool valid )
{
  mValid = valid;
}

void QgsMapLayer::setLegend( QgsMapLayerLegend *legend )
{
  if ( legend == mLegend )
    return;

  delete mLegend;
  mLegend = legend;

  if ( mLegend )
  {
    mLegend->setParent( this );
    connect( mLegend, &QgsMapLayerLegend::itemsChanged, this, &QgsMapLayer::legendChanged );
  }

  emit legendChanged();
}

QgsMapLayerLegend *QgsMapLayer::legend() const
{
  return mLegend;
}

QgsMapLayerStyleManager *QgsMapLayer::styleManager() const
{
  return mStyleManager;
}

void QgsMapLayer::setRenderer3D( QgsAbstract3DRenderer *renderer )
{
  if ( renderer == m3DRenderer )
    return;

  delete m3DRenderer;
  m3DRenderer = renderer;
  emit renderer3DChanged();
}

QgsAbstract3DRenderer *QgsMapLayer::renderer3D() const
{
  return m3DRenderer;
}

void QgsMapLayer::triggerRepaint( bool deferredUpdate )
{
  emit repaintRequested( deferredUpdate );
}

void QgsMapLayer::setMetadata( const QgsLayerMetadata &metadata )
{
  mMetadata = metadata;
//  mMetadata.saveToLayer( this );
  emit metadataChanged();
}

QString QgsMapLayer::htmlMetadata() const
{
  return QString();
}

QDateTime QgsMapLayer::timestamp() const
{
  return QDateTime();
}

void QgsMapLayer::emitStyleChanged()
{
  emit styleChanged();
}

void QgsMapLayer::setExtent( const QgsRectangle &r )
{
  mExtent = r;
}

static QList<const QgsMapLayer *> _depOutEdges( const QgsMapLayer *vl, const QgsMapLayer *that, const QSet<QgsMapLayerDependency> &layers )
{
  QList<const QgsMapLayer *> lst;
  if ( vl == that )
  {
    Q_FOREACH ( const QgsMapLayerDependency &dep, layers )
    {
      if ( const QgsMapLayer *l = QgsProject::instance()->mapLayer( dep.layerId() ) )
        lst << l;
    }
  }
  else
  {
    Q_FOREACH ( const QgsMapLayerDependency &dep, vl->dependencies() )
    {
      if ( const QgsMapLayer *l = QgsProject::instance()->mapLayer( dep.layerId() ) )
        lst << l;
    }
  }
  return lst;
}

static bool _depHasCycleDFS( const QgsMapLayer *n, QHash<const QgsMapLayer *, int> &mark, const QgsMapLayer *that, const QSet<QgsMapLayerDependency> &layers )
{
  if ( mark.value( n ) == 1 ) // temporary
    return true;
  if ( mark.value( n ) == 0 ) // not visited
  {
    mark[n] = 1; // temporary
    Q_FOREACH ( const QgsMapLayer *m, _depOutEdges( n, that, layers ) )
    {
      if ( _depHasCycleDFS( m, mark, that, layers ) )
        return true;
    }
    mark[n] = 2; // permanent
  }
  return false;
}

bool QgsMapLayer::hasDependencyCycle( const QSet<QgsMapLayerDependency> &layers ) const
{
  QHash<const QgsMapLayer *, int> marks;
  return _depHasCycleDFS( this, marks, this, layers );
}

bool QgsMapLayer::isReadOnly() const
{
  return true;
}

QString QgsMapLayer::originalXmlProperties() const
{
  return mOriginalXmlProperties;
}

void QgsMapLayer::setOriginalXmlProperties( const QString &originalXmlProperties )
{
  mOriginalXmlProperties = originalXmlProperties;
}

void QgsMapLayer::setProviderType( const QString &providerType )
{
  mProviderKey = providerType;
}

QSet<QgsMapLayerDependency> QgsMapLayer::dependencies() const
{
  return mDependencies;
}

bool QgsMapLayer::setDependencies( const QSet<QgsMapLayerDependency> &oDeps )
{
  QSet<QgsMapLayerDependency> deps;
  Q_FOREACH ( const QgsMapLayerDependency &dep, oDeps )
  {
    if ( dep.origin() == QgsMapLayerDependency::FromUser )
      deps << dep;
  }
  if ( hasDependencyCycle( deps ) )
    return false;

  mDependencies = deps;
  emit dependenciesChanged();
  return true;
}

void QgsMapLayer::setRefreshOnNotifyEnabled( bool enabled )
{
  if ( !dataProvider() )
    return;

  if ( enabled && !isRefreshOnNotifyEnabled() )
  {
    dataProvider()->setListening( enabled );
    connect( dataProvider(), &QgsVectorDataProvider::notify, this, &QgsMapLayer::onNotifiedTriggerRepaint );
  }
  else if ( !enabled && isRefreshOnNotifyEnabled() )
  {
    // we don't want to disable provider listening because someone else could need it (e.g. actions)
    disconnect( dataProvider(), &QgsVectorDataProvider::notify, this, &QgsMapLayer::onNotifiedTriggerRepaint );
  }
  mIsRefreshOnNofifyEnabled = enabled;
}

void QgsMapLayer::onNotifiedTriggerRepaint( const QString &message )
{
  if ( refreshOnNotifyMessage().isEmpty() || refreshOnNotifyMessage() == message )
    triggerRepaint();
}

