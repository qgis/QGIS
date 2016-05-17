class QgsRasterNuller : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasternuller.h>
%End
  public:
    QgsRasterNuller( QgsRasterInterface* input = 0 );
    ~QgsRasterNuller();

    struct NoData
    {
      double min;
      double max;
    };

    virtual QgsRasterNuller * clone() const /Factory/;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) / Factory /;

    void setNoData( int bandNo, const QgsRasterRangeList& noData );

    QgsRasterRangeList noData( int bandNo ) const;

    /** \brief Set output no data value. */
    void setOutputNoDataValue( int bandNo, double noData );
};
