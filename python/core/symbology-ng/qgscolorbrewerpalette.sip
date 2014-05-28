class QgsColorBrewerPalette
{
%TypeHeaderCode
#include <qgscolorbrewerpalette.h>
%End
  public:
    static QList<QColor> listSchemeColors( QString schemeName, int colors );

    static QStringList listSchemes();

    static QList<int> listSchemeVariants( QString schemeName );

    // extracted ColorBrewer data
    static const char *brewerString;
};
