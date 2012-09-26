class QgsProviderExtentCalcEvent : QEvent
{
%TypeHeaderCode
#include <qgsproviderextentcalcevent.h>
%End

  public:

    QgsProviderExtentCalcEvent( QgsRectangle* layerExtent );

    QgsRectangle* layerExtent() const;
};
