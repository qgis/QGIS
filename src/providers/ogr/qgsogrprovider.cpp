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
/* $Id$ */

#include "qgsogrprovider.h"
#include "qgslogger.h"

#include <iostream>
#include <cassert>

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>         // to collect version information
#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <cpl_error.h>

#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QMap>
#include <QString>

//TODO Following ifndef can be removed once WIN32 GEOS support
//    is fixed
#ifndef NOWIN32GEOSXXX
//XXX GEOS support on windows is broken until we can get VC++ to
//    tolerate geos.h without throwing a bunch of type errors. It
//    appears that the windows version of GEOS may be compiled with 
//    MINGW rather than VC++.
#endif 


#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsspatialrefsys.h"

static const QString TEXT_PROVIDER_KEY = "ogr";
static const QString TEXT_PROVIDER_DESCRIPTION = 
               QString("OGR data provider")
             + " (compiled against GDAL/OGR library version "
             + GDAL_RELEASE_NAME
             + ", running against GDAL/OGR library version "
             + GDALVersionInfo("RELEASE_NAME")
             + ")";



QgsOgrProvider::QgsOgrProvider(QString const & uri)
 : QgsVectorDataProvider(uri),
   ogrDataSource(0),
   extent_(0),
   ogrLayer(0),
   ogrDriver(0)
{
  OGRRegisterAll();

  // set the selection rectangle pointer to 0
  mSelectionRectangle = 0;
  // make connection to the data source

  QgsDebugMsg("Data source uri is " + uri);

  // try to open for update, but disable error messages to avoid a
  // message if the file is read only, because we cope with that
  // ourselves.
  CPLPushErrorHandler(CPLQuietErrorHandler);
  ogrDataSource = OGROpen(QFile::encodeName(uri).constData(), TRUE, &ogrDriver);
  CPLPopErrorHandler();

  if(ogrDataSource == NULL)
  {
    // try to open read-only
    ogrDataSource = OGROpen(QFile::encodeName(uri).constData(), FALSE, &ogrDriver);

    //TODO Need to set a flag or something to indicate that the layer
    //TODO is in read-only mode, otherwise edit ops will fail
    //TODO: capabilities() should now reflect this; need to test.
  }
  if (ogrDataSource != NULL) {

    QgsDebugMsg("Data source is valid");
    QgsDebugMsg("OGR Driver was " + QString(OGR_Dr_GetName(ogrDriver)));

    valid = true;

    ogrDriverName = OGR_Dr_GetName(ogrDriver);

    ogrLayer = OGR_DS_GetLayer(ogrDataSource,0);

    // get the extent_ (envelope) of the layer

    QgsDebugMsg("Starting get extent\n");

    // TODO: This can be expensive, do we really need it!

    extent_ = calloc(sizeof(OGREnvelope),1);
    OGR_L_GetExtent(ogrLayer,(OGREnvelope *) extent_, TRUE );

    QgsDebugMsg("Finished get extent\n");

    // getting the total number of features in the layer
    // TODO: This can be expensive, do we really need it!
    numberFeatures = OGR_L_GetFeatureCount(ogrLayer, TRUE);

    // check the validity of the layer

    QgsDebugMsg("checking validity\n");
    loadFields();
    QgsDebugMsg("Done checking validity\n");
  } else {
    QgsLogger::critical("Data source is invalid");
    const char *er = CPLGetLastErrorMsg();
    QgsLogger::critical(er);
    valid = false;
  }

  // create the geos objects
  geometryFactory = new GEOS_GEOM::GeometryFactory();
  assert(geometryFactory!=0);

  mSupportedNativeTypes.insert("Integer");
  mSupportedNativeTypes.insert("Real");
  mSupportedNativeTypes.insert("String");
}

QgsOgrProvider::~QgsOgrProvider()
{
  OGR_DS_Destroy(ogrDataSource);
  ogrDataSource = 0;
  free(extent_);
  extent_ = 0;
  delete geometryFactory;
  if( mSelectionRectangle )
  {
    OGR_G_DestroyGeometry( mSelectionRectangle );
    mSelectionRectangle = 0;
  }
}

void QgsOgrProvider::setEncoding(const QString& e)
{
  QgsVectorDataProvider::setEncoding(e);
  loadFields();
}

