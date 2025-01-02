/***************************************************************************
                            qgsannotationitemregistry.h
                            ------------------------
    begin                : October 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSANNOTATIONITEMREGISTRY_H
#define QGSANNOTATIONITEMREGISTRY_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgspathresolver.h"
#include <QGraphicsItem> //for QGraphicsItem::UserType
#include <QIcon>
#include <functional>

class QgsAnnotationItem;
class QDomElement;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief Stores metadata about one annotation item class.
 *
 * A companion class, QgsAnnotationItemAbstractGuiMetadata, handles the
 * GUI behavior of QgsAnnotationItems.
 *
 * \note In C++ you can use QgsAnnotationItemMetadata convenience class.
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationItemAbstractMetadata
{
  public:

    /**
     * Constructor for QgsAnnotationItemAbstractMetadata with the specified class \a type
     * and \a visibleName.
     *
     * The optional \a visiblePluralName argument can be used to specify a plural variant of the item type.
     */
    QgsAnnotationItemAbstractMetadata( const QString &type, const QString &visibleName, const QString &visiblePluralName = QString() )
      : mType( type )
      , mVisibleName( visibleName )
      , mVisibleNamePlural( visiblePluralName.isEmpty() ? visibleName : visiblePluralName )
    {}

    virtual ~QgsAnnotationItemAbstractMetadata() = default;

    /**
     * Returns the unique item type string for the annotation item class.
     */
    QString type() const { return mType; }

    /**
     * Returns a translated, user visible name for the annotation item class.
     * \see visiblePluralName()
     */
    QString visibleName() const { return mVisibleName; }

    /**
     * Returns a translated, user visible name for plurals of the annotation item class (e.g. "Labels" for a "Label" item).
     */
    QString visiblePluralName() const { return mVisibleNamePlural; }

    /**
     * Creates a new, default, annotation item of this class.
     */
    virtual QgsAnnotationItem *createItem() = 0 SIP_FACTORY;

  private:

    QString mType;
    QString mVisibleName;
    QString mVisibleNamePlural;
};

//! Annotation item creation function
typedef std::function<QgsAnnotationItem *()> QgsAnnotationItemCreateFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup core
 * \brief Convenience metadata class that uses static functions to create annotation items and their configuration widgets.
 * \note not available in Python bindings
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationItemMetadata : public QgsAnnotationItemAbstractMetadata
{
  public:

    /**
     * Constructor for QgsAnnotationItemMetadata with the specified class \a type
     * and \a visibleName, and function pointers for the various item creation functions.
     *
     * The \a visiblePluralName argument is used to specify a plural variant of the item type.
     */
    QgsAnnotationItemMetadata( const QString &type, const QString &visibleName, const QString &visiblePluralName,
                               const QgsAnnotationItemCreateFunc &pfCreate )
      : QgsAnnotationItemAbstractMetadata( type, visibleName, visiblePluralName )
      , mCreateFunc( pfCreate )
    {}

    /**
     * Returns the classes' item default creation function.
     */
    QgsAnnotationItemCreateFunc createFunction() const { return mCreateFunc; }

    QgsAnnotationItem *createItem() override { return mCreateFunc ? mCreateFunc() : nullptr; }

  protected:
    QgsAnnotationItemCreateFunc mCreateFunc = nullptr;

};

#endif


/**
 * \ingroup core
 * \class QgsAnnotationItemRegistry
 * \brief Registry of available annotation item types.
 *
 * QgsAnnotationItemRegistry is not usually directly created, but rather accessed through
 * QgsApplication::annotationItemRegistry().
 *
 * A companion class, QgsAnnotationItemGuiRegistry, handles the GUI behavior
 * of annotation items.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsAnnotationItemRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty item registry.
     *
     * QgsAnnotationItemRegistry is not usually directly created, but rather accessed through
     * QgsApplication::annotationItemRegistry().
     *
     * \see populate()
    */
    QgsAnnotationItemRegistry( QObject *parent = nullptr );

    ~QgsAnnotationItemRegistry() override;

    /**
     * Populates the registry with standard item types. If called on a non-empty registry
     * then this will have no effect and will return FALSE.
     */
    bool populate();

    QgsAnnotationItemRegistry( const QgsAnnotationItemRegistry &rh ) = delete;
    QgsAnnotationItemRegistry &operator=( const QgsAnnotationItemRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified item \a type. Returns NULLPTR if
     * a corresponding type was not found in the registry.
     */
    QgsAnnotationItemAbstractMetadata *itemMetadata( const QString &type ) const;

    /**
     * Registers a new annotation item type. Takes ownership of the metadata instance.
     */
    bool addItemType( QgsAnnotationItemAbstractMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of a annotation item given the item \a type.
     */
    QgsAnnotationItem *createItem( const QString &type ) const SIP_FACTORY;

    /**
     * Returns a map of available item types to translated name.
     */
    QMap< QString, QString> itemTypes() const;

  signals:

    /**
     * Emitted whenever a new item type is added to the registry, with the specified
     * \a type and visible \a name.
     */
    void typeAdded( const QString &type, const QString &name );

  private:
#ifdef SIP_RUN
    QgsAnnotationItemRegistry( const QgsAnnotationItemRegistry &rh );
#endif

    QMap<QString, QgsAnnotationItemAbstractMetadata *> mMetadata;

};

#endif //QGSANNOTATIONITEMREGISTRY_H



