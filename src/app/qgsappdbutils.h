/***************************************************************************
                             qgsappdbutils.h
                             ------------------------
    Date                 : April 2025
    Copyright            : (C) 2025 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAPPDBUTILS_H
#define QGSAPPDBUTILS_H

#include "qgis.h"
#include <QDialog>

class QgsDatabaseQueryHistoryWidget;
class QDialogButtonBox;
class QgsDatabaseItemGuiProvider;

class QgsAppDbUtils : public QObject
{
    Q_OBJECT

  public:
    void setup();
    void openQueryDialog( const QString &connectionUri, const QString &provider, const QString &sql );

  public slots:

    void showQueryHistory();

  private:
    QgsDatabaseItemGuiProvider *mDatabaseItemGuiProvider = nullptr;
};


class QgsQueryHistoryDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsQueryHistoryDialog.
     */
    QgsQueryHistoryDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:

    void clearHistory();
    void openQueryDialog( const QString &connectionUri, const QString &provider, const QString &sql );

  private:
    QgsDatabaseQueryHistoryWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSAPPDBUTILS_H
