/***************************************************************************
    qgsmaptooleditblanksegments.cpp
    ---------------------
    begin                : 2025/08/19
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooleditblanksegments.h"

#include <line_p.h>

#include "qgscoordinatetransform.h"
#include "qgsfeatureid.h"
#include "qgsguiutils.h"
#include "qgslinesymbollayer.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsnullpainterdevice.h"
#include "qgsrubberband.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingsregistrycore.h"
#include "qgssinglesymbolrenderer.h"
#include "qgssnappingutils.h"
#include "qgsstyleentityvisitor.h"
#include "qgssymbol.h"
#include "qgsvectorlayer.h"

#include "moc_qgsmaptooleditblanksegments.cpp"

constexpr int TOLERANCE = 20;


/////////

///@cond PRIVATE

namespace
{

  /**
   * \brief Symbol layer visitor to look for a symbol layer given its id
   */
  class SymbolLayerFinder : public QgsStyleEntityVisitorInterface
  {
    public:
      /**
       * Constructor
       * \param symbolLayerId symbol layer id to search
       */
      SymbolLayerFinder( const QString &symbolLayerId )
        : mSymbolLayerId( symbolLayerId ) {}

      bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
      {
        if ( node.type != QgsStyleEntityVisitorInterface::NodeType::SymbolRule )
          return false;

        return true;
      }

      /**
       * Visit recursively \a symbol
       */
      bool visitSymbol( const QgsSymbol *symbol )
      {
        if ( symbol && !mSymbol )
          mSymbol = symbol;

        for ( int idx = 0; idx < symbol->symbolLayerCount(); idx++ )
        {
          const QgsSymbolLayer *sl = symbol->symbolLayer( idx );
          if ( sl->id() == mSymbolLayerId )
          {
            mSymbolLayer = dynamic_cast<const QgsTemplatedLineSymbolLayerBase *>( sl );
            mSymbolLayerIndex = mSymbolLayer ? idx : -1;
            return false;
          }

          // recurse over sub symbols
          if ( const QgsSymbol *subSymbol = const_cast<QgsSymbolLayer *>( sl )->subSymbol();
               subSymbol && !visitSymbol( subSymbol ) )
          {
            return false;
          }
        }

        return true;
      }

      bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &leaf ) override
      {
        if ( leaf.entity && leaf.entity->type() == QgsStyle::SymbolEntity )
        {
          auto symbolEntity = static_cast<const QgsStyleSymbolEntity *>( leaf.entity );
          if ( symbolEntity->symbol() )
            visitSymbol( symbolEntity->symbol() );
        }
        return true;
      }

      /**
       * Returns symbol, nullptr if symbol layer hasn't been found
       */
      const QgsSymbol *symbol() const { return mSymbol; }

      /**
       * Returns symbol layer, nullptr if symbol layer hasn't been found
       */
      const QgsTemplatedLineSymbolLayerBase *symbolLayer() const { return mSymbolLayer; }

      /**
       * Returns index position in symbol symbol layer list, -1 if symbol layer hasn't been found
       */
      int symbolLayerIndex() const { return mSymbolLayerIndex; }

    private:
      const QgsSymbol *mSymbol = nullptr;
      const QgsTemplatedLineSymbolLayerBase *mSymbolLayer = nullptr;
      int mSymbolLayerIndex = -1;
      const QString &mSymbolLayerId;
  };

  /**
   * Helper method to access point from \a points for given \a partIndex part, \a ringIndex ring, \a pointIndex
   * and dealing with error in case of out-of-bounds index
   */
  const QPointF &pointAt( const QList<QList<QPolygonF>> &points, int partIndex, int ringIndex, int pointIndex )
  {
    if ( partIndex < 0 || partIndex >= points.count() )
      throw std::invalid_argument( "Blank segments internal error : Invalid part index" );

    const QList<QPolygonF> &rings = points.at( partIndex );
    if ( ringIndex < 0 || ringIndex >= rings.count() )
      throw std::invalid_argument( "blank segments internal error : Invalid ring index" );

    const QPolygonF &pts = rings.at( ringIndex );
    if ( pointIndex < 0 || pointIndex >= pts.count() )
      throw std::invalid_argument( "blank segments internal error : Invalid point index" );

    return pts.at( pointIndex );
  }

  enum ProjectedPointStatus
  {
    OK,            // ok, point is on the segment
    LINE_EMPTY,    // line is empty, cannot project point
    NOT_ON_SEGMENT // point is on the line, but not on the segment
  };

  /**
 * Returns point projected on segment defined by lineStartPt and lineEndPt
 * distance is updated with the distance between the point and the returned projected point
 */
  QPointF projectedPoint( const QPointF &lineStartPt, const QPointF &lineEndPt, const QPointF &point, double &distance, ProjectedPointStatus &status )
  {
    status = ProjectedPointStatus::OK;
    distance = -1;

    const double Ax = lineStartPt.x();
    const double Ay = lineStartPt.y();
    const double Bx = lineEndPt.x();
    const double By = lineEndPt.y();
    const double Cx = point.x();
    const double Cy = point.y();

    const double length = QgsGeometryUtilsBase::distance2D( Ax, Ay, Bx, By );
    if ( length == 0 )
    {
      status = ProjectedPointStatus::LINE_EMPTY;
      return QPointF();
    }

    const double r = ( ( Cx - Ax ) * ( Bx - Ax ) + ( Cy - Ay ) * ( By - Ay ) ) / std::pow( length, 2 );
    if ( r < 0 or r > 1 )
    {
      status = ProjectedPointStatus::NOT_ON_SEGMENT;
    }

    // projected point
    const double Px = Ax + r * ( Bx - Ax );
    const double Py = Ay + r * ( By - Ay );

    distance = QgsGeometryUtilsBase::distance2D( Cx, Cy, Px, Py );

    return QPointF( Px, Py );
  }

} //namespace
///@endcond


