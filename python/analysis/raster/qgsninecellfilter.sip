class QgsNineCellFilter
{
%TypeHeaderCode
#include <qgsninecellfilter.h>
%End
  public:
    /** Constructor that takes input file, output file and output format (GDAL string)*/
    QgsNineCellFilter( const QString& inputFile, const QString& outputFile, const QString& outputFormat );
    virtual ~QgsNineCellFilter();
    /** Starts the calculation, reads from mInputFile and stores the result in mOutputFile
      @param p progress dialog that receives update and that is checked for abort. 0 if no progress bar is needed.
      @return 0 in case of success*/
    int processRaster( QProgressDialog* p );

    double cellSizeX() const;
    void setCellSizeX( double size );
    double cellSizeY() const;
    void setCellSizeY( double size );

    double zFactor() const;
    void setZFactor( double factor );

    double inputNodataValue() const;
    void setInputNodataValue( double value );
    double outputNodataValue() const;
    void setOutputNodataValue( double value );

    /** Calculates output value from nine input values. The input values and the output value can be equal to the
      nodata value if not present or outside of the border. Must be implemented by subclasses*/
    virtual float processNineCellWindow( float* x11, float* x21, float* x31,
                                         float* x12, float* x22, float* x32,
                                         float* x13, float* x23, float* x33 ) = 0;
};
