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
#include "qgscoordinatereferencesystemutils.h"
#include "qgsdatasourceuri.h"
#include "qgsfileutils.h"
#include "qgslogger.h"
#include "qgsauthmanager.h"
#include "qgsmaplayer.h"
#include "moc_qgsmaplayer.cpp"
#include "qgsmaplayerlegend.h"
#include "qgsmaplayerstylemanager.h"
#include "qgspathresolver.h"
#include "qgsprojectfiletransform.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsprovidermetadata.h"
#include "qgsrasterlayer.h"
#include "qgsreadwritecontext.h"
#include "qgsrectangle.h"
#include "qgsscaleutils.h"
#include "qgssldexportcontext.h"
#include "qgsvectorlayer.h"
#include "qgsxmlutils.h"
#include "qgsstringutils.h"
#include "qgsmessagelog.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgslayernotesutils.h"
#include "qgsdatums.h"
#include "qgsprojoperation.h"
#include "qgsthreadingutils.h"
#include "qgsunittypes.h"

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

QgsMapLayer::QgsMapLayer( Qgis::LayerType type,
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
  connect( mRefreshTimer, &QTimer::timeout, this, [this]
  {

    switch ( mAutoRefreshMode )
    {
      case Qgis::AutoRefreshMode::Disabled:
        break;
      case Qgis::AutoRefreshMode::RedrawOnly:
        triggerRepaint( true );
        break;
      case Qgis::AutoRefreshMode::ReloadData:
        reload();
        break;
    }
  } );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( QStringLiteral( "Cloning layer '%1'" ).arg( name() ), 3 );
  layer->setBlendMode( blendMode() );

  const auto constStyles = styleManager()->styles();
  for ( const QString &s : constStyles )
  {
    layer->styleManager()->addStyle( s, styleManager()->style( s ) );
  }

  layer->setName( name() );

  if ( layer->dataProvider() && layer->dataProvider()->elevationProperties() )
  {
    if ( layer->dataProvider()->elevationProperties()->containsElevationData() )
      layer->mExtent3D = mExtent3D;
    else
      layer->mExtent2D = mExtent2D;
  }

  layer->setMaximumScale( maximumScale() );
  layer->setMinimumScale( minimumScale() );
  layer->setScaleBasedVisibility( hasScaleBasedVisibility() );
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

Qgis::LayerType QgsMapLayer::type() const
{
  // because QgsVirtualLayerProvider is not anywhere NEAR thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mLayerType;
}

QgsMapLayer::LayerFlags QgsMapLayer::flags() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mFlags;
}

void QgsMapLayer::setFlags( QgsMapLayer::LayerFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( flags == mFlags )
    return;

  mFlags = flags;
  emit flagsChanged();
}

Qgis::MapLayerProperties QgsMapLayer::properties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return Qgis::MapLayerProperties();
}

QString QgsMapLayer::id() const
{
  // because QgsVirtualLayerProvider is not anywhere NEAR thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mID;
}

bool QgsMapLayer::setId( const QString &id )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  if ( qobject_cast< QgsMapLayerStore * >( parent() ) )
  {
    // layer is already registered, cannot change id
    return false;
  }

  if ( id == mID )
    return false;

  mID = id;
  emit idChanged( id );
  return true;
}

void QgsMapLayer::setName( const QString &name )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( name == mLayerName )
    return;

  mLayerName = name;

  emit nameChanged();
}

QString QgsMapLayer::name() const
{
  // because QgsVirtualLayerProvider is not anywhere NEAR thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  QgsDebugMsgLevel( "returning name '" + mLayerName + '\'', 4 );
  return mLayerName;
}

QgsDataProvider *QgsMapLayer::dataProvider()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

const QgsDataProvider *QgsMapLayer::dataProvider() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return nullptr;
}

QgsProviderMetadata *QgsMapLayer::providerMetadata() const
{
  return QgsProviderRegistry::instance()->providerMetadata( providerType() );
}

void QgsMapLayer::setShortName( const QString &shortName )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setShortName( shortName );
}

QString QgsMapLayer::shortName() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mServerProperties->shortName();
}

void QgsMapLayer::setTitle( const QString &title )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setTitle( title );
}

QString QgsMapLayer::title() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->title();
}

void QgsMapLayer::setAbstract( const QString &abstract )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setAbstract( abstract );
}

QString QgsMapLayer::abstract() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->abstract();
}

void QgsMapLayer::setKeywordList( const QString &keywords )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setKeywordList( keywords );
}

QString QgsMapLayer::keywordList() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->keywordList();
}

void QgsMapLayer::setDataUrl( const QString &dataUrl )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setDataUrl( dataUrl );
}

QString QgsMapLayer::dataUrl() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->dataUrl();
}

void QgsMapLayer::setDataUrlFormat( const QString &dataUrlFormat )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setDataUrlFormat( dataUrlFormat );
}

QString QgsMapLayer::dataUrlFormat() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->dataUrlFormat();
}

void QgsMapLayer::setAttribution( const QString &attrib )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setAttribution( attrib );
}

QString QgsMapLayer::attribution() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->attribution();
}

void QgsMapLayer::setAttributionUrl( const QString &attribUrl )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mServerProperties->setAttributionUrl( attribUrl );
}

QString QgsMapLayer::attributionUrl() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mServerProperties->attributionUrl();

}

void QgsMapLayer::setMetadataUrl( const QString &metaUrl )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mServerProperties->metadataUrls().isEmpty() )
  {
    return QString();
  }
  else
  {
    return mServerProperties->metadataUrls().first().format;
  }
}

QString QgsMapLayer::publicSource( bool hidePassword ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // Redo this every time we're asked for it, as we don't know if
  // dataSource has changed.
  QString safeName = QgsDataSourceUri::removePassword( mDataSource, hidePassword );
  return safeName;
}

QString QgsMapLayer::source() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDataSource;
}

QgsRectangle QgsMapLayer::extent() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent2D.isNull() ? mExtent3D.toRectangle() : mExtent2D;
}

QgsBox3D QgsMapLayer::extent3D() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mExtent3D;
}

void QgsMapLayer::setBlendMode( const QPainter::CompositionMode blendMode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mBlendMode == blendMode )
    return;

  mBlendMode = blendMode;
  emit blendModeChanged( blendMode );
  emitStyleChanged();
}

QPainter::CompositionMode QgsMapLayer::blendMode() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mBlendMode;
}

void QgsMapLayer::setOpacity( double opacity )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( qgsDoubleNear( mLayerOpacity, opacity ) )
    return;
  mLayerOpacity = opacity;
  emit opacityChanged( opacity );
  emitStyleChanged();
}

double QgsMapLayer::opacity() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mLayerOpacity;
}

