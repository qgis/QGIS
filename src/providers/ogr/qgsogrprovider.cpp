/***************************************************************************
           qgsogrprovider.cpp Data provider for OGR supported formats
                    Formerly known as qgsshapefileprovider.cpp
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
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

#include "qgsogrprovider.h"
#include "qgslogger.h"

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>         // to collect version information
#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>

#include <limits>

#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QString>
#include <QTextCodec>

#include "qgsapplication.h"
#include "qgsdataitem.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectorlayerimport.h"

static const QString TEXT_PROVIDER_KEY = "ogr";
static const QString TEXT_PROVIDER_DESCRIPTION =
  QString( "OGR data provider" )
  + " (compiled against GDAL/OGR library version "
  + GDAL_RELEASE_NAME
  + ", running against GDAL/OGR library version "
  + GDALVersionInfo( "RELEASE_NAME" )
  + ")";

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8(x)   (x).toUtf8().constData()
#define TO8F(x)  (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8(x)   (x).toLocal8Bit().constData()
#define TO8F(x)  QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

class QgsCPLErrorHandler
{
    static void CPL_STDCALL showError( CPLErr errClass, int errNo, const char *msg )
    {
      QgsLogger::warning( QString( "OGR[%1] error %2: %3" ).arg( errClass ).arg( errNo ).arg( msg ) );
    }

  public:
    QgsCPLErrorHandler()
    {
      CPLPushErrorHandler( showError );
    }

    ~QgsCPLErrorHandler()
    {
      CPLPopErrorHandler();
    }
};


bool QgsOgrProvider::convertField( QgsField &field, const QTextCodec &encoding )
{
  OGRFieldType ogrType = OFTString; //default to string
  int ogrWidth = field.length();
  int ogrPrecision = field.precision();
  switch ( field.type() )
  {
    case QVariant::LongLong:
      ogrType = OFTString;
      ogrWidth = ogrWidth > 0 && ogrWidth <= 21 ? ogrWidth : 21;
      ogrPrecision = -1;
      break;

    case QVariant::String:
      ogrType = OFTString;
      if ( ogrWidth < 0 || ogrWidth > 255 )
        ogrWidth = 255;
      break;

    case QVariant::Int:
      ogrType = OFTInteger;
      ogrWidth = ogrWidth > 0 && ogrWidth <= 10 ? ogrWidth : 10;
      ogrPrecision = 0;
      break;

    case QVariant::Double:
      ogrType = OFTReal;
      break;

    default:
      return false;
  }

  field.setTypeName( encoding.toUnicode( OGR_GetFieldTypeName( ogrType ) ) );
  field.setLength( ogrWidth );
  field.setPrecision( ogrPrecision );
  return true;
}


QgsVectorLayerImport::ImportError QgsOgrProvider::createEmptyLayer(
  const QString& uri,
  const QgsFieldMap &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  QString encoding;
  QString driverName = "ESRI Shapefile";
  QStringList dsOptions, layerOptions;

  if ( options )
  {
    if ( options->contains( "fileEncoding" ) )
      encoding = options->value( "fileEncoding" ).toString();

    if ( options->contains( "driverName" ) )
      driverName = options->value( "driverName" ).toString();

    if ( options->contains( "datasourceOptions" ) )
      dsOptions << options->value( "datasourceOptions" ).toStringList();

    if ( options->contains( "layerOptions" ) )
      layerOptions << options->value( "layerOptions" ).toStringList();
  }

  if ( oldToNewAttrIdxMap )
    oldToNewAttrIdxMap->clear();
  if ( errorMessage )
    errorMessage->clear();

  if ( !overwrite )
  {
    QFileInfo fi( uri );
    if ( fi.exists() )
    {
      if ( errorMessage )
        *errorMessage += QObject::tr( "Unable to create the datasource. %1 exists and overwrite flag is false." )
                         .arg( uri );
      return QgsVectorLayerImport::ErrCreateDataSource;
    }
  }

  QgsVectorFileWriter *writer = new QgsVectorFileWriter(
    uri, encoding, fields, wkbType,
    srs, driverName, dsOptions, layerOptions );

  QgsVectorFileWriter::WriterError error = writer->hasError();
  if ( error )
  {
    if ( errorMessage )
      *errorMessage += writer->errorMessage();

    delete writer;
    return ( QgsVectorLayerImport::ImportError ) error;
  }

  if ( oldToNewAttrIdxMap )
  {
    QMap<int, int> attrIdxMap = writer->attrIdxToOgrIdx();
    for ( QMap<int, int>::const_iterator attrIt = attrIdxMap.begin(); attrIt != attrIdxMap.end(); ++attrIt )
    {
      oldToNewAttrIdxMap->insert( attrIt.key(), *attrIt );
    }
  }

  delete writer;
  return QgsVectorLayerImport::NoError;
}


QgsOgrProvider::QgsOgrProvider( QString const & uri )
    : QgsVectorDataProvider( uri ),
    ogrDataSource( 0 ),
    extent_( 0 ),
    ogrLayer( 0 ),
    ogrOrigLayer( 0 ),
    ogrDriver( 0 ),
    featuresCounted( -1 )
{
  QgsCPLErrorHandler handler;

  QgsApplication::registerOgrDrivers();

  // set the selection rectangle pointer to 0
  mSelectionRectangle = 0;
  // make connection to the data source

  QgsDebugMsg( "Data source uri is " + uri );

  // try to open for update, but disable error messages to avoid a
  // message if the file is read only, because we cope with that
  // ourselves.

  // This part of the code parses the uri transmitted to the ogr provider to
  // get the options the client wants us to apply

  // If there is no & in the uri, then the uri is just the filename. The loaded
  // layer will be layer 0.
  //this is not true for geojson
  if ( !uri.contains( '|', Qt::CaseSensitive ) )
  {
    mFilePath = uri;
    mLayerIndex = 0;
    mLayerName = QString::null;
  }
  else
  {
    QStringList theURIParts = uri.split( "|" );
    mFilePath = theURIParts.at( 0 );

    for ( int i = 1 ; i < theURIParts.size(); i++ )
    {
      QString part = theURIParts.at( i );
      int pos = part.indexOf( "=" );
      QString field = part.left( pos );
      QString value = part.mid( pos + 1 );

      if ( field == "layerid" )
      {
        bool ok;
        mLayerIndex = value.toInt( &ok );
        if ( ! ok )
        {
          mLayerIndex = -1;
        }
      }
      else if ( field == "layername" )
      {
        mLayerName = value;
      }

      if ( field == "subset" )
      {
        mSubsetString = value;
      }
    }
  }

  QgsDebugMsg( "mFilePath: " + mFilePath );
  QgsDebugMsg( "mLayerIndex: " + QString::number( mLayerIndex ) );
  QgsDebugMsg( "mLayerName: " + mLayerName );
  QgsDebugMsg( "mSubsetString: " + mSubsetString );
  CPLSetConfigOption( "OGR_ORGANIZE_POLYGONS", "ONLY_CCW" );  // "SKIP" returns MULTIPOLYGONs for multiringed POLYGONs

  ogrDataSource = OGROpen( TO8F( mFilePath ), true, &ogrDriver );
  if ( !ogrDataSource )
  {
    // try to open read-only
    ogrDataSource = OGROpen( TO8F( mFilePath ), false, &ogrDriver );

    //TODO Need to set a flag or something to indicate that the layer
    //TODO is in read-only mode, otherwise edit ops will fail
    //TODO: capabilities() should now reflect this; need to test.
  }
  if ( ogrDataSource )
  {

    QgsDebugMsg( "Data source is valid" );
    QgsDebugMsg( "OGR Driver was " + QString( OGR_Dr_GetName( ogrDriver ) ) );

    valid = true;

    ogrDriverName = OGR_Dr_GetName( ogrDriver );

    // We get the layer which was requested by the uri. The layername
    // has precedence over the layerid if both are given.
    if ( mLayerName.isNull() )
    {
      ogrOrigLayer = OGR_DS_GetLayer( ogrDataSource, mLayerIndex );
    }
    else
    {
      ogrOrigLayer = OGR_DS_GetLayerByName( ogrDataSource, TO8( mLayerName ) );
    }

    ogrLayer = ogrOrigLayer;
    setSubsetString( mSubsetString );
  }
  else
  {
    QgsLogger::critical( "Data source is invalid" );
    QgsLogger::critical( QString::fromUtf8( CPLGetLastErrorMsg() ) );
    valid = false;
  }

  // FIXME: sync with app/qgsnewvectorlayerdialog.cpp
  mNativeTypes
  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), "integer", QVariant::Int, 1, 10 )
  << QgsVectorDataProvider::NativeType( tr( "Decimal number (real)" ), "double", QVariant::Double, 1, 20, 0, 5 )
  << QgsVectorDataProvider::NativeType( tr( "Text (string)" ), "string", QVariant::String, 1, 255 )
  ;
}

QgsOgrProvider::~QgsOgrProvider()
{
  if ( ogrLayer != ogrOrigLayer )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, ogrLayer );
  }

  OGR_DS_Destroy( ogrDataSource );
  ogrDataSource = 0;

  if ( extent_ )
  {
    free( extent_ );
    extent_ = 0;
  }

  if ( mSelectionRectangle )
  {
    OGR_G_DestroyGeometry( mSelectionRectangle );
    mSelectionRectangle = 0;
  }
}

bool QgsOgrProvider::setSubsetString( QString theSQL, bool updateFeatureCount )
{
  QgsCPLErrorHandler handler;

  if ( theSQL == mSubsetString && featuresCounted >= 0 )
    return true;

  OGRLayerH prevLayer = ogrLayer;
  QString prevSubsetString = mSubsetString;
  mSubsetString = theSQL;

  if ( !mSubsetString.isEmpty() )
  {
    QString sql = QString( "SELECT * FROM %1 WHERE %2" )
                  .arg( quotedIdentifier( OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) ) ) )
                  .arg( mSubsetString );
    QgsDebugMsg( QString( "SQL: %1" ).arg( sql ) );
    ogrLayer = OGR_DS_ExecuteSQL( ogrDataSource, TO8( sql ), NULL, NULL );

    if ( !ogrLayer )
    {
      pushError( QString( "OGR[%1] error %2: %3" ).arg( CPLGetLastErrorType() ).arg( CPLGetLastErrorNo() ).arg( CPLGetLastErrorMsg() ) );
      ogrLayer = prevLayer;
      mSubsetString = prevSubsetString;
      return false;
    }
  }
  else
  {
    ogrLayer = ogrOrigLayer;
  }

  if ( prevLayer != ogrOrigLayer )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, prevLayer );
  }

  QString uri = mFilePath;
  if ( !mLayerName.isNull() )
  {
    uri += QString( "|layername=%1" ).arg( mLayerName );
  }
  else if ( mLayerIndex >= 0 )
  {
    uri += QString( "|layerid=%1" ).arg( mLayerIndex );
  }

  if ( !mSubsetString.isEmpty() )
  {
    uri += QString( "|subset=%1" ).arg( mSubsetString );
  }

  setDataSourceUri( uri );

  OGR_L_ResetReading( ogrLayer );

  // getting the total number of features in the layer
  // TODO: This can be expensive, do we really need it!
  if ( updateFeatureCount )
  {
    recalculateFeatureCount();
  }

  // check the validity of the layer
  QgsDebugMsg( "checking validity" );
  loadFields();
  QgsDebugMsg( "Done checking validity" );

  if ( extent_ )
  {
    free( extent_ );
    extent_ = 0;
  }

  return true;
}

QString QgsOgrProvider::subsetString()
{
  return mSubsetString;
}

QStringList QgsOgrProvider::subLayers() const
{
  QStringList theList = QStringList();
  if ( ! valid )
  {
    return theList;
  }
  for ( unsigned int i = 0; i < layerCount() ; i++ )
  {
    QString theLayerName = FROM8( OGR_FD_GetName( OGR_L_GetLayerDefn( OGR_DS_GetLayer( ogrDataSource, i ) ) ) );
    OGRwkbGeometryType layerGeomType = OGR_FD_GetGeomType( OGR_L_GetLayerDefn( OGR_DS_GetLayer( ogrDataSource, i ) ) );

    int theLayerFeatureCount = OGR_L_GetFeatureCount( OGR_DS_GetLayer( ogrDataSource, i ), 1 ) ;

    QString geom;
    switch ( layerGeomType )
    {
      case wkbUnknown:            geom = "Unknown"; break;
      case wkbPoint:              geom = "Point"; break;
      case wkbLineString:         geom = "LineString"; break;
      case wkbPolygon:            geom = "Polygon"; break;
      case wkbMultiPoint:         geom = "MultiPoint"; break;
      case wkbMultiLineString:    geom = "MultiLineString"; break;
      case wkbGeometryCollection: geom = "GeometryCollection"; break;
      case wkbNone:               geom = "None"; break;
      case wkbPoint25D:           geom = "Point25D"; break;
      case wkbLineString25D:      geom = "LineString25D"; break;
      case wkbPolygon25D:         geom = "Polygon25D"; break;
      case wkbMultiPoint25D:      geom = "MultiPoint25D"; break;
      case wkbMultiLineString25D: geom = "MultiLineString25D"; break;
      case wkbMultiPolygon25D:    geom = "MultiPolygon25D"; break;
      default: geom="Unknown WKB: " + QString::number( layerGeomType );
    }
    theList.append( QString::number( i ) + ":" + theLayerName + ":" + QString::number( theLayerFeatureCount ) + ":" + geom );
  }
  return theList;
}

void QgsOgrProvider::setEncoding( const QString& e )
{
  QgsVectorDataProvider::setEncoding( e );
  loadFields();
}

// This is reused by dataItem
int getOgrGeomType( OGRLayerH ogrLayer )
{
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  int geomType = wkbUnknown;
  if ( fdef )
  {
    geomType = OGR_FD_GetGeomType( fdef );

    //Some ogr drivers (e.g. GML) are not able to determine the geometry type of a layer like this.
    //In such cases, we examine the first feature
    if ( geomType == wkbUnknown )
    {
      OGR_L_ResetReading( ogrLayer );
      OGRFeatureH firstFeature = OGR_L_GetNextFeature( ogrLayer );
      if ( firstFeature )
      {
        OGRGeometryH firstGeometry = OGR_F_GetGeometryRef( firstFeature );
        if ( firstGeometry )
        {
          geomType = OGR_G_GetGeometryType( firstGeometry );
        }
        OGR_F_Destroy( firstFeature );
      }
      OGR_L_ResetReading( ogrLayer );
    }
  }
  return geomType;
}

void QgsOgrProvider::loadFields()
{
  //the attribute fields need to be read again when the encoding changes
  mAttributeFields.clear();

  geomType = getOgrGeomType( ogrLayer );
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  if ( fdef )
  {
    for ( int i = 0; i < OGR_FD_GetFieldCount( fdef ); ++i )
    {
      OGRFieldDefnH fldDef = OGR_FD_GetFieldDefn( fdef, i );
      OGRFieldType ogrType = OGR_Fld_GetType( fldDef );
      QVariant::Type varType;
      switch ( ogrType )
      {
        case OFTInteger: varType = QVariant::Int; break;
        case OFTReal: varType = QVariant::Double; break;
          // unsupported in OGR 1.3
          //case OFTDateTime: varType = QVariant::DateTime; break;
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1400
        case OFTString: varType = QVariant::String; break;
#endif
        default: varType = QVariant::String; // other unsupported, leave it as a string
      }

      mAttributeFields.insert(
        i, QgsField(
          mEncoding->toUnicode( OGR_Fld_GetNameRef( fldDef ) ), varType,
          mEncoding->toUnicode( OGR_GetFieldTypeName( ogrType ) ),
          OGR_Fld_GetWidth( fldDef ),
          OGR_Fld_GetPrecision( fldDef ) ) );
    }
  }
}


QString QgsOgrProvider::storageType() const
{
  // Delegate to the driver loaded in by OGR
  return ogrDriverName;
}

void QgsOgrProvider::setRelevantFields( bool fetchGeometry, const QgsAttributeList &fetchAttributes )
{
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
  if ( OGR_L_TestCapability( ogrLayer, OLCIgnoreFields ) )
  {
    QVector<const char*> ignoredFields;
    OGRFeatureDefnH featDefn = OGR_L_GetLayerDefn( ogrLayer );
    for ( int i = 0; i < mAttributeFields.size(); i++ )
    {
      if ( !fetchAttributes.contains( i ) )
      {
        // add to ignored fields
        ignoredFields.append( OGR_Fld_GetNameRef( OGR_FD_GetFieldDefn( featDefn, i ) ) );
      }
    }

    if ( !fetchGeometry )
      ignoredFields.append( "OGR_GEOMETRY" );
    ignoredFields.append( "OGR_STYLE" ); // not used by QGIS
    ignoredFields.append( NULL );

    OGR_L_SetIgnoredFields( ogrLayer, ignoredFields.data() );
  }
#endif
}

bool QgsOgrProvider::featureAtId( QgsFeatureId featureId,
                                  QgsFeature& feature,
                                  bool fetchGeometry,
                                  QgsAttributeList fetchAttributes )
{
  setRelevantFields( fetchGeometry, fetchAttributes );

  OGRFeatureH fet = OGR_L_GetFeature( ogrLayer, FID_TO_NUMBER( featureId ) );
  if ( !fet )
    return false;

  feature.setFeatureId( OGR_F_GetFID( fet ) );
  feature.clearAttributeMap();
  // skip features without geometry
  if ( !OGR_F_GetGeometryRef( fet ) && !mFetchFeaturesWithoutGeom )
  {
    OGR_F_Destroy( fet );
    return false;
  }

  /* fetch geometry */
  if ( fetchGeometry )
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef( fet );
    // skip features without geometry

    // get the wkb representation
    unsigned char *wkb = new unsigned char[OGR_G_WkbSize( geom )];
    OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

    feature.setGeometryAndOwnership( wkb, OGR_G_WkbSize( geom ) );
  }

  /* fetch attributes */
  for ( QgsAttributeList::iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it )
  {
    getFeatureAttribute( fet, feature, *it );
  }

  if ( OGR_F_GetGeometryRef( fet ) != NULL )
  {
    feature.setValid( true );
  }
  else
  {
    feature.setValid( false );
  }
  OGR_F_Destroy( fet );
  return true;
}

