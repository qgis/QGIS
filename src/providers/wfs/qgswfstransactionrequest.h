/***************************************************************************
    qgswfstransactionrequest.h
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWFSTRANSACTIONREQUEST_H
#define QGSWFSTRANSACTIONREQUEST_H

#include "qgswfsrequest.h"

/** Manages the Transaction requests */
class QgsWFSTransactionRequest : public QgsWFSRequest
{
    Q_OBJECT
  public:
    explicit QgsWFSTransactionRequest( const QString& theUri );

    /** Send the transaction document and return the server response */
    bool send( const QDomDocument& doc, QDomDocument& serverResponse );

  protected:
    virtual QString errorMessageWithReason( const QString& reason ) override;
};

#endif // QGSWFSTRANSACTIONREQUEST_H
