/***************************************************************************
                              qgscomposermanager.h
                             ------------------------
    begin                : September 11 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERMANAGER_H
#define QGSCOMPOSERMANAGER_H

#include "ui_qgscomposermanagerbase.h"

class QListWidgetItem;
class QgsComposer;

/**A dialog that shows the existing composer instances. Lets the user add new
instances and change title of existing ones*/
class QgsComposerManager: public QDialog, private Ui::QgsComposerManagerBase
{
    Q_OBJECT
  public:
    QgsComposerManager( QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsComposerManager();


  private:
    /**Stores the relation between items and composer pointers. A 0 pointer for the composer means that
      this composer needs to be created from a default template*/
    QMap<QListWidgetItem*, QgsComposer*> mItemComposerMap;

    /**Enters the composer instances and created the item-composer map*/
    void initialize();

    /**Returns the default templates (key: template name, value: absolute path to template file)*/
    QMap<QString, QString> defaultTemplates() const;

  private slots:
    void on_mAddButton_clicked();
    void remove_clicked();
    void show_clicked();
    void rename_clicked();
    void on_mComposerListWidget_itemChanged( QListWidgetItem * item );
};

#endif // QGSCOMPOSERMANAGER_H
