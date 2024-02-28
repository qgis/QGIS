/***************************************************************************
    qgsguiutils.h - Constants used throughout the QGIS GUI.
     ------------
    Date                 : 11-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGUIUTILS_H
#define QGSGUIUTILS_H

#include <QPair>
#include <QWidget>
#include <QStringList>
#include "qgis_gui.h"
#include "qgis.h"

#define SIP_NO_FILE

class QFont;

/**
 * \ingroup gui
 * \namespace QgsGuiUtils
 * \brief The QgsGuiUtils namespace contains constants and helper functions used throughout the QGIS GUI.
 * \note not available in Python bindings
 */
namespace QgsGuiUtils
{

  /**
   * /var ModalDialogFlags
   * /brief Flags used to create a modal dialog (adapted from QMessageBox).
   *
   * Using these flags for all modal dialogs throughout QGIS ensures that
   * for platforms such as the Mac where modal and modeless dialogs have
   * different looks, QGIS modal dialogs will look the same as Qt modal
   * dialogs and all modal dialogs will look distinct from modeless dialogs.
   * Although not the standard Mac modal look, it does lack the minimize
   * control which makes sense only for modeless dislogs.
   *
   * The Qt3 method of creating a TRUE Mac modal dialog is deprecated in Qt4
   * and should not be used due to conflicts with QMessageBox style dialogs.
   *
   * Qt::WindowMaximizeButtonHint is included but will be ignored if
   * the dialog is a fixed size and does not have a size grip.
   */
  static const Qt::WindowFlags ModalDialogFlags = Qt::WindowFlags();

  /**
   * Minimum magnification level allowed in map canvases.
   * \see CANVAS_MAGNIFICATION_MAX
   */
  constexpr double CANVAS_MAGNIFICATION_MIN = 0.1;

  /**
   * Maximum magnification level allowed in map canvases.
   * \see CANVAS_MAGNIFICATION_MAX
   */
  // Must be a factor of 2, so zooming in to max from 100% then zooming back out will result in 100% mag
  constexpr double CANVAS_MAGNIFICATION_MAX = 16.0;

  /**
   * Open files, preferring to have the default file selector be the
   * last one used, if any; also, prefer to start in the last directory
   * associated with filterName.
   *
   * \param filterName the name of the filter; used for persistent store key
   * \param filters    the file filters used for QFileDialog
   * \param selectedFiles string list of selected files; will be empty if none selected
   * \param enc        encoding?
   * \param title      the title for the dialog
   * \param cancelAll  add button to cancel further requests
   * \note
   *
   * Stores persistent settings under /UI/.  The sub-keys will be
   * filterName and filterName + "Dir".
   *
   * Opens dialog on last directory associated with the filter name, or
   * the current working directory if this is the first time invoked
   * with the current filter name.
   *
   * This method returns TRUE if cancel all was clicked, otherwise FALSE
  */
  bool GUI_EXPORT openFilesRememberingFilter( QString const &filterName,
      QString const &filters, QStringList &selectedFiles, QString &enc, QString &title,
      bool cancelAll = false );

  /**
   * A helper function to get an image name from the user. It will nicely
   * provide filters with all available writable image formats.
   * \param parent widget that should act as the parent for the file dialog
   * \param message the message to display to the user
   * \param defaultFilename default file name (empty by default)
   * \returns QPair<QString, QString> where first is the file name and second is
   * the file type
   */
  QPair<QString, QString> GUI_EXPORT getSaveAsImageName( QWidget *parent, const QString &message, const QString &defaultFilename = QString() );

  /**
   * Convenience function for readily creating file filters.
   *
   * Given a long name for a file filter and a regular expression, return
   * a file filter string suitable for use in a QFileDialog::OpenFiles()
   * call.  The regular express, glob, will have both all lower and upper
   * case versions added.
  */
  QString GUI_EXPORT createFileFilter_( QString const &longName, QString const &glob );

  /**
   * Create file filters suitable for use with QFileDialog
   *
   * \param format extension e.g. "png"
   * \returns QString e.g. "PNG format (*.png, *.PNG)"
   */
  QString GUI_EXPORT createFileFilter_( QString const &format );

  /**
   * Show font selection dialog.
   *
   * It is strongly recommended that you do not use this method, and instead use the standard
   * QgsFontButton widget to allow users consistent font selection behavior.
   *
   * \param ok TRUE on ok, FALSE on cancel
   * \param initial initial font
   * \param title optional dialog title
   * \returns QFont the selected fon
   */
  QFont GUI_EXPORT getFont( bool &ok, const QFont &initial, const QString &title = QString() );

  /**
   * Restore the wigget geometry from settings. Will use the objectName() of the widget  and if empty, or keyName is set, will
   * use keyName to save state into settings.
   * \param widget The widget to restore.
   * \param keyName Override for objectName() if needed.
   * \return TRUE if the geometry was restored.
   */
  bool GUI_EXPORT restoreGeometry( QWidget *widget, const QString &keyName = QString() );

