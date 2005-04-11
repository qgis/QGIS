//
// C++ Interface: qgscustomprojectiondialog
//
// Description: 
//
//
// Author: Tim Sutton tim@linfiniti.com, (C) 2005
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef QGSCUSTOMPROJECTIONDIALOG_H
#define QGSCUSTOMPROJECTIONDIALOG_H

#include <qdir.h>
#include <qnetworkprotocol.h> 
#include <qgscustomprojectiondialogbase.uic.h>

/**
The custom projection widget is used to define the projection family, ellipsoid and paremters needed by proj4 to assemble a customised projection definition. The resulting projection will be store in an sqlite backend.

@author Tim Sutton
*/
class QgsCustomProjectionDialog : public QgsCustomProjectionDialogBase
{
Q_OBJECT
public:
    QgsCustomProjectionDialog(QWidget* parent , const char* name = "", WFlags fl=0);
    ~QgsCustomProjectionDialog();
    //a recursive function to make a directory and its ancestors
    bool makeDir(QDir &theQDir);
public slots:    
    void pbnHelp_clicked();
    void pbnOK_clicked();
    void pbnApply_clicked();
    void pbnCancel_clicked();
    void cboProjectionFamily_textChanged( const QString & );
};

#endif
