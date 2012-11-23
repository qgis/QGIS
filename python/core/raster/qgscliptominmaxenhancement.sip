class QgsClipToMinMaxEnhancement : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgscliptominmaxenhancement.h>
%End

  public:
    QgsClipToMinMaxEnhancement( QgsContrastEnhancement::QgsRasterDataType, double, double );

    int enhance( double );

    bool isValueInDisplayableRange( double );
};
