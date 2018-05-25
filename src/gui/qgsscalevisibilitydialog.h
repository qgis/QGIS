/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSSCALEVISIBILITYDIALOG_H
#define QGSSCALEVISIBILITYDIALOG_H

#include <QDialog>
#include "qgis.h"
#include <QGroupBox>
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsScaleRangeWidget;

/**
 * \ingroup gui
 * \class QgsScaleVisibilityDialog
 * A dialog allowing users to enter a scale visibility range.
 */
class GUI_EXPORT QgsScaleVisibilityDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY( bool hasScaleVisibility READ hasScaleVisibility WRITE setScaleVisiblity )
    Q_PROPERTY( double minimumScale READ minimumScale WRITE setMinimumScale )
    Q_PROPERTY( double maximumScale READ maximumScale WRITE setMaximumScale )

  public:

    /**
     * Constructor for QgsScaleVisibilityDialog, with specified dialog \a title. The \a mapCanvas argument
     * can be used to associate the dialog with a map canvas, allowing use of the current map scale
     * within the dialog.
     */
    explicit QgsScaleVisibilityDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &title = QString(), QgsMapCanvas *mapCanvas = nullptr );

    /**
     * Returns true if scale based visibilty is enabled.
     */
    bool hasScaleVisibility() const;

    /**
     * Returns the selected minimum scale, or 0 if minimum scale is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see maximumScale()
     * \see setMinimumScale()
     */
    double minimumScale() const;

    /**
     * Returns the selected maximum scale, or 0 if maximum scale is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see minimumScale()
     * \see setMaximumScale())
     */
    double maximumScale() const;

  public slots:

    /**
     * Set whether scale based visibility is enabled.
     * \see hasScaleVisibility()
     */
    void setScaleVisiblity( bool hasScaleVisibility );

    /**
     * Set the minimum \a scale, or 0 to indicate the minimum is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see minimumScale()
     * \see setMaximumScale()
     */
    void setMinimumScale( double scale );

    /**
     * Set the maximum \a scale, or 0 to indicate the minimum is not set.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see maximumScale()
     * \see setMinimumScale()
     */
    void setMaximumScale( double scale );


  private:
    QGroupBox *mGroupBox = nullptr;
    QgsScaleRangeWidget *mScaleWidget = nullptr;

};

#endif // QGSSCALEVISIBILITYDIALOG_H
