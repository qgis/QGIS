class QgsSizeScaleWidget : QgsDataDefinedAssistant
{
%TypeHeaderCode
#include <qgssizescalewidget.h>
%End
  public:
    QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsMarkerSymbolV2 * symbol );

    QgsDataDefined dataDefined() const;

  protected:

    virtual void showEvent( QShowEvent * );
};

