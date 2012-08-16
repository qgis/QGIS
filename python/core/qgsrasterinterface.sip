
/**  Base class for processing modules.
 *
 */

class QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterinterface.h>
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

    virtual double noDataValue() const;

    //void * block( int bandNo, QgsRectangle  const & extent, int width, int height );

    //virtual void * readBlock( int bandNo, QgsRectangle  const & extent, int width, int height );

    virtual bool setInput( QgsRasterInterface* input );

    virtual bool on( );

    virtual void setOn( bool on );

    virtual QgsRasterInterface * srcInput();

    QImage * createImage( int width, int height, QImage::Format format );

    void setStatsOn( bool on );

    double time( bool cumulative = false );
};