bool QgsOgrProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );

  if ( !valid )
  {
    QgsLogger::warning( "Read attempt on an invalid shapefile data source" );
    return false;
  }

  OGRFeatureH fet;
  QgsRectangle selectionRect;

  setRelevantFields( mFetchGeom || mUseIntersect || !mFetchRect.isEmpty(),
                     mAttributesToFetch );

  while (( fet = OGR_L_GetNextFeature( ogrLayer ) ) )
  {
    // skip features without geometry
    if ( !mFetchFeaturesWithoutGeom && !OGR_F_GetGeometryRef( fet ) )
    {
      OGR_F_Destroy( fet );
      continue;
    }

    OGRFeatureDefnH featureDefinition = OGR_F_GetDefnRef( fet );
    QString featureTypeName = featureDefinition ? QString( OGR_FD_GetName( featureDefinition ) ) : QString( "" );
    feature.setFeatureId( OGR_F_GetFID( fet ) );
    feature.clearAttributeMap();
    feature.setTypeName( featureTypeName );

    /* fetch geometry */
    if ( mFetchGeom || mUseIntersect )
    {
      OGRGeometryH geom = OGR_F_GetGeometryRef( fet );

      if ( geom == 0 )
      {
        OGR_F_Destroy( fet );
        continue;
      }

      // get the wkb representation
      unsigned char *wkb = new unsigned char[OGR_G_WkbSize( geom )];
      OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), wkb );

      feature.setGeometryAndOwnership( wkb, OGR_G_WkbSize( geom ) );

      if ( mUseIntersect )
      {
        //precise test for intersection with search rectangle
        //first make QgsRectangle from OGRPolygon
        OGREnvelope env;
        memset( &env, 0, sizeof( env ) );
        if ( mSelectionRectangle )
          OGR_G_GetEnvelope( mSelectionRectangle, &env );
        if ( env.MinX != 0 || env.MinY != 0 || env.MaxX != 0 || env.MaxY != 0 ) //if envelope is invalid, skip the precise intersection test
        {
          selectionRect.set( env.MinX, env.MinY, env.MaxX, env.MaxY );
          if ( !feature.geometry()->intersects( selectionRect ) )
          {
            OGR_F_Destroy( fet );
            continue;
          }
        }

      }
    }

    /* fetch attributes */
    for ( QgsAttributeList::iterator it = mAttributesToFetch.begin(); it != mAttributesToFetch.end(); ++it )
    {
      getFeatureAttribute( fet, feature, *it );
    }

    /* we have a feature, end this cycle */
    break;

  } /* while */

  if ( fet )
  {
    if ( OGR_F_GetGeometryRef( fet ) != NULL )
    {
      feature.setValid( true );
    }
    else
    {
      feature.setValid( false );
    }
    OGR_F_Destroy( fet );
    return true;
  }
  else
  {
    QgsDebugMsg( "Feature is null" );
    // probably should reset reading here
    OGR_L_ResetReading( ogrLayer );
    return false;
  }
}

