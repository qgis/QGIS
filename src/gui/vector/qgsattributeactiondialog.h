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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsattributeactiondialogbase.h"
#include "qgsattributetableconfig.h"
#include "qgis_gui.h"
#include "qgis.h"

#include <QMap>

class QgsActionManager;
class QgsVectorLayer;
class QgsAction;

/**
 * \ingroup gui
 * \class QgsAttributeActionDialog
 */
class GUI_EXPORT QgsAttributeActionDialog: public QWidget, private Ui::QgsAttributeActionDialogBase
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
      ActionScopes,
      NotificationMessage,
      EnabledOnlyWhenEditable
    };

    //! UserRole for Type data
    enum Role
    {
      ActionType = Qt::UserRole,
      ActionId  = Qt::UserRole + 1
    };

  public:
    QgsAttributeActionDialog( const QgsActionManager &actions,
                              QWidget *parent = nullptr );

    void init( const QgsActionManager &action, const QgsAttributeTableConfig &attributeTableConfig );

    QList<QgsAction> actions() const;

    bool showWidgetInAttributeTable() const;

    QgsAttributeTableConfig::ActionWidgetStyle attributeTableWidgetStyle() const;

  private slots:
    void moveUp();
    void moveDown();
    void remove();
    void insert();
    void addDefaultActions();
    void itemDoubleClicked( QTableWidgetItem *item );
    void updateButtons();

  private:
    void insertRow( int row, const QgsAction &action );
    void insertRow( int row, Qgis::AttributeActionType type, const QString &name, const QString &actionText, const QString &iconPath, bool capture, const QString &shortTitle, const QSet<QString> &actionScopes, const QString &notificationMessage, bool isEnabledOnlyWhenEditable = false );
    void swapRows( int row1, int row2 );
    QgsAction rowToAction( int row ) const;

    QString textForType( Qgis::AttributeActionType type );

    void rowSelected( int row );

    QString uniqueName( QString name );

    QgsVectorLayer *mLayer = nullptr;
};

#endif