bool QgsMapLayer::readLayerXml( const QDomElement &layerElement, QgsReadWriteContext &context, QgsMapLayer::ReadFlags flags, QgsDataProvider *preloadedProvider )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mPreloadedProvider.reset( preloadedProvider );

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
  const QString dataSourceRaw = mne.text();
  mDataSource = provider.isEmpty() ? dataSourceRaw : QgsProviderRegistry::instance()->relativeToAbsoluteUri( provider, dataSourceRaw, context );

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
  if ( isSpatial() && type() != Qgis::LayerType::Annotation )
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
      const QString newId = mne.text();
      if ( newId != mID )
      {
        mID = mne.text();
        emit idChanged( mID );
      }
    }
  }

  // set name
  mnl = layerElement.namedItem( QStringLiteral( "layername" ) );
  mne = mnl.toElement();

  //name can be translated
  setName( context.projectTranslator()->translate( QStringLiteral( "project:layers:%1" ).arg( layerElement.namedItem( QStringLiteral( "id" ) ).toElement().text() ), mne.text() ) );

  // now let the children grab what they need from the Dom node.
  layerError = !readXml( layerElement, context );

  const QgsCoordinateReferenceSystem oldVerticalCrs = verticalCrs();
  const QgsCoordinateReferenceSystem oldCrs3D = mCrs3D;

  // overwrite CRS with what we read from project file before the raster/vector
  // file reading functions changed it. They will if projections is specified in the file.
  // FIXME: is this necessary? Yes, it is (autumn 2019)
  QgsCoordinateReferenceSystem::setCustomCrsValidation( savedValidation );
  mCRS = savedCRS;

  //vertical CRS
  {
    QgsCoordinateReferenceSystem verticalCrs;
    const QDomNode verticalCrsNode = layerElement.firstChildElement( QStringLiteral( "verticalCrs" ) );
    if ( !verticalCrsNode.isNull() )
    {
      verticalCrs.readXml( verticalCrsNode );
    }
    mVerticalCrs = verticalCrs;
  }
  rebuildCrs3D();

  //legendUrl
  const QDomElement legendUrlElem = layerElement.firstChildElement( QStringLiteral( "legendUrl" ) );
  if ( !legendUrlElem.isNull() )
  {
    mLegendUrl = legendUrlElem.text();
    mLegendUrlFormat = legendUrlElem.attribute( QStringLiteral( "format" ), QString() );
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
  if ( layerElement.hasAttribute( QStringLiteral( "autoRefreshMode" ) ) )
  {
    setAutoRefreshMode( qgsEnumKeyToValue( layerElement.attribute( QStringLiteral( "autoRefreshMode" ) ), Qgis::AutoRefreshMode::Disabled ) );
  }
  else
  {
    setAutoRefreshMode( layerElement.attribute( QStringLiteral( "autoRefreshEnabled" ), QStringLiteral( "0" ) ).toInt() ? Qgis::AutoRefreshMode::RedrawOnly : Qgis::AutoRefreshMode::Disabled );
  }
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

  if ( verticalCrs() != oldVerticalCrs )
    emit verticalCrsChanged();
  if ( mCrs3D != oldCrs3D )
    emit crs3DChanged();

  return ! layerError;
} // bool QgsMapLayer::readLayerXML


bool QgsMapLayer::readXml( const QDomNode &layer_node, QgsReadWriteContext &context )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( layer_node )
  Q_UNUSED( context )
  // NOP by default; children will over-ride with behavior specific to them

  // read Extent
  if ( mReadFlags & QgsMapLayer::FlagReadExtentFromXml )
  {
    const QDomNode extent3DNode = layer_node.namedItem( QStringLiteral( "extent3D" ) );
    if ( extent3DNode.isNull() )
    {
      const QDomNode extentNode = layer_node.namedItem( QStringLiteral( "extent" ) );
      if ( !extentNode.isNull() )
      {
        mExtent2D = QgsXmlUtils::readRectangle( extentNode.toElement() );
      }
    }
    else
    {
      mExtent3D = QgsXmlUtils::readBox3D( extent3DNode.toElement() );
    }
  }

  return true;
} // void QgsMapLayer::readXml


