#ifndef QGSCONDTIONALSTYLE_H
#define QGSCONDTIONALSTYLE_H

#include <QFont>
#include <QColor>
#include <QPixmap>

#include "qgsfeature.h"

/** \class QgsFieldFormat
 * Conditional styling for a rule.
 */
class CORE_EXPORT QgsConditionalStyle
{
  public:
    QgsConditionalStyle();
    QgsConditionalStyle( QString rule );

    /**
     * @brief Check if the rule matches using the given value and feature
     * @param value The current value being checked. \@value is replaced in the rule with this value.
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
     * Special value of \@value is replaced at run time with the check value
     * @param value The QgsExpression style rule to use for this style
     */
    void setRule( QString value ) { mRule = value; mValid = true; }

    /**
     * @brief Set the background color for the style
     * @param value QColor for background color
     */
    void setBackgroundColor( QColor value ) { mBackColor = value; mValid = true; }

    /**
     * @brief Set the text color for the style
     * @param value QColor for text color
     */
    void setTextColor( QColor value ) { mTextColor = value; mValid = true; }

    /**
     * @brief Set the font for the the style
     * @param value QFont to be used for text
     */
    void setFont( QFont value ) { mFont = value; mValid = true; }

    /**
     * @brief Set the icon for the style
     * @param value QIcon for style
     */
    void setIcon( QPixmap value ) { mIcon = value; mValid = true; }

    /**
     * @brief The icon set for style
     * @return A QPixmap that was set for the icon
     */
    QPixmap icon() { return mIcon; }

    /**
     * @brief The text color set for style
     * @return QColor for text color
     */
    QColor textColor() { return mTextColor; }

    /**
     * @brief The background color for style
     * @return QColor for background color
     */
    QColor backgroundColor() { return mBackColor; }
    /**
     * @brief The font for the style
     * @return QFont for the style
     */
    QFont font() { return mFont; }

    /**
     * @brief The condtion rule set for the style. Rule may contain variable \@value
     * to represent the current value
     * @return QString of the current set rule
     */
    QString rule() { return mRule; }

    /**
     * @brief isValid Check if this rule is valid.  A valid rule has one or more properties
     * set.
     * @return True if the rule is valid.
     */
    bool isValid() { return mValid; }

    /** Reads vector layer specific state from project file Dom node.
     *  @note Called by QgsMapLayer::readXML().
     */
    virtual bool readXml( const QDomNode& layer_node ) override;

    /** Write vector layer specific state to project file Dom node.
     *  @note Called by QgsMapLayer::writeXML().
     */
    virtual bool writeXml( QDomNode & layer_node, QDomDocument & doc ) override;

  private:
    bool mValid;
    QString mRule;
    QFont mFont;
    QColor mBackColor;
    QColor mTextColor;
    QPixmap mIcon;
};

#endif // QGSCONDTIONALSTYLE_H
