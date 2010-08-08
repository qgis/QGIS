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

#define WFS_THRESHOLD 200

#include "qgsapplication.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgswfsdata.h"
#include "qgswfsprovider.h"
#include "qgsspatialindex.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"
#include <QDomDocument>
#include <QMessageBox>
#include <QDomNodeList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QUrl>
#include <QWidget>
#include <QPair>
#include <cfloat>

static const QString TEXT_PROVIDER_KEY = "WFS";
static const QString TEXT_PROVIDER_DESCRIPTION = "WFS data provider";

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";
static const QString GML_NAMESPACE = "http://www.opengis.net/gml";

QgsWFSProvider::QgsWFSProvider( const QString& uri )
    : QgsVectorDataProvider( uri ),
    mNetworkRequestFinished( true ),
    mUseIntersect( false ),
    mSourceCRS( 0 ),
    mFeatureCount( 0 ),
    mValid( true )
{
  mSpatialIndex = new QgsSpatialIndex;
  if ( getFeature( uri ) == 0 )
  {
    mValid = true;
    getLayerCapabilities();

    //set spatial filter to the whole extent
    //select(mExtent, false); //MH TODO: fix this in provider0_9-branch
  }
  else
  {
    mValid = false;
  }
}

QgsWFSProvider::~QgsWFSProvider()
{
  mSelectedFeatures.clear();
  for ( int i = 0; i < mFeatures.size(); i++ )
    delete mFeatures[i];
  mFeatures.clear();
  delete mSpatialIndex;
}

bool QgsWFSProvider::nextFeature( QgsFeature& feature )
{
  feature.setValid( false );

  while ( true ) //go through the loop until we find a feature in the filter
  {
    if ( mSelectedFeatures.size() == 0 || mFeatureIterator == mSelectedFeatures.end() )
    {
      return 0;
    }

    feature.setFeatureId( mFeatures[*mFeatureIterator]->id() );

    //we need geometry anyway, e.g. for intersection tests
    QgsGeometry* geometry = mFeatures[*mFeatureIterator]->geometry();
    unsigned char *geom = geometry->asWkb();
    int geomSize = geometry->wkbSize();
    unsigned char* copiedGeom = new unsigned char[geomSize];
    memcpy( copiedGeom, geom, geomSize );
    feature.setGeometryAndOwnership( copiedGeom, geomSize );

    const QgsAttributeMap& attributes = mFeatures[*mFeatureIterator]->attributeMap();
    for ( QgsAttributeList::const_iterator it = mAttributesToFetch.begin(); it != mAttributesToFetch.end(); ++it )
    {
      feature.addAttribute( *it, attributes[*it] );
    }
    ++mFeatureIterator;
    if ( mUseIntersect )
    {
      if ( feature.geometry() && feature.geometry()->intersects( mSpatialFilter ) )
      {
        feature.setValid( true );
        return true;
      }
      else
      {
        continue; //go for the next feature
      }
    }
    else
    {
      feature.setValid( true );
      return true;
    }
  }
}



QGis::WkbType QgsWFSProvider::geometryType() const
{
  return mWKBType;
}

long QgsWFSProvider::featureCount() const
{
  return mFeatureCount;
}

uint QgsWFSProvider::fieldCount() const
{
  return mFields.size();
}

const QgsFieldMap & QgsWFSProvider::fields() const
{
  return mFields;
}

void QgsWFSProvider::rewind()
{
  mFeatureIterator = mSelectedFeatures.begin();
}

QgsCoordinateReferenceSystem QgsWFSProvider::crs()
{
  return mSourceCRS;
}

QgsRectangle QgsWFSProvider::extent()
{
  return mExtent;
}

bool QgsWFSProvider::isValid()
{
  return mValid;
}

void QgsWFSProvider::select( QgsAttributeList fetchAttributes,
                             QgsRectangle rect,
                             bool fetchGeometry,
                             bool useIntersect )
{
  mUseIntersect = useIntersect;
  mAttributesToFetch = fetchAttributes;
  mFetchGeom = fetchGeometry;

  if ( rect.isEmpty() )
  {
    mSpatialFilter = mExtent;
  }
  else
  {
    mSpatialFilter = rect;
  }

  mSelectedFeatures = mSpatialIndex->intersects( mSpatialFilter );
  mFeatureIterator = mSelectedFeatures.begin();
}

int QgsWFSProvider::getFeature( const QString& uri )
{
  QString geometryAttribute;

  //Local url or HTTP?
  if ( uri.startsWith( "http" ) )
  {
    mEncoding = QgsWFSProvider::GET;
  }
  else
  {
    mEncoding = QgsWFSProvider::FILE;
  }

  if ( mEncoding == QgsWFSProvider::FILE )
  {
    //guess geometry attribute and other attributes from schema or from .gml file
    if ( describeFeatureTypeFile( uri, mGeometryAttribute, mFields ) != 0 )
    {
      return 1;
    }
  }
  else //take schema with describeFeatureType request
  {
    QString describeFeatureUri = uri;
    describeFeatureUri.replace( QString( "GetFeature" ), QString( "DescribeFeatureType" ) );
    if ( describeFeatureType( describeFeatureUri, mGeometryAttribute, mFields ) != 0 )
    {
      return 1;
    }
  }

  if ( mEncoding == QgsWFSProvider::GET )
  {
    return getFeatureGET( uri, mGeometryAttribute );
  }
  else//local file
  {
    return getFeatureFILE( uri, mGeometryAttribute ); //read the features from disk
  }
}

