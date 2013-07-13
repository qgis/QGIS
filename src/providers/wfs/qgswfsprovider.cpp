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

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgsgml.h"
#include "qgscoordinatereferencesystem.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsprovider.h"
#include "qgsspatialindex.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsnetworkaccessmanager.h"
#include "qgsogcutils.h"

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
    : QgsVectorDataProvider( uri )
    , mNetworkRequestFinished( true )
    , mRequestEncoding( QgsWFSProvider::GET )
    , mUseIntersect( false )
    , mWKBType( QGis::WKBUnknown )
    , mSourceCRS( 0 )
    , mFeatureCount( 0 )
    , mValid( true )
    , mLayer( 0 )
    , mGetRenderedOnly( false )
    , mInitGro( false )
{
  mSpatialIndex = 0;
  if ( uri.isEmpty() )
  {
    mValid = false;
    return;
  }

  //Local url or HTTP?  [WBC 111221] refactored from getFeature()
  if ( uri.startsWith( "http" ) )
  {
    mRequestEncoding = QgsWFSProvider::GET;
  }
  else
  {
    mRequestEncoding = QgsWFSProvider::FILE;
  }

  //create mSourceCRS from url if possible [WBC 111221] refactored from GetFeatureGET()
  QString srsname = parameterFromUrl( "SRSNAME" );
  if ( !srsname.isEmpty() )
  {
    mSourceCRS.createFromOgcWmsCrs( srsname );
  }

  //fetch attributes of layer and type of its geometry attribute
  //WBC 111221: extracting geometry type here instead of getFeature allows successful
  //layer creation even when no features are retrieved (due to, e.g., BBOX or FILTER)
  if ( describeFeatureType( uri, mGeometryAttribute, mFields, mWKBType ) )
  {
    mValid = false;
    QgsMessageLog::instance()->logMessage( tr( "DescribeFeatureType failed for url %1" ).arg( uri ), tr( "WFS" ) );
    return;
  }

  //Failed to detect feature type from describeFeatureType -> get first feature from layer to detect type
  if ( mWKBType == QGis::WKBUnknown )
  {
    QString bkUri = dataSourceUri();
    QUrl typeDetectionUri( uri );
    typeDetectionUri.removeQueryItem( "BBOX" );
    typeDetectionUri.addQueryItem( "MAXFEATURES", "1" );
    setDataSourceUri( typeDetectionUri.toString() );
    reloadData();
    setDataSourceUri( bkUri );
  }

  if ( ! uri.contains( "BBOX" ) )
  { //"Cache Features" option; get all features in layer immediately
    reloadData();
  } //otherwise, defer feature retrieval until layer is first rendered

  if ( mValid )
  {
    getLayerCapabilities();
  }
}

QgsWFSProvider::~QgsWFSProvider()
{
  while ( !mActiveIterators.empty() )
  {
    QgsWFSFeatureIterator *it = *mActiveIterators.begin();
    QgsDebugMsg( "closing active iterator" );
    it->close();
  }

  deleteData();
  delete mSpatialIndex;
}

void QgsWFSProvider::reloadData()
{
  deleteData();
  delete mSpatialIndex;
  mSpatialIndex = new QgsSpatialIndex();
  mValid = !getFeature( dataSourceUri() );
}

void QgsWFSProvider::deleteData()
{
  mSelectedFeatures.clear();
  for ( int i = 0; i < mFeatures.size(); i++ )
  {
    delete mFeatures[i];
  }
  mFeatures.clear();
}

void QgsWFSProvider::copyFeature( QgsFeature* f, QgsFeature& feature, bool fetchGeometry, QgsAttributeList fetchAttributes )
{
  Q_UNUSED( fetchGeometry );

  if ( !f )
  {
    return;
  }

  //copy the geometry
  QgsGeometry* geometry = f->geometry();
  if ( geometry && fetchGeometry )
  {
    const unsigned char *geom = geometry->asWkb();
    int geomSize = geometry->wkbSize();
    unsigned char* copiedGeom = new unsigned char[geomSize];
    memcpy( copiedGeom, geom, geomSize );
    feature.setGeometryAndOwnership( copiedGeom, geomSize );
  }
  else
  {
    feature.setGeometry( 0 );
  }

  //and the attributes
  const QgsAttributes& attributes = f->attributes();
  feature.setAttributes( attributes );

  int i = 0;
  for ( QgsAttributeList::const_iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it )
  {
    feature.setAttribute( i, attributes[*it] );
    ++i;
  }

  //id and valid
  feature.setValid( true );
  feature.setFeatureId( f->id() );
  feature.setFields( &mFields ); // allow name-based attribute lookups
}

