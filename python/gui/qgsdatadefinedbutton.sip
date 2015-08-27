/** \ingroup gui
 * \class QgsDataDefinedAssistant
 * An assistant (wizard) dialog, accessible from a QgsDataDefinedButton.
 * Can be used to guide users through creation of an expression for the
 * data defined button.
 * @note added in 2.10
 */
class QgsDataDefinedAssistant: QDialog
{
%TypeHeaderCode
#include <qgsdatadefinedbutton.h>
%End
  public:
    virtual QgsDataDefined dataDefined() const = 0;
};

/** \ingroup gui
 * \class QgsDataDefinedButton
 * A button for defining data source field mappings or expressions.
 */
class QgsDataDefinedButton : QToolButton
{
%TypeHeaderCode
#include <qgsdatadefinedbutton.h>
%End
  public:
    enum DataType
    {
      String,
      Int,
      Double,
      AnyType
    };
    typedef QFlags<QgsDataDefinedButton::DataType> DataTypes;

    /**
     * Construct a new data defined button
     *
     * @param parent The parent QWidget
     * @param vl Pointer to the associated vector layer
     * @param datadefined Data defined property
     * @param datatypes The expected data types to be compared against the variant type of the QgsField from data source and expression result
     * @param description The description of expected input data
     */
    QgsDataDefinedButton( QWidget* parent /TransferThis/ = 0,
                          const QgsVectorLayer* vl = 0,
                          const QgsDataDefined* datadefined = 0,
                          DataTypes datatypes = AnyType,
                          QString description = "" );
    ~QgsDataDefinedButton();

    /**
     * Initialize a newly constructed data defined button (useful if button already included from form layout)
     *
     * @param vl Pointer to the associated vector layer
     * @param datadefined Data defined property
     * @param datatypes The expected data types to be compared against the variant type of the QgsField from data source and expression result
     * @param description The description of expected input data
     */
    void init( const QgsVectorLayer* vl,
               const QgsDataDefined* datadefined = 0,
               DataTypes datatypes = AnyType,
               QString description = QString( "" ) );

    QMap< QString, QString > definedProperty() const;

    /** Updates a QgsDataDefined with the current settings from the button
     * @param dd QgsDataDefined to update
     * @note added in QGIS 2.9
     */
    void updateDataDefined( QgsDataDefined* dd ) const;

    /** Returns a QgsDataDefined which reflects the current settings from the
     * button.
     * @note added in QGIS 2.9
     * @see updateDataDefined
     */
    QgsDataDefined currentDataDefined() const;

    /**
     * Whether the current data definition or expression is to be used
     */
    bool isActive() const;

    /**
     * Whether the current expression is to be used instead of field mapping
     */
    bool useExpression() const;

    /**
     * The current defined expression
     */
    QString getExpression() const;

    /**
     * The current defined field
     */
    QString getField() const;

    /**
     * The current definition
     * @returns empty QString if not active, otherwise currently defined expression or field name
     */
    QString currentDefinition() const;

    /**
     * The valid data types that will work for the definition (QVariant-coercible to expected type)
     * Compared against the variant type of the QgsField from data source and expression result
     */
    const DataTypes& validDataTypes() const;

    /**
     * The full definition description and current definition (internally generated on a contextual basis)
     */
    QString fullDescription() const;

    /**
     * The usage information about this data definition
     */
    QString usageInfo() const;

    /**
     * Set the usage information about this data definition
     */
    void setUsageInfo( const QString& info );

    /**
     * Register list of sibling widgets that get disabled/enabled when data definition or expression is set/unset
     */
    void registerEnabledWidgets( QList<QWidget*> wdgts );

    /**
     * Register a sibling widget that gets disabled/enabled when data definition or expression is set/unset
     */
    void registerEnabledWidget( QWidget* wdgt );

    /**
     * Return widget siblings that get disabled/enabled when data definition or expression is set/unset
     *
     * @return unguarded pointers from guarded ones
     */
    QList<QWidget*> registeredEnabledWidgets();

