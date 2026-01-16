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

#include "qgis_app.h"

#include <QDomDocument>
#include <QIcon>

class QWidget;
class QgsBrowserDockWidget;
class QDockWidget;
class QgisApp;
class QAction;

/**
 * Customization is read from an XML file so we can later keep track of
 * action order in menus and tool bars. This XML file is read on a tree
 * model where every node is an Item. Item is then inherited by any
 * customizable widget item.
 *
 * Main methods:
 *
 * - read() reads the XML file into the model
 * - load() updates the model with the different graphical elements
 * - apply() applies the model to the QGIS main window and the 2 browser widgets.
 *
 * \since QGIS 4.0
 */
class APP_EXPORT QgsCustomization
{
  public:
    /**
   * Constructor
   * \param customizationFile file path of the customization. This one is used
   * when calling read() and write()
   */
    QgsCustomization( const QString &customizationFile );

    /**
     * Destructor
     */
    ~QgsCustomization();

    /**
     * Copy constructor
     */
    QgsCustomization( const QgsCustomization &other );

    /**
     * Assignment operator
     */
    QgsCustomization &operator=( const QgsCustomization &other );

    /**
     * Set QGIS main window \a qgisApp
     * Customization model is updated according to main window menus, toolbars, dock widgets ..
     */
    void setQgisApp( QgisApp *qgisApp );

    /**
     * Returns TRUE if the customization is currently enabled. If disabled, the customization is not
     * applied on the application
     * \see setEnabled()
     */
    bool isEnabled() const;

    /**
     * Sets \a enabled state
     * If disabled, the customization is not applied on the application
     * \see isEnabled()
     */
    void setEnabled( bool enabled );

    /**
     * Returns path to the splash screen
     */
    QString splashPath() const;

    /**
     * \brief Represents an item that can be customized in the application
     * This is the base model class for all application customizable items. setVisible() allows to
     * change whether or not the item should be displayed or not.
     * It's also used to serialize these customization in an XML file.
     */
    class QgsItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsItem( QgsItem *parent = nullptr );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        QgsItem( const QString &name, const QString &title, QgsItem *parent = nullptr );

        /**
         * Destructor
         */
        virtual ~QgsItem();

        /**
         * Returns name
         */
        const QString &name() const;

        /**
         * Returns title
         */
        const QString &title() const;

        /**
         * Sets \a title
         */
        void setTitle( const QString &title );

        /**
         * Returns Item's parent
         */
        QgsItem *parent() const;

        /**
         * Returns TRUE if the item is visible
         */
        bool isVisible() const;

        /**
         * Sets item visibility to \a isVisible
         */
        void setVisible( bool isVisible );

        /**
         * Adds child item \a item
         */
        void addChild( std::unique_ptr<QgsItem> item );

        /**
         * Return child item at \a index position, nullptr if index is outside the children list bounds
         */
        QgsItem *getChild( int index ) const;

        /**
         * Returns child item with has the name \a name, nullptr if not found
         */
        template<class T>
        T *getChild( const QString &name ) const { return dynamic_cast<T *>( getChild( name ) ); }

        /**
         * Returns child item with has the name \a name, nullptr if not found
         */
        QgsItem *getChild( const QString &name ) const;

        /**
         * Return last child item, nullptr if the list is empty
         */
        template<class T>
        T *lastChild() const { return dynamic_cast<T *>( lastChild() ); }

        /**
         * Return last child item, nullptr if the list is empty
         */
        QgsItem *lastChild() const;

        /**
         * Returns list of child items
         */
        const std::vector<std::unique_ptr<QgsItem>> &childItemList() const;

        /**
         * Returns \a item position in child item list, -1 if it doesn't exist
         */
        int indexOf( QgsItem *item ) const;

        /**
         * Returns children count
         */
        unsigned int childrenCount() const;

        /**
         * Sets \a icon
         */
        void setIcon( const QIcon &icon );

        /**
         * Returns icon
         */
        QIcon icon() const;

        /**
         * Writes XML element to document \a doc as a child of the \a parent element
         */
        void writeXml( QDomDocument &doc, QDomElement &parent ) const;

        /**
         * Reads XML information from element \a elem.
         * Returns error string or an empty string if no error occurred
         */
        QString readXml( const QDomElement &elem );

        /**
         * Returns this item clone with \a parent as parent item
         */
        virtual std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const = 0;

      protected:
        /**
         * Returns XML tag
         */
        virtual QString xmlTag() const = 0;

