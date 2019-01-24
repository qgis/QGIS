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
class QgsFileDropEdit;
class QHBoxLayout;
#include <QWidget>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsfilterlineedit.h"

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

  public:

    /**
     * \brief The StorageMode enum determines if the file picker should pick files or directories
     */
    enum StorageMode
    {
      GetFile, //! Select a single file
      GetDirectory, //! Select a directory
      GetMultipleFiles, //! Select multiple files
      SaveFile, //! Select a single new or pre-existing file
    };

    /**
     * \brief The RelativeStorage enum determines if path is absolute, relative to the current project path or relative to a defined default path.
     */
    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };

    /**
     * \brief QgsFileWidget creates a widget for selecting a file or a folder.
     */
    explicit QgsFileWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * \brief Returns the current file path(s)
     * when multiple files are selected, they are quoted and separated
     * by a single space (for example: '"/path/foo" "path/bar"')
     * \see splitFilePaths()
     */
    QString filePath();

    /**
     * \brief Split the the quoted and space separated \a path and returns a QString list
     * \see filePath
     */
    static QStringList splitFilePaths( const QString &path );

    //! Sets the file path
    void setFilePath( QString path );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

    //! returns the open file dialog title
    QString dialogTitle() const;

    /**
     * \brief setDialogTitle defines the open file dialog title
     * \note if not defined, the title is "Select a file" or "Select a directory" or "Select one or more files" depending on the configuration.
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
     * Sets the selected filter when the file dialog opens.
     */
    void setSelectedFilter( const QString &selectedFilter ) { mSelectedFilter = selectedFilter; }

    /**
     * Returns the selected filter from the last opened file dialog.
     */
    QString selectedFilter() const { return mSelectedFilter; }

    /**
     * Sets whether a confirmation to overwrite an existing file will appear.
     * By default, a confirmation will appear.
     * \param confirmOverwrite If set to true, an overwrite confirmation will be shown
     */
    void setConfirmOverwrite( bool confirmOverwrite ) { mConfirmOverwrite = confirmOverwrite; }

    /**
     * Returns whether a confirmation will be shown when overwriting an existing file
     */
    bool confirmOverwrite() const { return mConfirmOverwrite; }

    //! determines if the tool button is shown
    bool fileWidgetButtonVisible() const;
    //! determines if the tool button is shown
    void setFileWidgetButtonVisible( bool visible );

    //! determines if the file path will be shown as a link
    bool useLink() const;
    //! determines if the file path will be shown as a link
    void setUseLink( bool useLink );

    //! returns if the links shows the full path or not
    bool fullUrl() const;
    //! determines if the links shows the full path or not
    void setFullUrl( bool fullUrl );

    //! returns the default root path
    QString defaultRoot() const;
    //! determines the default root path used as the first shown location when picking a file and used if the RelativeStorage is RelativeDefaultPath
    void setDefaultRoot( const QString &defaultRoot );

    //! returns the storage mode (i.e. file or directory)
    QgsFileWidget::StorageMode storageMode() const;
    //! determines the storage mode (i.e. file or directory)
    void setStorageMode( QgsFileWidget::StorageMode storageMode );

    //! returns if the relative path is with respect to the project path or the default path
    QgsFileWidget::RelativeStorage relativeStorage() const;
    //! determines if the relative path is with respect to the project path or the default path
    void setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage );

    /**
     * Returns a pointer to the widget's line edit, which can be used to customize
     * the appearance and behavior of the line edit portion of the widget.
     * \since QGIS 3.0
     */
    QgsFilterLineEdit *lineEdit();

  signals:
    //! emitted as soon as the current file or directory is changed
    void fileChanged( const QString & );

  private slots:
    void openFileDialog();
    void textEdited( const QString &path );

  private:
    QString mFilePath;
    bool mButtonVisible = true;
    bool mUseLink = false;
    bool mFullUrl = false;
    QString mDialogTitle;
    QString mFilter;
    QString mSelectedFilter;
    QString mDefaultRoot;
    bool mConfirmOverwrite = true;
    StorageMode mStorageMode = GetFile;
    RelativeStorage mRelativeStorage = Absolute;

    QLabel *mLinkLabel = nullptr;
    QgsFileDropEdit *mLineEdit = nullptr;
    QToolButton *mFileWidgetButton = nullptr;
    QHBoxLayout *mLayout = nullptr;

    //! returns a HTML code with a link to the given file path
    QString toUrl( const QString &path ) const;

    //! Returns a filePath with relative path options applied (or not) !
    QString relativePath( const QString &filePath, bool removeRelative ) const;

    friend class TestQgsFileWidget;
};



///@cond PRIVATE

#ifndef SIP_RUN

/**
 * \ingroup gui
 * A line edit for capturing file names that can have files dropped onto
 * it via drag & drop.
 *
 * Dropping can be limited to files only, files with a specific extension
 * or directories only. By default, dropping is limited to files only.
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsFileDropEdit: public QgsFilterLineEdit
{
    Q_OBJECT

  public:
    QgsFileDropEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void setStorageMode( QgsFileWidget::StorageMode storageMode ) { mStorageMode = storageMode; }

    void setFilters( const QString &filters );

  protected:

    void dragEnterEvent( QDragEnterEvent *event ) override;
    void dragLeaveEvent( QDragLeaveEvent *event ) override;
    void dropEvent( QDropEvent *event ) override;
    void paintEvent( QPaintEvent *e ) override;

  private:

    //! Returns file name if object meets drop criteria.
    QString acceptableFilePath( QDropEvent *event ) const;

    QStringList mAcceptableExtensions;
    QgsFileWidget::StorageMode mStorageMode = QgsFileWidget::GetFile;
    bool mDragActive;
    friend class TestQgsFileWidget;
};

#endif
///@endcond

#endif // QGSFILEWIDGET_H
