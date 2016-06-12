/***************************************************************************
    qgswfscapabilities.cpp
    ---------------------
    begin                : October 2011
    copyright            : (C) 2011 by Martin Dobias
                           (C) 2016 by Even Rouault
    email                : wonder dot sk at gmail dot com
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgswfscapabilities.h"
#include "qgswfsconstants.h"
#include "qgswfsutils.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"
#include "qgscrscache.h"

#include <QDomDocument>
#include <QSettings>
#include <QStringList>

QgsWFSCapabilities::QgsWFSCapabilities( const QString& theUri )
    : QgsWFSRequest( theUri )
{
  connect( this, SIGNAL( downloadFinished() ), this, SLOT( capabilitiesReplyFinished() ) );
}

QgsWFSCapabilities::~QgsWFSCapabilities()
{
}

bool QgsWFSCapabilities::requestCapabilities( bool synchronous )
{
  QUrl url( baseURL() );
  url.addQueryItem( "REQUEST", "GetCapabilities" );

  const QString& version = mUri.version();
  if ( version == QgsWFSConstants::VERSION_AUTO )
    // MapServer honours the order with the first value being the preferred one
    url.addQueryItem( "ACCEPTVERSIONS", "2.0.0,1.1.0,1.0.0" );
  else
    url.addQueryItem( "VERSION", version );

  if ( !sendGET( url, synchronous, false ) )
  {
    emit gotCapabilities();
    return false;
  }
  return true;
}

QgsWFSCapabilities::Capabilities::Capabilities()
{
  clear();
}

void QgsWFSCapabilities::Capabilities::clear()
{
  maxFeatures = 0;
  supportsHits = false;
  supportsPaging = false;
  supportsJoins = false;
  version = "";
  featureTypes.clear();
  spatialPredicatesList.clear();
  functionList.clear();
  setAllTypenames.clear();
  mapUnprefixedTypenameToPrefixedTypename.clear();
  setAmbiguousUnprefixedTypename.clear();
  useEPSGColumnFormat = false;
}

QString QgsWFSCapabilities::Capabilities::addPrefixIfNeeded( const QString& name ) const
{
  if ( name.contains( ':' ) )
    return name;
  if ( setAmbiguousUnprefixedTypename.contains( name ) )
    return "";
  return mapUnprefixedTypenameToPrefixedTypename[name];
}

void QgsWFSCapabilities::capabilitiesReplyFinished()
{
  const QByteArray& buffer = mResponse;

  QgsDebugMsg( "parsing capabilities: " + buffer );

  // parse XML
  QString capabilitiesDocError;
  QDomDocument capabilitiesDocument;
  if ( !capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
  {
    mErrorCode = QgsWFSRequest::XmlError;
    mErrorMessage = capabilitiesDocError;
    emit gotCapabilities();
    return;
  }

  QDomElement doc = capabilitiesDocument.documentElement();

  // handle exceptions
  if ( doc.tagName() == "ExceptionReport" )
  {
    QDomNode ex = doc.firstChild();
    QString exc = ex.toElement().attribute( "exceptionCode", "Exception" );
    QDomElement ext = ex.firstChild().toElement();
    mErrorCode = QgsWFSRequest::ServerExceptionError;
    mErrorMessage = exc + ": " + ext.firstChild().nodeValue();
    emit gotCapabilities();
    return;
  }

  mCaps.clear();

  //test wfs version
  mCaps.version = doc.attribute( "version" );
  if ( !mCaps.version.startsWith( "1.0" ) &&
       !mCaps.version.startsWith( "1.1" ) &&
       !mCaps.version.startsWith( "2.0" ) )
  {
    mErrorCode = WFSVersionNotSupported;
    mErrorMessage = tr( "WFS version %1 not supported" ).arg( mCaps.version );
    emit gotCapabilities();
    return;
  }

  // WFS 2.0 implementation are supposed to implement resultType=hits, and some
  // implementations (GeoServer) might advertize it, whereas others (MapServer) do not.
  // WFS 1.1 implementation too I think, but in the examples of the GetCapabilites
  // response of the WFS 1.1 standard (and in common implementations), this is
  // explictly advertized
  if ( mCaps.version.startsWith( "2.0" ) )
    mCaps.supportsHits = true;

  // Note: for conveniency, we do not use the elementsByTagNameNS() method as
  // the WFS and OWS namespaces URI are not the same in all versions

  // find <ows:OperationsMetadata>
  QDomElement operationsMetadataElem = doc.firstChildElement( "OperationsMetadata" );
  if ( !operationsMetadataElem.isNull() )
  {
    QDomNodeList contraintList = operationsMetadataElem.elementsByTagName( "Constraint" );
    for ( int i = 0; i < contraintList.size(); ++i )
    {
      QDomElement contraint = contraintList.at( i ).toElement();
      if ( contraint.attribute( "name" ) == "DefaultMaxFeatures" /* WFS 1.1 */ )
      {
        QDomElement value = contraint.firstChildElement( "Value" );
        if ( !value.isNull() )
        {
          mCaps.maxFeatures = value.text().toInt();
          QgsDebugMsg( QString( "maxFeatures: %1" ).arg( mCaps.maxFeatures ) );
        }
      }
      else if ( contraint.attribute( "name" ) == "CountDefault" /* WFS 2.0 (e.g. MapServer) */ )
      {
        QDomElement value = contraint.firstChildElement( "DefaultValue" );
        if ( !value.isNull() )
        {
          mCaps.maxFeatures = value.text().toInt();
          QgsDebugMsg( QString( "maxFeatures: %1" ).arg( mCaps.maxFeatures ) );
        }
      }
      else if ( contraint.attribute( "name" ) == "ImplementsResultPaging" /* WFS 2.0 */ )
      {
        QDomElement value = contraint.firstChildElement( "DefaultValue" );
        if ( !value.isNull() && value.text() == "TRUE" )
        {
          mCaps.supportsPaging = true;
          QgsDebugMsg( "Supports paging" );
        }
      }
      else if ( contraint.attribute( "name" ) == "ImplementsStandardJoins" ||
                contraint.attribute( "name" ) == "ImplementsSpatialJoins" /* WFS 2.0 */ )
      {
        QDomElement value = contraint.firstChildElement( "DefaultValue" );
        if ( !value.isNull() && value.text() == "TRUE" )
        {
          mCaps.supportsJoins = true;
          QgsDebugMsg( "Supports joins" );
        }
      }
    }

    // In WFS 2.0, max features can also be set in Operation.GetFeature (e.g. GeoServer)
    // and we are also interested by resultType=hits for WFS 1.1
    QDomNodeList operationList = operationsMetadataElem.elementsByTagName( "Operation" );
    for ( int i = 0; i < operationList.size(); ++i )
    {
      QDomElement operation = operationList.at( i ).toElement();
      if ( operation.attribute( "name" ) == "GetFeature" )
      {
        QDomNodeList operationContraintList = operation.elementsByTagName( "Constraint" );
        for ( int j = 0; j < operationContraintList.size(); ++j )
        {
          QDomElement contraint = operationContraintList.at( j ).toElement();
          if ( contraint.attribute( "name" ) == "CountDefault" )
          {
            QDomElement value = contraint.firstChildElement( "DefaultValue" );
            if ( !value.isNull() )
            {
              mCaps.maxFeatures = value.text().toInt();
              QgsDebugMsg( QString( "maxFeatures: %1" ).arg( mCaps.maxFeatures ) );
            }
            break;
          }
        }

        QDomNodeList parameterList = operation.elementsByTagName( "Parameter" );
        for ( int j = 0; j < parameterList.size(); ++j )
        {
          QDomElement parameter = parameterList.at( j ).toElement();
          if ( parameter.attribute( "name" ) == "resultType" )
          {
            QDomNodeList valueList = parameter.elementsByTagName( "Value" );
            for ( int k = 0; k < valueList.size(); ++k )
            {
              QDomElement value = valueList.at( k ).toElement();
              if ( value.text() == "hits" )
              {
                mCaps.supportsHits = true;
                QgsDebugMsg( "Support hits" );
                break;
              }
            }
          }
        }

        break;
      }
    }
  }

  //go to <FeatureTypeList>
  QDomElement featureTypeListElem = doc.firstChildElement( "FeatureTypeList" );
  if ( featureTypeListElem.isNull() )
  {
    emit gotCapabilities();
    return;
  }

  // Parse operations supported for all feature types
  bool insertCap, updateCap, deleteCap;
  parseSupportedOperations( featureTypeListElem.firstChildElement( "Operations" ),
                            insertCap,
                            updateCap,
                            deleteCap );

  // get the <FeatureType> elements
  QDomNodeList featureTypeList = featureTypeListElem.elementsByTagName( "FeatureType" );
  for ( int i = 0; i < featureTypeList.size(); ++i )
  {
    FeatureType featureType;
    QDomElement featureTypeElem = featureTypeList.at( i ).toElement();

    //Name
    QDomNodeList nameList = featureTypeElem.elementsByTagName( "Name" );
    if ( nameList.length() > 0 )
    {
      featureType.name = nameList.at( 0 ).toElement().text();
    }
    //Title
    QDomNodeList titleList = featureTypeElem.elementsByTagName( "Title" );
    if ( titleList.length() > 0 )
    {
      featureType.title = titleList.at( 0 ).toElement().text();
    }
    //Abstract
    QDomNodeList abstractList = featureTypeElem.elementsByTagName( "Abstract" );
    if ( abstractList.length() > 0 )
    {
      featureType.abstract = abstractList.at( 0 ).toElement().text();
    }

    //DefaultSRS is always the first entry in the feature srs list
    QDomNodeList defaultCRSList = featureTypeElem.elementsByTagName( "DefaultSRS" );
    if ( defaultCRSList.length() == 0 )
      // In WFS 2.0, this is spelled DefaultCRS...
      defaultCRSList = featureTypeElem.elementsByTagName( "DefaultCRS" );
    if ( defaultCRSList.length() > 0 )
    {
      QString srsname( defaultCRSList.at( 0 ).toElement().text() );
      // Some servers like Geomedia advertize EPSG:XXXX even in WFS 1.1 or 2.0
      if ( srsname.startsWith( "EPSG:" ) )
        mCaps.useEPSGColumnFormat = true;
      featureType.crslist.append( NormalizeSRSName( srsname ) );
    }

    //OtherSRS
    QDomNodeList otherCRSList = featureTypeElem.elementsByTagName( "OtherSRS" );
    if ( otherCRSList.length() == 0 )
      // In WFS 2.0, this is spelled OtherCRS...
      otherCRSList = featureTypeElem.elementsByTagName( "OtherCRS" );
    for ( int i = 0; i < otherCRSList.size(); ++i )
    {
      featureType.crslist.append( NormalizeSRSName( otherCRSList.at( i ).toElement().text() ) );
    }

    //Support <SRS> for compatibility with older versions
    QDomNodeList srsList = featureTypeElem.elementsByTagName( "SRS" );
    for ( int i = 0; i < srsList.size(); ++i )
    {
      featureType.crslist.append( NormalizeSRSName( srsList.at( i ).toElement().text() ) );
    }

    // Get BBox WFS 1.0 way
    QDomElement latLongBB = featureTypeElem.firstChildElement( "LatLongBoundingBox" );
    if ( latLongBB.hasAttributes() )
    {
      // Despite the name LatLongBoundingBox, the coordinates are supposed to
      // be expressed in <SRS>. From the WFS schema;
      // <!-- The LatLongBoundingBox element is used to indicate the edges of
      // an enclosing rectangle in the SRS of the associated feature type.
      featureType.bbox = QgsRectangle(
                           latLongBB.attribute( "minx" ).toDouble(),
                           latLongBB.attribute( "miny" ).toDouble(),
                           latLongBB.attribute( "maxx" ).toDouble(),
                           latLongBB.attribute( "maxy" ).toDouble() );
      featureType.bboxSRSIsWGS84 = false;

      // But some servers do not honour this and systematically reproject to WGS84
      // such as GeoServer. See http://osgeo-org.1560.x6.nabble.com/WFS-LatLongBoundingBox-td3813810.html
      // This is also true of TinyOWS
      if ( !featureType.crslist.isEmpty() &&
           featureType.bbox.xMinimum() >= -180 && featureType.bbox.yMinimum() >= -90 &&
           featureType.bbox.xMaximum() <= 180 && featureType.bbox.yMaximum() < 90 )
      {
        QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsByOgcWmsCrs( featureType.crslist[0] );
        if ( !crs.geographicFlag() )
        {
          // If the CRS is projected then check that projecting the corner of the bbox, assumed to be in WGS84,
          // into the CRS, and then back to WGS84, works (check that we are in the validity area)
          QgsCoordinateReferenceSystem crsWGS84 = QgsCRSCache::instance()->crsByOgcWmsCrs( "CRS:84" );
          QgsCoordinateTransform ct( crsWGS84, crs );

          QgsPoint ptMin( featureType.bbox.xMinimum(), featureType.bbox.yMinimum() );
          QgsPoint ptMinBack( ct.transform( ct.transform( ptMin, QgsCoordinateTransform::ForwardTransform ), QgsCoordinateTransform::ReverseTransform ) );
          QgsPoint ptMax( featureType.bbox.xMaximum(), featureType.bbox.yMaximum() );
          QgsPoint ptMaxBack( ct.transform( ct.transform( ptMax, QgsCoordinateTransform::ForwardTransform ), QgsCoordinateTransform::ReverseTransform ) );

          QgsDebugMsg( featureType.bbox.toString() );
          QgsDebugMsg( ptMinBack.toString() );
          QgsDebugMsg( ptMaxBack.toString() );

          if ( fabs( featureType.bbox.xMinimum() - ptMinBack.x() ) < 1e-5 &&
               fabs( featureType.bbox.yMinimum() - ptMinBack.y() ) < 1e-5 &&
               fabs( featureType.bbox.xMaximum() - ptMaxBack.x() ) < 1e-5 &&
               fabs( featureType.bbox.yMaximum() - ptMaxBack.y() ) < 1e-5 )
          {
            QgsDebugMsg( "Values of LatLongBoundingBox are consistent with WGS84 long/lat bounds, so as the CRS is projected, assume they are indeed in WGS84 and not in the CRS units" );
            featureType.bboxSRSIsWGS84 = true;
          }
        }
      }
    }
    else
    {
      // WFS 1.1 way
      QDomElement WGS84BoundingBox = featureTypeElem.firstChildElement( "WGS84BoundingBox" );
      if ( !WGS84BoundingBox.isNull() )
      {
        QDomElement lowerCorner = WGS84BoundingBox.firstChildElement( "LowerCorner" );
        QDomElement upperCorner = WGS84BoundingBox.firstChildElement( "UpperCorner" );
        if ( !lowerCorner.isNull() && !upperCorner.isNull() )
        {
          QStringList lowerCornerList = lowerCorner.text().split( " ", QString::SkipEmptyParts );
          QStringList upperCornerList = upperCorner.text().split( " ", QString::SkipEmptyParts );
          if ( lowerCornerList.size() == 2 && upperCornerList.size() == 2 )
          {
            featureType.bbox = QgsRectangle(
                                 lowerCornerList[0].toDouble(),
                                 lowerCornerList[1].toDouble(),
                                 upperCornerList[0].toDouble(),
                                 upperCornerList[1].toDouble() );
            featureType.bboxSRSIsWGS84 = true;
          }
        }
      }
    }

    // Parse Operations specific to the type name
    parseSupportedOperations( featureTypeElem.firstChildElement( "Operations" ),
                              featureType.insertCap,
                              featureType.updateCap,
                              featureType.deleteCap );
    featureType.insertCap |= insertCap;
    featureType.updateCap |= updateCap;
    featureType.deleteCap |= deleteCap;

    mCaps.featureTypes.push_back( featureType );
  }

  Q_FOREACH ( const FeatureType& f, mCaps.featureTypes )
  {
    mCaps.setAllTypenames.insert( f.name );
    QString unprefixed( QgsWFSUtils::removeNamespacePrefix( f.name ) );
    if ( !mCaps.setAmbiguousUnprefixedTypename.contains( unprefixed ) )
    {
      if ( mCaps.mapUnprefixedTypenameToPrefixedTypename.contains( unprefixed ) )
      {
        mCaps.setAmbiguousUnprefixedTypename.insert( unprefixed );
        mCaps.mapUnprefixedTypenameToPrefixedTypename.remove( unprefixed );
      }
      else
      {
        mCaps.mapUnprefixedTypenameToPrefixedTypename[unprefixed] = f.name;
      }
    }
  }

  //go to <Filter_Capabilities>
  QDomElement filterCapabilitiesElem = doc.firstChildElement( "Filter_Capabilities" );
  if ( !filterCapabilitiesElem.isNull() )
    parseFilterCapabilities( filterCapabilitiesElem );

  // Hard-coded functions
  Function f_ST_GeometryFromText( "ST_GeometryFromText", 1, 2 );
  f_ST_GeometryFromText.returnType = "gml:AbstractGeometryType";
  f_ST_GeometryFromText.argumentList << Argument( "wkt", "xs:string" );
  f_ST_GeometryFromText.argumentList << Argument( "srsname", "xs:string" );
  mCaps.functionList << f_ST_GeometryFromText;

  Function f_ST_GeomFromGML( "ST_GeomFromGML", 1 );
  f_ST_GeomFromGML.returnType = "gml:AbstractGeometryType";
  f_ST_GeomFromGML.argumentList << Argument( "gml", "xs:string" );
  mCaps.functionList << f_ST_GeomFromGML;

  Function f_ST_MakeEnvelope( "ST_MakeEnvelope", 4, 5 );
  f_ST_MakeEnvelope.returnType = "gml:AbstractGeometryType";
  f_ST_MakeEnvelope.argumentList << Argument( "minx", "xs:double" );
  f_ST_MakeEnvelope.argumentList << Argument( "miny", "xs:double" );
  f_ST_MakeEnvelope.argumentList << Argument( "maxx", "xs:double" );
  f_ST_MakeEnvelope.argumentList << Argument( "maxy", "xs:double" );
  f_ST_MakeEnvelope.argumentList << Argument( "srsname", "xs:string" );
  mCaps.functionList << f_ST_MakeEnvelope;

  emit gotCapabilities();
}

