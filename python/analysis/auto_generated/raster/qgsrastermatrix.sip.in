/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrastermatrix.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsRasterMatrix
{

%TypeHeaderCode
#include "qgsrastermatrix.h"
%End
  public:

    enum TwoArgOperator
    {
      opPLUS,
      opMINUS,
      opMUL,
      opDIV,
      opPOW,
      opEQ,
      opNE,
      opGT,
      opLT,
      opGE,
      opLE,
      opAND,
      opOR
    };

    enum OneArgOperator
    {
      opSQRT,
      opSIN,
      opCOS,
      opTAN,
      opASIN,
      opACOS,
      opATAN,
      opSIGN,
      opLOG,
      opLOG10,
    };

    QgsRasterMatrix();
%Docstring
Takes ownership of data array
%End

    QgsRasterMatrix( const QgsRasterMatrix &m );
    ~QgsRasterMatrix();

    bool isNumber() const;
%Docstring
Returns ``True`` if matrix is 1x1 (=scalar number)
%End
    double number() const;



    void setData( int cols, int rows, double *data, double nodataValue );

    int nColumns() const;
    int nRows() const;

    double nodataValue() const;
    void setNodataValue( double d );

    bool add( const QgsRasterMatrix &other );
%Docstring
Adds another matrix to this one
%End
    bool subtract( const QgsRasterMatrix &other );
%Docstring
Subtracts another matrix from this one
%End
    bool multiply( const QgsRasterMatrix &other );
    bool divide( const QgsRasterMatrix &other );
    bool power( const QgsRasterMatrix &other );
    bool equal( const QgsRasterMatrix &other );
    bool notEqual( const QgsRasterMatrix &other );
    bool greaterThan( const QgsRasterMatrix &other );
    bool lesserThan( const QgsRasterMatrix &other );
    bool greaterEqual( const QgsRasterMatrix &other );
    bool lesserEqual( const QgsRasterMatrix &other );
    bool logicalAnd( const QgsRasterMatrix &other );
    bool logicalOr( const QgsRasterMatrix &other );

    bool squareRoot();
    bool sinus();
    bool asinus();
    bool cosinus();
    bool acosinus();
    bool tangens();
    bool atangens();
    bool changeSign();
    bool log();
    bool log10();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrastermatrix.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
