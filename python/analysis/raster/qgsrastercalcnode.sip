class QgsRasterCalcNode
{
%TypeHeaderCode
#include <qgsrastercalcnode.h>
%End
  public:
    //! defines possible types of node
    enum Type
    {
      tOperator,
      tNumber,
      tRasterRef
    };

    //! possible operators
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
      opEQ,         // =
      opNE,         //!=
      opGT,         // >
      opLT,         // <
      opGE,         // >=
      opLE,         // <=
      opAND,
      opOR,
      opSIGN,       // change sign
      opNONE,
    };

    QgsRasterCalcNode();
    QgsRasterCalcNode( double number );
    QgsRasterCalcNode( Operator op, QgsRasterCalcNode* left, QgsRasterCalcNode* right );
    QgsRasterCalcNode( const QString& rasterName );
    ~QgsRasterCalcNode();

    Type type() const;

    //set left node
    void setLeft( QgsRasterCalcNode* left );
    void setRight( QgsRasterCalcNode* right );

    /**Calculates result (might be real matrix or single number)*/
    bool calculate( QMap<QString, QgsRasterMatrix*>& rasterData, QgsRasterMatrix& result ) const;

    static QgsRasterCalcNode* parseRasterCalcString( const QString& str, QString& parserErrorMsg ) /Factory/;
};
