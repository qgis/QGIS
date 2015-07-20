#ifndef QGSCONDTIONALSTYLE_H
#define QGSCONDTIONALSTYLE_H

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
    QgsConditionalStyle();
    QgsConditionalStyle( QString rule );
    bool matchForFeature( QString fieldName, QgsFeature *feature, QgsFields fields );
    bool matchForValue( QVariant value );
    QPixmap renderPreview();

    void setRule( QString value ) { mRule = value; mValid = true; }
    void setBackgroundColor( QColor value ) { mBackColor = value; mValid = true; }
    void setTextColor( QColor value ) { mTextColor = value; mValid = true; }
    void setFont( QFont value ) { mFont = value; mValid = true; }
    void setIcon( QPixmap value ) { mIcon = value; mValid = true; }

    QPixmap icon() { return mIcon; }
    QColor textColor() { return mTextColor; }
    QColor backgroundColor() { return mBackColor; }
    QFont font() { return mFont; }
    QString rule() { return mRule; }
    bool isValid() { return mValid; }
  private:
    bool mValid;
    QString mRule;
    QFont mFont;
    QColor mBackColor;
    QColor mTextColor;
    QPixmap mIcon;
};

#endif // QGSCONDTIONALSTYLE_H
