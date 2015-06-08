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
      tRasterRef,
      tMatrix
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
    QgsRasterCalcNode( QgsRasterMatrix* matrix );
    QgsRasterCalcNode( Operator op, QgsRasterCalcNode* left, QgsRasterCalcNode* right );
    QgsRasterCalcNode( const QString& rasterName );
    ~QgsRasterCalcNode();

    Type type() const;

    //set left node
    void setLeft( QgsRasterCalcNode* left );
    void setRight( QgsRasterCalcNode* right );

    /**Calculates result (might be real matrix or single number)*/
   // bool calculate( QMap<QString, QgsRasterCalculateInputMatrix*>& rasterData, QgsRasterMatrix& result ) const;

    /**Calculates result of raster calculation (might be real matrix or single number).
     * @param rasterData input raster data references, map of raster name to raster data block
     * @param result destination raster matrix for calculation results
     * @param row optional row number to calculate for calculating result by rows, or -1 to
     * calculate entire result
     * @note added in QGIS 2.10
     * @note not available in Python bindings
     */
    //bool calculate( QMap<QString, QgsRasterBlock* >& rasterData, QgsRasterMatrix& result, int row = -1 ) const;

    /**@deprecated use method which accepts QgsRasterBlocks instead
     */
    // bool calculate( QMap<QString, QgsRasterMatrix*>& rasterData, QgsRasterMatrix& result ) const /Deprecated/;

    static QgsRasterCalcNode* parseRasterCalcString( const QString& str, QString& parserErrorMsg ) /Factory/;
};
