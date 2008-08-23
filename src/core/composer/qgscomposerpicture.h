/***************************************************************************
                         qgscomposerpicture.h
                             -------------------
    begin                : September 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERPICTURE_H
#define QGSCOMPOSERPICTURE_H

#include "qgscomposeritem.h"
#include <QFile>
#include <QImage>
#include <QObject>

/** \ingroup MapComposer
 * A composer class that displays svg files or raster format (jpg, png, ...)
 * */
class CORE_EXPORT QgsComposerPicture: public QObject, public QgsComposerItem
{
  Q_OBJECT
 public:
  QgsComposerPicture(QgsComposition *composition);
  ~QgsComposerPicture();

  /**Reimplementation of QCanvasItem::paint*/
  void paint (QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget);

  /**Sets the source file of the image (may be svg or a raster format)*/
  void setPictureFile(const QString& path);
  QString pictureFile() const;

  /**Sets this items bound in scene coordinates such that 1 item size units
     corresponds to 1 scene size unit*/
  void setSceneRect(const QRectF& rectangle);

  void setRotation(double rotation);

  double rotation() const {return mRotation;}

  /** stores state in Dom node
     * @param node is Dom node corresponding to 'Composer' tag
     * @param temp write template file
     */
    bool writeXML(QDomElement& elem, QDomDocument & doc);

   /** sets state from Dom document
     * @param itemElem is Dom node corresponding to item tag
     */
    bool readXML(const QDomElement& itemElem, const QDomDocument& doc); 

 private:

  enum Mode //SVG or raster graphic format
    {
      SVG,
      RASTER,
      UNKNOWN
    };

  //default constructor is forbidden
  QgsComposerPicture();
  /**Updates content of current image using svg generator*/
  void updateImageFromSvg();

  QImage mImage;
  double mRotation;
  QFile mSourceFile;
  Mode mMode;
  /**False if image needs to be rendered from svg*/
  bool mSvgCacheUpToDate;
  int mCachedDpi; //store dpis for which the svg cache is valid
  QSize mDefaultSvgSize;

 signals:
  /**Tell the configuration widget that the settings need to be updated*/
  void settingsChanged();
};

#endif
