/***************************************************************************
  qgsfilewidget.h

 ---------------------
 begin                : 17.12.2015
 copyright            : (C) 2015 by Denis Rouzaud
 email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILEWIDGET_H
#define QGSFILEWIDGET_H

class QLabel;
class QToolButton;
class QVariant;
class QHBoxLayout;
class QgsFileDropEdit;

#include <QWidget>
#include <QFileDialog>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgshighlightablelineedit.h"


/**
 * \ingroup gui
 * \brief The QgsFileWidget class creates a widget for selecting a file or a folder.
 */
class GUI_EXPORT QgsFileWidget : public QWidget
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsFileWidget *>( sipCpp ) )
      sipType = sipType_QgsFileWidget;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT
    Q_PROPERTY( bool fileWidgetButtonVisible READ fileWidgetButtonVisible WRITE setFileWidgetButtonVisible )
    Q_PROPERTY( bool useLink READ useLink WRITE setUseLink )
    Q_PROPERTY( bool fullUrl READ fullUrl WRITE setFullUrl )
    Q_PROPERTY( QString dialogTitle READ dialogTitle WRITE setDialogTitle )
    Q_PROPERTY( QString filter READ filter WRITE setFilter )
    Q_PROPERTY( QString defaultRoot READ defaultRoot WRITE setDefaultRoot )
    Q_PROPERTY( StorageMode storageMode READ storageMode WRITE setStorageMode )
    Q_PROPERTY( RelativeStorage relativeStorage READ relativeStorage WRITE setRelativeStorage )
    Q_PROPERTY( QFileDialog::Options options READ options WRITE setOptions )

  public:

    /**
     * \brief The StorageMode enum determines if the file picker should pick files or directories
     */
    enum StorageMode
    {
      GetFile, //!< Select a single file
      GetDirectory, //!< Select a directory
      GetMultipleFiles, //!< Select multiple files
      SaveFile, //!< Select a single new or pre-existing file
    };
    Q_ENUM( StorageMode )

    /**
     * \brief The RelativeStorage enum determines if path is absolute, relative to the current project path or relative to a defined default path.
     */
    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };
    Q_ENUM( RelativeStorage )

    /**
     * \brief QgsFileWidget creates a widget for selecting a file or a folder.
     */
    explicit QgsFileWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * \brief Returns the current file path(s).
     *
     * When multiple files are selected they are quoted and separated
     * by a single space (for example: '"/path/foo" "path/bar"')
     *
     * \see setFilePath()
     * \see splitFilePaths()
     */
    QString filePath();

    /**
     * \brief Split the the quoted and space separated \a path and returns a list of strings.
     *
     * \see filePath()
     */
    static QStringList splitFilePaths( const QString &path );

    /**
     * Sets the current file \a path.
     *
     * \see filePath()
     */
    void setFilePath( const QString &path );

    /**
     * Sets whether the widget should be read only.
     */
    virtual void setReadOnly( bool readOnly );

    /**
     * Returns the open file dialog title.
     *
     * \see setDialogTitle()
     */
    QString dialogTitle() const;

    /**
     * \brief Sets the \a title to use for the open file dialog.
     *
     * \note If not defined, the title is "Select a file" or "Select a directory" or "Select one or more files" depending on the configuration.
     *
     * \see dialogTitle()
     */
    void setDialogTitle( const QString &title );

    //! returns the filters used for QDialog::getOpenFileName
    QString filter() const;

    /**
     * \brief setFilter sets the filter used by the model to filters. The filter is used to specify the kind of files that should be shown.
     * \param filter Only files that match the given filter are shown, it may be an empty string. If you want multiple filters, separate them with ';;',
     */
    void setFilter( const QString &filter );

    /**
     * Returns the additional options used for QFileDialog.
     *
     * \see setOptions()
     *
     * \since QGIS 3.14
     */
    QFileDialog::Options options() const;

    /**
     * \brief Set additional options used for QFileDialog.
     *
     * These options affect the look and feel of the QFileDialog shown when a user is interactively browsing
     * for paths.
     *
     * \see options()
     *
     * \since QGIS 3.14
     */
    void setOptions( QFileDialog::Options options );

    /**
     * Sets the selected filter when the file dialog opens.
     *
     * \see selectedFilter()
     */
    void setSelectedFilter( const QString &selectedFilter ) { mSelectedFilter = selectedFilter; }

    /**
     * Returns the selected filter from the last opened file dialog.
     *
     * \see setSelectedFilter()
     */
    QString selectedFilter() const { return mSelectedFilter; }

    /**
     * Sets whether a confirmation to overwrite an existing file will appear.
     *
     * By default, a confirmation will appear.
     *
     * \param confirmOverwrite If set to TRUE, an overwrite confirmation will be shown
     *
     * \see confirmOverwrite()
     */
    void setConfirmOverwrite( bool confirmOverwrite ) { mConfirmOverwrite = confirmOverwrite; }

    /**
     * Returns whether a confirmation will be shown when overwriting an existing file.
     *
     * \see setConfirmOverwrite()
     */
    bool confirmOverwrite() const { return mConfirmOverwrite; }

    /**
     * Returns TRUE if the tool button is shown.
     *
     * \see setFileWidgetButtonVisible()
     */
    bool fileWidgetButtonVisible() const;

    /**
     * Sets whether the tool button is \a visible.
     *
     * \see fileWidgetButtonVisible()
     */
    void setFileWidgetButtonVisible( bool visible );

    /**
     * Returns TRUE if the file path will be shown as a link.
     *
     * \see setUseLink()
     */
    bool useLink() const;

    /**
     * Sets whether the file path will be shown as a link.
     *
     * \see useLink()
     */
    void setUseLink( bool useLink );

    /**
     * Returns TRUE if the links shown use the full path.
     *
     * \see setFullUrl()
     */
    bool fullUrl() const;

    /**
     * Sets whether links shown use the full path.
     *
     * \see fullUrl()
     */
    void setFullUrl( bool fullUrl );

    /**
     * Returns the default root path.
     *
     * \see setDefaultRoot()
     */
    QString defaultRoot() const;

    /**
     * Returns the default root path used as the first shown location when picking a file and used if the RelativeStorage is RelativeDefaultPath.
     *
     * \see defaultRoot()
     */
    void setDefaultRoot( const QString &defaultRoot );

    /**
     * Returns the widget's storage mode (i.e. file or directory).
     *
     * \see setStorageMode()
     */
    QgsFileWidget::StorageMode storageMode() const;

    /**
     * Sets the widget's storage mode (i.e. file or directory).
     *
     * \see storageMode()
     */
    void setStorageMode( QgsFileWidget::StorageMode storageMode );

    /**
     * Returns if the relative path is with respect to the project path or the default path.
     *
     * \see setRelativeStorage()
     */
    QgsFileWidget::RelativeStorage relativeStorage() const;

    /**
     * Sets whether the relative path is with respect to the project path or the default path.
     *
     * \see relativeStorage()
     */
    void setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage );

    /**
     * Returns a pointer to the widget's line edit, which can be used to customize
     * the appearance and behavior of the line edit portion of the widget.
     * \since QGIS 3.0
     */
    QgsFilterLineEdit *lineEdit();

  signals:

    /**
     * Emitted whenever the current file or directory \a path is changed.
     */
    void fileChanged( const QString &path );

  private slots:
    void openFileDialog();
    void textEdited( const QString &path );
    void editLink();
    void fileDropped( const QString &filePath );

  protected:

    /**
     * Update buttons visibility
     */
    virtual void updateLayout();

    /**
     * Called whenever user select \a fileNames from dialog
     */
    virtual void setSelectedFileNames( QStringList fileNames );

    /**
     * Returns true if \a path is a multifiles
     */
    static bool isMultiFiles( const QString &path );

    /**
     * Update filePath according to \a filePaths list
     */
    void setFilePaths( const QStringList &filePaths );

    QString mFilePath;
    bool mButtonVisible = true;
    bool mUseLink = false;
    bool mFullUrl = false;
    bool mReadOnly = false;
    bool mIsLinkEdited = false;
    QString mDialogTitle;
    QString mFilter;
    QString mSelectedFilter;
    QString mDefaultRoot;
    bool mConfirmOverwrite = true;
    StorageMode mStorageMode = GetFile;
    RelativeStorage mRelativeStorage = Absolute;
    QFileDialog::Options mOptions = QFileDialog::Options();

    QLabel *mLinkLabel = nullptr;
    QgsFileDropEdit *mLineEdit = nullptr;
    QToolButton *mLinkEditButton = nullptr;
    QToolButton *mFileWidgetButton = nullptr;
    QHBoxLayout *mLayout = nullptr;

    //! returns a HTML code with a link to the given file path
    QString toUrl( const QString &path ) const;

    //! Returns a filePath with relative path options applied (or not) !
    QString relativePath( const QString &filePath, bool removeRelative ) const;

    friend class TestQgsFileWidget;
    friend class TestQgsExternalStorageFileWidget;
    friend class TestQgsExternalResourceWidgetWrapper;
};

///@cond PRIVATE



#ifndef SIP_RUN

/**
 * \ingroup gui
 * \brief A line edit for capturing file names that can have files dropped onto
 * it via drag & drop.
 *
 * Dropping can be limited to files only, files with a specific extension
 * or directories only. By default, dropping is limited to files only.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsFileDropEdit: public QgsHighlightableLineEdit
{
    Q_OBJECT

  public:
    QgsFileDropEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setStorageMode( QgsFileWidget::StorageMode storageMode ) { mStorageMode = storageMode; }

    void setFilters( const QString &filters );

    //! Returns file names if object meets drop criteria.
    QStringList acceptableFilePaths( QDropEvent *event ) const;

  signals:

    /**
     * Emitted when the file \a filePath is droppen onto the line edit.
     */
    void fileDropped( const QString &filePath );

  protected:

    //! Returns file name if object meets drop criteria.
    QString acceptableFilePath( QDropEvent *event ) const;

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;

  private:


    QStringList mAcceptableExtensions;
    QgsFileWidget::StorageMode mStorageMode = QgsFileWidget::GetFile;
    friend class TestQgsFileWidget;
};

#endif
///@endcond

#endif // QGSFILEWIDGET_H
