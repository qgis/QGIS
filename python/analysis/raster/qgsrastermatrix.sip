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
Returns true if matrix is 1x1 (=scalar number)
 :rtype: bool
%End
    double number() const;
%Docstring
 :rtype: float
%End



    void setData( int cols, int rows, double *data, double nodataValue );

    int nColumns() const;
%Docstring
 :rtype: int
%End
    int nRows() const;
%Docstring
 :rtype: int
%End

    double nodataValue() const;
%Docstring
 :rtype: float
%End
    void setNodataValue( double d );

    bool add( const QgsRasterMatrix &other );
%Docstring
Adds another matrix to this one
 :rtype: bool
%End
    bool subtract( const QgsRasterMatrix &other );
%Docstring
Subtracts another matrix from this one
 :rtype: bool
%End
    bool multiply( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool divide( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool power( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool equal( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool notEqual( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool greaterThan( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool lesserThan( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool greaterEqual( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool lesserEqual( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool logicalAnd( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End
    bool logicalOr( const QgsRasterMatrix &other );
%Docstring
 :rtype: bool
%End

    bool squareRoot();
%Docstring
 :rtype: bool
%End
    bool sinus();
%Docstring
 :rtype: bool
%End
    bool asinus();
%Docstring
 :rtype: bool
%End
    bool cosinus();
%Docstring
 :rtype: bool
%End
    bool acosinus();
%Docstring
 :rtype: bool
%End
    bool tangens();
%Docstring
 :rtype: bool
%End
    bool atangens();
%Docstring
 :rtype: bool
%End
    bool changeSign();
%Docstring
 :rtype: bool
%End
    bool log();
%Docstring
 :rtype: bool
%End
    bool log10();
%Docstring
 :rtype: bool
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrastermatrix.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
