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


#include "qgssqliteutils.h"
#include "qgs3drendererregistry.h"
#include "qgsabstract3drenderer.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgsfileutils.h"
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
#include "qgsmessagelog.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgsprovidermetadata.h"
#include "qgslayernotesutils.h"
#include "qgsdatums.h"
#include "qgsprojoperation.h"

#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QDomImplementation>
#include <QDomNode>
#include <QFile>
#include <QFileInfo>
#include <QLocale>
#include <QTextStream>
#include <QUrl>
#include <QTimer>
#include <QStandardPaths>
#include <QUuid>
#include <QRegularExpression>

#include <sqlite3.h>

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

QgsMapLayer::QgsMapLayer( QgsMapLayerType type,
                          const QString &lyrname,
                          const QString &source )
  : mDataSource( source )
  , mLayerName( lyrname )
  , mLayerType( type )
  , mServerProperties( std::make_unique<QgsMapLayerServerProperties>( this ) )
  , mUndoStack( new QUndoStack( this ) )
  , mUndoStackStyles( new QUndoStack( this ) )
  , mStyleManager( new QgsMapLayerStyleManager( this ) )
  , mRefreshTimer( new QTimer( this ) )
{
  mID = generateId( lyrname );
  connect( this, &QgsMapLayer::crsChanged, this, &QgsMapLayer::configChanged );
  connect( this, &QgsMapLayer::nameChanged, this, &QgsMapLayer::configChanged );
  connect( mRefreshTimer, &QTimer::timeout, this, [ = ] { triggerRepaint( true ); } );
}

QgsMapLayer::~QgsMapLayer()
{
  if ( project() && project()->pathResolver().writePath( mDataSource ).startsWith( "attachment:" ) )
  {
    project()->removeAttachedFile( mDataSource );
  }

  delete m3DRenderer;
  delete mLegend;
  delete mStyleManager;
}

void QgsMapLayer::clone( QgsMapLayer *layer ) const
{
  layer->setBlendMode( blendMode() );

  const auto constStyles = styleManager()->styles();
  for ( const QString &s : constStyles )
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
  layer->setLegendUrl( legendUrl() );
  layer->setLegendUrlFormat( legendUrlFormat() );
  layer->setDependencies( dependencies() );
  layer->mShouldValidateCrs = mShouldValidateCrs;
  layer->setCrs( crs() );
  layer->setCustomProperties( mCustomProperties );
  layer->setOpacity( mLayerOpacity );
  layer->setMetadata( mMetadata );
  layer->serverProperties()->copyTo( mServerProperties.get() );
}

QgsMapLayerType QgsMapLayer::type() const
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

