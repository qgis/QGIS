class QgsProviderCountCalcEvent : QEvent
{
%TypeHeaderCode
#include <qgsprovidercountcalcevent.h>
%End
  public:

    QgsProviderCountCalcEvent( long featuresCounted );

    long featuresCounted() const;
};
