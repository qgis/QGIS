#ifndef QGSDBSOURCESELECT_H
#define QGSDBSOURCESELECT_H
#include "qgsdbsourceselectbase.h"

class QgsDbSourceSelect : public QgsDbSourceSelectBase 
{
 public:
    QgsDbSourceSelect();
    ~QgsDbSourceSelect();
    void addNewConnection();
    void editConnection();
    void addTables();
    void dbConnect();
    QStringList selectedTables();
    QString connInfo();
 private:
    QString m_connInfo;
    QStringList m_selectedTables;
};


#endif // QGSDBSOURCESELECT_H