bool QgsMapLayer::writeLayerXml( QDomElement &layerElement, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mExtent3D.isNull() && dataProvider() && dataProvider()->elevationProperties() && dataProvider()->elevationProperties()->containsElevationData() )
    layerElement.appendChild( QgsXmlUtils::writeBox3D( mExtent3D, document ) );
  else if ( !mExtent2D.isNull() )
    layerElement.appendChild( QgsXmlUtils::writeRectangle( mExtent2D, document ) );

  if ( const QgsRectangle lWgs84Extent = wgs84Extent( true ); !lWgs84Extent.isNull() )
  {
    layerElement.appendChild( QgsXmlUtils::writeRectangle( lWgs84Extent, document, QStringLiteral( "wgs84extent" ) ) );
  }

  layerElement.setAttribute( QStringLiteral( "autoRefreshTime" ), QString::number( mRefreshTimer->interval() ) );
  layerElement.setAttribute( QStringLiteral( "autoRefreshMode" ), qgsEnumValueToKey( mAutoRefreshMode ) );
  layerElement.setAttribute( QStringLiteral( "refreshOnNotifyEnabled" ),  mIsRefreshOnNofifyEnabled ? 1 : 0 );
  layerElement.setAttribute( QStringLiteral( "refreshOnNotifyMessage" ),  mRefreshOnNofifyMessage );

  // ID
  QDomElement layerId = document.createElement( QStringLiteral( "id" ) );
  const QDomText layerIdText = document.createTextNode( id() );
  layerId.appendChild( layerIdText );

  layerElement.appendChild( layerId );

  if ( mVerticalCrs.isValid() )
  {
    QDomElement verticalSrsNode = document.createElement( QStringLiteral( "verticalCrs" ) );
    mVerticalCrs.writeXml( verticalSrsNode, document );
    layerElement.appendChild( verticalSrsNode );
  }

  // data source
  QDomElement dataSource = document.createElement( QStringLiteral( "datasource" ) );
  const QgsDataProvider *provider = dataProvider();
  const QString providerKey = provider ? provider->name() : QString();
  const QString srcRaw = encodedSource( source(), context );
  const QString src = providerKey.isEmpty() ? srcRaw : QgsProviderRegistry::instance()->absoluteToRelativeUri( providerKey, srcRaw, context );
  const QDomText dataSourceText = document.createTextNode( src );
  dataSource.appendChild( dataSourceText );
  layerElement.appendChild( dataSource );

  // layer name
  QDomElement layerName = document.createElement( QStringLiteral( "layername" ) );
  const QDomText layerNameText = document.createTextNode( name() );
  layerName.appendChild( layerNameText );
  layerElement.appendChild( layerName );

  // layer short name

  // TODO -- ideally this would be in QgsMapLayerServerProperties::writeXml, but that's currently
  // only called for SOME map layer subclasses!
  if ( !mServerProperties->shortName().isEmpty() )
  {
    QDomElement layerShortName = document.createElement( QStringLiteral( "shortname" ) );
    const QDomText layerShortNameText = document.createTextNode( mServerProperties->shortName() );
    layerShortName.appendChild( layerShortNameText );
    layerElement.appendChild( layerShortName );
  }

  // layer title
  if ( !mServerProperties->title().isEmpty() )
  {
    QDomElement layerTitle = document.createElement( QStringLiteral( "title" ) );
    const QDomText layerTitleText = document.createTextNode( mServerProperties->title() );
    layerTitle.appendChild( layerTitleText );

    if ( mServerProperties->title() != mServerProperties->wfsTitle() )
    {
      layerTitle.setAttribute( "wfs",  mServerProperties->wfsTitle() );
    }

    layerElement.appendChild( layerTitle );
  }

  // layer abstract
  if ( !mServerProperties->abstract().isEmpty() )
  {
    QDomElement layerAbstract = document.createElement( QStringLiteral( "abstract" ) );
    const QDomText layerAbstractText = document.createTextNode( mServerProperties->abstract() );
    layerAbstract.appendChild( layerAbstractText );
    layerElement.appendChild( layerAbstract );
  }

  // layer keyword list
  const QStringList keywordStringList = mServerProperties->keywordList().split( ',' );
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
  const QString aDataUrl = mServerProperties->dataUrl();
  if ( !aDataUrl.isEmpty() )
  {
    QDomElement layerDataUrl = document.createElement( QStringLiteral( "dataUrl" ) );
    const QDomText layerDataUrlText = document.createTextNode( aDataUrl );
    layerDataUrl.appendChild( layerDataUrlText );
    layerDataUrl.setAttribute( QStringLiteral( "format" ), mServerProperties->dataUrlFormat() );
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
  const QString aAttribution = mServerProperties->attribution();
  if ( !aAttribution.isEmpty() )
  {
    QDomElement layerAttribution = document.createElement( QStringLiteral( "attribution" ) );
    const QDomText layerAttributionText = document.createTextNode( aAttribution );
    layerAttribution.appendChild( layerAttributionText );
    layerAttribution.setAttribute( QStringLiteral( "href" ), mServerProperties->attributionUrl() );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
    layerElement.setAttribute( QStringLiteral( "autoRefreshMode" ), qgsEnumValueToKey( mAutoRefreshMode ) );
    layerElement.setAttribute( QStringLiteral( "autoRefreshTime" ), QString::number( autoRefreshInterval() ) );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( layer_node )
  Q_UNUSED( document )
  Q_UNUSED( context )
  // NOP by default; children will over-ride with behavior specific to them

  return true;
}

QString QgsMapLayer::encodedSource( const QString &source, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( context )
  return source;
}

QString QgsMapLayer::decodedSource( const QString &source, const QString &dataProvider, const QgsReadWriteContext &context ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( context )
  Q_UNUSED( dataProvider )
  return source;
}

void QgsMapLayer::resolveReferences( QgsProject *project )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  emit beforeResolveReferences( project );
  if ( m3DRenderer )
    m3DRenderer->resolveReferences( *project );
}


void QgsMapLayer::readCustomProperties( const QDomNode &layerNode, const QString &keyStartsWith )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mCustomProperties.writeXml( layerNode, doc );
}

void QgsMapLayer::readStyleManager( const QDomNode &layerNode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QDomElement styleMgrElem = layerNode.firstChildElement( QStringLiteral( "map-layer-style-manager" ) );
  if ( !styleMgrElem.isNull() )
    mStyleManager->readXml( styleMgrElem );
  else
    mStyleManager->reset();
}

void QgsMapLayer::writeStyleManager( QDomNode &layerNode, QDomDocument &doc ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mStyleManager )
  {
    QDomElement styleMgrElem = doc.createElement( QStringLiteral( "map-layer-style-manager" ) );
    mStyleManager->writeXml( styleMgrElem );
    layerNode.appendChild( styleMgrElem );
  }
}

QString QgsMapLayer::mapTipTemplate() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMapTipTemplate;
}

void QgsMapLayer::setMapTipTemplate( const QString &mapTip )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mMapTipTemplate == mapTip )
    return;

  mMapTipTemplate = mapTip;
  emit mapTipTemplateChanged();
}

void QgsMapLayer::setMapTipsEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mMapTipsEnabled == enabled )
    return;

  mMapTipsEnabled = enabled;
  emit mapTipsEnabledChanged();
}

bool QgsMapLayer::mapTipsEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMapTipsEnabled;
}

Qgis::DataProviderReadFlags QgsMapLayer::providerReadFlags( const QDomNode &layerNode, QgsMapLayer::ReadFlags layerReadFlags )
{
  Qgis::DataProviderReadFlags flags;
  if ( layerReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= Qgis::DataProviderReadFlag::TrustDataSource;
  }
  if ( layerReadFlags & QgsMapLayer::FlagForceReadOnly )
  {
    flags |= Qgis::DataProviderReadFlag::ForceReadOnly;
  }

  if ( layerReadFlags & QgsMapLayer::FlagReadExtentFromXml )
  {
    const QDomNode extent3DNode = layerNode.namedItem( QStringLiteral( "extent3D" ) );
    if ( extent3DNode.isNull() )
    {
      const QDomNode extentNode = layerNode.namedItem( QStringLiteral( "extent" ) );
      if ( !extentNode.isNull() )
      {
        flags |= Qgis::DataProviderReadFlag::SkipGetExtent;
      }
    }
    else
    {
      flags |= Qgis::DataProviderReadFlag::SkipGetExtent;
    }
  }

  return flags;
}

bool QgsMapLayer::isValid() const
{
  // because QgsVirtualLayerProvider is not anywhere NEAR thread safe:
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

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
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  // mMinScale (denominator!) is inclusive ( >= --> In range )
  // mMaxScale (denominator!) is exclusive ( < --> In range )
  return !mScaleBasedVisibility
         || ( ( mMinScale == 0 || !QgsScaleUtils::lessThanMaximumScale( scale, mMinScale ) )
              && ( mMaxScale == 0 || !QgsScaleUtils::equalToOrGreaterThanMinimumScale( scale, mMaxScale ) ) );
}

bool QgsMapLayer::hasScaleBasedVisibility() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mScaleBasedVisibility;
}

bool QgsMapLayer::hasAutoRefreshEnabled() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAutoRefreshMode != Qgis::AutoRefreshMode::Disabled;;
}

Qgis::AutoRefreshMode QgsMapLayer::autoRefreshMode() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mAutoRefreshMode;
}

int QgsMapLayer::autoRefreshInterval() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mRefreshTimer->interval();
}

