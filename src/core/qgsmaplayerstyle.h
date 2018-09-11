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
    Q_GADGET
  public:

    /**
     * Categories of style to distinguish appropriate sections for import/export
     * \since QGIS 3.4
     */
    enum StyleCategory
    {
      LayerConfiguration = 1 << 0, //!< Flags, display expression, read-only
      Symbology          = 1 << 1,
      Labels             = 1 << 2,
      Fields             = 1 << 3, //!< Aliases, WMS/WFS, expressions, constraints, virtual fields
      Forms              = 1 << 4,
      Actions            = 1 << 5,
      MapTips            = 1 << 6,
      Diagrams           = 1 << 2,
      AttributeTable     = 7 << 8,
      Rendering          = 1 << 9, //!< Scale visibility, simplify method, opacity
      CustomProperties   = 1 << 10,
      All = LayerConfiguration | Symbology | Labels | Fields | Forms | Actions |
            MapTips | Diagrams | AttributeTable | Rendering | CustomProperties,
    };
    Q_ENUM( StyleCategory )
    Q_DECLARE_FLAGS( StyleCategories, StyleCategory )
    Q_FLAG( StyleCategories )

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

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsMapLayerStyle::StyleCategories )


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