void QgsOgrProvider::loadFields()
{
  //the attribute fields need to be read again when the encoding changes
  mAttributeFields.clear();
  OGRFeatureDefnH fdef = OGR_L_GetLayerDefn(ogrLayer);
  if(fdef)
  {
    geomType = OGR_FD_GetGeomType(fdef);

    //Some ogr drivers (e.g. GML) are not able to determine the geometry type of a layer like this.
    //In such cases, we examine the first feature 
    if(geomType == wkbUnknown) 
    {
      OGR_L_ResetReading(ogrLayer);
      OGRFeatureH firstFeature = OGR_L_GetNextFeature(ogrLayer);
      if(firstFeature)
      {
        OGRGeometryH firstGeometry = OGR_F_GetGeometryRef(firstFeature);
        if(firstGeometry)
        {
          geomType = OGR_G_GetGeometryType(firstGeometry);
        }
        OGR_F_Destroy( firstFeature );
      }
      OGR_L_ResetReading(ogrLayer);
    }

    for(int i=0;i<OGR_FD_GetFieldCount(fdef);++i)
    {
      OGRFieldDefnH fldDef = OGR_FD_GetFieldDefn(fdef,i);
      OGRFieldType ogrType = OGR_Fld_GetType(fldDef);
      QVariant::Type varType;
      switch (ogrType)
      {
      case OFTInteger: varType = QVariant::Int; break;
      case OFTReal: varType = QVariant::Double; break;
        // unsupported in OGR 1.3
        //case OFTDateTime: varType = QVariant::DateTime; break;
#if GDAL_VERSION_NUM >= 1400
      case OFTString: varType = QVariant::String; break;
#endif
      default: varType = QVariant::String; // other unsupported, leave it as a string
      }

      mAttributeFields.insert(
        i, QgsField(
        mEncoding->toUnicode(OGR_Fld_GetNameRef(fldDef)), varType,
        mEncoding->toUnicode(OGR_GetFieldTypeName(ogrType)),
        OGR_Fld_GetWidth(fldDef),
        OGR_Fld_GetPrecision(fldDef) ));
    }
  }
}


QString QgsOgrProvider::storageType() const
{
  // Delegate to the driver loaded in by OGR
  return ogrDriverName;
}


bool QgsOgrProvider::getFeatureAtId(int featureId,
                                    QgsFeature& feature,
                                    bool fetchGeometry,
                                    QgsAttributeList fetchAttributes)
{
  OGRFeatureH fet = OGR_L_GetFeature(ogrLayer,featureId);
  if (fet == NULL)
    return false;

  feature.setFeatureId(OGR_F_GetFID(fet));

  /* fetch geometry */
  if (fetchGeometry)
  {
    OGRGeometryH geom = OGR_F_GetGeometryRef(fet);

    // get the wkb representation
    unsigned char *wkb = new unsigned char[OGR_G_WkbSize(geom)];
    OGR_G_ExportToWkb(geom,(OGRwkbByteOrder) QgsApplication::endian(), wkb);

    feature.setGeometryAndOwnership(wkb, OGR_G_WkbSize(geom));
  }

  /* fetch attributes */
  for(QgsAttributeList::iterator it = fetchAttributes.begin(); it != fetchAttributes.end(); ++it)
  {
    getFeatureAttribute(fet,feature,*it);
  }

  return true;
}

bool QgsOgrProvider::getNextFeature(QgsFeature& feature)
{
  if (!valid)
  {
    QgsLogger::warning("Read attempt on an invalid shapefile data source");
    return false;
  }

  OGRFeatureH fet;
  QgsRect selectionRect;

  while ((fet = OGR_L_GetNextFeature(ogrLayer)) != NULL)
  {
    // skip features without geometry
    if (OGR_F_GetGeometryRef(fet) == NULL && !mFetchFeaturesWithoutGeom)
    {
      OGR_F_Destroy( fet );
      continue;
    }

    OGRFeatureDefnH featureDefinition = OGR_F_GetDefnRef(fet);
    QString featureTypeName = featureDefinition ? QString(OGR_FD_GetName(featureDefinition)) : QString("");
    feature.setFeatureId(OGR_F_GetFID(fet));
    feature.setTypeName(featureTypeName);

    /* fetch geometry */
    if (mFetchGeom)
    {
      OGRGeometryH geom = OGR_F_GetGeometryRef(fet);

      // get the wkb representation
      unsigned char *wkb = new unsigned char[OGR_G_WkbSize(geom)];
      OGR_G_ExportToWkb(geom,(OGRwkbByteOrder) QgsApplication::endian(), wkb);

      feature.setGeometryAndOwnership(wkb, OGR_G_WkbSize(geom));

      if (mUseIntersect)
      {
        //precise test for intersection with search rectangle
        //first make QgsRect from OGRPolygon
        OGREnvelope env;
        memset( &env, 0, sizeof(env) );
        if(mSelectionRectangle)
          OGR_G_GetEnvelope(mSelectionRectangle,&env);
        if(env.MinX != 0 || env.MinY != 0 || env.MaxX != 0 || env.MaxY != 0 ) //if envelope is invalid, skip the precise intersection test
        {
          selectionRect.set(env.MinX, env.MinY, env.MaxX, env.MaxY);
          if(!feature.geometry()->intersects(selectionRect))
          {
            OGR_F_Destroy( fet );
            continue;
          }
        }

      }
    }

    /* fetch attributes */
    for(QgsAttributeList::iterator it = mAttributesToFetch.begin(); it != mAttributesToFetch.end(); ++it)
    {
      getFeatureAttribute(fet,feature,*it);
    }

    /* we have a feature, end this cycle */
    break;

  } /* while */

  if (fet)
  {
    OGR_F_Destroy( fet );
    return true;
  }
  else
  {
    QgsDebugMsg("Feature is null");  
    // probably should reset reading here
    OGR_L_ResetReading(ogrLayer);
    return false;
  }
}