void QgsMapLayer::setAutoRefreshInterval( int interval )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( interval <= 0 )
  {
    mRefreshTimer->stop();
    mRefreshTimer->setInterval( 0 );
    setAutoRefreshMode( Qgis::AutoRefreshMode::Disabled );
  }
  else
  {
    mRefreshTimer->setInterval( interval );
  }
  emit autoRefreshIntervalChanged( mRefreshTimer->isActive() ? mRefreshTimer->interval() : 0 );
}

void QgsMapLayer::setAutoRefreshEnabled( bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  setAutoRefreshMode( enabled ? Qgis::AutoRefreshMode::RedrawOnly : Qgis::AutoRefreshMode::Disabled );
}

void QgsMapLayer::setAutoRefreshMode( Qgis::AutoRefreshMode mode )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mode == mAutoRefreshMode )
    return;

  mAutoRefreshMode = mode;
  switch ( mAutoRefreshMode )
  {
    case Qgis::AutoRefreshMode::Disabled:
      mRefreshTimer->stop();
      break;

    case Qgis::AutoRefreshMode::RedrawOnly:
    case Qgis::AutoRefreshMode::ReloadData:
      if ( mRefreshTimer->interval() > 0 )
        mRefreshTimer->start();
      break;
  }

  emit autoRefreshIntervalChanged( mRefreshTimer->isActive() ? mRefreshTimer->interval() : 0 );
}

const QgsLayerMetadata &QgsMapLayer::metadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMetadata;
}

void QgsMapLayer::setMaximumScale( double scale )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mMinScale = scale;
}

double QgsMapLayer::maximumScale() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMinScale;
}

void QgsMapLayer::setMinimumScale( double scale )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mMaxScale = scale;
}

void QgsMapLayer::setScaleBasedVisibility( const bool enabled )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mScaleBasedVisibility = enabled;
}

double QgsMapLayer::minimumScale() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mMaxScale;
}

QStringList QgsMapLayer::subLayers() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QStringList();
}

void QgsMapLayer::setLayerOrder( const QStringList &layers )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( layers )
}

void QgsMapLayer::setSubLayerVisibility( const QString &name, bool vis )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( name )
  Q_UNUSED( vis )
}

bool QgsMapLayer::supportsEditing() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

QgsCoordinateReferenceSystem QgsMapLayer::crs() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mCRS;
}

QgsCoordinateReferenceSystem QgsMapLayer::verticalCrs() const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  switch ( mCRS.type() )
  {
    case Qgis::CrsType::Vertical: // would hope this never happens!
      QgsDebugError( QStringLiteral( "Layer has a vertical CRS set as the horizontal CRS!" ) );
      return mCRS;

    case Qgis::CrsType::Compound:
      return mCRS.verticalCrs();

    case Qgis::CrsType::Unknown:
    case Qgis::CrsType::Geodetic:
    case Qgis::CrsType::Geocentric:
    case Qgis::CrsType::Geographic2d:
    case Qgis::CrsType::Geographic3d:
    case Qgis::CrsType::Projected:
    case Qgis::CrsType::Temporal:
    case Qgis::CrsType::Engineering:
    case Qgis::CrsType::Bound:
    case Qgis::CrsType::Other:
    case Qgis::CrsType::DerivedProjected:
      break;
  }
  return mVerticalCrs;
}

QgsCoordinateReferenceSystem QgsMapLayer::crs3D() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCrs3D.isValid() ? mCrs3D : mCRS;
}

void QgsMapLayer::setCrs( const QgsCoordinateReferenceSystem &srs, bool emitSignal )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  const bool needToValidateCrs = mShouldValidateCrs && isSpatial() && !srs.isValid() && type() != Qgis::LayerType::Annotation;

  if ( mCRS == srs && !needToValidateCrs )
    return;

  const QgsCoordinateReferenceSystem oldVerticalCrs = verticalCrs();
  const QgsCoordinateReferenceSystem oldCrs3D = mCrs3D;
  const QgsCoordinateReferenceSystem oldCrs = mCRS;

  mCRS = srs;

  if ( needToValidateCrs )
  {
    mCRS.setValidationHint( tr( "Specify CRS for layer %1" ).arg( name() ) );
    mCRS.validate();
  }

  rebuildCrs3D();

  if ( emitSignal && mCRS != oldCrs )
    emit crsChanged();

  // Did vertical crs also change as a result of this? If so, emit signal
  if ( oldVerticalCrs != verticalCrs() )
    emit verticalCrsChanged();
  if ( oldCrs3D != mCrs3D )
    emit crs3DChanged();
}

bool QgsMapLayer::setVerticalCrs( const QgsCoordinateReferenceSystem &crs, QString *errorMessage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  bool res = true;
  if ( crs.isValid() )
  {
    // validate that passed crs is a vertical crs
    switch ( crs.type() )
    {
      case Qgis::CrsType::Vertical:
        break;

      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Compound:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geocentric:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Geographic3d:
      case Qgis::CrsType::Projected:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::DerivedProjected:
        if ( errorMessage )
          *errorMessage = QObject::tr( "Specified CRS is a %1 CRS, not a Vertical CRS" ).arg( qgsEnumValueToKey( crs.type() ) );
        return false;
    }
  }

  if ( crs != mVerticalCrs )
  {
    const QgsCoordinateReferenceSystem oldVerticalCrs = verticalCrs();
    const QgsCoordinateReferenceSystem oldCrs3D = mCrs3D;

    switch ( mCRS.type() )
    {
      case Qgis::CrsType::Compound:
        if ( crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Layer CRS is a Compound CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Geographic3d:
        if ( crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Layer CRS is a Geographic 3D CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Geocentric:
        if ( crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Layer CRS is a Geocentric CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Projected:
        if ( mCRS.hasVerticalAxis() && crs != oldVerticalCrs )
        {
          if ( errorMessage )
            *errorMessage = QObject::tr( "Layer CRS is a Projected 3D CRS, specified Vertical CRS will be ignored" );
          return false;
        }
        break;

      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::Vertical:
      case Qgis::CrsType::DerivedProjected:
        break;
    }

    mVerticalCrs = crs;
    res = rebuildCrs3D( errorMessage );

    // only emit signal if vertical crs was actually changed, so eg if mCrs is compound
    // then we haven't actually changed the vertical crs by this call!
    if ( verticalCrs() != oldVerticalCrs )
      emit verticalCrsChanged();
    if ( mCrs3D != oldCrs3D )
      emit crs3DChanged();
  }
  return res;
}

QgsCoordinateTransformContext QgsMapLayer::transformContext() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString myURI = publicSource();

  // first get base path for delimited text, spatialite and OGR layers,
  // as in these cases URI may contain layer name and/or additional
  // information. This also strips prefix in case if VSIFILE mechanism
  // is used
  if ( providerType() == QLatin1String( "ogr" ) || providerType() == QLatin1String( "delimitedtext" )
       || providerType() == QLatin1String( "gdal" ) || providerType() == QLatin1String( "spatialite" ) )
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return baseURI( PropertyType::Metadata );
}

QString QgsMapLayer::saveDefaultMetadata( bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadNamedMetadata( metadataUri(), resultFlag );
}

QString QgsMapLayer::styleURI() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return baseURI( PropertyType::Style );
}

QString QgsMapLayer::loadDefaultStyle( bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadNamedStyle( styleURI(), resultFlag, QgsMapLayer::AllStyleCategories, Qgis::LoadStyleFlag::IgnoreMissingStyleErrors );
}

bool QgsMapLayer::loadNamedMetadataFromDatabase( const QString &db, const QString &uri, QString &qmd )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadNamedPropertyFromDatabase( db, uri, qmd, PropertyType::Metadata );
}

