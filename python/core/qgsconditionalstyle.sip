/** \class QgsCondtionalStyle
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
    QgsConditionalStyle( QString rule );

    /**
     * @brief Check if the rule matches using the given value and feature
     * @param value The current value being checked. The "value" variable from the context is replaced with this value.
     * @param context Expression context for evaluating rule expression
     * @return True of the rule matches against the given feature
     */
    bool matches( QVariant value, QgsExpressionContext& context ) const;
    /**
     * @brief Render a preview icon of the rule.
     * @return QPixmap preview of the style
     */
    QPixmap renderPreview();

    /**
     * @brief Set the rule for the style.  Rules should be of QgsExpression syntax.
     * Special value of @value is replaced at run time with the check value
     * @param value
     */
    void setRule( QString value );

    /**
     * @brief Set the background color for the style
     * @param value QColor for background color
     */
    void setBackgroundColor( QColor value );

    /**
     * @brief Set the text color for the style
     * @param value QColor for text color
     */
    void setTextColor( QColor value );

    /**
     * @brief Set the font for the the style
     * @param value QFont to be used for text
     */
    void setFont( QFont value );

    /**
     * @brief Set the icon for the style. Icons are generated from symbols
     * @param value QgsSymbolV2 to be used when generating the icon
     */
    void setSymbol( QgsSymbolV2* value );

    /**
     * @brief The symbol used to generate the icon for the style
     * @return The QgsSymbolV2 used for the icon
     */
    QgsSymbolV2* symbol() const;

    /**
     * @brief The icon set for style
     * @return A QPixmap that was set for the icon
     */
    QPixmap icon() const;

    /**
     * @brief The text color set for style
     * @return QColor for text color
     */
    QColor textColor() const;

    /**
     * @brief The background color for style
     * @return QColor for background color
     */
    QColor backgroundColor() const;
    /**
     * @brief The font for the style
     * @return QFont for the style
     */
    QFont font() const;

    /**
     * @brief The condtion rule set for the style. Rule may contain variable @value
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

    /** Reads vector conditional style specific state from layer Dom node.
     */
    virtual bool readXml( const QDomNode& node );

    /** Write vector conditional style specific state from layer Dom node.
     */
    virtual bool writeXml( QDomNode & node, QDomDocument & doc );
};