void QgsOgrProvider::select(QgsAttributeList fetchAttributes, QgsRect rect, bool fetchGeometry, \
			    bool useIntersect)
{
  mUseIntersect = useIntersect;
  mAttributesToFetch = fetchAttributes;
  mFetchGeom = fetchGeometry;

  // spatial query to select features
  if(rect.isEmpty())
  {
    OGR_L_SetSpatialFilter(ogrLayer,0);
  }
  else
  {
    OGRGeometryH filter = 0;
    QString wktExtent = QString("POLYGON ((%1))").arg(rect.asPolygon());
    const char *wktText = wktExtent.toAscii();

    if(useIntersect)
    {
      // store the selection rectangle for use in filtering features during
      // an identify and display attributes
      if( mSelectionRectangle )
        OGR_G_DestroyGeometry( mSelectionRectangle );

      OGR_G_CreateFromWkt( (char **)&wktText,
        NULL, &mSelectionRectangle);
    }

    wktText = wktExtent.toAscii();
    OGR_G_CreateFromWkt( (char **)&wktText, NULL, &filter );
    QgsDebugMsg("Setting spatial filter using " + wktExtent);
    OGR_L_SetSpatialFilter( ogrLayer, filter );
    OGR_G_DestroyGeometry( filter );
  }  
}


unsigned char * QgsOgrProvider::getGeometryPointer(OGRFeatureH fet)
{
  OGRGeometryH geom = OGR_F_GetGeometryRef(fet);
  unsigned char *gPtr=0;

  if( geom == NULL )
    return NULL;

  // get the wkb representation
  gPtr = new unsigned char[OGR_G_WkbSize(geom)];

  OGR_G_ExportToWkb(geom,(OGRwkbByteOrder) QgsApplication::endian(), gPtr);
  return gPtr;
}


QgsRect QgsOgrProvider::extent()
{
  OGREnvelope *ext = (OGREnvelope *) extent_;
  mExtentRect.set(ext->MinX, ext->MinY, ext->MaxX, ext->MaxY);
  return mExtentRect;
}


size_t QgsOgrProvider::layerCount() const
{
  return OGR_DS_GetLayerCount(ogrDataSource);
} // QgsOgrProvider::layerCount()


/** 
 * Return the feature type
 */
QGis::WKBTYPE QgsOgrProvider::geometryType() const
{
  return (QGis::WKBTYPE) geomType;
}

/** 
 * Return the feature type
 */
long QgsOgrProvider::featureCount() const
{
  return numberFeatures;
}

/**
 * Return the number of fields
 */
uint QgsOgrProvider::fieldCount() const
{
  return mAttributeFields.size();
}

void QgsOgrProvider::getFeatureAttribute(OGRFeatureH ogrFet, QgsFeature & f, int attindex)
{
  OGRFieldDefnH fldDef = OGR_F_GetFieldDefnRef(ogrFet, attindex);

  if ( ! fldDef )
  {
    QgsDebugMsg("ogrFet->GetFieldDefnRef(attindex) returns NULL");
    return;
  }

  QVariant value;

  if( OGR_F_IsFieldSet(ogrFet, attindex) )
  {
    switch (mAttributeFields[attindex].type())
    {
    case QVariant::String: value = QVariant( mEncoding->toUnicode( OGR_F_GetFieldAsString(ogrFet,attindex) ) ); break;
    case QVariant::Int: value = QVariant( OGR_F_GetFieldAsInteger( ogrFet, attindex ) ); break;
    case QVariant::Double: value = QVariant( OGR_F_GetFieldAsDouble( ogrFet, attindex ) ); break;
      //case QVariant::DateTime: value = QVariant(QDateTime::fromString(str)); break;
    default: assert(NULL && "unsupported field type");
    }
  }
  else
  {
    value = QVariant( QString::null );
  }

  f.addAttribute(attindex, value);
}


const QgsFieldMap & QgsOgrProvider::fields() const
{
  return mAttributeFields;
}

void QgsOgrProvider::reset()
{
  OGR_L_ResetReading(ogrLayer);
}


//TODO - add sanity check for shape file layers, to include cheking to
//       see if the .shp, .dbf, .shx files are all present and the layer
//       actually has features
bool QgsOgrProvider::isValid()
{
  return valid;
}

