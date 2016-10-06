class QgsCubicRasterResampler: QgsRasterResampler
{
%TypeHeaderCode
#include "qgscubicrasterresampler.h"
%End
  public:
    QgsCubicRasterResampler();
    ~QgsCubicRasterResampler();
    virtual QgsCubicRasterResampler * clone() const /Factory/;
    void resample( const QImage& srcImage, QImage& dstImage );
    QString type() const;
};