Qgis::MapLayerProperties QgsMapLayer::properties() const
{
  return Qgis::MapLayerProperties();
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

QString QgsMapLayer::shortName() const
{
  return mShortName;
}

void QgsMapLayer::setMetadataUrl( const QString &metaUrl )
{
  QList<QgsMapLayerServerProperties::MetadataUrl> urls = serverProperties()->metadataUrls();
  if ( urls.isEmpty() )
  {
    const QgsMapLayerServerProperties::MetadataUrl newItem = QgsMapLayerServerProperties::MetadataUrl( metaUrl, QLatin1String(), QLatin1String() );
    urls.prepend( newItem );
  }
  else
  {
    const QgsMapLayerServerProperties::MetadataUrl old = urls.takeFirst();
    const QgsMapLayerServerProperties::MetadataUrl newItem( metaUrl, old.type, old.format );
    urls.prepend( newItem );
  }
  serverProperties()->setMetadataUrls( urls );
}

QString QgsMapLayer::metadataUrl() const
{
  if ( mServerProperties->metadataUrls().isEmpty() )
  {
    return QLatin1String();
  }
  else
  {
    return mServerProperties->metadataUrls().first().url;
  }
}

void QgsMapLayer::setMetadataUrlType( const QString &metaUrlType )
{
  QList<QgsMapLayerServerProperties::MetadataUrl> urls = mServerProperties->metadataUrls();
  if ( urls.isEmpty() )
  {
    const QgsMapLayerServerProperties::MetadataUrl newItem( QLatin1String(), metaUrlType, QLatin1String() );
    urls.prepend( newItem );
  }
  else
  {
    const QgsMapLayerServerProperties::MetadataUrl old = urls.takeFirst();
    const QgsMapLayerServerProperties::MetadataUrl newItem( old.url, metaUrlType, old.format );
    urls.prepend( newItem );
  }
  mServerProperties->setMetadataUrls( urls );
}

QString QgsMapLayer::metadataUrlType() const
{
  if ( mServerProperties->metadataUrls().isEmpty() )
  {
    return QLatin1String();
  }
  else
  {
    return mServerProperties->metadataUrls().first().type;
  }
}

void QgsMapLayer::setMetadataUrlFormat( const QString &metaUrlFormat )
{
  QList<QgsMapLayerServerProperties::MetadataUrl> urls = mServerProperties->metadataUrls();
  if ( urls.isEmpty() )
  {
    const QgsMapLayerServerProperties::MetadataUrl newItem( QLatin1String(), QLatin1String(), metaUrlFormat );
    urls.prepend( newItem );
  }
  else
  {
    const QgsMapLayerServerProperties::MetadataUrl old = urls.takeFirst( );
    const QgsMapLayerServerProperties::MetadataUrl newItem( old.url, old.type, metaUrlFormat );
    urls.prepend( newItem );
  }
  mServerProperties->setMetadataUrls( urls );
}

QString QgsMapLayer::metadataUrlFormat() const
{
  if ( mServerProperties->metadataUrls().isEmpty() )
  {
    return QString();
  }
  else
  {
    return mServerProperties->metadataUrls().first().format;
  }
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
  emitStyleChanged();
}

QPainter::CompositionMode QgsMapLayer::blendMode() const
{
  return mBlendMode;
}

void QgsMapLayer::setOpacity( double opacity )
{
  if ( qgsDoubleNear( mLayerOpacity, opacity ) )
    return;
  mLayerOpacity = opacity;
  emit opacityChanged( opacity );
  emitStyleChanged();
}

double QgsMapLayer::opacity() const
{
  return mLayerOpacity;
}

bool QgsMapLayer::readLayerXml( const QDomElement &layerElement, QgsReadWriteContext &context, QgsMapLayer::ReadFlags flags )
{
  bool layerError;
  mReadFlags = flags;

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
  mDataSource = context.pathResolver().readPath( mne.text() );

  // if the layer needs authentication, ensure the master password is set
  const thread_local QRegularExpression rx( "authcfg=([a-z]|[A-Z]|[0-9]){7}" );
  if ( rx.match( mDataSource ).hasMatch()
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

  const QDomNode srsNode = layerElement.namedItem( QStringLiteral( "srs" ) );
  mCRS.readXml( srsNode );
  mCRS.setValidationHint( tr( "Specify CRS for layer %1" ).arg( mne.text() ) );
  if ( isSpatial() && type() != QgsMapLayerType::AnnotationLayer )
    mCRS.validate();
  savedCRS = mCRS;

  // Do not validate any projections in children, they will be overwritten anyway.
  // No need to ask the user for a projections when it is overwritten, is there?
  savedValidation = QgsCoordinateReferenceSystem::customCrsValidation();
  QgsCoordinateReferenceSystem::setCustomCrsValidation( nullptr );

  const QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Layer" ), mne.text() );

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

  // set name
  mnl = layerElement.namedItem( QStringLiteral( "layername" ) );
  mne = mnl.toElement();

  //name can be translated
  setName( context.projectTranslator()->translate( QStringLiteral( "project:layers:%1" ).arg( layerElement.namedItem( QStringLiteral( "id" ) ).toElement().text() ), mne.text() ) );

  // now let the children grab what they need from the Dom node.
  layerError = !readXml( layerElement, context );

  // overwrite CRS with what we read from project file before the raster/vector
  // file reading functions changed it. They will if projections is specified in the file.
  // FIXME: is this necessary? Yes, it is (autumn 2019)
  QgsCoordinateReferenceSystem::setCustomCrsValidation( savedValidation );
  mCRS = savedCRS;

  //short name
  const QDomElement shortNameElem = layerElement.firstChildElement( QStringLiteral( "shortname" ) );
  if ( !shortNameElem.isNull() )
  {
    mShortName = shortNameElem.text();
  }

  //title
  const QDomElement titleElem = layerElement.firstChildElement( QStringLiteral( "title" ) );
  if ( !titleElem.isNull() )
  {
    mTitle = titleElem.text();
  }

  //abstract
  const QDomElement abstractElem = layerElement.firstChildElement( QStringLiteral( "abstract" ) );
  if ( !abstractElem.isNull() )
  {
    mAbstract = abstractElem.text();
  }

  //keywordList
  const QDomElement keywordListElem = layerElement.firstChildElement( QStringLiteral( "keywordList" ) );
  if ( !keywordListElem.isNull() )
  {
    QStringList kwdList;
    for ( QDomNode n = keywordListElem.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
      kwdList << n.toElement().text();
    }
    mKeywordList = kwdList.join( QLatin1String( ", " ) );
  }

  //dataUrl
  const QDomElement dataUrlElem = layerElement.firstChildElement( QStringLiteral( "dataUrl" ) );
  if ( !dataUrlElem.isNull() )
  {
    mDataUrl = dataUrlElem.text();
    mDataUrlFormat = dataUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }

  //legendUrl
  const QDomElement legendUrlElem = layerElement.firstChildElement( QStringLiteral( "legendUrl" ) );
  if ( !legendUrlElem.isNull() )
  {
    mLegendUrl = legendUrlElem.text();
    mLegendUrlFormat = legendUrlElem.attribute( QStringLiteral( "format" ), QString() );
  }

  //attribution
  const QDomElement attribElem = layerElement.firstChildElement( QStringLiteral( "attribution" ) );
  if ( !attribElem.isNull() )
  {
    mAttribution = attribElem.text();
    mAttributionUrl = attribElem.attribute( QStringLiteral( "href" ), QString() );
  }

  serverProperties()->readXml( layerElement );

  if ( serverProperties()->metadataUrls().isEmpty() )
  {
    // metadataUrl is still empty, maybe it's a QGIS Project < 3.22
    // keep for legacy
    const QDomElement metaUrlElem = layerElement.firstChildElement( QStringLiteral( "metadataUrl" ) );
    if ( !metaUrlElem.isNull() )
    {
      const QString url = metaUrlElem.text();
      const QString type = metaUrlElem.attribute( QStringLiteral( "type" ), QString() );
      const QString format = metaUrlElem.attribute( QStringLiteral( "format" ), QString() );
      const QgsMapLayerServerProperties::MetadataUrl newItem( url, type, format );
      mServerProperties->setMetadataUrls( QList<QgsMapLayerServerProperties::MetadataUrl>() << newItem );
    }
  }

  // mMetadata.readFromLayer( this );
  const QDomElement metadataElem = layerElement.firstChildElement( QStringLiteral( "resourceMetadata" ) );
  mMetadata.readMetadataXml( metadataElem );

  setAutoRefreshInterval( layerElement.attribute( QStringLiteral( "autoRefreshTime" ), QStringLiteral( "0" ) ).toInt() );
  setAutoRefreshEnabled( layerElement.attribute( QStringLiteral( "autoRefreshEnabled" ), QStringLiteral( "0" ) ).toInt() );
  setRefreshOnNofifyMessage( layerElement.attribute( QStringLiteral( "refreshOnNotifyMessage" ), QString() ) );
  setRefreshOnNotifyEnabled( layerElement.attribute( QStringLiteral( "refreshOnNotifyEnabled" ), QStringLiteral( "0" ) ).toInt() );

  // geographic extent is read only if necessary
  if ( mReadFlags & QgsMapLayer::ReadFlag::FlagTrustLayerMetadata )
  {
    const QDomNode wgs84ExtentNode = layerElement.namedItem( QStringLiteral( "wgs84extent" ) );
    if ( !wgs84ExtentNode.isNull() )
      mWgs84Extent = QgsXmlUtils::readRectangle( wgs84ExtentNode.toElement() );
  }

  mLegendPlaceholderImage = layerElement.attribute( QStringLiteral( "legendPlaceholderImage" ) );

  return ! layerError;
} // bool QgsMapLayer::readLayerXML


bool QgsMapLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  Q_UNUSED( layer_node )
  Q_UNUSED( context )
  // NOP by default; children will over-ride with behavior specific to them

  // read Extent
  if ( mReadFlags & QgsMapLayer::FlagReadExtentFromXml )
  {
    const QDomNode extentNode = layer_node.namedItem( QStringLiteral( "extent" ) );
    if ( !extentNode.isNull() )
    {
      mExtent = QgsXmlUtils::readRectangle( extentNode.toElement() );
    }
  }

  return true;
} // void QgsMapLayer::readXml


bool QgsMapLayer::writeLayerXml( QDomElement &layerElement, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  if ( !extent().isNull() )
  {
    layerElement.appendChild( QgsXmlUtils::writeRectangle( mExtent, document ) );
    layerElement.appendChild( QgsXmlUtils::writeRectangle( wgs84Extent( true ), document, QStringLiteral( "wgs84extent" ) ) );
  }

  layerElement.setAttribute( QStringLiteral( "autoRefreshTime" ), QString::number( mRefreshTimer->interval() ) );
  layerElement.setAttribute( QStringLiteral( "autoRefreshEnabled" ), mRefreshTimer->isActive() ? 1 : 0 );
  layerElement.setAttribute( QStringLiteral( "refreshOnNotifyEnabled" ),  mIsRefreshOnNofifyEnabled ? 1 : 0 );
  layerElement.setAttribute( QStringLiteral( "refreshOnNotifyMessage" ),  mRefreshOnNofifyMessage );

  // ID
  QDomElement layerId = document.createElement( QStringLiteral( "id" ) );
  const QDomText layerIdText = document.createTextNode( id() );
  layerId.appendChild( layerIdText );

  layerElement.appendChild( layerId );

  // data source
  QDomElement dataSource = document.createElement( QStringLiteral( "datasource" ) );
  const QString src = context.pathResolver().writePath( encodedSource( source(), context ) );
  const QDomText dataSourceText = document.createTextNode( src );
  dataSource.appendChild( dataSourceText );
  layerElement.appendChild( dataSource );

  // layer name
  QDomElement layerName = document.createElement( QStringLiteral( "layername" ) );
  const QDomText layerNameText = document.createTextNode( name() );
  layerName.appendChild( layerNameText );
  layerElement.appendChild( layerName );

  // layer short name
  if ( !mShortName.isEmpty() )
  {
    QDomElement layerShortName = document.createElement( QStringLiteral( "shortname" ) );
    const QDomText layerShortNameText = document.createTextNode( mShortName );
    layerShortName.appendChild( layerShortNameText );
    layerElement.appendChild( layerShortName );
  }

  // layer title
  if ( !mTitle.isEmpty() )
  {
    QDomElement layerTitle = document.createElement( QStringLiteral( "title" ) );
    const QDomText layerTitleText = document.createTextNode( mTitle );
    layerTitle.appendChild( layerTitleText );
    layerElement.appendChild( layerTitle );
  }

  // layer abstract
  if ( !mAbstract.isEmpty() )
  {
    QDomElement layerAbstract = document.createElement( QStringLiteral( "abstract" ) );
    const QDomText layerAbstractText = document.createTextNode( mAbstract );
    layerAbstract.appendChild( layerAbstractText );
    layerElement.appendChild( layerAbstract );
  }

  // layer keyword list
  const QStringList keywordStringList = keywordList().split( ',' );
  if ( !keywordStringList.isEmpty() )
  {
    QDomElement layerKeywordList = document.createElement( QStringLiteral( "keywordList" ) );
    for ( int i = 0; i < keywordStringList.size(); ++i )
    {
      QDomElement layerKeywordValue = document.createElement( QStringLiteral( "value" ) );
      const QDomText layerKeywordText = document.createTextNode( keywordStringList.at( i ).trimmed() );
      layerKeywordValue.appendChild( layerKeywordText );
      layerKeywordList.appendChild( layerKeywordValue );
    }
    layerElement.appendChild( layerKeywordList );
  }

  // layer dataUrl
  const QString aDataUrl = dataUrl();
  if ( !aDataUrl.isEmpty() )
  {
    QDomElement layerDataUrl = document.createElement( QStringLiteral( "dataUrl" ) );
    const QDomText layerDataUrlText = document.createTextNode( aDataUrl );
    layerDataUrl.appendChild( layerDataUrlText );
    layerDataUrl.setAttribute( QStringLiteral( "format" ), dataUrlFormat() );
    layerElement.appendChild( layerDataUrl );
  }

  // layer legendUrl
  const QString aLegendUrl = legendUrl();
  if ( !aLegendUrl.isEmpty() )
  {
    QDomElement layerLegendUrl = document.createElement( QStringLiteral( "legendUrl" ) );
    const QDomText layerLegendUrlText = document.createTextNode( aLegendUrl );
    layerLegendUrl.appendChild( layerLegendUrlText );
    layerLegendUrl.setAttribute( QStringLiteral( "format" ), legendUrlFormat() );
    layerElement.appendChild( layerLegendUrl );
  }

  // layer attribution
  const QString aAttribution = attribution();
  if ( !aAttribution.isEmpty() )
  {
    QDomElement layerAttribution = document.createElement( QStringLiteral( "attribution" ) );
    const QDomText layerAttributionText = document.createTextNode( aAttribution );
    layerAttribution.appendChild( layerAttributionText );
    layerAttribution.setAttribute( QStringLiteral( "href" ), attributionUrl() );
    layerElement.appendChild( layerAttribution );
  }

  // timestamp if supported
  if ( timestamp() > QDateTime() )
  {
    QDomElement stamp = document.createElement( QStringLiteral( "timestamp" ) );
    const QDomText stampText = document.createTextNode( timestamp().toString( Qt::ISODate ) );
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

  layerElement.setAttribute( QStringLiteral( "legendPlaceholderImage" ), mLegendPlaceholderImage );

  // now append layer node to map layer node
  return writeXml( layerElement, document, context );
}

void QgsMapLayer::writeCommonStyle( QDomElement &layerElement, QDomDocument &document,
                                    const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  // save categories
  const QMetaEnum metaEnum = QMetaEnum::fromType<QgsMapLayer::StyleCategories>();
  const QString categoriesKeys( metaEnum.valueToKeys( static_cast<int>( categories ) ) );
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
    const auto enumMap = qgsEnumMap<QgsMapLayer::LayerFlag>();
    for ( auto it = enumMap.constBegin(); it != enumMap.constEnd(); ++it )
    {
      const bool flagValue = mFlags.testFlag( it.key() );
      QDomElement flagElem = document.createElement( it.value() );
      flagElem.appendChild( document.createTextNode( QString::number( flagValue ) ) );
      layerFlagsElem.appendChild( flagElem );
    }
    layerElement.appendChild( layerFlagsElem );
  }

  if ( categories.testFlag( Temporal ) )
  {
    if ( QgsMapLayerTemporalProperties *properties = const_cast< QgsMapLayer * >( this )->temporalProperties() )
      properties->writeXml( layerElement, document, context );
  }

  if ( categories.testFlag( Elevation ) )
  {
    if ( QgsMapLayerElevationProperties *properties = const_cast< QgsMapLayer * >( this )->elevationProperties() )
      properties->writeXml( layerElement, document, context );
  }

  if ( categories.testFlag( Notes ) && QgsLayerNotesUtils::layerHasNotes( this ) )
  {
    QDomElement notesElem = document.createElement( QStringLiteral( "userNotes" ) );
    notesElem.setAttribute( QStringLiteral( "value" ), QgsLayerNotesUtils::layerNotes( this ) );
    layerElement.appendChild( notesElem );
  }

  // custom properties
  if ( categories.testFlag( CustomProperties ) )
  {
    writeCustomProperties( layerElement, document );
  }
}


bool QgsMapLayer::writeXml( QDomNode &layer_node, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( layer_node )
  Q_UNUSED( document )
  Q_UNUSED( context )
  // NOP by default; children will over-ride with behavior specific to them

  return true;
}

QString QgsMapLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  return source;
}

