
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

    QgsRasterProjector();

    /** \brief The destructor */
    ~QgsRasterProjector();

    virtual QgsRasterProjector *clone() const /Factory/;

    int bandCount() const;

    Qgis::DataType dataType( int bandNo ) const;

    //! \brief set source and destination CRS
    void setCrs( const QgsCoordinateReferenceSystem & srcCRS, const QgsCoordinateReferenceSystem & destCRS,
                 int srcDatumTransform = -1, int destDatumTransform = -1 );

    //! \brief Get source CRS
    QgsCoordinateReferenceSystem sourceCrs() const;

    //! \brief Get destination CRS
    QgsCoordinateReferenceSystem destinationCrs() const;

    Precision precision() const;
    void setPrecision( Precision precision );
    // Translated precision mode, for use in ComboBox etc.
    static QString precisionLabel( Precision precision );

    QgsRasterBlock *block( int bandNo, const QgsRectangle & extent, int width, int height, QgsRasterBlockFeedback* feedback = nullptr ) / Factory /;

    //! Calculate destination extent and size from source extent and size
    bool destExtentSize( const QgsRectangle& srcExtent, int srcXSize, int srcYSize,
                         QgsRectangle& destExtent, int& destXSize, int& destYSize );

    //! Calculate destination extent and size from source extent and size
    static bool extentSize( const QgsCoordinateTransform& ct,
                            const QgsRectangle& srcExtent, int srcXSize, int srcYSize,
                            QgsRectangle& destExtent, int& destXSize, int& destYSize );

};