void QgsOgrProvider::select( QgsAttributeList fetchAttributes, QgsRectangle rect, bool fetchGeometry, bool useIntersect )
{
  if ( geometryType() == QGis::WKBNoGeometry )
  {
    fetchGeometry = false;
  }

  mUseIntersect = useIntersect;
  mAttributesToFetch = fetchAttributes;
  mFetchGeom = fetchGeometry;
  mFetchRect = rect;

  // spatial query to select features
  if ( rect.isEmpty() )
  {
    OGR_L_SetSpatialFilter( ogrLayer, 0 );
  }
  else
  {
    OGRGeometryH filter = 0;
    QString wktExtent = QString( "POLYGON((%1))" ).arg( rect.asPolygon() );
    QByteArray ba = wktExtent.toAscii();
    const char *wktText = ba;

    if ( useIntersect )
    {
      // store the selection rectangle for use in filtering features during
      // an identify and display attributes
      if ( mSelectionRectangle )
        OGR_G_DestroyGeometry( mSelectionRectangle );

      OGR_G_CreateFromWkt(( char ** )&wktText, NULL, &mSelectionRectangle );
      wktText = ba;
    }

    OGR_G_CreateFromWkt(( char ** )&wktText, NULL, &filter );
    QgsDebugMsg( "Setting spatial filter using " + wktExtent );
    OGR_L_SetSpatialFilter( ogrLayer, filter );
    OGR_G_DestroyGeometry( filter );
  }

  //start with first feature
  OGR_L_ResetReading( ogrLayer );
}


unsigned char * QgsOgrProvider::getGeometryPointer( OGRFeatureH fet )
{
  OGRGeometryH geom = OGR_F_GetGeometryRef( fet );
  unsigned char *gPtr = 0;

  if ( !geom )
    return 0;

  // get the wkb representation
  gPtr = new unsigned char[OGR_G_WkbSize( geom )];

  OGR_G_ExportToWkb( geom, ( OGRwkbByteOrder ) QgsApplication::endian(), gPtr );
  return gPtr;
}


QgsRectangle QgsOgrProvider::extent()
{
  if ( !extent_ )
  {
    extent_ = calloc( sizeof( OGREnvelope ), 1 );

    // get the extent_ (envelope) of the layer
    QgsDebugMsg( "Starting get extent" );

    // TODO: This can be expensive, do we really need it!
    if ( ogrLayer == ogrOrigLayer )
    {
      OGR_L_GetExtent( ogrLayer, ( OGREnvelope * ) extent_, true );
    }
    else
    {
      OGREnvelope *bb = static_cast<OGREnvelope*>( extent_ );

      bb->MinX = std::numeric_limits<double>::max();
      bb->MinY = std::numeric_limits<double>::max();
      bb->MaxX = -std::numeric_limits<double>::max();
      bb->MaxY = -std::numeric_limits<double>::max();

      OGRFeatureH f;

      OGR_L_ResetReading( ogrLayer );
      while (( f = OGR_L_GetNextFeature( ogrLayer ) ) )
      {
        OGRGeometryH g = OGR_F_GetGeometryRef( f );
        if ( g )
        {
          OGREnvelope env;
          OGR_G_GetEnvelope( g, &env );

          if ( env.MinX < bb->MinX ) bb->MinX = env.MinX;
          if ( env.MinY < bb->MinY ) bb->MinY = env.MinY;
          if ( env.MaxX > bb->MaxX ) bb->MaxX = env.MaxX;
          if ( env.MaxY > bb->MaxY ) bb->MaxY = env.MaxY;
        }

        OGR_F_Destroy( f );
      }
      OGR_L_ResetReading( ogrLayer );
    }

    QgsDebugMsg( "Finished get extent" );
  }

  OGREnvelope *ext = static_cast<OGREnvelope *>( extent_ );
  mExtentRect.set( ext->MinX, ext->MinY, ext->MaxX, ext->MaxY );
  return mExtentRect;
}


size_t QgsOgrProvider::layerCount() const
{
  return OGR_DS_GetLayerCount( ogrDataSource );
} // QgsOgrProvider::layerCount()


/**
 * Return the feature type
 */
QGis::WkbType QgsOgrProvider::geometryType() const
{
  return ( QGis::WkbType ) geomType;
}

/**
 * Return the feature type
 */
long QgsOgrProvider::featureCount() const
{
  return featuresCounted;
}

/**
 * Return the number of fields
 */
uint QgsOgrProvider::fieldCount() const
{
  return mAttributeFields.size();
}

void QgsOgrProvider::getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex )
{
  OGRFieldDefnH fldDef = OGR_F_GetFieldDefnRef( ogrFet, attindex );

  if ( ! fldDef )
  {
    QgsDebugMsg( "ogrFet->GetFieldDefnRef(attindex) returns NULL" );
    return;
  }

  QVariant value;

  if ( OGR_F_IsFieldSet( ogrFet, attindex ) )
  {
    switch ( mAttributeFields[attindex].type() )
    {
      case QVariant::String: value = QVariant( mEncoding->toUnicode( OGR_F_GetFieldAsString( ogrFet, attindex ) ) ); break;
      case QVariant::Int: value = QVariant( OGR_F_GetFieldAsInteger( ogrFet, attindex ) ); break;
      case QVariant::Double: value = QVariant( OGR_F_GetFieldAsDouble( ogrFet, attindex ) ); break;
        //case QVariant::DateTime: value = QVariant(QDateTime::fromString(str)); break;
      default: assert( NULL && "unsupported field type" );
    }
  }
  else
  {
    value = QVariant( QString::null );
  }

  f.addAttribute( attindex, value );
}


const QgsFieldMap & QgsOgrProvider::fields() const
{
  return mAttributeFields;
}

void QgsOgrProvider::rewind()
{
  OGR_L_ResetReading( ogrLayer );
}


//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsOgrProvider::isValid()
{
  return valid;
}

bool QgsOgrProvider::addFeature( QgsFeature& f )
{
  bool returnValue = true;
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn( ogrLayer );
  OGRFeatureH feature = OGR_F_Create( fdef );

  if ( f.geometry() && f.geometry()->wkbSize() > 0 )
  {
    unsigned char* wkb = f.geometry()->asWkb();
    OGRGeometryH geom = NULL;

    if ( OGR_G_CreateFromWkb( wkb, NULL, &geom, f.geometry()->wkbSize() )
         != OGRERR_NONE )
    {
      return false;
    }

    OGR_F_SetGeometryDirectly( feature, geom );
  }

  QgsAttributeMap attrs = f.attributeMap();

  //add possible attribute information
  for ( QgsAttributeMap::iterator it = attrs.begin(); it != attrs.end(); ++it )
  {
    int targetAttributeId = it.key();

    // don't try to set field from attribute map if it's not present in layer
    if ( targetAttributeId >= OGR_FD_GetFieldCount( fdef ) )
      continue;

    //if(!s.isEmpty())
    // continue;
    //
    OGRFieldDefnH fldDef = OGR_FD_GetFieldDefn( fdef, targetAttributeId );
    OGRFieldType type = OGR_Fld_GetType( fldDef );

    if ( it->isNull() || ( type != OFTString && it->toString().isEmpty() ) )
    {
      OGR_F_UnsetField( feature, targetAttributeId );
    }
    else
    {
      switch ( type )
      {
        case OFTInteger:
          OGR_F_SetFieldInteger( feature, targetAttributeId, it->toInt() );
          break;

        case OFTReal:
          OGR_F_SetFieldDouble( feature, targetAttributeId, it->toDouble() );
          break;

        case OFTString:
          QgsDebugMsg( QString( "Writing string attribute %1 with %2, encoding %3" )
                       .arg( targetAttributeId )
                       .arg( it->toString() )
                       .arg( mEncoding->name().data() ) );
          OGR_F_SetFieldString( feature, targetAttributeId, mEncoding->fromUnicode( it->toString() ).constData() );
          break;

        default:
          QgsLogger::warning( "QgsOgrProvider::addFeature, no type found" );
          break;
      }
    }
  }

  if ( OGR_L_CreateFeature( ogrLayer, feature ) != OGRERR_NONE )
  {
    QgsLogger::warning( "Writing of the feature failed" );
    returnValue = false;
  }
  else
  {
    f.setFeatureId( OGR_F_GetFID( feature ) );
  }
  OGR_F_Destroy( feature );
  return returnValue;
}


