/***************************************************************************
                          qgsdlgvectorlayerproperties.h
                   Unified property dialog for vector layers
                             -------------------
    begin                : 2004-01-28
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /* $Id$ */
#ifndef QGSDLGVECTORLAYERPROPERTIES
#define QGSDLGVECTORLAYERPROPERTIES

#include "qgsdlgvectorlayerpropertiesbase.h"
#include "qgsrenderer.h"
#include "qpixmap.h"
#include "qlineedit.h"

class QgsVectorLayer;

class QgsDlgVectorLayerProperties : public QgsDlgVectorLayerPropertiesBase{
  Q_OBJECT
  public:
  QgsDlgVectorLayerProperties(QgsVectorLayer *lyr =0,QWidget *parent=0, const char *name=0);
  ~QgsDlgVectorLayerProperties();
  /**Sets the legend type to "single symbol", "graduated symbol" or "continuous color"*/
  void setLegendType(QString type);
  QString displayName();
  void setRendererDirty(bool enabled=true);
  /**Returns a pointer to the bufferDialog*/
  QDialog* getBufferDialog();
  void setBufferDialog(QDialog* dialog);
  QPixmap* getBufferPixmap();
  QgsRenderer* getBufferRenderer();


  public slots:
  void alterLayerDialog(const QString& string);
  void apply();
  void close();

  protected:
  QgsVectorLayer *layer;
  /**Flag indicating if the render type still has to be changed (true) or not (false)*/
  bool rendererDirty;
  /**Renderer dialog. If the legend type has changed, it is assigned to the vectorlayer if apply or ok are pressed*/
  QDialog* bufferDialog;
  QgsRenderer* bufferRenderer;
  QPixmap bufferPixmap;
};

inline void QgsDlgVectorLayerProperties::setBufferDialog(QDialog* dialog)
{
    bufferDialog=dialog; 
}

inline QPixmap* QgsDlgVectorLayerProperties::getBufferPixmap()
{
    return &bufferPixmap;
}

inline QString QgsDlgVectorLayerProperties::displayName()
{
    return txtDisplayName->text();
}

#endif
