/** \class QgsFieldUIProperties
 *  \ingroup core
 * Holds extra UI properties for a field.
 *
 * Currently this object holds informations about condtional styles but in future will hold
 * things like field widgets, etc
 */
class QgsFieldUIProperties
{
%TypeHeaderCode
#include <qgsfielduiproperties.h>
%End
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
    const QList<QgsConditionalStyle> getConditionalStyles();

    /**
     * @brief Find and return the matching style for the value and feature.
     * If no match is found a invalid QgsCondtionalStyle is return.
     *
     * @return A condtional style that matches the value and feature.
     * Check with QgsCondtionalStyle::isValid()
     */
    const QgsConditionalStyle matchingConditionalStyle( QVariant value, QgsFeature* feature );

    /** Reads field ui properties specific state from Dom node.
     */
    virtual bool readXml( const QDomNode& node );

    /** Write field ui properties specific state from Dom node.
     */
    virtual bool writeXml( QDomNode & node, QDomDocument & doc );

};