/**
 * Rubber band used to draw blank segments on edition
 */
class QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand : public QgsRubberBand
{
  public:
    /**
     * Constructor. Initialize blank segment rubber band points
     * \param partIndex geometry part index of the blank segment
     * \param ringIndex geometry ring index of the blank segment
     * \param startIndex rendered points index of the blank segment start point
     * \param endIndex rendered points index of the blank segment end point
     * \param startPt blank segment start point
     * \param endPt blank segment end point
     * \param canvas map canvas
     * \param points feature rendered points
     */
    BlankSegmentRubberBand( int partIndex, int ringIndex, int startIndex, int endIndex, QPointF startPt, QPointF endPt, QgsMapCanvas *canvas, const FeaturePoints &points );

    /**
     * \param canvas map canvas
     * \param points feature rendered points
     */
    BlankSegmentRubberBand( QgsMapCanvas *canvas, const FeaturePoints &points );

    /**
     * Initialize blank segment rubber band points
     * \param partIndex geometry part index of the blank segment
     * \param ringIndex geometry ring index of the blank segment
     * \param startIndex rendered points index of the blank segment start point
     * \param endIndex rendered points index of the blank segment end point
     * \param startPt blank segment start point
     * \param endPt blank segment end point
     */
    void setPoints( int partIndex, int ringIndex, int startIndex, int endIndex, QPointF startPt, QPointF endPt );

    /**
     * Copy \a blankSegment rubber band information into this one
     */
    void copyFrom( const BlankSegmentRubberBand &blankSegment );

    /**
     * Set highligh state according to \a highlighted
     */
    void setHighlighted( bool highlighted );

    /**
     * Returns start point
     */
    const QPointF &getStartPoint() const;

    /**
     * Returns end point
     */
    const QPointF &getEndPoint() const;

    /**
     * Returns start index
     */
    int getStartIndex() const;

    /**
     * Returns end index
     */
    int getEndIndex() const;

    /**
     * Returns part index
     */
    int getPartIndex() const;

    /**
     * Returns ring index
     */
    int getRingIndex() const;

    /**
     * Returns start and end distance from the first rendered point expressed in \a unit units.
     */
    QPair<double, double> getStartEndDistance( Qgis::RenderUnit unit ) const;

    /**
     * Returns blank segment points count
     */
    int pointsCount() const;

    /**
     * Returns blank segments point at \a index
     */
    const QPointF &pointAt( int index ) const;

  private:
    /**
     * Update rubber band displayed points according to blank segment points
     */
    void updatePoints();

    int mPartIndex = -1;
    int mRingIndex = -1;
    int mStartIndex = -1;
    int mEndIndex = -1;
    QPointF mStartPt;
    QPointF mEndPt;
    bool mNeedSwap = false;
    const FeaturePoints &mPoints; //! all feature rendered points
};

