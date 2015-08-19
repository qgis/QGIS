/***************************************************************************
                          qgsfieldconditionalformatwidget.h
                             -------------------
    begin                :
    copyright            :
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDCONDITIONALFORMATWIDGET_H
#define QGSFIELDCONDITIONALFORMATWIDGET_H

#include <QWidget>
#include <QStandardItemModel>
#include <QStandardItem>

#include "ui_qgsfieldconditionalformatwidget.h"
#include "qgsfieldcombobox.h"
#include "qgsconditionalstyle.h"


class GUI_EXPORT QgsFieldConditionalFormatWidget : public QWidget, private Ui::QgsFieldConditionalWidget
{
    Q_OBJECT
  public:
    explicit QgsFieldConditionalFormatWidget( QWidget *parent = 0 );

    void viewRules();

    void setLayer( QgsVectorLayer* theLayer );

    void editStyle( int index, QgsConditionalStyle style );

    void reset();

  signals:
    void rulesUpdates();

  public slots:

  private:
    QgsVectorLayer* mLayer;
    int mEditIndex;
    bool mEditing;
    QStandardItemModel* mModel;
    QgsSymbolV2* mSymbol;

  private slots:
    void updateIcon();
    void defaultPressed( QAbstractButton*button );
    bool isCustomSet();
    void ruleClicked( QModelIndex index );
    void reloadStyles();
    void cancelRule();
    void deleteRule();
    void saveRule();
    void addNewRule();
    void fieldChanged( QString fieldName );

};

#endif // QGSFIELDCONDITIONALFORMATWIDGET_H
