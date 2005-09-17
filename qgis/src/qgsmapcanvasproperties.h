
#ifndef QGSMAPCANVASPROPERTIES_H
#define QGSMAPCANVASPROPERTIES_H

#ifndef QPAINTDEVICEMETRICS_H
#include <qpaintdevicemetrics.h>
#endif

#include "qgsgeometry.h"
#include "qgsmaptopixel.h"
#include "qgsvectorlayer.h"
#include <qgsscalecalculator.h>


/**
 
   Implementation struct for QgsMapCanvas
 
  @note
 
  Changed to class from struct out of desperation to find workaround for g++ bug.
 
 */
class QgsMapCanvas::CanvasProperties
{
  public:

    CanvasProperties( int width, int height )
  : mapWindow( 0x0 ),
    mapLegend( 0 ),
    coordXForm( 0x0 ),
    pmCanvas( 0x0 ),
    bgColor( Qt::white ),
    dragging( false ),
    capturing( false ),
    drawing( false ),
    frozen( false ),
    dirty( true ),
    scaleCalculator( 0x0 )
    {
      mapWindow = new QRect;
      coordXForm = new QgsMapToPixel;
      pmCanvas = new QPixmap(width, height);
      scaleCalculator = new QgsScaleCalculator;
    // set the initial extent - can't use a constructor since QgsRect
    // normalizes the rectangle upon construction
      fullExtent.setXmin(9999999999.0);
      fullExtent.setYmin(999999999.0);
      fullExtent.setXmax(-999999999.0);
      fullExtent.setYmax(-999999999.0);
    }

    CanvasProperties()
  : mapWindow( 0x0 ),
    mapLegend( 0 ),
    coordXForm( 0x0 ),
    pmCanvas( 0x0 ),
    bgColor( Qt::white ),
    dragging( false ),
    capturing( false ),
    drawing( false ),
    frozen( false ),
    dirty( true ),
    scaleCalculator( 0x0 )
    {
      mapWindow = new QRect;
      coordXForm = new QgsMapToPixel;
      pmCanvas = new QPixmap;
      scaleCalculator = new QgsScaleCalculator;
    }


    ~CanvasProperties()
    {
      delete coordXForm;
      delete pmCanvas;
      delete mapWindow;
      delete scaleCalculator;
    } // ~CanvasProperties


    void initMetrics(QPaintDeviceMetrics *pdm)
    {
    // set the logical dpi
      mDpi = pdm->logicalDpiX();
      scaleCalculator->setDpi(mDpi);

    // set default map units
      mMapUnits = QGis::METERS;
      scaleCalculator->setMapUnits(mMapUnits);
    }

    void setMapUnits(QGis::units u)
    {
      mMapUnits = u;
      scaleCalculator->setMapUnits(mMapUnits);
    }

    QGis::units mapUnits()
    {
      return mMapUnits;
    }

  //! map containing the layers by name
    std::map< QString, QgsMapLayer *> layers;

  //! map containing the acetate objects by key (name)
    std::map< QString, QgsAcetateObject *> acetateObjects;

  //! list containing the names of layers in zorder
    std::list< QString > zOrder;

  //! Full extent of the map canvas
    QgsRect fullExtent;

  //! Current extent
    QgsRect currentExtent;

  //! Previous view extent
    QgsRect previousExtent;

  //! Map window rectangle
  //std::auto_ptr<QRect> mapWindow;
    QRect * mapWindow;

  //! Pointer to the map legend
  //std::auto_ptr<QgsLegend> mapLegend;
    QgsLegend * mapLegend;

  /** Pointer to the coordinate transform object used to transform
    coordinates from real world to device coordinates
   */
  //std::auto_ptr<QgsMapToPixel> coordXForm;
    QgsMapToPixel * coordXForm;

  /** The output spatial reference system that was used most
    recently. Obtained from a layer on this canvas
   */
    QgsSpatialRefSys previousOutputSRS;

  /**
     * \brief Currently selected map tool.
               * @see QGis::MapTools enum for valid values
   */
                  int mapTool;

  //!Flag to indicate status of mouse button
              bool mouseButtonDown;

  //! Map units per pixel
              double m_mupp;

  //! Rubber band box for dynamic zoom
              QRect zoomBox;

  //! Beginning point of a rubber band
              QPoint rubberStartPoint;

  //! Is the beginning point of a rubber band valid?  (If not, this segment of the rubber band will not be drawn)
              bool rubberStartPointIsValid;

  //! Mid point of a rubber band
              QPoint rubberMidPoint;

  //! End point of a rubber band
              QPoint rubberStopPoint;

  //! Is the end point of a rubber band valid?  (If not, this segment of the rubber band will not be drawn)
              bool rubberStopPointIsValid;

  //! The snapped-to segment before this vertex number (identifying the vertex that is being moved)
              QgsGeometryVertexIndex snappedAtVertex;

  //! The snapped-to segment before this vertex number (identifying the segment that a new vertex is being added to)
              QgsGeometryVertexIndex snappedBeforeVertex;

  //! The snapped-to feature ID
              int snappedAtFeatureId;

  //! The snapped-to geometry
              QgsGeometry snappedAtGeometry;

  //! Pixmap used for restoring the canvas.
              /** @note using QGuardedPtr causes sefault for some reason -- XXX trying again */
  //QGuardedPtr<QPixmap> pmCanvas;
  //std::auto_ptr<QPixmap> pmCanvas;
              QPixmap * pmCanvas;

  //! Background color for the map canvas
              QColor bgColor;

  //! Flag to indicate a map canvas drag operation is taking place
              bool dragging;

  //! Flag to indicate a map canvas capture operation is taking place
              bool capturing;
  
  //! Vector containing the inital color for a layer
              std::vector < QColor > initialColor;

  //! Flag indicating a map refresh is in progress
              bool drawing;


  //! Flag indicating if the map canvas is frozen.
              bool frozen;

  /*! \brief Flag to track the state of the Map canvas.
              *
              * The canvas is
              * flagged as dirty by any operation that changes the state of
              * the layers or the view extent. If the canvas is not dirty, paint
              * events are handled by bit-blitting the stored canvas bitmap to
              * the canvas. This improves performance by not reading the data source
              * when no real change has occurred
   */
                 bool dirty;


  //! Value use to calculate the search radius when identifying features
  // TODO - Do we need this?
             double radiusValue;

  //std::auto_ptr<QgsScaleCalculator> scaleCalculator;
             QgsScaleCalculator * scaleCalculator;

  //! DPI of physical display
             int mDpi;

  //! Map units for the data on the canvas
             QGis::units mMapUnits;

  //! Map scale of the canvas at its current zool level
             double mScale;

  private:

  /** not copyable
   */
    CanvasProperties( CanvasProperties const & rhs )
    {
    // XXX maybe should be NOP just like operator=() to be consistent
      std::cerr << __FILE__ << ":" << __LINE__
          << " should not be here since CanvasProperties shouldn't be copyable\n";
    } // CanvasProperties copy ctor


  /** not copyable
   */
    CanvasProperties & operator=( CanvasProperties const & rhs )
    {
      if ( this == &rhs )
      {
        return *this;
      }

      std::cerr << __FILE__ << ":" << __LINE__
          << " should not be here since CanvasProperties shouldn't be copyable\n";

      return *this;
    } // CanvasProperties assignment operator

}
; // struct QgsMapCanvas::CanvasProperties

#endif