QgsMapToolEditBlankSegmentsBase::QgsMapToolEditBlankSegmentsBase( QgsMapCanvas *canvas, QgsVectorLayer *layer, QgsLineSymbolLayer *symbolLayer, int blankSegmentFieldIndex )
  : QgsMapTool( canvas )
  , mLayer( layer )
  , mSymbolLayerId( symbolLayer->id() )
  , mBlankSegmentsFieldIndex( blankSegmentFieldIndex )
  , mEditedBlankSegment( new BlankSegmentRubberBand( canvas, mPoints ) )
  , mStartRubberBand( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) )
  , mEndRubberBand( new QgsRubberBand( canvas, Qgis::GeometryType::Point ) )
{
  auto initRubberBand = []( QgsRubberBand *rb ) {
    rb->setWidth( QgsGuiUtils::scaleIconSize( 4 ) );
    rb->setColor( QgsSettingsRegistryCore::settingsDigitizingLineColor->value() );
    rb->setIcon( QgsRubberBand::IconType::ICON_BOX );
  };

  initRubberBand( mStartRubberBand );
  initRubberBand( mEndRubberBand );

  mEditedBlankSegment->setHighlighted( true );
}

QgsMapToolEditBlankSegmentsBase::~QgsMapToolEditBlankSegmentsBase() = default;

void QgsMapToolEditBlankSegmentsBase::activate()
{
  if ( !mLayer || !mLayer->renderer() )
    return;

  if ( !mSymbol || !mSymbolLayer )
  {
    // search and symbol and symbol layer
    SymbolLayerFinder finder( mSymbolLayerId );
    mLayer->renderer()->accept( &finder );
    if ( finder.symbol() && finder.symbolLayer() && finder.symbolLayerIndex() > -1 )
    {
      mSymbol.reset( finder.symbol()->clone() );
      if ( mSymbolLayer = createRenderedPointsSymbolLayer( finder.symbolLayer() ); mSymbolLayer )
      {
        // set our on symbol layer to later retrieve rendered points
        mSymbol->changeSymbolLayer( finder.symbolLayerIndex(), mSymbolLayer );
      }
      else
      {
        mSymbol.reset();
        QgsDebugError( "Fail to create fake templated line symbol layer" );
      }
    }
    else
    {
      QgsDebugError( "Fail to retrieve symbol and templated line symbol layer" );
    }
  }

  QgsMapTool::activate();
}

void QgsMapToolEditBlankSegmentsBase::canvasMoveEvent( QgsMapMouseEvent *e )
{
  if ( !mSymbol || mPoints.isEmpty() )
    return;

  const QPoint &pos = e->pos();

  if ( canvas()->extent() != mExtent )
  {
    loadFeaturePoints();

    // If edition has been started, we cancel them because they are no longer in the same extent reference
    switch ( mState )
    {
      case State::SELECT_FEATURE:
      case State::BLANK_SEGMENT_SELECTED:
      case State::FEATURE_SELECTED:
        break;

      case State::BLANK_SEGMENT_CREATION_STARTED:
        mState = State::FEATURE_SELECTED;
        setCurrentBlankSegment( -1 );
        break;

      case State::BLANK_SEGMENT_MODIFICATION_STARTED:
        mState = State::BLANK_SEGMENT_SELECTED;
        // force original blank segment
        setCurrentBlankSegment( mCurrentBlankSegmentIndex );
        break;
    }
  }

  switch ( mState )
  {
    case State::SELECT_FEATURE:
      return;

    case State::BLANK_SEGMENT_SELECTED:
      updateHoveredBlankSegment( pos );
      break;

    case State::FEATURE_SELECTED:
    {
      updateHoveredBlankSegment( pos );

      // display current start point to create a new blank segment
      mStartRubberBand->setVisible( mHoveredBlankSegmentIndex == -1 );
      if ( mHoveredBlankSegmentIndex == -1 )
      {
        const QgsMapToPixel &m2p = *( canvas()->getCoordinateTransform() );
        mStartRubberBand->reset( Qgis::GeometryType::Point );
        double distance;
        int partIndex = -1, ringIndex = -1, pointIndex = -1;
        const QPointF closestPoint = getClosestPoint( pos, distance, partIndex, ringIndex, pointIndex );

        // for now end point is the same as start one
        mStartRubberBand->addPoint( m2p.toMapCoordinates( closestPoint.x(), closestPoint.y() ) );
      }

      break;
    }

    case State::BLANK_SEGMENT_MODIFICATION_STARTED:
    case State::BLANK_SEGMENT_CREATION_STARTED:
    {
      double distance = -1;
      int partIndex = -1;
      int pointIndex = -1;
      int ringIndex = -1;
      QPointF P = getClosestPoint( pos, distance, partIndex, ringIndex, pointIndex );
      if ( distance > -1 && pointIndex > -1 )
      {
        mEditedBlankSegment->setPoints( partIndex, ringIndex, mEditedBlankSegment->getStartIndex(), pointIndex, mEditedBlankSegment->getStartPoint(), P );
        updateStartEndRubberBand();
      }
    }
    break;
  }
}

