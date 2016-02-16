class QgsSizeScaleWidget : QgsDataDefinedAssistant
{
%TypeHeaderCode
#include <qgssizescalewidget.h>
%End
  public:
    QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsMarkerSymbolV2 * symbol );

    QgsDataDefined dataDefined() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* layer() const;

  protected:

    virtual void showEvent( QShowEvent * );
};

