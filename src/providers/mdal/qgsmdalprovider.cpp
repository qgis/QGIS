/***************************************************************************
                         qgsmeshmemorydataprovider.cpp
                         -----------------------------
    begin                : April 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <string>

#include "qgsmdalprovider.h"

#ifdef HAVE_GUI
#include "qgssourceselectprovider.h"
#include "qgsmdalsourceselect.h"
#endif

static const QString TEXT_PROVIDER_KEY = QStringLiteral( "mdal" );
static const QString TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "MDAL provider" );

bool QgsMdalProvider::isValid() const
{
  return mMeshH != nullptr;
}

QString QgsMdalProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsMdalProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QgsCoordinateReferenceSystem QgsMdalProvider::crs() const
{
  return mCrs;
}

QgsMdalProvider::QgsMdalProvider( const QString &uri, const ProviderOptions &options )
  : QgsMeshDataProvider( uri, options )
{
  QByteArray curi = uri.toAscii();
  mMeshH = MDAL_LoadMesh( curi.constData() );
  if ( mMeshH )
  {
    const QString proj = MDAL_M_projection( mMeshH );
    if ( !proj.isEmpty() )
      mCrs.createFromString( proj );
  }
}

QgsMdalProvider::~QgsMdalProvider()
{
  if ( mMeshH )
    MDAL_CloseMesh( mMeshH );
}

int QgsMdalProvider::vertexCount() const
{
  if ( mMeshH )
    return MDAL_M_vertexCount( mMeshH );
  else
    return 0;
}

int QgsMdalProvider::faceCount() const
{
  if ( mMeshH )
    return MDAL_M_faceCount( mMeshH );
  else
    return 0;
}

QgsMeshVertex QgsMdalProvider::vertex( int index ) const
{
  Q_ASSERT( index < vertexCount() );
  double x = MDAL_M_vertexXCoordinatesAt( mMeshH, index );
  double y = MDAL_M_vertexYCoordinatesAt( mMeshH, index );
  QgsMeshVertex vertex( x, y );
  return vertex;
}

QgsMeshFace QgsMdalProvider::face( int index ) const
{
  Q_ASSERT( index < faceCount() );
  QgsMeshFace face;
  int n_face_vertices = MDAL_M_faceVerticesCountAt( mMeshH, index );
  for ( int j = 0; j < n_face_vertices; ++j )
  {
    int vertex_index = MDAL_M_faceVerticesIndexAt( mMeshH, index, j );
    face.push_back( vertex_index );
  }
  return face;
}

/*----------------------------------------------------------------------------------------------*/

bool QgsMdalProvider::addDataset( const QString &uri )
{
  int datasetCount = datasetGroupCount();

  std::string str = uri.toStdString();
  MDAL_M_LoadDatasets( mMeshH, str.c_str() );

  if ( datasetCount == datasetGroupCount() )
  {
    return false;
  }
  else
  {
    mExtraDatasetUris << uri;
    emit datasetGroupsAdded( datasetGroupCount() - datasetCount );
    emit dataChanged();
    return true; // Ok
  }
}

QStringList QgsMdalProvider::extraDatasets() const
{
  return mExtraDatasetUris;
}

int QgsMdalProvider::datasetGroupCount() const
{
  return MDAL_M_datasetGroupCount( mMeshH );
}


int QgsMdalProvider::datasetCount( int groupIndex ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, groupIndex );
  if ( !group )
    return 0;
  return MDAL_G_datasetCount( group );
}

QgsMeshDatasetGroupMetadata QgsMdalProvider::datasetGroupMetadata( int groupIndex ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, groupIndex );
  if ( !group )
    return QgsMeshDatasetGroupMetadata();


  bool isScalar = MDAL_G_hasScalarData( group );
  bool isOnVertices = MDAL_G_isOnVertices( group );
  QString name = MDAL_G_name( group );

  QMap<QString, QString> metadata;
  int n = MDAL_G_metadataCount( group );
  for ( int i = 0; i < n; ++i )
  {
    QString key = MDAL_G_metadataKey( group, i );
    QString value = MDAL_G_metadataValue( group, i );
    metadata[key] = value;
  }

  QgsMeshDatasetGroupMetadata meta(
    name,
    isScalar,
    isOnVertices,
    metadata
  );

  return meta;
}

QgsMeshDatasetMetadata QgsMdalProvider::datasetMetadata( QgsMeshDatasetIndex index ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDatasetMetadata();

  DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDatasetMetadata();

  bool isValid = MDAL_D_isValid( dataset );
  double time = MDAL_D_time( dataset );

  QgsMeshDatasetMetadata meta(
    time,
    isValid
  );

  return meta;

}

QgsMeshDatasetValue QgsMdalProvider::datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return QgsMeshDatasetValue();

  DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return QgsMeshDatasetValue();

  QgsMeshDatasetValue val;

  if ( MDAL_G_hasScalarData( group ) )
  {
    val.setX( MDAL_D_value( dataset, valueIndex ) );
  }
  else
  {
    val.setX( MDAL_D_valueX( dataset, valueIndex ) );
    val.setY( MDAL_D_valueY( dataset, valueIndex ) );
  }

  return val;
}

bool QgsMdalProvider::isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const
{
  DatasetGroupH group = MDAL_M_datasetGroup( mMeshH, index.group() );
  if ( !group )
    return false;

  DatasetH dataset = MDAL_G_dataset( group, index.dataset() );
  if ( !dataset )
    return false;

  return MDAL_D_active( dataset, faceIndex );
}

/*----------------------------------------------------------------------------------------------*/

/**
 * Class factory to return a pointer to a newly created
 * QgsGdalProvider object
 */
QGISEXTERN QgsMdalProvider *classFactory( const QString *uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsMdalProvider( *uri, options );
}

/**
 * Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN void cleanupProvider()
{
}

#ifdef HAVE_GUI

//! Provider for mdal mesh source select
class QgsMdalMeshSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "mdal" ); }
    QString text() const override { return QObject::tr( "Mesh" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 22; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddMeshLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsMdalSourceSelect( parent, fl, widgetMode );
    }
};


QGISEXTERN QList<QgsSourceSelectProvider *> *sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> *providers = new QList<QgsSourceSelectProvider *>();

  *providers
      << new QgsMdalMeshSourceSelectProvider;

  return providers;
}

#endif
