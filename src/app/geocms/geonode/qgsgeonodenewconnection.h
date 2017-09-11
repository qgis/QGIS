/***************************************************************************
                              qgsgeonodenewconnection.h
                              -------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEONODENEWCONNECTION_H
#define QGSGEONODENEWCONNECTION_H

#include "qgsnewhttpconnection.h"

class QgsGeoNodeNewConnection : public QgsNewHttpConnection
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGeoNodeNewConnection( QWidget *parent = nullptr, const QString &connName = QString::null, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

  public slots:

    //! Test the connection using the parameters supplied
    void testConnection();

  protected:

    bool validate() override;

};

#endif //QGSGEONODENEWCONNECTION_H