bool QgsMapLayer::loadNamedStyleFromDatabase( const QString &db, const QString &uri, QString &qml )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadNamedPropertyFromDatabase( db, uri, qml, PropertyType::Style );
}

bool QgsMapLayer::loadNamedPropertyFromDatabase( const QString &db, const QString &uri, QString &xml, QgsMapLayer::PropertyType type )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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


QString QgsMapLayer::loadNamedStyle( const QString &uri, bool &resultFlag, QgsMapLayer::StyleCategories categories, Qgis::LoadStyleFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return loadNamedStyle( uri, resultFlag, false, categories, flags );
}

QString QgsMapLayer::loadNamedProperty( const QString &uri, QgsMapLayer::PropertyType type, bool &namedPropertyExists, bool &propertySuccessfullyLoaded, StyleCategories categories, Qgis::LoadStyleFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsDebugMsgLevel( QStringLiteral( "uri = %1 myURI = %2" ).arg( uri, publicSource() ), 4 );

  namedPropertyExists = false;
  propertySuccessfullyLoaded = false;
  if ( uri.isEmpty() )
    return QString();

  QDomDocument myDocument( QStringLiteral( "qgis" ) );

  // location of problem associated with errorMsg
  int line, column;
  QString myErrorMessage;

  QFile myFile( uri );
  if ( myFile.open( QFile::ReadOnly ) )
  {
    QgsDebugMsgLevel( QStringLiteral( "file found %1" ).arg( uri ), 2 );
    namedPropertyExists = true;

    // read file
    propertySuccessfullyLoaded = myDocument.setContent( &myFile, &myErrorMessage, &line, &column );
    if ( !propertySuccessfullyLoaded )
      myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
    myFile.close();
  }
  else
  {
    const QFileInfo project( QgsProject::instance()->fileName() ); // skip-keyword-check
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
          namedPropertyExists = true;
          propertySuccessfullyLoaded = myDocument.setContent( xml, &myErrorMessage, &line, &column );
          if ( !propertySuccessfullyLoaded )
          {
            myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
          }
        }
        else
        {
          if ( !flags.testFlag( Qgis::LoadStyleFlag::IgnoreMissingStyleErrors ) )
          {
            myErrorMessage = tr( "Style not found in database" );
          }
        }
        break;
      }
      case QgsMapLayer::Metadata:
      {
        if ( loadNamedMetadataFromDatabase( QDir( QgsApplication::qgisSettingsDirPath() ).absoluteFilePath( QStringLiteral( "qgis.qmldb" ) ), uri, xml ) ||
             ( project.exists() && loadNamedMetadataFromDatabase( project.absoluteDir().absoluteFilePath( project.baseName() + ".qmldb" ), uri, xml ) ) ||
             loadNamedMetadataFromDatabase( QDir( QgsApplication::pkgDataPath() ).absoluteFilePath( QStringLiteral( "resources/qgis.qmldb" ) ), uri, xml ) )
        {
          namedPropertyExists = true;
          propertySuccessfullyLoaded = myDocument.setContent( xml, &myErrorMessage, &line, &column );
          if ( !propertySuccessfullyLoaded )
          {
            myErrorMessage = tr( "%1 at line %2 column %3" ).arg( myErrorMessage ).arg( line ).arg( column );
          }
        }
        else
        {
          myErrorMessage = tr( "Metadata not found in database" );
        }
        break;
      }
    }
  }

  if ( !propertySuccessfullyLoaded )
  {
    return myErrorMessage;
  }

  switch ( type )
  {
    case QgsMapLayer::Style:
      propertySuccessfullyLoaded = importNamedStyle( myDocument, myErrorMessage, categories );
      if ( !propertySuccessfullyLoaded )
        myErrorMessage = tr( "Loading style file %1 failed because:\n%2" ).arg( uri, myErrorMessage );
      break;
    case QgsMapLayer::Metadata:
      propertySuccessfullyLoaded = importNamedMetadata( myDocument, myErrorMessage );
      if ( !propertySuccessfullyLoaded )
        myErrorMessage = tr( "Loading metadata file %1 failed because:\n%2" ).arg( uri, myErrorMessage );
      break;
  }
  return myErrorMessage;
}

