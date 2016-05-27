class QgsMapToPixelSimplifier : QgsAbstractGeometrySimplifier
{
%TypeHeaderCode
#include "qgsmaptopixelgeometrysimplifier.h"
%End
  public:
    //! Types of simplification algorithms that can be used
    enum SimplifyAlgorithm
    {
      Distance    = 0, //!< The simplification uses the distance between points to remove duplicate points
      SnapToGrid  = 1, //!< The simplification uses a grid (similar to ST_SnapToGrid) to remove duplicate points
      Visvalingam = 2, //!< The simplification gives each point in a line an importance weighting, so that least important points are removed first
    };

    //! Constructor
    QgsMapToPixelSimplifier( int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm = Distance );
    virtual ~QgsMapToPixelSimplifier();

    //! Applicable simplification flags
    enum SimplifyFlag
    {
      NoFlags          = 0, //!< No simplification can be applied
      SimplifyGeometry = 1, //!< The geometries can be simplified using the current map2pixel context state
      SimplifyEnvelope = 2, //!< The geometries can be fully simplified by its BoundingBox
    };

  protected:

    //! Returns the squared 2D-distance of the vector defined by the two points specified
    static float calculateLengthSquared2D( double x1, double y1, double x2, double y2 );

    //! Returns whether the points belong to the same grid
    static bool equalSnapToGrid( double x1, double y1, double x2, double y2, double gridOriginX, double gridOriginY, float gridInverseSizeXY );

  public:
    //! Gets the simplification hints of the vector layer managed
    int simplifyFlags() const;
    //! Sets the simplification hints of the vector layer managed
    void setSimplifyFlags( int simplifyFlags );

    //! Gets the local simplification algorithm of the vector layer managed
    SimplifyAlgorithm simplifyAlgorithm() const;
    //! Sets the local simplification algorithm of the vector layer managed
    void setSimplifyAlgorithm( SimplifyAlgorithm simplifyAlgorithm );

    //! Returns a simplified version the specified geometry
    virtual QgsGeometry* simplify( QgsGeometry* geometry ) const;
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( QgsGeometry* geometry ) const;

    //! Simplifies the specified WKB-point array
    virtual bool simplifyPoints( QgsWKBTypes::Type wkbType, QgsConstWkbPtr& sourceWkbPtr, QPolygonF& targetPoints ) const;

    // MapToPixel simplification helper methods
  public:

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    static bool isGeneralizableByMapBoundingBox( const QgsRectangle& envelope, double map2pixelTol );

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    bool isGeneralizableByMapBoundingBox( const QgsRectangle& envelope ) const;

    //! Simplifies the geometry when is applied the specified map2pixel context
    static bool simplifyGeometry( QgsGeometry* geometry, int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm = Distance );

    //! Simplifies the WKB-point array when is applied the specified map2pixel context
    static bool simplifyPoints( QgsWKBTypes::Type wkbType, QgsConstWkbPtr& sourceWkbPtr, QPolygonF& targetPoints, int simplifyFlags, double tolerance, SimplifyAlgorithm simplifyAlgorithm = Distance );

};
