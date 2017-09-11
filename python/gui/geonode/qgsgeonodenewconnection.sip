/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/geonode/qgsgeonodenewconnection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsGeoNodeNewConnection : QDialog
{
%Docstring
*************************************************************************
                                                                         *
   This program is free software; you can redistribute it and/or modify  *
   it under the terms of the GNU General Public License as published by  *
   the Free Software Foundation; either version 2 of the License, or     *
   (at your option) any later version.                                   *
                                                                         *
**************************************************************************
%End

%TypeHeaderCode
#include "qgsgeonodenewconnection.h"
%End
  public:
    QgsGeoNodeNewConnection( QWidget *parent = 0, const QString &connName = QString::null, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor
%End

  public slots:
    virtual void accept();

    void okButtonBehavior( const QString & );
    void testConnection();
%Docstring
Test the connection using the parameters supplied
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/geonode/qgsgeonodenewconnection.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
