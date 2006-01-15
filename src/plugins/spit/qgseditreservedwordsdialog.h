#ifndef QGSEDITRESERVEDWORDSDIALOG_H
#define QGSEDITRESERVEDWORDSDIALOG_H
#include "ui_qgseditreservedwordsbase.h"
#include "qgisgui.h"
class QgsEditReservedWordsDialog : public QDialog, private Ui::QgsEditReservedWordsBase
{
    Q_OBJECT
public:
    QgsEditReservedWordsDialog(QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags);
    ~QgsEditReservedWordsDialog();
    void addColumn(QString column, bool isReserved, int index);
    void setReservedWords(const QStringList &);
    QStringList columnNames();
    //! Set the description displayed in the dialog
    void setDescription(const QString &description);
public slots:
    void checkWord(Q3ListViewItem *, int , const QString&);
    void editWord(Q3ListViewItem *);



};
#endif //QGSEDITRESERVEDWORDSDIALOG_H
