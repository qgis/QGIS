/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsgroupwmsdatadialog.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsGroupWmsDataDialog: QDialog
{

%TypeHeaderCode
#include "qgsgroupwmsdatadialog.h"
%End
  public:
    QgsGroupWmsDataDialog( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor
%End

    QString groupTitle();
%Docstring
return group WMS title
%End

    QString groupShortName();
%Docstring
return group WMS short name
%End

    QString groupAbstract();
%Docstring
return group WMS abstract
%End


  public slots:
    void setGroupTitle( const QString &title );
%Docstring
set group WMS title
%End

    void setGroupShortName( const QString &shortName );
%Docstring
set group WMS short name
%End

    void setGroupAbstract( const QString &abstract );
%Docstring
set group WMS abstract
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsgroupwmsdatadialog.h                                      *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