QString QgsMapLayer::decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  Q_UNUSED( dataProvider )
  return source;
}

void QgsMapLayer::resolveReferences( QgsProject *project )
{
  emit beforeResolveReferences( project );
  if ( m3DRenderer )
    m3DRenderer->resolveReferences( *project );
}


void QgsMapLayer::readCustomProperties( const QDomNode &layerNode, const QString &keyStartsWith )
{
  const QgsObjectCustomProperties oldKeys = mCustomProperties;

  mCustomProperties.readXml( layerNode, keyStartsWith );

  for ( const QString &key : mCustomProperties.keys() )
  {
    if ( !oldKeys.contains( key ) || mCustomProperties.value( key ) != oldKeys.value( key ) )
    {
      emit customPropertyChanged( key );
    }
  }
}

void QgsMapLayer::writeCustomProperties( QDomNode &layerNode, QDomDocument &doc ) const
{
  mCustomProperties.writeXml( layerNode, doc );
}

void QgsMapLayer::readStyleManager( const QDomNode &layerNode )
{
  const QDomElement styleMgrElem = layerNode.firstChildElement( QStringLiteral( "map-layer-style-manager" ) );
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
  Q_UNUSED( signal )
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
  Q_UNUSED( layers )
  // NOOP
}

void QgsMapLayer::setSubLayerVisibility( const QString &name, bool vis )
{
  Q_UNUSED( name )
  Q_UNUSED( vis )
  // NOOP
}

