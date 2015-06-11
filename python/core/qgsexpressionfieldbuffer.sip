class QgsExpressionFieldBuffer
{
%TypeHeaderCode
#include <qgsexpressionfieldbuffer.h>
%End
  public:
    class ExpressionField
    {
      ExpressionField();
      ExpressionField( QString exp, QgsField fld );
    };

    QgsExpressionFieldBuffer();

    /**
     * Add an expression to the buffer
     *
     * @param exp expression to add
     * @param fld field to add
     */
    void addExpression( const QString& exp, const QgsField& fld );

    /**
     * Remove an expression from the buffer
     *
     * @param index index of expression to remove
     */
    void removeExpression( int index );

    /**
     * Changes the expression at a given index
     *
     * @param index The index of the expression to change
     * @param exp   The new expression to set
     *
     * @note added in 2.9
     */
    void updateExpression( int index, const QString& exp );

    /**
     * Saves expressions to xml under the layer node
     */
    void writeXml( QDomNode& layer_node, QDomDocument& document ) const;

    /**
     * Reads expressions from project file
     */
    void readXml( const QDomNode& layer_node );

    /**
     * Adds fields with the expressions buffered in this object to a QgsFields object
     *
     * @param flds The fields to be updated
     */
    void updateFields( QgsFields& flds );

    const QList<QgsExpressionFieldBuffer::ExpressionField>& expressions() const;
};
