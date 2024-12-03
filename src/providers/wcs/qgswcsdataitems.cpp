/***************************************************************************
    qgswcsdataitems.cpp
    ---------------------
    begin                : 2 July, 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswcsdataitems.h"
#include "moc_qgswcsdataitems.cpp"
#include "qgslogger.h"
#include "qgsdatasourceuri.h"
#include "qgsowsconnection.h"

#ifdef HAVE_GUI
#include "qgswcssourceselect.h"
#endif

#include <QFileInfo>
#include <QSettings>

QgsWCSConnectionItem::QgsWCSConnectionItem( QgsDataItem *parent, QString name, QString path, QString uri )
  : QgsDataCollectionItem( parent, name, path, QStringLiteral( "WCS" ) )
  , mUri( uri )
{
  mIconName = QStringLiteral( "mIconConnect.svg" );
  mCapabilities |= Qgis::BrowserItemCapability::Collapse;
}

QVector<QgsDataItem *> QgsWCSConnectionItem::createChildren()
{
  QVector<QgsDataItem *> children;

  QgsDataSourceUri uri;
  uri.setEncodedUri( mUri );
  QgsDebugMsgLevel( "mUri = " + mUri, 2 );

  mWcsCapabilities.setUri( uri );

  // Attention: supportedLayers() gives tree leafes, not top level
  if ( !mWcsCapabilities.lastError().isEmpty() )
  {
    //children.append( new QgsErrorItem( this, tr( "Failed to retrieve layers" ), mPath + "/error" ) );
    // TODO: show the error without adding child
    return children;
  }

  const QVector<QgsWcsCoverageSummary> summaries = mWcsCapabilities.capabilities().contents.coverageSummary;
  for ( const QgsWcsCoverageSummary &coverageSummary : summaries )
  {
    // Attention, the name may be empty
    QgsDebugMsgLevel( QString::number( coverageSummary.orderId ) + ' ' + coverageSummary.identifier + ' ' + coverageSummary.title, 2 );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;

    QgsWCSLayerItem *layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + '/' + pathName, mWcsCapabilities.capabilities(), uri, coverageSummary );

    children.append( layer );
  }
  return children;
}

bool QgsWCSConnectionItem::equal( const QgsDataItem *other )
{
  if ( type() != other->type() )
  {
    return false;
  }
  const QgsWCSConnectionItem *o = dynamic_cast<const QgsWCSConnectionItem *>( other );
  if ( !o )
  {
    return false;
  }

  return ( mPath == o->mPath && mName == o->mName );
}


// ---------------------------------------------------------------------------

QgsWCSLayerItem::QgsWCSLayerItem( QgsDataItem *parent, QString name, QString path, const QgsWcsCapabilitiesProperty &capabilitiesProperty, const QgsDataSourceUri &dataSourceUri, const QgsWcsCoverageSummary &coverageSummary )
  : QgsLayerItem( parent, name, path, QString(), Qgis::BrowserLayerType::Raster, QStringLiteral( "wcs" ) )
  , mCapabilities( capabilitiesProperty )
  , mDataSourceUri( dataSourceUri )
  , mCoverageSummary( coverageSummary )
{
  mSupportedCRS = mCoverageSummary.supportedCrs;
  QgsDebugMsgLevel( "uri = " + mDataSourceUri.encodedUri(), 2 );
  mUri = createUri();
  // Populate everything, it costs nothing, all info about layers is collected
  for ( const QgsWcsCoverageSummary &coverageSummary : std::as_const( mCoverageSummary.coverageSummary ) )
  {
    // Attention, the name may be empty
    QgsDebugMsgLevel( QString::number( coverageSummary.orderId ) + ' ' + coverageSummary.identifier + ' ' + coverageSummary.title, 2 );
    QString pathName = coverageSummary.identifier.isEmpty() ? QString::number( coverageSummary.orderId ) : coverageSummary.identifier;
    QgsWCSLayerItem *layer = new QgsWCSLayerItem( this, coverageSummary.title, mPath + '/' + pathName, mCapabilities, mDataSourceUri, coverageSummary );
    mChildren.append( layer );
  }

  if ( mChildren.isEmpty() )
  {
    mIconName = QStringLiteral( "mIconWcs.svg" );
  }
  setState( Qgis::BrowserItemState::Populated );
}

QString QgsWCSLayerItem::createUri()
{
  if ( mCoverageSummary.identifier.isEmpty() )
    return QString(); // layer collection

  // Number of styles must match number of layers
  mDataSourceUri.setParam( QStringLiteral( "identifier" ), mCoverageSummary.identifier );

  // TODO(?): with WCS 1.0 GetCapabilities does not contain CRS and formats,
  // to get them we would need to call QgsWcsCapabilities::describeCoverage
  // but it is problematic to get QgsWcsCapabilities here (copy not allowed
  // by QObject, pointer is dangerous (OWS provider is changing parent))
  // We leave CRS and format default for now.

  QString format;
  // get first supported by GDAL and server
  // TODO
  //QStringList mimes = QgsGdalProvider::supportedMimes().keys();
  QStringList mimes;
  // prefer tiff
  if ( mimes.contains( QStringLiteral( "image/tiff" ) ) && mCoverageSummary.supportedFormat.contains( QStringLiteral( "image/tiff" ) ) )
  {
    format = QStringLiteral( "image/tiff" );
  }
  else
  {
    const auto constMimes = mimes;
    for ( const QString &f : constMimes )
    {
      if ( mCoverageSummary.supportedFormat.indexOf( f ) >= 0 )
      {
        format = f;
        break;
      }
    }
  }
  if ( !format.isEmpty() )
  {
    mDataSourceUri.setParam( QStringLiteral( "format" ), format );
  }

  QString crs;

  // TODO: prefer project CRS
  // get first known if possible
  QgsCoordinateReferenceSystem testCrs;
  for ( const QString &c : std::as_const( mCoverageSummary.supportedCrs ) )
  {
    testCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( c );
    if ( testCrs.isValid() )
    {
      crs = c;
      break;
    }
  }
  if ( crs.isEmpty() && !mCoverageSummary.supportedCrs.isEmpty() )
  {
    crs = mCoverageSummary.supportedCrs.value( 0 );
  }
  if ( !crs.isEmpty() )
  {
    mDataSourceUri.setParam( QStringLiteral( "crs" ), crs );
  }

  return mDataSourceUri.encodedUri();
}

// ---------------------------------------------------------------------------

QgsWCSRootItem::QgsWCSRootItem( QgsDataItem *parent, QString name, QString path )
  : QgsConnectionsRootItem( parent, name, path, QStringLiteral( "WCS" ) )
{
  mCapabilities |= Qgis::BrowserItemCapability::Fast;
  mIconName = QStringLiteral( "mIconWcs.svg" );
  populate();
}

QVector<QgsDataItem *> QgsWCSRootItem::createChildren()
{
  QVector<QgsDataItem *> connections;
  const QStringList list = QgsOwsConnection::connectionList( "WCS" );
  for ( const QString &connName : list )
  {
    QgsOwsConnection connection( QStringLiteral( "WCS" ), connName );
    QgsDataItem *conn = new QgsWCSConnectionItem( this, connName, mPath + '/' + connName, connection.uri().encodedUri() );
    connections.append( conn );
  }
  return connections;
}

#ifdef HAVE_GUI

QWidget *QgsWCSRootItem::paramWidget()
{
  QgsWCSSourceSelect *select = new QgsWCSSourceSelect( nullptr, Qt::WindowFlags(), QgsProviderRegistry::WidgetMode::Manager );
  connect( select, &QgsOWSSourceSelect::connectionsChanged, this, &QgsWCSRootItem::onConnectionsChanged );
  return select;
}

void QgsWCSRootItem::onConnectionsChanged()
{
  refresh();
}

#endif

// ---------------------------------------------------------------------------
QString QgsWcsDataItemProvider::name()
{
  return QStringLiteral( "WCS" );
}

QString QgsWcsDataItemProvider::dataProviderKey() const
{
  return QStringLiteral( "wcs" );
}

Qgis::DataItemProviderCapabilities QgsWcsDataItemProvider::capabilities() const
{
  return Qgis::DataItemProviderCapability::NetworkSources;
}

QgsDataItem *QgsWcsDataItemProvider::createDataItem( const QString &path, QgsDataItem *parentItem )
{
  QgsDebugMsgLevel( "thePath = " + path, 2 );
  if ( path.isEmpty() )
  {
    // Top level WCS
    return new QgsWCSRootItem( parentItem, QStringLiteral( "WCS" ), QStringLiteral( "wcs:" ) );
  }

  // path schema: wcs:/connection name (used by OWS)
  if ( path.startsWith( QLatin1String( "wcs:/" ) ) )
  {
    QString connectionName = path.split( '/' ).last();
    if ( QgsOwsConnection::connectionList( QStringLiteral( "WCS" ) ).contains( connectionName ) )
    {
      QgsOwsConnection connection( QStringLiteral( "WCS" ), connectionName );
      return new QgsWCSConnectionItem( parentItem, QStringLiteral( "WCS" ), path, connection.uri().encodedUri() );
    }
  }

  return nullptr;
}
