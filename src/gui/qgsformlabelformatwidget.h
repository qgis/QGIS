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


// We don't want to expose this in the public API
#define SIP_NO_FILE

/// @cond private

#include "ui_qgsformlabelformatwidget.h"
#include "qgsconditionalstyle.h"
#include "qgsattributeeditorelement.h"
#include "qgis_gui.h"

#include <QColor>
#include <QFont>

/**
 * \ingroup gui
 * \class QgsFormLabelFormatWidget
 * \brief A widget for customizing drag and drop form labels and tabs font and color.
 * \note This class is not a part of public API
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsFormLabelFormatWidget : public QWidget, private Ui::QgsFormLabelFormatWidget
{
    Q_OBJECT
  public:

    /**
     * Creates a QgsFormLabelFormatWidget with given \a parent.
     */
    explicit QgsFormLabelFormatWidget( QWidget *parent = nullptr );

    /**
     * Sets the widget style to \a labelStyle.
     */
    void setLabelStyle( const QgsAttributeEditorElement::LabelStyle &labelStyle );

    /**
     * Returns the current label style from the wiget.
     */
    QgsAttributeEditorElement::LabelStyle labelStyle( ) const;

};

/// @endcond private

#endif // QGSFORMLABELFORMATWIDGET_H
