class QgsSearchQueryBuilder : QDialog
{
%TypeHeaderCode
#include <qgssearchquerybuilder.h>
%End

  public:
    QgsSearchQueryBuilder( QgsVectorLayer* layer, QWidget *parent /TransferThis/ = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsSearchQueryBuilder();

    //! returns newly created search string
    QString searchString();

    //! change search string shown in text field
    void setSearchString( QString searchString );
};
