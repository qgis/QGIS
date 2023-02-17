/***************************************************************************
    qgsmaptoolshapeabstract.h  -  base class for map tools digitizing shapes
    ---------------------
    begin                : January 2022
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHAPEABSTRACT_H
#define QGSMAPTOOLSHAPEABSTRACT_H

// no bindings for now, not stable yet
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgsabstractgeometry.h"
#include "qgsmaptoolcapture.h"

#include <QString>
#include <QIcon>

class QgsMapMouseEvent;
class QgsVectorLayer;
class QgsGeometryRubberBand;
class QKeyEvent;


/**
 * \ingroup gui
 * \brief QgsMapToolShapeAbstract is a base class for shape map tools to be used by QgsMapToolCapture.
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsMapToolShapeAbstract
  : public QObject
{
    Q_OBJECT
  public:
    //! List of different shapes
    enum class ShapeCategory
    {
      Curve, //!< Curve
      Circle,//!< Circle
      Ellipse,//!< Ellipse
      Rectangle,//!< Rectangle
      RegularPolygon,//!< RegularPolygon (e.g pentagons or hexagons)
    };
    Q_ENUM( ShapeCategory )

    //! Constructor
    QgsMapToolShapeAbstract( const QString &id, QgsMapToolCapture *parentTool )
      : mId( id ), mParentTool( parentTool )
    {
      Q_ASSERT( !mId.isEmpty() );
      Q_ASSERT( parentTool );
    }

    virtual ~QgsMapToolShapeAbstract();

    //! Returns the id of the shape tool (equivalent to the one from the metadata)
    QString id() const {return mId;}

    /**
     * Called for a mouse release event
     * Must return TRUE if the digitization has ended and the geometry is correctly set
     */
    virtual bool cadCanvasReleaseEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) = 0;

    //! Called for a mouse move event
    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e, QgsMapToolCapture::CaptureMode mode ) = 0;

    /**
     * Filters a key press event
     * Ignores the event in default implementation
     */
    virtual void keyPressEvent( QKeyEvent *e );

    /**
     * Filters a key release event
     * Ignores the event in default implementation
     */
    virtual void keyReleaseEvent( QKeyEvent *e );

    //! Activates the map tool with the last captured map point
    virtual void activate( QgsMapToolCapture::CaptureMode mode, const QgsPoint &lastCapturedMapPoint ) {Q_UNUSED( mode ); Q_UNUSED( lastCapturedMapPoint )}

    //! Deactivates the map tool
    virtual void deactivate() {clean();}

    //! Called to clean the map tool (after canceling the operation or when the digitization has finished)
    virtual void clean();

    //! Called to undo last action (last point added)
    virtual void undo();

  private:
    QString mId;

  protected:
    QgsMapToolCapture *mParentTool = nullptr;

    //! points (in map coordinates)
    QgsPointSequence mPoints;

    QgsGeometryRubberBand *mTempRubberBand = nullptr;

};



#endif // QGSMAPTOOLSHAPEABSTRACT_H
