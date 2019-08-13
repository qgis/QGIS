/***************************************************************************
    qgsdiscoverrelationsdlg.h
    ---------------------
    begin                : September 2016
    copyright            : (C) 2016 by Patrick Valsecchi
    email                : patrick dot valsecchi at camptocamp dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDISCOVERRELATIONSDLG_H
#define QGSDISCOVERRELATIONSDLG_H

#include <QDialog>
#include "ui_qgsdiscoverrelationsdialogbase.h"
#include "qgsrelation.h"
#include "qgis_app.h"

class QgsRelationManager;
class QgsVectorLayer;

/**
 * Shows the list of relations discovered from the providers.
 *
 * The user can select some of them to add them to his project.
 */
class APP_EXPORT QgsDiscoverRelationsDialog : public QDialog, private Ui::QgsDiscoverRelationsDialogBase
{
    Q_OBJECT

  public:
    explicit QgsDiscoverRelationsDialog( const QList<QgsRelation> &existingRelations, const QList<QgsVectorLayer *> &layers, QWidget *parent = nullptr );

    /**
     * Gets the selected relations.
     */
    QList<QgsRelation> relations() const;

  private slots:
    void onSelectionChanged();

  private:
    QList<QgsVectorLayer *> mLayers;
    QList<QgsRelation> mFoundRelations;

    void addRelation( const QgsRelation &rel );

};

#endif // QGSDISCOVERRELATIONSDLG_H
