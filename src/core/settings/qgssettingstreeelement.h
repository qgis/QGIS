/***************************************************************************
  qgssettingstreeelement.h
  --------------------------------------
  Date                 : December 2022
  Copyright            : (C) 2022 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSTREEELEMENT_H
#define QGSSETTINGSTREEELEMENT_H

#include <QObject>

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgssettingsregistry.h"

class QgsSettingsTreeNamedListElement;
class QgsSettingsEntryString;


/**
 * \ingroup core
 * \class QgsSettingsTreeElement
 * \brief QgsSettingsTreeElement is a tree element for the settings registry
 * to help organizing and introspecting the registry.
 * It is either a root element, a normal element or
 * a named list (to store a group of settings under a dynamic named key).
 * The root element holds a pointer to a registry (might be null)
 * to automatically register a settings entry on its creation when a parent is provided.
 *
 * \see QgsSettingsEntryBase
 * \see QgsSettingsRegistry
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsTreeElement
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsSettingsTreeNamedListElement * >( sipCpp ) )
      sipType = sipType_QgsSettingsTreeNamedListElement;
    else if ( dynamic_cast< QgsSettingsTreeElement * >( sipCpp ) )
      sipType = sipType_QgsSettingsTreeElement;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_GADGET

  public:
    //! Type of tree element
    enum class Type
    {
      Root, //!< Root Element
      Standard, //!< Normal Element
      NamedList, //! Named List Element
    };
    Q_ENUM( Type )

    //! Options for named list elements
    enum class Option
    {
      NamedListSelectedItemSetting, //!< Creates a setting to store which is the current item
    };

    Q_ENUM( Option )
    Q_DECLARE_FLAGS( Options, Option )
    Q_FLAG( Options )

    virtual ~QgsSettingsTreeElement();

    /**
     * Creates a tree root element
     * \note This is not available in Python bindings. Use QgsSettings.createPluginTreeElement instead.
     */
    static QgsSettingsTreeElement *createRootElement() SIP_SKIP;

    //! Creates a normal tree element
    QgsSettingsTreeElement *createChildElement( const QString &key ) SIP_THROW( QgsSettingsException ) SIP_KEEPREFERENCE;

    /**
     * Creates a named list tree element.
     * This is useful to register groups of settings for several named items (for instance credentials for several named services)
     */
    QgsSettingsTreeNamedListElement *createNamedListElement( const QString &key, const QgsSettingsTreeElement::Options &options = QgsSettingsTreeElement::Options() ) SIP_THROW( QgsSettingsException ) SIP_KEEPREFERENCE;


    //! Returns the type of element
    Type type() const {return mType;}

    /**
     * Registers a child setting
     * \param setting the setting to register
     * \param key the key of the setting (not the complete key from its parents)
     * \note Ownership of the setting is transferred
     * \note This is automatically done when calling the setting's constructor with the parent argument
     */
    void registerChildSetting( QgsSettingsEntryBase *setting, const QString &key ) SIP_THROW( QgsSettingsException );

    /**
     * Unregisters the child setting
     * \param setting the setting to unregister
     * \param deleteSettingValues if TRUE, the values of the settings will also be deleted
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     */
    void unregisterChildSetting( QgsSettingsEntryBase *setting, bool deleteSettingValues = false, const QStringList &parentsNamedItems = QStringList() );

    //! Unregisters the child tree \a element
    void unregisterChildElement( QgsSettingsTreeElement *element );

    //! Returns the children elements
    QList<QgsSettingsTreeElement *> childrenElements() const {return mChildrenElements;}

    //! Returns the existing child element if it exists at the given \a key
    QgsSettingsTreeElement *childElement( const QString &key );

    //! Returns the children settings
    QList<const QgsSettingsEntryBase *> childrenSettings() const {return reinterpret_cast<QList<const QgsSettingsEntryBase *> const &>( mChildrenSettings );}
    // This is not working with SIP, no idea why it tries to make a list of const

    //! Returns the existing child settings if it exists at the given \a key
    QgsSettingsEntryBase *childSetting( const QString &key );

    //! Returns the parent of the element or nullptr if it does not exists
    QgsSettingsTreeElement *parent() const {return mParent;}

    //! Returns the key of the element (without its parents)
    QString key() const {return mKey;}

    //! Returns the complete key of the element (including its parents)
    QString completeKey() const {return mCompleteKey;}

    //! Returns the number of named elements in the complete key
    int namedElementsCount() const {return mNamedElementsCount;}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QMetaEnum metaEnum = QMetaEnum::fromType<QgsSettingsTreeElement::Type>();

    QString str = QStringLiteral( "<QgsSettingsTreeElement (%1): %2>" ).arg( metaEnum.valueToKey( static_cast<int>( sipCpp->type() ) ), sipCpp->key() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:
    //! Registers a child elements
    void registerChildElement( QgsSettingsTreeElement *element ) SIP_THROW( QgsSettingsException );

    Type mType = Type::Root;


  private:

    /**
     * \note This is not available in Python bindings. Use method createElement on an existing tree element.
     * \see QgsSettings.createPluginTreeElement
     */
    QgsSettingsTreeElement() = default SIP_FORCE;

    QgsSettingsTreeElement( const QgsSettingsTreeElement &other ) = default SIP_FORCE;

    //! itilaize the tree element
    void init( QgsSettingsTreeElement *parent, const QString &key );

    friend class QgsSettingsTreeNamedListElement;

    QgsSettingsTreeElement *childElementAtKey( const QString &key );

    QList<QgsSettingsTreeElement *> mChildrenElements;
    QList<QgsSettingsEntryBase *> mChildrenSettings;
    QgsSettingsTreeElement *mParent = nullptr;

    QString mKey;
    QString mCompleteKey;
    int mNamedElementsCount = 0;
};



