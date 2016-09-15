/** \ingroup gui
 * Base widget allowing to edit a collection, using a table.
 * @note added in QGIS 3.0
 */
class QgsTableWidgetBase: public QWidget
{
%TypeHeaderCode
#include "qgstablewidgetbase.h"
%End
  public:
    /**
     * Constructor.
     */
    explicit QgsTableWidgetBase( QWidget* parent );

  protected:
    /**
     * Initialise the table with the given model.
     * Must be called once in the child class' constructor.
     */
    void init( QAbstractTableModel* model );

  signals:
    /**
     * Emitted each time a key or a value is changed.
     */
    void valueChanged();
};
