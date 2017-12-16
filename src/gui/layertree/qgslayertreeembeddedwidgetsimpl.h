/***************************************************************************
  qgslayertreeembeddedwidgetsimpl.h
  --------------------------------------
  Date                 : May 2016
  Copyright            : (C) 2016 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYERTREEEMBEDDEDWIDGETSIMPL_H
#define QGSLAYERTREEEMBEDDEDWIDGETSIMPL_H

#include <QWidget>
#include "qgslayertreeembeddedwidgetregistry.h"


class QSlider;
class QTimer;
class QgsMapLayer;

SIP_NO_FILE

///@cond PRIVATE

/**
 * \brief Implementation of simple transparency widget to be used in layer tree view
 *
 * \note private class - not in QGIS API
 */
class QgsLayerTreeOpacityWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsLayerTreeOpacityWidget( QgsMapLayer *layer );

    QSize sizeHint() const override;

    class Provider : public QgsLayerTreeEmbeddedWidgetProvider
    {
      public:
        QString id() const override;
        QString name() const override;
        QgsLayerTreeOpacityWidget *createWidget( QgsMapLayer *layer, int widgetIndex ) override;
        bool supportsLayer( QgsMapLayer *layer ) override;
    };

  public slots:
    void sliderValueChanged( int value );
    void updateOpacityFromSlider();
    void layerTrChanged();

  private:
    QgsMapLayer *mLayer = nullptr;
    QSlider *mSlider = nullptr;
    QTimer *mTimer = nullptr;
};
///@endcond
#endif // QGSLAYERTREEEMBEDDEDWIDGETSIMPL_H