bool QgsOgrProvider::addFeatures( QgsFeatureList & flist )
{
  setRelevantFields( true, mAttributeFields.keys() );

  bool returnvalue = true;
  for ( QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it )
  {
    if ( !addFeature( *it ) )
    {
      returnvalue = false;
    }
  }

  if ( !syncToDisc() )
  {
    returnvalue = false;
  }
  recalculateFeatureCount();

  if ( returnvalue )
    clearMinMaxCache();

  return returnvalue;
}

bool QgsOgrProvider::addAttributes( const QList<QgsField> &attributes )
{
  bool returnvalue = true;

  for ( QList<QgsField>::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter )
  {
    OGRFieldType type;

    switch ( iter->type() )
    {
      case QVariant::Int:
        type = OFTInteger;
        break;
      case QVariant::Double:
        type = OFTReal;
        break;
      case QVariant::String:
        type = OFTString;
        break;
      default:
        QgsLogger::warning( QString( "QgsOgrProvider::addAttributes, type %1 not found" ).arg( iter->typeName() ) );
        returnvalue = false;
        continue;
    }

    OGRFieldDefnH fielddefn = OGR_Fld_Create( TO8( iter->name() ), type );
    OGR_Fld_SetWidth( fielddefn, iter->length() );
    OGR_Fld_SetPrecision( fielddefn, iter->precision() );

    if ( OGR_L_CreateField( ogrLayer, fielddefn, true ) != OGRERR_NONE )
    {
      QgsLogger::warning( "QgsOgrProvider.cpp: writing of field failed" );
      returnvalue = false;
    }
    OGR_Fld_Destroy( fielddefn );
  }
  loadFields();
  return returnvalue;
}

bool QgsOgrProvider::changeAttributeValues( const QgsChangedAttributesMap & attr_map )
{
  if ( attr_map.isEmpty() )
    return true;

  clearMinMaxCache();

  setRelevantFields( true, mAttributeFields.keys() );

  for ( QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it )
  {
    QgsFeatureId fid = it.key();

    if ( FID_TO_NUMBER( fid ) > std::numeric_limits<long>::max() )
    {
      QgsLogger::warning( "QgsOgrProvider::changeAttributeValues, feature id too large for OGR" );
      continue;
    }

    OGRFeatureH of = OGR_L_GetFeature( ogrLayer, static_cast<long>( FID_TO_NUMBER( fid ) ) );

    if ( !of )
    {
      QgsLogger::warning( "QgsOgrProvider::changeAttributeValues, Cannot read feature, cannot change attributes" );
      continue;
    }

    const QgsAttributeMap& attr = it.value();

    for ( QgsAttributeMap::const_iterator it2 = attr.begin(); it2 != attr.end(); ++it2 )
    {
      int f = it2.key();

      OGRFieldDefnH fd = OGR_F_GetFieldDefnRef( of, f );
      if ( !fd )
      {
        QgsLogger::warning( "QgsOgrProvider::changeAttributeValues, Field " + QString::number( f ) + " doesn't exist" );
        continue;
      }

      OGRFieldType type = OGR_Fld_GetType( fd );

      if ( it2->isNull() || ( type != OFTString && it2->toString().isEmpty() ) )
      {
        OGR_F_UnsetField( of, f );
      }
      else
      {

        switch ( type )
        {
          case OFTInteger:
            OGR_F_SetFieldInteger( of, f, it2->toInt() );
            break;
          case OFTReal:
            OGR_F_SetFieldDouble( of, f, it2->toDouble() );
            break;
          case OFTString:
            OGR_F_SetFieldString( of, f, mEncoding->fromUnicode( it2->toString() ).constData() );
            break;
          default:
            QgsLogger::warning( "QgsOgrProvider::changeAttributeValues, Unknown field type, cannot change attribute" );
            break;
        }
      }
    }

    OGRErr res;
    if (( res = OGR_L_SetFeature( ogrLayer, of ) ) != OGRERR_NONE )
    {
      QgsLogger::warning( "QgsOgrProvider::changeAttributeValues, setting the feature failed: " + QString::number( res ) );
    }
  }

  OGR_L_SyncToDisk( ogrLayer );
  return true;
}

bool QgsOgrProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  OGRErr res;
  OGRFeatureH theOGRFeature = 0;
  OGRGeometryH theNewGeometry = 0;

  setRelevantFields( true, mAttributeFields.keys() );

  for ( QgsGeometryMap::iterator it = geometry_map.begin(); it != geometry_map.end(); ++it )
  {
    if ( FID_TO_NUMBER( it.key() ) > std::numeric_limits<long>::max() )
    {
      QgsLogger::warning( "QgsOgrProvider::changeGeometryValues, fid too large for OGR" );
      continue;
    }

    theOGRFeature = OGR_L_GetFeature( ogrLayer, static_cast<long>( FID_TO_NUMBER( it.key() ) ) );
    if ( !theOGRFeature )
    {
      QgsLogger::warning( "QgsOgrProvider::changeGeometryValues, cannot find feature" );
      continue;
    }

    //create an OGRGeometry
    if ( OGR_G_CreateFromWkb( it->asWkb(),
                              OGR_L_GetSpatialRef( ogrLayer ),
                              &theNewGeometry,
                              it->wkbSize() ) != OGRERR_NONE )
    {
      QgsLogger::warning( "QgsOgrProvider::changeGeometryValues, error while creating new OGRGeometry" );
      OGR_G_DestroyGeometry( theNewGeometry );
      theNewGeometry = 0;
      continue;
    }

    if ( !theNewGeometry )
    {
      QgsLogger::warning( "QgsOgrProvider::changeGeometryValues, new geometry is NULL" );
      continue;
    }

    //set the new geometry
    if (( res = OGR_F_SetGeometryDirectly( theOGRFeature, theNewGeometry ) ) != OGRERR_NONE )
    {
      QgsLogger::warning( "QgsOgrProvider::changeGeometryValues, error while replacing geometry: " + QString::number( res ) );
      OGR_G_DestroyGeometry( theNewGeometry );
      theNewGeometry = 0;
      continue;
    }


    if (( res = OGR_L_SetFeature( ogrLayer, theOGRFeature ) ) != OGRERR_NONE )
    {
      QgsLogger::warning( "QgsOgrProvider::changeGeometryValues, error while setting feature: " + QString::number( res ) );
      OGR_G_DestroyGeometry( theNewGeometry );
      theNewGeometry = 0;
      continue;
    }

    OGR_F_Destroy( theOGRFeature );
  }
  return syncToDisc();
}

bool QgsOgrProvider::createSpatialIndex()
{
  QgsCPLErrorHandler handler;

  QString layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) );

  QString sql = QString( "CREATE SPATIAL INDEX ON %1" ).arg( quotedIdentifier( layerName ) );  // quote the layer name so spaces are handled
  QgsDebugMsg( QString( "SQL: %1" ).arg( sql ) );
  OGR_DS_ExecuteSQL( ogrDataSource, TO8( sql ), OGR_L_GetSpatialFilter( ogrOrigLayer ), "" );

  QFileInfo fi( mFilePath );     // to get the base name
  //find out, if the .qix file is there
  QFile indexfile( fi.path().append( "/" ).append( fi.completeBaseName() ).append( ".qix" ) );
  return indexfile.exists();
}

bool QgsOgrProvider::createAttributeIndex( int field )
{
  QString layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) );
  QString dropSql = QString( "DROP INDEX ON %1" ).arg( quotedIdentifier( layerName ) );
  QString createSql = QString( "CREATE INDEX ON %1 USING %2" ).arg( quotedIdentifier( layerName ) ).arg( fields()[field].name() );
  OGR_DS_ExecuteSQL( ogrDataSource, TO8( dropSql ), OGR_L_GetSpatialFilter( ogrOrigLayer ), "SQL" );
  OGR_DS_ExecuteSQL( ogrDataSource, TO8( createSql ), OGR_L_GetSpatialFilter( ogrOrigLayer ), "SQL" );

  QFileInfo fi( mFilePath );     // to get the base name
  //find out, if the .idm file is there
  QFile indexfile( fi.path().append( "/" ).append( fi.completeBaseName() ).append( ".idm" ) );
  return indexfile.exists();
}

