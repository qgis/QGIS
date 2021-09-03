/***************************************************************************
                          qgsaddtaborgroup.h
        Add a tab or a group for the tab and group display of fields
                             -------------------
    begin                : 2012-07-30
    copyright            : (C) 2012 by Denis Rouzaud
    email                : denis dot rouzaud at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADDTABORGROUP
#define QGSADDTABORGROUP

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsaddtaborgroupbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"
#include "qgshelp.h"

class QTreeWidgetItem;
class QgsVectorLayer;

/**
 * \ingroup gui
 * \brief Dialog to add a tab or group of attributes
 *
 * \note This class is not a part of public API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsAddTabOrGroup : public QDialog, private Ui::QgsAddTabOrGroupBase
{
    Q_OBJECT

  public:
    typedef QPair<QString, QTreeWidgetItem *> TabPair;

  public:
    //! constructor
    QgsAddTabOrGroup( QgsVectorLayer *lyr, const QList<TabPair> &tabList, QTreeWidgetItem *currentTab = nullptr, QWidget *parent = nullptr );

    //! Returns the name of the tab or group
    QString name();

    //! Returns tree item corresponding to the added tab
    QTreeWidgetItem *tab();

    //! Returns the column count
    int columnCount() const;

    //! Returns whether the tab button is checked
    bool tabButtonIsChecked();

    //! Accepts the dialog
    void accept() override;

  private slots:
    void mGroupButton_toggled( bool checked );
    void mTabButton_toggled( bool checked );
    void showHelp();

  protected:
    QgsVectorLayer *mLayer = nullptr;
    QList< TabPair > mTabs;
};

#endif
