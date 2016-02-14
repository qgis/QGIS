class QgsComposerLegendItem : QStandardItem
{
%TypeHeaderCode
#include <qgscomposerlegenditem.h>
%End

  public:
    QgsComposerLegendItem();
    QgsComposerLegendItem( const QString& text );
    QgsComposerLegendItem( const QIcon& icon, const QString& text );
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
    /** Read item content from xml
      @param itemElem item to read from
      @param xServerAvailable Read item icons if true (QIcon needs x-server)*/
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true ) = 0;

    virtual ItemType itemType() const = 0;
    virtual QStandardItem* clone() const = 0;

    QgsComposerLegendStyle::Style style() const;
    void setStyle( QgsComposerLegendStyle::Style style );

    // Get text defined by user
    virtual QString userText() const;
    // Set text defined by user
    virtual void setUserText( const QString & text );

  protected:
    void writeXMLChildren( QDomElement& elem, QDomDocument& doc ) const;
};


class QgsComposerSymbolV2Item: QgsComposerLegendItem
{
%TypeHeaderCode
#include <qgscomposerlegenditem.h>
%End

  public:
    QgsComposerSymbolV2Item();
    QgsComposerSymbolV2Item( const QString& text );
    QgsComposerSymbolV2Item( const QIcon& icon, const QString& text );
    virtual ~QgsComposerSymbolV2Item();

    virtual QStandardItem* clone() const /Factory/;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    /** Set symbol (takes ownership)*/
    void setSymbolV2( QgsSymbolV2* s /Transfer/ );
    QgsSymbolV2* symbolV2();

    ItemType itemType() const;
};

class QgsComposerRasterSymbolItem : QgsComposerLegendItem
{
%TypeHeaderCode
#include <qgscomposerlegenditem.h>
%End
  public:
    QgsComposerRasterSymbolItem();
    QgsComposerRasterSymbolItem( const QString& text );
    QgsComposerRasterSymbolItem( const QIcon& icon, const QString& text );
    virtual ~QgsComposerRasterSymbolItem();

    virtual QStandardItem* clone() const /Factory/;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );
    ItemType itemType() const;

    void setLayerID( const QString& id );
    QString layerID() const;

    void setColor( const QColor& c );
    QColor color() const;
};

class QgsComposerLayerItem : QgsComposerLegendItem
{
%TypeHeaderCode
#include <qgscomposerlegenditem.h>
%End

  public:
    QgsComposerLayerItem();
    QgsComposerLayerItem( const QString& text );
    virtual ~QgsComposerLayerItem();
    virtual QStandardItem* clone() const /Factory/;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    ItemType itemType() const;

    void setLayerID( const QString& id );
    QString layerID() const;

    void setShowFeatureCount( bool show );
    bool showFeatureCount() const;

    void setDefaultStyle( double scaleDenominator = -1, const QString& rule = "" );
};

class QgsComposerGroupItem : QgsComposerLegendItem
{
%TypeHeaderCode
#include <qgscomposerlegenditem.h>
%End

  public:
    QgsComposerGroupItem();
    QgsComposerGroupItem( const QString& text );
    virtual ~QgsComposerGroupItem();
    virtual QStandardItem* clone() const /Factory/;

    virtual void writeXML( QDomElement& elem, QDomDocument& doc ) const;
    virtual void readXML( const QDomElement& itemElem, bool xServerAvailable = true );

    ItemType itemType() const;
};