bool QgsMapLayer::importNamedMetadata( QDomDocument &document, QString &errorMessage )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
    if ( type() == Qgis::LayerType::Vector && !myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).isNull() )
    {
      QgsVectorLayer *vl = qobject_cast<QgsVectorLayer *>( this );
      const Qgis::GeometryType importLayerGeometryType = static_cast<Qgis::GeometryType>( myRoot.firstChildElement( QStringLiteral( "layerGeometryType" ) ).text().toInt() );
      if ( importLayerGeometryType != Qgis::GeometryType::Unknown && vl->geometryType() != importLayerGeometryType )
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  if ( type() == Qgis::LayerType::Vector )
  {
    //Getting the selectionLayer geometry
    const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( this );
    const QString geoType = QString::number( static_cast<int>( vl->geometryType() ) );

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return saveDefaultStyle( resultFlag, AllStyleCategories );
}

QString QgsMapLayer::saveDefaultStyle( bool &resultFlag, StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return saveNamedStyle( styleURI(), resultFlag, categories );
}

QString QgsMapLayer::saveNamedMetadata( const QString &uri, bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return saveNamedProperty( uri, QgsMapLayer::Metadata, resultFlag );
}

QString QgsMapLayer::loadNamedMetadata( const QString &uri, bool &resultFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  bool metadataExists = false;
  bool metadataSuccessfullyLoaded = false;
  const QString message = loadNamedProperty( uri, QgsMapLayer::Metadata, metadataExists, metadataSuccessfullyLoaded );

  // TODO QGIS 4.0 -- fix API for loadNamedMetadata so we can return metadataExists too
  ( void )metadataExists;
  resultFlag = metadataSuccessfullyLoaded;
  return message;
}

QString QgsMapLayer::saveNamedProperty( const QString &uri, QgsMapLayer::PropertyType type, bool &resultFlag, StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
      resultFlag = false;
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return saveNamedProperty( uri, QgsMapLayer::Style, resultFlag, categories );
}

void QgsMapLayer::exportSldStyle( QDomDocument &doc, QString &errorMsg ) const
{

  return exportSldStyleV2( doc, errorMsg, QgsSldExportContext() );
}

void QgsMapLayer::exportSldStyleV2( QDomDocument &doc, QString &errorMsg, const QgsSldExportContext &exportContext ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

  QVariant context;
  context.setValue( exportContext );

  props[ QStringLiteral( "SldExportContext" ) ] = context;

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
  QgsSldExportContext context;
  context.setExportFilePath( uri );
  return saveSldStyleV2( resultFlag, context );
}

QString QgsMapLayer::saveSldStyleV2( bool &resultFlag, const QgsSldExportContext &exportContext ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsMapLayer *mlayer = qobject_cast<const QgsMapLayer *>( this );

  const QString uri { exportContext.exportFilePath() };

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
      resultFlag = false;
      return tr( "The directory containing your dataset needs to be writable!" );
    }

    // now construct the file name for our .sld style file
    const QString myFileName = myFileInfo.path() + QDir::separator() + myFileInfo.completeBaseName() + ".sld";

    QString errorMsg;
    QDomDocument myDocument;

    QgsSldExportContext context { exportContext };
    context.setExportFilePath( myFileName );

    mlayer->exportSldStyleV2( myDocument, errorMsg, context );

    if ( !errorMsg.isNull() )
    {
      resultFlag = false;
      return errorMsg;
    }

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( node )
  Q_UNUSED( errorMessage )
  Q_UNUSED( context )
  Q_UNUSED( categories )
  return false;
}

bool QgsMapLayer::writeStyle( QDomNode &node, QDomDocument &doc, QString &errorMessage,
                              const QgsReadWriteContext &context, QgsMapLayer::StyleCategories categories ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  const QgsDataProvider::ProviderOptions options;

  Qgis::DataProviderReadFlags flags;
  if ( loadDefaultStyleFlag )
  {
    flags |= Qgis::DataProviderReadFlag::LoadDefaultStyle;
  }

  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= Qgis::DataProviderReadFlag::TrustDataSource;
  }
  setDataSource( dataSource,
                 baseName.isEmpty() ? mLayerName : baseName,
                 provider.isEmpty() ? mProviderKey : provider,
                 options, flags );
}

void QgsMapLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider,
                                 const QgsDataProvider::ProviderOptions &options, bool loadDefaultStyleFlag )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Qgis::DataProviderReadFlags flags;
  if ( loadDefaultStyleFlag )
  {
    flags |= Qgis::DataProviderReadFlag::LoadDefaultStyle;
  }

  if ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata )
  {
    flags |= Qgis::DataProviderReadFlag::TrustDataSource;
  }
  setDataSource( dataSource, baseName, provider, options, flags );
}

void QgsMapLayer::setDataSource( const QString &dataSource, const QString &baseName, const QString &provider,
                                 const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( ( mReadFlags & QgsMapLayer::FlagTrustLayerMetadata ) &&
       !( flags & Qgis::DataProviderReadFlag::TrustDataSource ) )
  {
    flags |= Qgis::DataProviderReadFlag::TrustDataSource;
  }
  setDataSourcePrivate( dataSource, baseName, provider, options, flags );
  emit dataSourceChanged();
  emit dataChanged();
  triggerRepaint();
}


void QgsMapLayer::setDataSourcePrivate( const QString &dataSource, const QString &baseName, const QString &provider,
                                        const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  Q_UNUSED( dataSource )
  Q_UNUSED( baseName )
  Q_UNUSED( provider )
  Q_UNUSED( options )
  Q_UNUSED( flags )
}


QString QgsMapLayer::providerType() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mProviderKey;
}

void QgsMapLayer::readCommonStyle( const QDomElement &layerElement, const QgsReadWriteContext &context,
                                   QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
    if ( layerElement.hasAttribute( QStringLiteral( "autoRefreshMode" ) ) )
    {
      setAutoRefreshMode( qgsEnumKeyToValue( layerElement.attribute( QStringLiteral( "autoRefreshMode" ) ), Qgis::AutoRefreshMode::Disabled ) );
      setAutoRefreshInterval( layerElement.attribute( QStringLiteral( "autoRefreshTime" ) ).toInt() );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mUndoStack;
}

QUndoStack *QgsMapLayer::undoStackStyles()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mUndoStackStyles;
}

QStringList QgsMapLayer::customPropertyKeys() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCustomProperties.keys();
}

void QgsMapLayer::setCustomProperty( const QString &key, const QVariant &value )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mCustomProperties.contains( key ) || mCustomProperties.value( key ) != value )
  {
    mCustomProperties.setValue( key, value );
    emit customPropertyChanged( key );
  }
}

void QgsMapLayer::setCustomProperties( const QgsObjectCustomProperties &properties )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mCustomProperties = properties;
  for ( const QString &key : mCustomProperties.keys() )
  {
    emit customPropertyChanged( key );
  }
}

const QgsObjectCustomProperties &QgsMapLayer::customProperties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mCustomProperties;
}

QVariant QgsMapLayer::customProperty( const QString &value, const QVariant &defaultValue ) const
{
  // non fatal for now -- the "rasterize" processing algorithm is not thread safe and calls this
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS_NON_FATAL

  return mCustomProperties.value( value, defaultValue );
}

void QgsMapLayer::removeCustomProperty( const QString &key )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mCustomProperties.contains( key ) )
  {
    mCustomProperties.remove( key );
    emit customPropertyChanged( key );
  }
}

int QgsMapLayer::listStylesInDatabase( QStringList &ids, QStringList &names, QStringList &descriptions, QString &msgError )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->listStyles( mProviderKey, mDataSource, ids, names, descriptions, msgError );
}

QString QgsMapLayer::getStyleFromDatabase( const QString &styleId, QString &msgError )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->getStyleById( mProviderKey, mDataSource, styleId, msgError );
}

bool QgsMapLayer::deleteStyleFromDatabase( const QString &styleId, QString &msgError )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QgsProviderRegistry::instance()->deleteStyleById( mProviderKey, mDataSource, styleId, msgError );
}

