class QgsBilinearRasterResampler : QgsRasterResampler
{
%TypeHeaderCode
#include "qgsbilinearrasterresampler.h"
%End
  public:
    QgsBilinearRasterResampler();
    ~QgsBilinearRasterResampler();

    void resample( const QImage& srcImage, QImage& dstImage );
    QString type() const;
    QgsRasterResampler * clone() const /Factory/;
};
