/***************************************************************************
  qgslayertreemodellegendfullwidthnode.h
  --------------------------------------
  Date                 : March 2017
  Copyright            : (C) 2017 by Stéphane Brunner
  Email                : stephane dot brunner at camptocamp dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEMODELLEGENDFULLWIDTHNODE_H
#define QGSLAYERTREEMODELLEGENDFULLWIDTHNODE_H

#include "qgslayertreemodellegendnode.h"

/**
 * \ingroup core
 * Implementation of legend node interface for displaying arbitrary label without icon.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFullWidthLabelLegendNode : public QgsSimpleLegendNode
{
    Q_OBJECT

  public:

    /**
     * The QgsFullWidthLabelLegendNode constructor
     * \param nodeLayer The node layer
     * \param label The label
     */
    QgsFullWidthLabelLegendNode( QgsLayerTreeLayer *nodeLayer, const QString &label );

    /**
     * Is a full width symbol, Is used when the text is above the legend graphics.
     */
    virtual bool fullWidth() const override { return true; }

};

/**
 * \ingroup core
 * Implementation of legend node interface for displaying preview of vector symbols without label.
 *
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsFullWidthSymbolLegendNode : public QgsSymbolLegendNode
{
    Q_OBJECT

  public:

    /**
     * QgsFullWidthSymbolLegendNode constructor
     * \param nodeLayer The node layer
     * \param item The legend symbol
     * \param parent The parent node
     */
    QgsFullWidthSymbolLegendNode(
      QgsLayerTreeLayer *nodeLayer, const QgsLegendSymbolItem &item,
      QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Is a full width symbol, Is used when the text is above the legend graphics.
     */
    virtual bool fullWidth() const override { return true; }

    /**
     * Is an editable symbol.
     */
    virtual bool editable() const override { return false; }

  protected:

    /**
     * Get the symbol metrics
     * \param settings
     * \param context
     * \param symbol
     * \return width, height, widthOffset, heightOffset
     */
    SymbolMetrics symbolMetrics(
      const QgsLegendSettings &settings, const QgsRenderContext context,
      QgsSymbol *symbol SIP_TRANSFER ) const override;

};

#endif // QGSLAYERTREEMODELLEGENDFULLWIDTHNODE_H
