#ifndef QGSFIELDUIPROPERTIES_H
#define QGSFIELDUIPROPERTIES_H

#include "qgsfeature.h"
#include "qgsconditionalstyle.h"

/** \class QgsFieldUIProperties
 *
 */
class CORE_EXPORT QgsFieldUIProperties
{
  public:
    QgsFieldUIProperties();
    void setConditionalStyles( QList<QgsConditionalStyle> styles );

    QList<QgsConditionalStyle> getConditionalStyles();
    QgsConditionalStyle matchingConditionalStyle( QgsFeature* feature, QgsFields fields );

  private:
    QList<QgsConditionalStyle> mStyles;
};

#endif // QGSFIELDUIPROPERTIES_H
