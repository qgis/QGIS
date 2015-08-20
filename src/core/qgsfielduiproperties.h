#ifndef QGSFIELDUIPROPERTIES_H
#define QGSFIELDUIPROPERTIES_H

#include <QDomDocument>
#include <QDomNode>

#include "qgsfeature.h"
#include "qgsconditionalstyle.h"

/** \class QgsFieldUIProperties
 * Holds extra UI properties for a field.
 *
 * Currently this object holds informations about condtional styles but in future will hold
 * things like field widgets, etc
 *
 * TODO Move UI field related stuff from QgsVectorLayer here
 */
class CORE_EXPORT QgsFieldUIProperties
{
  public:
    QgsFieldUIProperties();

    /**
     * @brief Set the condtional styles for the field UI properties.
     * @param styles
     */
    void setConditionalStyles( QList<QgsConditionalStyle> styles );

    /**
     * @brief Returns the condtional styles set for the field UI properties
     * @return A list of condtional styles that have been set.
     */
    QList<QgsConditionalStyle> getConditionalStyles() const;

    /**
     * @brief Find and return all matching styles for a value and context.
     * If no match is found an empty list is returned.
     * @param value current cell value
     * @param context expression context for evaluating conditional rules
     * @return A list of conditional styles that matches the value and context.
     * @see matchingConditionalStyle
     */
    QList<QgsConditionalStyle> matchingConditionalStyles(QVariant value, QgsExpressionContext& context ) const;

    /**
     * @brief Find and return the matching style for the value and context.
     * If no match is found a invalid QgsConditionalStyle is return.
     * @param value current cell value
     * @param context expression context for evaluating conditional rules
     * @return A conditional style that matches the value and context.
     * Check with QgsConditionalStyle::isValid()
     * @see matchingConditionalStyles
     */
    QgsConditionalStyle matchingConditionalStyle( QVariant value, QgsExpressionContext& context ) const;

    /** Reads field ui properties specific state from Dom node.
     */
    virtual bool readXml( const QDomNode& node );

    /** Write field ui properties specific state from Dom node.
     */
    virtual bool writeXml( QDomNode & node, QDomDocument & doc ) const;

  private:
    QList<QgsConditionalStyle> mStyles;
};

#endif // QGSFIELDUIPROPERTIES_H
