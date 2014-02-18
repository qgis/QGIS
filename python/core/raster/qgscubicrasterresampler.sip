class QgsCubicRasterResampler: QgsRasterResampler
{
%TypeHeaderCode
#include "qgscubicrasterresampler.h"
%End
  public:
    QgsCubicRasterResampler();
    ~QgsCubicRasterResampler();
    QgsRasterResampler * clone() const /Factory/;
    void resample( const QImage& srcImage, QImage& dstImage );
    QString type() const;
};
