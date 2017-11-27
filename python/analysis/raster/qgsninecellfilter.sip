/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsninecellfilter.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsNineCellFilter
{
%Docstring
 Base class for raster analysis methods that work with a 3x3 cell filter and calculate the value of each cell based on
the cell value and the eight neighbour cells. Common examples are slope and aspect calculation in DEMs. Subclasses only implement
the method that calculates the new value from the nine values. Everything else (reading file, writing file) is done by this subclass*
%End

%TypeHeaderCode
#include "qgsninecellfilter.h"
%End
  public:
    QgsNineCellFilter( const QString &inputFile, const QString &outputFile, const QString &outputFormat );
%Docstring
Constructor that takes input file, output file and output format (GDAL string)
%End
    virtual ~QgsNineCellFilter();

    int processRaster( QgsFeedback *feedback = 0 );
%Docstring
 Starts the calculation, reads from mInputFile and stores the result in mOutputFile
\param feedback feedback object that receives update and that is checked for cancelation.
:return: 0 in case of success*
 :rtype: int
%End

    double cellSizeX() const;
%Docstring
 :rtype: float
%End
    void setCellSizeX( double size );
    double cellSizeY() const;
%Docstring
 :rtype: float
%End
    void setCellSizeY( double size );

    double zFactor() const;
%Docstring
 :rtype: float
%End
    void setZFactor( double factor );

    double inputNodataValue() const;
%Docstring
 :rtype: float
%End
    void setInputNodataValue( double value );
    double outputNodataValue() const;
%Docstring
 :rtype: float
%End
    void setOutputNodataValue( double value );

    virtual float processNineCellWindow( float *x11, float *x21, float *x31,
                                         float *x12, float *x22, float *x32,
                                         float *x13, float *x23, float *x33 ) = 0;
%Docstring
 Calculates output value from nine input values. The input values and the output value can be equal to the
nodata value if not present or outside of the border. Must be implemented by subclasses*
 :rtype: float
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsninecellfilter.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
