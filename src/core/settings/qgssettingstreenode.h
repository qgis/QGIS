/***************************************************************************
  qgssettingstreenode.h
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

#ifndef QGSSETTINGSTREENODE_H
#define QGSSETTINGSTREENODE_H

#include <QObject>

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"

class QgsSettingsTreeNamedListNode;
class QgsSettingsEntryBase;
class QgsSettingsEntryString;

/**
 * \ingroup core
 * \class QgsSettingsTreeNode
 * \brief QgsSettingsTreeNode is a tree node for the settings tree
 * to help organizing and introspecting the tree.
 *
 * It is either a root node, a normal node or
 * a named list (to store a group of settings under a dynamic named key).
 * to automatically register a settings entry on its creation when a parent is provided.
 *
 * \see QgsSettingsTree
 * \see QgsSettingsEntryBase
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsTreeNode
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast< QgsSettingsTreeNamedListNode * >( sipCpp ) )
      sipType = sipType_QgsSettingsTreeNamedListNode;
    else if ( dynamic_cast< QgsSettingsTreeNode * >( sipCpp ) )
      sipType = sipType_QgsSettingsTreeNode;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_GADGET

  public:

    virtual ~QgsSettingsTreeNode();

    /**
     * Creates a tree root node
     * \note This is not available in Python bindings. Use QgsSettingsTree.createPluginTreeNode instead.
     */
    static QgsSettingsTreeNode *createRootNode() SIP_SKIP;

    /**
     * Creates a normal tree node
     * It will return the existing child node if it exists at the given key
     * \throws QgsSettingsException if a setting exists with the same key
     */
    QgsSettingsTreeNode *createChildNode( const QString &key ) SIP_THROW( QgsSettingsException ) SIP_KEEPREFERENCE;

    /**
     * Creates a named list tree node.
     * This is useful to register groups of settings for several named items (for instance credentials for several named services)
     */
    QgsSettingsTreeNamedListNode *createNamedListNode( const QString &key, const Qgis::SettingsTreeNodeOptions &options = Qgis::SettingsTreeNodeOptions() ) SIP_THROW( QgsSettingsException ) SIP_KEEPREFERENCE;


    //! Returns the type of node
    Qgis::SettingsTreeNodeType type() const {return mType;}

    /**
     * Registers a child setting
     * \param setting the setting to register
     * \param key the key of the setting (not the complete key from its parents)
     * \note Ownership of the setting is transferred
     * \note The registration is automatically done when calling the setting's constructor with the parent argument signature
     * \throws QgsSettingsException if a setting exists with the same key
     */
    void registerChildSetting( const QgsSettingsEntryBase *setting, const QString &key ) SIP_THROW( QgsSettingsException );

    /**
     * Unregisters the child setting
     * \param setting the setting to unregister
     * \param deleteSettingValues if TRUE, the values of the settings will also be deleted
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     */
    void unregisterChildSetting( const QgsSettingsEntryBase *setting, bool deleteSettingValues = false, const QStringList &parentsNamedItems = QStringList() );

    //! Unregisters the child tree \a node
    void unregisterChildNode( QgsSettingsTreeNode *node );

    //! Returns the children nodes
    QList<QgsSettingsTreeNode *> childrenNodes() const {return mChildrenNodes;}

    //! Returns the existing child node if it exists at the given \a key
    QgsSettingsTreeNode *childNode( const QString &key ) const;

    //! Returns the children settings
    QList<const QgsSettingsEntryBase *> childrenSettings() const {return mChildrenSettings;}

    //! Returns the existing child settings if it exists at the given \a key
    const QgsSettingsEntryBase *childSetting( const QString &key ) const;

    //! Returns the parent of the node or nullptr if it does not exists
    QgsSettingsTreeNode *parent() const {return mParent;}

    //! Returns the key of the node (without its parents)
    QString key() const {return mKey;}

    //! Returns the complete key of the node (including its parents)
    QString completeKey() const {return mCompleteKey;}

    //! Returns the number of named nodes in the complete key
    int namedNodesCount() const {return mNamedNodesCount;}

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    const QMetaEnum metaEnum = QMetaEnum::fromType<Qgis::SettingsTreeNodeType>();

    QString str = QStringLiteral( "<QgsSettingsTreeNode (%1): %2>" ).arg( metaEnum.valueToKey( static_cast<int>( sipCpp->type() ) ), sipCpp->key() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

  protected:
    //! Registers a child nodes
    void registerChildNode( QgsSettingsTreeNode *node );

    Qgis::SettingsTreeNodeType mType = Qgis::SettingsTreeNodeType::Root;

  private:

    /**
     * \note This is not available in Python bindings. Use method createNode on an existing tree node.
     * \see QgsSettingsTree.createPluginTreeNode
     */
    QgsSettingsTreeNode() = default SIP_FORCE;

    QgsSettingsTreeNode( const QgsSettingsTreeNode &other ) = default SIP_FORCE;

    //! itilaize the tree node
    void init( QgsSettingsTreeNode *parent, const QString &key );

    friend class QgsSettingsTree;
    friend class QgsSettingsTreeNamedListNode;

    QgsSettingsTreeNode *childNodeAtKey( const QString &key );

    QList<QgsSettingsTreeNode *> mChildrenNodes;
    QList<const QgsSettingsEntryBase *> mChildrenSettings;
    QgsSettingsTreeNode *mParent = nullptr;

    QString mKey;
    QString mCompleteKey;
    int mNamedNodesCount = 0;
};



