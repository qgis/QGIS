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
    QList<QgsConditionalStyle> conditionalStyles();

    /** Reads field ui properties specific state from Dom node.
     */
    virtual bool readXml( const QDomNode& node );

    /** Write field ui properties specific state from Dom node.
     */
    virtual bool writeXml( QDomNode & node, QDomDocument & doc );

  private:
    QList<QgsConditionalStyle> mStyles;
};

#endif // QGSFIELDUIPROPERTIES_H
