typedef QList<QgsConditionalStyle> QgsConditionalStyles;


/**
 * @brief The QgsConditionalLayerStyles class holds conditional style information
 * for a layer. This includes field styles and full row styles.
 */
class QgsConditionalLayerStyles
{
%TypeHeaderCode
#include <qgsconditionalstyle.h>
%End
  public:
    QgsConditionalLayerStyles();

    QList<QgsConditionalStyle> rowStyles();

    /**
     * @brief Set the conditional styles that apply to full rows of data in the attribute table.
     * Each row will check be checked against each rule.
     * @param styles The styles to assign to all the rows
     * @note added in QGIS 2.12
     */
    void setRowStyles( const QList<QgsConditionalStyle>& styles );

    /**
     * @brief Set the conditional styles for the field UI properties.
     * @param fieldName name of field
     * @param styles
     */
    void setFieldStyles( const QString& fieldName, const QList<QgsConditionalStyle>& styles );

    /**
     * @brief Returns the conditional styles set for the field UI properties
     * @return A list of conditional styles that have been set.
     */
    QList<QgsConditionalStyle> fieldStyles( const QString& fieldName );

    /** Reads field ui properties specific state from Dom node.
     */
    bool readXml( const QDomNode& node );

    /** Write field ui properties specific state from Dom node.
     */
    bool writeXml( QDomNode & node, QDomDocument & doc ) const;
};

/** \class QgsConditionalStyle
 *  \ingroup core
 * Conditional styling for a rule.
 */
class QgsConditionalStyle
{
%TypeHeaderCode
#include <qgsconditionalstyle.h>
%End
  public:
    QgsConditionalStyle();
    QgsConditionalStyle( const QgsConditionalStyle& other );
    QgsConditionalStyle( const QString& rule );
    ~QgsConditionalStyle();

    /**
     * @brief Check if the rule matches using the given value and feature
     * @param value The current value being checked. The "value" variable from the context is replaced with this value.
     * @param context Expression context for evaluating rule expression
     * @return True of the rule matches against the given feature
     */
    bool matches( const QVariant& value, QgsExpressionContext& context ) const;

    /**
     * @brief Render a preview icon of the rule.
     * @return QPixmap preview of the style
     */
    QPixmap renderPreview() const;

    /**
     * @brief Set the name of the style.  Names are optional but handy for display
     * @param value The name given to the style
     */
    void setName( const QString& value );

    /**
     * @brief Set the rule for the style.  Rules should be of QgsExpression syntax.
     * Special value of \@value is replaced at run time with the check value
     * @param value The QgsExpression style rule to use for this style
     */
    void setRule( const QString& value );

    /**
     * @brief Set the background color for the style
     * @param value QColor for background color
     */
    void setBackgroundColor( const QColor& value );

    /**
     * @brief Set the text color for the style
     * @param value QColor for text color
     */
    void setTextColor( const QColor& value );

    /**
     * @brief Set the font for the the style
     * @param value QFont to be used for text
     */
    void setFont( const QFont& value );

    /**
     * @brief Set the icon for the style. Icons are generated from symbols
     * @param value QgsSymbolV2 to be used when generating the icon
     */
    void setSymbol( QgsSymbolV2* value );

    /**
     * @brief The name of the style.
     * @return The name of the style. Names are optional so might be empty.
     */
    QString displayText() const;

    /**
     * @brief The name of the style.
     * @return The name of the style. Names are optional so might be empty.
     */
    QString name() const;

    /**
     * @brief The icon set for style generated from the set symbol
     * @return A QPixmap that was set for the icon using the symbol
     */
    QPixmap icon() const;

    /**
     * @brief The symbol used to generate the icon for the style
     * @return The QgsSymbolV2 used for the icon
     */
    QgsSymbolV2* symbol() const;

    /**
     * @brief The text color set for style
     * @return QColor for text color
     */
    QColor textColor() const;

    /**
     * @brief Check if the text color is valid for render.
     * Valid colors are non invalid QColors and a color with a > 0 alpha
     * @return True of the color set for text is valid.
     */
    bool validTextColor() const;

    /**
     * @brief The background color for style
     * @return QColor for background color
     */
    QColor backgroundColor() const;

    /**
     * @brief Check if the background color is valid for render.
     * Valid colors are non invalid QColors and a color with a > 0 alpha
     * @return True of the color set for background is valid.
     */
    bool validBackgroundColor() const;
    /**
     * @brief The font for the style
     * @return QFont for the style
     */
    QFont font() const;

    /**
     * @brief The condition rule set for the style. Rule may contain variable \@value
     * to represent the current value
     * @return QString of the current set rule
     */
    QString rule() const;

    /**
     * @brief isValid Check if this rule is valid.  A valid rule has one or more properties
     * set.
     * @return True if the rule is valid.
     */
    bool isValid() const;

    /**
     * @brief Find and return the matching styles for the value and feature.
     * If no match is found a invalid QgsConditionalStyle is return.
     *
     * @return A condtional style that matches the value and feature.
     * Check with QgsConditionalStyle::isValid()
     */
    static QList<QgsConditionalStyle> matchingConditionalStyles( const QList<QgsConditionalStyle>& styles, const QVariant& value, QgsExpressionContext& context );

    /**
     * @brief Find and return the matching style for the value and feature.
     * If no match is found a invalid QgsConditionalStyle is return.
     *
     * @return A condtional style that matches the value and feature.
     * Check with QgsConditionalStyle::isValid()
     */
    static QgsConditionalStyle matchingConditionalStyle( const QList<QgsConditionalStyle>& styles, const QVariant& value, QgsExpressionContext& context );

    /**
     * @brief Compress a list of styles into a single style.  This can be used to stack the elements of the
     * styles. The font of the last style is used in the output.
     * @param styles The list of styles to compress down
     * @return A single style generated from joining each style property.
     */
    static QgsConditionalStyle compressStyles( const QList<QgsConditionalStyle>& styles );

    /** Reads vector conditional style specific state from layer Dom node.
     */
    bool readXml( const QDomNode& node );

    /** Write vector conditional style specific state from layer Dom node.
     */
    bool writeXml( QDomNode & node, QDomDocument & doc ) const;
};
