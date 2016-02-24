class QgsFilterTableFieldsDialog : QDialog
{
%TypeHeaderCode
#include <qgsfiltertablefieldsdialog.h>
%End

  public:
    /**
     * Constructor
     * @param vl The concerned vector layer
     * @param visible the cuurent list of visible fields name
     * @param parent parent object
     * @param flags window flags
     */
    QgsFilterTableFieldsDialog( const QgsVectorLayer* vl, QStringList visible, QWidget *parent /TransferThis/ = nullptr, Qt::WindowFlags flags = Qt::Window );
    ~QgsFilterTableFieldsDialog();

    /**
     * Get the selected fields name
     * @return The selected fields name
     */
    QStringList selectedFields();
};
