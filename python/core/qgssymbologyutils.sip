
/**Namespace containing static methods which are useful for the symbology widgets*/
namespace QgsSymbologyUtils
{
%TypeHeaderCode
#include <qgssymbologyutils.h>
%End

    QPixmap qString2LinePixmap(QString string);
    QPixmap char2LinePixmap(const char* c);
    QPixmap qString2PatternPixmap(QString string);
    QPixmap char2PatternPixmap(const char* c);
    QString penStyle2QString(Qt::PenStyle penstyle);
    const char* penStyle2Char(Qt::PenStyle penstyle);
    QPixmap penStyle2Pixmap(Qt::PenStyle penstyle);
    Qt::PenStyle qString2PenStyle(QString string);
    Qt::PenStyle char2PenStyle(const char* c);
    QString brushStyle2QString(Qt::BrushStyle brushstyle);
    const char* brushStyle2Char(Qt::BrushStyle brushstyle);
    QPixmap brushStyle2Pixmap(Qt::BrushStyle brushstyle);
    Qt::BrushStyle qString2BrushStyle(QString string);
    Qt::BrushStyle char2BrushStyle(const char* c);
};