bool QgsWFSProvider::addFeatures( QgsFeatureList &flist )
{
  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  //find out typename from uri and strip namespace prefix
  QString tname = typeNameFromUrl();
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
    QDomElement insertElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Insert" );
    transactionElem.appendChild( insertElem );

    QDomElement featureElem = transactionDoc.createElementNS( mWfsNamespace, tname );

    //add thematic attributes
    QgsAttributeMap featureAttributes = featureIt->attributeMap();
    QgsFieldMap::const_iterator fieldIt = mFields.constBegin();
    for ( ; fieldIt != mFields.constEnd(); ++fieldIt )
    {
      QgsAttributeMap::const_iterator valueIt = featureAttributes.find( fieldIt.key() );
      if ( valueIt != featureAttributes.constEnd() )
      {
        QVariant fieldValue = valueIt.value();
        if ( fieldValue.isValid() && !fieldValue.isNull() )
        {
          QDomElement fieldElem = transactionDoc.createElementNS( mWfsNamespace, fieldIt.value().name() );
          QDomText fieldText = transactionDoc.createTextNode( fieldValue.toString() );
          fieldElem.appendChild( fieldText );
          featureElem.appendChild( fieldElem );
        }
      }
    }

    //add geometry column (as gml)
    QDomElement geomElem = transactionDoc.createElementNS( mWfsNamespace, mGeometryAttribute );
    QDomElement gmlElem = createGeometryElem( featureIt->geometry(), transactionDoc );
    if ( !gmlElem.isNull() )
    {
      geomElem.appendChild( gmlElem );
      featureElem.appendChild( geomElem );
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
    //transaction successful. Add the features to mSpatialIndex
    if ( mSpatialIndex )
    {
      QStringList idList = insertedFeatureIds( serverResponse );
      QStringList::const_iterator idIt = idList.constBegin();
      featureIt = flist.begin();

      for ( ; idIt != idList.constEnd() && featureIt != flist.end(); ++idIt, ++featureIt )
      {
        int newId = findNewKey();
        featureIt->setFeatureId( newId );
        mFeatures.insert( newId, new QgsFeature( *featureIt ) );
        mIdMap.insert( newId, *idIt );
        mSpatialIndex->insertFeature( *featureIt );
        mFeatureCount = mFeatures.size();
      }
      return true;
    }
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
  QString tname = typeNameFromUrl();
  if ( tname.isNull() )
  {
    return false;
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );
  //delete element
  QDomElement deleteElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Delete" );
  deleteElem.setAttribute( "typeName", tname );
  QDomElement filterElem = transactionDoc.createElementNS( "http://www.opengis.net/ogc", "Filter" );


  QgsFeatureIds::const_iterator idIt = id.constBegin();
  for ( ; idIt != id.constEnd(); ++idIt )
  {
    //find out feature id
    QMap< int, QString >::const_iterator fidIt = mIdMap.find( *idIt );
    if ( fidIt == mIdMap.constEnd() )
    {
      continue;
    }
    QDomElement featureIdElem = transactionDoc.createElementNS( "http://www.opengis.net/ogc", "FeatureId" );
    featureIdElem.setAttribute( "fid", fidIt.value() );
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
    idIt = id.constBegin();
    for ( ; idIt != id.constEnd(); ++idIt )
    {
      QMap<int, QgsFeature* >::iterator fIt = mFeatures.find( *idIt );
      if ( fIt != mFeatures.end() )
      {
        if ( mSpatialIndex )
        {
          mSpatialIndex->deleteFeature( *fIt.value() );
        }
        delete fIt.value();
        mFeatures.remove( *idIt );
      }
    }
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

bool QgsWFSProvider::changeGeometryValues( QgsGeometryMap & geometry_map )
{
  //find out typename from uri and strip namespace prefix
  QString tname = typeNameFromUrl();
  if ( tname.isNull() )
  {
    return false;
  }

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  QgsGeometryMap::iterator geomIt = geometry_map.begin();
  for ( ; geomIt != geometry_map.end(); ++geomIt )
  {
    //find out feature id
    QMap< int, QString >::const_iterator fidIt = mIdMap.find( geomIt.key() );
    if ( fidIt == mIdMap.constEnd() )
    {
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Update" );
    updateElem.setAttribute( "typeName", tname );
    //Property
    QDomElement propertyElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Property" );
    QDomElement nameElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Name" );
    QDomText nameText = transactionDoc.createTextNode( mGeometryAttribute );
    nameElem.appendChild( nameText );
    propertyElem.appendChild( nameElem );
    QDomElement valueElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Value" );
    QDomElement gmlElem = createGeometryElem( &geomIt.value(), transactionDoc );
    valueElem.appendChild( gmlElem );
    propertyElem.appendChild( valueElem );
    updateElem.appendChild( propertyElem );

    //filter
    QDomElement filterElem = transactionDoc.createElementNS( "http://www.opengis.net/ogc", "Filter" );
    QDomElement featureIdElem = transactionDoc.createElementNS( "http://www.opengis.net/ogc", "FeatureId" );
    featureIdElem.setAttribute( "fid", fidIt.value() );
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
    geomIt = geometry_map.begin();
    for ( ; geomIt != geometry_map.end(); ++geomIt )
    {
      QMap<int, QgsFeature* >::iterator fIt = mFeatures.find( geomIt.key() );
      if ( fIt == mFeatures.end() )
      {
        continue;
      }
      QgsFeature* currentFeature = fIt.value();
      if ( !currentFeature )
      {
        continue;
      }

      if ( mSpatialIndex )
      {
        mSpatialIndex->deleteFeature( *currentFeature );
        fIt.value()->setGeometry( geomIt.value() );
        mSpatialIndex->insertFeature( *currentFeature );
      }
    }
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
  QString tname = typeNameFromUrl();
  if ( tname.isNull() )
  {
    return false;
  }

  const QgsFieldMap& fieldMap = fields();

  //create <Transaction> xml
  QDomDocument transactionDoc;
  QDomElement transactionElem = createTransactionElement( transactionDoc );
  transactionDoc.appendChild( transactionElem );

  QgsChangedAttributesMap::const_iterator attIt = attr_map.constBegin();
  for ( ; attIt != attr_map.constEnd(); ++attIt )
  {
    //find out wfs server feature id
    QMap< int, QString >::const_iterator fidIt = mIdMap.find( attIt.key() );
    if ( fidIt == mIdMap.constEnd() )
    {
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Update" );
    updateElem.setAttribute( "typeName", tname );

    QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
    for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
    {
      QString fieldName;
      QgsFieldMap::const_iterator fieldIt = fieldMap.find( attMapIt.key() );
      if ( fieldIt == fieldMap.constEnd() )
      {
        continue;
      }
      fieldName = fieldIt.value().name();

      QDomElement propertyElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Property" );

      QDomElement nameElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Name" );
      QDomText nameText = transactionDoc.createTextNode( fieldName );
      nameElem.appendChild( nameText );
      propertyElem.appendChild( nameElem );

      QDomElement valueElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Value" );
      QDomText valueText = transactionDoc.createTextNode( attMapIt.value().toString() );
      valueElem.appendChild( valueText );
      propertyElem.appendChild( valueElem );

      updateElem.appendChild( propertyElem );
    }

    //Filter
    QDomElement filterElem = transactionDoc.createElementNS( "http://www.opengis.net/ogc", "Filter" );
    QDomElement featureIdElem = transactionDoc.createElementNS( "http://www.opengis.net/ogc", "FeatureId" );
    featureIdElem.setAttribute( "fid", fidIt.value() );
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
    //change attributes in mFeatures
    attIt = attr_map.constBegin();
    for ( ; attIt != attr_map.constEnd(); ++attIt )
    {
      QMap<int, QgsFeature*>::iterator fIt = mFeatures.find( attIt.key() );
      if ( fIt == mFeatures.end() )
      {
        continue;
      }

      QgsFeature* currentFeature = fIt.value();
      if ( !currentFeature )
      {
        continue;
      }

      QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
      for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
      {
        currentFeature->changeAttribute( attMapIt.key(), attMapIt.value() );
      }
    }
    return true;
  }
  else
  {
    handleException( serverResponse );
    return false;
  }
}

int QgsWFSProvider::describeFeatureType( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields )
{
  switch ( mEncoding )
  {
    case QgsWFSProvider::GET:
      return describeFeatureTypeGET( uri, geometryAttribute, fields );
    case QgsWFSProvider::FILE:
      return describeFeatureTypeFile( uri, geometryAttribute, fields );
  }
  return 1;
}

int QgsWFSProvider::getFeatureGET( const QString& uri, const QString& geometryAttribute )
{
  //the new and faster method with the expat SAX parser

  //allows fast searchings with attribute name. Also needed is attribute Index and type infos
  QMap<QString, QPair<int, QgsField> > thematicAttributes;
  for ( QgsFieldMap::const_iterator it = mFields.begin(); it != mFields.end(); ++it )
  {
    thematicAttributes.insert( it.value().name(), qMakePair( it.key(), it.value() ) );
  }

  QgsWFSData dataReader( uri, &mExtent, &mSourceCRS, mFeatures, mIdMap, geometryAttribute, thematicAttributes, &mWKBType );
  QObject::connect( &dataReader, SIGNAL( dataProgressAndSteps( int , int ) ), this, SLOT( handleWFSProgressMessage( int, int ) ) );

  //also connect to statusChanged signal of qgisapp (if it exists)
  QWidget* mainWindow = 0;

  QWidgetList topLevelWidgets = qApp->topLevelWidgets();
  for ( QWidgetList::iterator it = topLevelWidgets.begin(); it != topLevelWidgets.end(); ++it )
  {
    if (( *it )->objectName() == "QgisApp" )
    {
      mainWindow = *it;
      break;
    }
  }

  if ( mainWindow )
  {
    QObject::connect( this, SIGNAL( dataReadProgressMessage( QString ) ), mainWindow, SLOT( showStatusMessage( QString ) ) );
  }

  if ( dataReader.getWFSData() != 0 )
  {
    QgsDebugMsg( "getWFSData returned with error" );
    return 1;
  }

  QgsDebugMsg( QString( "feature count after request is: %1" ).arg( mFeatures.size() ) );
  QgsDebugMsg( QString( "mExtent after request is: %1" ).arg( mExtent.toString() ) );

  for ( QMap<int, QgsFeature*>::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
  {
    QgsDebugMsg( "feature " + QString::number(( *it )->id() ) );
    mSpatialIndex->insertFeature( *( it.value() ) );
  }

  mFeatureCount = mFeatures.size();

  return 0;
}

int QgsWFSProvider::getFeatureFILE( const QString& uri, const QString& geometryAttribute )
{
  QFile gmlFile( uri );
  if ( !gmlFile.open( QIODevice::ReadOnly ) )
  {
    mValid = false;
    return 1;
  }

  QDomDocument gmlDoc;
  QString errorMsg;
  int errorLine, errorColumn;
  if ( !gmlDoc.setContent( &gmlFile, true, &errorMsg, &errorLine, &errorColumn ) )
  {
    mValid = false;
    return 2;
  }

  QDomElement featureCollectionElement = gmlDoc.documentElement();
  //get and set Extent
  if ( getExtentFromGML2( &mExtent, featureCollectionElement ) != 0 )
  {
    return 3;
  }

  setCRSFromGML2( featureCollectionElement );

  if ( getFeaturesFromGML2( featureCollectionElement, geometryAttribute ) != 0 )
  {
    return 4;
  }

  return 0;
}

int QgsWFSProvider::describeFeatureTypeGET( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields )
{
  if ( !mNetworkRequestFinished )
  {
    return 1;
  }

  mNetworkRequestFinished = false;

  QNetworkRequest request( uri );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );
  connect( reply, SIGNAL( finished() ), this, SLOT( networkRequestFinished() ) );
  while ( !mNetworkRequestFinished )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WFS_THRESHOLD );
  }

  QByteArray response = reply->readAll();
  reply->deleteLater();

  QDomDocument describeFeatureDocument;

  if ( !describeFeatureDocument.setContent( response, true ) )
  {
    return 2;
  }

  if ( readAttributesFromSchema( describeFeatureDocument, geometryAttribute, fields ) != 0 )
  {
    return 3;
  }

  return 0;
}

void QgsWFSProvider::networkRequestFinished()
{
  mNetworkRequestFinished = true;
}

int QgsWFSProvider::describeFeatureTypeFile( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields )
{
  //first look in the schema file
  QString noExtension = uri;
  noExtension.chop( 3 );
  QString schemaUri = noExtension.append( "xsd" );
  QFile schemaFile( schemaUri );

  if ( schemaFile.open( QIODevice::ReadOnly ) )
  {
    QDomDocument schemaDoc;
    if ( !schemaDoc.setContent( &schemaFile, true ) )
    {
      return 1; //xml file not readable or not valid
    }

    if ( readAttributesFromSchema( schemaDoc, geometryAttribute, fields ) != 0 )
    {
      return 2;
    }
    return 0;
  }

  std::list<QString> thematicAttributes;

  //if this fails (e.g. no schema file), try to guess the geometry attribute and the names of the thematic attributes from the .gml file
  if ( guessAttributesFromFile( uri, geometryAttribute, thematicAttributes ) != 0 )
  {
    return 1;
  }

  fields.clear();
  int i = 0;
  for ( std::list<QString>::const_iterator it = thematicAttributes.begin(); it != thematicAttributes.end(); ++it, ++i )
  {
    // TODO: is this correct?
    fields[i] = QgsField( *it, QVariant::String, "unknown" );
  }
  return 0;
}

int QgsWFSProvider::readAttributesFromSchema( QDomDocument& schemaDoc, QString& geometryAttribute, QgsFieldMap& fields )
{
  //get the <schema> root element
  QDomNodeList schemaNodeList = schemaDoc.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "schema" );
  if ( schemaNodeList.length() < 1 )
  {
    return 1;
  }
  QDomElement schemaElement = schemaNodeList.at( 0 ).toElement();
  mWfsNamespace = schemaElement.attribute( "targetNamespace" );
  QDomElement complexTypeElement; //the <complexType> element corresponding to the feature type

  //find out, on which lines the first <element> or the first <complexType> occur. If <element> occurs first (mapserver), read the type of the relevant <complexType> tag. If <complexType> occurs first (geoserver), search for information about the feature type directly under this first complexType element

  int firstElementTagPos = schemaElement.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "element" ).at( 0 ).toElement().columnNumber();
  int firstComplexTypeTagPos = schemaElement.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "complexType" ).at( 0 ).toElement().columnNumber();

  if ( firstComplexTypeTagPos < firstElementTagPos )
  {
    //geoserver
    complexTypeElement = schemaElement.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "complexType" ).at( 0 ).toElement();
  }
  else
  {
    //UMN mapserver
    QString complexTypeType;
    QDomNodeList typeElementNodeList = schemaElement.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "element" );
    QDomElement typeElement = typeElementNodeList.at( 0 ).toElement();
    complexTypeType = typeElement.attribute( "type" );

    if ( complexTypeType.isEmpty() )
    {
      return 3;
    }

    //remove the namespace on complexTypeType
    if ( complexTypeType.contains( ":" ) )
    {
      complexTypeType = complexTypeType.section( ":", 1, 1 );
    }

    //find <complexType name=complexTypeType
    QDomNodeList complexTypeNodeList = schemaElement.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "complexType" );
    for ( uint i = 0; i < complexTypeNodeList.length(); ++i )
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
  QDomNodeList attributeNodeList = complexTypeElement.elementsByTagNameNS( "http://www.w3.org/2001/XMLSchema", "element" );
  if ( attributeNodeList.size() < 1 )
  {
    return 5;
  }

  for ( uint i = 0; i < attributeNodeList.length(); ++i )
  {
    QDomElement attributeElement = attributeNodeList.at( i ).toElement();
    //attribute name
    QString name = attributeElement.attribute( "name" );
    //attribute type
    QString type = attributeElement.attribute( "type" );

    //is it a geometry attribute?
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty" and no name
    if (( type.startsWith( "gml:" ) && type.endsWith( "PropertyType" ) ) || name.isEmpty() )
    {
      geometryAttribute = name;
    }
    else //todo: distinguish between numerical and non-numerical types
    {
      QVariant::Type  attributeType = QVariant::String; //string is default type
      if ( type.contains( "double", Qt::CaseInsensitive ) || type.contains( "float", Qt::CaseInsensitive ) )
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
      fields[fields.size()] = QgsField( name, attributeType, type );
    }
  }
  return 0;
}