void QgsMapToolEditBlankSegmentsBase::canvasPressEvent( QgsMapMouseEvent *e )
{
  switch ( mState )
  {
    case State::SELECT_FEATURE:
    {
      //find the closest feature to the pressed position
      const QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Area );
      if ( !m.isValid() )
      {
        emit messageEmitted( tr( "No feature was detected at the clicked position. Please click closer to the feature or enhance the search tolerance under Settings->Options->Digitizing->Search radius for vertex edits" ), Qgis::MessageLevel::Critical );
        return;
      }

      mCurrentFeatureId = m.featureId();
      loadFeaturePoints();
      updateHoveredBlankSegment( e->pos() );

      mState = State::FEATURE_SELECTED;
      break;
    }
    case State::FEATURE_SELECTED:

      // new blank segment selected
      if ( mHoveredBlankSegmentIndex > -1 )
      {
        mState = State::BLANK_SEGMENT_SELECTED;
        setCurrentBlankSegment( mHoveredBlankSegmentIndex );
      }
      // init first point of new blank segment
      else
      {
        mState = State::BLANK_SEGMENT_CREATION_STARTED;
        double distance = -1;
        int partIndex = -1;
        int pointIndex = -1;
        int ringIndex = -1;
        QPointF P = getClosestPoint( e->pos(), distance, partIndex, ringIndex, pointIndex );
        if ( distance > -1 && pointIndex > -1 )
        {
          mEditedBlankSegment->setPoints( partIndex, ringIndex, pointIndex, pointIndex, P, P );
          updateStartEndRubberBand();
        }
      }

      break;

    case State::BLANK_SEGMENT_SELECTED:
    {
      Q_ASSERT( mCurrentBlankSegmentIndex > -1 && mCurrentBlankSegmentIndex < static_cast<int>( mBlankSegments.size() ) );
      const QObjectUniquePtr<BlankSegmentRubberBand> &currentBlankSegment = mBlankSegments.at( mCurrentBlankSegmentIndex );

      // selected blank segment has changed
      if ( mHoveredBlankSegmentIndex > -1 && mHoveredBlankSegmentIndex != mCurrentBlankSegmentIndex )
      {
        setCurrentBlankSegment( mHoveredBlankSegmentIndex );
      }
      else
      {
        const double distanceFromStart = ( currentBlankSegment->getStartPoint() - e->pos() ).manhattanLength();
        const double distanceFromEnd = ( currentBlankSegment->getEndPoint() - e->pos() ).manhattanLength();

        // user clicked on start or end point to move it
        if ( std::min( distanceFromStart, distanceFromEnd ) < TOLERANCE )
        {
          if ( distanceFromStart < distanceFromEnd )
          {
            // start point become end point because we always edit end point
            mEditedBlankSegment->setPoints( currentBlankSegment->getPartIndex(), currentBlankSegment->getRingIndex(), currentBlankSegment->getEndIndex(), currentBlankSegment->getStartIndex(), currentBlankSegment->getEndPoint(), currentBlankSegment->getStartPoint() );
          }

          mState = State::BLANK_SEGMENT_MODIFICATION_STARTED;
        }
      }
    }

    break;

    case State::BLANK_SEGMENT_MODIFICATION_STARTED:
    case State::BLANK_SEGMENT_CREATION_STARTED:

      // this is a new one
      if ( mCurrentBlankSegmentIndex < 0 )
      {
        mBlankSegments.emplace_back( new BlankSegmentRubberBand( canvas(), mPoints ) );
        mBlankSegments.back()->copyFrom( *mEditedBlankSegment );
        mState = State::FEATURE_SELECTED;
      }
      // modify an existing one
      else
      {
        QObjectUniquePtr<BlankSegmentRubberBand> &blankSegment = mBlankSegments.at( mCurrentBlankSegmentIndex );
        blankSegment->copyFrom( *mEditedBlankSegment );
        mState = State::BLANK_SEGMENT_SELECTED;
      }

      updateAttribute();
      break;
  }
}

