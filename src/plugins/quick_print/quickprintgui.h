/***************************************************************************
  Quick Print is a plugin to quickly print a map with minimal effort.
  -------------------
         begin                : Jan 2008
         copyright            : (c) Tim Sutton, 2008
         email                : tim@linfiniti.com

 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QuickPrintGUI_H
#define QuickPrintGUI_H

#include <QDialog>
#include <qgsmaprender.h>
#include <qgsmapcanvas.h>
#include <ui_quickprintguibase.h>

/**
@author Tim Sutton
*/
class QuickPrintGui : public QDialog, private Ui::QuickPrintGuiBase
{
Q_OBJECT
public:
    QuickPrintGui( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~QuickPrintGui();
    void setMapCanvas(QgsMapCanvas * thepCanvas) {mpMapCanvas = thepCanvas;};;
private:
    static const int context_id = 0;

    enum SymbolScalingType {ScaleUp, ScaleDown};
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

    void renderPrintScaleBar(QPainter * thepPainter, QgsMapCanvas * thepMapCanvas);
    void readSettings();
    void writeSettings();
    QgsMapCanvas * mpMapCanvas;
private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_buttonBox_helpRequested();

};

#endif
