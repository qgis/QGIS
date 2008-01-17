/***************************************************************************
    quickprint.h 
    -------------------
    begin                : Jan 21, 2004
    copyright            : (C) 2004 by Tim Sutton
    email                : tim@linfiniti.com
  
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 /*  $Id: plugin.h 7796 2007-12-16 22:11:38Z homann $ */
#ifndef QGSQUICKPRINT_H
#define QGSQUICKPRINT_H

//QT4 includes
#include <QObject>

//QGIS includes
#include <qgsmaprender.h>
#include <qgsmapcanvas.h>


/**
* \class QgsQgsQuickPrint
* \brief A convenience class for quickly printing a map.
* Prints a map with a map title, scale bar, north arrow, legend etc.
*/
class GUI_EXPORT QgsQuickPrint:public QObject
{
  Q_OBJECT;
  public:
      
  QgsQuickPrint();
  //! Destructor
  virtual ~QgsQuickPrint();
  enum SymbolScalingType {ScaleUp, ScaleDown};

public slots:
  void printMap(); 
  void setTitle(QString theText);
  void setName(QString theText);
  void setCopyright(QString theText);
  void setNorthArrow(QString theFileName);
  void setLogo1(QString theFileName);
  void setLogo2(QString theFileName);
  void setOutputPdf(QString theFileName);
  void setMapCanvas(QgsMapCanvas * thepMapCanvas);
private:
  void renderPrintScaleBar(QPainter * thepPainter, QgsMapCanvas * thepMapCanvas);
  /**
   * Scale symbols in all layers by the specified amount.
   * Typically used for printing. Each symbol in 
   * each layer of the active mapcanvas will be iterated 
   * to. If the symbol is a point symbol its size 
   * will be multiplied by the scale factor or divided. In order
   * to choose an appropriate scale factor, typically
   * you should divide the print resolution by the 
   * screen resolution (often 72dpi or 96dpi).
   * @param theScaleFactor - amount by which symbol sizes
   * will be multiplied.
   * @param SymbolScalingType - whether the sizes should
   * be scaled up or down.
   * @see scaleTextLabels
   */
  void scalePointSymbols( int theScaleFactor, SymbolScalingType theDirection );
  /**
   * Scale text labels in all layers by the specified amount.
   * Typically used for printing. Each label in 
   * each layer of the active mapcanvas will be iterated 
   * to. The font point size 
   * will be multiplied by the scale factor or divided. In order
   * to choose an appropriate scale factor, typically
   * you should divide the print resolution by the 
   * screen resolution (often 72dpi or 96dpi).
   * @param theScaleFactor - amount by which symbol sizes
   * will be multiplied.
   * @param SymbolScalingType - whether the sizes should
   * be scaled up or down.
   * @see scalePointSymbols
   */
  void scaleTextLabels( int theScaleFactor, SymbolScalingType theDirection);

  QgsMapCanvas * mpMapCanvas;
  QString mTitleText;
  QString mNameText;
  QString mCopyrightText;
  QString mNorthArrowFile;
  QString mLogo1File;
  QString mLogo2File;
  QString mOutputFileName;
};

#endif //QGSQUICKPRINT_H