QString QgsWFSCapabilities::NormalizeSRSName( QString crsName )
{
  QRegExp re( "urn:ogc:def:crs:([^:]+).+([^:]+)", Qt::CaseInsensitive );
  if ( re.exactMatch( crsName ) )
  {
    return re.cap( 1 ) + ':' + re.cap( 2 );
  }
  // urn:x-ogc:def:crs:EPSG:xxxx as returned by http://maps.warwickshire.gov.uk/gs/ows? in WFS 1.1
  QRegExp re2( "urn:x-ogc:def:crs:([^:]+).+([^:]+)", Qt::CaseInsensitive );
  if ( re2.exactMatch( crsName ) )
  {
    return re2.cap( 1 ) + ':' + re2.cap( 2 );
  }
  return crsName;
}

int QgsWFSCapabilities::defaultExpirationInSec()
{
  QSettings s;
  return s.value( "/qgis/defaultCapabilitiesExpiry", "24" ).toInt() * 60 * 60;
}

void QgsWFSCapabilities::parseSupportedOperations( const QDomElement& operationsElem,
    bool& insertCap,
    bool& updateCap,
    bool& deleteCap )
{
  insertCap = false;
  updateCap = false;
  deleteCap = false;

  // TODO: remove me when WFS-T 1.1 or 2.0 is done
  if ( !mCaps.version.startsWith( "1.0" ) )
    return;

  if ( operationsElem.isNull() )
  {
    return;
  }

  QDomNodeList childList = operationsElem.childNodes();
  for ( int i = 0; i < childList.size(); ++i )
  {
    QDomElement elt = childList.at( i ).toElement();
    QString elemName = elt.tagName();
    /* WFS 1.0 */
    if ( elemName == "Insert" )
    {
      insertCap = true;
    }
    else if ( elemName == "Update" )
    {
      updateCap = true;
    }
    else if ( elemName == "Delete" )
    {
      deleteCap = true;
    }
    /* WFS 1.1 */
    else if ( elemName == "Operation" )
    {
      QString elemText = elt.text();
      if ( elemText == "Insert" )
      {
        insertCap = true;
      }
      else if ( elemText == "Update" )
      {
        updateCap = true;
      }
      else if ( elemText == "Delete" )
      {
        deleteCap = true;
      }
    }
  }
}