bool QgsOgrProvider::deleteFeatures( const QgsFeatureIds & id )
{
  QgsCPLErrorHandler handler;

  bool returnvalue = true;
  for ( QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it )
  {
    if ( !deleteFeature( *it ) )
    {
      returnvalue = false;
    }
  }

  if ( !syncToDisc() )
  {
    returnvalue = false;
  }

  QString layerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrOrigLayer ) );

  QString sql = QString( "REPACK %1" ).arg( layerName );   // don't quote the layer name as it works with spaces in the name and won't work if the name is quoted
  QgsDebugMsg( QString( "SQL: %1" ).arg( sql ) );
  OGR_DS_ExecuteSQL( ogrDataSource, TO8( sql ), NULL, NULL );

  recalculateFeatureCount();

  clearMinMaxCache();

  if ( extent_ )
  {
    free( extent_ );
    extent_ = 0;
  }

  return returnvalue;
}

bool QgsOgrProvider::deleteFeature( QgsFeatureId id )
{
  if ( FID_TO_NUMBER( id ) > std::numeric_limits<long>::max() )
  {
    QgsDebugMsg( "id too large for OGR" );
    return false;
  }

  return OGR_L_DeleteFeature( ogrLayer, FID_TO_NUMBER( id ) ) == OGRERR_NONE;
}

int QgsOgrProvider::capabilities() const
{
  int ability = NoCapabilities;

  // collect abilities reported by OGR
  if ( ogrLayer )
  {
    // Whilst the OGR documentation (e.g. at
    // http://www.gdal.org/ogr/classOGRLayer.html#a17) states "The capability
    // codes that can be tested are represented as strings, but #defined
    // constants exists to ensure correct spelling", we always use strings
    // here.  This is because older versions of OGR don't always have all
    // the #defines we want to test for here.

    if ( OGR_L_TestCapability( ogrLayer, "RandomRead" ) )
      // true if the GetFeature() method works *efficiently* for this layer.
      // TODO: Perhaps influence if QGIS caches into memory
      //       (vs read from disk every time) based on this setting.
    {
      // the latter flag is here just for compatibility
      ability |= QgsVectorDataProvider::SelectAtId | QgsVectorDataProvider::SelectGeometryAtId;
    }

    if ( OGR_L_TestCapability( ogrLayer, "SequentialWrite" ) )
      // true if the CreateFeature() method works for this layer.
    {
      ability |= QgsVectorDataProvider::AddFeatures;
    }

    if ( OGR_L_TestCapability( ogrLayer, "DeleteFeature" ) )
      // true if this layer can delete its features
    {
      ability |= DeleteFeatures;
    }

    if ( OGR_L_TestCapability( ogrLayer, "RandomWrite" ) )
      // true if the SetFeature() method is operational on this layer.
    {
      // TODO According to http://shapelib.maptools.org/ (Shapefile C Library V1.2)
      // TODO "You can't modify the vertices of existing structures".
      // TODO Need to work out versions of shapelib vs versions of GDAL/OGR
      // TODO And test appropriately.

      ability |= ChangeAttributeValues;
      ability |= ChangeGeometries;
    }

#if 0
    if ( OGR_L_TestCapability( ogrLayer, "FastSpatialFilter" ) )
      // true if this layer implements spatial filtering efficiently.
      // Layers that effectively read all features, and test them with the
      // OGRFeature intersection methods should return false.
      // This can be used as a clue by the application whether it should build
      // and maintain it's own spatial index for features in this layer.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should build and maintain it's own spatial index for features in this layer.
    }

    if ( OGR_L_TestCapability( ogrLayer, "FastFeatureCount" ) )
      // true if this layer can return a feature count
      // (via OGRLayer::GetFeatureCount()) efficiently ... ie. without counting
      // the features. In some cases this will return true until a spatial
      // filter is installed after which it will return false.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to count features.
    }

    if ( OGR_L_TestCapability( ogrLayer, "FastGetExtent" ) )
      // true if this layer can return its data extent
      // (via OGRLayer::GetExtent()) efficiently ... ie. without scanning
      // all the features. In some cases this will return true until a
      // spatial filter is installed after which it will return false.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to calculate extent.
    }

    if ( OGR_L_TestCapability( ogrLayer, "FastSetNextByIndex" ) )
      // true if this layer can perform the SetNextByIndex() call efficiently.
    {
      // No use required for this QGIS release.
    }
#endif

    // OGR doesn't handle shapefiles without attributes, ie. missing DBFs well, fixes #803
    if ( ogrDriverName == "ESRI Shapefile" )
    {
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1260
      // test for Shapefile type and GDAL >= 1.2.6
      ability |= CreateSpatialIndex;
#endif
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1600
      // adding attributes was added in GDAL 1.6
      ability |= AddAttributes;
#endif
      ability |= CreateAttributeIndex;

      if ( mAttributeFields.size() == 0 )
      {
        QgsDebugMsg( "OGR doesn't handle shapefile without attributes well, ie. missing DBFs" );
        ability &= ~( AddFeatures | DeleteFeatures | ChangeAttributeValues | AddAttributes | DeleteAttributes );
      }

      if (( ability & ChangeAttributeValues ) == 0 )
      {
        // on readonly shapes OGR reports that it can delete features although it can't RandomWrite
        ability &= ~( AddAttributes | DeleteFeatures );
      }
    }
  }

  return ability;
}


QString QgsOgrProvider::name() const
{
  return TEXT_PROVIDER_KEY;
} // QgsOgrProvider::name()


QString  QgsOgrProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
} //  QgsOgrProvider::description()


/**

  Convenience function for readily creating file filters.

  Given a long name for a file filter and a regular expression, return
  a file filter string suitable for use in a QFileDialog::OpenFiles()
  call.  The regular express, glob, will have both all lower and upper
  case versions added.

  @note

  Copied from qgisapp.cpp.

  @todo XXX This should probably be generalized and moved to a standard
            utility type thingy.

*/
static QString createFileFilter_( QString const &longName, QString const &glob )
{
  return longName + " [OGR] (" + glob.toLower() + " " + glob.toUpper() + ");;";
} // createFileFilter_