    /**
     * Clears list of sibling widgets
     */
    void clearEnabledWidgets();

    /**
     * Register list of sibling widgets that get checked when data definition or expression is active
     */
    void registerCheckedWidgets( QList<QWidget*> wdgts );

    /**
     * Register a sibling widget that get checked when data definition or expression is active
     */
    void registerCheckedWidget( QWidget* wdgt );

    /**
     * Return widget siblings that get checked when data definition or expression is active
     *
     * @return unguarded pointers from guarded ones
     */
    QList<QWidget*> registeredCheckedWidgets();

    /**
     * Clears list of checkable sibling widgets
     */
    void clearCheckedWidgets();

    //! Callback function for retrieving the expression context for the button
    //typedef QgsExpressionContext( *ExpressionContextCallback )( const void* context );

    /** Register callback function for retrieving the expression context for the button
     * @param fnGetExpressionContext call back function, will be called when the data defined
     * button requires the current expression context
     * @param context context for callback function
     * @note added in QGIS 2.12
     * @note not available in Python bindings
     */
    //void registerGetExpressionContextCallback( ExpressionContextCallback fnGetExpressionContext, const void* context );

    /**
     * Sets an assistant used to define the data defined object properties.
     * Ownership of the assistant is transferred to the widget.
     * @param title menu title for the assistant
     * @param assistant data defined assistant. Set to null to remove the assistant
     * option from the button.
     * @note added in 2.10
     */
    void setAssistant( const QString& title, QgsDataDefinedAssistant * assistant /Transfer/ );

    /**
     * Common descriptions for expected input values
     */
    static QString trString();
    static QString charDesc();
    static QString boolDesc();
    static QString anyStringDesc();
    static QString intDesc();
    static QString intPosDesc();
    static QString intPosOneDesc();
    static QString doubleDesc();
    static QString doublePosDesc();
    static QString double0to1Desc();
    static QString doubleXYDesc();
    static QString double180RotDesc();
    static QString intTranspDesc();
    static QString unitsMmMuDesc();
    static QString unitsMmMuPercentDesc();
    static QString colorNoAlphaDesc();
    static QString colorAlphaDesc();
    static QString textHorzAlignDesc();
    static QString textVertAlignDesc();
    static QString penJoinStyleDesc();
    static QString blendModesDesc();
    static QString svgPathDesc();
    static QString filePathDesc();
    static QString paperSizeDesc();
    static QString paperOrientationDesc();
    static QString horizontalAnchorDesc();
    static QString verticalAnchorDesc();
    static QString gradientTypeDesc();
    static QString gradientCoordModeDesc();
    static QString gradientSpreadDesc();
    static QString lineStyleDesc();
    static QString capStyleDesc();
    static QString fillStyleDesc();
    static QString markerStyleDesc();
    static QString customDashDesc();

  public slots:
    /**
     * Set whether the current data definition or expression is to be used
     */
    void setActive( bool active );

    /**
     * Set siblings' enabled property when data definition or expression is set/unset
     */
    void disableEnabledWidgets( bool disable );

    /**
     * Set siblings' checked property when data definition or expression is active
     */
    void checkCheckedWidgets( bool check );

  signals:
    /**
     * Emitted when data definition or expression is changed
     * @param definition The current definition or expression (empty string if inactive)
     */
    void dataDefinedChanged( const QString& definition );

    /**
     * Emitted when active state changed
     * @param active Whether the definition is active
     */
    void dataDefinedActivated( bool active );

  protected:
    void mouseReleaseEvent( QMouseEvent *event );

    /**
     * Set whether the current expression is to be used instead of field mapping
     */
    void setUseExpression( bool use );

    /**
     * Set the current defined expression
     */
    void setExpression( QString exp );

    /**
     * Set the current defined field
     */
    void setField( QString field );

};

QFlags<QgsDataDefinedButton::DataType> operator|(QgsDataDefinedButton::DataType f1, QFlags<QgsDataDefinedButton::DataType> f2);
