class QgsSymbologyV2Conversion
{
%TypeHeaderCode
#include <qgssymbologyv2conversion.h>
%End
  public:

    /** Read old renderer definition from XML and create matching new renderer */
    static QgsFeatureRendererV2* readOldRenderer( const QDomNode& layerNode, QGis::GeometryType geomType );

    static QString penStyle2QString( Qt::PenStyle penstyle );
    static Qt::PenStyle qString2PenStyle( const QString& string );
    static QString brushStyle2QString( Qt::BrushStyle brushstyle );
    static Qt::BrushStyle qString2BrushStyle( const QString& string );
};
