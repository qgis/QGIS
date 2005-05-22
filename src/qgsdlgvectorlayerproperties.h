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
#ifdef WIN32
#include "qgsdlgvectorlayerpropertiesbase.h"
#else
#include "qgsdlgvectorlayerpropertiesbase.uic.h"
#endif
#include "qgsrenderer.h"
#include "qpixmap.h"
#include "qlineedit.h"

class QgsVectorLayer;
class QgsLabelDialog;
class QgsAttributeActionDialog;

class QgsDlgVectorLayerProperties : public QgsDlgVectorLayerPropertiesBase{
  Q_OBJECT
  public:
  QgsDlgVectorLayerProperties(QgsVectorLayer *lyr =0,QWidget *parent=0, const char *name=0, bool modal=true);
  ~QgsDlgVectorLayerProperties();
  /**Sets the legend type to "single symbol", "graduated symbol" or "continuous color"*/
  void setLegendType(QString type);
  /**Returns the display name entered in the dialog*/
  QString displayName();
  /**Sets the attribute that is used in the Identify Results dialog box*/
  void setDisplayField(QString name);
  /**Sets the rendererDirty flag*/
  void setRendererDirty(bool enabled=true);
  /**Returns a pointer to the bufferDialog*/
  QDialog* getBufferDialog();
  /**Sets the buffer dialog*/
  void setBufferDialog(QDialog* dialog);
  /**Returns a pointer to the buffer pixmap*/
  QPixmap* getBufferPixmap();
  /**Returns a pointer to the buffer renderer*/
  QgsRenderer* getBufferRenderer();


  public slots:
  void alterLayerDialog(const QString& string);
  /** Reset to original (vector layer) values */
  void reset();
  /** Get metadata about the layer in nice formatted html */
  QString getMetadata();
  
  
  //
  //methods reimplemented from qt designer base class
  //


  void pbnCancel_clicked();
  void pbnOK_clicked();
  void pbnApply_clicked();
  void btnHelp_clicked();
  void pbnQueryBuilder_clicked();
  void pbnIndex_clicked();
  void pbnChangeSpatialRefSys_clicked();

  protected:
  QgsVectorLayer *layer;
  /**Flag indicating that the render type has changed compared to the vector layer (true)*/
  bool rendererDirty;
  /**Renderer dialog which is shown. If apply is pressed, it assigned to the vector layer*/
  QDialog* bufferDialog;
  /**Buffer renderer, which is assigned to the vector layer when apply is pressed*/
  QgsRenderer* bufferRenderer;
  /**Label dialog. If apply is pressed, options are applied to vector's QgsLabel */
  QgsLabelDialog* labelDialog;
  /**Actions dialog. If apply is pressed, the actions are stored for later use */
  QgsAttributeActionDialog* actionDialog;
  /**Buffer pixmap which takes the picture of renderers before they are assigned to the vector layer*/
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
