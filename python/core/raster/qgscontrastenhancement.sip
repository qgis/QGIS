
class QgsContrastEnhancement
{
%TypeHeaderCode
#include <qgscontrastenhancement.h>
%End

  public:

    /** \brief This enumerator describes the types of contrast enhancement algorithms that can be used.  */
    enum ContrastEnhancementAlgorithm
    {
      NoEnhancement,                  //this should be the default color scaling algorithm
      StretchToMinimumMaximum,        //linear histogram enhanceContrast
      StretchAndClipToMinimumMaximum,
      ClipToMinimumMaximum,
      UserDefinedEnhancement
    };

    /** These are exactly the same as GDAL pixel data types
     ** This was added so that the python bindings could be built,
     ** which initially was a problem because GDALDataType was passed
     ** around as an argument to numerous method, including the constructor.
     **
     ** It seems like there should be a better way to do this...
     */
    enum QgsRasterDataType
    {
      QGS_Unknown = 0,
      /*! Eight bit unsigned integer */           QGS_Byte = 1,
      /*! Sixteen bit unsigned integer */         QGS_UInt16 = 2,
      /*! Sixteen bit signed integer */           QGS_Int16 = 3,
      /*! Thirty two bit unsigned integer */      QGS_UInt32 = 4,
      /*! Thirty two bit signed integer */        QGS_Int32 = 5,
      /*! Thirty two bit floating point */        QGS_Float32 = 6,
      /*! Sixty four bit floating point */        QGS_Float64 = 7,
      /*! Complex Int16 */                        QGS_CInt16 = 8,
      /*! Complex Int32 */                        QGS_CInt32 = 9,
      /*! Complex Float32 */                      QGS_CFloat32 = 10,
      /*! Complex Float64 */                      QGS_CFloat64 = 11,
      QGS_TypeCount = 12          /* maximum type # + 1 */
    };

    QgsContrastEnhancement( QgsContrastEnhancement::QgsRasterDataType theDatatype = QGS_Byte );
    ~QgsContrastEnhancement();

    /*
     *
     * Static methods
     *
     */
    /** \brief Helper function that returns the maximum possible value for a GDAL data type */
    static double maximumValuePossible( QgsRasterDataType );

    /** \brief Helper function that returns the minimum possible value for a GDAL data type */
    static double minimumValuePossible( QgsRasterDataType );

    /*
     *
     * Non-Static Inline methods
     *
     */
    /** \brief Return the maximum value for the contrast enhancement range. */
    double maximumValue() const;

    /** \brief Return the minimum value for the contrast enhancement range. */
    double minimumValue() const;

    ContrastEnhancementAlgorithm contrastEnhancementAlgorithm() const;

    static ContrastEnhancementAlgorithm contrastEnhancementAlgorithmFromString( const QString& contrastEnhancementString );

    /*
     *
     * Non-Static methods
     *
     */
    /** \brief Apply the contrast enhancement to a value. Return values are 0 - 254, -1 means the pixel was clipped and should not be displayed */
    int enhanceContrast( double );

    /** \brief Return true if pixel is in stretable range, false if pixel is outside of range (i.e., clipped) */
    bool isValueInDisplayableRange( double );

    /** \brief Set the contrast enhancement algorithm */
    void setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm, bool generateTable = true );

    /** \brief A public method that allows the user to set their own custom contrast enhancment function */
    void setContrastEnhancementFunction( QgsContrastEnhancementFunction* );

    /** \brief Set the maximum value for the contrast enhancement range. */
    void setMaximumValue( double, bool generateTable = true );

    /** \brief Return the minimum value for the contrast enhancement range. */
    void setMinimumValue( double, bool generateTable = true );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    void readXML( const QDomElement& elem );
};