bool QgsOgrProvider::addFeature(QgsFeature& f)
{ 
  bool returnValue = true;
  OGRFeatureDefnH fdef=OGR_L_GetLayerDefn(ogrLayer);
  OGRFeatureH feature= OGR_F_Create(fdef);
  QGis::WKBTYPE ftype = f.geometry()->wkbType();
  unsigned char* wkb = f.geometry()->wkbBuffer();

  if( f.geometry()->wkbSize() > 0 )
  {
    OGRGeometryH geom = NULL;

    if( OGR_G_CreateFromWkb( wkb, NULL, &geom, f.geometry()->wkbSize() )
      != OGRERR_NONE )
    {
      return false;
    }

    OGR_F_SetGeometryDirectly( feature, geom );
  }

  QgsAttributeMap attrs = f.attributeMap();

  //add possible attribute information
  for(QgsAttributeMap::iterator it = attrs.begin(); it != attrs.end(); ++it)
  {
    int targetAttributeId = it.key();

    // don't try to set field from attribute map if it's not present in layer
    if (targetAttributeId >= OGR_FD_GetFieldCount(fdef))
      continue;

    //if(!s.isEmpty())
    // continue;
    //
    OGRFieldDefnH fldDef = OGR_FD_GetFieldDefn( fdef, targetAttributeId );
    OGRFieldType type = OGR_Fld_GetType( fldDef );

    if( it->isNull() || (type!=OFTString && it->toString().isEmpty()) )
    {
      OGR_F_UnsetField(feature, targetAttributeId);
    }
    else
    {
      switch( type )
      {
        case OFTInteger:
          OGR_F_SetFieldInteger(feature,targetAttributeId,it->toInt());
          break;

        case OFTReal:
          OGR_F_SetFieldDouble(feature,targetAttributeId,it->toDouble());
          break;

        case OFTString:
          QgsDebugMsg( QString("Writing string attribute %1 with %2, encoding %3")
              .arg( targetAttributeId )
              .arg( it->toString() )
              .arg( mEncoding->name().data() ) );
          OGR_F_SetFieldString(feature,targetAttributeId,mEncoding->fromUnicode(it->toString()).constData());
          break;

        default:
          QgsLogger::warning("QgsOgrProvider::addFeature, no type found");
          break;
      }
    }
  }

  if( OGR_L_CreateFeature(ogrLayer,feature) != OGRERR_NONE)
  {
    QgsLogger::warning("Writing of the feature failed");
    returnValue = false;
  }
  ++numberFeatures;
  OGR_F_Destroy( feature );
  return returnValue;
}


bool QgsOgrProvider::addFeatures(QgsFeatureList & flist)
{
  bool returnvalue=true;
  for(QgsFeatureList::iterator it = flist.begin(); it != flist.end(); ++it)
  {
    if(!addFeature(*it))
    {
      returnvalue=false;
    }
  }

  // flush features
  OGR_L_SyncToDisk(ogrLayer);
  numberFeatures = OGR_L_GetFeatureCount(ogrLayer,TRUE); //new feature count
  return returnvalue;
}

bool QgsOgrProvider::addAttributes(const QgsNewAttributesMap & attributes)
{
  bool returnvalue=true;

  for(QgsNewAttributesMap::const_iterator iter = attributes.begin(); iter != attributes.end(); ++iter)
  {
    OGRFieldDefnH fielddefn = 
      OGR_Fld_Create(mEncoding->fromUnicode(iter.key()).data(),OFTInteger);

    if(*iter=="OFTInteger")
    {
      OGR_Fld_SetType( fielddefn, OFTInteger );
    }
    else if(*iter=="OFTReal")
    {
      OGR_Fld_SetType( fielddefn, OFTReal );
    }
    else if(*iter=="OFTString")
    {
      OGR_Fld_SetType( fielddefn, OFTString );
    }
    else
    {
      QgsLogger::warning("QgsOgrProvider::addAttributes, type not found");
      returnvalue=false;
      continue;
    }

    if( OGR_L_CreateField(ogrLayer,fielddefn,TRUE) != OGRERR_NONE)
    {
      QgsLogger::warning("QgsOgrProvider.cpp: writing of OFTInteger field failed");	
      returnvalue=false;
    }
    OGR_Fld_Destroy( fielddefn );
  }

  return returnvalue;
}

bool QgsOgrProvider::changeAttributeValues(const QgsChangedAttributesMap & attr_map)
{   
  for(QgsChangedAttributesMap::const_iterator it = attr_map.begin(); it != attr_map.end(); ++it)
  {
    long fid = (long) it.key();

    OGRFeatureH of = OGR_L_GetFeature( ogrLayer, fid );

    if ( !of )
    {
      QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Cannot read feature, cannot change attributes");
      return false;
    }

    const QgsAttributeMap& attr = it.value();

    for( QgsAttributeMap::const_iterator it2 = attr.begin(); it2 != attr.end(); ++it2 )
    {
      int f = it2.key();

      OGRFieldDefnH fd = OGR_F_GetFieldDefnRef( of, f );
      if (fd == NULL)
      {
        QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Field " + QString::number(f) + " doesn't exist");
        continue;
      }

      OGRFieldType type = OGR_Fld_GetType( fd );

      if( it2->isNull() || (type!=OFTString && it2->toString().isEmpty()) )
      {
        OGR_F_UnsetField( of, f);
      }
      else
      {
        
        switch ( type )
        {
          case OFTInteger:
            OGR_F_SetFieldInteger ( of, f, it2->toInt() );
            break;
          case OFTReal:
            OGR_F_SetFieldDouble ( of, f, it2->toDouble() );
            break;
          case OFTString:
            OGR_F_SetFieldString ( of, f, mEncoding->fromUnicode(it2->toString()).constData() );
            break;
          default:
            QgsLogger::warning("QgsOgrProvider::changeAttributeValues, Unknown field type, cannot change attribute");
            break;
        }
      }
    }

    OGR_L_SetFeature( ogrLayer, of );
  }

  OGR_L_SyncToDisk( ogrLayer );

  return true;
}

