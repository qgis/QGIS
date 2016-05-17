/***************************************************************************
                qgsattributeactiondialog.h  -  attribute action dialog
                             -------------------

This class creates and manages the Action tab of the Vector Layer
Properties dialog box. Changes made in the dialog box are propagated
back to QgsVectorLayer.

    begin                : October 2004
    copyright            : (C) 2004 by Gavin Macaulay
    email                : gavin at macaulay dot co dot nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSATTRIBUTEACTIONDIALOG_H
#define QGSATTRIBUTEACTIONDIALOG_H

#include "ui_qgsattributeactiondialogbase.h"
#include "qgsactionmanager.h"
#include "qgsfield.h"
#include "qgsattributetableconfig.h"
#include <QMap>

class APP_EXPORT QgsAttributeActionDialog: public QWidget, private Ui::QgsAttributeActionDialogBase
{
    Q_OBJECT
  private:
    enum ColumnIndexes
    {
      Type,
      Description,
      ShortTitle,
      ActionText,
      Capture,
      ShowInAttributeTable
    };

  public:
    QgsAttributeActionDialog( const QgsActionManager& actions,
                              QWidget* parent = nullptr );

    ~QgsAttributeActionDialog() {}

    void init( const QgsActionManager& action , const QgsAttributeTableConfig& attributeTableConfig );

    QList<QgsAction> actions() const;

    bool showWidgetInAttributeTable() const;

    QgsAttributeTableConfig::ActionWidgetStyle attributeTableWidgetStyle() const;

  private slots:
    void moveUp();
    void moveDown();
    void remove();
    void insert();
    void addDefaultActions();
    void itemDoubleClicked( QTableWidgetItem* item );
    void updateButtons();

  private:
    void insertRow( int row, const QgsAction& action );
    void insertRow( int row, QgsAction::ActionType type, const QString& name, const QString& actionText, const QString& iconPath, bool capture );
    void swapRows( int row1, int row2 );
    QgsAction rowToAction( int row ) const;

    QString textForType( QgsAction::ActionType type );

    void rowSelected( int row );

    QString uniqueName( QString name );

    QgsVectorLayer* mLayer;
};

#endif