/**
 * \ingroup core
 * \class QgsSettingsTreeNamedListElement
 * \brief QgsSettingsTreeNamedListElement is a named list tree element for the settings registry
 * to help organizing and introspecting the registry.
 * the named list element is used to store a group of settings under a dynamically named key.
 *
 * \see QgsSettingsTreeElement
 * \see QgsSettingsEntryBase
 * \see QgsSettingsRegistry
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsTreeNamedListElement : public QgsSettingsTreeElement
{
  public:
    virtual ~QgsSettingsTreeNamedListElement();

    /**
     *  Returns the list of items
     * \param parentsNamedItems the list of named items in the parent named list (if any)
    */
    QStringList items( const QStringList &parentsNamedItems = QStringList() ) const SIP_THROW( QgsSettingsException );

    /**
     *  Returns the list of items
     * \param origin can be used to restrict the origin of the setting (local or global)
     * \param parentsNamedItems the list of named items in the parent named list (if any)
    */
    QStringList items( Qgis::SettingsOrigin origin, const QStringList &parentsNamedItems = QStringList() ) const SIP_THROW( QgsSettingsException );


    /**
     * Sets the selected named item from the named list element
     * \param item the item to set as selected
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     */
    void setSelectedItem( const QString &item, const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    /**
     * Returns the selected named item from the named list element
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     */
    QString selectedItem( const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    /**
     * Deletes a named item from the named list element
     * \param item the item to delete
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     */
    void deleteItem( const QString &item, const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    //! Returns the setting used to store the selected item
    QgsSettingsEntryString *selectedItemSetting() const {return mSelectedItemSetting;}

  protected:
    //! Init the elements with the specific \a options
    void initNamedList( const QgsSettingsTreeElement::Options &options );

  private:
    friend class QgsSettingsTreeElement;

    /**
     * \note This is not available in Python bindings. Use method createNamedListElement on an existing tree element.
     * \see QgsSettings.createPluginTreeElement
     */
    QgsSettingsTreeNamedListElement() = default SIP_FORCE;

    QgsSettingsTreeNamedListElement( const QgsSettingsTreeNamedListElement &other ) = default SIP_FORCE;

    //! Returns the key with named items placeholders filled with args
    QString completeKeyWithNamedItems( const QString &key, const QStringList &namedItems ) const;

    QgsSettingsTreeElement::Options mOptions;
    QgsSettingsEntryString *mSelectedItemSetting = nullptr;
    QString mItemsCompleteKey;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsSettingsTreeElement::Options )

#endif  // QGSSETTINGSTREEELEMENT_H
