#include "qgswfsfeatureiterator.h"
#include "qgsspatialindex.h"
#include "qgswfsprovider.h"

QgsWFSFeatureIterator::QgsWFSFeatureIterator( QgsWFSProvider* provider, const QgsFeatureRequest& request ):
    QgsAbstractFeatureIterator( request ), mProvider( provider )
{
  //select ids
  //get iterator
  if ( !mProvider )
  {
    return;
  }

  switch ( request.filterType() )
  {
    case QgsFeatureRequest::FilterRect:
      if ( mProvider->mSpatialIndex )
      {
        mSelectedFeatures = mProvider->mSpatialIndex->intersects( request.filterRect() );
      }
      break;
    case QgsFeatureRequest::FilterFid:
      mSelectedFeatures.push_back( request.filterFid() );
      break;
    case QgsFeatureRequest::FilterNone:
      mSelectedFeatures = mProvider->mFeatures.keys();
    default: //QgsFeatureRequest::FilterNone
      mSelectedFeatures = mProvider->mFeatures.keys();
  }

  mFeatureIterator = mSelectedFeatures.constBegin();
}

QgsWFSFeatureIterator::~QgsWFSFeatureIterator()
{

}

bool QgsWFSFeatureIterator::nextFeature( QgsFeature& f )
{
  if ( !mProvider )
  {
    return false;
  }

  if ( mFeatureIterator == mSelectedFeatures.constEnd() )
  {
    return false;
  }

  QMap<QgsFeatureId, QgsFeature* >::iterator it = mProvider->mFeatures.find( *mFeatureIterator );
  if ( it == mProvider->mFeatures.end() )
  {
    return false;
  }
  QgsFeature* fet =  it.value();
  mProvider->copyFeature( fet, f, !( mRequest.flags() & QgsFeatureRequest::NoGeometry ), mRequest.subsetOfAttributes() );
  ++mFeatureIterator;
  return true;
}

bool QgsWFSFeatureIterator::rewind()
{
  return false;
}

bool QgsWFSFeatureIterator::close()
{
  return false;
}
