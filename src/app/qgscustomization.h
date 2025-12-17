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
 * Customization is now read from an XML file so we can later keep track of
 * action order in menus and tool bars. This XML file is read on a tree
 * model where every node is an Item. Item is then inherited by any
 * customizable widget item.

 * read() methods reads the XML file into the model. load() update the
 * model with the different graphical element and apply() apply the model
 * to the QGIS main window and the 2 browser widgets.
 */
class APP_EXPORT QgsCustomization
{
  public:
    /**
   * Constructor
   * \param customizationFile file path of the customization. this one used
   * when calling read() and write()
   */
    QgsCustomization( const QString &customizationFile );

    /**
     * Destructor
     */
    ~QgsCustomization();

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
     * \brief Represents an item tha can be customized in the application
     */
    class Item
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        Item( Item *parent = nullptr );

        /**
         * Constructor
         * \param name name identifier
         * \param parent parent Item
         */
        Item( const QString &name, const QString &title, Item *parent = nullptr );

        /**
         * Destructor
         */
        virtual ~Item();

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
        Item *parent() const;

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
        void addItem( std::unique_ptr<Item> item );

        /**
         * Return child item at \a index position
         */
        Item *getChild( int index ) const;

        /**
         * Returns child item with has the name \a name, nullptr if not found
         */
        template<class T>
        T *getChild( const QString &name ) const { return dynamic_cast<T *>( getChild( name ) ); }

        /**
         * Returns child item with has the name \a name, nullptr if not found
         */
        Item *getChild( const QString &name ) const;

        /**
         * Return last child item
         */
        template<class T>
        T *lastChild() const { return dynamic_cast<T *>( lastChild() ); }

        /**
         * Return last child item
         */
        Item *lastChild() const;

        /**
         * Returns list of child items
         */
        const std::vector<std::unique_ptr<Item>> &childItemList() const;

        /**
         * Returns \a item position in child item list, -1 if it doesn't exist
         */
        long indexOf( Item *item ) const;

        /**
         * Returns children count
         */
        unsigned long childrenCount() const;

        /**
         * Sets \a icon
         */
        void setIcon( const QIcon &icon );

        /**
         * Returns icon
         */
        virtual QIcon icon() const;

        /**
         * Writes XML element to document \a doc as a child of the \a parent element
         */
        void writeXml( QDomDocument &doc, QDomElement &parent ) const;

        /**
         * Reads XML information from element \a elem.
         * Returns error string or an empty string if no error occurred
         */
        QString readXml( const QDomElement &elem );

      protected:
        /**
         * Returns XML tag
         */
        virtual QString xmlTag() const = 0;

        /**
         * Creates child item from \a childElem element
         */
        virtual std::unique_ptr<Item> createChildItem( const QDomElement &childElem );

        QString mName;
        QString mTitle;
        bool mVisible = true;
        Item *mParent = nullptr;
        QMap<QString, Item *> mChildItems; // for name quick access
        std::vector<std::unique_ptr<Item>> mChildItemList;