bool QgsOgrProvider::changeGeometryValues(QgsGeometryMap & geometry_map)
{
  OGRFeatureH theOGRFeature = 0;
  OGRGeometryH theNewGeometry = 0;

  for (QgsGeometryMap::iterator it = geometry_map.begin(); it != geometry_map.end(); ++it)
  {
    theOGRFeature = OGR_L_GetFeature(ogrLayer,it.key());
    if(!theOGRFeature)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, cannot find feature");
      continue;
    }

    //create an OGRGeometry
    if (OGR_G_CreateFromWkb(it->wkbBuffer(),
      OGR_L_GetSpatialRef(ogrLayer),
      &theNewGeometry,
      it->wkbSize()) != OGRERR_NONE)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, error while creating new OGRGeometry");
      OGR_G_DestroyGeometry( theNewGeometry );
      theNewGeometry = 0;
      continue;
    }

    if(!theNewGeometry)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, new geometry is NULL");
      continue;
    }

    //set the new geometry
    if(OGR_F_SetGeometryDirectly(theOGRFeature, theNewGeometry) != OGRERR_NONE)
    {
      QgsLogger::warning("QgsOgrProvider::changeGeometryValues, error while replacing geometry");
      OGR_G_DestroyGeometry( theNewGeometry );
      theNewGeometry = 0;
      continue;
    }

    OGR_L_SetFeature(ogrLayer,theOGRFeature);
    OGR_F_Destroy( theOGRFeature);
  }
  OGR_L_SyncToDisk(ogrLayer);
  return true;
}

