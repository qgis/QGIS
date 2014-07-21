/***************************************************************************
                         qgscomposerlegenditem.h  -  description
                         ------------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERLEGENDITEM_H
#define QGSCOMPOSERLEGENDITEM_H

#include "qgscomposerlegendstyle.h"
#include <QStandardItem>
class QDomDocument;
class QDomElement;

class QgsLegendSettings;
class ItemContext;

/**Abstract base class for the legend item types*/
class CORE_EXPORT QgsComposerLegendItem: public QStandardItem
{
  public:
    QgsComposerLegendItem( QgsComposerLegendStyle::Style s = QgsComposerLegendStyle::Undefined );
    QgsComposerLegendItem( const QString& text, QgsComposerLegendStyle::Style s = QgsComposerLegendStyle::Undefined );
    QgsComposerLegendItem( const QIcon& icon, const QString& text, QgsComposerLegendStyle::Style s = QgsComposerLegendStyle::Undefined );
    virtual ~QgsComposerLegendItem();

    enum ItemType
    {
      GroupItem = QStandardItem::UserType,
      LayerItem,
      SymbologyV2Item,
      RasterSymbolItem,
      StyleItem
    };

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const = 0;
    /**Read item content from xml
      @param itemElem item to read from
      @param xServerAvailable Read item icons if true (QIcon needs x-server)*/
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true ) = 0;

    virtual ItemType itemType() const = 0;
    virtual QStandardItem* clone() const = 0;

    QgsComposerLegendStyle::Style style() const { return mStyle; }
    void setStyle( QgsComposerLegendStyle::Style style ) { mStyle = style; }

    // Get text defined by user
    virtual QString userText() const { return mUserText; }
    // Set text defined by user
    virtual void setUserText( const QString & text ) { mUserText = text; }

  protected:
    void writeXMLChildren( QDomElement& elem, QDomDocument& doc ) const;

    QgsComposerLegendStyle::Style mStyle;

    // User defined text
    QString mUserText;
};

/**
 * The QgsComposerBaseSymbolItem class is base class for implementations of custom legend items.
 *
 * @note added in 2.6
 */
class CORE_EXPORT QgsComposerBaseSymbolItem : public QgsComposerLegendItem
{
  public:

    struct ItemContext
    {
      //! Painter
      QPainter* painter;
      //! Top-left corner of the legend item
      QPointF point;
      //! offset from the left side where label should start
      double labelXOffset;
    };

    struct ItemMetrics
    {
      QSizeF symbolSize;
      QSizeF labelSize;
    };

    /** Entry point called from QgsLegendRenderer to do the rendering.
     *  Default implementation calls drawSymbol() and drawSymbolText() methods.
     *
     *  If ctx is null, this is just first stage when preparing layout - without actual rendering.
     */
    virtual ItemMetrics draw( const QgsLegendSettings& settings, ItemContext* ctx );

    /**
     * Draws symbol on the left side of the item
     * @param itemHeight Minimal height of the legend item - used for correct positioning when rendering
     * @return Real size of the symbol (may be bigger than "normal" symbol size from settings)
     */
    virtual QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

    /**
     * Draws label on the right side of the item
     * @param symbolSize  Real size of the associated symbol - used for correct positioning when rendering
     * @return Size of the label (may span multiple lines)
     */
    virtual QSizeF drawSymbolText( const QgsLegendSettings& settings, ItemContext* ctx, const QSizeF& symbolSize ) const;

  protected:
    QgsComposerBaseSymbolItem();

};


class QgsSymbolV2;

class CORE_EXPORT QgsComposerSymbolV2Item : public QgsComposerBaseSymbolItem
{
  public:
    QgsComposerSymbolV2Item();
    QgsComposerSymbolV2Item( const QString& text );
    QgsComposerSymbolV2Item( const QIcon& icon, const QString& text );
    virtual ~QgsComposerSymbolV2Item();

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    /**Set symbol (takes ownership)*/
    void setSymbolV2( QgsSymbolV2* s );
    QgsSymbolV2* symbolV2() const {return mSymbolV2;}

    ItemType itemType() const { return SymbologyV2Item; }

    /** Draws a symbol at the current y position and returns the new x position. Returns real symbol height, because for points,
     it is possible that it differs from mSymbolHeight */
    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

  private:
    QgsSymbolV2* mSymbolV2;
};

class CORE_EXPORT QgsComposerRasterSymbolItem : public QgsComposerBaseSymbolItem
{
  public:
    QgsComposerRasterSymbolItem();
    QgsComposerRasterSymbolItem( const QString& text );
    QgsComposerRasterSymbolItem( const QIcon& icon, const QString& text );
    virtual ~QgsComposerRasterSymbolItem();

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    void setLayerID( const QString& id ) { mLayerID = id; }
    QString layerID() const { return mLayerID; }
    ItemType itemType() const { return RasterSymbolItem; }

    void setColor( const QColor& c ) { mColor = c; }
    QColor color() const { return mColor; }

    QSizeF drawSymbol( const QgsLegendSettings& settings, ItemContext* ctx, double itemHeight ) const;

  private:
    QString mLayerID;
    QColor mColor;
};

class CORE_EXPORT QgsComposerLayerItem : public QgsComposerLegendItem
{
  public:
    QgsComposerLayerItem();
    QgsComposerLayerItem( const QString& text );
    virtual ~QgsComposerLayerItem();
    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    ItemType itemType() const { return LayerItem; }

    void setLayerID( const QString& id ) { mLayerID = id; }
    QString layerID() const { return mLayerID; }

    void setShowFeatureCount( bool show ) { mShowFeatureCount = show; }
    bool showFeatureCount() const { return mShowFeatureCount; }

    void setDefaultStyle( double scaleDenominator = -1, QString rule = "" );

    /** Draws a layer item */
    QSizeF draw( const QgsLegendSettings& settings, QPainter* painter = 0, QPointF point = QPointF() );

  private:
    QString mLayerID;
    // Show vector feature counts
    bool mShowFeatureCount;
};

class CORE_EXPORT QgsComposerGroupItem: public QgsComposerLegendItem
{
  public:
    QgsComposerGroupItem();
    QgsComposerGroupItem( const QString& text );
    virtual ~QgsComposerGroupItem();
    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    ItemType itemType() const { return GroupItem; }

    /** Draws a group item.
     * Returns list of sizes of layers and groups including this group.
     */
    QSizeF draw( const QgsLegendSettings& settings, QPainter* painter = 0, QPointF point = QPointF() );

};

class CORE_EXPORT QgsComposerStyleItem: public QStandardItem
{
  public:
    QgsComposerStyleItem( );
    QgsComposerStyleItem( QgsComposerLegendItem *item );
    ~QgsComposerStyleItem();
};

#endif // QGSCOMPOSERLEGENDITEM_H
