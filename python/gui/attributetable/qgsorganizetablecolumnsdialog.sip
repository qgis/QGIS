/** /class QgsOrganizeTableColumnsDialog
 * /ingroup gui
 * Dialog for organising (hiding and reordering) columns in the attributes table.
 * /note added in QGIS 2.16
 */

class QgsOrganizeTableColumnsDialog : QDialog
{
%TypeHeaderCode
#include <qgsorganizetablecolumnsdialog.h>
%End

  public:
    /**
     * Constructor
     * @param vl The concerned vector layer
     * @param visible the current list of visible fields name
     * @param parent parent object
     * @param flags window flags
     */
    QgsOrganizeTableColumnsDialog(const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = nullptr, Qt::WindowFlags flags = Qt::Window );

    /**
     * Destructor
     */
    ~QgsOrganizeTableColumnsDialog();

    /**
     * Get the updated configuration
     */
    QgsAttributeTableConfig config() const;

  public slots:
    /**
     * showAll checks all the  fields to show them all in the attribute table
     */
    void showAll();

    /**
     * hideAll unchecks all the fields to hide them all in the attribute table
     */
    void hideAll();
};
