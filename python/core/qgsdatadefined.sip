/** \ingroup core
 * \class QgsDataDefined
 * A container class for data source field mapping or expression.
 * @note added in QGIS 1.9
 */

class QgsDataDefined
{
%TypeHeaderCode
#include <qgsdatadefined.h>
%End
  public:
    /**
     * Construct a new data defined object
     *
     * @param active Whether the current data defined is active
     * @param useexpr Whether to use expression instead of field
     * @param expr Expression string
     * @param field Field name string
     */
    QgsDataDefined( bool active = false,
                    bool useexpr = false,
                    const QString& expr = QString(),
                    const QString& field = QString() );

    ~QgsDataDefined();

    bool isActive() const;
    void setActive( bool active );

    bool useExpression() const;
    void setUseExpression( bool use );

    QString expressionString() const;
    void setExpressionString( const QString& expr );


    //QMap<QString, QVariant> expressionParams() const;

    //void setExpressionParams( QMap<QString, QVariant> params );
    void insertExpressionParam( QString key, QVariant param );

    bool prepareExpression( QgsVectorLayer* layer );
    bool expressionIsPrepared() const;

    QgsExpression* expression();
    QStringList referencedColumns( QgsVectorLayer* layer );

    QString field() const;
    void setField( const QString& field );

    //QMap< QString, QString > toMap();

};
