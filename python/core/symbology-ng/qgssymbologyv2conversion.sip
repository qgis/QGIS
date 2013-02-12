class QgsSymbologyV2Conversion
{
%TypeHeaderCode
#include <qgssymbologyv2conversion.h>
%End

  public:
    //! return a symbol in new symbology as close as possible to old symbol
    //! @note not all properties will be preserved
    static QgsSymbolV2* symbolV1toV2( const QgsSymbol* s ) /Factory/;

    //! return a symbol in old symbology as close as possible to new symbol
    //! @note not all properties will be preserved
    static QgsSymbol* symbolV2toV1( QgsSymbolV2* s ) /Factory/;

    //! convert layer from old symbology to new symbology
    //! @note not all properties will be preserved
    static void rendererV1toV2( QgsVectorLayer* layer );

    //! convert layer from new symbology to old symbology
    //! @note not all properties will be preserved
    static void rendererV2toV1( QgsVectorLayer* layer );

};
