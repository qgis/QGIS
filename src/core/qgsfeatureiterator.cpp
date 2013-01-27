#include "qgsfeatureiterator.h"


QgsAbstractFeatureIterator::QgsAbstractFeatureIterator( const QgsFeatureRequest& request )
    : mRequest( request ),
    mClosed( false ),
    refs( 0 )
{
}

QgsAbstractFeatureIterator::~QgsAbstractFeatureIterator()
{
}

void QgsAbstractFeatureIterator::ref()
{
  refs++;
}
void QgsAbstractFeatureIterator::deref()
{
  refs--;
  if ( !refs )
    delete this;
}

///////

QgsFeatureIterator& QgsFeatureIterator::operator=( const QgsFeatureIterator & other )
{
  if ( this != &other )
  {
    if ( mIter )
      mIter->deref();
    mIter = other.mIter;
    if ( mIter )
      mIter->ref();
  }
  return *this;
}
