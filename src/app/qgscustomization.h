/***************************************************************************
                             qgscustomization.h  - Customization
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCUSTOMIZATION_H
#define QGSCUSTOMIZATION_H

#include "ui_qgscustomizationdialogbase.h"

#include <QDialog>
#include <QDomNode>
#include <QEvent>
#include <QMouseEvent>
#include <QSettings>
#include <QTreeWidgetItem>

class QString;
class QWidget;
class QTreeWidgetItem;

class QgsCustomizationDialog : public QMainWindow, private Ui::QgsCustomizationDialogBase
{
    Q_OBJECT
  public:
    QgsCustomizationDialog();
    ~QgsCustomizationDialog();

    // get item by path
    QTreeWidgetItem *item( QString thePath, QTreeWidgetItem *theItem = 0 );

    //

    // return current item state for given path
    bool itemChecked( QString thePath );
    // set item state for given path
    void setItemChecked( QString thePath, bool on );

    // recursively save tree item to settings
    void itemToSettings( QString thePath, QTreeWidgetItem *theItem, QSettings *theSettings );
    // recursively save settings to tree items
    void settingsToItem( QString thePath, QTreeWidgetItem *theItem, QSettings *theSettings );

    // save current tree to settings
    void treeToSettings( QSettings *theSettings );

    // restore current tree from settings
    void settingsToTree( QSettings *theSettings );

    // switch widget item in tree
    bool switchWidget( QWidget * widget, QMouseEvent *event );

    // Get path of the widget
    QString widgetPath( QWidget * theWidget, QString thePath = QString() );

    void setCatch( bool on );
    bool catchOn( );

  private slots:
    //void on_btnQgisUser_clicked();

    // Save to settings
    void ok();
    void apply();

    void cancel();

    // Reset values from settings
    void reset();

    // Save to settings to file
    void on_actionSave_triggered( bool checked );

    // Load settings from file
    void on_actionLoad_triggered( bool checked );

    void on_actionExpandAll_triggered( bool checked );
    void on_actionCollapseAll_triggered( bool checked );
    void on_actionSelectAll_triggered( bool checked );

  private:
    void init();
    QTreeWidgetItem * createTreeItemWidgets( );
    QTreeWidgetItem * readWidgetsXmlNode( QDomNode theNode );

    QString mLastDirSettingsName;
    QSettings mSettings;
};

class QgsCustomization : public QObject
{
    Q_OBJECT

  public:
    enum Status
    {
      NotSet    = 0,
      User      = 1, // Set by user
      Default   = 2  // Default customization loaded and set
    };

    //! Returns the instance pointer, creating the object on the first call
    static QgsCustomization* instance();

    void openDialog();
    static void customizeWidget( QWidget * widget, QEvent * event );
    static void customizeWidget( QString path, QWidget * widget );
    static void removeFromLayout( QLayout *theLayout, QWidget * widget );

    void updateMainWindow( QMenu * theToolBarMenu );

    // make sure to enable/disable before creating QgisApp in order to get it customized (or not)
    void setEnabled( bool enabled ) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    // Load and set default customization
    void loadDefault();

    // Internal Qt widget which has to bes kipped in paths
    static QStringList mInternalWidgets;

    QString statusPath() { return mStatusPath; }

  public slots:
    void preNotify( QObject * receiver, QEvent * event, bool * done );

  protected:
    QgsCustomization( );
    ~QgsCustomization();
    QgsCustomizationDialog *pDialog;

    bool mEnabled;
    QString mStatusPath;

    void updateMenu( QMenu* menu, QSettings& settings );
    void createTreeItemMenus( );
    void createTreeItemToolbars( );
    void createTreeItemDocks( );
    void createTreeItemStatus( );
    void addTreeItemMenu( QTreeWidgetItem* parentItem, QMenu* menu );
    void addTreeItemActions( QTreeWidgetItem* parentItem, const QList<QAction*>& actions );
    QList<QTreeWidgetItem*> mMainWindowItems;
    friend class QgsCustomizationDialog; // in order to access mMainWindowItems

  private slots:

  private:
    static QgsCustomization* pinstance;
    QSettings mSettings;

};
#endif // QGSCUSTOMIZATION_H