bool QgsOgrProvider::createSpatialIndex()
{
  QString filename=dataSourceUri().section('/',-1,-1);//find out the filename from the uri
  QString layername=filename.section('.',0,0);
  QString sql="CREATE SPATIAL INDEX ON "+layername;
  OGR_DS_ExecuteSQL (ogrDataSource, sql.toAscii(), OGR_L_GetSpatialFilter(ogrLayer),"");
  //find out, if the .qix file is there
  QString indexname = dataSourceUri();
  indexname.truncate(dataSourceUri().length()-filename.length());
  indexname=indexname+layername+".qix";
  QFile indexfile(indexname);
  if(indexfile.exists())
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool QgsOgrProvider::deleteFeatures(const QgsFeatureIds & id)
{
  bool returnvalue=true;
  for (QgsFeatureIds::const_iterator it = id.begin(); it != id.end(); ++it)
  {
    if(!deleteFeature(*it))
    {
      returnvalue=false;
    }
  }

  OGR_L_SyncToDisk(ogrLayer);
  QString filename=dataSourceUri().section('/',-1,-1);//find out the filename from the uri
  QString layername=filename.section('.',0,0);
  QString sql="REPACK " + layername;
  OGR_DS_ExecuteSQL(ogrDataSource,sql.toLocal8Bit().data(), NULL, NULL);
  numberFeatures = OGR_L_GetFeatureCount(ogrLayer,TRUE); //new feature count
  return returnvalue;
}

bool QgsOgrProvider::deleteFeature(int id)
{
  OGRErr res = OGR_L_DeleteFeature(ogrLayer,id);
  return (res == OGRERR_NONE);
}

int QgsOgrProvider::capabilities() const
{
  int ability = NoCapabilities;

  // collect abilities reported by OGR
  if (ogrLayer)
  {
    // Whilst the OGR documentation (e.g. at
    // http://www.gdal.org/ogr/classOGRLayer.html#a17) states "The capability
    // codes that can be tested are represented as strings, but #defined
    // constants exists to ensure correct spelling", we always use strings
    // here.  This is because older versions of OGR don't always have all
    // the #defines we want to test for here.

    if (OGR_L_TestCapability(ogrLayer,"RandomRead"))
      // TRUE if the GetFeature() method works *efficiently* for this layer.
      // TODO: Perhaps influence if QGIS caches into memory 
      //       (vs read from disk every time) based on this setting.
    {
      ability |= QgsVectorDataProvider::RandomSelectGeometryAtId;
    }
    else
    {
      ability |= QgsVectorDataProvider::SequentialSelectGeometryAtId;
    }
    ability |= QgsVectorDataProvider::SelectGeometryAtId;

    if (OGR_L_TestCapability(ogrLayer,"SequentialWrite"))
      // TRUE if the CreateFeature() method works for this layer.
    {
      ability |= QgsVectorDataProvider::AddFeatures;
    }

    if (OGR_L_TestCapability(ogrLayer,"DeleteFeature"))
      // TRUE if this layer can delete its features
    {
      ability |= DeleteFeatures;
    }

    if (OGR_L_TestCapability(ogrLayer,"RandomWrite"))
      // TRUE if the SetFeature() method is operational on this layer.
    {
      // TODO According to http://shapelib.maptools.org/ (Shapefile C Library V1.2)
      // TODO "You can't modify the vertices of existing structures".
      // TODO Need to work out versions of shapelib vs versions of GDAL/OGR
      // TODO And test appropriately.

      ability |= ChangeAttributeValues;
      ability |= QgsVectorDataProvider::ChangeGeometries;
    }

    if (OGR_L_TestCapability(ogrLayer,"FastSpatialFilter"))
      // TRUE if this layer implements spatial filtering efficiently.
      // Layers that effectively read all features, and test them with the 
      // OGRFeature intersection methods should return FALSE.
      // This can be used as a clue by the application whether it should build
      // and maintain it's own spatial index for features in this layer.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should build and maintain it's own spatial index for features in this layer.
    }

    if (OGR_L_TestCapability(ogrLayer,"FastFeatureCount"))
      // TRUE if this layer can return a feature count
      // (via OGRLayer::GetFeatureCount()) efficiently ... ie. without counting
      // the features. In some cases this will return TRUE until a spatial
      // filter is installed after which it will return FALSE.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to count features.
    }

    if (OGR_L_TestCapability(ogrLayer,"FastGetExtent"))
      // TRUE if this layer can return its data extent 
      // (via OGRLayer::GetExtent()) efficiently ... ie. without scanning
      // all the features. In some cases this will return TRUE until a
      // spatial filter is installed after which it will return FALSE.
    {
      // TODO: Perhaps use as a clue by QGIS whether it should spawn a thread to calculate extent.
    }

    if (OGR_L_TestCapability(ogrLayer,"FastSetNextByIndex"))
      // TRUE if this layer can perform the SetNextByIndex() call efficiently.
    {
      // No use required for this QGIS release.
    }

    if (1)
    {
      // Ideally this should test for Shapefile type and GDAL >= 1.2.6
      // In reality, createSpatialIndex() looks after itself.
      ability |= QgsVectorDataProvider::CreateSpatialIndex;
    }

    // OGR doesn't handle shapefiles without attributes, ie. missing DBFs well, fixes #803
    if( ogrDriverName.startsWith("ESRI") && mAttributeFields.size()==0 )
    {
      QgsDebugMsg("OGR doesn't handle shapefile without attributes well, ie. missing DBFs");
      ability &= ~(AddFeatures|DeleteFeatures|ChangeAttributeValues|AddAttributes|DeleteAttributes);
    }
  }

  return ability;

  /*
  return (QgsVectorDataProvider::AddFeatures
  | QgsVectorDataProvider::ChangeAttributeValues
  | QgsVectorDataProvider::CreateSpatialIndex);
  */
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
static QString createFileFilter_(QString const &longName, QString const &glob)
{
  return "[OGR] " + 
    longName + " (" + glob.lower() + " " + glob.upper() + ");;";
} // createFileFilter_