bool QgsMapLayer::supportsEditing() const
{
  return false;
}

QgsCoordinateReferenceSystem QgsMapLayer::crs() const
{
  return mCRS;
}

void QgsMapLayer::setCrs( const QgsCoordinateReferenceSystem &srs, bool emitSignal )
{
  mCRS = srs;

  if ( mShouldValidateCrs && isSpatial() && !mCRS.isValid() && type() != QgsMapLayerType::AnnotationLayer )
  {
    mCRS.setValidationHint( tr( "Specify CRS for layer %1" ).arg( name() ) );
    mCRS.validate();
  }

  if ( emitSignal )
    emit crsChanged();
}

QgsCoordinateTransformContext QgsMapLayer::transformContext() const
{
  const QgsDataProvider *lDataProvider = dataProvider();
  return lDataProvider ? lDataProvider->transformContext() : QgsCoordinateTransformContext();
}

QString QgsMapLayer::formatLayerName( const QString &name )
{
  QString layerName( name );
  layerName.replace( '_', ' ' );
  layerName = QgsStringUtils::capitalize( layerName, Qgis::Capitalization::ForceFirstLetterToCapital );
  return layerName;
}

QString QgsMapLayer::baseURI( PropertyType type ) const
{
  QString myURI = publicSource();

  // first get base path for delimited text, spatialite and OGR layers,
  // as in these cases URI may contain layer name and/or additional
  // information. This also strips prefix in case if VSIFILE mechanism
  // is used
  if ( providerType() == QLatin1String( "ogr" ) || providerType() == QLatin1String( "delimitedtext" ) ||
       providerType() == QLatin1String( "spatialite" ) )
  {
    QVariantMap components = QgsProviderRegistry::instance()->decodeUri( providerType(), myURI );
    myURI = components["path"].toString();
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
  if ( const QgsProviderMetadata *metadata = QgsProviderRegistry::instance()->providerMetadata( providerType() ) )
  {
    if ( metadata->providerCapabilities() & QgsProviderMetadata::SaveLayerMetadata )
    {
      try
      {
        QString errorMessage;
        resultFlag = QgsProviderRegistry::instance()->saveLayerMetadata( providerType(), mDataSource, mMetadata, errorMessage );
        if ( resultFlag )
          return tr( "Successfully saved default layer metadata" );
        else
          return errorMessage;
      }
      catch ( QgsNotSupportedException &e )
      {
        resultFlag = false;
        return e.what();
      }
    }
  }

  // fallback default metadata saving method, for providers which don't support (or implement) saveLayerMetadata
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
    const QFileInfo project( QgsProject::instance()->fileName() );
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
  const QDomElement myRoot = document.firstChildElement( QStringLiteral( "qgis" ) );
  if ( myRoot.isNull() )
  {
    errorMessage = tr( "Root <qgis> element could not be found" );
    return false;
  }

  return mMetadata.readMetadataXml( myRoot );
}

bool QgsMapLayer::importNamedStyle( QDomDocument &myDocument, QString &myErrorMessage, QgsMapLayer::StyleCategories categories )
{
  const QDomElement myRoot = myDocument.firstChildElement( QStringLiteral( "qgis" ) );
  if ( myRoot.isNull() )
  {
    myErrorMessage = tr( "Root <qgis> element could not be found" );
    return false;
  }

  // get style file version string, if any
  const QgsProjectVersion fileVersion( myRoot.attribute( QStringLiteral( "version" ) ) );
  const QgsProjectVersion thisVersion( Qgis::version() );

  if ( thisVersion > fileVersion )
  {
    QgsProjectFileTransform styleFile( myDocument, fileVersion );
    styleFile.updateRevision( thisVersion );
  }

  // Get source categories
  const QgsMapLayer::StyleCategories sourceCategories = QgsXmlUtils::readFlagAttribute( myRoot, QStringLiteral( "styleCategories" ), QgsMapLayer::AllStyleCategories );

  //Test for matching geometry type on vector layers when applying, if geometry type is given in the style
  if ( ( sourceCategories.testFlag( QgsMapLayer::Symbology ) || sourceCategories.testFlag( QgsMapLayer::Symbology3D ) ) &&
       ( categories.testFlag( QgsMapLayer::Symbology ) || categories.testFlag( QgsMapLayer::Symbology3D ) ) )
  {
    if ( type() == QgsMapLayerType::VectorLayer && !myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).isNull() )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( this );
      const QgsWkbTypes::GeometryType importLayerGeometryType = static_cast<QgsWkbTypes::GeometryType>( myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).text().toInt() );
      if ( importLayerGeometryType != QgsWkbTypes::GeometryType::UnknownGeometry && vl->geometryType() != importLayerGeometryType )
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
  const QDomDocumentType documentType = DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument myDocument( documentType );

  QDomElement myRootNode = myDocument.createElement( QStringLiteral( "qgis" ) );
  myRootNode.setAttribute( QStringLiteral( "version" ), Qgis::version() );
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
  const QDomDocumentType documentType = DomImplementation.createDocumentType( QStringLiteral( "qgis" ), QStringLiteral( "http://mrcc.com/qgis.dtd" ), QStringLiteral( "SYSTEM" ) );
  QDomDocument myDocument( documentType );

  QDomElement myRootNode = myDocument.createElement( QStringLiteral( "qgis" ) );
  myRootNode.setAttribute( QStringLiteral( "version" ), Qgis::version() );
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
  if ( type() == QgsMapLayerType::VectorLayer )
  {
    //Getting the selectionLayer geometry
    const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( this );
    const QString geoType = QString::number( vl->geometryType() );

    //Adding geometryinformation
    QDomElement layerGeometryType = myDocument.createElement( QStringLiteral( "layerGeometryType" ) );
    const QDomText type = myDocument.createTextNode( geoType );

    layerGeometryType.appendChild( type );
    myRootNode.appendChild( layerGeometryType );
  }

  doc = myDocument;
}