      private:
        QIcon mIcon;
    };

    /**
     * \brief Represents an action
     */
    class Action : public Item
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        Action( Item *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        Action( const QString &name, const QString &title, Item *parent );

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

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;

      private:
        QAction *mQAction = nullptr;
        qsizetype mQActionIndex = -1;
    };

    /**
     * Represent a Menu
     * Inherits from Action because QMenu are stored within a QAction and we want to keep
     * track of the menu associated action
     */
    class Menu : public Action
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        Menu( Item *parent );
        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        Menu( const QString &name, const QString &title, Item *parent );

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;

      private:
        QIcon mIcon;
    };

    /**
     * Represents a toolbar
     */
    class ToolBar : public Item
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        ToolBar( Item *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        ToolBar( const QString &name, const QString &title, Item *parent );

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

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;

      private:
        // used to backup the original visibility state when we change visibility state
        bool mWasVisible = false;
    };

    /**
     * Root item for all ToolBar item
     */
    class ToolBars : public Item
    {
      public:
        /**
         * Constructor
         */
        ToolBars();

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Root item for all Menus item
     */
    class Menus : public Item
    {
      public:
        /**
         * Constructor
         */
        Menus();

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represent a Dock
     */
    class Dock : public Item
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        Dock( Item *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        Dock( const QString &name, const QString &title, Item *parent );

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

      protected:
        QString xmlTag() const override;
        // used to backup the original visibility state when we change visibility state
        bool mWasVisible = false;
    };

    /**
     * Root item for all dock items
     */
    class Docks : public Item
    {
      public:
        /**
         * Constructor
         */
        Docks();

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represent a QgsBrowserDockWidget item
     */
    class BrowserItem : public Item
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        BrowserItem( Item *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param title title
         * \param parent parent Item
         */
        BrowserItem( const QString &name, const QString &title, Item *parent );

      protected:
        QString xmlTag() const override;
    };

    /**
     * Root item for all browser items
     */
    class BrowserItems : public Item
    {
      public:
        /**
         * Constructor
         */
        BrowserItems();

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Represent a QgsStatusBar widget
     */
    class StatusBarWidget : public Item
    {
      public:
        /**
         * Constructor
         * \param parent parent Item
         */
        StatusBarWidget( Item *parent );

        /**
         * Constructor
         * \param name name identifier
         * \param parent parent Item
         */
        StatusBarWidget( const QString &name, Item *parent );

      protected:
        QString xmlTag() const override;
    };

    /**
     * Root item for all StatusBarWidget
     */
    class StatusBarWidgets : public Item
    {
      public:
        /**
         * Constructor
         */
        StatusBarWidgets();

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
    };

    /**
     * Returns browser items to customize QgsBrowserDockWidget content
     */
    const std::unique_ptr<QgsCustomization::BrowserItems> &browserItems() const;

    /**
     * Returns dock items to customize visible QgsDockWidget
     */
    const std::unique_ptr<QgsCustomization::Docks> &docks() const;

    /**
     * Returns menus items to customize QMenu content
     */
    const std::unique_ptr<QgsCustomization::Menus> &menus() const;

    /**
     * Returns status bar items to customize QgsStatusBar displayed widgets
     */
    const std::unique_ptr<QgsCustomization::StatusBarWidgets> &statusBarWidgets() const;

    /**
     * Returns toolbar items to customize QToolBar content
     */
    const std::unique_ptr<QgsCustomization::ToolBars> &toolBars() const;

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

  protected:
    /**
   * Add action items as children of \a item for each \a widget actions
   */
    void addActions( Item *item, QWidget *widget ) const;

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
     * Writes customization model to XML file \a fileName
     * Returns error string or an empty string if no error occurred
     */
    QString writeXML( const QString &fileName ) const;

    /**
     * Reads customization model from XML file \a fileName
     */
    QString readXml( const QString &fileName );

  private:
    /**
   * Helper class to iterate over widget actions
   */
    class QWidgetIterator
    {
      public:
        /**
         * Constructor
         * \param widget this class will iterate over the widget actions
         */
        QWidgetIterator( QWidget *widget );

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
    static void updateActionVisibility( QgsCustomization::Item *item, QWidget *widget );

    std::unique_ptr<BrowserItems> mBrowserItems;
    std::unique_ptr<Docks> mDocks;
    std::unique_ptr<Menus> mMenus;
    std::unique_ptr<StatusBarWidgets> mStatusBarWidgets;
    std::unique_ptr<ToolBars> mToolBars;
    bool mEnabled = false;
    QString mSplashPath;

    QgisApp *mQgisApp = nullptr;
    QList<QgsBrowserDockWidget *> mBrowserWidgets;
    QString mCustomizationFile;

    friend class TestQgsCustomization;
};
#endif // QGSCUSTOMIZATION_H
