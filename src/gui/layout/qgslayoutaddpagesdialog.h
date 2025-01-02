/***************************************************************************
                             qgslayoutaddpagesdialog.h
                             -------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTADDPAGESDIALOG_H
#define QGSLAYOUTADDPAGESDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "ui_qgslayoutnewpagedialog.h"

#include "qgslayoutsize.h"
#include "qgslayoutpoint.h"
#include "qgslayoutitem.h"
#include "qgslayoutmeasurementconverter.h"

/**
 * \ingroup gui
 * \brief A dialog for configuring properties of new pages to be added to a layout
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutAddPagesDialog : public QDialog, private Ui::QgsLayoutNewPageDialog
{
    Q_OBJECT

  public:
    //! Page insertion positions
    enum PagePosition
    {
      BeforePage,
      AfterPage,
      AtEnd
    };

    /**
     * Constructor for QgsLayoutAddPagesDialog.
     */
    QgsLayoutAddPagesDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Sets the \a layout associated with the dialog. This allows the dialog
     * to retrieve properties from the layout and perform tasks like automatic
     * conversion of units.
     */
    void setLayout( QgsLayout *layout );

    /**
     * Returns the number of pages to insert.
     */
    int numberPages() const;

    /**
     * Returns the position at which to insert the new pages.
     */
    PagePosition pagePosition() const;

    /**
     * Returns the page number for which new pages should be inserted before/after.
     */
    int beforePage() const;

    /**
     * Returns the desired page size.
     */
    QgsLayoutSize pageSize() const;

  private slots:

    void positionChanged( int index );
    void pageSizeChanged( int index );
    void orientationChanged( int index );
    void setToCustomSize();
    void showHelp();

  private:
    bool mSettingPresetSize = false;

    QgsLayoutMeasurementConverter mConverter;
};

#endif // QGSLAYOUTADDPAGESDIALOG_H
