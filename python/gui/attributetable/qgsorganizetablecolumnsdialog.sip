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
    QgsOrganizeTableColumnsDialog(const QgsVectorLayer* vl, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::Window );

    /**
     * Destructor
     */
    ~QgsOrganizeTableColumnsDialog();

    /**
     * Get the updated configuration
     */
    QgsAttributeTableConfig config() const;
};
