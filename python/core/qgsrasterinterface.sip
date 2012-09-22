
/**  Base class for processing modules.
 *
 */

class QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterinterface.h>
#include <qgsrasterresamplefilter.h>
//#include <qgsrasternuller.h>
#include <qgsrasterrenderer.h>
#include <qgsrasterprojector.h>
#include <qgsrasterdataprovider.h>

// QgsRasterRenderer subclass headers must be here because ConvertToSubClassCode 
// from QgsRasterRenderer is probably included
#include <qgspalettedrasterrenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgssinglebandpseudocolorrenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgssinglebandcolordatarenderer.h>
%End

%ConvertToSubClassCode
  if (dynamic_cast<QgsRasterResampleFilter*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterResampleFilter;
//  if (dynamic_cast<QgsRasterNuller*>(sipCpp) != NULL)
//    sipClass = sipClass_QgsRasterNuller;
  if (dynamic_cast<QgsRasterRenderer*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterRenderer;
  if (dynamic_cast<QgsRasterProjector*>(sipCpp) != NULL)
    sipClass = sipClass_QgsRasterProjector;
  if (dynamic_cast<QgsRasterDataProvider*>(sipCpp) != NULL)
  {
    sipClass = sipClass_QgsRasterDataProvider;
    *sipCppRet = static_cast<QgsRasterDataProvider*>(sipCpp);
  }
  else
    sipClass = 0;
%End

  public:

    enum DataType
    {
      /*! Unknown or unspecified type */          UnknownDataType = 0,
      /*! Eight bit unsigned integer */           Byte = 1,
      /*! Sixteen bit unsigned integer */         UInt16 = 2,
      /*! Sixteen bit signed integer */           Int16 = 3,
      /*! Thirty two bit unsigned integer */      UInt32 = 4,
      /*! Thirty two bit signed integer */        Int32 = 5,
      /*! Thirty two bit floating point */        Float32 = 6,
      /*! Sixty four bit floating point */        Float64 = 7,
      /*! Complex Int16 */                        CInt16 = 8,
      /*! Complex Int32 */                        CInt32 = 9,
      /*! Complex Float32 */                      CFloat32 = 10,
      /*! Complex Float64 */                      CFloat64 = 11,
      /*! Color, alpha, red, green, blue, 4 bytes the same as
          QImage::Format_ARGB32 */                ARGB32 = 12,
      /*! Color, alpha, red, green, blue, 4 bytes  the same as
          QImage::Format_ARGB32_Premultiplied */  ARGB32_Premultiplied = 13,

      TypeCount = 14          /* maximum type # + 1 */
    };

    QgsRasterInterface( QgsRasterInterface * input = 0 );

    virtual ~QgsRasterInterface();

    int typeSize( int dataType ) const;

    virtual QgsRasterInterface *clone() const = 0;

    int dataTypeSize( int bandNo ) const;

    bool typeIsNumeric( DataType type ) const;

    bool typeIsColor( DataType type ) const;

    virtual DataType dataType( int bandNo ) const;

    virtual int bandCount() const;

    virtual double noDataValue( int bandNo ) const;

    virtual void * block( int bandNo, const QgsRectangle  & extent, int width, int height );

    virtual void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    virtual bool setInput( QgsRasterInterface* input );

    virtual QgsRasterInterface * input () const;

    virtual bool on( );

    virtual void setOn( bool on );

    QgsRasterInterface * srcInput();

    QImage * createImage( int width, int height, QImage::Format format );

    void setStatsOn( bool on );

    double time( bool cumulative = false );

    static QString printValue( double value );

  protected:
    static double readValue( void *data, QgsRasterInterface::DataType type, int index );

    static void writeValue( void *data, QgsRasterInterface::DataType type, int index, double value );
};

