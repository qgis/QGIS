/** \ingroup core
* \class QgsScaleExpression
* \brief Class storing parameters of a scale expression, which is a subclass of
* QgsExpression for expressions which return a size or width.
* \note Added in version 2.9
*/

class QgsScaleExpression : QgsExpression
{
%TypeHeaderCode
#include <qgsscaleexpression.h>
%End

  public:

    enum Type
    {
      Linear,
      Area,
      Flannery,
      Unknown
    };

    /** Constructor for QgsScaleExpression which parses an expression string
     * to determine whether it's a scale expression
     * @param expression expression string
     */
    QgsScaleExpression( const QString &expression );

    /** Constructor for QgsScaleExpression which creates an expression from
     * specified parameters
     * @param type scale method
     * @param baseExpression expression (or field) used for value
     * @param minValue minimum value, corresponds to specified minimum size
     * @param maxValue maximum value, corresponds to specified maximum size
     * @param minSize minimum size
     * @param maxSize maximum size
     * @param nullSize size in case expression evaluates to NULL
     */
    QgsScaleExpression( Type type, const QString& baseExpression, double minValue, double maxValue, double minSize, double maxSize, double nullSize = 0 );

    operator bool() const;

    /** Calculates the size corresponding to a specific value.
     * @param value
     * @returns calculated size using expression's parameters and type
     */
    double size( double value ) const;

    /** Returns the minimum size calculated by the expression
     * @see maxSize
     */
    double minSize() const;

    /** Returns the maximum size calculated by the expression
     * @see minSize
     */
    double maxSize() const;

    /** Returns the minimum value expected by the expression. The minimum
     * value corresponds to the expression's minimum size.
     * @see maxValue
     */
    double minValue() const;

    /** Returns the maximum value expected by the expression. The maximum
     * value corresponds to the expression's maximum size.
     * @see minValue
     */
    double maxValue() const;

    /** Returns the size value when expression evaluates to NULL.
     * @see nullSize
     */
    double nullSize() const;

    /** Returns the base expression string (or field reference) used for
     * calculating the values to be mapped to a size.
     */
    QString baseExpression() const;

    /** Returns the scale expression's type (method used to calculate
     * the size from a value).
     */
    Type type() const;

};