QString createFilters( QString type )
{
  /**Database drivers available*/
  static QString myDatabaseDrivers;
  /**Protocol drivers available*/
  static QString myProtocolDrivers;
  /**File filters*/
  static QString myFileFilters;
  /**Directory drivers*/
  static QString myDirectoryDrivers;
  /**Extensions*/
  static QStringList myExtensions;
  /**Wildcards*/
  static QStringList myWildcards;

  // if we've already built the supported vector string, just return what
  // we've already built

  if ( myFileFilters.isEmpty() || myFileFilters.isNull() )
  {
    // register ogr plugins
    QgsApplication::registerOgrDrivers();

    // first get the GDAL driver manager
    OGRSFDriverH driver;          // current driver
    QString driverName;           // current driver name

    // Grind through all the drivers and their respective metadata.
    // We'll add a file filter for those drivers that have a file
    // extension defined for them; the others, welll, even though
    // theoreticaly we can open those files because there exists a
    // driver for them, the user will have to use the "All Files" to
    // open datasets with no explicitly defined file name extension.
    QgsDebugMsg( QString( "Driver count: %1" ).arg( OGRGetDriverCount() ) );

    for ( int i = 0; i < OGRGetDriverCount(); ++i )
    {
      driver = OGRGetDriver( i );

      Q_CHECK_PTR( driver );

      if ( !driver )
      {
        QgsLogger::warning( "unable to get driver " + QString::number( i ) );
        continue;
      }

      driverName = OGR_Dr_GetName( driver );

      if ( driverName.startsWith( "AVCBin" ) )
      {
        myDirectoryDrivers += QObject::tr( "Arc/Info Binary Coverage" ) + ",AVCBin;";
      }
      else if ( driverName.startsWith( "AVCE00" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Arc/Info ASCII Coverage" ), "*.e00" );
        myExtensions << "e00";
      }
      else if ( driverName.startsWith( "BNA" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Atlas BNA" ), "*.bna" );
        myExtensions << "bna";
      }
      else if ( driverName.startsWith( "CSV" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Comma Separated Value" ), "*.csv" );
        myExtensions << "csv";
      }
      else if ( driverName.startsWith( QObject::tr( "DODS" ) ) )
      {
        myProtocolDrivers += "DODS/OPeNDAP,DODS;";
      }
      else if ( driverName.startsWith( "PGeo" ) )
      {
        myDatabaseDrivers += QObject::tr( "ESRI Personal GeoDatabase" ) + ",PGeo;";
#ifdef WIN32
        myFileFilters += createFileFilter_( QObject::tr( "ESRI Personal GeoDatabase" ), "*.mdb" );
        myExtensions << "mdb";
#endif
      }
      else if ( driverName.startsWith( "SDE" ) )
      {
        myDatabaseDrivers += QObject::tr( "ESRI ArcSDE" ) + ",SDE;";
      }
      else if ( driverName.startsWith( "ESRI" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "ESRI Shapefiles" ), "*.shp" );
        myExtensions << "shp" << "dbf";
      }
      else if ( driverName.startsWith( QObject::tr( "FMEObjects Gateway" ) ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "FMEObjects Gateway" ), "*.fdd" );
        myExtensions << "fdd";
      }
      else if ( driverName.startsWith( "GeoJSON" ) )
      {
        myProtocolDrivers += "GeoJSON,GeoJSON;";
        myFileFilters += createFileFilter_( QObject::tr( "GeoJSON" ), "*.geojson" );
        myExtensions << "geojson";
      }
      else if ( driverName.startsWith( "GeoRSS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "GeoRSS" ), "*.xml" );
        myExtensions << "xml";
      }
      else if ( driverName.startsWith( "GML" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Geography Markup Language [GML]" ), "*.gml" );
        myExtensions << "gml";
      }
      else if ( driverName.startsWith( "GMT" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Generic Mapping Tools [GMT]" ), "*.gmt" );
        myExtensions << "gmt";
      }
      else if ( driverName.startsWith( "GPX" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "GPS eXchange Format [GPX]" ), "*.gpx" );
        myExtensions << "gpx";
      }
      else if ( driverName.startsWith( "GRASS" ) )
      {
        myDirectoryDrivers += QObject::tr( "Grass Vector" ) + ",GRASS;";
      }
      else if ( driverName.startsWith( "IDB" ) )
      {
        myDatabaseDrivers += QObject::tr( "Informix DataBlade" ) + ",IDB;";
      }
      else if ( driverName.startsWith( "Interlis 1" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "INTERLIS 1" ), "*.itf *.xml *.ili" );
        myExtensions << "itf" << "xml" << "ili";
      }
      else if ( driverName.startsWith( "Interlis 2" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "INTERLIS 2" ), "*.itf *.xml *.ili" );
        myExtensions << "itf" << "xml" << "ili";
      }
      else if ( driverName.startsWith( "INGRES" ) )
      {
        myDatabaseDrivers += QObject::tr( "INGRES" ) + ",INGRES;";
      }
      else if ( driverName.startsWith( "KML" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Keyhole Markup Language [KML]" ), "*.kml" );
        myExtensions << "kml";
      }
      else if ( driverName.startsWith( "MapInfo File" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Mapinfo File" ), "*.mif *.tab" );
        myExtensions << "mif" << "tab";
      }
      else if ( driverName.startsWith( "DGN" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Microstation DGN" ), "*.dgn" );
        myExtensions << "dgn";
      }
      else if ( driverName.startsWith( "MySQL" ) )
      {
        myDatabaseDrivers += QObject::tr( "MySQL" ) + ",MySQL;";
      }
      else if ( driverName.startsWith( "OCI" ) )
      {
        myDatabaseDrivers += QObject::tr( "Oracle Spatial" ) + ",OCI;";
      }
      else if ( driverName.startsWith( "ODBC" ) )
      {
        myDatabaseDrivers += QObject::tr( "ODBC" ) + ",ODBC;";
      }
      else if ( driverName.startsWith( "OGDI" ) )
      {
        myDatabaseDrivers += QObject::tr( "OGDI Vectors" ) + ",OGDI;";
      }
      else if ( driverName.startsWith( "PostgreSQL" ) )
      {
        myDatabaseDrivers += QObject::tr( "PostgreSQL" ) + ",PostgreSQL;";
      }
      else if ( driverName.startsWith( "S57" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "S-57 Base file" ),
                                            "*.000" );
        myExtensions << "000";
      }
      else if ( driverName.startsWith( "SDTS" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Spatial Data Transfer Standard [SDTS]" ),
                                            "*catd.ddf" );
        myWildcards << "*catd.ddf";
      }
      else if ( driverName.startsWith( "SQLite" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "SQLite" ),
                                            "*.sqlite" );
        myExtensions << "sqlite";
      }
      else if ( driverName.startsWith( "UK .NTF" ) )
      {
        myDirectoryDrivers += QObject::tr( "UK. NTF2" ) + ",UK. NTF;";
      }
      else if ( driverName.startsWith( "TIGER" ) )
      {
        myDirectoryDrivers += QObject::tr( "U.S. Census TIGER/Line" ) + ",TIGER;";
      }
      else if ( driverName.startsWith( "VRT" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "VRT - Virtual Datasource" ),
                                            "*.vrt" );
        myExtensions << "vrt";
      }
      else if ( driverName.startsWith( "XPlane" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "X-Plane/Flightgear" ),
                                            "apt.dat nav.dat fix.dat awy.dat" );
        myWildcards << "apt.dat" << "nav.dat" << "fix.dat" << "awy.dat";
      }
      else if ( driverName.startsWith( "Geoconcept" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "Geoconcept" ), "*.gxt *.txt" );
        myExtensions << "gxt" << "txt";
      }
      else if ( driverName.startsWith( "DXF" ) )
      {
        myFileFilters += createFileFilter_( QObject::tr( "AutoCAD DXF" ), "*.dxf" );
        myExtensions << "dxf";
      }
      else
      {
        // NOP, we don't know anything about the current driver
        // with regards to a proper file filter string
        QgsDebugMsg( "fileVectorFilters, unknown driver: " + driverName );
      }

    }                           // each loaded GDAL driver

    // can't forget the default case

    myFileFilters += QObject::tr( "All files" ) + " (*)";
  }

  if ( type == "file" )
  {
    return myFileFilters;
  }
  if ( type == "database" )
  {
    return myDatabaseDrivers;
  }
  if ( type == "protocol" )
  {
    return myProtocolDrivers;
  }
  if ( type == "directory" )
  {
    return myDirectoryDrivers;
  }
  if ( type == "extensions" )
  {
    return myExtensions.join( "|" );
  }
  if ( type == "wildcards" )
  {
    return myWildcards.join( "|" );
  }
  else
  {
    return "";
  }
}


QGISEXTERN QString fileVectorFilters()
{
  return createFilters( "file" );
}

QString QgsOgrProvider::fileVectorFilters() const
{
  return createFilters( "file" );
}

QGISEXTERN QString databaseDrivers()
{
  return createFilters( "database" );
}

QString QgsOgrProvider::databaseDrivers() const
{
  return createFilters( "database" );
}

QGISEXTERN QString protocolDrivers()
{
  return createFilters( "protocol" );
}

QString QgsOgrProvider::protocolDrivers() const
{
  return createFilters( "protocol" );
}

QGISEXTERN QString directoryDrivers()
{
  return  createFilters( "directory" );
}

QString QgsOgrProvider::directoryDrivers() const
{
  return  createFilters( "directory" );
}

QGISEXTERN QStringList fileExtensions()
{
  return  createFilters( "extensions" ).split( "|" );
}

QGISEXTERN QStringList wildcards()
{
  return  createFilters( "wildcards" ).split( "|" );
}


/**
 * Class factory to return a pointer to a newly created
 * QgsOgrProvider object
 */
QGISEXTERN QgsOgrProvider * classFactory( const QString *uri )
{
  return new QgsOgrProvider( *uri );
}



/** Required key function (used to map the plugin to a data store type)
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

/**Creates an empty data source
@param uri location to store the file(s)
@param format data format (e.g. "ESRI Shapefile"
@param vectortype point/line/polygon or multitypes
@param attributes a list of name/type pairs for the initial attributes
@return true in case of success*/
QGISEXTERN bool createEmptyDataSource( const QString &uri,
                                       const QString &format,
                                       const QString &encoding,
                                       QGis::WkbType vectortype,
                                       const std::list<std::pair<QString, QString> > &attributes,
                                       const QgsCoordinateReferenceSystem *srs = NULL )
{
  QgsDebugMsg( QString( "Creating empty vector layer with format: %1" ).arg( format ) );

  OGRSFDriverH driver;
  QgsApplication::registerOgrDrivers();
  driver = OGRGetDriverByName( format.toAscii() );
  if ( !driver )
  {
    return false;
  }

  QString driverName = OGR_Dr_GetName( driver );

  if ( driverName == "ESRI Shapefile" )
  {
    if ( !uri.endsWith( ".shp", Qt::CaseInsensitive ) )
    {
      return false;
    }

    // check for duplicate fieldnames
    QSet<QString> fieldNames;
    std::list<std::pair<QString, QString> >::const_iterator fldIt;
    for ( fldIt = attributes.begin(); fldIt != attributes.end(); ++fldIt )
    {
      QString name = fldIt->first.left( 10 );
      if ( fieldNames.contains( name ) )
      {
        QgsDebugMsg( QString( "duplicate field (10 significant characters): %1" ).arg( name ) );
        return false;
      }
      fieldNames << name;
    }

    QgsVectorFileWriter::deleteShapeFile( uri );
  }
  else
  {
    QFile::remove( uri );
  }

  OGRDataSourceH dataSource;
  dataSource = OGR_Dr_CreateDataSource( driver, TO8F( uri ), NULL );
  if ( !dataSource )
  {
    return false;
  }

  //consider spatial reference system
  OGRSpatialReferenceH reference = NULL;

  QgsCoordinateReferenceSystem mySpatialRefSys;
  if ( srs )
  {
    mySpatialRefSys = *srs;
  }
  else
  {
    mySpatialRefSys.validate();
  }

  QString myWkt = mySpatialRefSys.toWkt();

  if ( !myWkt.isNull()  &&  myWkt.length() != 0 )
  {
    reference = OSRNewSpatialReference( myWkt.toLocal8Bit().data() );
  }

  // Map the qgis geometry type to the OGR geometry type
  OGRwkbGeometryType OGRvectortype = wkbUnknown;
  switch ( vectortype )
  {
    case QGis::WKBPoint:
      OGRvectortype = wkbPoint;
      break;
    case QGis::WKBLineString:
      OGRvectortype = wkbLineString;
      break;
    case QGis::WKBPolygon:
      OGRvectortype = wkbPolygon;
      break;
    case QGis::WKBMultiPoint:
      OGRvectortype = wkbMultiPoint;
      break;
    case QGis::WKBMultiLineString:
      OGRvectortype = wkbMultiLineString;
      break;
    case QGis::WKBMultiPolygon:
      OGRvectortype = wkbMultiPolygon;
      break;
    default:
    {
      QgsDebugMsg( QString( "Unknown vector type of: %1" ).arg(( int )( vectortype ) ) );
      return false;
      break;
    }
  }

  OGRLayerH layer;
  layer = OGR_DS_CreateLayer( dataSource, TO8F( QFileInfo( uri ).completeBaseName() ), reference, OGRvectortype, NULL );
  if ( !layer )
  {
    return false;
  }

  //create the attribute fields

  QTextCodec* codec = QTextCodec::codecForName( encoding.toLocal8Bit().data() );
  if ( !codec )
  {
    // fall back to "System" codec
    codec = QTextCodec::codecForLocale();
    Q_ASSERT( codec );
  }


  for ( std::list<std::pair<QString, QString> >::const_iterator it = attributes.begin(); it != attributes.end(); ++it )
  {
    QStringList fields = it->second.split( ";" );

    if ( fields.size() == 0 )
      continue;

    int width = fields.size() > 1 ? fields[1].toInt() : -1;
    int precision = fields.size() > 2 ? fields[2].toInt() : -1;

    OGRFieldDefnH field;
    if ( fields[0] == "Real" )
    {
      if ( width < 0 )
        width = 32;
      if ( precision < 0 )
        precision = 3;

      field = OGR_Fld_Create( TO8( it->first ), OFTReal );
      OGR_Fld_SetWidth( field, width );
      OGR_Fld_SetPrecision( field, precision );
    }
    else if ( fields[0] == "Integer" )
    {
      if ( width < 0 || width > 10 )
        width = 10;

      field = OGR_Fld_Create( TO8( it->first ), OFTInteger );
      // limit to 10.  otherwise OGR sets it to 11 and recognizes as OFTDouble later
      OGR_Fld_SetWidth( field, width );
    }
    else if ( fields[0] == "String" )
    {
      if ( width < 0 || width > 255 )
        width = 255;

      field = OGR_Fld_Create( TO8( it->first ), OFTString );
      OGR_Fld_SetWidth( field, width );
    }
    else
    {
      continue;
    }

    if ( OGR_L_CreateField( layer, field, true ) != OGRERR_NONE )
    {
      QgsLogger::warning( "creation of field failed" );
    }
  }

  OGR_DS_Destroy( dataSource );

  if ( driverName == "ESRI Shapefile" )
  {
    QString layerName = uri.left( uri.indexOf( ".shp", Qt::CaseInsensitive ) );
    QFile prjFile( layerName + ".qpj" );
    if ( prjFile.open( QIODevice::WriteOnly ) )
    {
      QTextStream prjStream( &prjFile );
      prjStream << myWkt.toLocal8Bit().data() << endl;
      prjFile.close();
    }
    else
    {
      QgsDebugMsg( "Couldn't open file " + layerName + ".qpj" );
    }
  }

  QgsDebugMsg( QString( "GDAL Version number %1" ).arg( GDAL_VERSION_NUM ) );
#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1310
  if ( reference )
  {
    OSRRelease( reference );
  }
#endif //GDAL_VERSION_NUM
  return true;
}