QString QgsMapLayer::saveDefaultStyle( bool &resultFlag )
{
  return saveDefaultStyle( resultFlag, AllStyleCategories );
}

QString QgsMapLayer::saveDefaultStyle( bool &resultFlag, StyleCategories categories )
{
  return saveNamedStyle( styleURI(), resultFlag, categories );
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
      const QgsReadWriteContext context;
      exportNamedStyle( myDocument, myErrorMessage, context, categories );
      break;
  }

  const QFileInfo myFileInfo( filename );
  if ( myFileInfo.exists() || filename.endsWith( QgsMapLayer::extensionPropertyType( type ), Qt::CaseInsensitive ) )
  {
    const QFileInfo myDirInfo( myFileInfo.path() );  //excludes file name
    if ( !myDirInfo.isWritable() )
    {
      return tr( "The directory containing your dataset needs to be writable!" );
    }

    // now construct the file name for our .qml or .qmd file
    const QString myFileName = myFileInfo.path() + QDir::separator() + myFileInfo.completeBaseName() + QgsMapLayer::extensionPropertyType( type );

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
    const QString qml = myDocument.toString();

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

  const QDomNode header = myDocument.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
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

  QVariantMap props;
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

  const QFileInfo myFileInfo( filename );
  if ( myFileInfo.exists() || filename.endsWith( QLatin1String( ".sld" ), Qt::CaseInsensitive ) )
  {
    const QFileInfo myDirInfo( myFileInfo.path() );  //excludes file name
    if ( !myDirInfo.isWritable() )
    {
      return tr( "The directory containing your dataset needs to be writable!" );
    }

    // now construct the file name for our .sld style file
    const QString myFileName = myFileInfo.path() + QDir::separator() + myFileInfo.completeBaseName() + ".sld";

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
  const QDomElement myRoot = myDocument.firstChildElement( QStringLiteral( "StyledLayerDescriptor" ) );
  if ( myRoot.isNull() )
  {
    myErrorMessage = QStringLiteral( "Error: StyledLayerDescriptor element not found in %1" ).arg( uri );
    resultFlag = false;
    return myErrorMessage;
  }

  // now get the style node out and pass it over to the layer
  // to deserialise...
  const QDomElement namedLayerElem = myRoot.firstChildElement( QStringLiteral( "NamedLayer" ) );
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
  Q_UNUSED( node )
  Q_UNUSED( errorMessage )
  Q_UNUSED( context )
  Q_UNUSED( categories )
  return false;
}

bool QgsMapLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                              const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  Q_UNUSED( node )
  Q_UNUSED( doc )
  Q_UNUSED( errorMessage )
  Q_UNUSED( context )
  Q_UNUSED( categories )
  return false;
}


void QgsMapLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider,
                                 bool loadDefaultStyleFlag )
{
  const QgsDataProvider::ProviderOptions options;

  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  if ( loadDefaultStyleFlag )
  {
    flags |= QgsDataProvider::FlagLoadDefaultStyle;
  }

  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= QgsDataProvider::FlagTrustDataSource;
  }
  setDataSource( dataSource, baseName, provider, options, flags );
}

void QgsMapLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider,
                                 const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag )
{
  QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags();
  if ( loadDefaultStyleFlag )
  {
    flags |= QgsDataProvider::FlagLoadDefaultStyle;
  }

  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= QgsDataProvider::FlagTrustDataSource;
  }
  setDataSource( dataSource, baseName, provider, options, flags );
}

void QgsMapLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider,
                                 const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{

  if ( ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata ) &&
       !( flags & QgsDataProvider::FlagTrustDataSource ) )
  {
    flags |= QgsDataProvider::FlagTrustDataSource;
  }
  setDataSourcePrivate( dataSource, baseName, provider, options, flags );
  emit dataSourceChanged();
  emit dataChanged();
  triggerRepaint();
}


void QgsMapLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
                                        const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags )
{
  Q_UNUSED( dataSource )
  Q_UNUSED( baseName )
  Q_UNUSED( provider )
  Q_UNUSED( options )
  Q_UNUSED( flags )
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
    const QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "3D Symbology" ) );

    QgsAbstract3DRenderer *r3D = nullptr;
    QDomElement renderer3DElem = layerElement.firstChildElement( QStringLiteral( "renderer-3d" ) );
    if ( !renderer3DElem.isNull() )
    {
      const QString type3D = renderer3DElem.attribute( QStringLiteral( "type" ) );
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
    const QDomElement flagsElem = layerElement.firstChildElement( QStringLiteral( "flags" ) );
    LayerFlags flags = mFlags;
    const auto enumMap = qgsEnumMap<QgsMapLayer::LayerFlag>();
    for ( auto it = enumMap.constBegin(); it != enumMap.constEnd(); ++it )
    {
      const QDomNode flagNode = flagsElem.namedItem( it.value() );
      if ( flagNode.isNull() )
        continue;
      const bool flagValue = flagNode.toElement().text() == "1" ? true : false;
      if ( flags.testFlag( it.key() ) && !flagValue )
        flags &= ~it.key();
      else if ( !flags.testFlag( it.key() ) && flagValue )
        flags |= it.key();
    }
    setFlags( flags );
  }

  if ( categories.testFlag( Temporal ) )
  {
    const QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Temporal" ) );

    if ( QgsMapLayerTemporalProperties *properties = temporalProperties() )
      properties->readXml( layerElement.toElement(), context );
  }

  if ( categories.testFlag( Elevation ) )
  {
    const QgsReadWriteContextCategoryPopper p = context.enterCategory( tr( "Elevation" ) );

    if ( QgsMapLayerElevationProperties *properties = elevationProperties() )
      properties->readXml( layerElement.toElement(), context );
  }

  if ( categories.testFlag( Notes ) )
  {
    const QDomElement notesElem = layerElement.firstChildElement( QStringLiteral( "userNotes" ) );
    if ( !notesElem.isNull() )
    {
      const QString notes = notesElem.attribute( QStringLiteral( "value" ) );
      QgsLayerNotesUtils::setLayerNotes( this, notes );
    }
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
  if ( !mCustomProperties.contains( key ) || mCustomProperties.value( key ) != value )
  {
    mCustomProperties.setValue( key, value );
    emit customPropertyChanged( key );
  }
}

void QgsMapLayer::setCustomProperties( const QgsObjectCustomProperties &properties )
{
  mCustomProperties = properties;
  for ( const QString &key : mCustomProperties.keys() )
  {
    emit customPropertyChanged( key );
  }
}

const QgsObjectCustomProperties &QgsMapLayer::customProperties() const
{
  return mCustomProperties;
}

QVariant QgsMapLayer::customProperty( const QString &value, const QVariant &defaultValue ) const
{
  return mCustomProperties.value( value, defaultValue );
}

void QgsMapLayer::removeCustomProperty( const QString &key )
{

  if ( mCustomProperties.contains( key ) )
  {
    mCustomProperties.remove( key );
    emit customPropertyChanged( key );
  }
}

QgsError QgsMapLayer::error() const
{
  return mError;
}



bool QgsMapLayer::isEditable() const
{
  return false;
}

bool QgsMapLayer::isModified() const
{
  return false;
}

bool QgsMapLayer::isSpatial() const
{
  return true;
}

bool QgsMapLayer::isTemporary() const
{
  // invalid layers are temporary? -- who knows?!
  if ( !isValid() )
    return false;

  if ( mProviderKey == QLatin1String( "memory" ) )
    return true;

  const QVariantMap sourceParts = QgsProviderRegistry::instance()->decodeUri( mProviderKey, mDataSource );
  const QString path = sourceParts.value( QStringLiteral( "path" ) ).toString();
  if ( path.isEmpty() )
    return false;

  // check if layer path is inside one of the standard temporary file locations for this platform
  const QStringList tempPaths = QStandardPaths::standardLocations( QStandardPaths::TempLocation );
  for ( const QString &tempPath : tempPaths )
  {
    if ( path.startsWith( tempPath ) )
      return true;
  }

  return false;
}

