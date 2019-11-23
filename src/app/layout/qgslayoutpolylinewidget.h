/***************************************************************************
                         qgslayoutpolylinewidget.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTPOLYLINEWIDGET_H
#define QGSLAYOUTPOLYLINEWIDGET_H

#include "ui_qgslayoutpolylinewidgetbase.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitempolyline.h"

/**
 * Input widget for QgsLayoutItemPolyline
 */
class QgsLayoutPolylineWidget: public QgsLayoutItemBaseWidget, private Ui::QgsLayoutPolylineWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsLayoutPolylineWidget( QgsLayoutItemPolyline *polyline );
    void setMasterLayout( QgsMasterLayoutInterface *masterLayout ) override;

  protected:

    bool setNewItem( QgsLayoutItem *item ) override;

  private:
    QPointer< QgsLayoutItemPolyline > mPolyline;
    QgsLayoutItemPropertiesWidget *mItemPropertiesWidget = nullptr;

    void enableStartSvgInputElements( bool enable );
    void enableEndSvgInputElements( bool enable );

  private slots:

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();

    void symbolChanged();
    void arrowStrokeWidthChanged( double d );
    void arrowHeadWidthChanged( double d );
    void arrowHeadFillColorChanged( const QColor &newColor );
    void arrowHeadStrokeColorChanged( const QColor &newColor );
    void startArrowHeadToggled( bool toggled );
    void endArrowHeadToggled( bool toggled );
    void startNoMarkerToggled( bool toggled );
    void endNoMarkerToggled( bool toggled );
    void startSvgMarkerToggled( bool toggled );
    void endSvgMarkerToggled( bool toggled );
    void mStartMarkerLineEdit_textChanged( const QString &text );
    void mEndMarkerLineEdit_textChanged( const QString &text );
    void mStartMarkerToolButton_clicked();
    void mEndMarkerToolButton_clicked();
};

#endif // QGSLAYOUTPOLYLINEWIDGET_H
