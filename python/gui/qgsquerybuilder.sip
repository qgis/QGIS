class QgsQueryBuilder : QDialog
{
%TypeHeaderCode
#include <qgsquerybuilder.h>
%End

  public:
    QgsQueryBuilder( QgsVectorLayer *layer, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsQueryBuilder();

    QString sql();
    void setSql( QString sqlStatement );

    void setDatasourceDescription( QString uri );
};
