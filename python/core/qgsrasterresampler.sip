class QgsRasterResampler
{
%TypeHeaderCode
    #include "qgsrasterresampler.h"
    #include "qgsbilinearrasterresampler.h"
    #include "qgscubicrasterresampler.h"

%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsBilinearRasterResampler*>(sipCpp) != NULL)
    sipClass = sipClass_QgsBilinearRasterResampler;
  else if (dynamic_cast<QgsCubicRasterResampler*>(sipCpp) != NULL)
    sipClass = sipClass_QgsCubicRasterResampler;
  else
    sipClass = 0;
%End

  public:
    virtual void resample( const QImage& srcImage, QImage& dstImage ) = 0;
    virtual QString type() const = 0;
};

class QgsBilinearRasterResampler: QgsRasterResampler
{
%TypeHeaderCode
    #include "qgsbilinearrasterresampler.h"
%End
  public:
    QgsBilinearRasterResampler();
    ~QgsBilinearRasterResampler();

    void resample( const QImage& srcImage, QImage& dstImage );
    QString type() const;
};

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
};
