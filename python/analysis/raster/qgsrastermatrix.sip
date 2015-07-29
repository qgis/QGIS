class QgsRasterMatrix
{
%TypeHeaderCode
#include <qgsrastermatrix.h>
%End

  public:

    enum TwoArgOperator
    {
      opPLUS,
      opMINUS,
      opMUL,
      opDIV,
      opPOW,
      opEQ,         // =
      opNE,         // != resp. <>
      opGT,         // >
      opLT,         // <
      opGE,         // >=
      opLE,         // <=
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
      opSIGN
    };

    /** Takes ownership of data array*/
    QgsRasterMatrix();
    //! @note note available in python bindings
    // QgsRasterMatrix( int nCols, int nRows, float* data, double nodataValue );
    QgsRasterMatrix( const QgsRasterMatrix& m );
    ~QgsRasterMatrix();

    /** Returns true if matrix is 1x1 (=scalar number)*/
    bool isNumber() const;
    double number() const;

    /** Returns data array (but not ownership)*/
    //! @note not available in python bindings
    // double* data();
    /** Returns data and ownership. Sets data and nrows, ncols of this matrix to 0*/
    //! @note not available in python bindings
    // double* takeData();

    void setData( int cols, int rows, double* data, double nodataValue );

    int nColumns() const;
    int nRows() const;

    double nodataValue() const;
    void setNodataValue( double d );

    // QgsRasterMatrix& operator=( const QgsRasterMatrix& m );
    /** Adds another matrix to this one*/
    bool add( const QgsRasterMatrix& other );
    /** Subtracts another matrix from this one*/
    bool subtract( const QgsRasterMatrix& other );
    bool multiply( const QgsRasterMatrix& other );
    bool divide( const QgsRasterMatrix& other );
    bool power( const QgsRasterMatrix& other );
    bool equal( const QgsRasterMatrix& other );
    bool notEqual( const QgsRasterMatrix& other );
    bool greaterThan( const QgsRasterMatrix& other );
    bool lesserThan( const QgsRasterMatrix& other );
    bool greaterEqual( const QgsRasterMatrix& other );
    bool lesserEqual( const QgsRasterMatrix& other );
    bool logicalAnd( const QgsRasterMatrix& other );
    bool logicalOr( const QgsRasterMatrix& other );

    bool squareRoot();
    bool sinus();
    bool asinus();
    bool cosinus();
    bool acosinus();
    bool tangens();
    bool atangens();
    bool changeSign();
};
