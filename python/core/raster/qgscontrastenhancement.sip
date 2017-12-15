/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgscontrastenhancement.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsContrastEnhancement
{
%Docstring
Manipulates raster pixel values so that they enhanceContrast or clip into a
specified numerical range according to the specified
ContrastEnhancementAlgorithm.
%End

%TypeHeaderCode
#include "qgscontrastenhancement.h"
%End
  public:

    enum ContrastEnhancementAlgorithm
    {
      NoEnhancement,
      StretchToMinimumMaximum,
      StretchAndClipToMinimumMaximum,
      ClipToMinimumMaximum,
      UserDefinedEnhancement
    };

    QgsContrastEnhancement( Qgis::DataType datatype = Qgis::Byte );
    QgsContrastEnhancement( const QgsContrastEnhancement &ce );
    ~QgsContrastEnhancement();


    static double maximumValuePossible( Qgis::DataType );
%Docstring
 Helper function that returns the maximum possible value for a GDAL data type

:rtype: float
%End

    static double minimumValuePossible( Qgis::DataType );
%Docstring
 Helper function that returns the minimum possible value for a GDAL data type

:rtype: float
%End

    static QString contrastEnhancementAlgorithmString( ContrastEnhancementAlgorithm algorithm );
%Docstring
 Return a string to serialize ContrastEnhancementAlgorithm

:rtype: str
%End

    static ContrastEnhancementAlgorithm contrastEnhancementAlgorithmFromString( const QString &contrastEnhancementString );
%Docstring
 Deserialize ContrastEnhancementAlgorithm

:rtype: ContrastEnhancementAlgorithm
%End

    double maximumValue() const;
%Docstring
 Return the maximum value for the contrast enhancement range.

:rtype: float
%End

    double minimumValue() const;
%Docstring
 Return the minimum value for the contrast enhancement range.

:rtype: float
%End

    ContrastEnhancementAlgorithm contrastEnhancementAlgorithm() const;
%Docstring

:rtype: ContrastEnhancementAlgorithm
%End

    int enhanceContrast( double );
%Docstring
 Apply the contrast enhancement to a value. Return values are 0 - 254, -1 means the pixel was clipped and should not be displayed

:rtype: int
%End

    bool isValueInDisplayableRange( double );
%Docstring
 Return true if pixel is in stretable range, false if pixel is outside of range (i.e., clipped)

:rtype: bool
%End

    void setContrastEnhancementAlgorithm( ContrastEnhancementAlgorithm, bool generateTable = true );
%Docstring
 Set the contrast enhancement algorithm
%End

    void setContrastEnhancementFunction( QgsContrastEnhancementFunction * );
%Docstring
 A public method that allows the user to set their own custom contrast enhancement function
%End

    void setMaximumValue( double, bool generateTable = true );
%Docstring
 Set the maximum value for the contrast enhancement range.
%End

    void setMinimumValue( double, bool generateTable = true );
%Docstring
 Return the minimum value for the contrast enhancement range.
%End

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;

    void readXml( const QDomElement &elem );

  private:
    const QgsContrastEnhancement &operator=( const QgsContrastEnhancement & );
%Docstring

:rtype: QgsContrastEnhancement
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgscontrastenhancement.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
