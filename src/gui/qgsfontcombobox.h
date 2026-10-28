/***************************************************************************
    qgsfontcombobox.h
     --------------------------------------
    Date                 : July 2026
    Copyright            : (C) 2026 Nyall Dawson
    Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFONTCOMBOBOX_H
#define QGSFONTCOMBOBOX_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QFontComboBox>

/**
 * \ingroup gui
 * \brief A QFontComboBox with improved usability.
 *
 * \since QGIS 4.2
 */
class GUI_EXPORT QgsFontComboBox : public QFontComboBox
{
#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsFontComboBox *>( sipCpp ) )
      sipType = sipType_QgsFontComboBox;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT

  public:
    /**
     * Constructor for QgsFontComboBox, with the specified \a parent widget.
     */
    explicit QgsFontComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );
};

#endif // QGSFONTCOMBOBOX_H
