#ifndef QGSLINESTYLEDIALOG_H
#define QGSLINESTYLEDIALOG_H

class qnamespace;
#include "qgslinestyledialogbase.h"

/**Dialog class to query line styles*/
class QgsLineStyleDialog: public QgsLineStyleDialogBase
{
  Q_OBJECT
 public:
    QgsLineStyleDialog(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0);
    ~QgsLineStyleDialog();
    Qt::PenStyle style();
 protected:
    Qt::PenStyle m_style;
 protected slots:
     /**Queries the selected style if the ok button is pressed and stores it in m_style*/
     void queryStyle();
};

inline QgsLineStyleDialog::~QgsLineStyleDialog()
{

}

#endif
