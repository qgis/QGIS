/***************************************************************************
                            qgslayoutitemguiregistry.h
                            --------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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
#ifndef QGSLAYOUTITEMGUIREGISTRY_H
#define QGSLAYOUTITEMGUIREGISTRY_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsapplication.h"
#include "qgspathresolver.h"
#include <QGraphicsItem> //for QGraphicsItem::UserType
#include <QIcon>
#include <functional>

#include "qgslayoutitem.h" // temporary

class QgsLayout;
class QgsLayoutView;
class QgsLayoutItem;
class QgsLayoutViewRubberBand;

/**
 * \ingroup gui
 * \brief Stores GUI metadata about one layout item class.
 *
 * This is a companion to QgsLayoutItemAbstractMetadata, storing only
 * the components related to the GUI behavior of a layout item.
 *
 * \note In C++ you can use QgsLayoutItemGuiMetadata convenience class.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutItemAbstractGuiMetadata
{
  public:

    /**
     * Constructor for QgsLayoutItemAbstractGuiMetadata with the specified class \a type.
     */
    QgsLayoutItemAbstractGuiMetadata( int type )
      : mType( type )
    {}

    virtual ~QgsLayoutItemAbstractGuiMetadata() = default;

    /**
     * Returns the unique item type code for the layout item class.
     */
    int type() const { return mType; }

    /**
     * Returns an icon representing creation of the layout item type.
     */
    virtual QIcon creationIcon() const { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddBasicRectangle.svg" ) ); }

    /**
     * Creates a configuration widget for layout items of this type. Can return nullptr if no configuration GUI is required.
     */
    virtual QWidget *createItemWidget() SIP_FACTORY { return nullptr; }

    /**
     * Creates a rubber band for use when creating layout items of this type. Can return nullptr if no rubber band
     * should be created. The default behavior is to create a rectangular rubber band.
     */
    virtual QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) SIP_FACTORY;

  private:

    int mType = -1;
};

//! Layout item configuration widget creation function
typedef std::function<QWidget *()> QgsLayoutItemWidgetFunc SIP_SKIP;

//! Layout rubber band creation function
typedef std::function<QgsLayoutViewRubberBand *( QgsLayoutView * )> QgsLayoutItemRubberBandFunc SIP_SKIP;

#ifndef SIP_RUN

/**
 * \ingroup gui
 * Convenience metadata class that uses static functions to handle layout item GUI behavior.
 * \since QGIS 3.0
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsLayoutItemGuiMetadata : public QgsLayoutItemAbstractGuiMetadata
{
  public:

    /**
     * Constructor for QgsLayoutItemGuiMetadata with the specified class \a type
     * and \a creationIcon, and function pointers for the various
     * configuration widget creation functions.
     */
    QgsLayoutItemGuiMetadata( int type, const QIcon &creationIcon,
                              QgsLayoutItemWidgetFunc pfWidget = nullptr,
                              QgsLayoutItemRubberBandFunc pfRubberBand = nullptr )
      : QgsLayoutItemAbstractGuiMetadata( type )
      , mIcon( creationIcon )
      , mWidgetFunc( pfWidget )
      , mRubberBandFunc( pfRubberBand )
    {}

    /**
     * Returns the classes' configuration widget creation function.
     * \see setWidgetFunction()
     */
    QgsLayoutItemWidgetFunc widgetFunction() const { return mWidgetFunc; }

    /**
     * Sets the classes' configuration widget creation \a function.
     * \see widgetFunction()
     */
    void setWidgetFunction( QgsLayoutItemWidgetFunc function ) { mWidgetFunc = function; }

    /**
     * Returns the classes' rubber band creation function.
     * \see setRubberBandCreationFunction()
     */
    QgsLayoutItemRubberBandFunc rubberBandCreationFunction() const { return mRubberBandFunc; }

    /**
     * Sets the classes' rubber band creation \a function.
     * \see rubberBandCreationFunction()
     */
    void setRubberBandCreationFunction( QgsLayoutItemRubberBandFunc function ) { mRubberBandFunc = function; }

    QIcon creationIcon() const override { return mIcon.isNull() ? QgsLayoutItemAbstractGuiMetadata::creationIcon() : mIcon; }
    QWidget *createItemWidget() override { return mWidgetFunc ? mWidgetFunc() : nullptr; }
    QgsLayoutViewRubberBand *createRubberBand( QgsLayoutView *view ) override { return mRubberBandFunc ? mRubberBandFunc( view ) : nullptr; }

  protected:
    QIcon mIcon;
    QgsLayoutItemWidgetFunc mWidgetFunc = nullptr;
    QgsLayoutItemRubberBandFunc mRubberBandFunc = nullptr;

};

#endif

/**
 * \ingroup core
 * \class QgsLayoutItemGuiRegistry
 * \brief Registry of available layout item GUI behavior.
 *
 * QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
 * QgsGui::layoutItemGuiRegistry().
 *
 * This acts as a companion to QgsLayoutItemRegistry, handling only
 * the components related to the GUI behavior of layout items.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutItemGuiRegistry : public QObject
{
    Q_OBJECT

  public:

    /**
     * Creates a new empty item GUI registry.
     *
     * QgsLayoutItemGuiRegistry is not usually directly created, but rather accessed through
     * QgsGui::layoutItemGuiRegistry().
     *
     * \see populate()
    */
    QgsLayoutItemGuiRegistry( QObject *parent = nullptr );

    ~QgsLayoutItemGuiRegistry();

    /**
     * Populates the registry with standard item types. If called on a non-empty registry
     * then this will have no effect and will return false.
     */
    bool populate();

    //! QgsLayoutItemGuiRegistry cannot be copied.
    QgsLayoutItemGuiRegistry( const QgsLayoutItemGuiRegistry &rh ) = delete;
    //! QgsLayoutItemGuiRegistry cannot be copied.
    QgsLayoutItemGuiRegistry &operator=( const QgsLayoutItemGuiRegistry &rh ) = delete;

    /**
     * Returns the metadata for the specified item \a type. Returns nullptr if
     * a corresponding type was not found in the registry.
     */
    QgsLayoutItemAbstractGuiMetadata *itemMetadata( int type ) const;

    /**
     * Registers the gui metadata for a new layout item type. Takes ownership of the metadata instance.
     */
    bool addLayoutItemGuiMetadata( QgsLayoutItemAbstractGuiMetadata *metadata SIP_TRANSFER );

    /**
     * Creates a new instance of a layout item configuration widget for the specified item \a type.
     */
    QWidget *createItemWidget( int type ) const SIP_FACTORY;

    /**
     * Creates a new rubber band item for the specified item \a type and destination \a view.
     * \note not available from Python bindings
     */
    QgsLayoutViewRubberBand *createItemRubberBand( int type, QgsLayoutView *view ) const SIP_SKIP;

    /**
     * Returns a list of available item types handled by the registry.
     */
    QList< int > itemTypes() const;

  signals:

    /**
     * Emitted whenever a new item type is added to the registry, with the specified
     * \a type.
     */
    void typeAdded( int type );

  private:
#ifdef SIP_RUN
    QgsLayoutItemGuiRegistry( const QgsLayoutItemGuiRegistry &rh );
#endif

    QMap<int, QgsLayoutItemAbstractGuiMetadata *> mMetadata;

};

#endif //QGSLAYOUTITEMGUIREGISTRY_H



