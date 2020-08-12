/***************************************************************************
    qgsogrtransaction.h  -  Transaction support for OGR layers
                             -------------------
    begin                : June 13, 2018
    copyright            : (C) 2018 by Even Rouault
    email                : even.rouault @ spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRTRANSACTION_H
#define QGSOGRTRANSACTION_H

#include "qgstransaction.h"
#include "qgsogrprovider.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsOgrTransaction : public QgsTransaction
{
    Q_OBJECT

  public:
    explicit QgsOgrTransaction( const QString &connString, QgsOgrDatasetSharedPtr ds );

    /**
     * Executes the SQL query in database.
     *
     * \param sql The SQL query to execute
     * \param error The error or an empty string if none
     * \param isDirty True to add an undo/redo command in the edition buffer, false otherwise
     * \param name Name of the operation ( only used if `isDirty` is true)
     */
    bool executeSql( const QString &sql, QString &error, bool isDirty = false, const QString &name = QString() ) override;

    QgsOgrDatasetSharedPtr sharedDS() const { return mSharedDS; }

  private:

    QgsOgrDatasetSharedPtr mSharedDS = nullptr;

    bool beginTransaction( QString &error, int statementTimeout ) override;
    bool commitTransaction( QString &error ) override;
    bool rollbackTransaction( QString &error ) override;

};

///@endcond
#endif // QGSOGRTRANSACTION_H
