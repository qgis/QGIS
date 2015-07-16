#ifndef QGSFIELDUIPROPERTIES_H
#define QGSFIELDUIPROPERTIES_H

#include <QFont>
#include <QColor>
#include <QPixmap>

#include "qgsfeature.h"

/** \class QgsFieldFormat
 * Hold formatting rules and style information for a field
 */
class CORE_EXPORT QgsConditionalStyle
{
  public:
    bool matchForFeature( QString fieldName, QgsFeature *feature, QgsFields fields );
    bool matchForValue( QVariant value );
    QPixmap renderPreview();
    QString rule;
    QFont font;
    QColor backColor;
    QColor textColor;
    QPixmap icon;
};

/** \class QgsFieldUIProperties
 *
 */
class CORE_EXPORT QgsFieldUIProperties
{
  public:
    QgsFieldUIProperties();
    void setConditionalStyles( QList<QgsConditionalStyle> styles );

    QList<QgsConditionalStyle> getConditionalStyles();
  private:
    QList<QgsConditionalStyle> mStyles;
};

#endif // QGSFIELDUIPROPERTIES_H
