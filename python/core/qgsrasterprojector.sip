
/**  Raster projector */

class QgsRasterProjector : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterprojector.h>
#include <qgscoordinatereferencesystem.h>
%End
  public:
    QgsRasterProjector(
      QgsCoordinateReferenceSystem theSrcCRS,
      QgsCoordinateReferenceSystem theDestCRS,
      QgsRectangle theDestExtent,
      int theDestRows, int theDestCols,
      double theMaxSrcXRes, double theMaxSrcYRes,
      QgsRectangle theExtent
    );
    QgsRasterProjector(
      QgsCoordinateReferenceSystem theSrcCRS,
      QgsCoordinateReferenceSystem theDestCRS,
      double theMaxSrcXRes, double theMaxSrcYRes,
      QgsRectangle theExtent
    );
    QgsRasterProjector();

    ~QgsRasterProjector();

    QgsRasterInterface * clone() const;

    int bandCount() const;

    QgsRasterInterface::DataType dataType( int bandNo ) const;

    void setCRS( const QgsCoordinateReferenceSystem & theSrcCRS, const QgsCoordinateReferenceSystem & theDestCRS );

    QgsCoordinateReferenceSystem srcCrs() const;

    QgsCoordinateReferenceSystem destCrs() const;

    void setMaxSrcRes( double theMaxSrcXRes, double theMaxSrcYRes );

    QgsRectangle srcExtent();

    int srcRows();
    int srcCols();
    void setSrcRows( int theRows );
    void setSrcCols( int theCols );

    void srcRowCol( int theDestRow, int theDestCol, int *theSrcRow, int *theSrcCol );

    int dstRows() const;
    int dstCols() const;

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );
};

