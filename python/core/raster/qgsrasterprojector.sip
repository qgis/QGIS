
/** Raster projector */

class QgsRasterProjector : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterprojector.h>
#include <qgscoordinatereferencesystem.h>
%End

  public:
    /** Precison defines if each pixel is reprojected or approximate reprojection based
     *  on an approximation matrix of reprojected points is used.
     */
    enum Precision
    {
      Approximate, //!< Approximate (default), fast but possibly inaccurate
      Exact,   //!< Exact, precise but slow
    };

    /** \brief QgsRasterProjector implements approximate projection support for
     * it calculates grid of points in source CRS for target CRS + extent
     * which are used to calculate affine transformation matrices.
     */

    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        int theSrcDatumTransform,
                        int theDestDatumTransform,
                        const QgsRectangle& theDestExtent,
                        int theDestRows, int theDestCols,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      );

    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        const QgsRectangle& theDestExtent,
                        int theDestRows, int theDestCols,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      );
    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
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

    Precision precision() const;
    void setPrecision( Precision precision );
    // Translated precision mode, for use in ComboBox etc.
    static QString precisionLabel( Precision precision );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;

    /** Calculate destination extent and size from source extent and size */
    bool destExtentSize( const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
                         QgsRectangle& theDestExtent, int& theDestXSize, int& theDestYSize );

    /** Calculate destination extent and size from source extent and size */
    static bool extentSize( const QgsCoordinateTransform* ct,
                            const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
                            QgsRectangle& theDestExtent, int& theDestXSize, int& theDestYSize );
};
