#ifndef QGSCONTCOLDIALOG_H
#define QGSCONTCOLDIALOG_H

#include "qgscontcoldialogbase.h"
#include <map>

class QgsVectorLayer;

class QgsContColDialog: public QgsContColDialogBase
{
    Q_OBJECT
 public: 
    QgsContColDialog(QgsVectorLayer* layer);
    ~QgsContColDialog();

 protected slots:
    void apply();
    void selectMinimumColor();
    void selectMaximumColor();
 protected:
    QgsVectorLayer* m_vectorlayer;
    /**Stores the names and numbers of the fields with numeric values*/
     std::map<QString,int> m_fieldmap;
 private:
    QgsContColDialog();
};

#endif