void QgsMapLayer::setValid( bool valid )
{
  if ( mValid == valid )
    return;

  mValid = valid;
  emit isValidChanged();
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
    connect( mLegend, &QgsMapLayerLegend::itemsChanged, this, &QgsMapLayer::legendChanged, Qt::UniqueConnection );
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
  emit repaintRequested();
  trigger3DUpdate();
}

QgsAbstract3DRenderer *QgsMapLayer::renderer3D() const
{
  return m3DRenderer;
}

void QgsMapLayer::triggerRepaint( bool deferredUpdate )
{
  if ( mRepaintRequestedFired )
    return;
  mRepaintRequestedFired = true;
  emit repaintRequested( deferredUpdate );
  mRepaintRequestedFired = false;
}

void QgsMapLayer::trigger3DUpdate()
{
  emit request3DUpdate();
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
  if ( !mBlockStyleChangedSignal )
    emit styleChanged();
}

void QgsMapLayer::setExtent( const QgsRectangle &extent )
{
  updateExtent( extent );
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

QString QgsMapLayer::generateId( const QString &layerName )
{
  // Generate the unique ID of this layer
  const QString uuid = QUuid::createUuid().toString();
  // trim { } from uuid
  QString id = layerName + '_' + uuid.mid( 1, uuid.length() - 2 );
  // Tidy the ID up to avoid characters that may cause problems
  // elsewhere (e.g in some parts of XML). Replaces every non-word
  // character (word characters are the alphabet, numbers and
  // underscore) with an underscore.
  // Note that the first backslash in the regular expression is
  // there for the compiler, so the pattern is actually \W
  id.replace( QRegularExpression( "[\\W]" ), QStringLiteral( "_" ) );
  return id;
}

bool QgsMapLayer::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
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
  const auto constODeps = oDeps;
  for ( const QgsMapLayerDependency &dep : constODeps )
  {
    if ( dep.origin() == QgsMapLayerDependency::FromUser )
      deps << dep;
  }

  mDependencies = deps;
  emit dependenciesChanged();
  return true;
}

void QgsMapLayer::setRefreshOnNotifyEnabled( bool enabled )
{
  QgsDataProvider *lDataProvider = dataProvider();

  if ( !lDataProvider )
    return;

  if ( enabled && !isRefreshOnNotifyEnabled() )
  {
    lDataProvider->setListening( enabled );
    connect( lDataProvider, &QgsDataProvider::notify, this, &QgsMapLayer::onNotified );
  }
  else if ( !enabled && isRefreshOnNotifyEnabled() )
  {
    // we don't want to disable provider listening because someone else could need it (e.g. actions)
    disconnect( lDataProvider, &QgsDataProvider::notify, this, &QgsMapLayer::onNotified );
  }
  mIsRefreshOnNofifyEnabled = enabled;
}

QgsProject *QgsMapLayer::project() const
{
  if ( QgsMapLayerStore *store = qobject_cast<QgsMapLayerStore *>( parent() ) )
  {
    return qobject_cast<QgsProject *>( store->parent() );
  }
  return nullptr;
}

void QgsMapLayer::onNotified( const QString &message )
{
  if ( refreshOnNotifyMessage().isEmpty() || refreshOnNotifyMessage() == message )
  {
    triggerRepaint();
    emit dataChanged();
  }
}

QgsRectangle QgsMapLayer::wgs84Extent( bool forceRecalculate ) const
{
  QgsRectangle wgs84Extent;

  if ( ! forceRecalculate && ! mWgs84Extent.isNull() )
  {
    wgs84Extent = mWgs84Extent;
  }
  else if ( ! mExtent.isNull() )
  {
    QgsCoordinateTransform transformer { crs(), QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() ), transformContext() };
    transformer.setBallparkTransformsAreAppropriate( true );
    try
    {
      wgs84Extent = transformer.transformBoundingBox( mExtent );
    }
    catch ( const QgsCsException &cse )
    {
      QgsMessageLog::logMessage( tr( "Error transforming extent: %1" ).arg( cse.what() ) );
      wgs84Extent = QgsRectangle();
    }
  }
  return wgs84Extent;
}

void QgsMapLayer::updateExtent( const QgsRectangle &extent ) const
{
  if ( extent == mExtent )
    return;

  mExtent = extent;

  // do not update the wgs84 extent if we trust layer metadata
  if ( mReadFlags & QgsMapLayer::ReadFlag::FlagTrustLayerMetadata )
    return;

  mWgs84Extent = wgs84Extent( true );
}

void QgsMapLayer::invalidateWgs84Extent()
{
  // do not update the wgs84 extent if we trust layer metadata
  if ( mReadFlags & QgsMapLayer::ReadFlag::FlagTrustLayerMetadata )
    return;

  mWgs84Extent = QgsRectangle();
}

