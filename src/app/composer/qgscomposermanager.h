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

    /** Returns the default templates (key: template name, value: absolute path to template file)
     * @param fromUser whether to return user templates from ~/.qgis/composer_templates (added in 1.9)
     */
    QMap<QString, QString> defaultTemplates( bool fromUser = false ) const;

    /** Open local directory with user's system, creating it if not present
     * @note added in QGIS 1.9
     */
    void openLocalDirectory( const QString& localDirPath );

    QString mDefaultTemplatesDir;
    QString mUserTemplatesDir;

  private slots:
    void on_mAddButton_clicked();
    /** Slot to track combobox to use specific template path
     * @note added in 1.9
     */
    void on_mTemplate_currentIndexChanged( int indx );
    /** Slot to choose path to template
     * @note added in QGIS 1.9
     */
    void on_mTemplatePathBtn_pressed();
    /** Slot to open default templates dir with user's system
     * @note added in QGIS 1.9
     */
    void on_mTemplatesDefaultDirBtn_pressed();
    /** Slot to open user templates dir with user's system
     * @note added in QGIS 1.9
     */
    void on_mTemplatesUserDirBtn_pressed();

    void remove_clicked();
    void show_clicked();
    /** Duplicate composer
     * @note added in 1.9
     */
    void duplicate_clicked();
    void rename_clicked();
    void on_mComposerListWidget_itemChanged( QListWidgetItem * item );
};

#endif // QGSCOMPOSERMANAGER_H
