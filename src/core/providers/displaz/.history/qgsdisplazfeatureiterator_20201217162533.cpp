/***************************************************************************
    QgsDisplazfeatureiterator.cpp
    ---------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsdisplazfeatureiterator.h"

#include "qgsdisplazprovider.h"
#include "qgssqliteexpressioncompiler.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsexception.h"
#include "qgswkbtypes.h"
#include "qgsmessagelog.h"
#include <QTextCodec>
#include <QFile>

// using from provider:
// - setRelevantFields(), mRelevantFieldsForNextFeature
// - ogrLayer
// - mFetchFeaturesWithoutGeom
// - mAttributeFields
// - mEncoding


QgsDisplazFeatureIterator::QgsDisplazFeatureIterator( QgsDisplazFeatureSource *source, bool ownSource, const QgsFeatureRequest &request )
  : QgsAbstractFeatureIteratorFromSource<QgsDisplazFeatureSource>( source, ownSource, request )
  , mFirstFieldIsFid(source->mFirstFieldIsFid)
  , mFieldsWithoutFid(source->mFieldsWithoutFid)
{
///https://blog.csdn.net/tszhangjunqiao/article/details/38518483
 	
  for ( const auto &id :  mRequest.filterFids() )
  {
    mFilterFids.insert( id );
  }
  mFilterFidsIt = mFilterFids.begin();

  mRequest.setTimeout( -1 );
  m_source = source;
  //mRequest.setFlags(QgsFeatureRequest::ExactIntersect);
  if ( mRequest.destinationCrs().isValid() && mRequest.destinationCrs() != mSource->mCrs )
  {
    mTransform = QgsCoordinateTransform( mSource->mCrs, mRequest.destinationCrs(), mRequest.transformContext() );
  }
  try
  {
    mFilterRect = filterRectToSourceCrs( mTransform );
  }
  catch ( QgsCsException & )
  {
    // can't reproject mFilterRect
    close();
    return;
  }
  mFetchGeometry = (!mFilterRect.isNull()) ||!(mRequest.flags() & QgsFeatureRequest::NoGeometry);
  QgsAttributeList attrs = (mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes) ? mRequest.subsetOfAttributes() : mSource->mFields.allAttributesList();
 
  // ensure that all attributes required for expression filter are being fetched
  if (mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && request.filterType() == QgsFeatureRequest::FilterExpression)
  {
	  //ensure that all fields required for filter expressions are prepared
	  QSet<int> attributeIndexes = request.filterExpression()->referencedAttributeIndexes(mSource->mFields);
	  attributeIndexes += qgis::listToSet(attrs);
	  attrs = qgis::setToList(attributeIndexes);
	  mRequest.setSubsetOfAttributes(attrs);
  }
  // also need attributes required by order by
  if (mRequest.flags() & QgsFeatureRequest::SubsetOfAttributes && !mRequest.orderBy().isEmpty())
  {
	  QSet<int> attributeIndexes;
	  const auto usedAttributeIndices = mRequest.orderBy().usedAttributeIndices(mSource->mFields);
	  for (int attrIdx : usedAttributeIndices)
	  {
		  attributeIndexes << attrIdx;
	  }
	  attributeIndexes += qgis::listToSet(attrs);
	  attrs = qgis::setToList(attributeIndexes);
	  mRequest.setSubsetOfAttributes(attrs);
  }

  if (request.filterType() == QgsFeatureRequest::FilterExpression && request.filterExpression()->needsGeometry())
  {
	  mFetchGeometry = true;
  }
  
  // make sure we fetch just relevant fields
  // unless it's a VRT data source filtered by geometry as we don't know which
  // attributes make up the geometry and OGR won't fetch them to evaluate the
  // filter if we choose to ignore them (fixes #11223)
  if (mFilterRect.isNull())
  {
	 // QgsDisplazProviderUtils::setRelevantFields(mOgrLayer, mSource->mFields.count(), mFetchGeometry, attrs, mSource->mFirstFieldIsFid, mSource->mSubsetString);
	//  if (mOgrLayerOri && mOgrLayerOri != mOgrLayer)
	//	  QgsOgrProviderUtils::setRelevantFields(mOgrLayerOri, mSource->mFields.count(), mFetchGeometry, attrs, mSource->mFirstFieldIsFid, mSource->mSubsetString);
  }


  // pointcloud 锟斤拷 tree 锟斤拷锟斤拷 spatialindex

  if (!m_pointarrayfields)
  {
    m_pointarrayfields = mSource->mPointCloudData->GetFiled();
    for (size_t i = 0; i < m_pointarrayfields->size(); ++i)
    {

      const PointCloudGeomField& field = (*m_pointarrayfields)[i];

      if (field.name == "position")
      {
        Positionindex = i;
        m_P = (V3f*)field.as<float>();
        offset = mSource->mPointCloudData->offset();
      }
      if (field.name == "classification")
      {
        classificationindex = i;
      }
      if (field.name == "intensity")
      {
        intensityindex = i;
      }
      if (field.name == "returnNumber")
      {
        returnNumberindex = i;
      }
      if (field.name == "numberOfReturns")
      {
        numberofreturnindex = i;
      }
      if (field.name == "pointSourceId")
      {
        pointsoureindex = i;
      }
    }
  }

  Imath::Box3d box3d = mSource->mPointCloudData->boundingBox();
  float scale = box3d.size().x / mFilterRect.width();

  if (!mFilterRect.isNull() && mSource->mPointCloudData)
  {
	  mUsingFeatureIdList = false;
	  //mSource->mPointCloudData->
	  float xmin = mFilterRect.xMinimum();
	  float xmax = mFilterRect.xMaximum();
	  float ymin = mFilterRect.yMinimum();
	  float ymax = mFilterRect.yMaximum();
	  Imath::Box2f filterbox(Imath::V2f(xmin, ymin), Imath::V2f(xmax, ymax));
	  mdrawlist = mSource->mPointCloudData->getPointsOnlyInFilterRect(std::log10((std::pow(scale+0.1,0.9)))+0.1,false,filterbox); // 锟斤拷pointarray锟斤拷写 锟斤拷取锟斤拷锟斤拷锟侥猴拷锟斤拷   //intersects(mFilterRect);
	  //QgsDebugMsg("Features returned by spatial index: " + QString::number(mFeatureIdList.count()));
	 
	  if (mdrawlist.numVertices>40000)
	  {
		  decimal_step = std::floorl(mdrawlist.numVertices / 50000);
	  }
	  else
	  {
		  decimal_step = 1;
	  }
  }
  else
  {
	  mUsingFeatureIdList = true;
  }
  //start with first feature
  rewind();
}

QgsDisplazFeatureIterator::~QgsDisplazFeatureIterator()
{
  close();
}

bool QgsDisplazFeatureIterator::fetchFeature(QgsFeature &feature)
{
	feature.setValid(false);
	if (mClosed) // 锟斤拷 pointcloud 锟斤拷锟斤拷啥锟斤拷锟斤拷锟斤拷
	{
		return false; 
	}
	
	 // count++; // singpoint 模式下 count++;
	//count += mdrawlist.numVertices; // multipoint 模式下
	//mdrawlist.moreToDraw
	
	if (!mUsingFeatureIdList)
	{
		bool unfinish = checkFeatureSinglePoint(mSource->mPointCloudData, feature);
		//bool unfinish = checkFeature(mSource->mPointCloudData, feature);// multipoint的实现方式
		if (!unfinish)
		{
			if (mdrawlist.moreToDraw && count<10000)
			{
				float xmin = mFilterRect.xMinimum();
				float xmax = mFilterRect.xMaximum();
				float ymin = mFilterRect.yMinimum();
				float ymax = mFilterRect.yMaximum();
				Imath::Box2f filterbox(Imath::V2f(xmin, ymin), Imath::V2f(xmax, ymax));
				Imath::Box3d box3d = mSource->mPointCloudData->boundingBox();
				float scale = box3d.size().x / mFilterRect.width()/2;
				if (m_iterration<3)
				{
					mdrawlist = mSource->mPointCloudData->getPointsOnlyInFilterRect(std::log10((std::pow(scale + 0.1, 0.5))) + 1.0, true, filterbox);
					m_iterration++;
					if (mdrawlist.numVertices>50000)
					{
						decimal_step = std::floorl(mdrawlist.numVertices /50000);
					}
					else
					{
						decimal_step = 1;
					}
					return true;
				}
				else
				{
					close();
					return false;
				}

			}
			else
			{
				close();
				return false;
			}
		}
		else
		{
			return true;
		}
	}
	/*  另外一种实现方式
	if (mUsingFeatureIdList)
	{
		checkFeature(mSource->mPointCloudData, feature);
		if (mdrawlist.moreToDraw )
		{
			float xmin = mFilterRect.xMinimum();
			float xmax = mFilterRect.xMaximum();
			float ymin = mFilterRect.yMinimum();
			float ymax = mFilterRect.yMaximum();
			Imath::Box2f filterbox(Imath::V2f(xmin, ymin), Imath::V2f(xmax, ymax));
			Imath::Box3d box3d = mSource->mPointCloudData->boundingBox();
			float scale = box3d.size().x / mFilterRect.width();
			if (m_iterration<scale)
			{
				mdrawlist = mSource->mPointCloudData->getPointsOnlyInFilterRect(0.2, true, filterbox);
				m_iterration++;
				return true;
			}
			else
			{
				close();
				return false;
			}
			
		}
		else
		{
			close();
			return false;
		}
	}
	*/ 

}

