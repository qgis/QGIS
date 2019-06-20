/***************************************************************************
                             qgslayoutguidecollection.h
                             --------------------------
    begin                : July 2017
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
#ifndef QGSLAYOUTGUIDECOLLECTION_H
#define QGSLAYOUTGUIDECOLLECTION_H

#include "qgis_core.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutpoint.h"
#include "qgslayoutitempage.h"
#include "qgslayoutserializableobject.h"
#include <QPen>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QGraphicsLineItem>
#include <memory>

class QgsLayout;
class QgsLayoutPageCollection;
class QDomElement;
class QDomDocument;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsLayoutGuide
 * \brief Contains the configuration for a single snap guide used by a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutGuide : public QObject
{

    Q_OBJECT

  public:

    /**
     * Constructor for a new guide with the specified \a orientation and
     * initial \a position.
     *
     * A layout must be set by calling setLayout() before the guide can be used.
     * Adding the guide to a QgsLayoutGuideCollection will automatically set
     * the corresponding layout for you.
     */
    QgsLayoutGuide( Qt::Orientation orientation, QgsLayoutMeasurement position, QgsLayoutItemPage *page );

    ~QgsLayoutGuide() override;

    /**
     * Returns the layout the guide belongs to.
     * \see setLayout()
     */
    QgsLayout *layout() const;

    /**
     * Sets the \a layout the guide belongs to.
     *
     * \note Adding the guide to a QgsLayoutGuideCollection will automatically set
     * the corresponding layout for you.
     *
     * \see layout()
     */
    void setLayout( QgsLayout *layout );

    /**
     * Returns the guide's orientation.
     */
    Qt::Orientation orientation() const;

    /**
     * Returns the guide's position within the page.
     *
     * The position indicates either the horizontal or vertical position
     * of the guide, depending on the guide's orientation().
     *
     * \see setPosition()
     */
    QgsLayoutMeasurement position() const;

    /**
     * Sets the guide's \a position within the page.
     *
     * The \a position argument indicates either the horizontal or vertical position
     * of the guide, depending on the guide's orientation().
     *
     * \see position()
     */
    void setPosition( QgsLayoutMeasurement position );

    /**
     * Returns the page the guide is contained within.
     *
     * \see setPage()
     */
    QgsLayoutItemPage *page();

    /**
     * Sets the \a page the guide is contained within.
     *
     * \see page()
     */
    void setPage( QgsLayoutItemPage *page );

    /**
     * Updates the position of the guide's line item.
     */
    void update();

    /**
     * Returns the guide's line item.
     */
    QGraphicsLineItem *item();

    /**
     * Returns the guide's position in absolute layout units.
     * \see setLayoutPosition()
     */
    double layoutPosition() const;

    /**
     * Sets the guide's \a position in absolute layout units.
     * \see layoutPosition()
     */
    void setLayoutPosition( double position );

  signals:

    /**
     * Emitted when the guide's position is changed.
     */
    void positionChanged();

  private:

    Qt::Orientation mOrientation = Qt::Vertical;

    //! Horizontal/vertical position of guide on page
    QgsLayoutMeasurement mPosition;

    //! Page
    QPointer< QgsLayoutItemPage > mPage;

    QPointer< QgsLayout > mLayout;

    //! Line item used in scene for guide
    QGraphicsLineItem *mLineItem = nullptr;

};

