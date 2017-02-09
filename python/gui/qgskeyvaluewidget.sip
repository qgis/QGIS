/** \ingroup gui
 * Widget allowing to edit a QVariantMap, using a table.
 * @note added in QGIS 3.0
 */
class QgsKeyValueWidget : public QgsTableWidgetBase
{
%TypeHeaderCode
#include "qgskeyvaluewidget.h"
%End
  public:
    /**
     * Constructor.
     */
    explicit QgsKeyValueWidget( QWidget* parent /TransferThis/ = 0 );

    /**
     * Set the initial value of the widget.
     */
    void setMap( const QVariantMap& map );

    /**
     * Get the edit value.
     * @return the QVariantMap
     */
    QVariantMap map() const;

  signals:
    /**
     * Emitted each time a key or a value is changed.
     */
    void valueChanged();
};
