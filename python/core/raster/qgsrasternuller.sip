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

    QgsRasterInterface * clone() const /Factory/;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) / Factory /;

    void setNoData( QList<QgsRasterNuller::NoData> noData );

    QList<QgsRasterNuller::NoData> noData() const;

    /** \brief Set output no data value. */
    void setOutputNoData( double noData );
};