/**
 * \ingroup core
 * \class QgsSettingsTreeNamedListNode
 * \brief QgsSettingsTreeNamedListNode is a named list tree node for the settings tree
 * to help organizing and introspecting the tree.
 * the named list node is used to store a group of settings under a dynamically named key.
 *
 * \see QgsSettingsTree
 * \see QgsSettingsTreeNode
 * \see QgsSettingsEntryBase
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsSettingsTreeNamedListNode : public QgsSettingsTreeNode
{
  public:
    virtual ~QgsSettingsTreeNamedListNode();

    /**
     *  Returns the list of items
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
    */
    QStringList items( const QStringList &parentsNamedItems = QStringList() ) const SIP_THROW( QgsSettingsException );

    /**
     *  Returns the list of items
     * \param origin can be used to restrict the origin of the setting (local or global)
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
    */
    QStringList items( Qgis::SettingsOrigin origin, const QStringList &parentsNamedItems = QStringList() ) const SIP_THROW( QgsSettingsException );


    /**
     * Sets the selected named item from the named list node
     * \param item the item to set as selected
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
     */
    void setSelectedItem( const QString &item, const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    /**
     * Returns the selected named item from the named list node
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
     */
    QString selectedItem( const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    /**
     * Deletes a named item from the named list node
     * \param item the item to delete
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
     */
    void deleteItem( const QString &item, const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    /**
     * Deletes all items from the named list node
     * \param parentsNamedItems the list of named items in the parent named list (if any)
     * \throws QgsSettingsException if the number of given parent named items doesn't match the complete key definition
     * \since QGIS 3.30.1
     */
    void deleteAllItems( const QStringList &parentsNamedItems = QStringList() ) SIP_THROW( QgsSettingsException );

    //! Returns the setting used to store the selected item
    const QgsSettingsEntryString *selectedItemSetting() const {return mSelectedItemSetting;}

  protected:
    //! Init the nodes with the specific \a options
    void initNamedList( const Qgis::SettingsTreeNodeOptions &options );

  private:
    friend class QgsSettingsTreeNode;

    /**
     * \note This is not available in Python bindings. Use method createNamedListNode on an existing tree node.
     * \see QgsSettingsTree.createPluginTreeNode
     */
    QgsSettingsTreeNamedListNode() = default SIP_FORCE;

    QgsSettingsTreeNamedListNode( const QgsSettingsTreeNamedListNode &other ) = default SIP_FORCE;

    //! Returns the key with named items placeholders filled with args
    QString completeKeyWithNamedItems( const QString &key, const QStringList &namedItems ) const;

    Qgis::SettingsTreeNodeOptions mOptions;
    const QgsSettingsEntryString *mSelectedItemSetting = nullptr;
    QString mItemsCompleteKey;
};

#endif  // QGSSETTINGSTREENODE_H