void QgsMapLayer::saveStyleToDatabase( const QString &name, const QString &description,
                                       bool useAsDefault, const QString &uiFileContent, QString &msgError, QgsMapLayer::StyleCategories categories )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString sldStyle, qmlStyle;
  QDomDocument qmlDocument, sldDocument;
  QgsReadWriteContext context;
  exportNamedStyle( qmlDocument, msgError, context, categories );
  if ( !msgError.isNull() )
  {
    return;
  }
  qmlStyle = qmlDocument.toString();

  this->exportSldStyle( sldDocument, msgError );
  if ( !msgError.isNull() )
  {
    return;
  }
  sldStyle = sldDocument.toString();

  QgsProviderRegistry::instance()->saveStyle( mProviderKey,
      mDataSource, qmlStyle, sldStyle, name,
      description, uiFileContent, useAsDefault, msgError );
}

QString QgsMapLayer::loadNamedStyle( const QString &theURI, bool &resultFlag, bool loadFromLocalDB, QgsMapLayer::StyleCategories categories, Qgis::LoadStyleFlags flags )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QString returnMessage;
  QString qml, errorMsg;
  QString styleName;
  if ( !loadFromLocalDB && dataProvider() && dataProvider()->styleStorageCapabilities().testFlag( Qgis::ProviderStyleStorageCapability::LoadFromDatabase ) )
  {
    qml = QgsProviderRegistry::instance()->loadStoredStyle( mProviderKey, mDataSource, styleName, errorMsg );
  }

  // Style was successfully loaded from provider storage
  if ( !qml.isEmpty() )
  {
    QDomDocument myDocument( QStringLiteral( "qgis" ) );
    myDocument.setContent( qml );
    resultFlag = importNamedStyle( myDocument, errorMsg );
    returnMessage = QObject::tr( "Loaded from Provider" );
  }
  else
  {
    QGIS_PROTECT_QOBJECT_THREAD_ACCESS

    bool styleExists = false;
    bool styleSuccessfullyLoaded = false;

    returnMessage = loadNamedProperty( theURI, PropertyType::Style, styleExists, styleSuccessfullyLoaded, categories, flags );

    // TODO QGIS 4.0 -- fix API for loadNamedStyle so we can return styleExists too
    ( void )styleExists;
    resultFlag = styleSuccessfullyLoaded;
  }

  if ( ! styleName.isEmpty() )
  {
    styleManager()->renameStyle( styleManager()->currentStyle(), styleName );
  }

  if ( resultFlag )
    emit styleLoaded( categories );

  return returnMessage;
}

QgsError QgsMapLayer::error() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mError;
}

bool QgsMapLayer::isEditable() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

bool QgsMapLayer::isModified() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return false;
}

bool QgsMapLayer::isSpatial() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

bool QgsMapLayer::isTemporary() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mValid == valid )
    return;

  mValid = valid;
  emit isValidChanged();
}

void QgsMapLayer::setLegend( QgsMapLayerLegend *legend )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mLegend;
}

QgsMapLayerStyleManager *QgsMapLayer::styleManager() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mStyleManager;
}

void QgsMapLayer::setRenderer3D( QgsAbstract3DRenderer *renderer )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return m3DRenderer;
}

void QgsMapLayer::triggerRepaint( bool deferredUpdate )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( mRepaintRequestedFired )
    return;
  mRepaintRequestedFired = true;
  emit repaintRequested( deferredUpdate );
  mRepaintRequestedFired = false;
}

void QgsMapLayer::trigger3DUpdate()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  emit request3DUpdate();
}

void QgsMapLayer::setMetadata( const QgsLayerMetadata &metadata )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mMetadata = metadata;
//  mMetadata.saveToLayer( this );
  emit metadataChanged();
}

QString QgsMapLayer::htmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QString();
}

QDateTime QgsMapLayer::timestamp() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return QDateTime();
}

void QgsMapLayer::emitStyleChanged()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( !mBlockStyleChangedSignal )
    emit styleChanged();
}

void QgsMapLayer::setExtent( const QgsRectangle &extent )
{
  updateExtent( extent );
}

void QgsMapLayer::setExtent3D( const QgsBox3D &extent )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  updateExtent( extent );
}

bool QgsMapLayer::isReadOnly() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

QString QgsMapLayer::originalXmlProperties() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mOriginalXmlProperties;
}

void QgsMapLayer::setOriginalXmlProperties( const QString &originalXmlProperties )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  const thread_local QRegularExpression idRx( QStringLiteral( "[\\W]" ) );
  id.replace( idRx, QStringLiteral( "_" ) );
  return id;
}

bool QgsMapLayer::accept( QgsStyleEntityVisitorInterface * ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return true;
}

bool QgsMapLayer::hasMapTips() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mapTipsEnabled() && !mMapTipTemplate.isEmpty();
}

void QgsMapLayer::setProviderType( const QString &providerType )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  mProviderKey = providerType;
}

QSet<QgsMapLayerDependency> QgsMapLayer::dependencies() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  return mDependencies;
}

bool QgsMapLayer::setDependencies( const QSet<QgsMapLayerDependency> &oDeps )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( QgsMapLayerStore *store = qobject_cast<QgsMapLayerStore *>( parent() ) )
  {
    return qobject_cast<QgsProject *>( store->parent() );
  }
  return nullptr;
}

void QgsMapLayer::onNotified( const QString &message )
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( refreshOnNotifyMessage().isEmpty() || refreshOnNotifyMessage() == message )
  {
    triggerRepaint();
    emit dataChanged();
  }
}

