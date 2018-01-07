/***************************************************************************
    qgsrelationadddlg.h
    ---------------------
    begin                : December 2015
    copyright            : (C) 2015 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRELATIONADDDLG_H
#define QGSRELATIONADDDLG_H

#include <QDialog>
#include "ui_qgsrelationadddlgbase.h"
#include "qgis_app.h"
#include "qgsrelation.h"

class QgsVectorLayer;

class APP_EXPORT QgsRelationAddDlg : public QDialog, private Ui::QgsRelationAddDlgBase
{
    Q_OBJECT

  public:
    explicit QgsRelationAddDlg( QWidget *parent = nullptr );

    QString referencingLayerId();
    QString referencedLayerId();
    QList< QPair< QString, QString > > references();
    QString relationId();
    QString relationName();
    QgsRelation::RelationStrength relationStrength();

  private slots:

    void checkDefinitionValid();

};

#endif // QGSRELATIONADDDLG_H
