/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrastercalcnode.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterCalcNode
{

%TypeHeaderCode
#include "qgsrastercalcnode.h"
%End
  public:
    enum Type
    {
      tOperator,
      tNumber,
      tRasterRef,
      tMatrix
    };

    enum Operator
    {
      opPLUS,
      opMINUS,
      opMUL,
      opDIV,
      opPOW,
      opSQRT,
      opSIN,
      opCOS,
      opTAN,
      opASIN,
      opACOS,
      opATAN,
      opEQ,
      opNE,
      opGT,
      opLT,
      opGE,
      opLE,
      opAND,
      opOR,
      opSIGN,
      opLOG,
      opLOG10,
      opNONE,
    };

    QgsRasterCalcNode();
%Docstring
 Constructor for QgsRasterCalcNode.
%End

    QgsRasterCalcNode( double number );
    QgsRasterCalcNode( QgsRasterMatrix *matrix );
    QgsRasterCalcNode( Operator op, QgsRasterCalcNode *left, QgsRasterCalcNode *right );
    QgsRasterCalcNode( const QString &rasterName );
    ~QgsRasterCalcNode();


    Type type() const;
%Docstring
QgsRasterCalcNode cannot be copied
 :rtype: Type
%End

    void setLeft( QgsRasterCalcNode *left );
    void setRight( QgsRasterCalcNode *right );


    static QgsRasterCalcNode *parseRasterCalcString( const QString &str, QString &parserErrorMsg ) /Factory/;
%Docstring
 :rtype: QgsRasterCalcNode
%End

  private:
    QgsRasterCalcNode( const QgsRasterCalcNode &rh );
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/analysis/raster/qgsrastercalcnode.h                              *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