static QgsWFSCapabilities::Function getSpatialPredicate( const QString& name )
{
  QgsWFSCapabilities::Function f;
  // WFS 1.0 advertize Intersect, but for conveniency we internally convert it to Intersects
  if ( name == "Intersect" )
    f.name = "ST_Intersects";
  else
    f.name = ( name == "BBOX" ) ? "BBOX" : "ST_" + name;
  f.returnType = "xs:boolean";
  if ( name == "DWithin" || name == "Beyond" )
  {
    f.minArgs = 3;
    f.maxArgs = 3;
    f.argumentList << QgsWFSCapabilities::Argument( "geometry", "gml:AbstractGeometryType" );
    f.argumentList << QgsWFSCapabilities::Argument( "geometry", "gml:AbstractGeometryType" );
    f.argumentList << QgsWFSCapabilities::Argument( "distance" );
  }
  else
  {
    f.minArgs = 2;
    f.maxArgs = 2;
    f.argumentList << QgsWFSCapabilities::Argument( "geometry", "gml:AbstractGeometryType" );
    f.argumentList << QgsWFSCapabilities::Argument( "geometry", "gml:AbstractGeometryType" );
  }
  return f;
}

void QgsWFSCapabilities::parseFilterCapabilities( const QDomElement& filterCapabilitiesElem )
{
  // WFS 1.0
  QDomElement spatial_Operators = filterCapabilitiesElem.firstChildElement( "Spatial_Capabilities" ).firstChildElement( "Spatial_Operators" );
  QDomElement spatial_Operator = spatial_Operators.firstChildElement();
  while ( !spatial_Operator.isNull() )
  {
    QString name = spatial_Operator.tagName();
    if ( !name.isEmpty() )
    {
      mCaps.spatialPredicatesList << getSpatialPredicate( name );
    }
    spatial_Operator = spatial_Operator.nextSiblingElement();
  }

  // WFS 1.1 and 2.0
  QDomElement spatialOperators = filterCapabilitiesElem.firstChildElement( "Spatial_Capabilities" ).firstChildElement( "SpatialOperators" );
  QDomElement spatialOperator = spatialOperators.firstChildElement( "SpatialOperator" );
  while ( !spatialOperator.isNull() )
  {
    QString name = spatialOperator.attribute( "name" );
    if ( !name.isEmpty() )
    {
      mCaps.spatialPredicatesList << getSpatialPredicate( name );
    }
    spatialOperator = spatialOperator.nextSiblingElement( "SpatialOperator" );
  }

  // WFS 1.0
  QDomElement function_Names = filterCapabilitiesElem.firstChildElement( "Scalar_Capabilities" )
                               .firstChildElement( "Arithmetic_Operators" )
                               .firstChildElement( "Functions" )
                               .firstChildElement( "Function_Names" );
  QDomElement function_NameElem = function_Names.firstChildElement( "Function_Name" );
  while ( !function_NameElem.isNull() )
  {
    Function f;
    f.name = function_NameElem.text();
    bool ok;
    int nArgs = function_NameElem.attribute( "nArgs" ).toInt( &ok );
    if ( ok )
    {
      if ( nArgs >= 0 )
      {
        f.minArgs = nArgs;
        f.maxArgs = nArgs;
      }
      else
      {
        f.minArgs = -nArgs;
      }
    }
    mCaps.functionList << f;
    function_NameElem = function_NameElem.nextSiblingElement( "Function_Name" );
  }

  // WFS 1.1
  QDomElement functionNames = filterCapabilitiesElem.firstChildElement( "Scalar_Capabilities" )
                              .firstChildElement( "ArithmeticOperators" )
                              .firstChildElement( "Functions" )
                              .firstChildElement( "FunctionNames" );
  QDomElement functionNameElem = functionNames.firstChildElement( "FunctionName" );
  while ( !functionNameElem.isNull() )
  {
    Function f;
    f.name = functionNameElem.text();
    bool ok;
    int nArgs = functionNameElem.attribute( "nArgs" ).toInt( &ok );
    if ( ok )
    {
      if ( nArgs >= 0 )
      {
        f.minArgs = nArgs;
        f.maxArgs = nArgs;
      }
      else
      {
        f.minArgs = -nArgs;
      }
    }
    mCaps.functionList << f;
    functionNameElem = functionNameElem.nextSiblingElement( "FunctionName" );
  }

  QDomElement functions = filterCapabilitiesElem.firstChildElement( "Functions" );
  QDomElement functionElem = functions.firstChildElement( "Function" );
  while ( !functionElem.isNull() )
  {
    QString name = functionElem.attribute( "name" );
    if ( !name.isEmpty() )
    {
      Function f;
      f.name = name;
      QDomElement returnsElem = functionElem.firstChildElement( "Returns" );
      f.returnType = returnsElem.text();
      QDomElement argumentsElem = functionElem.firstChildElement( "Arguments" );
      QDomElement argumentElem = argumentsElem.firstChildElement( "Argument" );
      while ( !argumentElem.isNull() )
      {
        Argument arg;
        arg.name = argumentElem.attribute( "name" );
        arg.type = argumentElem.firstChildElement( "Type" ).text();
        f.argumentList << arg;
        argumentElem = argumentElem.nextSiblingElement( "Argument" );
      }
      f.minArgs = f.argumentList.count();
      f.maxArgs = f.argumentList.count();
      mCaps.functionList << f;
    }
    functionElem = functionElem.nextSiblingElement( "Function" );
  }
}

QString QgsWFSCapabilities::errorMessageWithReason( const QString& reason )
{
  return tr( "Download of capabilities failed: %1" ).arg( reason );
}
