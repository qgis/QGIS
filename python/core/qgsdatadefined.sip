/** \ingroup core
 * \class QgsDataDefined
 * A container class for data source field mapping or expression.
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

    /**
     * Construct a new data defined object, analysing the expression to determine
     * if it's a simple field reference or an expression.
     * @param expression can be null
     */
    explicit QgsDataDefined( const QgsExpression * expression );

    /**
     * Construct a new data defined object, analysing the string to determine
     * if it's a simple field reference or an expression
     * @param string field reference or an expression, can be empty
     * @note added in QGIS 2.9
     */
    explicit QgsDataDefined( const QString& string );

    /**
     * Copy constructor. Note that copies of data defined objects with expressions
     * will not be prepared.
     */
    QgsDataDefined( const QgsDataDefined& other );

    virtual ~QgsDataDefined();

    /**Returns whether the data defined container is set to all the default
     * values, ie, disabled, with empty expression and no assigned field
     * @returns true if data defined container is set to default values
     * @note added in QGIS 2.7
     */
    bool hasDefaultValues() const;

    bool isActive() const;
    void setActive( bool active );

    bool useExpression() const;
    void setUseExpression( bool use );

    QString expressionString() const;
    void setExpressionString( const QString& expr );

    // @note not available in python bindings
    //QMap<QString, QVariant> expressionParams() const;
    // @note not available in python bindings
    //void setExpressionParams( QMap<QString, QVariant> params );
    void insertExpressionParam( QString key, QVariant param );

    /** Prepares the expression using a vector layer
     * @param layer vector layer
     * @returns true if expression was successfully prepared
     */
    bool prepareExpression( QgsVectorLayer* layer );

    /** Prepares the expression using a fields collection
     * @param fields
     * @returns true if expression was successfully prepared
     * @note added in QGIS 2.9
     */
    bool prepareExpression( const QgsFields &fields );

    /** Returns whether the data defined object's expression is prepared
     * @returns true if expression is prepared
     */
    bool expressionIsPrepared() const;

    QgsExpression* expression();
    QStringList referencedColumns( QgsVectorLayer* layer );

    QString field() const;
    void setField( const QString& field );

    // @note not available in python bindings
    //QMap< QString, QString > toMap();

    /**Returns a DOM element containing the properties of the data defined container.
     * @param document DOM document
     * @param elementName name for DOM element
     * @returns DOM element corresponding to data defined container
     * @note added in QGIS 2.7
     * @see setFromXmlElement
     */
    QDomElement toXmlElement( QDomDocument &document, const QString &elementName ) const;

    /**Sets the properties of the data defined container from an XML element. Calling
     * this will overwrite all the current properties of the container.
     * @param element DOM element
     * @returns true if properties were successfully read from element
     * @note added in QGIS 2.7
     * @see toXmlElement
     */
    bool setFromXmlElement( const QDomElement& element );

    bool operator==( const QgsDataDefined &other ) const;
    bool operator!=( const QgsDataDefined &other ) const;

};
