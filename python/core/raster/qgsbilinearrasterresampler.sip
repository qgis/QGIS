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
    virtual QgsBilinearRasterResampler * clone() const /Factory/;
};
