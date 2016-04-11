/***************************************************************************
                              qgswfsprovider.cpp
                              -------------------
  begin                : July 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsogcutils.h"

#include "qgswfsconstants.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsprovider.h"
#include "qgswfscapabilities.h"
#include "qgswfsdescribefeaturetype.h"
#include "qgswfstransactionrequest.h"
#include "qgswfsshareddata.h"
#include "qgswfsutils.h"

#include <QDomDocument>
#include <QMessageBox>
#include <QDomNodeList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QWidget>
#include <QPair>
#include <QTimer>

#include <cfloat>

static const QString TEXT_PROVIDER_KEY = "WFS";
static const QString TEXT_PROVIDER_DESCRIPTION = "WFS data provider";

QgsWFSProvider::QgsWFSProvider( const QString& uri )
    : QgsVectorDataProvider( uri )
    , mShared( new QgsWFSSharedData( uri ) )
    , mWKBType( QGis::WKBUnknown )
    , mValid( true )
    , mCapabilities( 0 )
{
  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  //create mSourceCRS from url if possible [WBC 111221] refactored from GetFeatureGET()
  QString srsname = mShared->mURI.SRSName();
  if ( !srsname.isEmpty() )
  {
    if ( srsname == "EPSG:900913" )
      mShared->mSourceCRS.createFromOgcWmsCrs( "EPSG:3857" );
    else
      mShared->mSourceCRS.createFromOgcWmsCrs( srsname );
  }

  // Must be called first to establish the version, in case we are in auto-detection
  if ( !getCapabilities() )
  {
    mValid = false;
    return;
  }

  mSubsetString = mShared->mURI.filter();
  mShared->computeFilter();

  //fetch attributes of layer and type of its geometry attribute
  //WBC 111221: extracting geometry type here instead of getFeature allows successful
  //layer creation even when no features are retrieved (due to, e.g., BBOX or FILTER)
  if ( !describeFeatureType( mShared->mGeometryAttribute, mShared->mFields, mWKBType ) )
  {
    mValid = false;
    return;
  }

  //Failed to detect feature type from describeFeatureType -> get first feature from layer to detect type
  if ( mWKBType == QGis::WKBUnknown )
  {
    QgsWFSFeatureDownloader downloader( mShared.data() );
    connect( &downloader, SIGNAL( featureReceived( QVector<QgsWFSFeatureGmlIdPair> ) ),
             this, SLOT( featureReceivedAnalyzeOneFeature( QVector<QgsWFSFeatureGmlIdPair> ) ) );
    downloader.run( false, /* serialize features */
                    1 /* maxfeatures */ );
  }

  qRegisterMetaType<QgsRectangle>( "QgsRectangle" );
}

QgsWFSProvider::~QgsWFSProvider()
{
}

void QgsWFSProvider::featureReceivedAnalyzeOneFeature( QVector<QgsWFSFeatureGmlIdPair> list )
{
  if ( list.size() != 0 )
  {
    QgsFeature feat = list[0].first;
    const QgsGeometry* geometry = feat.constGeometry();
    if ( geometry )
    {
      mWKBType = geometry->wkbType();
    }
  }
}

QString QgsWFSProvider::subsetString()
{
  return mSubsetString;
}

bool QgsWFSProvider::setSubsetString( const QString& theSQL, bool updateFeatureCount )
{
  mSubsetString = theSQL;
  mCacheMinMaxDirty = true;

  // update URI
  mShared->mURI.setFilter( theSQL );
  setDataSourceUri( mShared->mURI.uri() );
  mShared->computeFilter();
  reloadData();
  if ( updateFeatureCount )
    featureCount();
  return true;
}


QgsAbstractFeatureSource* QgsWFSProvider::featureSource() const
{
  QgsWFSFeatureSource *fs = new QgsWFSFeatureSource( this );
  /*connect( fs, SIGNAL( extentRequested( const QgsRectangle & ) ),
           this, SLOT( extendExtent( const QgsRectangle & ) ) );*/
  return fs;
}

