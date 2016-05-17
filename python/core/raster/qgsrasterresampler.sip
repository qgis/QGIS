class QgsRasterResampler
{
%TypeHeaderCode
    #include "qgsrasterresampler.h"
    #include "qgsbilinearrasterresampler.h"
    #include "qgscubicrasterresampler.h"

%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsBilinearRasterResampler*>(sipCpp) != NULL)
    sipType = sipType_QgsBilinearRasterResampler;
  else if (dynamic_cast<QgsCubicRasterResampler*>(sipCpp) != NULL)
    sipType = sipType_QgsCubicRasterResampler;
  else
    sipType = 0;
%End

  public:
    virtual ~QgsRasterResampler();
    virtual void resample( const QImage& srcImage, QImage& dstImage ) = 0;
    virtual QString type() const = 0;
    virtual QgsRasterResampler * clone() const = 0 /Factory/;
};