QGis::WkbType QgsWFSProvider::geometryType() const
{
  return mWKBType;
}

long QgsWFSProvider::featureCount() const
{
  return mFeatureCount;
}

const QgsFields& QgsWFSProvider::fields() const
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

QgsFeatureIterator QgsWFSProvider::getFeatures( const QgsFeatureRequest& request )
{
  if ( !( request.flags() & QgsFeatureRequest::NoGeometry ) )
  {
    QgsRectangle rect = request.filterRect();
    if ( !rect.isEmpty() )
    {
      QString dsURI = dataSourceUri();
      //first time through, initialize GetRenderedOnly args
      //ctor cannot initialize because layer object not available then
      if ( ! mInitGro )
      { //did user check "Cache Features" in WFS layer source selection?
        if ( dsURI.contains( "BBOX" ) )
        { //no: initialize incremental getFeature
          if ( initGetRenderedOnly( rect ) )
          {
            mGetRenderedOnly = true;
          }
          else
          { //initialization failed;
            QgsDebugMsg( QString( "GetRenderedOnly initialization failed; incorrect operation may occur\n%1" )
                         .arg( dataSourceUri() ) );
            QMessageBox( QMessageBox::Warning, "Non-Cached layer initialization failed!",
                         QString( "Incorrect operation may occur:\n%1" ).arg( dataSourceUri() ) );
          }
        }
        mInitGro = true;
      }

      if ( mGetRenderedOnly )
      { //"Cache Features" was not selected for this layer
        //has rendered extent expanded beyond last-retrieved WFS extent?
        //NB: "intersect" instead of "contains" tolerates rounding errors;
        //    avoids unnecessary second fetch on zoom-in/zoom-out sequences
        QgsRectangle olap( rect );
        olap = olap.intersect( &mGetExtent );
        if ( qgsDoubleNear( rect.width(), olap.width() ) && qgsDoubleNear( rect.height(), olap.height() ) )
        { //difference between canvas and layer extents is within rounding error: do not re-fetch
          QgsDebugMsg( QString( "Layer %1 GetRenderedOnly: no fetch required" ).arg( mLayer->name() ) );
        }
        else
        { //combined old and new extents might speed up local panning & zooming
          mGetExtent.combineExtentWith( &rect );
          //but see if the combination is useless or too big
          double pArea = mGetExtent.width() * mGetExtent.height();
          double cArea = rect.width() * rect.height();
          if ( olap.isEmpty() || pArea > ( cArea * 4.0 ) )
          { //new canvas extent does not overlap or combining old and new extents would
            //fetch > 4 times the area to be rendered; get only what will be rendered
            mGetExtent = rect;
          }
          QgsDebugMsg( QString( "Layer %1 GetRenderedOnly: fetching extent %2" )
                       .arg( mLayer->name(), mGetExtent.asWktCoordinates() ) );
          dsURI = dsURI.replace( QRegExp( "BBOX=[^&]*" ),
                                 QString( "BBOX=%1,%2,%3,%4" )
                                 .arg( mGetExtent.xMinimum(), 0, 'f' )
                                 .arg( mGetExtent.yMinimum(), 0, 'f' )
                                 .arg( mGetExtent.xMaximum(), 0, 'f' )
                                 .arg( mGetExtent.yMaximum(), 0, 'f' ) );
          //TODO: BBOX may not be combined with FILTER. WFS spec v. 1.1.0, sec. 14.7.3 ff.
          //      if a FILTER is present, the BBOX must be merged into it, capabilities permitting.
          //      Else one criterion must be abandoned and the user warned.  [WBC 111221]
          setDataSourceUri( dsURI );
          reloadData();
          mLayer->updateExtents();
        }
      }
    }

  }
  return QgsFeatureIterator( new QgsWFSFeatureIterator( this, request ) );
}

