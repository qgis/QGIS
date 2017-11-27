/***************************************************************************
    qgsrelationmanagerdialog.h
     --------------------------------------
    Date                 : 23.2.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRELATIONMANAGERDIALOG_H
#define QGSRELATIONMANAGERDIALOG_H

#include <QWidget>
#include "ui_qgsrelationmanagerdialogbase.h"
#include "qgis_app.h"

class QgsRelation;
class QgsRelationManager;
class QgsRelationManagerTreeModel;
class QgsVectorLayer;

class APP_EXPORT QgsRelationManagerDialog : public QWidget, private Ui::QgsRelationManagerDialogBase
{
    Q_OBJECT

  public:
    explicit QgsRelationManagerDialog( QgsRelationManager *relationMgr, QWidget *parent = nullptr );

    void setLayers( const QList<QgsVectorLayer *> & );

    void addRelation( const QgsRelation &rel );
    QList< QgsRelation > relations();

  private slots:
    void mBtnAddRelation_clicked();
    void mBtnDiscoverRelations_clicked();
    void mBtnRemoveRelation_clicked();
    void onSelectionChanged();

  private:
    QgsRelationManager *mRelationManager = nullptr;
    QList< QgsVectorLayer * > mLayers;
};

#endif // QGSRELATIONMANAGERDIALOG_H
