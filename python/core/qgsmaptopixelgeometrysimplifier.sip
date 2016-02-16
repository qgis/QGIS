class QgsMapToPixelSimplifier : QgsAbstractGeometrySimplifier
{
%TypeHeaderCode
#include "qgsmaptopixelgeometrysimplifier.h"
%End
  public:
    QgsMapToPixelSimplifier( int simplifyFlags, double tolerance );
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

  public:
    int simplifyFlags() const;
    void setSimplifyFlags( int simplifyFlags );

    //! Returns a simplified version the specified geometry
    virtual QgsGeometry* simplify( QgsGeometry* geometry ) const;
    //! Simplifies the specified geometry
    virtual bool simplifyGeometry( QgsGeometry* geometry ) const;

    // MapToPixel simplification helper methods
  public:

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    static bool isGeneralizableByMapBoundingBox( const QgsRectangle& envelope, double map2pixelTol );

    //! Returns whether the envelope can be replaced by its BBOX when is applied the specified map2pixel context
    bool isGeneralizableByMapBoundingBox( const QgsRectangle& envelope ) const;

    //! Simplifies the geometry when is applied the specified map2pixel context
    static bool simplifyGeometry( QgsGeometry* geometry, int simplifyFlags, double tolerance );

};
