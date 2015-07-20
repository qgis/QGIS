#ifndef QGSFIELDUIPROPERTIES_H
#define QGSFIELDUIPROPERTIES_H

#include "qgsfeature.h"
#include "qgscondtionalstyle.h"

/** \class QgsFieldUIProperties
 *
 */
class CORE_EXPORT QgsFieldUIProperties
{
  public:
    QgsFieldUIProperties();
    void setConditionalStyles( QList<QgsConditionalStyle> styles );

    QList<QgsConditionalStyle> getConditionalStyles();
    QgsConditionalStyle matchingConditionalStyle( QString fieldName, QgsFeature* feature, QgsFields fields );

  private:
    QList<QgsConditionalStyle> mStyles;
};

#endif // QGSFIELDUIPROPERTIES_H
