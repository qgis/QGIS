/***************************************************************************
    qgsalignmentcombobox.h
    ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSALIGNMENTCOMBOBOX_H
#define QGSALIGNMENTCOMBOBOX_H

#include "qgis.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QComboBox>

/**
 * \ingroup gui
 * \class QgsAlignmentComboBox
 * \brief A combo box which allows choice of alignment settings (e.g. left, right, ...).
 *
 * Available alignment choices can be manually specified by calling
 * setAvailableAlignments(), which is useful
 * when only a subset of Qt's alignment options should be exposed.
 *
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsAlignmentComboBox : public QComboBox
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAlignmentComboBox, with the specified parent widget.
     */
    QgsAlignmentComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the available alignment choices shown in the combo box.
     */
    void setAvailableAlignments( Qt::Alignment alignments );

    /**
     * Returns the current alignment choice.
     *
     * \see setCurrentAlignment()
     */
    Qt::Alignment currentAlignment() const;

    /**
     * Sets the current \a alignment choice.
     *
     * \see currentAlignment()
     */
    void setCurrentAlignment( Qt::Alignment alignment );

    /**
     * Returns the current alignment choice as a QGIS horizontal text alignment enum.
     *
     * \see verticalAlignment()
     * \since QGIS 4.0
     */
    Qgis::TextHorizontalAlignment horizontalAlignment() const;

    /**
     * Returns the current alignment choice as a QGIS horizontal text alignment enum.
     *
     * \see verticalAlignment()
     * \since QGIS 4.0
     */
    Qgis::TextVerticalAlignment verticalAlignment() const;

    /**
     * Sets the current \a alignment choice from a QGIS horizontal text alignment enum.
     *
     * \since QGIS 4.0
     */
    void setCurrentAlignment( Qgis::TextHorizontalAlignment alignment );

    /**
     * Sets the current \a alignment choice from a QGIS vertical text alignment enum.
     *
     * \since QGIS 4.0
     */
    void setCurrentAlignment( Qgis::TextVerticalAlignment alignment );

    /**
     * Sets the \a text and \a icon to use for a particular \a alignment option,
     * replacing the default text or icon.
     *
     * If \a text or \a icon is not specified, they will not be changed from the default.
     *
     * \note This must be called after first filtering the available alignment options via setAvailableAlignments().
     */
    void customizeAlignmentDisplay( Qt::Alignment alignment, const QString &text = QString(), const QIcon &icon = QIcon() );

  signals:

    /**
     * Emitted when the alignment is changed.
     */
    void changed();

  private:
    void populate();

    Qt::Alignment mAlignments = Qt::AlignLeft | Qt::AlignHCenter | Qt::AlignRight;
    bool mBlockChanged = false;
};

#endif //QGSALIGNMENTCOMBOBOX_H