QgsRectangle QgsMapLayer::wgs84Extent( bool forceRecalculate ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  QgsRectangle wgs84Extent;

  if ( ! forceRecalculate && ! mWgs84Extent.isNull() )
  {
    wgs84Extent = mWgs84Extent;
  }
  else if ( ! mExtent2D.isNull() || ! mExtent3D.isNull() )
  {
    QgsCoordinateTransform transformer { crs(), QgsCoordinateReferenceSystem::fromOgcWmsCrs( geoEpsgCrsAuthId() ), transformContext() };
    transformer.setBallparkTransformsAreAppropriate( true );
    try
    {
      if ( mExtent2D.isNull() )
        wgs84Extent = transformer.transformBoundingBox( mExtent3D.toRectangle() );
      else
        wgs84Extent = transformer.transformBoundingBox( mExtent2D );
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
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( extent == mExtent2D )
    return;

  mExtent2D = extent;

  // do not update the wgs84 extent if we trust layer metadata
  if ( mReadFlags & QgsMapLayer::ReadFlag::FlagTrustLayerMetadata )
    return;

  mWgs84Extent = wgs84Extent( true );
}

void QgsMapLayer::updateExtent( const QgsBox3D &extent ) const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  if ( extent == mExtent3D )
    return;

  if ( extent.isNull() )
  {
    if ( !extent.toRectangle().isNull() )
    {
      // bad 3D extent param but valid in 2d --> update 2D extent
      updateExtent( extent.toRectangle() );
    }
    else
    {
      QgsDebugMsgLevel( QStringLiteral( "Unable to update extent with empty parameter" ), 1 );
    }
  }
  else
  {
    mExtent3D = extent;

    // do not update the wgs84 extent if we trust layer metadata
    if ( mReadFlags & QgsMapLayer::ReadFlag::FlagTrustLayerMetadata )
      return;

    mWgs84Extent = wgs84Extent( true );
  }
}

bool QgsMapLayer::rebuildCrs3D( QString *error )
{
  bool res = true;
  if ( !mCRS.isValid() )
  {
    mCrs3D = QgsCoordinateReferenceSystem();
  }
  else if ( !mVerticalCrs.isValid() )
  {
    mCrs3D = mCRS;
  }
  else
  {
    switch ( mCRS.type() )
    {
      case Qgis::CrsType::Compound:
      case Qgis::CrsType::Geographic3d:
      case Qgis::CrsType::Geocentric:
        mCrs3D = mCRS;
        break;

      case Qgis::CrsType::Projected:
      {
        QString tempError;
        mCrs3D = mCRS.hasVerticalAxis() ? mCRS : QgsCoordinateReferenceSystem::createCompoundCrs( mCRS, mVerticalCrs, error ? *error : tempError );
        res = mCrs3D.isValid();
        break;
      }

      case Qgis::CrsType::Vertical:
        // nonsense situation
        mCrs3D = QgsCoordinateReferenceSystem();
        res = false;
        break;

      case Qgis::CrsType::Unknown:
      case Qgis::CrsType::Geodetic:
      case Qgis::CrsType::Geographic2d:
      case Qgis::CrsType::Temporal:
      case Qgis::CrsType::Engineering:
      case Qgis::CrsType::Bound:
      case Qgis::CrsType::Other:
      case Qgis::CrsType::DerivedProjected:
      {
        QString tempError;
        mCrs3D = QgsCoordinateReferenceSystem::createCompoundCrs( mCRS, mVerticalCrs, error ? *error : tempError );
        res = mCrs3D.isValid();
        break;
      }
    }
  }
  return res;
}

void QgsMapLayer::invalidateWgs84Extent()
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

  // do not update the wgs84 extent if we trust layer metadata
  if ( mReadFlags & QgsMapLayer::ReadFlag::FlagTrustLayerMetadata )
    return;

  mWgs84Extent = QgsRectangle();
}

QString QgsMapLayer::generalHtmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS

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

QString QgsMapLayer::customPropertyHtmlMetadata() const
{
  QString metadata;
  // custom properties
  if ( const auto keys = customPropertyKeys(); !keys.isEmpty() )
  {
    metadata += QStringLiteral( "<h1>" ) + tr( "Custom properties" ) + QStringLiteral( "</h1>\n<hr>\n" );
    metadata += QLatin1String( "<table class=\"list-view\">\n<tbody>" );
    for ( const QString &key : keys )
    {
      // keys prefaced with _ are considered private/internal details
      if ( key.startsWith( '_' ) )
        continue;

      const QVariant propValue = customProperty( key );
      QString stringValue;
      if ( propValue.type() == QVariant::List || propValue.type() == QVariant::StringList )
      {
        for ( const QString &s : propValue.toStringList() )
        {
          stringValue += "<p style=\"margin: 0;\">" + s.toHtmlEscaped() + "</p>";
        }
      }
      else
      {
        stringValue = propValue.toString().toHtmlEscaped();

        //if the result string is empty but propValue is not, the conversion has failed
        if ( stringValue.isEmpty() && !QgsVariantUtils::isNull( propValue ) )
          stringValue = tr( "<i>value cannot be displayed</i>" );
      }

      metadata += QStringLiteral( "<tr><td class=\"highlight\">%1</td><td>%2</td></tr>" ).arg( key.toHtmlEscaped(), stringValue );
    }
    metadata += QLatin1String( "</tbody></table>\n" );
    metadata += QLatin1String( "<br><br>\n" );
  }
  return metadata;
}

QString QgsMapLayer::crsHtmlMetadata() const
{
  QGIS_PROTECT_QOBJECT_THREAD_ACCESS
  QString metadata;

  auto addCrsInfo = [&metadata]( const QgsCoordinateReferenceSystem & c, bool includeType, bool includeOperation, bool includeCelestialBody )
  {
    if ( !c.isValid() )
      metadata += QStringLiteral( "<tr><td colspan=\"2\" class=\"highlight\">" ) + tr( "Unknown" ) + QStringLiteral( "</td></tr>\n" );
    else
    {
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Name" ) + QStringLiteral( "</td><td>" ) + c.userFriendlyIdentifier( Qgis::CrsIdentifierType::FullString ) + QStringLiteral( "</td></tr>\n" );

      // map units
      metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Units" ) + QStringLiteral( "</td><td>" )
                  + ( c.isGeographic() ? tr( "Geographic (uses latitude and longitude for coordinates)" ) : QgsUnitTypes::toString( c.mapUnits() ) )
                  + QStringLiteral( "</td></tr>\n" );

      if ( includeType )
      {
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Type" ) + QStringLiteral( "</td><td>" ) + QgsCoordinateReferenceSystemUtils::crsTypeToString( c.type() ) + QStringLiteral( "</td></tr>\n" );
      }

      if ( includeOperation )
      {
        // operation
        const QgsProjOperation operation = c.operation();
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Method" ) + QStringLiteral( "</td><td>" ) + operation.description() + QStringLiteral( "</td></tr>\n" );
      }

      if ( includeCelestialBody )
      {
        // celestial body
        try
        {
          const QString celestialBody = c.celestialBodyName();
          if ( !celestialBody.isEmpty() )
          {
            metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Celestial Body" ) + QStringLiteral( "</td><td>" ) + celestialBody + QStringLiteral( "</td></tr>\n" );
          }
        }
        catch ( QgsNotSupportedException & )
        {

        }
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
        metadata += QStringLiteral( "<tr><td class=\"highlight\">" ) + tr( "Coordinate Epoch" ) + QStringLiteral( "</td><td>%1</td></tr>\n" ).arg( qgsDoubleToString( c.coordinateEpoch(), 3 ) );
      }
    }
  };

  metadata += QStringLiteral( "<h1>" ) + tr( "Coordinate Reference System (CRS)" ) + QStringLiteral( "</h1>\n<hr>\n" );
  metadata += QLatin1String( "<table class=\"list-view\">\n" );
  addCrsInfo( crs(), true, true, true );
  metadata += QLatin1String( "</table>\n<br><br>\n" );

  if ( verticalCrs().isValid() )
  {
    metadata += QStringLiteral( "<h1>" ) + tr( "Vertical Coordinate Reference System (CRS)" ) + QStringLiteral( "</h1>\n<hr>\n" );
    metadata += QLatin1String( "<table class=\"list-view\">\n" );
    addCrsInfo( verticalCrs(), false, false, false );
    metadata += QLatin1String( "</table>\n<br><br>\n" );
  }

  return metadata;
}
