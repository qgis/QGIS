/***************************************************************************
                         qgscomposeritemgroup.h
                         ----------------------
    begin                : 2nd June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposeritem.h"
#include <QSet>

/** \ingroup MapComposer
 * A container for grouping several QgsComposerItems
 */
class CORE_EXPORT QgsComposerItemGroup: public QgsComposerItem
{
    Q_OBJECT
  public:
    QgsComposerItemGroup( QgsComposition* c );
    ~QgsComposerItemGroup();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const { return ComposerItemGroup; }

    /**Adds an item to the group. All the group members are deleted
     if the group is deleted*/
    void addItem( QgsComposerItem* item );
    /**Removes the items but does not delete them*/
    void removeItems();
    /**Draw outline and ev. selection handles*/
    void paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0 );
    /**Sets this items bound in scene coordinates such that 1 item size units
       corresponds to 1 scene size unit*/
    void setSceneRect( const QRectF& rectangle );

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc is the Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is the Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    QSet<QgsComposerItem*> items() { return mItems; }

  signals:
    void childItemDeleted( QgsComposerItem* item );

  protected:
    void drawFrame( QPainter* p );

  private:
    QSet<QgsComposerItem*> mItems;
    QRectF mSceneBoundingRectangle;
};