int QgsWFSProvider::guessAttributesFromFile( const QString& uri, QString& geometryAttribute, std::list<QString>& thematicAttributes ) const
{
  QFile gmlFile( uri );
  if ( !gmlFile.open( QIODevice::ReadOnly ) )
  {
    return 1;
  }

  QDomDocument gmlDoc;
  if ( !gmlDoc.setContent( &gmlFile, true ) )
  {
    return 2; //xml file not readable or not valid
  }


  //find gmlCollection element
  QDomElement featureCollectionElement = gmlDoc.documentElement();

  //get the first feature to guess attributes
  QDomNodeList featureList = featureCollectionElement.elementsByTagNameNS( GML_NAMESPACE, "featureMember" );
  if ( featureList.size() < 1 )
  {
    return 3; //we need at least one attribute
  }

  QDomElement featureElement = featureList.at( 0 ).toElement();
  QDomNode attributeNode = featureElement.firstChild().firstChild();
  if ( attributeNode.isNull() )
  {
    return 4;
  }
  QString attributeText;
  QDomElement attributeChildElement;
  QString attributeChildLocalName;

  while ( !attributeNode.isNull() )//loop over attributes
  {
    QString attributeNodeName = attributeNode.toElement().localName();
    attributeChildElement = attributeNode.firstChild().toElement();
    if ( attributeChildElement.isNull() )//no child element means it is a thematic attribute for sure
    {
      thematicAttributes.push_back( attributeNode.toElement().localName() );
      attributeNode = attributeNode.nextSibling();
      continue;
    }

    attributeChildLocalName = attributeChildElement.localName();
    if ( attributeChildLocalName == "Point" || attributeChildLocalName == "LineString" ||
         attributeChildLocalName == "Polygon" || attributeChildLocalName == "MultiPoint" ||
         attributeChildLocalName == "MultiLineString" || attributeChildLocalName == "MultiPolygon" ||
         attributeChildLocalName == "Surface" || attributeChildLocalName == "MultiSurface" )
    {
      geometryAttribute = attributeNode.toElement().localName(); //a geometry attribute
    }
    else
    {
      thematicAttributes.push_back( attributeNode.toElement().localName() ); //a thematic attribute
    }
    attributeNode = attributeNode.nextSibling();
  }

  return 0;
}