void QgsMapToolEditBlankSegmentsBase::keyPressEvent( QKeyEvent *e )
{
  // !!! We need to ignore event instead of accept them if we want to consume them
  // see QgsMapCanvas::keyPressEvent

  switch ( mState )
  {
    case State::SELECT_FEATURE:
      return;

    case State::BLANK_SEGMENT_SELECTED:
      if ( e->matches( QKeySequence::Delete ) && mCurrentBlankSegmentIndex > -1 )
      {
        mState = State::FEATURE_SELECTED;
        int toRemoveIndex = mCurrentBlankSegmentIndex;
        setCurrentBlankSegment( -1 );
        mBlankSegments.erase( mBlankSegments.begin() + toRemoveIndex );
        updateAttribute();
        e->ignore();
      }
      else if ( e->matches( QKeySequence::Cancel ) )
      {
        mState = State::FEATURE_SELECTED;
        setCurrentBlankSegment( -1 );
        e->ignore();
      }

      break;

    case State::FEATURE_SELECTED:
      if ( e->matches( QKeySequence::Cancel ) )
      {
        mCurrentFeatureId = FID_NULL;
        mState = State::SELECT_FEATURE;
        loadFeaturePoints();
        e->ignore();
      }
      break;

    case State::BLANK_SEGMENT_CREATION_STARTED:
      if ( e->matches( QKeySequence::Cancel ) )
      {
        mState = State::FEATURE_SELECTED;
        setCurrentBlankSegment( -1 );
        e->ignore();
      }
      break;

    case State::BLANK_SEGMENT_MODIFICATION_STARTED:
      if ( e->matches( QKeySequence::Cancel ) )
      {
        mState = State::BLANK_SEGMENT_SELECTED;
        // force original blank segment
        setCurrentBlankSegment( mCurrentBlankSegmentIndex );
        e->ignore();
      }
      break;
  }
}

QPair<double, double> QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getStartEndDistance( Qgis::RenderUnit unit ) const
{
  double startDistance = 0;
  const int startIndex = mNeedSwap ? mEndIndex : mStartIndex;
  const QPointF startPt = mNeedSwap ? mEndPt : mStartPt;
  for ( int i = 1; i < startIndex; i++ )
  {
    startDistance += QgsGeometryUtilsBase::distance2D( ::pointAt( mPoints, mPartIndex, mRingIndex, i - 1 ), ::pointAt( mPoints, mPartIndex, mRingIndex, i ) );
  }

  startDistance += QgsGeometryUtilsBase::distance2D( ::pointAt( mPoints, mPartIndex, mRingIndex, startIndex - 1 ), startPt );

  double endDistance = startDistance;
  for ( int i = 1; i < pointsCount(); i++ )
  {
    endDistance += QgsGeometryUtilsBase::distance2D( pointAt( i ), pointAt( i - 1 ) );
  }

  QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( mMapCanvas->mapSettings() );

  startDistance = renderContext.convertFromPainterUnits( startDistance, unit );
  endDistance = renderContext.convertFromPainterUnits( endDistance, unit );

  return QPair<double, double>( startDistance, endDistance );
}

int QgsMapToolEditBlankSegmentsBase::getClosestBlankSegmentIndex( const QPointF &point, double &distance ) const
{
  // search for closest blankSegment
  distance = -1;
  int iBlankSegment = -1;
  for ( int i = 0; i < static_cast<int>( mBlankSegments.size() ); i++ )
  {
    const QObjectUniquePtr<BlankSegmentRubberBand> &blankSegment = mBlankSegments.at( i );
    for ( int iPoint = 1; iPoint < blankSegment->pointsCount(); iPoint++ )
    {
      double d = 0;
      ProjectedPointStatus status = ProjectedPointStatus::OK;

      try
      {
        projectedPoint( blankSegment->pointAt( iPoint - 1 ), blankSegment->pointAt( iPoint ), point, d, status );
      }
      catch ( std::invalid_argument e )
      {
        QgsDebugError( e.what() );
        continue;
      }

      switch ( status )
      {
        case ProjectedPointStatus::LINE_EMPTY:
          continue;

        case ProjectedPointStatus::NOT_ON_SEGMENT:
          d = std::min( ( blankSegment->getStartPoint() - point ).manhattanLength(), ( blankSegment->getEndPoint() - point ).manhattanLength() );
          break;

        case ProjectedPointStatus::OK:
          break;
      }

      if ( distance == -1 || d < distance )
      {
        distance = d;
        iBlankSegment = i;
      }
    }
  }

  return iBlankSegment;
}

