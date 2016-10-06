class QgsOrderByDialog : QDialog
{
%TypeHeaderCode
#include "qgsorderbydialog.h"
%End
  public:
    /**
     * Create a new order by dialog. This helps building order by structures.
     *
     * @param layer  The vector layer for which the order by should be produced
     * @param parent The parent widget, optional
     */
    QgsOrderByDialog( QgsVectorLayer* layer, QWidget* parent /TransferThis/ = nullptr );

    /**
     * Set the order by to manage
     */
    void setOrderBy( const QgsFeatureRequest::OrderBy& orderBy );

    /**
     * Get the order by defined in the dialog
     */
    QgsFeatureRequest::OrderBy orderBy();

  protected:

    bool eventFilter( QObject *obj, QEvent *e );
 };