int QgsWFSProvider::getExtentFromGML2( QgsRectangle* extent, const QDomElement& wfsCollectionElement ) const
{
  QDomNodeList boundedByList = wfsCollectionElement.elementsByTagNameNS( GML_NAMESPACE, "boundedBy" );
  if ( boundedByList.length() < 1 )
  {
    return 1;
  }
  QDomElement boundedByElement = boundedByList.at( 0 ).toElement();
  QDomNode childNode = boundedByElement.firstChild();
  if ( childNode.isNull() )
  {
    return 2;
  }

  //support <gml:Box>, <gml:coordinates> and <gml:Envelope>,<gml::lowerCorner>,<gml::upperCorner>. What
  //about <gml:Envelope>, <gml:pos>?
  QString bboxName = childNode.localName();
  if ( bboxName != "Box" )
  {
    return 3;
  }

  QDomNode coordinatesNode = childNode.firstChild();
  if ( coordinatesNode.localName() == "coordinates" )
  {
    std::list<QgsPoint> boundingPoints;
    if ( readGML2Coordinates( boundingPoints, coordinatesNode.toElement() ) != 0 )
    {
      return 5;
    }

    if ( boundingPoints.size() != 2 )
    {
      return 6;
    }

    std::list<QgsPoint>::const_iterator it = boundingPoints.begin();
    extent->setXMinimum( it->x() );
    extent->setYMinimum( it->y() );
    ++it;
    extent->setXMaximum( it->x() );
    extent->setYMaximum( it->y() );
    return 0;
  }
  else if ( coordinatesNode.localName() == "coord" )
  {
    //first <coord> element
    QDomElement xElement, yElement;
    bool conversion1, conversion2; //string->double conversion success
    xElement = coordinatesNode.firstChild().toElement();
    yElement = xElement.nextSibling().toElement();
    if ( xElement.isNull() || yElement.isNull() )
    {
      return 7;
    }
    double x1 = xElement.text().toDouble( &conversion1 );
    double y1 = yElement.text().toDouble( &conversion2 );
    if ( !conversion1 || !conversion2 )
    {
      return 8;
    }

    //second <coord> element
    coordinatesNode = coordinatesNode.nextSibling();
    xElement = coordinatesNode.firstChild().toElement();
    yElement = xElement.nextSibling().toElement();
    if ( xElement.isNull() || yElement.isNull() )
    {
      return 9;
    }
    double x2 = xElement.text().toDouble( &conversion1 );
    double y2 = yElement.text().toDouble( &conversion2 );
    if ( !conversion1 || !conversion2 )
    {
      return 10;
    }
    extent->setXMinimum( x1 );
    extent->setYMinimum( y1 );
    extent->setXMaximum( x2 );
    extent->setYMaximum( y2 );
    return 0;
  }
  else
  {
    return 11; //no valid tag for the bounding box
  }
}

int QgsWFSProvider::setCRSFromGML2( const QDomElement& wfsCollectionElement )
{
  QgsDebugMsg( "entering." );
  //search <gml:boundedBy>
  QDomNodeList boundedByList = wfsCollectionElement.elementsByTagNameNS( GML_NAMESPACE, "boundedBy" );
  if ( boundedByList.size() < 1 )
  {
    QgsDebugMsg( "Error, could not find boundedBy element" );
    return 1;
  }
  //search <gml:Envelope>
  QDomElement boundedByElem = boundedByList.at( 0 ).toElement();
  QDomNodeList boxList = boundedByElem.elementsByTagNameNS( GML_NAMESPACE, "Box" );
  if ( boxList.size() < 1 )
  {
    QgsDebugMsg( "Error, could not find Envelope element" );
    return 2;
  }
  QDomElement boxElem = boxList.at( 0 ).toElement();
  //getAttribute 'srsName'
  QString srsName = boxElem.attribute( "srsName" );
  if ( srsName.isEmpty() )
  {
    QgsDebugMsg( "Error, srsName is empty" );
    return 3;
  }
  QgsDebugMsg( "srsName is: " + srsName );


  //extract the EpsgCrsId id
  bool conversionSuccess;
  if ( srsName.contains( "#" ) )//geoserver has "http://www.opengis.net/gml/srs/epsg.xml#4326"
  {
    int epsgId = srsName.section( "#", 1, 1 ).toInt( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 4;
    }
    srsName = QString( "EPSG:%1" ).arg( epsgId );
  }
  else if ( !srsName.contains( ":" ) ) //mapserver has "EPSG:4326"
    srsName = GEO_EPSG_CRS_AUTHID;

  if ( !mSourceCRS.createFromOgcWmsCrs( srsName ) )
  {
    QgsDebugMsg( "Error, creation of QgsCoordinateReferenceSystem failed" );
    return 6;
  }
  return 0;
}

