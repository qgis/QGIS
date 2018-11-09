/***************************************************************************
  qgsmaplayersty.h
  --------------------------------------
  Date                 : September 2019
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSTYLE_H
#define QGSMAPLAYERSTYLE_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QByteArray>
#include <QMap>
#include <QStringList>
#include <QObject>


class QDomElement;
class QgsMapLayer;

/**
 * \ingroup core
 * Stores style information (renderer, opacity, labeling, diagrams etc.) applicable to a map layer.
 *
 * Stored data are considered as opaque - it is not possible to access them directly or modify them - it is
 * only possible to read or write layer's current style.
 *
 * \since QGIS 2.8
 */
class CORE_EXPORT QgsMapLayerStyle
{
  public:
    //! construct invalid style
    QgsMapLayerStyle() = default;

    //! construct style from QML definition (XML)
    explicit QgsMapLayerStyle( const QString &xmlData );

    //! Tell whether the style is valid (i.e. there is something stored in it)
    bool isValid() const;

    //! Remove any stored style data (will get invalid)
    void clear();

    //! Returns XML content of the style
    QString xmlData() const;

    //! Store layer's active style information in the instance
    void readFromLayer( QgsMapLayer *layer );
    //! Apply stored layer's style information to the layer
    void writeToLayer( QgsMapLayer *layer ) const;

    //! Read style configuration (for project file reading)
    void readXml( const QDomElement &styleElement );
    //! Write style configuration (for project file writing)
    void writeXml( QDomElement &styleElement ) const;

  private:
    QString mXmlData;
};


/**
 * \ingroup core
 * Restore overridden layer style on destruction.
 *
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsMapLayerStyleOverride
{
  public:

    /**
     * Construct a style override object associated with a map layer.
     * The overridden style will be restored upon object destruction.
     */
    QgsMapLayerStyleOverride( QgsMapLayer *layer )
      : mLayer( layer )
    {
    }

    ~QgsMapLayerStyleOverride();

    /**
     * Temporarily apply a different style to the layer. The argument
     * can be either a style name or a full QML style definition.
     */
    void setOverrideStyle( const QString &style );

  private:

    QgsMapLayer *mLayer = nullptr;
    bool mStyleOverridden = false;
};
#endif // QGSMAPLAYERSTYLE_H
