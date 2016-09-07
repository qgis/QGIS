/** \ingroup gui
 * Widget allowing to edit a QVariantList, using a table.
 * @note added in QGIS 3.0
 */
class QgsListWidget : public QgsTableWidgetBase
{
%TypeHeaderCode
#include "qgslistwidget.h"
%End
  public:
    /**
     * Constructor.
     */
    explicit QgsListWidget( QVariant::Type subType, QWidget* parent = nullptr );

    /**
     * Set the initial value of the widget.
     */
    void setList( const QVariantList& list );

    /**
     * Get the edit value.
     * @return the QVariantList
     */
    QVariantList list() const;

    /**
     * Check the content is valid
     * @return true if valid
     */
    bool valid() const;
};