bool QgsOgrProvider::crsFromWkt( QgsCoordinateReferenceSystem &srs, const char *wkt )
{
  void *hCRS = OSRNewSpatialReference( NULL );

  if ( OSRImportFromWkt( hCRS, ( char ** ) &wkt ) == OGRERR_NONE )
  {
    if ( OSRAutoIdentifyEPSG( hCRS ) == OGRERR_NONE )
    {
      QString authid = QString( "%1:%2" )
                       .arg( OSRGetAuthorityName( hCRS, NULL ) )
                       .arg( OSRGetAuthorityCode( hCRS, NULL ) );
      QgsDebugMsg( "authid recognized as " + authid );
      srs.createFromOgcWmsCrs( authid );
    }
    else
    {
      // get the proj4 text
      char *pszProj4;
      OSRExportToProj4( hCRS, &pszProj4 );
      QgsDebugMsg( pszProj4 );
      OGRFree( pszProj4 );

      char *pszWkt = NULL;
      OSRExportToWkt( hCRS, &pszWkt );
      QString myWktString = QString( pszWkt );
      OGRFree( pszWkt );

      // create CRS from Wkt
      srs.createFromWkt( myWktString );
    }
  }

  OSRRelease( hCRS );

  return srs.isValid();
}


QgsCoordinateReferenceSystem QgsOgrProvider::crs()
{
  QgsDebugMsg( "entering." );

  QgsCoordinateReferenceSystem srs;

  if ( ogrDriver )
  {
    QString driverName = OGR_Dr_GetName( ogrDriver );

    if ( driverName == "ESRI Shapefile" )
    {
      QString layerName = mFilePath.left( mFilePath.indexOf( ".shp", Qt::CaseInsensitive ) );
      QFile prjFile( layerName + ".qpj" );
      if ( prjFile.open( QIODevice::ReadOnly ) )
      {
        QTextStream prjStream( &prjFile );
        QString myWktString = prjStream.readLine();
        prjFile.close();

        if ( crsFromWkt( srs, myWktString.toUtf8().constData() ) )
          return srs;
      }
    }
  }

  OGRSpatialReferenceH mySpatialRefSys = OGR_L_GetSpatialRef( ogrLayer );
  if ( mySpatialRefSys )
  {
    // get the proj4 text
    char *pszProj4;
    OSRExportToProj4( mySpatialRefSys, &pszProj4 );
    QgsDebugMsg( pszProj4 );
    OGRFree( pszProj4 );

    char *pszWkt = NULL;
    OSRExportToWkt( mySpatialRefSys, &pszWkt );
    crsFromWkt( srs, pszWkt );
    OGRFree( pszWkt );
  }
  else
  {
    QgsDebugMsg( "no spatial reference found" );
  }

  return srs;
}

void QgsOgrProvider::uniqueValues( int index, QList<QVariant> &uniqueValues, int limit )
{
  uniqueValues.clear();

  QgsField fld = mAttributeFields.value( index );
  if ( fld.name().isNull() )
  {
    return; //not a provider field
  }

  QString theLayerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) );

  QString sql = QString( "SELECT DISTINCT %1 FROM %2" )
                .arg( quotedIdentifier( fld.name() ) )
                .arg( quotedIdentifier( theLayerName ) );

  if ( !mSubsetString.isEmpty() )
  {
    sql += QString( " WHERE %1" ).arg( mSubsetString );
  }

  sql += QString( " ORDER BY %1" ).arg( quotedIdentifier( fld.name() ) );

  QgsDebugMsg( QString( "SQL: %1" ).arg( sql ) );
  OGRLayerH l = OGR_DS_ExecuteSQL( ogrDataSource, TO8( sql ), NULL, "SQL" );
  if ( l == 0 )
    return QgsVectorDataProvider::uniqueValues( index, uniqueValues, limit );

  OGRFeatureH f;
  while ( 0 != ( f = OGR_L_GetNextFeature( l ) ) )
  {
    uniqueValues << convertValue( fld.type(), mEncoding->toUnicode( OGR_F_GetFieldAsString( f, 0 ) ) );
    OGR_F_Destroy( f );

    if ( limit >= 0 && uniqueValues.size() >= limit )
      break;
  }

  OGR_DS_ReleaseResultSet( ogrDataSource, l );
}

QVariant QgsOgrProvider::minimumValue( int index )
{
  QgsFieldMap::const_iterator attIt = mAttributeFields.find( index );
  if ( attIt == mAttributeFields.constEnd() )
  {
    return QVariant();
  }
  const QgsField& fld = attIt.value();

  QString theLayerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) );

  QString sql = QString( "SELECT MIN(%1) FROM %2" )
                .arg( quotedIdentifier( fld.name() ) )
                .arg( quotedIdentifier( theLayerName ) );

  if ( !mSubsetString.isEmpty() )
  {
    sql += QString( " WHERE %1" ).arg( mSubsetString );
  }

  OGRLayerH l = OGR_DS_ExecuteSQL( ogrDataSource, TO8( sql ), NULL, "SQL" );

  if ( l == 0 )
    return QgsVectorDataProvider::minimumValue( index );

  OGRFeatureH f = OGR_L_GetNextFeature( l );
  if ( f == 0 )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, l );
    return QVariant();
  }

  QVariant value = convertValue( fld.type(), mEncoding->toUnicode( OGR_F_GetFieldAsString( f, 0 ) ) );
  OGR_F_Destroy( f );

  OGR_DS_ReleaseResultSet( ogrDataSource, l );

  return value;
}

QVariant QgsOgrProvider::maximumValue( int index )
{
  QgsFieldMap::const_iterator attIt = mAttributeFields.find( index );
  if ( attIt == mAttributeFields.constEnd() )
  {
    return QVariant();
  }
  const QgsField& fld = mAttributeFields[index];

  QString theLayerName = OGR_FD_GetName( OGR_L_GetLayerDefn( ogrLayer ) );

  QString sql = QString( "SELECT MAX(%1) FROM %2" )
                .arg( quotedIdentifier( fld.name() ) )
                .arg( quotedIdentifier( theLayerName ) );

  if ( !mSubsetString.isEmpty() )
  {
    sql += QString( " WHERE %1" ).arg( mSubsetString );
  }

  OGRLayerH l = OGR_DS_ExecuteSQL( ogrDataSource, TO8( sql ), NULL, "SQL" );
  if ( l == 0 )
    return QgsVectorDataProvider::maximumValue( index );

  OGRFeatureH f = OGR_L_GetNextFeature( l );
  if ( f == 0 )
  {
    OGR_DS_ReleaseResultSet( ogrDataSource, l );
    return QVariant();
  }

  QVariant value = convertValue( fld.type(), mEncoding->toUnicode( OGR_F_GetFieldAsString( f, 0 ) ) );
  OGR_F_Destroy( f );

  OGR_DS_ReleaseResultSet( ogrDataSource, l );

  return value;
}

