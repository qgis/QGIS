/** \ingroup core
 * A linear enhanceContrast enhancement that first clips to min max and then enhanceContrastes
 * linearly between min and max.
 */
class QgsLinearMinMaxEnhancementWithClip : QgsContrastEnhancementFunction
{
%TypeHeaderCode
#include <qgslinearminmaxenhancementwithclip.h>
%End

  public:
    QgsLinearMinMaxEnhancementWithClip( QGis::DataType, double, double );

    int enhance( double );

    bool isValueInDisplayableRange( double );
};