QGISEXTERN QString fileVectorFilters()
{
  static QString myFileFilters;

  // if we've already built the supported vector string, just return what
  // we've already built
  if ( ! ( myFileFilters.isEmpty() || myFileFilters.isNull() ) )
  {
    return myFileFilters;
  }

  // register ogr plugins
  OGRRegisterAll();

  // first get the GDAL driver manager

  OGRSFDriverH driver;          // current driver

  QString driverName;           // current driver name

  // Grind through all the drivers and their respective metadata.
  // We'll add a file filter for those drivers that have a file
  // extension defined for them; the others, welll, even though
  // theoreticaly we can open those files because there exists a
  // driver for them, the user will have to use the "All Files" to
  // open datasets with no explicitly defined file name extension.
  QgsDebugMsg( QString("Driver count: %1").arg( OGRGetDriverCount() ) );

  for (int i = 0; i < OGRGetDriverCount(); ++i)
  {
    driver = OGRGetDriver(i);

    Q_CHECK_PTR(driver);

    if (!driver)
    {
      QgsLogger::warning("unable to get driver " + QString::number(i));
      continue;
    }

    driverName = OGR_Dr_GetName(driver);


    if (driverName.startsWith("ESRI"))
    {
      myFileFilters += createFileFilter_("ESRI Shapefiles", "*.shp");
    }
    else if (driverName.startsWith("UK"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("SDTS"))
    {
      myFileFilters += createFileFilter_( "Spatial Data Transfer Standard",
        "*catd.ddf" );
    }
    else if (driverName.startsWith("TIGER"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("S57"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("MapInfo"))
    {
      myFileFilters += createFileFilter_("MapInfo", "*.mif *.tab");
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("DGN"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("VRT"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("AVCBin"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("REC"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("Memory"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("Jis"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("GML"))
    {
      // XXX not yet supported; post 0.1 release task
      myFileFilters += createFileFilter_( "Geography Markup Language",
        "*.gml" );
    }
    else if (driverName.startsWith("CSV"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("PostgreSQL"))
    {
      // XXX needs file filter extension
    }
    else if (driverName.startsWith("GRASS")) 
    { 
      // XXX needs file filter extension 
    } 
    else if (driverName.startsWith("KML")) 
    { 
      // XXX needs file filter extension 
    } 
    else if (driverName.startsWith("Interlis 1")) 
    { 
      // XXX needs file filter extension 
    } 
    else if (driverName.startsWith("Interlis 2")) 
    { 
      // XXX needs file filter extension 
    } 
    else if (driverName.startsWith("SQLite")) 
    { 
      // XXX needs file filter extension 
    } 
    else if (driverName.startsWith("MySQL")) 
    { 
      // XXX needs file filter extension 
    } 
    else
    {
      // NOP, we don't know anything about the current driver
      // with regards to a proper file filter string
      QgsLogger::debug("fileVectorFilters, unknown driver: " + driverName);
    }

  }                           // each loaded GDAL driver

  // can't forget the default case

  myFileFilters += "All files (*.*)";

  return myFileFilters;

} // fileVectorFilters() const


QString QgsOgrProvider::fileVectorFilters() const
{
  return fileVectorFilters();
} // QgsOgrProvider::fileVectorFilters() const


/**
 * Class factory to return a pointer to a newly created 
 * QgsOgrProvider object
 */
QGISEXTERN QgsOgrProvider * classFactory(const QString *uri)
{
  return new QgsOgrProvider(*uri);
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
QGISEXTERN bool createEmptyDataSource(const QString& uri,
                                      const QString& format,
                                      const QString& encoding,
                                      QGis::WKBTYPE vectortype,
                                      const std::list<std::pair<QString, QString> >& attributes)
{
  OGRSFDriverH driver;
  OGRRegisterAll();
  driver = OGRGetDriverByName(format.toAscii());
  if(driver == NULL)
  {
    return false;
  }

  OGRDataSourceH dataSource;
  dataSource = OGR_Dr_CreateDataSource(driver,QFile::encodeName(uri).constData(), NULL);
  if(dataSource == NULL)
  {
    return false;
  }

  //consider spatial reference system
  OGRSpatialReferenceH reference = NULL;
  QgsSpatialRefSys mySpatialRefSys;
  mySpatialRefSys.validate();
  QString myWKT = mySpatialRefSys.toWkt();

  if( !myWKT.isNull()  &&  myWKT.length() != 0 )
  {
    reference = OSRNewSpatialReference(myWKT.toLocal8Bit().data());
  }

  // Map the qgis geometry type to the OGR geometry type
  OGRwkbGeometryType OGRvectortype = wkbUnknown;
  switch (vectortype)
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
      QgsLogger::debug("Unknown vector type of: ", (int)(vectortype), 1, 
        __FILE__, __FUNCTION__, __LINE__);
      return false;
      break;
    }
  }

  OGRLayerH layer;	
  layer = OGR_DS_CreateLayer(dataSource,QFile::encodeName(QFileInfo(uri).baseName()).constData(), reference, OGRvectortype, NULL);
  if(layer == NULL)
  {
    return false;
  }

  //create the attribute fields

  QTextCodec* codec=QTextCodec::codecForName(encoding.toLocal8Bit().data());

  for(std::list<std::pair<QString, QString> >::const_iterator it= attributes.begin(); it != attributes.end(); ++it)
  {
    if(it->second == "Real")
    {
      OGRFieldDefnH field = OGR_Fld_Create(codec->fromUnicode(it->first).data(), OFTReal);
      OGR_Fld_SetPrecision(field,3);
      OGR_Fld_SetWidth(field,32);
      if( OGR_L_CreateField(layer,field,TRUE) != OGRERR_NONE)
      {
        QgsLogger::warning("creation of OFTReal field failed");
      }
    }
    else if(it->second == "Integer")
    {
      OGRFieldDefnH field = OGR_Fld_Create(codec->fromUnicode(it->first).data(), OFTInteger);
      OGR_Fld_SetWidth(field,10); // limit to 10.  otherwise OGR sets it to 11 and recognizes as OFTDouble later
      if(OGR_L_CreateField(layer,field,TRUE) != OGRERR_NONE)
      {
        QgsLogger::warning("creation of OFTInteger field failed");
      }
    }
    else if(it->second == "String")
    {
      OGRFieldDefnH field = OGR_Fld_Create(codec->fromUnicode(it->first).data(), OFTString);
      if(OGR_L_CreateField(layer,field,TRUE) != OGRERR_NONE)
      {
        QgsLogger::warning("creation of OFTString field failed");
      }
    }
  }

  OGR_DS_Destroy(dataSource);

  QgsDebugMsg( QString("GDAL Version number %1").arg( GDAL_VERSION_NUM ) );
#if GDAL_VERSION_NUM >= 1310
  if(reference)
  {
    OSRRelease( reference );
  }
#endif //GDAL_VERSION_NUM
  return true;
}

QgsSpatialRefSys QgsOgrProvider::getSRS()
{
  QgsDebugMsg("QgsOgrProvider::getSRS()");

  QgsSpatialRefSys srs;

  OGRSpatialReferenceH mySpatialRefSys = OGR_L_GetSpatialRef(ogrLayer);
  if (mySpatialRefSys == NULL)
  {
    QgsDebugMsg("no spatial reference found"); 
  }
  else
  {
    // get the proj4 text
    char * ppszProj4;
    OSRExportToProj4(mySpatialRefSys, &ppszProj4 );
    QgsDebugMsg(ppszProj4); 
    char    *pszWKT = NULL;
    OSRExportToWkt(mySpatialRefSys, &pszWKT );
    QString myWKTString = QString(pszWKT);
    OGRFree(pszWKT);  

    // create SRS from WKT
    srs.createFromWkt( myWKTString );
  }

  return srs;
}

void QgsOgrProvider::getUniqueValues(int index, QStringList &uniqueValues)
{
  QgsField fld = mAttributeFields[index];
  QFileInfo fi( dataSourceUri() );
  if( !fi.exists() )
    return;
  
  QString sql = QString("SELECT DISTINCT %1 FROM %2 ORDER BY %1").arg( fld.name() ).arg( fi.baseName() );

  uniqueValues.clear();

  OGRLayerH l = OGR_DS_ExecuteSQL(ogrDataSource, sql.toAscii(), NULL, "SQL");
  if(l==0)
    return;

  OGRFeatureH f;
  while( f=OGR_L_GetNextFeature(l) )
  {
    uniqueValues.append( mEncoding->toUnicode(OGR_F_GetFieldAsString(f, 0)) );
    OGR_F_Destroy(f);
  }

  OGR_DS_ReleaseResultSet(ogrDataSource, l);
}



QVariant QgsOgrProvider::minValue(int index)
{
  QgsField fld = mAttributeFields[index];
  QFileInfo fi( dataSourceUri() );
  if( !fi.exists() )
    return QVariant();
  
  QString sql = QString("SELECT MIN(%1) FROM %2").arg( fld.name() ).arg( fi.baseName() );

  OGRLayerH l = OGR_DS_ExecuteSQL(ogrDataSource, sql.toAscii(), NULL, "SQL");

  if(l==0)
    return QVariant();

  OGRFeatureH f = OGR_L_GetNextFeature(l);
  if(f==0)
  {
    OGR_DS_ReleaseResultSet(ogrDataSource, l);
    return QVariant();
  }

  QString str = mEncoding->toUnicode( OGR_F_GetFieldAsString(f,0) );
  OGR_F_Destroy(f);

  QVariant value;
 
  switch (fld.type())
  {
    case QVariant::String: value = QVariant(str); break;
    case QVariant::Int: value = QVariant(str.toInt()); break;
    case QVariant::Double: value = QVariant(str.toDouble()); break;
    //case QVariant::DateTime: value = QVariant(QDateTime::fromString(str)); break;
    default: assert(NULL && "unsupported field type");
  }
  
  OGR_DS_ReleaseResultSet(ogrDataSource, l);

  return value;
}

QVariant QgsOgrProvider::maxValue(int index)
{
  QgsField fld = mAttributeFields[index];
  QFileInfo fi( dataSourceUri() );
  if( !fi.exists() )
    return QVariant();
  
  QString sql = QString("SELECT MAX(%1) FROM %2").arg( fld.name() ).arg( fi.baseName() );

  OGRLayerH l = OGR_DS_ExecuteSQL(ogrDataSource, sql.toAscii(), NULL, "SQL");
  if(l==0)
    return QVariant();

  OGRFeatureH f = OGR_L_GetNextFeature(l);
  if(f==0)
  {
    OGR_DS_ReleaseResultSet(ogrDataSource, l);
    return QVariant();
  }

  QString str = mEncoding->toUnicode( OGR_F_GetFieldAsString(f,0) );
  OGR_F_Destroy(f);

  QVariant value;
 
  switch (fld.type())
  {
    case QVariant::String: value = QVariant(str); break;
    case QVariant::Int: value = QVariant(str.toInt()); break;
    case QVariant::Double: value = QVariant(str.toDouble()); break;
    //case QVariant::DateTime: value = QVariant(QDateTime::fromString(str)); break;
    default: assert(NULL && "unsupported field type");
  }
  
  OGR_DS_ReleaseResultSet(ogrDataSource, l);

  return value;
}
