/***************************************************************************
    qgsdatabaseschemaselectiondialog.h
    ---------------------
    begin                : March 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATABASESCHEMASELECTIONDIALOG_H
#define QGSDATABASESCHEMASELECTIONDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>

#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsdatabaseschemacombobox.h"

/**
 * \ingroup gui
 * \brief Dialog which displays selection of a schema for a specific database connection.
 *
 * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation.
 *
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsDatabaseSchemaSelectionDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsDatabaseSchemaSelectionDialog, for the specified \a connection.
     *
     * Ownership of \a connection is transferred.
     */
    explicit QgsDatabaseSchemaSelectionDialog( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the name of the current schema selected in the combo box.
     */
    QString selectedSchema() const;

  private:
    QDialogButtonBox *mButtonBox = nullptr;
    QgsDatabaseSchemaComboBox *mCboSchema = nullptr;
};
#endif // QGSDATABASESCHEMASELECTIONDIALOG_H
