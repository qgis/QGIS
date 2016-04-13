/***************************************************************************
    qgsnullsymbolrendererwidget.h
    ---------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNULLSYMBOLRENDERERWIDGET_H
#define QGSNULLSYMBOLRENDERERWIDGET_H

#include "qgsrendererv2widget.h"

class QgsNullSymbolRenderer;
class QgsSymbolV2SelectorDialog;

class QMenu;

/** \ingroup gui
 * \class QgsNullSymbolRendererWidget
 * \brief Blank widget for customising QgsNullSymbolRenderer.
 * \note Added in version 2.16
 */

class GUI_EXPORT QgsNullSymbolRendererWidget : public QgsRendererV2Widget
{
    Q_OBJECT

  public:

    //! Creates a new QgsNullSymbolRendererWidget object
    static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

    //! Constructor for QgsNullSymbolRendererWidget
    QgsNullSymbolRendererWidget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );
    ~QgsNullSymbolRendererWidget();

    //! Returns a pointer to the configured renderer
    virtual QgsFeatureRendererV2* renderer();

  protected:

    //! Renderer being configured by the widget
    QgsNullSymbolRenderer* mRenderer;

};


#endif // QGSNULLSYMBOLRENDERERWIDGET_H
