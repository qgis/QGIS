/** \ingroup core
 * \class QgsDataDefined
 * A container class for data source field mapping or expression.
 * \note QgsDataDefined objects are implicitly shared.
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

    /** Creates a QgsDataDefined from a decoded QgsStringMap.
     * @param map string map encoding of QgsDataDefined
     * @param baseName base name for values in the string map
     * @returns new QgsDataDefined if string map was successfully interpreted
     * @see toMap
     * @note added in QGIS 2.9
     */
    static QgsDataDefined* fromMap( const QgsStringMap& map, const QString& baseName = QString() );

    virtual ~QgsDataDefined();

    /** Returns whether the data defined container is set to all the default
     * values, ie, disabled, with empty expression and no assigned field
     * @returns true if data defined container is set to default values
     * @note added in QGIS 2.7
     */
    bool hasDefaultValues() const;

    bool isActive() const;
    void setActive( bool active );

    /**
     * Returns if the field or the expression part is active.
     *
     * @return True if it is in expression mode.
     */
    bool useExpression() const;

    /**
     * Controls if the field or the expression part is active.
     * For QGIS<=2.10 it is mandatory to call this after {@link setExpressionString}
     * or {@link setField}.
     *
     * @param use True if it should be set to expression mode.
     */
    void setUseExpression( bool use );

    /**
     * Returns the expression string of this QgsDataDefined.
     *
     * @return An expression
     *
     * @see field()
     * @see expressionOrField()
     */
    QString expressionString() const;

    /**
     * Sets the expression for this QgsDataDefined.
     * Will also set useExpression to true.
     *
     * @param expr The expression to set
     *
     * @see setField
     */
    void setExpressionString( const QString& expr );

    /**
     * Returns an expression which represents a single field if useExpression returns false, otherwise
     * returns the current expression string.
     * @return An expression
     *
     * @note added in 2.12
     */
    QString expressionOrField() const;

    //! @note not available in python bindings
    //QMap<QString, QVariant> expressionParams() const;
    //! @note not available in python bindings
    //void setExpressionParams( QMap<QString, QVariant> params );
    void insertExpressionParam( const QString& key, const QVariant& param );

    /** Prepares the expression using a vector layer
     * @param layer vector layer
     * @returns true if expression was successfully prepared
     * @deprecated use QgsExpressionContext variant instead
     */
    bool prepareExpression( QgsVectorLayer* layer ) /Deprecated/;

    /** Prepares the expression using a fields collection
     * @param fields
     * @returns true if expression was successfully prepared
     * @note added in QGIS 2.9
     * @deprecated use QgsExpressionContext variant instead
     */
    bool prepareExpression( const QgsFields &fields ) /Deprecated/;

    /** Prepares the expression using an expression context.
     * @param context expression context
     * @returns true if expression was successfully prepared
     * @note added in QGIS 2.12
     */
    bool prepareExpression( const QgsExpressionContext &context = QgsExpressionContext() );

    /** Returns whether the data defined object's expression is prepared
     * @returns true if expression is prepared
     */
    bool expressionIsPrepared() const;

    QgsExpression* expression();

    /** Returns the columns referenced by the QgsDataDefined
     * @param layer vector layer, used for preparing the expression if required
     * @deprecated use QgsExpressionContext variant instead
     */
    QStringList referencedColumns( QgsVectorLayer* layer ) /Deprecated/;

    /** Returns the columns referenced by the QgsDataDefined
     * @param fields vector layer, used for preparing the expression if required
     * @note added in QGIS 2.9
     * @deprecated use QgsExpressionContext variant instead
     */
    QStringList referencedColumns( const QgsFields& fields ) /Deprecated/;

    /** Returns the columns referenced by the QgsDataDefined
     * @param context expression context, used for preparing the expression if required
     * @note added in QGIS 2.12
     */
    QStringList referencedColumns( const QgsExpressionContext& context = QgsExpressionContext() );

    /**
     * Get the field which this QgsDataDefined represents. Be aware that this may return
     * a field name which may not be active if useExpression is true.
     *
     * @return A fieldname
     *
     * @see expressionOrField()
     */
    QString field() const;

    /**
     * Set the field name which this QgsDataDefined represents.
     * Will set useExpression to false.
     *
     * @param field
     */
    void setField( const QString& field );

    /** Encodes the QgsDataDefined into a string map.
     * @param baseName optional base name for values in the string map. Can be used
     * to differentiate multiple QgsDataDefineds encoded in the same string map.
     * @see fromMap
     */
    QMap< QString, QString > toMap( const QString& baseName = QString() ) const;

    /** Returns a DOM element containing the properties of the data defined container.
     * @param document DOM document
     * @param elementName name for DOM element
     * @returns DOM element corresponding to data defined container
     * @note added in QGIS 2.7
     * @see setFromXmlElement
     */
    QDomElement toXmlElement( QDomDocument &document, const QString &elementName ) const;

    /** Sets the properties of the data defined container from an XML element. Calling
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