void QgsWFSProvider::reloadData()
{
  mShared->invalidateCache();
  QgsVectorDataProvider::reloadData();
}

QGis::WkbType QgsWFSProvider::geometryType() const
{
  return mWKBType;
}

long QgsWFSProvider::featureCount() const
{
  return mShared->getFeatureCount();
}

const QgsFields& QgsWFSProvider::fields() const
{
  return mShared->mFields;
}

QgsCoordinateReferenceSystem QgsWFSProvider::crs()
{
  return mShared->mSourceCRS;
}

QgsRectangle QgsWFSProvider::extent()
{
  return mExtent;
}

bool QgsWFSProvider::isValid()
{
  return mValid;
}

QgsFeatureIterator QgsWFSProvider::getFeatures( const QgsFeatureRequest& request )
{
  return new QgsWFSFeatureIterator( new QgsWFSFeatureSource( this ), true, request );
}

bool QgsWFSProvider::addFeatures( QgsFeatureList &flist )
{
  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  //find out typename from uri and strip namespace prefix
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }
  removeNamespacePrefix( tname );

  //Add the features
  QgsFeatureList::iterator featureIt = flist.begin();
  for ( ; featureIt != flist.end(); ++featureIt )
  {
    //Insert element
    QDomElement insertElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Insert" );
    transactionElem.appendChild( insertElem );

    QDomElement featureElem = transactionDoc.createElementNS( mApplicationNamespace, tname );

    QgsAttributes featureAttributes = featureIt->attributes();
    int nAttrs = featureAttributes.size();
    for ( int i = 0; i < nAttrs; ++i )
    {
      const QVariant& value = featureAttributes.at( i );
      if ( value.isValid() && !value.isNull() )
      {
        QDomElement fieldElem = transactionDoc.createElementNS( mApplicationNamespace, mShared->mFields.at( i ).name() );
        QDomText fieldText = transactionDoc.createTextNode( value.toString() );
        fieldElem.appendChild( fieldText );
        featureElem.appendChild( fieldElem );
      }
    }

    //add geometry column (as gml)
    const QgsGeometry* geometry = featureIt->constGeometry();
    if ( geometry != nullptr )
    {
      QDomElement geomElem = transactionDoc.createElementNS( mApplicationNamespace, mShared->mGeometryAttribute );
      QgsGeometry the_geom( *geometry );
      // convert to multi if the layer geom type is multi and the geom is not
      if ( QGis::isMultiType( this->geometryType( ) ) && ! the_geom.isMultipart( ) )
      {
        the_geom.convertToMultiType();
      }
      QDomElement gmlElem = QgsOgcUtils::geometryToGML( &the_geom, transactionDoc );
      if ( !gmlElem.isNull() )
      {
        gmlElem.setAttribute( "srsName", crs().authid() );
        geomElem.appendChild( gmlElem );
        featureElem.appendChild( geomElem );
      }
    }

    insertElem.appendChild( featureElem );
  }

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    //transaction successful. Add the features to the cache
    QStringList idList = insertedFeatureIds( serverResponse );
    QStringList::const_iterator idIt = idList.constBegin();
    featureIt = flist.begin();

    QVector<QgsWFSFeatureGmlIdPair> serializedFeatureList;
    for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      serializedFeatureList.push_back( QgsWFSFeatureGmlIdPair( *featureIt, *idIt ) );
    }
    mShared->serializeFeatures( serializedFeatureList );

    // And now set the feature id from the one got from the database
    QMap< QString, QgsFeatureId > map;
    for ( int idx = 0; idx < serializedFeatureList.size(); idx++ )
      map[ serializedFeatureList[idx].second ] = serializedFeatureList[idx].first.id();

    idIt = idList.constBegin();
    featureIt = flist.begin();
    for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
    {
      if ( map.find( *idIt ) != map.end() )
        featureIt->setFeatureId( map[*idIt] );
    }

    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::deleteFeatures( const QgsFeatureIds &id )
{
  if ( id.size() < 1 )
  {
    return true;
  }

  //find out typename from uri and strip namespace prefix
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );
  //delete element
  QDomElement deleteElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Delete" );
  deleteElem.setAttribute( "typeName", tname );
  QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "Filter" );


  QgsFeatureIds::const_iterator idIt = id.constBegin();
  for ( ; idIt != id.constEnd(); ++idIt )
  {
    //find out feature id
    QString gmlid = mShared->findGmlId( *idIt );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot identify feature of id %1" ).arg( *idIt ) );
      continue;
    }
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    featureIdElem.setAttribute( "fid", gmlid );
    filterElem.appendChild( featureIdElem );
  }

  deleteElem.appendChild( filterElem );
  transactionElem.appendChild( deleteElem );

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    mShared->deleteFeatures( id );
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::changeGeometryValues( const QgsGeometryMap &geometry_map )
{
  //find out typename from uri and strip namespace prefix
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  QgsGeometryMap::const_iterator geomIt = geometry_map.constBegin();
  for ( ; geomIt != geometry_map.constEnd(); ++geomIt )
  {
    QString gmlid = mShared->findGmlId( geomIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot identify feature of id %1" ).arg( geomIt.key() ) );
      continue;
    }
    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Update" );
    updateElem.setAttribute( "typeName", tname );
    //Property
    QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Property" );
    QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Name" );
    QDomText nameText = transactionDoc.createTextNode( mShared->mGeometryAttribute );
    nameElem.appendChild( nameText );
    propertyElem.appendChild( nameElem );
    QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Value" );
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( &geomIt.value(), transactionDoc );
    gmlElem.setAttribute( "srsName", crs().authid() );
    valueElem.appendChild( gmlElem );
    propertyElem.appendChild( valueElem );
    updateElem.appendChild( propertyElem );

    //filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "Filter" );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    featureIdElem.setAttribute( "fid", gmlid );
    filterElem.appendChild( featureIdElem );
    updateElem.appendChild( filterElem );

    transactionElem.appendChild( updateElem );
  }

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    mShared->changeGeometryValues( geometry_map );
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::changeAttributeValues( const QgsChangedAttributesMap &attr_map )
{
  //find out typename from uri and strip namespace prefix
  QString tname = mShared->mURI.typeName();
  if ( tname.isNull() )
  {
    return false;
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  QgsChangedAttributesMap::const_iterator attIt = attr_map.constBegin();
  for ( ; attIt != attr_map.constEnd(); ++attIt )
  {
    QString gmlid = mShared->findGmlId( attIt.key() );
    if ( gmlid.isEmpty() )
    {
      QgsDebugMsg( QString( "Cannot identify feature of id %1" ).arg( attIt.key() ) );
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Update" );
    updateElem.setAttribute( "typeName", tname );

    QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
    for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
    {
      QString fieldName = mShared->mFields.at( attMapIt.key() ).name();
      QDomElement propertyElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Property" );

      QDomElement nameElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Name" );
      QDomText nameText = transactionDoc.createTextNode( fieldName );
      nameElem.appendChild( nameText );
      propertyElem.appendChild( nameElem );

      QDomElement valueElem = transactionDoc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Value" );
      QDomText valueText = transactionDoc.createTextNode( attMapIt.value().toString() );
      valueElem.appendChild( valueText );
      propertyElem.appendChild( valueElem );

      updateElem.appendChild( propertyElem );
    }

    //Filter
    QDomElement filterElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "Filter" );
    QDomElement featureIdElem = transactionDoc.createElementNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    featureIdElem.setAttribute( "fid", gmlid );
    filterElem.appendChild( featureIdElem );
    updateElem.appendChild( filterElem );

    transactionElem.appendChild( updateElem );
  }

  QDomDocument serverResponse;
  bool success = sendTransactionDocument( transactionDoc, serverResponse );
  if ( !success )
  {
    return false;
  }

  if ( transactionSuccess( serverResponse ) )
  {
    mShared->changeAttributeValues( attr_map );
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::describeFeatureType( QString& geometryAttribute, QgsFields& fields, QGis::WkbType& geomType )
{
  fields.clear();

  QgsWFSDescribeFeatureType describeFeatureType( mShared->mURI.uri() );
  if ( !describeFeatureType.requestFeatureType( mShared->mWFSVersion,
       mShared->mURI.typeName() ) )
  {
    QgsMessageLog::logMessage( tr( "DescribeFeatureType failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( describeFeatureType.errorMessage() ), tr( "WFS" ) );
    return false;
  }

  const QByteArray& response = describeFeatureType.response();

  QDomDocument describeFeatureDocument;
  QString errorMsg;
  if ( !describeFeatureDocument.setContent( response, true, &errorMsg ) )
  {
    QgsDebugMsg( response );
    QgsMessageLog::logMessage( tr( "DescribeFeatureType failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( errorMsg ), tr( "WFS" ) );
    return false;
  }

  if ( readAttributesFromSchema( describeFeatureDocument,
                                 geometryAttribute, fields, geomType ) != 0 )
  {
    QgsMessageLog::logMessage( tr( "DescribeFeatureType failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( tr( "failed retrieving attributes from schema" ) ), tr( "WFS" ) );
    return false;
  }

  return true;
}

int QgsWFSProvider::readAttributesFromSchema( QDomDocument& schemaDoc, QString& geometryAttribute, QgsFields& fields, QGis::WkbType& geomType )
{
  //get the <schema> root element
  QDomNodeList schemaNodeList = schemaDoc.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "schema" );
  if ( schemaNodeList.length() < 1 )
  {
    return 1;
  }
  QDomElement schemaElement = schemaNodeList.at( 0 ).toElement();
  mApplicationNamespace = schemaElement.attribute( "targetNamespace" );
  QDomElement complexTypeElement; //the <complexType> element corresponding to the feature type

  //find out, on which lines the first <element> or the first <complexType> occur. If <element> occurs first (mapserver), read the type of the relevant <complexType> tag. If <complexType> occurs first (geoserver), search for information about the feature type directly under this first complexType element

  int firstElementTagPos = schemaElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "element" ).at( 0 ).toElement().columnNumber();
  int firstComplexTypeTagPos = schemaElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "complexType" ).at( 0 ).toElement().columnNumber();

  if ( firstComplexTypeTagPos < firstElementTagPos )
  {
    //geoserver
    complexTypeElement = schemaElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "complexType" ).at( 0 ).toElement();
  }
  else
  {
    //UMN mapserver
    QString complexTypeType;
    QDomNodeList typeElementNodeList = schemaElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "element" );
    QDomElement typeElement = typeElementNodeList.at( 0 ).toElement();
    complexTypeType = typeElement.attribute( "type" );

    if ( complexTypeType.isEmpty() )
    {
      return 3;
    }

    //remove the namespace on complexTypeType
    if ( complexTypeType.contains( ':' ) )
    {
      complexTypeType = complexTypeType.section( ':', 1, 1 );
    }

    //find <complexType name=complexTypeType
    QDomNodeList complexTypeNodeList = schemaElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "complexType" );
    for ( int i = 0; i < complexTypeNodeList.size(); ++i )
    {
      if ( complexTypeNodeList.at( i ).toElement().attribute( "name" ) == complexTypeType )
      {
        complexTypeElement = complexTypeNodeList.at( i ).toElement();
        break;
      }
    }
  }

  if ( complexTypeElement.isNull() )
  {
    return 4;
  }

  //we have the relevant <complexType> element. Now find out the geometry and the thematic attributes
  QDomNodeList attributeNodeList = complexTypeElement.elementsByTagNameNS( QgsWFSConstants::XMLSCHEMA_NAMESPACE, "element" );
  if ( attributeNodeList.size() < 1 )
  {
    return 5;
  }

  bool foundGeometryAttribute = false;

  for ( int i = 0; i < attributeNodeList.size(); ++i )
  {
    QDomElement attributeElement = attributeNodeList.at( i ).toElement();
    //attribute name
    QString name = attributeElement.attribute( "name" );
    //attribute type
    QString type = attributeElement.attribute( "type" );

    //is it a geometry attribute?
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty" and no name
    QRegExp gmlPT( "gml:(.*)PropertyType" );
    // the GeometryAssociationType has been seen in #11785
    if ( type.indexOf( gmlPT ) == 0 || type == "gml:GeometryAssociationType" || name.isEmpty() )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = geomTypeFromPropertyType( geometryAttribute, gmlPT.cap( 1 ) );
    }
    else //todo: distinguish between numerical and non-numerical types
    {
      QVariant::Type  attributeType = QVariant::String; //string is default type
      if ( type.contains( "double", Qt::CaseInsensitive ) || type.contains( "float", Qt::CaseInsensitive ) || type.contains( "decimal", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::Double;
      }
      else if ( type.contains( "int", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::Int;
      }
      else if ( type.contains( "long", Qt::CaseInsensitive ) )
      {
        attributeType = QVariant::LongLong;
      }
      fields.append( QgsField( name, attributeType, type ) );
    }
  }
  if ( !foundGeometryAttribute )
  {
    geomType = QGis::WKBNoGeometry;
  }

  return 0;
}

QString QgsWFSProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsWFSProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

int QgsWFSProvider::capabilities() const
{
  return mCapabilities;
}

void QgsWFSProvider::removeNamespacePrefix( QString& tname ) const
{
  if ( tname.contains( ':' ) )
  {
    QStringList splitList = tname.split( ':' );
    if ( splitList.size() > 1 )
    {
      tname = splitList.at( 1 );
    }
  }
}

QString QgsWFSProvider::nameSpacePrefix( const QString& tname ) const
{
  QStringList splitList = tname.split( ':' );
  if ( splitList.size() < 2 )
  {
    return QString();
  }
  return splitList.at( 0 );
}

bool QgsWFSProvider::sendTransactionDocument( const QDomDocument& doc, QDomDocument& serverResponse )
{
  if ( doc.isNull() )
  {
    return false;
  }

  QgsWFSTransactionRequest request( mShared->mURI.uri() );
  return request.send( doc, serverResponse );
}

QDomElement QgsWFSProvider::createTransactionElement( QDomDocument& doc ) const
{
  QDomElement transactionElem = doc.createElementNS( QgsWFSConstants::WFS_NAMESPACE, "Transaction" );
  transactionElem.setAttribute( "version", "1.0.0" );
  transactionElem.setAttribute( "service", "WFS" );
  transactionElem.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );

  QUrl describeFeatureTypeURL( mShared->mURI.baseURL() );
  // For tests (since the URL contains part of random data, we need to replace it with a fixed content)
  if ( mShared->mURI.baseURL().toString().contains( "fake_qgis_http_endpoint" ) )
    describeFeatureTypeURL = QUrl( "http://fake_qgis_http_endpoint" );
  describeFeatureTypeURL.addQueryItem( "REQUEST", "DescribeFeatureType" );
  describeFeatureTypeURL.addQueryItem( "VERSION", "1.0.0" );
  describeFeatureTypeURL.addQueryItem( "TYPENAME", mShared->mURI.typeName() );

  transactionElem.setAttribute( "xsi:schemaLocation", mApplicationNamespace + ' '
                                + describeFeatureTypeURL.toEncoded() );

  QString namespacePrefix = nameSpacePrefix( mShared->mURI.typeName() );
  if ( !namespacePrefix.isEmpty() )
  {
    transactionElem.setAttribute( "xmlns:" + namespacePrefix, mApplicationNamespace );
  }
  transactionElem.setAttribute( "xmlns:gml", QgsWFSConstants::GML_NAMESPACE );

  return transactionElem;
}

bool QgsWFSProvider::transactionSuccess( const QDomDocument& serverResponse ) const
{
  if ( serverResponse.isNull() )
  {
    return false;
  }

  QDomElement documentElem = serverResponse.documentElement();
  if ( documentElem.isNull() )
  {
    return false;
  }

  QDomNodeList transactionResultList = documentElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, "TransactionResult" );
  if ( transactionResultList.size() < 1 )
  {
    return false;
  }

  QDomNodeList statusList = transactionResultList.at( 0 ).toElement().elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, "Status" );
  if ( statusList.size() < 1 )
  {
    return false;
  }

  if ( statusList.at( 0 ).firstChildElement().localName() == "SUCCESS" )
  {
    return true;
  }
  else
  {
    return false;
  }
}

QStringList QgsWFSProvider::insertedFeatureIds( const QDomDocument& serverResponse ) const
{
  QStringList ids;
  if ( serverResponse.isNull() )
  {
    return ids;
  }

  QDomElement rootElem = serverResponse.documentElement();
  if ( rootElem.isNull() )
  {
    return ids;
  }

  QDomNodeList insertResultList = rootElem.elementsByTagNameNS( QgsWFSConstants::WFS_NAMESPACE, "InsertResult" );
  for ( int i = 0; i < insertResultList.size(); ++i )
  {
    QDomNodeList featureIdList = insertResultList.at( i ).toElement().elementsByTagNameNS( QgsWFSConstants::OGC_NAMESPACE, "FeatureId" );
    for ( int j = 0; j < featureIdList.size(); ++j )
    {
      QString fidString = featureIdList.at( j ).toElement().attribute( "fid" );
      if ( !fidString.isEmpty() )
      {
        ids << fidString;
      }
    }
  }
  return ids;
}

bool QgsWFSProvider::getCapabilities()
{
  mCapabilities = 0;

  QgsWFSCapabilities getCapabilities( mShared->mURI.uri() );
  if ( !getCapabilities.requestCapabilities( true ) )
  {
    QgsMessageLog::logMessage( tr( "GetCapabilities failed for url %1: %2" ).
                               arg( dataSourceUri() ).arg( getCapabilities.errorMessage() ), tr( "WFS" ) );
    return false;
  }

  const QgsWFSCapabilities::Capabilities caps = getCapabilities.capabilities();

  mShared->mWFSVersion = caps.version;
  if ( mShared->mURI.maxNumFeatures() > 0 )
    mShared->mMaxFeatures = mShared->mURI.maxNumFeatures();
  else
    mShared->mMaxFeatures = caps.maxFeatures;
  mShared->mMaxFeaturesServer = caps.maxFeatures;
  mShared->mSupportsHits = caps.supportsHits;
  mShared->mSupportsPaging = caps.supportsPaging;

  //find the <FeatureType> for this layer
  QString thisLayerName = mShared->mURI.typeName();
  for ( int i = 0; i < caps.featureTypes.size(); i++ )
  {
    if ( thisLayerName == caps.featureTypes[i].name )
    {
      const QgsRectangle& r = caps.featureTypes[i].bboxLongLat;
      if ( mShared->mSourceCRS.authid().isEmpty() && caps.featureTypes[i].crslist.size() != 0 )
      {
        mShared->mSourceCRS.createFromOgcWmsCrs( caps.featureTypes[i].crslist[0] );
      }
      if ( !r.isNull() )
      {
        QgsCoordinateReferenceSystem src;
        src.createFromOgcWmsCrs( "CRS:84" );
        QgsCoordinateTransform ct( src, mShared->mSourceCRS );

        QgsDebugMsg( "latlon ext:" + r.toString() );
        QgsDebugMsg( "src:" + src.authid() );
        QgsDebugMsg( "dst:" + mShared->mSourceCRS.authid() );

        mExtent = ct.transformBoundingBox( r, QgsCoordinateTransform::ForwardTransform );

        QgsDebugMsg( "layer ext:" + mExtent.toString() );
      }
      if ( caps.featureTypes[i].insertCap )
      {
        mCapabilities |= QgsVectorDataProvider::AddFeatures;
      }
      if ( caps.featureTypes[i].updateCap )
      {
        mCapabilities |= QgsVectorDataProvider::ChangeAttributeValues;
        mCapabilities |= QgsVectorDataProvider::ChangeGeometries;
      }
      if ( caps.featureTypes[i].deleteCap )
      {
        mCapabilities |= QgsVectorDataProvider::DeleteFeatures;
      }

      return true;
    }
  }
  QgsMessageLog::logMessage( tr( "Could not find typename %1 in capabilites for url %2" ).
                             arg( thisLayerName ).arg( dataSourceUri() ), tr( "WFS" ) );
  return false;
}

QGis::WkbType QgsWFSProvider::geomTypeFromPropertyType( const QString& attName, const QString& propType )
{
  Q_UNUSED( attName );

  QgsDebugMsg( QString( "DescribeFeatureType geometry attribute \"%1\" type is \"%2\"" )
               .arg( attName, propType ) );
  if ( propType == "Point" )
    return QGis::WKBPoint;
  if ( propType == "LineString" || propType == "Curve" )
    return QGis::WKBLineString;
  if ( propType == "Polygon" || propType == "Surface" )
    return QGis::WKBPolygon;
  if ( propType == "MultiPoint" )
    return QGis::WKBMultiPoint;
  if ( propType == "MultiLineString" || propType == "MultiCurve" )
    return QGis::WKBMultiLineString;
  if ( propType == "MultiPolygon" || propType == "MultiSurface" )
    return QGis::WKBMultiPolygon;
  return QGis::WKBUnknown;
}

void QgsWFSProvider::handleException( const QDomDocument& serverResponse )
{
  QgsDebugMsg( QString( "server response: %1" ).arg( serverResponse.toString() ) );

  QDomElement exceptionElem = serverResponse.documentElement();
  if ( exceptionElem.isNull() )
  {
    pushError( tr( "empty response" ) );
    return;
  }

  if ( exceptionElem.tagName() == "ServiceExceptionReport" )
  {
    pushError( tr( "WFS service exception:%1" ).arg( exceptionElem.firstChildElement( "ServiceException" ).text() ) );
    return;
  }

  if ( exceptionElem.tagName() == "WFS_TransactionResponse" )
  {
    pushError( tr( "unsuccessful service response: %1" ).arg( exceptionElem.firstChildElement( "TransactionResult" ).firstChildElement( "Message" ).text() ) );
    return;
  }

  if ( exceptionElem.tagName() == "ExceptionReport" )
  {
    QDomElement exception = exceptionElem.firstChildElement( "Exception" );
    pushError( tr( "WFS exception report (code=%1 text=%2)" )
               .arg( exception.attribute( "exceptionCode", tr( "missing" ) ),
                     exception.firstChildElement( "ExceptionText" ).text() )
             );
    return;
  }

  pushError( tr( "unhandled response: %1" ).arg( exceptionElem.tagName() ) );
}

QGISEXTERN QgsWFSProvider* classFactory( const QString *uri )
{
  return new QgsWFSProvider( *uri );
}

QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QGISEXTERN bool isProvider()
{
  // This function should normally be called just once, but better check
  // so as to avoid doing twice the initial cleanup of the temporary cache
  // (which should normally be empty, unless QGIS was killed)
  static bool firstTime = true;
  if ( firstTime )
  {
    QgsWFSUtils::init();
    firstTime = false;
  }

  return true;
}
