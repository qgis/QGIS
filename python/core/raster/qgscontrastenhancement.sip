
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

    QgsContrastEnhancement( QGis::DataType theDatatype = QGis::Byte );
    ~QgsContrastEnhancement();

    /*
     *
     * Static methods
     *
     */
    /** \brief Helper function that returns the maximum possible value for a GDAL data type */
    static double maximumValuePossible( QGis::DataType );

    /** \brief Helper function that returns the minimum possible value for a GDAL data type */
    static double minimumValuePossible( QGis::DataType );

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

    static QString contrastEnhancementAlgorithmString( ContrastEnhancementAlgorithm algorithm );

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