/**
 * \ingroup core
 * \class QgsLayoutGuideCollection
 * \brief Stores and manages the snap guides used by a layout.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutGuideCollection : public QAbstractTableModel, public QgsLayoutSerializableObject
{

    Q_OBJECT

  public:

    //! Model roles
    enum Roles
    {
      OrientationRole = Qt::UserRole, //!< Guide orientation role
      PositionRole, //!< Guide position role
      UnitsRole, //!< Guide position units role
      PageRole, //!< Guide page role
      LayoutPositionRole, //!< Guide position in layout coordinates
    };

    /**
     * Constructor for QgsLayoutGuideCollection belonging to the specified layout,
     * and linked to the specified \a pageCollection.
     */
    QgsLayoutGuideCollection( QgsLayout *layout, QgsLayoutPageCollection *pageCollection );
    ~QgsLayoutGuideCollection() override;

    QString stringType() const override { return QStringLiteral( "LayoutGuideCollection" ); }
    QgsLayout *layout() override;

    int rowCount( const QModelIndex & ) const override;
    int columnCount( const QModelIndex & ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    bool removeRows( int row, int count, const QModelIndex &parent = QModelIndex() ) override;

    /**
     * Adds a \a guide to the collection. Ownership of the guide is transferred to the
     * collection, and the guide will automatically have the correct layout
     * set.
     */
    void addGuide( QgsLayoutGuide *guide SIP_TRANSFER );

    /**
     * Removes the specified \a guide, and deletes it.
     * \see clear()
     */
    void removeGuide( QgsLayoutGuide *guide );

    /**
     * Sets the absolute \a position (in layout coordinates) for \a guide within the layout.
     */
    void setGuideLayoutPosition( QgsLayoutGuide *guide, double position );

    /**
     * Removes all guides from the collection.
     * \see removeGuide()
     */
    void clear();

    /**
     * Resets all other pages' guides to match the guides from the specified \a sourcePage.
     */
    void applyGuidesToAllOtherPages( int sourcePage );

    /**
     * Updates the position (and visibility) of all guide line items.
     */
    void update();

    /**
     * Returns a list of all guides contained in the collection.
     */
    QList< QgsLayoutGuide * > guides();

    /**
     * Returns the list of guides contained in the collection with the specified
     * \a orientation and on a matching \a page.
     * If \a page is -1, guides from all pages will be returned.
     * \see guidesOnPage()
     */
    QList< QgsLayoutGuide * > guides( Qt::Orientation orientation, int page = -1 );

    /**
     * Returns the list of guides contained on a matching \a page.
     * \see guides()
     */
    QList< QgsLayoutGuide * > guidesOnPage( int page );

    /**
     * Returns TRUE if the guide lines should be drawn.
     * \see setVisible()
     */
    bool visible() const;

    /**
     * Sets whether the guide lines should be \a visible.
     * \see visible()
     */
    void setVisible( bool visible );

    /**
     * Stores the collection's state in a DOM element. The \a parentElement should refer to the parent layout's DOM element.
     * \see readXml()
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Sets the collection's state from a DOM element. collectionElement is the DOM node corresponding to the collection.
     * \see writeXml()
     */
    bool readXml( const QDomElement &collectionElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private slots:

    void pageAboutToBeRemoved( int pageNumber );

  private:

    enum UndoRoles
    {
      Move = 10000,
      Remove = 20000,
    };

    QgsLayout *mLayout = nullptr;
    QgsLayoutPageCollection *mPageCollection = nullptr;

    QList< QgsLayoutGuide * > mGuides;
    int mHeaderSize = 0;

    bool mGuidesVisible = true;
    bool mBlockUndoCommands = false;

    friend class QgsLayoutGuideCollectionUndoCommand;

};


/**
 * \ingroup core
 * \class QgsLayoutGuideProxyModel
 * \brief Filters QgsLayoutGuideCollection models to guides of a single orientation (horizontal or vertical).
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutGuideProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutGuideProxyModel, filtered to guides of the specified \a orientation and \a page only.
     *
     * Page numbers begin at 0.
     */
    explicit QgsLayoutGuideProxyModel( QObject *parent SIP_TRANSFERTHIS, Qt::Orientation orientation, int page );

    /**
     * Sets the current \a page for filtering matching guides. Page numbers begin at 0.
     */
    void setPage( int page );

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const override;

  private:
    Qt::Orientation mOrientation = Qt::Horizontal;
    int mPage = 0;

};

#endif //QGSLAYOUTGUIDECOLLECTION_H
