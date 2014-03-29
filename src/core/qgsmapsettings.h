#ifndef QGSMAPSETTINGS_H
#define QGSMAPSETTINGS_H

#include <QColor>
#include <QSize>
#include <QStringList>

#include "qgscoordinatereferencesystem.h"
#include "qgsmaptopixel.h"
#include "qgsrectangle.h"
#include "qgsscalecalculator.h"

class QPainter;

class QgsCoordinateTransform;
class QgsScaleCalculator;
class QgsMapRendererJob;
class QgsMapLayer;


class CORE_EXPORT QgsMapSettings
{
  public:
    QgsMapSettings();

    QgsRectangle extent() const;
    void setExtent( const QgsRectangle& rect );

    QSize outputSize() const;
    void setOutputSize( const QSize& size );

    int outputDpi() const;
    void setOutputDpi( int dpi );

    QStringList layers() const;
    void setLayers( const QStringList& layers );

    //! sets whether to use projections for this layer set
    void setCrsTransformEnabled( bool enabled );
    //! returns true if projections are enabled for this layer set
    bool hasCrsTransformEnabled() const;

    //! sets destination coordinate reference system
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );
    //! returns CRS of destination coordinate reference system
    const QgsCoordinateReferenceSystem& destinationCrs() const;

    QGis::UnitType mapUnits() const;
    void setMapUnits( QGis::UnitType u );

    void setBackgroundColor( const QColor& color ) { mBackgroundColor = color; }
    QColor backgroundColor() const { return mBackgroundColor; }

    void setSelectionColor( const QColor& color ) { mSelectionColor = color; }
    QColor selectionColor() const { return mSelectionColor; }

    enum Flag
    {
      Antialiasing       = 0x01,
      DrawEditingInfo    = 0x02,
      ForceVectorOutput  = 0x04,
      UseAdvancedEffects = 0x08,
      DrawLabeling       = 0x10
      // TODO: ignore scale-based visibility (overview)
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    void setFlags( Flags flags );
    void setFlag( Flag flag, bool on = true );
    Flags flags() const;
    bool testFlag( Flag flag ) const;

    bool hasValidSettings() const;
    QgsRectangle visibleExtent() const;
    double mapUnitsPerPixel() const;
    double scale() const;


    // -- utility functions --

    const QgsMapToPixel& mapToPixel() const { return mMapToPixel; }

    /**
     * @brief transform bounding box from layer's CRS to output CRS
     * @see layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent ) const;

    /**
     * @brief transform bounding box from output CRS to layer's CRS
     * @see mapToLayerCoordinates( QgsMapLayer* theLayer,QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent ) const;

    /**
     * @brief transform point coordinates from layer's CRS to output CRS
     * @return the transformed point
     */
    QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point ) const;

    /**
     * @brief transform rectangle from layer's CRS to output CRS
     * @see layerExtentToOutputExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) const;

    /**
     * @brief transform point coordinates from output CRS to layer's CRS
     * @return the transformed point
     */
    QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point ) const;

    /**
     * @brief transform rectangle from output CRS to layer's CRS
     * @see outputExtentToLayerExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) const;


    //! returns current extent of layer set
    QgsRectangle fullExtent() const;

    /* serialization */

    void readXML( QDomNode& theNode );

    void writeXML( QDomNode& theNode, QDomDocument& theDoc );

  protected:

    int mDpi;

    QSize mSize;

    QgsRectangle mExtent;

    QStringList mLayers;

    bool mProjectionsEnabled;
    QgsCoordinateReferenceSystem mDestCRS;

    QColor mBackgroundColor;
    QColor mSelectionColor;

    Flags mFlags;

    // derived properties
    bool mValid; //!< whether the actual settings are valid (set in updateDerived())
    QgsRectangle mVisibleExtent; //!< extent with some additional white space that matches the output aspect ratio
    double mMapUnitsPerPixel;
    double mScale;


    // utiity stuff
    QgsScaleCalculator mScaleCalculator;
    QgsMapToPixel mMapToPixel;


    void updateDerived();

    const QgsCoordinateTransform* coordTransform( QgsMapLayer *layer ) const;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapSettings::Flags )


#endif // QGSMAPSETTINGS_H