  /**
   * Save the wigget geometry into settings. Will use the objectName() of the widget  and if empty, or keyName is set, will
   * use keyName to save state into settings.
   * \param widget The widget to save.
   * \param keyName Override for objectName() if needed.
   */
  void GUI_EXPORT saveGeometry( QWidget *widget, const QString &keyName = QString() );

  /**
   * Creates a key for the given widget that can be used to store related data in settings.
   * Will use objectName() or class name if objectName() is not set. Can be overridden using \a keyName.
   * \param widget The widget to make the key from.
   * \param keyName Override for objectName() if needed. If not set will use objectName()
   * \return A key name that can be used for the widget in settings.
   */
  QString createWidgetKey( QWidget *widget, const QString &keyName = QString() );

  /**
   * Scales an icon size to compensate for display pixel density, making the icon
   * size hi-dpi friendly, whilst still resulting in pixel-perfect sizes for low-dpi
   * displays.
   *
   * \a standardSize should be set to a standard icon size, e.g. 16, 24, 48, etc.
   *
   * \since QGIS 3.6
   */
  int GUI_EXPORT scaleIconSize( int standardSize );

  /**
   * Returns the user-preferred size of a window's toolbar icons.
   * \param dockableToolbar If set to TRUE, the icon size will be returned for dockable window panel's toolbars.
   * \returns a QSize object representing an icon's width and height.
   * \since QGIS 3.8
   */
  QSize GUI_EXPORT iconSize( bool dockableToolbar = false );

  /**
   * Returns dockable panel toolbar icon width based on the provided window toolbar width.
   * \param size Icon size from which the output size will be derived from.
   * \returns a QSize object representing an icon's width and height.
   * \since QGIS 3.8
   */
  QSize GUI_EXPORT panelIconSize( QSize size );

  /**
   * Returns a localized string representation of the \a value with the appropriate number of
   * decimals supported by the \a dataType. Trailing zeroes after decimal separator are not
   * show unless \a displayTrailingZeroes is set.
   * Note that for floating point types the number of decimals may exceed the actual internal
   * precision because the precision is always calculated on the mantissa and the conversion to
   * string interprets the precision as decimal places.
   * \since QGIS 3.16
   */
  QString GUI_EXPORT displayValueWithMaximumDecimals( const Qgis::DataType dataType, const double value, bool displayTrailingZeroes = false );

  /**
   * Returns the maximum number of significant digits a for the given \a rasterDataType.
   * \since QGIS 3.16
   */
  int GUI_EXPORT significantDigits( const Qgis::DataType rasterDataType );

}

/**
 * Temporarily disables updates for a QWidget for the lifetime of the object.
 *
 * When the object is deleted, the updates are re-enabled.
 *
 * \ingroup gui
 * \since QGIS 3.34
 */
class GUI_EXPORT QWidgetUpdateBlocker
{
  public:

    /**
     * Constructor for QWidgetUpdateBlocker. Blocks updates for the specified \a widget.
     *
     * The caller must ensure that \a widget exists for the lifetime of this object.
     */
    QWidgetUpdateBlocker( QWidget *widget );

    //! QWidgetUpdateBlocker cannot be copied
    QWidgetUpdateBlocker( const QWidgetUpdateBlocker &other ) = delete;
    //! QWidgetUpdateBlocker cannot be copied
    QWidgetUpdateBlocker &operator=( const QWidgetUpdateBlocker &other ) = delete;

    ~QWidgetUpdateBlocker();

    /**
     * Releases the update block early (i.e. before this object is destroyed).
     */
    void release();

  private:

    QWidget *mWidget = nullptr;
};

/**
 * Temporarily sets a cursor override for the QApplication for the lifetime of the object.
 *
 * When the object is deleted, the cursor override is removed.
 *
 * \ingroup gui
 * \see QgsTemporaryCursorRestoreOverride
 * \since QGIS 3.2
 */
class GUI_EXPORT QgsTemporaryCursorOverride
{
  public:

    /**
     * Constructor for QgsTemporaryCursorOverride. Sets the application override
     * cursor to \a cursor.
     */
    QgsTemporaryCursorOverride( const QCursor &cursor );

    ~QgsTemporaryCursorOverride();

    /**
     * Releases the cursor override early (i.e. before this object is destroyed).
     */
    void release();

  private:

    bool mHasOverride = true;

};

/**
 * Temporarily removes all cursor overrides for the QApplication for the lifetime of the object.
 *
 * When the object is deleted, all stacked cursor overrides are restored.
 *
 * \ingroup gui
 * \see QgsTemporaryCursorOverride
 * \since QGIS 3.8
 */
class GUI_EXPORT QgsTemporaryCursorRestoreOverride
{
  public:

    /**
     * Constructor for QgsTemporaryCursorRestoreOverride. Removes all application override
     * cursors.
     */
    QgsTemporaryCursorRestoreOverride();

    ~QgsTemporaryCursorRestoreOverride();

    /**
     * Restores the cursor override early (i.e. before this object is destroyed).
     */
    void restore();

  private:

    std::vector< QCursor > mCursors;

};

#endif // QGSGUIUTILS_H
