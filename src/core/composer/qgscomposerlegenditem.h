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
#include "qgslegendsymbolitemv2.h"
#include <QStandardItem>

class QDomDocument;
class QDomElement;

class QgsComposerLayerItem;
class QgsLegendSettings;
class QgsMapLayer;
class QgsSymbolV2;
class QgsVectorLayer;


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
      // TODO: unify all item types derived from BaseSymbol item under one value (to allow extensibility)
      SymbologyV2Item,
      RasterSymbolItem,
      RasterImageItem,
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
    QgsComposerLayerItem* parentLayerItem() const;
    QgsMapLayer* parentMapLayer() const;

  protected:
    QgsComposerBaseSymbolItem();

};


class CORE_EXPORT QgsComposerSymbolV2Item : public QgsComposerBaseSymbolItem
{
  public:
    QgsComposerSymbolV2Item();
    QgsComposerSymbolV2Item( const QgsLegendSymbolItemV2& item );
    //! @deprecated
    Q_DECL_DEPRECATED QgsComposerSymbolV2Item( const QString& text );
    //! @deprecated
    Q_DECL_DEPRECATED QgsComposerSymbolV2Item( const QIcon& icon, const QString& text );
    virtual ~QgsComposerSymbolV2Item();

    //! override text + lazy creation of icon
    virtual QVariant data( int role ) const;

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    /** Set symbol (takes ownership)
      @deprecated */
    Q_DECL_DEPRECATED void setSymbolV2( QgsSymbolV2* s );
    /** @deprecated */
    Q_DECL_DEPRECATED QgsSymbolV2* symbolV2() const { return mItem.symbol(); }

    ItemType itemType() const { return SymbologyV2Item; }

    //! @note added in 2.6
    QString ruleKey() const { return mItem.ruleKey(); }

    //! @note added in 2.6
    const QgsLegendSymbolItemV2& itemData() const { return mItem; }

    //! @note added in 2.6
    static QgsComposerSymbolV2Item* findItemByRuleKey( QgsComposerLayerItem* parentLayerItem, QString ruleKey );

  private:

    QgsVectorLayer* parentVectorLayer() const;
    QString label() const;

    QgsLegendSymbolItemV2 mItem;
    mutable QIcon mIcon;
};

class CORE_EXPORT QgsComposerRasterSymbolItem : public QgsComposerBaseSymbolItem
{
  public:
    QgsComposerRasterSymbolItem();
    QgsComposerRasterSymbolItem( const QColor& color, const QString& label );
    //! @deprecated
    Q_DECL_DEPRECATED QgsComposerRasterSymbolItem( const QString& text );
    //! @deprecated
    Q_DECL_DEPRECATED QgsComposerRasterSymbolItem( const QIcon& icon, const QString& text );
    virtual ~QgsComposerRasterSymbolItem();

    virtual QVariant data( int role ) const;

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    //! @deprecated
    Q_DECL_DEPRECATED void setLayerID( const QString& id ) { mLayerID = id; }
    //! @deprecated
    Q_DECL_DEPRECATED QString layerID() const { return mLayerID; }
    ItemType itemType() const { return RasterSymbolItem; }

    void setColor( const QColor& c ) { mColor = c; }
    QColor color() const { return mColor; }

  private:
    QString mLayerID;

    QColor mColor;
    QString mLabel;
};


/**
 * Draws a raster image in the legend
 * @note added in 2.6
 */
class CORE_EXPORT QgsComposerRasterImageItem : public QgsComposerBaseSymbolItem
{
  public:
    QgsComposerRasterImageItem();
    QgsComposerRasterImageItem( const QImage& image );

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    ItemType itemType() const { return RasterImageItem; }

  private:
    QImage mImage;
};


class CORE_EXPORT QgsComposerLayerItem : public QgsComposerLegendItem
{
  public:
    QgsComposerLayerItem();
    QgsComposerLayerItem( const QString& text );
    virtual ~QgsComposerLayerItem();

    virtual QVariant data( int role ) const;

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    ItemType itemType() const { return LayerItem; }

    void setLayerID( const QString& id ) { mLayerID = id; }
    QString layerID() const { return mLayerID; }

    void setShowFeatureCount( bool show ) { mShowFeatureCount = show; }
    bool showFeatureCount() const { return mShowFeatureCount; }

    //! @deprecated does nothing
    Q_DECL_DEPRECATED void setDefaultStyle( double scaleDenominator = -1, QString rule = "" );

    //! convenience method to obtain layer pointed at
    //! @note added in 2.6
    QgsMapLayer* mapLayer() const;

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

};

/**
 * Item used for 2nd column of the legend model for layers and groups to indicate
 * style of the item (e.g. hidden, group, sub-group)
 */
class CORE_EXPORT QgsComposerStyleItem: public QStandardItem
{
  public:
    QgsComposerStyleItem( );
    QgsComposerStyleItem( QgsComposerLegendItem *item );
    ~QgsComposerStyleItem();
};

#endif // QGSCOMPOSERLEGENDITEM_H