QString QgsOgrProvider::quotedIdentifier( QString field )
{
  field.replace( '\\', "\\\\" );
  field.replace( '"', "\\\"" );
  field.replace( "'", "\\'" );
  return field.prepend( "\"" ).append( "\"" );
}

bool QgsOgrProvider::syncToDisc()
{
  OGR_L_SyncToDisk( ogrLayer );

  //for shapefiles: is there already a spatial index?
  if ( !mFilePath.isEmpty() )
  {
    QFileInfo fi( mFilePath );

    //remove the suffix and add .qix
    int suffixLength = fi.suffix().length();
    if ( suffixLength > 0 )
    {
      QString indexFilePath = mFilePath;
      indexFilePath.chop( suffixLength );
      indexFilePath.append( "qix" );
      QFile indexFile( indexFilePath );
      if ( indexFile.exists() ) //there is already a spatial index file
      {
        //the already existing spatial index is removed automatically by OGR
        return createSpatialIndex();
      }
    }
  }

  return true;
}

void QgsOgrProvider::recalculateFeatureCount()
{
  OGRGeometryH filter = OGR_L_GetSpatialFilter( ogrLayer );
  if ( filter )
  {
    filter = OGR_G_Clone( filter );
    OGR_L_SetSpatialFilter( ogrLayer, 0 );
  }

  // feature count returns number of features within current spatial filter
  // so we remove it if there's any and then put it back
  featuresCounted = OGR_L_GetFeatureCount( ogrLayer, true );

  if ( filter )
  {
    OGR_L_SetSpatialFilter( ogrLayer, filter );
  }
}

QGISEXTERN int dataCapabilities()
{
  return  QgsDataProvider::File | QgsDataProvider::Dir;
}

QgsOgrLayerItem::QgsOgrLayerItem( QgsDataItem* parent,
                                  QString name, QString path, QString uri, LayerType layerType )
    : QgsLayerItem( parent, name, path, uri, layerType, "ogr" )
{
}

QgsOgrLayerItem::~QgsOgrLayerItem()
{
}

QgsLayerItem::Capability QgsOgrLayerItem::capabilities()
{
  QgsDebugMsg( "mPath = " + mPath );
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = OGROpen( TO8F( mPath ), true, &hDriver );

  if ( !hDataSource )
    return NoCapabilities;

  QString  driverName = OGR_Dr_GetName( hDriver );
  OGR_DS_Destroy( hDataSource );

  if ( driverName == "ESRI Shapefile" )
    return SetCrs;

  return NoCapabilities;
}

bool QgsOgrLayerItem::setCrs( QgsCoordinateReferenceSystem crs )
{
  QgsDebugMsg( "mPath = " + mPath );
  OGRRegisterAll();
  OGRSFDriverH hDriver;
  OGRDataSourceH hDataSource = OGROpen( TO8F( mPath ), true, &hDriver );

  if ( !hDataSource )
    return false;

  QString  driverName = OGR_Dr_GetName( hDriver );
  OGR_DS_Destroy( hDataSource );

  // we are able to assign CRS only to shapefiles :-(
  if ( driverName == "ESRI Shapefile" )
  {
    QString layerName = mPath.left( mPath.indexOf( ".shp", Qt::CaseInsensitive ) );
    QString wkt = crs.toWkt();

    // save ordinary .prj file
    OGRSpatialReferenceH hSRS = OSRNewSpatialReference( wkt.toLocal8Bit().data() );
    OSRMorphToESRI( hSRS ); // this is the important stuff for shapefile .prj
    char* pszOutWkt = NULL;
    OSRExportToWkt( hSRS, &pszOutWkt );
    QFile prjFile( layerName + ".prj" );
    if ( prjFile.open( QIODevice::WriteOnly ) )
    {
      QTextStream prjStream( &prjFile );
      prjStream << pszOutWkt << endl;
      prjFile.close();
    }
    else
    {
      QgsDebugMsg( "Couldn't open file " + layerName + ".prj" );
      return false;
    }
    OSRDestroySpatialReference( hSRS );
    CPLFree( pszOutWkt );

    // save qgis-specific .qpj file (maybe because of better wkt compatibility?)
    QFile qpjFile( layerName + ".qpj" );
    if ( qpjFile.open( QIODevice::WriteOnly ) )
    {
      QTextStream qpjStream( &qpjFile );
      qpjStream << wkt.toLocal8Bit().data() << endl;
      qpjFile.close();
    }
    else
    {
      QgsDebugMsg( "Couldn't open file " + layerName + ".qpj" );
      return false;
    }

    return true;
  }

  // It it is impossible to assign a crs to an existing layer
  // No OGR_L_SetSpatialRef : http://trac.osgeo.org/gdal/ticket/4032
  return false;
}

QGISEXTERN QgsDataItem * dataItem( QString thePath, QgsDataItem* parentItem )
{
  if ( thePath.isEmpty() )
    return 0;

  QFileInfo info( thePath );
  if ( info.isFile() )
  {
    // We have to filter by extensions, otherwise e.g. all Shapefile files are displayed
    // because OGR drive can open also .dbf, .shx.
    QStringList myExtensions = fileExtensions();
    if ( myExtensions.indexOf( info.suffix().toLower() ) < 0 )
    {
      bool matches = false;
      foreach( QString wildcard, wildcards() )
      {
        QRegExp rx( wildcard, Qt::CaseInsensitive, QRegExp::Wildcard );
        if ( rx.exactMatch( info.fileName() ) )
        {
          matches = true;
          break;
        }
      }
      if ( !matches )
        return 0;
    }

    // .dbf should probably appear if .shp is not present
    if ( info.suffix().toLower() == "dbf" )
    {
      QString pathShp = thePath.left( thePath.count() - 4 ) + ".shp";
      if ( QFileInfo( pathShp ).exists() )
        return 0;
    }

    OGRRegisterAll();
    OGRSFDriverH hDriver;
    OGRDataSourceH hDataSource = OGROpen( TO8F( thePath ), false, &hDriver );

    if ( !hDataSource )
      return 0;

    QString  driverName = OGR_Dr_GetName( hDriver );
    QgsDebugMsg( "OGR Driver : " + driverName );

    int numLayers = OGR_DS_GetLayerCount( hDataSource );

    if ( numLayers == 0 )
    {
      OGR_DS_Destroy( hDataSource );
      return 0;
    }

    QgsDataCollectionItem * collection = 0;
    if ( numLayers > 1 )
    {
      collection = new QgsDataCollectionItem( parentItem, info.fileName(), thePath );
    }

    for ( int i = 0; i < numLayers; i++ )
    {
      OGRLayerH hLayer = OGR_DS_GetLayer( hDataSource, i );
      OGRFeatureDefnH hDef = OGR_L_GetLayerDefn( hLayer );

      QgsLayerItem::LayerType layerType = QgsLayerItem::Vector;
      int ogrType = getOgrGeomType( hLayer );
      switch ( ogrType )
      {
        case wkbUnknown:
        case wkbGeometryCollection:
          break;
        case wkbNone:
          layerType = QgsLayerItem::TableLayer;
          break;
        case wkbPoint:
        case wkbMultiPoint:
        case wkbPoint25D:
        case wkbMultiPoint25D:
          layerType = QgsLayerItem::Point;
          break;
        case wkbLineString:
        case wkbMultiLineString:
        case wkbLineString25D:
        case wkbMultiLineString25D:
          layerType = QgsLayerItem::Line;
          break;
        case wkbPolygon:
        case wkbMultiPolygon:
        case wkbPolygon25D:
        case wkbMultiPolygon25D:
          layerType = QgsLayerItem::Polygon;
          break;
        default:
          break;
      }

      QgsDebugMsg( QString( "ogrType = %1 layertype = %2" ).arg( ogrType ).arg( layerType ) );

      QString name = info.fileName();

      QString layerName = FROM8( OGR_FD_GetName( hDef ) );
      QgsDebugMsg( "OGR layer name : " + layerName );

      QString path = thePath;
      if ( numLayers > 1 )
      {
        name = layerName;
        path += "/" + name;
      }

      QString layerUri = thePath + "|layerid=" + QString::number( i );
      QgsDebugMsg( "OGR layer uri : " + layerUri );

      QgsOgrLayerItem * item = new QgsOgrLayerItem( collection ? collection : parentItem, name, path, layerUri, layerType );
      if ( numLayers == 1 )
      {
        OGR_DS_Destroy( hDataSource );
        return item;
      }
      collection->addChild( item );
    }
    collection->setPopulated();
    OGR_DS_Destroy( hDataSource );
    return collection;
  }

  return 0;
}

QGISEXTERN QgsVectorLayerImport::ImportError createEmptyLayer(
  const QString& uri,
  const QgsFieldMap &fields,
  QGis::WkbType wkbType,
  const QgsCoordinateReferenceSystem *srs,
  bool overwrite,
  QMap<int, int> *oldToNewAttrIdxMap,
  QString *errorMessage,
  const QMap<QString, QVariant> *options )
{
  return QgsOgrProvider::createEmptyLayer(
           uri, fields, wkbType, srs, overwrite,
           oldToNewAttrIdxMap, errorMessage, options
         );
}
