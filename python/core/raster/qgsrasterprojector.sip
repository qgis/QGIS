
/** Raster projector */

class QgsRasterProjector : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterprojector.h>
#include <qgscoordinatereferencesystem.h>
%End

  public:
    /** \brief QgsRasterProjector implements approximate projection support for
     * it calculates grid of points in source CRS for target CRS + extent
     * which are used to calculate affine transformation matrices.
     */

    QgsRasterProjector(
      const QgsCoordinateReferenceSystem& theSrcCRS,
      const QgsCoordinateReferenceSystem& theDestCRS,
      int theSrcDatumTransform,
      int theDestDatumTransform,
      const QgsRectangle& theDestExtent,
      int theDestRows, int theDestCols,
      double theMaxSrcXRes, double theMaxSrcYRes,
      const QgsRectangle& theExtent
    );

    QgsRasterProjector(
      const QgsCoordinateReferenceSystem& theSrcCRS,
      const QgsCoordinateReferenceSystem& theDestCRS,
      const QgsRectangle& theDestExtent,
      int theDestRows, int theDestCols,
      double theMaxSrcXRes, double theMaxSrcYRes,
      const QgsRectangle& theExtent
    );
    QgsRasterProjector(
      const QgsCoordinateReferenceSystem& theSrcCRS,
      const QgsCoordinateReferenceSystem& theDestCRS,
      double theMaxSrcXRes, double theMaxSrcYRes,
      const QgsRectangle& theExtent
    );
    QgsRasterProjector();

    /** \brief The destructor */
    ~QgsRasterProjector();

    virtual QgsRasterProjector *clone() const /Factory/;

    int bandCount() const;

    int dataType( int bandNo ) const;

    /** \brief set source and destination CRS */
    void setCRS( const QgsCoordinateReferenceSystem & theSrcCRS, const QgsCoordinateReferenceSystem & theDestCRS,
                 int srcDatumTransform = -1, int destDatumTransform = -1 );

    /** \brief Get source CRS */
    QgsCoordinateReferenceSystem srcCrs() const;

    /** \brief Get destination CRS */
    QgsCoordinateReferenceSystem destCrs() const;

    /** \brief set maximum source resolution */
    void setMaxSrcRes( double theMaxSrcXRes, double theMaxSrcYRes );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;
};