QPointF QgsMapToolEditBlankSegmentsBase::getClosestPoint( const QPointF &point, double &distance, int &partIndex, int &ringIndex, int &pointIndex ) const
{
  distance = -1;
  QPointF currentPoint;

  // iterate through all points from parts and ring to get the closest one
  for ( int iPart = 0; iPart < mPoints.count(); iPart++ )
  {
    const QList<QPolygonF> &rings = mPoints.at( iPart );
    for ( int iRing = 0; iRing < rings.count(); iRing++ )
    {
      const QPolygonF &points = rings.at( iRing );
      for ( int i = 1; i < points.count(); i++ )
      {
        double d = 0;
        ProjectedPointStatus status = ProjectedPointStatus::OK;
        QPointF P = projectedPoint( points.at( i - 1 ), points.at( i ), point, d, status );
        switch ( status )
        {
          case ProjectedPointStatus::LINE_EMPTY:
          case ProjectedPointStatus::NOT_ON_SEGMENT:
            continue;

          case ProjectedPointStatus::OK:
            break;
        }

        if ( distance == -1 || d < distance )
        {
          distance = d;
          currentPoint = P;
          partIndex = iPart;
          ringIndex = iRing;
          pointIndex = i;
        }
      }
    }
  }
  return currentPoint;
}


void QgsMapToolEditBlankSegmentsBase::updateStartEndRubberBand()
{
  mStartRubberBand->reset( Qgis::GeometryType::Point );
  mEndRubberBand->reset( Qgis::GeometryType::Point );

  bool displayEndPoint = true;
  switch ( mState )
  {
    case State::SELECT_FEATURE:
    case State::BLANK_SEGMENT_CREATION_STARTED:
      return;

    case State::FEATURE_SELECTED:
      displayEndPoint = false;
      [[fallthrough]];

    case State::BLANK_SEGMENT_SELECTED:
    case State::BLANK_SEGMENT_MODIFICATION_STARTED:
      break;
  }

  const QgsMapToPixel &m2p = *( canvas()->getCoordinateTransform() );

  const QPointF &startPoint = mEditedBlankSegment->getStartPoint();
  mStartRubberBand->addPoint( m2p.toMapCoordinates( startPoint.x(), startPoint.y() ) );

  if ( displayEndPoint )
  {
    const QPointF &endPoint = mEditedBlankSegment->getEndPoint();
    mEndRubberBand->addPoint( m2p.toMapCoordinates( endPoint.x(), endPoint.y() ) );
  }
}

void QgsMapToolEditBlankSegmentsBase::updateHoveredBlankSegment( const QPoint &pos )
{
  double distance = -1;
  int iBlankSegment = getClosestBlankSegmentIndex( pos, distance );

  if ( mHoveredBlankSegmentIndex > -1
       && mHoveredBlankSegmentIndex < static_cast<int>( mBlankSegments.size() )
       && mHoveredBlankSegmentIndex != mCurrentBlankSegmentIndex )
  {
    mBlankSegments.at( mHoveredBlankSegmentIndex )->setHighlighted( false );
  }

  // blank segment is hovered
  if ( iBlankSegment > -1 && distance < TOLERANCE )
  {
    mHoveredBlankSegmentIndex = iBlankSegment;
    if ( mHoveredBlankSegmentIndex < static_cast<int>( mBlankSegments.size() ) )
    {
      mBlankSegments.at( mHoveredBlankSegmentIndex )->setHighlighted( true );
    }
  }
  // no blank segment hovered, display the first point to create a new blank segment
  else
  {
    mHoveredBlankSegmentIndex = -1;
  }
}

void QgsMapToolEditBlankSegmentsBase::setCurrentBlankSegment( int currentBlankSegmentIndex )
{
  // copy current blank segment so we can edit it later (and hide the original one)

  if ( mCurrentBlankSegmentIndex > -1
       && mCurrentBlankSegmentIndex < static_cast<int>( mBlankSegments.size() ) )
  {
    mBlankSegments.at( mCurrentBlankSegmentIndex )->setVisible( true );
    mBlankSegments.at( mCurrentBlankSegmentIndex )->setHighlighted( false );
  }

  mCurrentBlankSegmentIndex = currentBlankSegmentIndex;
  // NOLINTBEGIN(bugprone-branch-clone)
  if ( mCurrentBlankSegmentIndex > -1 && mCurrentBlankSegmentIndex < static_cast<int>( mBlankSegments.size() ) )
  {
    mBlankSegments.at( mCurrentBlankSegmentIndex )->setVisible( false );
    mEditedBlankSegment->copyFrom( *( mBlankSegments.at( mCurrentBlankSegmentIndex ).get() ) );
  }
  else
  {
    mEditedBlankSegment->setVisible( false );
  }
  // NOLINTEND(bugprone-branch-clone)

  updateStartEndRubberBand();
}

