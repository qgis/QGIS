class QgsCubicRasterResampler: QgsRasterResampler
{
%TypeHeaderCode
#include "qgscubicrasterresampler.h"
%End
  public:
    QgsCubicRasterResampler();
    ~QgsCubicRasterResampler();
    void resample( const QImage& srcImage, QImage& dstImage );
    QString type() const;
    QgsRasterResampler * clone() const /Factory/;
};