bool QgsDisplazFeatureIterator::nextFeatureFilterExpression( QgsFeature &f )
{
  if ( !mExpressionCompiled )
    return QgsAbstractFeatureIterator::nextFeatureFilterExpression( f );
  else
    return fetchFeature( f );
}

bool QgsDisplazFeatureIterator::checkFeatureSinglePoint(std::shared_ptr<Geometry> geom, QgsFeature &feature)
{
	std::shared_ptr<PointArray> pointcloud = std::dynamic_pointer_cast<PointArray>(geom);
	size_t pointcount = pointcloud->pointCount();
	//decimal_step = 1;
	if (mdrawlist.index.size() >= decimal_step)
	{
		std::list<size_t>::iterator it = mdrawlist.index.begin();
		try
		{
			//QgsGeometry geo = QgsGeometry::fromPointXY(QgsPointXY(m_P[*it].x + offset.x, m_P[*it].y+offset.x));
			feature.setGeometry(QgsGeometry::fromPointXY(QgsPointXY(m_P[*it].x + offset.x, m_P[*it].y + offset.y)));
		}
		catch (const std::exception& e)
		{
			e.what();
			return false;
		}

		  /*for (size_t i = 0; i < m_pointarrayfields->size(); ++i)
		  {
			feature.setId(*it);
			feature.initAttributes(mSource->mFields.count());
			feature.setFields(mSource->mFields); // allow name-based attribute lookups
			feature.setAttribute(2, 10);
			feature.setAttribute(3, 4);
		  }
		  */
		  feature.setValid(true);
		  geometryToDestinationCrs(feature, mTransform);
		  count++;

		  //multipoint.append(QgsPoint(Vertex.x, Vertex.y));//; , Vertex.z));
		  for (size_t i = 0; i < decimal_step; i++)
		  {
			mdrawlist.index.erase(it);
			it = mdrawlist.index.begin();
			mdrawlist.numVertices--;
		  }

		 return true;
	}
	else
	{
		return false;
	}
	
	
}
/*
check multipoint 
*/
bool QgsDisplazFeatureIterator::checkFeature(std::shared_ptr<Geometry> geom, QgsFeature &feature )
{  
	//QgsMultiPoint

	QgsMultiPointXY  multipoint;
	std::shared_ptr<PointArray> pointcloud = std::dynamic_pointer_cast<PointArray>(geom);
	size_t pointcount = pointcloud->pointCount();
	
	decimal_step = 5;

	if (mdrawlist.index.size() >= 1)
	{
		std::list<size_t>::iterator it = mdrawlist.index.begin();

		while (mdrawlist.numVertices>0)
		{
			V3f Vertex = V3f(m_P[*it]) + pointcloud->offset();
			mdrawlist.index.erase(it);
			mdrawlist.numVertices--;
			it = mdrawlist.index.begin();
			multipoint.append(QgsPoint(Vertex.x, Vertex.y));//; , Vertex.z));
		}
		QgsGeometry geo = QgsGeometry::fromMultiPointXY(multipoint);
		feature.setGeometry(geo);
		feature.setValid(true);
		geometryToDestinationCrs(feature, mTransform);

		return true;
	}
	else
	{
		return false;
	}

	/*
	if (mdrawlist.index.size()>=1)
	{
		
		for (std::list<size_t>::iterator it = mdrawlist.index.begin(); it != mdrawlist.index.end(); ++it)
		{
			V3f Vertex = V3f(m_P[*it]) + pointcloud->offset();
			multipoint.append(QgsPoint(Vertex.x, Vertex.y));//; , Vertex.z));
		}
		QgsGeometry geo = QgsGeometry::fromMultiPointXY(multipoint);
		feature.setGeometry(geo);
		// we have a feature, end this cycle
		feature.setValid(true);
		geometryToDestinationCrs(feature, mTransform);

		mdrawlist.index.clear();
		mdrawlist.numVertices = 0;
		return true;
	 }
	else
	{
		return false;
	}*/


}


bool QgsDisplazFeatureIterator::rewind()
{
  return true;
}


bool QgsDisplazFeatureIterator::close()
{

  iteratorClosed();

  mClosed = true;
  return true;
}



bool QgsDisplazFeatureIterator::readFeature( ) const
{
 
  return true;
}


QgsDisplazFeatureSource::QgsDisplazFeatureSource( const QgsDisplazProvider *p )
  : mDataSource( p->dataSourceUri( true ) )
  , mLayerName( p->name() )
  , mLayerIndex( 0 )
  , mSubsetString("")
  , mEncoding(nullptr) // no copying - this is a borrowed pointer from Qt
  , mFields( p->fields() )
  , mFirstFieldIsFid(false)
  , mCrs( p->crs() )
  , mWkbType( p->wkbType() )
  , mPointCloudData( p->getGeom())
{

}

QgsDisplazFeatureSource::~QgsDisplazFeatureSource()
{

}

QgsFeatureIterator QgsDisplazFeatureSource::getFeatures( const QgsFeatureRequest &request )
{
  return QgsFeatureIterator( new QgsDisplazFeatureIterator( this, false, request ) );
}
