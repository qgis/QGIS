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

class QgsRelation;
class QgsRelationManager;
class QgsRelationManagerTreeModel;
class QgsVectorLayer;

class GUI_EXPORT QgsRelationManagerDialog : public QWidget, private Ui::QgsRelationManagerDialogBase
{
    Q_OBJECT

  public:
    explicit QgsRelationManagerDialog( QgsRelationManager* relationMgr, QWidget *parent = 0 );
    ~QgsRelationManagerDialog();

    void setLayers( QList< QgsVectorLayer* > );

    void addRelation( const QgsRelation& rel );
    QList< QgsRelation > relations();

  public slots:
    void on_mBtnAddRelation_clicked();
    void on_mBtnRemoveRelation_clicked();

  private:
    QgsRelationManager* mRelationManager;
    QList< QgsVectorLayer* > mLayers;
};

#endif // QGSRELATIONMANAGERDIALOG_H
