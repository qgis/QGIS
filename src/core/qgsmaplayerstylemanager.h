/***************************************************************************
  qgsmaplayerstylemanager.h
  --------------------------------------
  Date                 : January 2015
  Copyright            : (C) 2015 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSTYLEMANAGER_H
#define QGSMAPLAYERSTYLEMANAGER_H


class QgsMapLayer;

#include <QByteArray>
#include <QMap>
#include <QStringList>

class QDomElement;

/**
 * Stores style information (renderer, transparency, labeling, diagrams etc.) applicable to a map layer.
 *
 * Stored data are considered as opaque - it is not possible to access them directly or modify them - it is
 * only possible to read or write layer's current style.
 *
 * @note added in 2.8
 */
class CORE_EXPORT QgsMapLayerStyle
{
  public:
    //! construct invalid style
    QgsMapLayerStyle();

    //! Tell whether the style is valid (i.e. there is something stored in it)
    bool isValid() const;

    //! Return information about the style - for debugging purposes only
    QString dump() const;

    //! Store layer's active style information in the instance
    void readFromLayer( QgsMapLayer* layer );
    //! Apply stored layer's style information to the layer
    void writeToLayer( QgsMapLayer* layer ) const;

    //! Read style configuration (for project file reading)
    void readXml( const QDomElement& styleElement );
    //! Write style configuration (for project file writing)
    void writeXml( QDomElement& styleElement ) const;

  private:
    QByteArray mXmlData;
};


/**
 * Management of styles for use with one map layer. Stored styles are identified by their names. The manager
 * always keep track of which style of the stored ones is currently active. When the current style is changed,
 * the new style is applied to the associated layer.
 *
 * The class takes care of updating itself when the layer's current style configuration changes.
 * When some of layer style's properties change (e.g. transparency / colors), the style manager will
 * record them in the currently active style without any extra effort required.
 *
 * When an instance is created, it creates "default" style (with empty name) recorded from the associated map layer
 * and it is set as the current style.
 *
 * The instance must always contain at least one style. If no extra styles are wanted, the style manager should get
 * disabled in QgsMapLayer instance.
 *
 * @note added in 2.8
 */
class CORE_EXPORT QgsMapLayerStyleManager
{
  public:
    //! Construct a style manager associated with a map layer (must not be null)
    QgsMapLayerStyleManager( QgsMapLayer* layer );

    //! Read configuration (for project loading)
    void readXml( const QDomElement& mgrElement );
    //! Write configuration (for project saving)
    void writeXml( QDomElement& mgrElement ) const;

    //! Return list of all defined style names
    QStringList styles() const;
    //! Return data of a stored style - accessed by its unique name
    QgsMapLayerStyle style( const QString& name ) const;

    //! Add a style with given name and data
    //! @return true on success (name is unique and style is valid)
    bool addStyle( const QString& name, const QgsMapLayerStyle& style );
    //! Add style by cloning the current one
    //! @return true on success
    bool addStyleFromLayer( const QString& name );
    //! Remove a stored style
    //! @return true on success (style exists and it is not the last one)
    bool removeStyle( const QString& name );

    //! Return name of the current style
    QString currentStyle() const;
    //! Set a different style as the current style - will apply it to the layer
    //! @return true on success
    bool setCurrentStyle( const QString& name );

  private:
    void syncCurrentStyle();
    void ensureCurrentInSync() const;

  private:
    QgsMapLayer* mLayer;
    QMap<QString, QgsMapLayerStyle> mStyles;
    QString mCurrentStyle;
};

#endif // QGSMAPLAYERSTYLEMANAGER_H
