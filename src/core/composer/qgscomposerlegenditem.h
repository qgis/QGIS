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

#include <QStandardItem>
class QDomDocument;
class QDomElement;

/**Abstract base class for the legend item types*/
class CORE_EXPORT QgsComposerLegendItem: public QStandardItem
{
  public:

    QgsComposerLegendItem();
    QgsComposerLegendItem( const QString& text );
    QgsComposerLegendItem( const QIcon& icon, const QString& text );
    virtual ~QgsComposerLegendItem();

    enum ItemType
    {
      GroupItem = QStandardItem::UserType,
      LayerItem,
      SymbologyItem,
      SymbologyV2Item,
      RasterSymbolItem
    };

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const = 0;
    /**Read item content from xml
      @param itemElem item to read from
      @param xServerAvailable Read item icons if true (QIcon needs x-server)*/
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true ) = 0;

    virtual ItemType itemType() const = 0;
    virtual QStandardItem* clone() const = 0;

  protected:
    void writeXMLChildren( QDomElement& elem, QDomDocument& doc ) const;
};

class QgsSymbol;

class CORE_EXPORT QgsComposerSymbolItem: public QgsComposerLegendItem
{
  public:
    QgsComposerSymbolItem();
    QgsComposerSymbolItem( const QString& text );
    QgsComposerSymbolItem( const QIcon& icon, const QString& text );
    virtual ~QgsComposerSymbolItem();

    virtual QStandardItem* clone() const;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    /**Set symbol (takes ownership)*/
    void setSymbol( QgsSymbol* s );
    QgsSymbol* symbol() {return mSymbol;}

    void setLayerID( const QString& id ) { mLayerID = id; }
    QString layerID() const { return mLayerID; }

    ItemType itemType() const { return SymbologyItem; }

  private:
    QgsSymbol* mSymbol;
    QString mLayerID; //this is needed to read the symbol from XML
};

class QgsSymbolV2;

class CORE_EXPORT QgsComposerSymbolV2Item: public QgsComposerLegendItem
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
    QgsSymbolV2* symbolV2() {return mSymbolV2;}

    ItemType itemType() const { return SymbologyV2Item; }

  private:
    QgsSymbolV2* mSymbolV2;
};

class CORE_EXPORT QgsComposerRasterSymbolItem: public QgsComposerLegendItem
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

    void setColor( const QColor& c ){ mColor = c; }
    QColor color() const { return mColor; }

  private:
    QString mLayerID;
    QColor mColor;
};

class CORE_EXPORT QgsComposerLayerItem: public QgsComposerLegendItem
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

  private:
    QString mLayerID;
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

#endif // QGSCOMPOSERLEGENDITEM_H