void QgsMapToolEditBlankSegmentsBase::updateAttribute()
{
  if ( !mSymbolLayer )
    return;

  QList<QList<QgsBlankSegmentUtils::BlankSegments>> blankSegments;
  for ( const QObjectUniquePtr<BlankSegmentRubberBand> &blankSegment : mBlankSegments )
  {
    try
    {
      QPair<double, double> startEndDistance = blankSegment->getStartEndDistance( mSymbolLayer->blankSegmentsUnit() );

      const int partIndex = blankSegment->getPartIndex();
      if ( partIndex >= blankSegments.count() )
        blankSegments.resize( partIndex + 1 );

      QList<QgsBlankSegmentUtils::BlankSegments> &rings = blankSegments[partIndex];
      const int ringIndex = blankSegment->getRingIndex();
      if ( ringIndex >= rings.count() )
        rings.resize( ringIndex + 1 );

      QgsBlankSegmentUtils::BlankSegments &segments = rings[ringIndex];
      segments << startEndDistance;
    }
    catch ( std::invalid_argument e )
    {
      QgsDebugError( e.what() );
      return;
    }
  }

  QStringList strParts;
  for ( QList<QgsBlankSegmentUtils::BlankSegments> &part : blankSegments )
  {
    QStringList strRings;
    for ( QgsBlankSegmentUtils::BlankSegments &ring : part )
    {
      std::sort( ring.begin(), ring.end() );
      QStringList strDistances;
      for ( const QPair<double, double> &distance : ring )
      {
        strDistances << ( QString::number( distance.first ) + " " + QString::number( distance.second ) );
      }

      strRings << "(" + strDistances.join( "," ) + ")";
    }

    strParts << "(" + strRings.join( "," ) + ")";
  }

  QString strNewBlankSegments = "(" + strParts.join( "," ) + ")";

  mLayer->beginEditCommand( tr( "Set blank segment list" ) );
  if ( mLayer->changeAttributeValue( mCurrentFeatureId, mBlankSegmentsFieldIndex, strNewBlankSegments ) )
  {
    mLayer->endEditCommand();
    mLayer->triggerRepaint();
  }
  else
  {
    mLayer->destroyEditCommand();
  }
}

void QgsMapToolEditBlankSegmentsBase::loadFeaturePoints()
{
  mPoints.clear();
  mExtent = canvas()->extent();
  mBlankSegments.clear();
  setCurrentBlankSegment( -1 );

  if ( FID_IS_NULL( mCurrentFeatureId ) || !mSymbolLayer )
    return;

  QgsFeature feature;
  feature = mLayer->getFeature( mCurrentFeatureId );
  QString currentBlankSegments = feature.attribute( mBlankSegmentsFieldIndex ).toString();


  // render feature to update mPoints
  QgsRenderContext context = QgsRenderContext::fromMapSettings( canvas()->mapSettings() );
  QgsCoordinateTransform transform( canvas()->mapSettings().layerTransform( mLayer ) );
  QgsNullPaintDevice nullPaintDevice;
  QPainter painter( &nullPaintDevice );
  context.setPainter( &painter );
  context.setCoordinateTransform( transform );
  mSymbol->startRender( context );
  mSymbol->renderFeature( feature, context );
  mSymbol->stopRender( context );

  if ( mPoints.isEmpty() )
    return;

  QString error;
  QList<QList<QgsBlankSegmentUtils::BlankSegments>> allBlankSegments = QgsBlankSegmentUtils::parseBlankSegments( currentBlankSegments, context, mSymbolLayer->blankSegmentsUnit(), error );
  if ( !error.isEmpty() )
  {
    emit messageEmitted( tr( "Error while parsing feature blank segments: %1" ).arg( error ), Qgis::MessageLevel::Critical );
    return;
  }

  // iterate through all points from parts and ring to get the closest one
  for ( int iPart = 0; iPart < mPoints.count(); iPart++ )
  {
    const QList<QPolygonF> &rings = mPoints.at( iPart );
    for ( int iRing = 0; iRing < rings.count(); iRing++ )
    {
      if ( iPart >= allBlankSegments.count() || iRing >= allBlankSegments.at( iPart ).count() )
        continue;

      const QgsBlankSegmentUtils::BlankSegments &blankSegments = allBlankSegments.at( iPart ).at( iRing );

      double currentLength = 0;
      int iPoint = 0;
      const QPolygonF &points = rings.at( iRing );
      for ( QPair<double, double> ba : blankSegments )
      {
        while ( iPoint < points.count() - 1 && currentLength < ba.first )
        {
          iPoint++;
          currentLength += QgsGeometryUtilsBase::distance2D( points.at( iPoint ), points.at( iPoint - 1 ) );
        }

        if ( iPoint == points.count() )
          break;

        int startIndex = iPoint;
        Line l( points.at( iPoint ), points.at( iPoint - 1 ) );
        QPointF startPt = points.at( iPoint ) + l.diffForInterval( currentLength - ba.first );

        while ( iPoint < points.count() - 1 && currentLength < ba.second )
        {
          iPoint++;
          currentLength += QgsGeometryUtilsBase::distance2D( points.at( iPoint ), points.at( iPoint - 1 ) );
        }

        if ( iPoint == points.count() )
          break;

        int endIndex = iPoint;
        Line l2( points.at( iPoint ), points.at( iPoint - 1 ) );
        QPointF endPt = points.at( iPoint ) + l2.diffForInterval( currentLength - ba.second );

        mBlankSegments.emplace_back( new BlankSegmentRubberBand( iPart, iRing, startIndex, endIndex, startPt, endPt, canvas(), mPoints ) );
      }
    }
  }
}

QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::BlankSegmentRubberBand( QgsMapCanvas *canvas, const FeaturePoints &points )
  : QgsRubberBand( canvas )
  , mPoints( points )
{
  setHighlighted( false );
  setColor( QgsSettingsRegistryCore::settingsDigitizingLineColor->value() );
}

QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::BlankSegmentRubberBand( int partIndex, int ringIndex, int startIndex, int endIndex, QPointF startPt, QPointF endPt, QgsMapCanvas *canvas, const FeaturePoints &points )
  : BlankSegmentRubberBand( canvas, points )
{
  setPoints( partIndex, ringIndex, startIndex, endIndex, startPt, endPt );
}

void QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::setPoints( int partIndex, int ringIndex, int startIndex, int endIndex, QPointF startPt, QPointF endPt )
{
  mPartIndex = partIndex;
  mRingIndex = ringIndex;
  mStartIndex = startIndex;
  mEndIndex = endIndex;
  mStartPt = startPt;
  mEndPt = endPt;

  mNeedSwap = false;
  if ( mStartIndex == mEndIndex )
  {
    try
    {
      if ( const QPointF &startIndexPoint = ::pointAt( mPoints, mPartIndex, mRingIndex, mStartIndex );
           QgsGeometryUtilsBase::distance2D( startPt, startIndexPoint ) < QgsGeometryUtilsBase::distance2D( endPt, startIndexPoint ) )
      {
        mNeedSwap = true;
      }
    }
    catch ( std::invalid_argument e )
    {
      QgsDebugError( e.what() );
    }
  }
  else if ( mStartIndex > mEndIndex )
  {
    mNeedSwap = true;
  }

  updatePoints();
}

void QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::updatePoints()
{
  const QgsMapToPixel &m2p = *( mMapCanvas->getCoordinateTransform() );

  reset();
  for ( int iPoint = 0; iPoint < pointsCount(); iPoint++ )
  {
    try
    {
      const QPointF &point = pointAt( iPoint );
      const QgsPointXY mapPoint = m2p.toMapCoordinates( point.x(), point.y() );
      addPoint( mapPoint, iPoint == pointsCount() - 1 ); // update only last one
    }
    catch ( std::invalid_argument e )
    {
      QgsDebugError( e.what() );
    }
  }
}


void QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::copyFrom( const BlankSegmentRubberBand &blankSegment )
{
  setPoints( blankSegment.mPartIndex, blankSegment.mRingIndex, blankSegment.getStartIndex(), blankSegment.getEndIndex(), blankSegment.getStartPoint(), blankSegment.getEndPoint() );
}

void QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::setHighlighted( bool highlighted )
{
  setWidth( QgsGuiUtils::scaleIconSize( highlighted ? 4 : 2 ) );
  update();
}


const QPointF &QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getStartPoint() const
{
  return mStartPt;
}

const QPointF &QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getEndPoint() const
{
  return mEndPt;
}

int QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getStartIndex() const
{
  return mStartIndex;
}

int QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getEndIndex() const
{
  return mEndIndex;
}

int QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getPartIndex() const
{
  return mPartIndex;
}

int QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::getRingIndex() const
{
  return mRingIndex;
}

int QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::pointsCount() const
{
  return std::abs( mEndIndex - mStartIndex ) + 2;
}

const QPointF &QgsMapToolEditBlankSegmentsBase::BlankSegmentRubberBand::pointAt( int index ) const
{
  return index == 0 ?
                    // first point
           ( mNeedSwap ? mEndPt : mStartPt )
                    // last point
                    : ( index == pointsCount() - 1 ? ( mNeedSwap ? mStartPt : mEndPt )
                                                   // point in between
                                                   : ::pointAt( mPoints, mPartIndex, mRingIndex, ( mNeedSwap ? mEndIndex : mStartIndex ) + index - 1 ) );
}
