class QgsSizeScaleWidget : QgsDataDefinedAssistant
{
%TypeHeaderCode
#include <qgssizescalewidget.h>
%End
  public:
    QgsSizeScaleWidget( const QgsVectorLayer * layer, const QgsMarkerSymbol * symbol );

    virtual QgsAbstractProperty* property() const /Factory/;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* layer() const;

  protected:

    virtual void showEvent( QShowEvent * );
};

