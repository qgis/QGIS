/***************************************************************************
    qgsarcgisresttokenmanagerdialog.h
     --------------------------------------
    Date                 : October 2018
    Copyright            : (C) 2018 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSARCGISRESTTOKENMANAGERDIALOG_H
#define QGSARCGISRESTTOKENMANAGERDIALOG_H

#include <QDialog>

#include "ui_qgsarcgisresttokenmanagerbase.h"

class QgsArcGisRestTokenManagerTableModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    enum TableColumns
    {
      ServerColumn  = 0,
      TokenColumn,
    };

    QgsArcGisRestTokenManagerTableModel( QObject *parent = nullptr );

    void removeTokens( const QModelIndexList &indexes );
    void addToken( const QString &server );

    void saveTokens();

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

  private:

    QStringList mServers;
    QStringList mTokens;

};

class QgsArcGisRestTokenManagerDialog : public QDialog, private Ui::QgsArcgisRestTokenManagerBase
{
    Q_OBJECT

  public:
    explicit QgsArcGisRestTokenManagerDialog( QWidget *parent = nullptr );

  public slots:

    void accept() override;

    void addToken();
    void removeToken();

  private:

    QgsArcGisRestTokenManagerTableModel *mModel = new QgsArcGisRestTokenManagerTableModel( this );
};


#endif // QGSARCGISRESTTOKENMANAGERDIALOG_H
