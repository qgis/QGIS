#include "qgsmaptoolcircularstringcurvepoint.h"
#include "qgscircularstringv2.h"
#include "qgscompoundcurvev2.h"
#include "qgsgeometryrubberband.h"
#include "qgsmapcanvas.h"
#include "qgspointv2.h"
#include <QMouseEvent>

QgsMapToolCircularStringCurvePoint::QgsMapToolCircularStringCurvePoint( QgsMapToolCapture* parentTool,
    QgsMapCanvas* canvas, CaptureMode mode ): QgsMapToolAddCircularString( parentTool, canvas, mode )
{

}

QgsMapToolCircularStringCurvePoint::~QgsMapToolCircularStringCurvePoint()
{
}

void QgsMapToolCircularStringCurvePoint::canvasMapReleaseEvent( QgsMapMouseEvent* e )
{
  QgsPointV2 mapPoint( e->mapPoint().x(), e->mapPoint().y() );

  if ( e->button() == Qt::LeftButton )
  {
    if ( mPoints.size() < 1 ) //connection to vertex of previous line segment needed?
    {
      const QgsCompoundCurveV2* compoundCurve = mParentTool->captureCurve();
      if ( compoundCurve )
      {
        if ( compoundCurve->nCurves() > 0 )
        {
          const QgsCurveV2* curve = compoundCurve->curveAt( compoundCurve->nCurves() - 1 );
          if ( curve )
          {
            //mParentTool->captureCurve() is in layer coordinates, but we need map coordinates
            QgsPointV2 endPointLayerCoord = curve->endPoint();
            QgsPoint mapPoint = toMapCoordinates( mCanvas->currentLayer(), QgsPoint( endPointLayerCoord.x(), endPointLayerCoord.y() ) );
            mPoints.append( QgsPointV2( mapPoint.x(), mapPoint.y() ) );
          }
        }
      }
    }
    mPoints.append( mapPoint );
    if ( !mCenterPointRubberBand && mShowCenterPointRubberBand )
    {
      createCenterPointRubberBand();
    }

    if ( mPoints.size() > 1 )
    {
      if ( !mRubberBand )
      {
        mRubberBand = createGeometryRubberBand(( mCaptureMode == CapturePolygon ) ? QGis::Polygon : QGis::Line );
        mRubberBand->show();
      }

      QgsCircularStringV2* c = new QgsCircularStringV2();
      QList< QgsPointV2 > rubberBandPoints = mPoints;
      rubberBandPoints.append( mapPoint );
      c->setPoints( rubberBandPoints );
      mRubberBand->setGeometry( c );
    }
    if (( mPoints.size() ) % 2 == 1 )
    {
      removeCenterPointRubberBand();
    }
  }
  else if ( e->button() == Qt::RightButton )
  {
    deactivate();
    if ( mParentTool )
    {
      mParentTool->canvasReleaseEvent( e );
    }
  }
}

void QgsMapToolCircularStringCurvePoint::canvasMapMoveEvent( QgsMapMouseEvent* e )
{
  QgsPointV2 mapPoint( e->mapPoint().x(), e->mapPoint().y() );
  QgsVertexId idx; idx.part = 0; idx.ring = 0; idx.vertex = mPoints.size();
  if ( mRubberBand )
  {
    mRubberBand->moveVertex( idx, mapPoint );
    updateCenterPointRubberBand( mapPoint );
  }
}
