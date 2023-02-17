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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerstyle.h"

#include <QByteArray>
#include <QMap>
#include <QStringList>
#include <QObject>


class QDomElement;

class QgsMapLayer;

/**
 * \ingroup core
 * \brief Management of styles for use with one map layer
 *
 * Stored styles are identified by their names. The manager
 * always keep track of which style of the stored ones is currently active. When the current style is changed,
 * the new style is applied to the associated layer.
 *
 * The class takes care of updating itself when the layer's current style configuration changes.
 * When some of layer style's properties change (e.g. opacity / colors), the style manager will
 * record them in the currently active style without any extra effort required.
 *
 * When an instance is created, it creates "default" style (with empty name) recorded from the associated map layer
 * and it is set as the current style. The instance must always contain at least one style (which is the current).
 *
 * The style which is marked as current has no style data stored in its entry. This is to avoid duplication
 * of data as all data is stored in map layer and can be retrieved easily. Also when saving, the entry for
 * the current style will be saved with no data because everything is already saved by the map layer itself.
 *
 * The class also features support for temporary change of the layer's style, ideal for short-term use of a custom
 * style followed by restoration of the original style (for example, when rendering a map with a different than current style).
 *
 * \since QGIS 2.8
 */
class CORE_EXPORT QgsMapLayerStyleManager : public QObject
{
    Q_OBJECT
  public:

    /**
     * Construct a style manager associated with a map layer (must not be NULLPTR).
     * The style manager will be parented to \a layer.
     */
    QgsMapLayerStyleManager( QgsMapLayer *layer SIP_TRANSFERTHIS );

    //! Gets pointer to the associated map layer
    QgsMapLayer *layer() const { return mLayer; }

    //! Reset the style manager to a basic state - with one default style which is set as current
    void reset();

    //! Read configuration (for project loading)
    void readXml( const QDomElement &mgrElement );
    //! Write configuration (for project saving)
    void writeXml( QDomElement &mgrElement ) const;

    //! Returns list of all defined style names
    QStringList styles() const;

    /**
     * Gets available styles for the associated map layer.
     * \returns A map of map layer style by style name
     * \since QGIS 3.0
     */
    QMap<QString, QgsMapLayerStyle> mapLayerStyles() const;

    //! Returns data of a stored style - accessed by its unique name
    QgsMapLayerStyle style( const QString &name ) const;

    /**
     * Add a style with given name and data
     * \returns TRUE on success (name is unique and style is valid)
     */
    bool addStyle( const QString &name, const QgsMapLayerStyle &style );

    /**
     * Add style by cloning the current one
     * \returns TRUE on success
     */
    bool addStyleFromLayer( const QString &name );

    /**
     * Remove a stored style
     * \returns TRUE on success (style exists and it is not the last one)
     */
    bool removeStyle( const QString &name );

    /**
     * Rename a stored style to a different name
     * \returns TRUE on success (style exists and new name is unique)
     */
    bool renameStyle( const QString &name, const QString &newName );

    //! Returns name of the current style
    QString currentStyle() const;

    /**
     * Set a different style as the current style - will apply it to the layer
     * \returns TRUE on success
     */
    bool setCurrentStyle( const QString &name );

    /**
     * Temporarily apply a different style to the layer. The argument
     * can be either a style name or a full QML style definition.
     * Each call must be paired with restoreOverrideStyle()
     */
    bool setOverrideStyle( const QString &styleDef );
    //! Restore the original store after a call to setOverrideStyle()
    bool restoreOverrideStyle();

    /**
     * Returns TRUE if this is the default style
     *
     * \since QGIS 3.0
     */
    static bool isDefault( const QString &styleName );

    /**
     * Copies all styles from \a other.
     * In case there is already a style with the same name it will be overwritten.
     *
     * \since QGIS 3.14
     */
    void copyStylesFrom( QgsMapLayerStyleManager *other );

  signals:
    //! Emitted when a new style has been added
    void styleAdded( const QString &name );
    //! Emitted when a style has been removed
    void styleRemoved( const QString &name );
    //! Emitted when a style has been renamed
    void styleRenamed( const QString &oldName, const QString &newName );
    //! Emitted when the current style has been changed
    void currentStyleChanged( const QString &currentName );

  private:
    QgsMapLayer *mLayer = nullptr;
    QMap<QString, QgsMapLayerStyle> mStyles;
    QString mCurrentStyle;
    QgsMapLayerStyle *mOverriddenOriginalStyle = nullptr;
    static QString defaultStyleName();
};

#endif // QGSMAPLAYERSTYLEMANAGER_H
