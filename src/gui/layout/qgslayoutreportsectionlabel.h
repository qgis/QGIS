/***************************************************************************
                             qgslayoutreportsectionlabel.h
                             -----------------------
    begin                : January 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTREPORTSECTIONLABEL_H
#define QGSLAYOUTREPORTSECTIONLABEL_H

#define SIP_NO_FILE

#include <QGraphicsRectItem>
#include "qgis_gui.h"
#include "qgslayoutview.h"
#include "qgslayout.h"

///@cond PRIVATE

/**
 * \ingroup gui
 * Draws a label describing the current report section within a layout designer view.
 *
 * \note not available in Python bindings
 *
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutReportSectionLabel: public QGraphicsRectItem
{

  public:

    /**
     * Constructor for QgsLayoutReportSectionLabel.
     */
    QgsLayoutReportSectionLabel( QgsLayout *layout, QgsLayoutView *view );

    void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr ) override;

    //! Sets the \a label.
    void setLabel( const QString &label );

  private:

    QPointer< QgsLayout > mLayout;
    QPointer< QgsLayoutView > mView;
    QString mLabel;

};

///@endcond PRIVATE

#endif // QGSLAYOUTREPORTSECTIONLABEL_H
