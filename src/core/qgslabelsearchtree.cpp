#include "qgslabelsearchtree.h"
#include "labelposition.h"

bool searchCallback( QgsLabelPosition* pos, void* context )
{
  QList<QgsLabelPosition*>* list = static_cast< QList<QgsLabelPosition*>* >( context );
  list->push_back( pos );
  return true;
}

QgsLabelSearchTree::QgsLabelSearchTree()
{
}

QgsLabelSearchTree::~QgsLabelSearchTree()
{
  clear();
}

void QgsLabelSearchTree::label( const QgsPoint& p, QList<QgsLabelPosition*>& posList )
{
  double c_min[2]; c_min[0] = p.x() - 1; c_min[1] = p.y() - 1;
  double c_max[2]; c_max[0] = p.x() + 1; c_max[1] = p.y() + 1;

  mSearchResults.clear();
  mSpatialIndex.Search( c_min, c_max, searchCallback, &mSearchResults );
  posList = mSearchResults;
}

bool QgsLabelSearchTree::insertLabel( LabelPosition* labelPos, int featureId, const QString& layerName )
{
  if ( !labelPos )
  {
    return false;
  }

  double c_min[2];
  double c_max[2];
  labelPos->getBoundingBox( c_min, c_max );

  QVector<QgsPoint> cornerPoints;
  for ( int i = 0; i < 4; ++i )
  {
    cornerPoints.push_back( QgsPoint( labelPos->getX( i ), labelPos->getY( i ) ) );
  }
  QgsLabelPosition* newEntry = new QgsLabelPosition( featureId, labelPos->getAlpha(), cornerPoints, QgsRectangle( c_min[0], c_min[1], c_max[0], c_max[1] ),
      labelPos->getWidth(), labelPos->getHeight(), layerName, labelPos->getUpsideDown() );
  mSpatialIndex.Insert( c_min, c_max, newEntry );
  return true;
}

void QgsLabelSearchTree::clear()
{
  RTree<QgsLabelPosition*, double, 2, double>::Iterator indexIt;
  mSpatialIndex.GetFirst( indexIt );
  while ( !mSpatialIndex.IsNull( indexIt ) )
  {
    delete mSpatialIndex.GetAt( indexIt );
    mSpatialIndex.GetNext( indexIt );
  }
  mSpatialIndex.RemoveAll();
}
