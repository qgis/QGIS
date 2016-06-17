/***************************************************************************
  roadgraphplugin.h
  --------------------------------------
  Date                 : 2010-10-19
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS@list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

#ifndef ROADGRAPH_LINEVECTORLAYERSETTINGSWIDGET_H
#define ROADGRAPH_LINEVECTORLAYERSETTINGSWIDGET_H

#include <QWidget>

class RgLineVectorLayerSettings;

// forward declaration QT-classes
class QComboBox;
class QLineEdit;
class QSpinBox;
class QLineEdit;

// forward declaration Qgis-classes
class QgsVectorLayer;
class QgsMapLayerComboBox;

/**
@author Sergey Yakushev
*/
/**
* \class RgLineVectorLayerSettingsWidget
* \brief
*/
class RgLineVectorLayerSettingsWidget : public QWidget
{
    Q_OBJECT
  public:
    RgLineVectorLayerSettingsWidget( RgLineVectorLayerSettings *s, QWidget* parent = nullptr );

  private slots:
    void on_mcbLayers_selectItem();

  private:
    QgsVectorLayer * selectedLayer();

  public:
    /**
     * list of passible layers
     */
    QgsMapLayerComboBox *mcbLayers;

    /**
     * list of possible fields for use as direction
     */
    QComboBox *mcbDirection;

    /**
     *
     */
    QLineEdit *mleFirstPointToLastPointDirection;

    /**
     *
     */
    QLineEdit *mleLastPointToFirstPointDirection;

    /**
     *
     */
    QLineEdit *mleBothDirection;

    /**
     * default direction value
     */
    QComboBox *mcbDirectionDefault;

    /**
     * list of possible fields for use as speed
     */
    QComboBox *mcbSpeed;

    /**
     * Default speed value
     */
    QSpinBox *msbSpeedDefault;

    /**
     * Unit of speed
     */
    QComboBox *mcbUnitOfSpeed;
};
#endif
