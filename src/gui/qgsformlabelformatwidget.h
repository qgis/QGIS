/***************************************************************************
  qgsformlabelformatwidget.h - QgsFormLabelFormatWidget

 ---------------------
 begin                : 22.4.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFORMLABELFORMATWIDGET_H
#define QGSFORMLABELFORMATWIDGET_H

#include "ui_qgsformlabelformatwidget.h"
#include "qgsconditionalstyle.h"
#include "qgis_gui.h"

#include <QColor>
#include <QFont>

/**
 * \ingroup gui
 * \class QgsFormLabelFormatWidget
 * \brief A widget for customizing drag and drop form labels and tabs.
 * \since QGIS 2.26
 */
class QgsFormLabelFormatWidget : public QWidget, private Ui::QgsFormLabelFormatWidget
{
    Q_OBJECT
  public:
    explicit QgsFormLabelFormatWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setColor( const QColor &color );

    void setFont( const QFont &font );

    QColor color() const;

    QFont font() const;

};

#endif // QGSFORMLABELFORMATWIDGET_H
