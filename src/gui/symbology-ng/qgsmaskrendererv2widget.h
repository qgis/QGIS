/***************************************************************************
    qgsmaskrendererv2widget.h
    ---------------------
    begin                : April 2014
    copyright            : (C) 2014 Hugo Mercier / Oslandia
    email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMASKRENDERERV2WIDGET_H
#define QGSMASKRENDERERV2WIDGET_H

#include "ui_qgsmaskrendererwidgetbase.h"
#include "qgsmaskrendererv2.h"
#include "qgsrendererv2widget.h"

class QMenu;

/**
 * A widget used represent options of a QgsMaskRendererV2
 */
class GUI_EXPORT QgsMaskRendererV2Widget : public QgsRendererV2Widget, private Ui::QgsMaskRendererWidgetBase
{
  Q_OBJECT

 public:
  /** static creation method
   * @param layer the layer where this renderer is applied
   * @param style
   * @param renderer the mask renderer (will take ownership)
   */
  static QgsRendererV2Widget* create( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

  /** Constructor
   * @param layer the layer where this renderer is applied
   * @param style
   * @param renderer the mask renderer (will take ownership)
   */
  QgsMaskRendererV2Widget( QgsVectorLayer* layer, QgsStyleV2* style, QgsFeatureRendererV2* renderer );

  /** @returns the current feature renderer */
  virtual QgsFeatureRendererV2* renderer();

 protected:
  /** the mask renderer */
  QScopedPointer<QgsMaskRendererV2> mRenderer;
  /** the widget used to represent the mask's embedded renderer */
  QScopedPointer<QgsRendererV2Widget> mEmbeddedRendererWidget;

 private slots:
  void on_mRendererComboBox_currentIndexChanged( int index );
};


#endif // QGSMASKRENDERERV2WIDGET_H
