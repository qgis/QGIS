#ifndef QGSMAPLAYERSTYLEMANAGER_H
#define QGSMAPLAYERSTYLEMANAGER_H


class QgsMapLayer;


#include <QByteArray>
#include <QMap>
#include <QStringList>

class QDomElement;

/** stores style information (renderer, transparency, labeling, diagrams etc.) applicable to a map layer */
class QgsMapLayerStyle
{
  public:
    QgsMapLayerStyle(); // consutrct invalid style

    bool isValid() const;

    QString dump() const; // for debugging only

    void loadFromLayer( QgsMapLayer* layer );
    void applyToLayer( QgsMapLayer* layer ) const;

    void readXml( const QDomElement& styleElement );
    void writeXml( QDomElement& styleElement ) const;

  private:
    QByteArray mXmlData;
};


/** Management of styles for use with one map layer */
class QgsMapLayerStyleManager
{
  public:
    QgsMapLayerStyleManager( QgsMapLayer* layer );

    void readXml( const QDomElement& mgrElement );
    void writeXml( QDomElement& mgrElement ) const;

    QStringList styles() const;
    QgsMapLayerStyle style( const QString& name ) const;

    bool addStyle( const QString& name, const QgsMapLayerStyle& style );
    //! Add style by cloning the current one
    bool addStyleFromLayer( const QString& name );
    bool removeStyle( const QString& name );

    QString currentStyle() const;
    bool setCurrentStyle( const QString& name ); // applies to the mLayer! (+ sync previous style)

  private:
    void syncCurrentStyle();
    void ensureCurrentInSync() const;

  private:
    QgsMapLayer* mLayer;
    QMap<QString, QgsMapLayerStyle> mStyles;
    QString mCurrentStyle;
};

#endif // QGSMAPLAYERSTYLEMANAGER_H
