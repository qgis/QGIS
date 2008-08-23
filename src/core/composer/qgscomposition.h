/***************************************************************************
                              qgscomposition.h 
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSITION_H
#define QGSCOMPOSITION_H

#include <QGraphicsScene>

class QgsComposerItem;
class QgsComposerMap;
class QGraphicsRectItem;
class QgsMapRenderer;

class QDomDocument;
class QDomElement;

/** \ingroup MapComposer
 * Graphics scene for map printing. It manages the paper item which always 
 * is the item in the back (z-value 0).
 * */
class CORE_EXPORT QgsComposition: public QGraphicsScene
{
 public:

  /** \brief Plot type */
  enum PlotStyle 
    {
      Preview = 0, // Use cache etc
      Print,       // Render well
      Postscript   // Fonts need different scaling!
    };

  QgsComposition(QgsMapRenderer* mapRenderer);
  ~QgsComposition();

  /**Changes size of paper item*/
  void setPaperSize(double width, double height);

  /**Returns height of paper item*/
  double paperHeight() const;

  /**Returns width of paper item*/
  double paperWidth() const;

  /**Returns the topmose composer item. Ignores mPaperItem*/
  QgsComposerItem* composerItemAt(const QPointF & position);

  QList<QgsComposerItem*> selectedComposerItems();

  /**Returns pointers to all composer maps in the scene*/
  QList<const QgsComposerMap*> composerMapItems() const;

  /**Returns the composer map with specified id
   @return id or 0 pointer if the composer map item does not exist*/
  const QgsComposerMap* getComposerMapById(int id) const;

  int printoutResolution() const {return mPrintoutResolution;}
  void setPrintoutResolution(int dpi){mPrintoutResolution = dpi;}

  /**Returns pointer to map renderer of qgis map canvas*/
  QgsMapRenderer* mapRenderer(){return mMapRenderer;}

  QgsComposition::PlotStyle plotStyle() const {return mPlotStyle;}
  void setPlotStyle(QgsComposition::PlotStyle style) {mPlotStyle = style;}

  /**Returns the pixel font size for a font that has point size set.
   The result depends on the resolution (dpi) and of the preview mode. Each item that sets 
  a font should call this function before drawing text*/
  int pixelFontSize(double pointSize) const;

  /**Does the inverse calculation and returns points for pixels (equals to mm in QgsComposition)*/
  double pointFontSize(int pixelSize) const;

  /**Writes settings to xml (paper dimension)*/
  bool writeXML(QDomElement& composerElem, QDomDocument& doc);

  /**Reads settings from xml file*/
  bool readXML(const QDomElement& compositionElem, const QDomDocument& doc);

 private:
  /**Pointer to map renderer of QGIS main map*/
  QgsMapRenderer* mMapRenderer;
  QgsComposition::PlotStyle mPlotStyle;
  QGraphicsRectItem* mPaperItem;

  /**Dpi for printout*/
  int mPrintoutResolution;

  QgsComposition(); //default constructor is forbidden
};

#endif 



