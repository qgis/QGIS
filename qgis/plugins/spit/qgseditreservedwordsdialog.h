#ifndef QGSEDITRESERVEDWORDSDIALOG_H
#define QGSEDITRESERVEDWORDSDIALOG_H
#include "qgseditreservedwordsbase.h"
class QgsEditReservedWordsDialog : public QgsEditReservedWordsBase
{
    Q_OBJECT
public:
    QgsEditReservedWordsDialog(QWidget *parent=0, const char *name=0);
    ~QgsEditReservedWordsDialog();
    void addColumn(QString column, bool isReserved, int index);
    void setReservedWords(const QStringList &);
    QStringList columnNames();
    //! Set the description displayed in the dialog
    void setDescription(const QString &description);
public slots:
    void checkWord(QListViewItem *, int , const QString&);
    void editWord(QListViewItem *);



};
#endif //QGSEDITRESERVEDWORDSDIALOG_H
