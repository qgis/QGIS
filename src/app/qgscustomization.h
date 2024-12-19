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
#include "qgshelp.h"

#include <QDialog>
#include <QDomNode>

#include "qgis_app.h"

class QString;
class QWidget;
class QTreeWidgetItem;
class QEvent;
class QMouseEvent;
class QSettings;
class QgsBrowserDockWidget;

class APP_EXPORT QgsCustomizationDialog : public QMainWindow, private Ui::QgsCustomizationDialogBase
{
    Q_OBJECT
  public:
    QgsCustomizationDialog( QWidget *parent, QSettings *settings );

    // get item by path
    QTreeWidgetItem *item( const QString &path, QTreeWidgetItem *widgetItem = nullptr );

    //

    // return current item state for given path
    bool itemChecked( const QString &path );
    // set item state for given path
    void setItemChecked( const QString &path, bool on );

    // recursively save tree item to settings
    void itemToSettings( const QString &path, QTreeWidgetItem *item, QSettings *settings );
    // recursively save settings to tree items
    void settingsToItem( const QString &path, QTreeWidgetItem *item, QSettings *settings );

    // save current tree to settings
    void treeToSettings( QSettings *settings );

    // restore current tree from settings
    void settingsToTree( QSettings *settings );

    // switch widget item in tree
    bool switchWidget( QWidget *widget, QMouseEvent *event );

    // Get path of the widget
    QString widgetPath( QWidget *widget, const QString &path = QString() );

    void setCatch( bool on );
    bool catchOn();

  private slots:
    //void on_btnQgisUser_clicked();

    // Save to settings
    void ok();
    void apply();

    void cancel();

    void showHelp();

    // Reset values from settings
    void reset();

    // Save to settings to file
    void actionSave_triggered( bool checked );

    // Load settings from file
    void actionLoad_triggered( bool checked );

    void actionExpandAll_triggered( bool checked );
    void actionCollapseAll_triggered( bool checked );
    void actionSelectAll_triggered( bool checked );

    void enableCustomization( bool checked );
    bool filterItems( const QString &text );

  private:
    void init();
    QTreeWidgetItem *createTreeItemWidgets();
    QTreeWidgetItem *readWidgetsXmlNode( const QDomNode &node );
    QAction *findAction( QToolButton *toolbutton );

    QString mLastDirSettingsName;
    QSettings *mSettings = nullptr;

  protected:
    QMap<QTreeWidgetItem *, bool> mTreeInitialExpand;
    QMap<QTreeWidgetItem *, bool> mTreeInitialVisible;
};

class APP_EXPORT QgsCustomization : public QObject
{
    Q_OBJECT

  public:
    enum Status
    {
      NotSet    = 0,
      User      = 1, // Set by user
      Default   = 2  // Default customization loaded and set
    };
    Q_ENUM( Status )

    //! Returns the instance pointer, creating the object on the first call
    static QgsCustomization *instance();

    void openDialog( QWidget *parent );
    static void customizeWidget( QWidget *widget, QEvent *event, QSettings *settings );
    static void customizeWidget( const QString &path, QWidget *widget, QSettings *settings );
    static void removeFromLayout( QLayout *layout, QWidget *widget );

    void updateBrowserWidget( QgsBrowserDockWidget *model );
    void updateMainWindow( QMenu *toolBarMenu, QMenu *panelMenu );

    // make sure to enable/disable before creating QgisApp in order to get it customized (or not)
    void setEnabled( bool enabled ) { mEnabled = enabled; }
    bool isEnabled() const { return mEnabled; }

    void setSettings( QSettings *settings ) { mSettings = settings ;}

    // Returns the path to the splash screen
    QString splashPath() const;

    // Loads and sets default customization
    void loadDefault();

    QString statusPath() const { return mStatusPath; }

  public slots:
    void preNotify( QObject *receiver, QEvent *event, bool *done );

  protected:
    QgsCustomization();
    ~QgsCustomization() override = default;
    QgsCustomizationDialog *pDialog = nullptr;

    bool mEnabled = false;
    QSettings *mSettings = nullptr;
    QString mStatusPath;

    void updateMenu( QMenu *menu, QSettings *settings );
    void createTreeItemMenus();
    void createTreeItemToolbars();
    void createTreeItemDocks();
    void createTreeItemStatus();
    void createTreeItemBrowser();
    void addTreeItemMenu( QTreeWidgetItem *parentItem, const QMenu *menu, const QAction *action = nullptr );
    void addTreeItemActions( QTreeWidgetItem *parentItem, const QList<QAction *> &actions );
    QList<QTreeWidgetItem *> mMainWindowItems;
    QTreeWidgetItem *mBrowserItem = nullptr;
    friend class QgsCustomizationDialog; // in order to access mMainWindowItems and mBrowserItem

  private:
    static QgsCustomization *sInstance;

};
#endif // QGSCUSTOMIZATION_H

