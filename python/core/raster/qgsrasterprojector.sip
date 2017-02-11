
/** \ingroup core
 * \brief QgsRasterProjector implements approximate projection support for
 * it calculates grid of points in source CRS for target CRS + extent
 * which are used to calculate affine transformation matrices.
 * \class QgsRasterProjector
 */
class QgsRasterProjector : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterprojector.h>
#include <qgscoordinatereferencesystem.h>
%End

  public:
    /** Precision defines if each pixel is reprojected or approximate reprojection based
     *  on an approximation matrix of reprojected points is used.
     */
    enum Precision
    {
      Approximate, //!< Approximate (default), fast but possibly inaccurate
      Exact,   //!< Exact, precise but slow
    };

    //! @deprecated since 2.18: use default constructor
    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        int theSrcDatumTransform,
                        int theDestDatumTransform,
                        const QgsRectangle& theDestExtent,
                        int theDestRows, int theDestCols,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      ) /Deprecated/;
    //! @deprecated since 2.18: use default constructor
    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        const QgsRectangle& theDestExtent,
                        int theDestRows, int theDestCols,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      ) /Deprecated/;
    //! @deprecated since 2.18: use default constructor
    QgsRasterProjector( const QgsCoordinateReferenceSystem& theSrcCRS,
                        const QgsCoordinateReferenceSystem& theDestCRS,
                        double theMaxSrcXRes, double theMaxSrcYRes,
                        const QgsRectangle& theExtent
                      ) /Deprecated/;
    QgsRasterProjector();

    /** \brief The destructor */
    ~QgsRasterProjector();

    virtual QgsRasterProjector *clone() const /Factory/;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    /** \brief set source and destination CRS */
    void setCRS( const QgsCoordinateReferenceSystem & theSrcCRS, const QgsCoordinateReferenceSystem & theDestCRS,
                 int srcDatumTransform = -1, int destDatumTransform = -1 );

    /** \brief Get source CRS */
    QgsCoordinateReferenceSystem srcCrs() const;

    /** \brief Get destination CRS */
    QgsCoordinateReferenceSystem destCrs() const;

    /** @deprecated since 2.18, does nothing */
    void setMaxSrcRes( double theMaxSrcXRes, double theMaxSrcYRes ) /Deprecated/;

    Precision precision() const;
    void setPrecision( Precision precision );
    // Translated precision mode, for use in ComboBox etc.
    static QString precisionLabel( Precision precision );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height ) / Factory /;
    QgsRasterBlock *block2( int bandNo, const QgsRectangle & extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) / Factory /;

    /** Calculate destination extent and size from source extent and size */
    bool destExtentSize( const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
                         QgsRectangle& theDestExtent, int& theDestXSize, int& theDestYSize );

    /** Calculate destination extent and size from source extent and size */
    static bool extentSize( const QgsCoordinateTransform* ct,
                            const QgsRectangle& theSrcExtent, int theSrcXSize, int theSrcYSize,
                            QgsRectangle& theDestExtent, int& theDestXSize, int& theDestYSize );
};
