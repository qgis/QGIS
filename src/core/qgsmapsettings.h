#ifndef QGSMAPSETTINGS_H
#define QGSMAPSETTINGS_H

#include <QSize>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgsrectangle.h"
#include "qgsscalecalculator.h"

class QPainter;

class QgsCoordinateTransform;
class QgsScaleCalculator;
class QgsMapRendererJob;
class QgsMapLayer;


struct QgsMapSettings
{
  // TODO

  QgsRectangle extent() const;
  void setExtent(const QgsRectangle& rect);

  QSize outputSize() const;
  void setOutputSize(const QSize& size);

  double outputDpi() const;
  void setOutputDpi(double dpi);

  QStringList layers() const;
  void setLayers(const QStringList& layers);

  //! sets whether to use projections for this layer set
  void setProjectionsEnabled( bool enabled );
  //! returns true if projections are enabled for this layer set
  bool hasCrsTransformEnabled() const;

  void updateDerived(); // TODO: should be protected, called automatically

  bool hasValidSettings() const;
  QgsRectangle visibleExtent() const;
  double mapUnitsPerPixel() const;
  double scale() const;



  // TODO: utility functions


  /**
   * @brief transform bounding box from layer's CRS to output CRS
   * @see layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) if you want to transform a rectangle
   * @return a bounding box (aligned rectangle) containing the transformed extent
   */
  QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent );

  /**
   * @brief transform bounding box from output CRS to layer's CRS
   * @see mapToLayerCoordinates( QgsMapLayer* theLayer,QgsRectangle rect ) if you want to transform a rectangle
   * @return a bounding box (aligned rectangle) containing the transformed extent
   */
  QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent );

  /**
   * @brief transform point coordinates from layer's CRS to output CRS
   * @return the transformed point
   */
  QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point );

  /**
   * @brief transform rectangle from layer's CRS to output CRS
   * @see layerExtentToOutputExtent() if you want to transform a bounding box
   * @return the transformed rectangle
   */
  QgsRectangle layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );

  /**
   * @brief transform point coordinates from output CRS to layer's CRS
   * @return the transformed point
   */
  QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point );

  /**
   * @brief transform rectangle from output CRS to layer's CRS
   * @see outputExtentToLayerExtent() if you want to transform a bounding box
   * @return the transformed rectangle
   */
  QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );


protected:

  double mDpi;

  QSize mSize;

  QgsRectangle mExtent;

  QStringList mLayers;

  bool mProjectionsEnabled;
  QgsCoordinateReferenceSystem mDestCRS;

  // derived properties
  bool mValid; //!< whether the actual settings are valid (set in updateDerived())
  QgsRectangle mVisibleExtent; //!< extent with some additional white space that matches the output aspect ratio
  double mMapUnitsPerPixel;
  double mScale;


  // utiity stuff
  QgsScaleCalculator mScaleCalculator;

  const QgsCoordinateTransform* coordTransform( QgsMapLayer *layer );
};


#endif // QGSMAPSETTINGS_H
