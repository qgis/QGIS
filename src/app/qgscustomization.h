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
         * Insert \a item at \a position
         */
        void insertItem( int position, std::unique_ptr<Item> item );

        /**
         * Delete item at \a position
         */
        void deleteItem( int position );

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

        /**
         * Returns this item clone with \a parent as parent item
         */
        virtual std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const = 0;

        /**
         * Item capability
         */
        enum class ItemCapability : int
        {
          None = 0,                  //! No capability
          UserMenuChild = 1 << 0,    //! Support adding UserMenu item as child
          ActionRefChild = 1 << 1,   //! Support adding ActionRef as child
          UserToolBarChild = 1 << 2, //! Support adding UserToolBar as child
          Rename = 1 << 3,           //! Support renaming
          Delete = 1 << 4,           //! Support delete
          Drag = 1 << 5              //! Support dragging for later droping
        };

        /**
         * Returns TRUE if \a pcapability is active
         */
        bool hasCapability( ItemCapability pcapability ) const;

      protected:
        /**
         * Returns XML tag
         */
        virtual QString xmlTag() const = 0;

        /**
         * Creates child item from \a childElem element
         */
        virtual std::unique_ptr<Item> createChildItem( const QDomElement &childElem );

        /**
         * Copy \a other item attributes to this item
         */
        virtual void copyItemAttributes( const QgsCustomization::Item *other );

        /**
         * Write item content to XML element \a elem
         */
        virtual void writeXmlItem( QDomElement &elem ) const;

        /**
         * Read item content from XML element \a elem
         */
        virtual void readXmlItem( const QDomElement &elem );

        /**
         * Returns item capabilities
         */
        virtual ItemCapability capabilities() const;

        QString mName;

      private:
        QString mTitle;
        bool mVisible = true;
        Item *mParent = nullptr;
        QIcon mIcon;
        QMap<QString, Item *> mChildItems; // for name quick access
        std::vector<std::unique_ptr<Item>> mChildItemList;
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

        /**
         * Returns action path in the application
         */
        QString path() const;

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
        void copyItemAttributes( const Item *other ) override;
        ItemCapability capabilities() const override;

      private:
        QAction *mQAction = nullptr;
        qsizetype mQActionIndex = -1;
    };

    class ActionRef : public Action
    {
      public:
        ActionRef( Item *parent );
        ActionRef( const QString &name, const QString &title, const QString &path, Item *parent );

        /**
         * Returns referenced action path. Path is a '/' separated list of
         * items name representing the targeted item in its hierarchy
         */
        const QString &path() const;

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement & ) override;
        void readXmlItem( const QDomElement &elem ) override;
        void writeXmlItem( QDomElement &elem ) const override;
        ItemCapability capabilities() const override;
        void copyItemAttributes( const Item *other ) override;

      private:
        QString mPath;
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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
        ItemCapability capabilities() const override;
    };

    class UserMenu : public Menu
    {
      public:
        UserMenu( Item *parent )
          : Menu( parent )
        {}
        UserMenu( const QString &name, const QString &title, Item *parent )
          : Menu( name, title, parent )
        {}

        ItemCapability capabilities() const override
        {
          return static_cast<ItemCapability>(
            static_cast<int>( ItemCapability::ActionRefChild )
            | static_cast<int>( ItemCapability::UserMenuChild )
            | static_cast<int>( ItemCapability::Rename )
            | static_cast<int>( ItemCapability::Delete )
          );
        }

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override
        {
          auto clone = std::make_unique<QgsCustomization::UserMenu>( parent );
          clone->copyItemAttributes( this );
          return clone;
        }

      protected:
        QString xmlTag() const override
        {
          return QStringLiteral( "UserMenu" );
        }

        void writeXmlItem( QDomElement &elem ) const override
        {
          elem.setAttribute( QStringLiteral( "title" ), title() );
        };

        void readXmlItem( const QDomElement &elem ) override
        {
          setTitle( elem.attribute( QStringLiteral( "title" ) ) );
        };

        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override
        {
          if ( childElem.tagName() == QStringLiteral( "ActionRef" ) )
            return std::make_unique<ActionRef>( this );
          else if ( childElem.tagName() == QStringLiteral( "UserMenu" ) )
            return std::make_unique<UserMenu>( this );
          else
            return nullptr;
        }
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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
        void copyItemAttributes( const Item *other ) override;

      private:
        // used to backup the original visibility state when we change visibility state
        bool mWasVisible = false;
    };

    class UserToolBar : public ToolBar
    {
      public:
        UserToolBar( Item *parent )
          : ToolBar( parent )
        {}
        UserToolBar( const QString &name, const QString &title, Item *parent )
          : ToolBar( name, title, parent )
        {}

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override
        {
          auto clone = std::make_unique<QgsCustomization::UserToolBar>( parent );
          clone->copyItemAttributes( this );
          return clone;
        }

      protected:
        QString xmlTag() const override
        {
          return QStringLiteral( "UserToolBar" );
        }

        void writeXmlItem( QDomElement &elem ) const override
        {
          elem.setAttribute( QStringLiteral( "title" ), title() );
        };

        void readXmlItem( const QDomElement &elem ) override
        {
          setTitle( elem.attribute( QStringLiteral( "title" ) ) );
        };

        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override
        {
          if ( childElem.tagName() == QStringLiteral( "ActionRef" ) )
            return std::make_unique<ActionRef>( this );
          else
            return nullptr;
        }

        ItemCapability capabilities() const override
        {
          return static_cast<ItemCapability>(
            static_cast<int>( ItemCapability::ActionRefChild )
            | static_cast<int>( ItemCapability::Rename )
            | static_cast<int>( ItemCapability::Delete )
          );
        }
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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item * = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
        ItemCapability capabilities() const override;
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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item * = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        std::unique_ptr<Item> createChildItem( const QDomElement &childElem ) override;
        ItemCapability capabilities() const override;
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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

      protected:
        QString xmlTag() const override;
        void copyItemAttributes( const Item *other ) override;

      private:
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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item * = nullptr ) const override;

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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item * = nullptr ) const override;

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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item *parent = nullptr ) const override;

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

        std::unique_ptr<QgsCustomization::Item> clone( QgsCustomization::Item * = nullptr ) const override;

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

    /**
     * Returns a menu unique name within the entire application
     */
    QString uniqueMenuName() const;

    /**
     * Returns a tool bar unique name within the entire application
     */
    QString uniqueToolBarName() const;

    /**
     * Returns an action  unique name within the entire application
     */
    QString uniqueActionName( const QString &originalActionName ) const;

    /**
     * Returns customization item according to its \a path. \a path is a '/' separated list of
     * items name representing the returned item in its hierarchy
     * Returns nullptr if the item is not found or not the appropriate type
     */
    template<class T>
    T *getItem( const QString &path ) const
    {
      return dynamic_cast<T *>( getItem( path ) );
    }

    /**
     * Returns customization item according to its \a path. \a path is a '/' separated list of
     * items name representing the returned item in its hierarchy
     * Returns nullptr if the item is not found
     */
    QgsCustomization::Item *getItem( const QString &path ) const;

  private:
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
    template<class WidgetType>
    static void updateMenuActionVisibility( QgsCustomization::Item *parentItem, WidgetType *parentWidget );

    QString uniqueItemName( const QString &baseName ) const;
    QAction *findAction( const QString &path ) const;

    std::unique_ptr<BrowserItems> mBrowserItems;
    std::unique_ptr<Docks> mDocks;
    std::unique_ptr<Menus> mMenus;
    std::unique_ptr<StatusBarWidgets> mStatusBarWidgets;
    std::unique_ptr<ToolBars> mToolBars;
    bool mEnabled = false;
    QString mSplashPath;

    QgisApp *mQgisApp = nullptr;
    QString mCustomizationFile;

    friend class TestQgsCustomization;
};
#endif // QGSCUSTOMIZATION_H