QString QgsMapLayer::generalHtmlMetadata() const
{
  QString metadata = QStringLiteral( "<h1>" ) + tr( "General" ) + QStringLiteral( "</h1>\n<hr>\n" ) + QStringLiteral( "<table class=\"list-view\">\n" );

  // name
  metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + name() + QStringLiteral( "</td></tr>\n" );

  QString path;
  bool isLocalPath = false;
  if ( dataProvider() )
  {
    // local path
    QVariantMap uriComponents = QgsProviderRegistry::instance()->decodeUri( dataProvider()->name(), publicSource() );
    if ( uriComponents.contains( QStringLiteral( "path" ) ) )
    {
      path = uriComponents[QStringLiteral( "path" )].toString();
      QFileInfo fi( path );
      if ( fi.exists() )
      {
        isLocalPath = true;
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Path" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl::fromLocalFile( path ).toString(), QDir::toNativeSeparators( path ) ) ) + QStringLiteral( "</td></tr>\n" );

        QDateTime lastModified = fi.lastModified();
        QString lastModifiedFileName;
        QSet<QString> sidecarFiles = QgsFileUtils::sidecarFilesForPath( path );
        if ( fi.isFile() )
        {
          qint64 fileSize = fi.size();
          if ( !sidecarFiles.isEmpty() )
          {
            lastModifiedFileName = fi.fileName();
            QStringList sidecarFileNames;
            for ( const QString &sidecarFile : sidecarFiles )
            {
              QFileInfo sidecarFi( sidecarFile );
              fileSize += sidecarFi.size();
              if ( sidecarFi.lastModified() > lastModified )
              {
                lastModified = sidecarFi.lastModified();
                lastModifiedFileName = sidecarFi.fileName();
              }
              sidecarFileNames << sidecarFi.fileName();
            }
            metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + ( sidecarFiles.size() > 1 ? tr( "Sidecar files" ) : tr( "Sidecar file" ) ) + QStringLiteral( "</td><td>%1" ).arg( sidecarFileNames.join( QLatin1String( ", " ) ) ) + QStringLiteral( "</td></tr>\n" );
          }
          metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + ( !sidecarFiles.isEmpty() ? tr( "Total size" ) : tr( "Size" ) ) + QStringLiteral( "</td><td>%1" ).arg( QgsFileUtils::representFileSize( fileSize ) ) + QStringLiteral( "</td></tr>\n" );
        }
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Last modified" ) + QStringLiteral( "</td><td>%1" ).arg( QLocale().toString( fi.lastModified() ) ) + ( !lastModifiedFileName.isEmpty() ? QStringLiteral( " (%1)" ).arg( lastModifiedFileName ) : QString() ) + QStringLiteral( "</td></tr>\n" );
      }
    }
    if ( uriComponents.contains( QStringLiteral( "url" ) ) )
    {
      const QString url = uriComponents[QStringLiteral( "url" )].toString();
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "URL" ) + QStringLiteral( "</td><td>%1" ).arg( QStringLiteral( "<a href=\"%1\">%2</a>" ).arg( QUrl( url ).toString(), url ) ) + QStringLiteral( "</td></tr>\n" );
    }
  }

  // data source
  if ( publicSource() != path || !isLocalPath )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Source" ) + QStringLiteral( "</td><td>%1" ).arg( publicSource() != path ? publicSource() : path ) + QStringLiteral( "</td></tr>\n" );

  // provider
  if ( dataProvider() )
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Provider" ) + QStringLiteral( "</td><td>%1" ).arg( dataProvider()->name() ) + QStringLiteral( "</td></tr>\n" );

  metadata += QLatin1String( "</table>\n<br><br>" );
  return metadata;
}

QString QgsMapLayer::crsHtmlMetadata() const
{
  QString metadata = QStringLiteral( "<h1>" ) + tr( "Coordinate Reference System (CRS)" ) + QStringLiteral( "</h1>\n<hr>\n" );
  metadata += QLatin1String( "<table class=\"list-view\">\n" );

  // Identifier
  const QgsCoordinateReferenceSystem c = crs();
  if ( !c.isValid() )
    metadata += QStringLiteral( "<tr><td colspan=\"2\" class=\"highlight\">" ) + tr( "Unknown" ) + QStringLiteral( "</td></tr>\n" );
  else
  {
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + c.userFriendlyIdentifier( QgsCoordinateReferenceSystem::FullString ) + QStringLiteral( "</td></tr>\n" );

    // map units
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Units" ) + QStringLiteral( "</td><td>" )
                + ( c.isGeographic() ? tr( "Geographic (uses latitude and longitude for coordinates)" ) : QgsUnitTypes::toString( c.mapUnits() ) )
                + QStringLiteral( "</td></tr>\n" );


    // operation
    const QgsProjOperation operation = c.operation();
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Method" ) + QStringLiteral( "</td><td>" ) + operation.description() + QStringLiteral( "</td></tr>\n" );

    // celestial body
    try
    {
      const QString celestialBody = c.celestialBodyName();
      if ( !celestialBody.isEmpty() )
      {
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Celestial body" ) + QStringLiteral( "</td><td>" ) + celestialBody + QStringLiteral( "</td></tr>\n" );
      }
    }
    catch ( QgsNotSupportedException & )
    {

    }

    QString accuracyString;
    // dynamic crs with no epoch?
    if ( c.isDynamic() && std::isnan( c.coordinateEpoch() ) )
    {
      accuracyString = tr( "Based on a dynamic CRS, but no coordinate epoch is set. Coordinates are ambiguous and of limited accuracy." );
    }

    // based on datum ensemble?
    try
    {
      const QgsDatumEnsemble ensemble = c.datumEnsemble();
      if ( ensemble.isValid() )
      {
        QString id;
        if ( !ensemble.code().isEmpty() )
          id = QStringLiteral( "<i>%1</i> (%2:%3)" ).arg( ensemble.name(), ensemble.authority(), ensemble.code() );
        else
          id = QStringLiteral( "<i>%</i>”" ).arg( ensemble.name() );

        if ( ensemble.accuracy() > 0 )
        {
          accuracyString = tr( "Based on %1, which has a limited accuracy of <b>at best %2 meters</b>." ).arg( id ).arg( ensemble.accuracy() );
        }
        else
        {
          accuracyString = tr( "Based on %1, which has a limited accuracy." ).arg( id );
        }
      }
    }
    catch ( QgsNotSupportedException & )
    {

    }

    if ( !accuracyString.isEmpty() )
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Accuracy" ) + QStringLiteral( "</td><td>" ) + accuracyString + QStringLiteral( "</td></tr>\n" );
    }

    // static/dynamic
    metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Reference" ) + QStringLiteral( "</td><td>%1</td></tr>\n" ).arg( c.isDynamic() ? tr( "Dynamic (relies on a datum which is not plate-fixed)" ) : tr( "Static (relies on a datum which is plate-fixed)" ) );

    // coordinate epoch
    if ( !std::isnan( c.coordinateEpoch() ) )
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Coordinate epoch" ) + QStringLiteral( "</td><td>%1</td></tr>\n" ).arg( c.coordinateEpoch() );
    }
  }

  metadata += QLatin1String( "</table>\n<br><br>\n" );
  return metadata;
}