        /**
         * Creates child item from \a childElem element
         */
        virtual std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem );

        /**
         * Copy \a other item attributes to this item
         */
        virtual void copyItemAttributes( const QgsCustomization::QgsItem *other );

        QString mName;

      private:
        QString mTitle;
        bool mVisible = true;
        QgsItem *mParent = nullptr;
        QIcon mIcon;
        QMap<QString, QgsItem *> mChildItems; // for name quick access
        std::vector<std::unique_ptr<QgsItem>> mChildItemList;
    };

    /**
     * \brief Represents an action
     */
    class QgsActionItem : public QgsItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsActionItem( QgsItem *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        QgsActionItem( const QString &name, const QString &title, QgsItem *parent );

        /**
         * Associated QAction \a qaction and its index \a actionIndex in the widget holding the action
         * (Menu, WidgetAction). Used to restore it at the right position if previously removed
         */
        void setQAction( QAction *qaction, qsizetype actionIndex );

        /**
         * Returns associated QAction
         */
        QAction *qAction() const;

        /**
         * Returns actionIndex in the widget holding the action
         */
        qsizetype qActionIndex() const;

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsActionItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
        void copyItemAttributes( const QgsItem *other ) override;

      private:
        QAction *mQAction = nullptr;
        qsizetype mQActionIndex = -1;
    };

    /**
     * Represent a Menu
     * Inherits from Action because QMenu are stored within a QAction and we want to keep
     * track of the menu associated action
     */
    class QgsMenuItem : public QgsActionItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsMenuItem( QgsItem *parent );
        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        QgsMenuItem( const QString &name, const QString &title, QgsItem *parent );

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsMenuItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represents a toolbar
     */
    class QgsToolBarItem : public QgsItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsToolBarItem( QgsItem *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        QgsToolBarItem( const QString &name, const QString &title, QgsItem *parent );

        /**
         * Sets original dock widget visible state
         * \see wasVisible()
         */
        void setWasVisible( const bool &wasVisible );

        /**
         * Returns original dock widget visible state
         * \see setWasVisible()
         */
        bool wasVisible() const;

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsToolBarItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
        void copyItemAttributes( const QgsItem *other ) override;

      private:
        // used to backup the original visibility state when we change visibility state
        bool mWasVisible = false;
    };

    /**
     * Root item for all ToolBar item
     */
    class QgsToolBarsItem : public QgsItem
    {
      public:
        /**
         * Constructor
         */
        QgsToolBarsItem();

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsToolBarsItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Root item for all Menus item
     */
    class QgsMenusItem : public QgsItem
    {
      public:
        /**
         * Constructor
         */
        QgsMenusItem();

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsMenusItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represent a Dock
     */
    class QgsDockItem : public QgsItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsDockItem( QgsItem *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        QgsDockItem( const QString &name, const QString &title, QgsItem *parent );

        /**
         * Sets original dock widget visible state
         * \see wasVisible()
         */
        void setWasVisible( const bool &wasVisible );

        /**
         * Returns original dock widget visible state
         * \see setWasVisible()
         */
        bool wasVisible() const;

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsDockItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        void copyItemAttributes( const QgsItem *other ) override;

      private:
        // used to backup the original visibility state when we change visibility state
        bool mWasVisible = false;
    };

    /**
     * Root item for all dock items
     */
    class QgsDocksItem : public QgsItem
    {
      public:
        /**
         * Constructor
         */
        QgsDocksItem();

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsDocksItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represent a QgsBrowserDockWidget item
     */
    class QgsBrowserElementItem : public QgsItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsBrowserElementItem( QgsItem *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        QgsBrowserElementItem( const QString &name, const QString &title, QgsItem *parent );

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsBrowserElementItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
    };

    /**
     * Root item for all browser items
     */
    class QgsBrowserElementsItem : public QgsItem
    {
      public:
        /**
         * Constructor
         */
        QgsBrowserElementsItem();

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsBrowserElementsItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represent a QgsStatusBar widget
     */
    class QgsStatusBarWidgetItem : public QgsItem
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        QgsStatusBarWidgetItem( QgsItem *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param parent parent Item
         */
        QgsStatusBarWidgetItem( const QString &name, QgsItem *parent );

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsStatusBarWidgetItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
    };

    /**
     * Root item for all StatusBarWidget
     */
    class QgsStatusBarWidgetsItem : public QgsItem
    {
      public:
        /**
         * Constructor
         */
        QgsStatusBarWidgetsItem();

        /**
         * Returns this item clone with \a parent as parent item
         */
        std::unique_ptr<QgsCustomization::QgsStatusBarWidgetsItem> cloneSameType( QgsCustomization::QgsItem *parent = nullptr ) const;

        std::unique_ptr<QgsCustomization::QgsItem> clone( QgsCustomization::QgsItem *parent = nullptr ) const override { return cloneSameType( parent ); };

      protected:
        QString xmlTag() const override;
        std::unique_ptr<QgsItem> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Returns browser items to customize QgsBrowserDockWidget content
     */
    QgsCustomization::QgsBrowserElementsItem *browserElementsItem() const;

    /**
     * Returns dock items to customize visible QgsDockWidget
     */
    QgsCustomization::QgsDocksItem *docksItem() const;

    /**
     * Returns menus items to customize QMenu content
     */
    QgsCustomization::QgsMenusItem *menusItem() const;

    /**
     * Returns status bar items to customize QgsStatusBar displayed widgets
     */
    QgsCustomization::QgsStatusBarWidgetsItem *statusBarWidgetsItem() const;

    /**
     * Returns toolbar items to customize QToolBar content
     */
    QgsCustomization::QgsToolBarsItem *toolBarsItem() const;

    /**
     * Apply customization to the application
     */
    void apply() const;

    /**
     * Reads customization file (given at construction time) to update customization content
     */
    void read();

    /**
     * Reads customization file \a filePath to update customization content
     * Returns error string or an empty string if no error occurred
     */
    QString readFile( const QString &filePath );

    /**
     * Write customization to file (given at construction time)
     * Returns error string or an empty string if no error occurred
     */
    QString write() const;

    /**
     * Write customization to \a filePath file
     * Returns error string or an empty string if no error occurred
     */
    QString writeFile( const QString &filePath ) const;

  private:
    /**
     * Add action items as children of \a item for each \a widget actions
     */
    void addActions( QgsItem *item, QWidget *widget ) const;

    /**
     * Update customization model with current application customization elements (actins, menus, dockWidgets...)
     */
    void load();

    /**
     * Update customization model with current application QgsBrowserDockWidget elements
     */
    void loadApplicationBrowserItems();

    /**
     * Update customization model with current application dock widgets elements
     */
    void loadApplicationDocks();

    /**
     * Update customization model with current application menus elements
     */
    void loadApplicationMenus();

    /**
     * Update customization model with current application QgsStatusBar elements
     */
    void loadApplicationStatusBarWidgets();

    /**
     * Update customization model with current application toolbar elements
     */
    void loadApplicationToolBars();

    /**
     * Apply browser items customization to the application
     */
    void applyToBrowserItems() const;

    /**
     * Apply docks customization to the application
     */
    void applyToDocks() const;

    /**
     * Apply menus customization to the application
     */
    void applyToMenus() const;

    /**
     * Apply status bar customization to the application
     */
    void applyToStatusBarWidgets() const;

    /**
     * Apply toolbar customization to the application
     */
    void applyToToolBars() const;

    /**
     * Helper class to iterate over widget actions
     */
    class QgsQActionsIterator
    {
      public:
        /**
         * Constructor
         * \param widget this class will iterate over the widget actions
         */
        QgsQActionsIterator( QWidget *widget );

        /**
         * Iterator information
         */
        struct Info
        {
            QWidget *widget = nullptr;
            QAction *action = nullptr;
            qsizetype index = -1;
            QString name;
            QString title;
            QIcon icon;
            bool isMenu = false;
        };

        struct Iterator
        {
            Iterator( QWidget *ptr, qsizetype idx );

            Info operator*() const;
            Iterator &operator++();
            bool operator==( const Iterator &b ) const;

          private:
            qsizetype idx;
            QList<QAction *> mActions;
        };

        Iterator begin();
        Iterator end();

      private:
        QWidget *mWidget = nullptr;

        friend class TestQgsCustomization;
    };

    /**
     * Backward compatibility method to import old QGIS3 ini file
     */
    void loadOldIniFile( const QString &filePath );

    /**
     * Update \a widget visibility based on \a item
     */
    static void updateActionVisibility( QgsCustomization::QgsItem *item, QWidget *widget );

    std::unique_ptr<QgsBrowserElementsItem> mBrowserItems;
    std::unique_ptr<QgsDocksItem> mDocks;
    std::unique_ptr<QgsMenusItem> mMenus;
    std::unique_ptr<QgsStatusBarWidgetsItem> mStatusBarWidgets;
    std::unique_ptr<QgsToolBarsItem> mToolBars;
    bool mEnabled = false;
    QString mSplashPath;

    QgisApp *mQgisApp = nullptr;
    QString mCustomizationFile;

    friend class TestQgsCustomization;
};
#endif // QGSCUSTOMIZATION_H
