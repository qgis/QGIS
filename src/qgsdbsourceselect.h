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
    void dbConnect();
};


#endif // QGSDBSOURCESELECT_H