int QgsWFSProvider::getFeaturesFromGML2( const QDomElement& wfsCollectionElement, const QString& geometryAttribute )
{
  QDomNodeList featureTypeNodeList = wfsCollectionElement.elementsByTagNameNS( GML_NAMESPACE, "featureMember" );
  QDomElement currentFeatureMemberElem;
  QDomElement layerNameElem;
  QDomNode currentAttributeChild;
  QDomElement currentAttributeElement;
  int counter = 0;
  QgsFeature* f = 0;
  unsigned char* wkb = 0;
  int wkbSize = 0;
  QGis::WkbType currentType;
  mFeatureCount = 0;

  for ( int i = 0; i < featureTypeNodeList.size(); ++i )
  {
    f = new QgsFeature( counter );
    currentFeatureMemberElem = featureTypeNodeList.at( i ).toElement();
    //the first child element is always <namespace:layer>
    layerNameElem = currentFeatureMemberElem.firstChild().toElement();
    //the children are the attributes
    currentAttributeChild = layerNameElem.firstChild();
    int attr = 0;
    bool numeric = false;

    while ( !currentAttributeChild.isNull() )
    {
      currentAttributeElement = currentAttributeChild.toElement();

      if ( currentAttributeElement.localName() != "boundedBy" )
      {
        currentAttributeElement.text().toDouble( &numeric );
        if (( currentAttributeElement.localName() ) != geometryAttribute ) //a normal attribute
        {
          if ( numeric )
          {
            f->addAttribute( attr++, QVariant( currentAttributeElement.text().toDouble() ) );
          }
          else
          {
            f->addAttribute( attr++, QVariant( currentAttributeElement.text() ) );
          }
        }
        else //a geometry attribute
        {
          getWkbFromGML2( currentAttributeElement, &wkb, &wkbSize, &currentType );
          mWKBType = currentType; //a more sophisticated method is necessary
          f->setGeometryAndOwnership( wkb, wkbSize );
        }
      }
      currentAttributeChild = currentAttributeChild.nextSibling();
    }
    if ( wkb && wkbSize > 0 )
    {
      //insert bbox and pointer to feature into search tree
      mSpatialIndex->insertFeature( *f );
      mFeatures.insert( f->id(), f );
      ++mFeatureCount;
    }
    ++counter;
  }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2( const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  QDomNode geometryChild = geometryElement.firstChild();
  if ( geometryChild.isNull() )
  {
    return 1;
  }
  QDomElement geometryTypeElement = geometryChild.toElement();
  QString geomType = geometryTypeElement.localName();
  if ( geomType == "Point" )
  {
    return getWkbFromGML2Point( geometryTypeElement, wkb, wkbSize, type );
  }
  else if ( geomType == "LineString" )
  {
    return getWkbFromGML2LineString( geometryTypeElement, wkb, wkbSize, type );
  }
  else if ( geomType == "Polygon" )
  {
    return getWkbFromGML2Polygon( geometryTypeElement, wkb, wkbSize, type );
  }
  else if ( geomType == "MultiPoint" )
  {
    return getWkbFromGML2MultiPoint( geometryTypeElement, wkb, wkbSize, type );
  }
  else if ( geomType == "MultiLineString" )
  {
    return getWkbFromGML2MultiLineString( geometryTypeElement, wkb, wkbSize, type );
  }
  else if ( geomType == "MultiPolygon" )
  {
    return getWkbFromGML2MultiPolygon( geometryTypeElement, wkb, wkbSize, type );
  }
  else //unknown type
  {
    *wkb = 0;
    *wkbSize = 0;
  }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2Point( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  QDomNodeList coordList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( coordList.size() < 1 )
  {
    return 1;
  }
  QDomElement coordElement = coordList.at( 0 ).toElement();
  std::list<QgsPoint> pointCoordinate;
  if ( readGML2Coordinates( pointCoordinate, coordElement ) != 0 )
  {
    return 2;
  }

  if ( pointCoordinate.size() < 1 )
  {
    return 3;
  }

  std::list<QgsPoint>::const_iterator point_it = pointCoordinate.begin();
  char e = QgsApplication::endian();
  double x = point_it->x();
  double y = point_it->y();
  int size = 1 + sizeof( int ) + 2 * sizeof( double );
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBPoint;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  memcpy( &( *wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
  wkbPosition += sizeof( double );
  memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
  return 0;
}

int QgsWFSProvider::getWkbFromGML2Polygon( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  //read all the coordinates (as QgsPoint) into memory. Each linear ring has an entry in the vector
  std::vector<std::list<QgsPoint> > ringCoordinates;

  //read coordinates for outer boundary
  QDomNodeList outerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "outerBoundaryIs" );
  if ( outerBoundaryList.size() < 1 ) //outer ring is necessary
  {
    return 1;
  }
  QDomElement coordinatesElement = outerBoundaryList.at( 0 ).firstChild().firstChild().toElement();
  if ( coordinatesElement.isNull() )
  {
    return 2;
  }
  std::list<QgsPoint> exteriorPointList;
  if ( readGML2Coordinates( exteriorPointList, coordinatesElement ) != 0 )
  {
    return 3;
  }
  ringCoordinates.push_back( exteriorPointList );

  //read coordinates for inner boundary
  QDomNodeList innerBoundaryList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "innerBoundaryIs" );
  for ( int i = 0; i < innerBoundaryList.size(); ++i )
  {
    std::list<QgsPoint> interiorPointList;
    QDomElement coordinatesElement = innerBoundaryList.at( i ).firstChild().firstChild().toElement();
    if ( coordinatesElement.isNull() )
    {
      return 4;
    }
    if ( readGML2Coordinates( interiorPointList, coordinatesElement ) != 0 )
    {
      return 5;
    }
    ringCoordinates.push_back( interiorPointList );
  }

  //calculate number of bytes to allocate
  int nrings = ringCoordinates.size();
  int npoints = 0;//total number of points
  for ( std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it )
  {
    npoints += it->size();
  }
  int size = 1 + 2 * sizeof( int ) + nrings * sizeof( int ) + 2 * npoints * sizeof( double );
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBPolygon;
  char e = QgsApplication::endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPointsInRing = 0;
  double x, y;

  //fill the contents into *wkb
  memcpy( &( *wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &nrings, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::vector<std::list<QgsPoint> >::const_iterator it = ringCoordinates.begin(); it != ringCoordinates.end(); ++it )
  {
    nPointsInRing = it->size();
    memcpy( &( *wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
    wkbPosition += sizeof( int );
    //iterate through the string list converting the strings to x-/y- doubles
    std::list<QgsPoint>::const_iterator iter;
    for ( iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      y = iter->y();
      //qWarning("currentCoordinate: " + QString::number(x) + " // " + QString::number(y));
      memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
      wkbPosition += sizeof( double );
      memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
      wkbPosition += sizeof( double );
    }
  }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2LineString( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  QDomNodeList coordinatesList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
  if ( coordinatesList.size() < 1 )
  {
    return 1;
  }
  QDomElement coordinatesElement = coordinatesList.at( 0 ).toElement();
  std::list<QgsPoint> lineCoordinates;
  if ( readGML2Coordinates( lineCoordinates, coordinatesElement ) != 0 )
  {
    return 2;
  }

  char e = QgsApplication::endian();
  int size = 1 + 2 * sizeof( int ) + lineCoordinates.size() * 2 * sizeof( double );
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBLineString;
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPoints = lineCoordinates.size();

  //fill the contents into *wkb
  memcpy( &( *wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );

  std::list<QgsPoint>::const_iterator iter;
  for ( iter = lineCoordinates.begin(); iter != lineCoordinates.end(); ++iter )
  {
    x = iter->x();
    y = iter->y();
    memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2MultiPoint( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  std::list<QgsPoint> pointList;
  std::list<QgsPoint> currentPoint;
  QDomNodeList pointMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "pointMember" );
  if ( pointMemberList.size() < 1 )
  {
    return 1;
  }
  QDomNodeList pointNodeList;
  QDomNodeList coordinatesList;
  for ( int i = 0; i < pointMemberList.size(); ++i )
  {
    //<Point> element
    pointNodeList = pointMemberList.at( i ).toElement().elementsByTagNameNS( GML_NAMESPACE, "Point" );
    if ( pointNodeList.size() < 1 )
    {
      continue;
    }
    //<coordinates> element
    coordinatesList = pointNodeList.at( 0 ).toElement().elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
    if ( coordinatesList.size() < 1 )
    {
      continue;
    }
    currentPoint.clear();
    if ( readGML2Coordinates( currentPoint, coordinatesList.at( 0 ).toElement() ) != 0 )
    {
      continue;
    }
    if ( currentPoint.size() < 1 )
    {
      continue;
    }
    pointList.push_back(( *currentPoint.begin() ) );
  }

  //calculate the required wkb size
  int size = 1 + 2 * sizeof( int ) + pointList.size() * ( 2 * sizeof( double ) + 1 + sizeof( int ) );
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiPoint;

  //fill the wkb content
  char e = QgsApplication::endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nPoints = pointList.size(); //number of points
  double x, y;
  memcpy( &( *wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &nPoints, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::list<QgsPoint>::const_iterator it = pointList.begin(); it != pointList.end(); ++it )
  {
    memcpy( &( *wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
    wkbPosition += sizeof( int );
    x = it->x();
    memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
    wkbPosition += sizeof( double );
    y = it->y();
    memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
    wkbPosition += sizeof( double );
  }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2MultiLineString( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  //geoserver has
  //<gml:MultiLineString>
  //<gml:lineStringMember>
  //<gml:LineString>

  //mapserver has directly
  //<gml:MultiLineString
  //<gml:LineString

  std::list<std::list<QgsPoint> > lineCoordinates; //first list: lines, second list: points of one line
  QDomElement currentLineStringElement;
  QDomNodeList currentCoordList;

  QDomNodeList lineStringMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "lineStringMember" );
  if ( lineStringMemberList.size() > 0 ) //geoserver
  {
    for ( int i = 0; i < lineStringMemberList.size(); ++i )
    {
      QDomNodeList lineStringNodeList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "LineString" );
      if ( lineStringNodeList.size() < 1 )
      {
        return 1;
      }
      currentLineStringElement = lineStringNodeList.at( 0 ).toElement();
      currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
      if ( currentCoordList.size() < 1 )
      {
        return 2;
      }
      std::list<QgsPoint> currentPointList;
      if ( readGML2Coordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
      {
        return 3;
      }
      lineCoordinates.push_back( currentPointList );
    }
  }
  else
  {
    QDomNodeList lineStringList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "LineString" );
    if ( lineStringList.size() > 0 ) //mapserver
    {
      for ( int i = 0; i < lineStringList.size(); ++i )
      {
        currentLineStringElement = lineStringList.at( i ).toElement();
        currentCoordList = currentLineStringElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
        if ( currentCoordList.size() < 1 )
        {
          return 4;
        }
        std::list<QgsPoint> currentPointList;
        if ( readGML2Coordinates( currentPointList, currentCoordList.at( 0 ).toElement() ) != 0 )
        {
          return 5;
        }
        lineCoordinates.push_back( currentPointList );
      }
    }
    else
    {
      return 6;
    }
  }


  //calculate the required wkb size
  int size = ( lineCoordinates.size() + 1 ) * ( 1 + 2 * sizeof( int ) );
  for ( std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it )
  {
    size += it->size() * 2 * sizeof( double );
  }
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiLineString;

  //fill the wkb content
  char e = QgsApplication::endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  int nLines = lineCoordinates.size();
  int nPoints; //number of points in a line
  double x, y;
  memcpy( &( *wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &nLines, sizeof( int ) );
  wkbPosition += sizeof( int );
  for ( std::list<std::list<QgsPoint> >::const_iterator it = lineCoordinates.begin(); it != lineCoordinates.end(); ++it )
  {
    memcpy( &( *wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
    wkbPosition += sizeof( int );
    nPoints = it->size();
    memcpy( &( *wkb )[wkbPosition], &nPoints, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( std::list<QgsPoint>::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      x = iter->x();
      //qWarning("x is: " + QString::number(x));
      y = iter->y();
      //qWarning("y is: " + QString::number(y));
      memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
      wkbPosition += sizeof( double );
      memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
      wkbPosition += sizeof( double );
    }
  }
  return 0;
}

int QgsWFSProvider::getWkbFromGML2MultiPolygon( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const
{
  //first list: different polygons, second list: different rings, third list: different points
  std::list<std::list<std::list<QgsPoint> > > multiPolygonPoints;
  QDomElement currentPolygonMemberElement;
  QDomNodeList polygonList;
  QDomElement currentPolygonElement;
  QDomNodeList outerBoundaryList;
  QDomElement currentOuterBoundaryElement;
  QDomElement currentInnerBoundaryElement;
  QDomNodeList innerBoundaryList;
  QDomNodeList linearRingNodeList;
  QDomElement currentLinearRingElement;
  QDomNodeList currentCoordinateList;

  QDomNodeList polygonMemberList = geometryElement.elementsByTagNameNS( GML_NAMESPACE, "polygonMember" );
  for ( int i = 0; i < polygonMemberList.size(); ++i )
  {
    std::list<std::list<QgsPoint> > currentPolygonList;
    currentPolygonMemberElement = polygonMemberList.at( i ).toElement();
    polygonList = currentPolygonMemberElement.elementsByTagNameNS( GML_NAMESPACE, "Polygon" );
    if ( polygonList.size() < 1 )
    {
      continue;
    }
    currentPolygonElement = polygonList.at( 0 ).toElement();

    //find exterior ring
    outerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "outerBoundaryIs" );
    if ( outerBoundaryList.size() < 1 )
    {
      continue;
    }

    currentOuterBoundaryElement = outerBoundaryList.at( 0 ).toElement();
    std::list<QgsPoint> ringCoordinates;

    linearRingNodeList = currentOuterBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, "LinearRing" );
    if ( linearRingNodeList.size() < 1 )
    {
      continue;
    }
    currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
    currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
    if ( currentCoordinateList.size() < 1 )
    {
      continue;
    }
    if ( readGML2Coordinates( ringCoordinates, currentCoordinateList.at( 0 ).toElement() ) != 0 )
    {
      continue;
    }
    currentPolygonList.push_back( ringCoordinates );

    //find interior rings
    QDomNodeList innerBoundaryList = currentPolygonElement.elementsByTagNameNS( GML_NAMESPACE, "innerBoundaryIs" );
    for ( int j = 0; j < innerBoundaryList.size(); ++j )
    {
      std::list<QgsPoint> ringCoordinates;
      currentInnerBoundaryElement = innerBoundaryList.at( j ).toElement();
      linearRingNodeList = currentInnerBoundaryElement.elementsByTagNameNS( GML_NAMESPACE, "LinearRing" );
      if ( linearRingNodeList.size() < 1 )
      {
        continue;
      }
      currentLinearRingElement = linearRingNodeList.at( 0 ).toElement();
      currentCoordinateList = currentLinearRingElement.elementsByTagNameNS( GML_NAMESPACE, "coordinates" );
      if ( currentCoordinateList.size() < 1 )
      {
        continue;
      }
      if ( readGML2Coordinates( ringCoordinates, currentCoordinateList.at( 0 ).toElement() ) != 0 )
      {
        continue;
      }
      currentPolygonList.push_back( ringCoordinates );
    }
    multiPolygonPoints.push_back( currentPolygonList );
  }

  int size = 1 + 2 * sizeof( int );
  //calculate the wkb size
  for ( std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    size += 1 + 2 * sizeof( int );
    for ( std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      size += sizeof( int ) + 2 * iter->size() * sizeof( double );
    }
  }
  *wkb = new unsigned char[size];
  *wkbSize = size;
  *type = QGis::WKBMultiPolygon;
  int polygonType = QGis::WKBPolygon;
  char e = QgsApplication::endian();
  int wkbPosition = 0; //current offset from wkb beginning (in bytes)
  double x, y;
  int nPolygons = multiPolygonPoints.size();
  int nRings;
  int nPointsInRing;

  //fill the contents into *wkb
  memcpy( &( *wkb )[wkbPosition], &e, 1 );
  wkbPosition += 1;
  memcpy( &( *wkb )[wkbPosition], type, sizeof( int ) );
  wkbPosition += sizeof( int );
  memcpy( &( *wkb )[wkbPosition], &nPolygons, sizeof( int ) );
  wkbPosition += sizeof( int );

  for ( std::list<std::list<std::list<QgsPoint> > >::const_iterator it = multiPolygonPoints.begin(); it != multiPolygonPoints.end(); ++it )
  {
    memcpy( &( *wkb )[wkbPosition], &e, 1 );
    wkbPosition += 1;
    memcpy( &( *wkb )[wkbPosition], &polygonType, sizeof( int ) );
    wkbPosition += sizeof( int );
    nRings = it->size();
    memcpy( &( *wkb )[wkbPosition], &nRings, sizeof( int ) );
    wkbPosition += sizeof( int );
    for ( std::list<std::list<QgsPoint> >::const_iterator iter = it->begin(); iter != it->end(); ++iter )
    {
      nPointsInRing = iter->size();
      memcpy( &( *wkb )[wkbPosition], &nPointsInRing, sizeof( int ) );
      wkbPosition += sizeof( int );
      for ( std::list<QgsPoint>::const_iterator iterator = iter->begin(); iterator != iter->end(); ++iterator )
      {
        x = iterator->x();
        y = iterator->y();
        memcpy( &( *wkb )[wkbPosition], &x, sizeof( double ) );
        wkbPosition += sizeof( double );
        memcpy( &( *wkb )[wkbPosition], &y, sizeof( double ) );
        wkbPosition += sizeof( double );
      }
    }
  }
  return 0;
}

int QgsWFSProvider::readGML2Coordinates( std::list<QgsPoint>& coords, const QDomElement elem ) const
{
  QString coordSeparator = ",";
  QString tupelSeparator = " ";
  //"decimal" has to be "."

  coords.clear();

  if ( elem.hasAttribute( "cs" ) )
  {
    coordSeparator = elem.attribute( "cs" );
  }
  if ( elem.hasAttribute( "ts" ) )
  {
    tupelSeparator = elem.attribute( "ts" );
  }

  QStringList tupels = elem.text().split( tupelSeparator, QString::SkipEmptyParts );
  QStringList tupel_coords;
  double x, y;
  bool conversionSuccess;

  QStringList::const_iterator it;
  for ( it = tupels.constBegin(); it != tupels.constEnd(); ++it )
  {
    tupel_coords = ( *it ).split( coordSeparator, QString::SkipEmptyParts );
    if ( tupel_coords.size() < 2 )
    {
      continue;
    }
    x = tupel_coords.at( 0 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    y = tupel_coords.at( 1 ).toDouble( &conversionSuccess );
    if ( !conversionSuccess )
    {
      return 1;
    }
    coords.push_back( QgsPoint( x, y ) );
  }
  return 0;
}

void QgsWFSProvider::handleWFSProgressMessage( int done, int total )
{
  QString totalString;
  if ( total == 0 )
  {
    totalString = tr( "unknown" );
  }
  else
  {
    totalString = QString::number( total );
  }
  QString message( tr( "received %1 bytes from %2" ).arg( done ).arg( totalString ) );
  emit dataReadProgressMessage( message );
}

QDomElement QgsWFSProvider::createGeometryElem( QgsGeometry* geom, QDomDocument& doc ) /*const*/
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement geomElement;

  QString geomTypeName;
  QGis::WkbType wkbType = geom->wkbType();
  switch ( wkbType )
  {
    case QGis::WKBPoint:
      geomElement = createPointElem( geom, doc );
      break;
    case QGis::WKBMultiPoint:
      geomElement = createMultiPointElem( geom, doc );
      break;
    case QGis::WKBLineString:
      geomElement = createLineStringElem( geom, doc );
      break;
    case QGis::WKBMultiLineString:
      geomElement = createMultiLineStringElem( geom, doc );
      break;
    case QGis::WKBPolygon:
      geomElement = createPolygonElem( geom, doc );
      break;
    case QGis::WKBMultiPolygon:
      geomElement = createMultiPolygonElem( geom, doc );
      break;
    default:
      return QDomElement();
  }

  if ( !geomElement.isNull() )
  {
    //append layer srs
    QgsCoordinateReferenceSystem layerCrs = crs();
    if ( layerCrs.isValid() )
    {
      geomElement.setAttribute( "srsName", QString( "EPSG:" ) + QString::number( layerCrs.epsg() ) );
    }
  }
  return geomElement;
}

QDomElement QgsWFSProvider::createLineStringElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement lineStringElem = doc.createElementNS( "http://www.opengis.net/gml", "LineString" );
  QDomElement coordElem = createCoordinateElem( geom->asPolyline(), doc );
  lineStringElem.appendChild( coordElem );
  return lineStringElem;
}

QDomElement QgsWFSProvider::createMultiLineStringElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement multiLineStringElem = doc.createElementNS( "http://www.opengis.net/gml", "MultiLineString" );
  QgsMultiPolyline multiline = geom->asMultiPolyline();

  QgsMultiPolyline::const_iterator multiLineIt = multiline.constBegin();
  for ( ; multiLineIt != multiline.constEnd(); ++multiLineIt )
  {
    QgsGeometry* lineGeom = QgsGeometry::fromPolyline( *multiLineIt );
    if ( lineGeom )
    {
      QDomElement lineStringMemberElem = doc.createElementNS( "http://www.opengis.net/gml", "lineStringMember" );
      QDomElement lineElem = createLineStringElem( lineGeom, doc );
      lineStringMemberElem.appendChild( lineElem );
      multiLineStringElem.appendChild( lineStringMemberElem );
    }
    delete lineGeom;
  }

  return multiLineStringElem;
}

QDomElement QgsWFSProvider::createPointElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement pointElem = doc.createElementNS( "http://www.opengis.net/gml", "Point" );
  QgsPoint p = geom->asPoint();
  QVector<QgsPoint> v;
  v.append( p );
  QDomElement coordElem = createCoordinateElem( v, doc );
  pointElem.appendChild( coordElem );
  return pointElem;
}

QDomElement QgsWFSProvider::createMultiPointElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement multiPointElem = doc.createElementNS( "http://www.opengis.net/gml", "MultiPoint" );
  QgsMultiPoint multiPoint = geom->asMultiPoint();

  QgsMultiPoint::const_iterator multiPointIt = multiPoint.constBegin();
  for ( ; multiPointIt != multiPoint.constEnd(); ++multiPointIt )
  {
    QgsGeometry* pointGeom = QgsGeometry::fromPoint( *multiPointIt );
    if ( pointGeom )
    {
      QDomElement multiPointMemberElem = doc.createElementNS( "http://www.opengis.net/gml", "pointMember" );
      QDomElement pointElem = createPointElem( pointGeom, doc );
      multiPointMemberElem.appendChild( pointElem );
      multiPointElem.appendChild( multiPointMemberElem );
    }
  }
  return multiPointElem;
}

QDomElement QgsWFSProvider::createPolygonElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }

  QDomElement polygonElem = doc.createElementNS( "http://www.opengis.net/gml", "Polygon" );
  QgsPolygon poly = geom->asPolygon();
  for ( int i = 0; i < poly.size(); ++i )
  {
    QString boundaryName;
    if ( i == 0 )
    {
      boundaryName = "outerBoundaryIs";
    }
    else
    {
      boundaryName = "innerBoundaryIs";
    }
    QDomElement boundaryElem = doc.createElementNS( "http://www.opengis.net/gml", boundaryName );
    QDomElement ringElem = doc.createElementNS( "http://www.opengis.net/gml", "LinearRing" );
    QDomElement coordElem = createCoordinateElem( poly.at( i ), doc );
    ringElem.appendChild( coordElem );
    boundaryElem.appendChild( ringElem );
    polygonElem.appendChild( boundaryElem );
  }
  return polygonElem;
}

QDomElement QgsWFSProvider::createMultiPolygonElem( QgsGeometry* geom, QDomDocument& doc ) const
{
  if ( !geom )
  {
    return QDomElement();
  }
  QDomElement multiPolygonElem = doc.createElementNS( "http://www.opengis.net/gml", "MultiPolygon" );
  QgsMultiPolygon multipoly = geom->asMultiPolygon();

  QgsMultiPolygon::const_iterator polyIt = multipoly.constBegin();
  for ( ; polyIt != multipoly.constEnd(); ++polyIt )
  {
    QgsGeometry* polygonGeom = QgsGeometry::fromPolygon( *polyIt );
    if ( polygonGeom )
    {
      QDomElement polygonMemberElem = doc.createElementNS( "http://www.opengis.net/gml", "polygonMember" );
      QDomElement polygonElem = createPolygonElem( polygonGeom, doc );
      delete polygonGeom;
      polygonMemberElem.appendChild( polygonElem );
      multiPolygonElem.appendChild( polygonMemberElem );
    }
  }
  return multiPolygonElem;
}

QDomElement QgsWFSProvider::createCoordinateElem( const QVector<QgsPoint> points, QDomDocument& doc ) const
{
  QDomElement coordElem = doc.createElementNS( "http://www.opengis.net/gml", "coordinates" );
  coordElem.setAttribute( "cs", "," );
  coordElem.setAttribute( "ts", " " );

  QString coordString;
  QVector<QgsPoint>::const_iterator pointIt = points.constBegin();
  for ( ; pointIt != points.constEnd(); ++pointIt )
  {
    if ( pointIt != points.constBegin() )
    {
      coordString += " ";
    }
    coordString += QString::number( pointIt->x() );
    coordString += ",";
    coordString += QString::number( pointIt->y() );
  }

  QDomText coordText = doc.createTextNode( coordString );
  coordElem.appendChild( coordText );
  return coordElem;
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

QString QgsWFSProvider::typeNameFromUrl() const
{
  QStringList urlSplit = dataSourceUri().split( "?" );
  if ( urlSplit.size() > 1 )
  {
    QStringList keyValueSplit = urlSplit.at( 1 ).split( "&" );
    QStringList::const_iterator kvIt = keyValueSplit.constBegin();
    for ( ; kvIt != keyValueSplit.constEnd(); ++kvIt )
    {
      if ( kvIt->startsWith( "typename", Qt::CaseInsensitive ) )
      {
        QStringList equalSplit = kvIt->split( "=" );
        if ( equalSplit.size() > 1 )
        {
          return equalSplit.at( 1 );
        }
      }
    }
  }

  return QString();
}

void QgsWFSProvider::removeNamespacePrefix( QString& tname ) const
{
  if ( tname.contains( ":" ) )
  {
    QStringList splitList = tname.split( ":" );
    if ( splitList.size() > 1 )
    {
      tname = splitList.at( 1 );
    }
  }
}

QString QgsWFSProvider::nameSpacePrefix( const QString& tname ) const
{
  QStringList splitList = tname.split( ":" );
  if ( splitList.size() < 2 )
  {
    return QString();
  }
  return splitList.at( 0 );
}

bool QgsWFSProvider::sendTransactionDocument( const QDomDocument& doc, QDomDocument& serverResponse )
{
  if ( doc.isNull() || !mNetworkRequestFinished )
  {
    return false;
  }

  mNetworkRequestFinished = false;
  QString serverUrl = dataSourceUri().split( "?" ).at( 0 );
  QNetworkRequest request( serverUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "text/xml" );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->post( request, doc.toByteArray( -1 ) );

  connect( reply, SIGNAL( finished() ), this, SLOT( networkRequestFinished() ) );
  while ( !mNetworkRequestFinished )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WFS_THRESHOLD );
  }

  QByteArray response = reply->readAll();
  reply->deleteLater();
  serverResponse.setContent( response, true );

  return true;
}

QDomElement QgsWFSProvider::createTransactionElement( QDomDocument& doc ) const
{
  QDomElement transactionElem = doc.createElementNS( "http://www.opengis.net/wfs", "Transaction" );
  transactionElem.setAttribute( "version", "1.0.0" );
  transactionElem.setAttribute( "service", "WFS" );
  transactionElem.setAttribute( "xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance" );
  transactionElem.setAttribute( "xsi:schemaLocation", mWfsNamespace + " " \
                                + dataSourceUri().replace( QString( "GetFeature" ), QString( "DescribeFeatureType" ) ) );

  QString namespacePrefix = nameSpacePrefix( typeNameFromUrl() );
  if ( !namespacePrefix.isEmpty() )
  {
    transactionElem.setAttribute( "xmlns:" + namespacePrefix, mWfsNamespace );
  }

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

  QDomNodeList transactionResultList = documentElem.elementsByTagNameNS( "http://www.opengis.net/wfs", "TransactionResult" );
  if ( transactionResultList.size() < 1 )
  {
    return false;
  }

  QDomNodeList statusList = transactionResultList.at( 0 ).toElement().elementsByTagNameNS( "http://www.opengis.net/wfs", "Status" );
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

  QDomNodeList insertResultList = rootElem.elementsByTagNameNS( "http://www.opengis.net/wfs", "InsertResult" );
  for ( int i = 0; i < insertResultList.size(); ++i )
  {
    QDomNodeList featureIdList = insertResultList.at( i ).toElement().elementsByTagNameNS( "http://www.opengis.net/ogc", "FeatureId" );
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

int QgsWFSProvider::findNewKey() const
{
  if ( mFeatures.isEmpty() )
  {
    return 0;
  }

  //else return highest key + 1
  QMap<int, QgsFeature*>::const_iterator lastIt = mFeatures.end();
  lastIt --;
  return lastIt.key() + 1;
}

void QgsWFSProvider::getLayerCapabilities()
{
  int capabilities = 0;
  if ( !mNetworkRequestFinished )
  {
    mCapabilities = 0;
    return;
  }
  mNetworkRequestFinished = false;


  //get capabilities document
  QString uri = dataSourceUri();
  uri.replace( QString( "GetFeature" ), QString( "GetCapabilities" ) );
  QNetworkRequest request( uri );
  QNetworkReply* reply = QgsNetworkAccessManager::instance()->get( request );

  connect( reply, SIGNAL( finished() ), this, SLOT( networkRequestFinished() ) );
  while ( !mNetworkRequestFinished )
  {
    QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, WFS_THRESHOLD );
  }
  QByteArray response = reply->readAll();
  reply->deleteLater();

  QDomDocument capabilitiesDocument;
  QString capabilitiesDocError;
  if ( !capabilitiesDocument.setContent( response, true, &capabilitiesDocError ) )
  {
    mCapabilities = 0;
    return;
  }

  //go to <FeatureTypeList>
  QDomElement featureTypeListElem = capabilitiesDocument.documentElement().firstChildElement( "FeatureTypeList" );
  if ( featureTypeListElem.isNull() )
  {
    mCapabilities = 0;
    return;
  }

  QDomElement operationsElem = featureTypeListElem.firstChildElement( "Operations" );
  if ( !operationsElem.isNull() )
  {
    appendSupportedOperations( operationsElem, capabilities );
  }

  //find the <FeatureType> for this layer
  QString thisLayerName = typeNameFromUrl();
  QDomNodeList featureTypeList = featureTypeListElem.elementsByTagName( "FeatureType" );
  for ( int i = 0; i < featureTypeList.size(); ++i )
  {
    QString name = featureTypeList.at( i ).firstChildElement( "Name" ).text();
    if ( name == thisLayerName )
    {
      appendSupportedOperations( featureTypeList.at( i ).firstChildElement( "Operations" ), capabilities );
      break;
    }
  }

  mCapabilities = capabilities;
}

void QgsWFSProvider::appendSupportedOperations( const QDomElement& operationsElem, int& capabilities ) const
{
  if ( operationsElem.isNull() )
  {
    return;
  }

  QDomNodeList childList = operationsElem.childNodes();
  for ( int i = 0; i < childList.size(); ++i )
  {
    QString elemName = childList.at( i ).toElement().tagName();
    if ( elemName == "Insert" )
    {
      capabilities |= QgsVectorDataProvider::AddFeatures;
    }
    else if ( elemName == "Update" )
    {
      capabilities |= QgsVectorDataProvider::ChangeAttributeValues;
      capabilities |= QgsVectorDataProvider::ChangeGeometries;
    }
    else if ( elemName == "Delete" )
    {
      capabilities |= QgsVectorDataProvider::DeleteFeatures;
    }
  }
}

void QgsWFSProvider::handleException( const QDomDocument& serverResponse ) const
{
  QDomElement exceptionElem = serverResponse.documentElement();
  if ( exceptionElem.isNull() || exceptionElem.tagName() != "ServiceExceptionReport" )
  {
    return;
  }

  //possibly this class is used not in a gui application
  if ( QApplication::topLevelWidgets().size() < 1 )
  {
    return;
  }
  QString message = exceptionElem.firstChildElement( "ServiceException" ).text();
  QMessageBox::critical( 0, tr( "Error" ), message );
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
  return true;
}
