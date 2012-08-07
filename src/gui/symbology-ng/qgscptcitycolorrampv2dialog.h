/***************************************************************************
    qgscptcitycolorrampv2dialog.h
    ---------------------
    begin                : July 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCPTCITYCOLORRAMPV2DIALOG_H
#define QGSCPTCITYCOLORRAMPV2DIALOG_H

#include <QDialog>

#include "ui_qgscptcitycolorrampv2dialogbase.h"

class QgsCptCityColorRampV2;

class GUI_EXPORT QgsCptCityColorRampV2Dialog : public QDialog, private Ui::QgsCptCityColorRampV2DialogBase
{
    Q_OBJECT

  public:
    QgsCptCityColorRampV2Dialog( QgsCptCityColorRampV2* ramp, QWidget* parent = NULL );

  public slots:
    void setSchemeName();
    void setVariantName();
    void populateSchemes( QString view = "author" );
    void populateVariants();

    void on_treeWidget_currentItemChanged( QTreeWidgetItem * current, QTreeWidgetItem * previous );
    void on_treeWidget_itemExpanded( QTreeWidgetItem * item );
    void on_buttonGroupView_buttonClicked( QAbstractButton * button );

  protected:

    void updatePreview();
    QTreeWidgetItem* findPath( QString path );
    QTreeWidgetItem * makeCollectionItem( const QString& path );
    void makeSchemeItem( QTreeWidgetItem *item, const QString& path, const QString& schemeName );

    QgsCptCityColorRampV2* mRamp;
};

#endif
