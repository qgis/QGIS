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
     * @param feature The feature to match the values from.
     * @return True of the rule matches against the given feature
     */
    bool matches( QVariant value, QgsFeature *feature = 0 );

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
     * @brief Set the icon for the style
     * @param value QIcon for style
     */
    void setIcon( QPixmap value );

    /**
     * @brief The icon set for style
     * @return A QPixmap that was set for the icon
     */
    QPixmap icon();

    /**
     * @brief The text color set for style
     * @return QColor for text color
     */
    QColor textColor();

    /**
     * @brief The background color for style
     * @return QColor for background color
     */
    QColor backgroundColor();
    /**
     * @brief The font for the style
     * @return QFont for the style
     */
    QFont font();

    /**
     * @brief The condtion rule set for the style. Rule may contain variable @value
     * to represent the current value
     * @return QString of the current set rule
     */
    QString rule();
    /**
     * @brief isValid Check if this rule is valid.  A valid rule has one or more properties
     * set.
     * @return True if the rule is valid.
     */
    bool isValid();
};
