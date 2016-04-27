class QgsOrganizeTableColumnsDialog : QDialog
{
%TypeHeaderCode
#include <qgsorganizetablecolumnsdialog.h>
%End

  public:
    /**
     * Constructor
     * @param vl The concerned vector layer
     * @param visible the cuurent list of visible fields name
     * @param parent parent object
     * @param flags window flags
     */
    QgsOrganizeTableColumnsDialog( const QgsVectorLayer* vl, QStringList visible, QWidget *parent /TransferThis/ = nullptr, Qt::WindowFlags flags = Qt::Window );
    ~QgsOrganizeTableColumnsDialog();

    /**
     * Get the selected fields name
     * @return The selected fields name
     */
    QStringList selectedFields();
};