int QgsWFSProvider::getFeature( const QString& uri )
{
  if ( mRequestEncoding == QgsWFSProvider::GET )
  {
    return getFeatureGET( uri, mGeometryAttribute );
  }
  else  //local file
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
  QString tname = parameterFromUrl( "typename" );
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
    const QgsFields* fields = featureIt->fields();
    if ( !fields )
    {
      continue;
    }

    QgsAttributes featureAttributes = featureIt->attributes();
    int nAttrs = featureAttributes.size();
    for ( int i = 0; i < nAttrs; ++i )
    {
      const QVariant& value = featureAttributes.at( i );
      if ( value.isValid() && !value.isNull() )
      {
        QDomElement fieldElem = transactionDoc.createElementNS( mWfsNamespace, fields->field( i ).name() );
        QDomText fieldText = transactionDoc.createTextNode( value.toString() );
        fieldElem.appendChild( fieldText );
        featureElem.appendChild( fieldElem );
      }
    }

    //add geometry column (as gml)
    QDomElement geomElem = transactionDoc.createElementNS( mWfsNamespace, mGeometryAttribute );
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( featureIt->geometry(), transactionDoc );
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
        QgsFeatureId newId = findNewKey();
        featureIt->setFeatureId( newId );
        mFeatures.insert( newId, new QgsFeature( *featureIt ) );
        mIdMap.insert( newId, *idIt );
        mSpatialIndex->insertFeature( *featureIt );
        mFeatureCount = mFeatures.size();
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

bool QgsWFSProvider::deleteFeatures( const QgsFeatureIds &id )
{
  if ( id.size() < 1 )
  {
    return true;
  }

  //find out typename from uri and strip namespace prefix
  QString tname = parameterFromUrl( "typename" );
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
    QMap< QgsFeatureId, QString >::const_iterator fidIt = mIdMap.find( *idIt );
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
      QMap<QgsFeatureId, QgsFeature* >::iterator fIt = mFeatures.find( *idIt );
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
  QString tname = parameterFromUrl( "typename" );
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
    QMap< QgsFeatureId, QString >::const_iterator fidIt = mIdMap.find( geomIt.key() );
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
    QDomElement gmlElem = QgsOgcUtils::geometryToGML( &geomIt.value(), transactionDoc );
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
      QMap<QgsFeatureId, QgsFeature* >::iterator fIt = mFeatures.find( geomIt.key() );
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
  QString tname = parameterFromUrl( "typename" );
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
    //find out wfs server feature id
    QMap< QgsFeatureId, QString >::const_iterator fidIt = mIdMap.find( attIt.key() );
    if ( fidIt == mIdMap.constEnd() )
    {
      continue;
    }

    QDomElement updateElem = transactionDoc.createElementNS( "http://www.opengis.net/wfs", "Update" );
    updateElem.setAttribute( "typeName", tname );

    QgsAttributeMap::const_iterator attMapIt = attIt.value().constBegin();
    for ( ; attMapIt != attIt.value().constEnd(); ++attMapIt )
    {
      QString fieldName = mFields.at( attMapIt.key() ).name();
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
      QMap<QgsFeatureId, QgsFeature*>::iterator fIt = mFeatures.find( attIt.key() );
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
        currentFeature->setAttribute( attMapIt.key(), attMapIt.value() );
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

int QgsWFSProvider::describeFeatureType( const QString& uri, QString& geometryAttribute,
    QgsFields& fields, QGis::WkbType& geomType )
//NB: also called from QgsWFSSourceSelect::on_treeWidget_itemDoubleClicked() to build filters.
//    a temporary provider object is constructed with a null URI, which bypasses much provider
//    instantiation logic: refresh(), getFeature(), etc.  therefore, many provider class members
//    are only default values or uninitialized when running under the source select dialog!
{
  fields.clear();
  //Local url or HTTP?  WBC111221 refactored here from getFeature()
  switch ( mRequestEncoding )
  {
    case QgsWFSProvider::GET:
    {
      return describeFeatureTypeGET( uri, geometryAttribute, fields, geomType );
    }
    case QgsWFSProvider::FILE:
    {
      return describeFeatureTypeFile( uri, geometryAttribute, fields, geomType );
    }
  }
  QgsDebugMsg( "SHOULD NOT OCCUR: mEncoding undefined" );
  return 1;
}

int QgsWFSProvider::getFeatureGET( const QString& uri, const QString& geometryAttribute )
{
  //the new and faster method with the expat SAX parser

  //allows fast searchings with attribute name. Also needed is attribute Index and type infos
  QMap<QString, QPair<int, QgsField> > thematicAttributes;
  for ( int i = 0; i < mFields.count(); ++i )
  {
    thematicAttributes.insert( mFields.at( i ).name(), qMakePair( i, mFields.at( i ) ) );
  }

  QString typeName = parameterFromUrl( "typename" );
  //QgsWFSData dataReader( uri, &mExtent, mFeatures, mIdMap, geometryAttribute, thematicAttributes, &mWKBType );
  QgsGml dataReader( typeName, geometryAttribute, mFields );
  //dataReader.setFeatureType( typeName, geometryAttribute, mFields );

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

  //if ( dataReader.getWFSData() != 0 )
  if ( dataReader.getFeatures( uri, &mWKBType, &mExtent ) != 0 )
  {
    QgsDebugMsg( "getWFSData returned with error" );
    return 1;
  }
  mFeatures = dataReader.featuresMap();
  mIdMap = dataReader.idsMap();

  QgsDebugMsg( QString( "feature count after request is: %1" ).arg( mFeatures.size() ) );
  QgsDebugMsg( QString( "mExtent after request is: %1" ).arg( mExtent.toString() ) );

  if ( mWKBType != QGis::WKBNoGeometry )
  {
    for ( QMap<QgsFeatureId, QgsFeature*>::iterator it = mFeatures.begin(); it != mFeatures.end(); ++it )
    {
      QgsDebugMsg( "feature " + FID_TO_STRING(( *it )->id() ) );
      mSpatialIndex->insertFeature( *( it.value() ) );
    }
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
  if ( mWKBType != QGis::WKBNoGeometry && getExtentFromGML2( &mExtent, featureCollectionElement ) != 0 )
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

int QgsWFSProvider::describeFeatureTypeGET( const QString& uri, QString& geometryAttribute, QgsFields& fields, QGis::WkbType& geomType )
{
  if ( !mNetworkRequestFinished )
  {
    return 1;
  }

  mNetworkRequestFinished = false;
  QUrl describeFeatureUrl( uri );
  describeFeatureUrl.removeQueryItem( "SRSNAME" );
  describeFeatureUrl.removeQueryItem( "REQUEST" );
  describeFeatureUrl.addQueryItem( "REQUEST", "DescribeFeatureType" );
  QNetworkRequest request( describeFeatureUrl.toString() );
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

  if ( readAttributesFromSchema( describeFeatureDocument,
                                 geometryAttribute, fields, geomType ) != 0 )
  {
    QgsDebugMsg( QString( "FAILED: readAttributesFromSchema" ) );
    return 3;
  }

  return 0;
}

void QgsWFSProvider::networkRequestFinished()
{
  mNetworkRequestFinished = true;
}

int QgsWFSProvider::describeFeatureTypeFile( const QString& uri, QString& geometryAttribute, QgsFields& fields, QGis::WkbType& geomType )
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

    if ( readAttributesFromSchema( schemaDoc, geometryAttribute, fields, geomType ) != 0 )
    {
      return 2;
    }
    return 0;
  }

  std::list<QString> thematicAttributes;

  //if this fails (e.g. no schema file), try to guess the geometry attribute and the names of the thematic attributes from the .gml file
  if ( guessAttributesFromFile( uri, geometryAttribute, thematicAttributes, geomType ) != 0 )
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

int QgsWFSProvider::readAttributesFromSchema( QDomDocument& schemaDoc, QString& geometryAttribute, QgsFields& fields, QGis::WkbType& geomType )
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

  bool foundGeometryAttribute = false;

  for ( uint i = 0; i < attributeNodeList.length(); ++i )
  {
    QDomElement attributeElement = attributeNodeList.at( i ).toElement();
    //attribute name
    QString name = attributeElement.attribute( "name" );
    //attribute type
    QString type = attributeElement.attribute( "type" );

    //is it a geometry attribute?
    //MH 090428: sometimes the <element> tags for geometry attributes have only attribute ref="gml:polygonProperty" and no name
    QRegExp gmlPT( "gml:(.*)PropertyType" );
    if ( type.indexOf( gmlPT ) == 0 || name.isEmpty() )
    {
      foundGeometryAttribute = true;
      geometryAttribute = name;
      geomType = geomTypeFromPropertyType( geometryAttribute, gmlPT.cap( 1 ) );
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
      fields.append( QgsField( name, attributeType, type ) );
    }
  }
  if ( !foundGeometryAttribute )
  {
    geomType = QGis::WKBNoGeometry;
  }

  return 0;
}

int QgsWFSProvider::guessAttributesFromFile( const QString& uri, QString& geometryAttribute, std::list<QString>& thematicAttributes, QGis::WkbType& geomType ) const
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
  bool foundGeometryAttribute = false;

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

  if ( !foundGeometryAttribute )
  {
    geomType = QGis::WKBNoGeometry;
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
  QgsFeature* f = 0;
  unsigned char* wkb = 0;
  int wkbSize = 0;
  mFeatureCount = 0;

  for ( int i = 0; i < featureTypeNodeList.size(); ++i )
  {
    f = new QgsFeature( mFeatureCount );
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
            f->setAttribute( attr++, QVariant( currentAttributeElement.text().toDouble() ) );
          }
          else
          {
            f->setAttribute( attr++, QVariant( currentAttributeElement.text() ) );
          }
        }
        else //a geometry attribute
        {
          f->setGeometry( QgsOgcUtils::geometryFromGML( currentAttributeElement ) );
        }
      }
      currentAttributeChild = currentAttributeChild.nextSibling();
    }
    if ( wkb && wkbSize > 0 )
    {
      //insert bbox and pointer to feature into search tree
      mSpatialIndex->insertFeature( *f );
    }

    mFeatures.insert( f->id(), f );
    ++mFeatureCount;
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

QString QgsWFSProvider::parameterFromUrl( const QString& name ) const
{
  QStringList urlSplit = dataSourceUri().split( "?" );
  if ( urlSplit.size() > 1 )
  {
    QStringList keyValueSplit = urlSplit.at( 1 ).split( "&" );
    QStringList::const_iterator kvIt = keyValueSplit.constBegin();
    for ( ; kvIt != keyValueSplit.constEnd(); ++kvIt )
    {
      if ( kvIt->startsWith( name, Qt::CaseInsensitive ) )
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

  QUrl typeDetectionUri( dataSourceUri() );
  typeDetectionUri.removeQueryItem( "REQUEST" );
  typeDetectionUri.removeQueryItem( "TYPENAME" );
  typeDetectionUri.removeQueryItem( "BBOX" );
  typeDetectionUri.removeQueryItem( "FILTER" );
  typeDetectionUri.removeQueryItem( "FEATUREID" );
  typeDetectionUri.removeQueryItem( "PROPERTYNAME" );
  typeDetectionUri.removeQueryItem( "MAXFEATURES" );
  typeDetectionUri.removeQueryItem( "OUTPUTFORMAT" );
  QString serverUrl = typeDetectionUri.toString();

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
  transactionElem.setAttribute( "xsi:schemaLocation", mWfsNamespace + " "
                                + dataSourceUri().replace( QString( "GetFeature" ), QString( "DescribeFeatureType" ) ) );

  QString namespacePrefix = nameSpacePrefix( parameterFromUrl( "typename" ) );
  if ( !namespacePrefix.isEmpty() )
  {
    transactionElem.setAttribute( "xmlns:" + namespacePrefix, mWfsNamespace );
  }
  transactionElem.setAttribute( "xmlns:gml", GML_NAMESPACE );

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

QgsFeatureId QgsWFSProvider::findNewKey() const
{
  if ( mFeatures.isEmpty() )
  {
    return 0;
  }

  //else return highest key + 1
  QMap<QgsFeatureId, QgsFeature*>::const_iterator lastIt = mFeatures.end();
  lastIt--;
  QgsFeatureId id = lastIt.key();
  return ++id;
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
  QString thisLayerName = parameterFromUrl( "typename" );
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

//initialization for getRenderedOnly option
//(formerly "Only request features overlapping the current view extent")
bool QgsWFSProvider::initGetRenderedOnly( const QgsRectangle rect )
{
  Q_UNUSED( rect );

  //find our layer
  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  QMap<QString, QgsMapLayer*>::const_iterator layersIt = layers.begin();
  for ( ; layersIt != layers.end() ; ++layersIt )
  {
    if (( mLayer = dynamic_cast<QgsVectorLayer*>( layersIt.value() ) ) )
    {
      if ( mLayer->dataProvider() == this )
      {
        QgsDebugMsg( QString( "found layer %1" ).arg( mLayer->name() ) );
        break;
      }
    }
  }
  if ( layersIt == layers.end() )
  {
    QgsDebugMsg( "SHOULD NOT OCCUR: initialize() did not find layer." );
    return false;
  }
  return true;
}

QGis::WkbType QgsWFSProvider::geomTypeFromPropertyType( QString attName, QString propType )
{
  Q_UNUSED( attName );
  const QStringList geomTypes = ( QStringList()
                                  //all GML v.2.1.3 _geometryProperty group members, except MultiGeometryPropertyType
                                  //sequence must exactly match enum Qgis::WkbType
                                  << ""  // unknown geometry, enum 0
                                  << "Point"
                                  << "LineString"
                                  << "Polygon"
                                  << "MultiPoint"
                                  << "MultiLineString"
                                  << "MultiPolygon" );

  QgsDebugMsg( QString( "DescribeFeatureType geometry attribute \"%1\" type is \"%2\"" )
               .arg( attName, propType ) );
  int i = geomTypes.indexOf( propType );
  if ( i <= 0 )
  { // feature type missing or unknown
    i = ( int ) QGis::WKBUnknown;
  }
  return ( QGis::WkbType ) i;
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
