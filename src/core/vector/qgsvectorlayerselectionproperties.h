/***************************************************************************
                         qgsvectorlayerselectionproperties.h
                         ---------------
    begin                : July 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSVECTORLAYERSELECTIONPROPERTIES_H
#define QGSVECTORLAYERSELECTIONPROPERTIES_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerselectionproperties.h"

#include <QColor>

class QgsSymbol;

/**
 * \class QgsVectorLayerSelectionProperties
 * \ingroup core
 * \brief Implementation of layer selection properties for vector layers.
 *
 * \since QGIS 3.34
 */
class CORE_EXPORT QgsVectorLayerSelectionProperties : public QgsMapLayerSelectionProperties
{
    Q_OBJECT
    Q_PROPERTY( Qgis::SelectionRenderingMode selectionRenderingMode READ selectionRenderingMode WRITE setSelectionRenderingMode NOTIFY selectionRenderingModeChanged )
    Q_PROPERTY( QColor selectionColor READ selectionColor WRITE setSelectionColor NOTIFY selectionColorChanged )

  public:

    /**
     * Constructor for QgsVectorLayerSelectionProperties, with the specified \a parent object.
     */
    QgsVectorLayerSelectionProperties( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsVectorLayerSelectionProperties() override;

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    [[nodiscard]] QgsVectorLayerSelectionProperties *clone() const override SIP_FACTORY;

    /**
     * Returns the selection rendering mode to use for selected features in the layer.
     *
     * \see setSelectionRenderingMode()
     */
    [[nodiscard]] Qgis::SelectionRenderingMode selectionRenderingMode() const;

    /**
     * Sets the selection rendering \a mode to use for selected features in the layer.
     *
     * \see selectionRenderingMode()
     */
    void setSelectionRenderingMode( Qgis::SelectionRenderingMode mode );

    /**
     * Returns the color to use for rendering selected features in the layer.
     *
     * An invalid color indicates that the default (i.e. project level) selection
     * color should be used instead.
     *
     * \see setSelectionColor()
     */
    [[nodiscard]] QColor selectionColor() const;

    /**
     * Sets the \a color to use for rendering selected features in the layer.
     *
     * An invalid \a color indicates that the default (i.e. project level) selection
     * color should be used instead.
     *
     * \see selectionColor()
     */
    void setSelectionColor( const QColor &color );

    /**
     * Returns the symbol used to render selected features in the layer.
     *
     * May be NULLPTR if the default symbol should be used instead.
     *
     * \see setSelectionSymbol()
     */
    [[nodiscard]] QgsSymbol *selectionSymbol() const;

    /**
     * Sets the \a symbol used to render selected features in the layer.
     *
     * Ownership of \a symbol is transferred to the plot. If \a symbol is NULLPTR then
     * the default symbol will be used instead.
     *
     * \see selectionSymbol()
     */
    void setSelectionSymbol( QgsSymbol *symbol SIP_TRANSFER );

  signals:

    /**
     * Emitted whenever the selection rendering mode changes.
     * \since QGIS 4.0
     */
    void selectionRenderingModeChanged();

    /**
     * Emitted whenever the selection color changes.
     * \since QGIS 4.0
     */
    void selectionColorChanged();

    /**
     * Emitted whenever the selection symbol changes.
     * \since QGIS 4.0
     */
    void selectionSymbolChanged();

  private:

    Qgis::SelectionRenderingMode mSelectionRenderingMode = Qgis::SelectionRenderingMode::Default;
    QColor mSelectionColor;
    std::unique_ptr< QgsSymbol > mSelectionSymbol;
};

#endif // QGSVECTORLAYERSELECTIONPROPERTIES_H
