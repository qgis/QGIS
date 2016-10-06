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

class QgsFilterLineEdit;

#include <QWidget>

/** \ingroup gui
 * @brief The QgsFileWidget class creates a widget for selecting a file or a folder.
 */
class GUI_EXPORT QgsFileWidget : public QWidget
{
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
     * @brief The StorageMode enum determines if the file picker should pick files or directories
     */
    enum StorageMode
    {
      GetFile,
      GetDirectory
    };

    /**
     * @brief The RelativeStorage enum determines if path is absolute, relative to the current project path or relative to a defined default path.
     */
    enum RelativeStorage
    {
      Absolute,
      RelativeProject,
      RelativeDefaultPath
    };

    /**
     * @brief QgsFileWidget creates a widget for selecting a file or a folder.
     */
    explicit QgsFileWidget( QWidget *parent = 0 );

    ~QgsFileWidget();

    //! Returns the current file path
    QString filePath();

    //! Sets the file path
    void setFilePath( QString path );

    //! defines if the widget is readonly
    void setReadOnly( bool readOnly );

    //! returns the open file dialog title
    QString dialogTitle() const;
    /**
     * @brief setDialogTitle defines the open file dialog title
     * @note if not defined, the title is "Select a file" or "Select a directory" depending on the configuration.
     */
    void setDialogTitle( const QString& title );

    //! returns the filters used for QDialog::getOpenFileName
    QString filter() const;
    /**
     * @brief setFilter sets the filter used by the model to filters. The filter is used to specify the kind of files that should be shown.
     * @param filter Only files that match the given filter are shown, it may be an empty string. If you want multiple filters, separate them with ';;',
     */
    void setFilter( const QString &filter );

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
    void setDefaultRoot( const QString& defaultRoot );

    //! returns the storage mode (i.e. file or directory)
    QgsFileWidget::StorageMode storageMode() const;
    //! determines the storage mode (i.e. file or directory)
    void setStorageMode( QgsFileWidget::StorageMode storageMode );

    //! returns if the relative path is with respect to the project path or the default path
    QgsFileWidget::RelativeStorage relativeStorage() const;
    //! determines if the relative path is with respect to the project path or the default path
    void setRelativeStorage( QgsFileWidget::RelativeStorage relativeStorage );

  signals:
    //! emitted as soon as the current file or directory is changed
    void fileChanged( const QString& );

  private slots:
    void openFileDialog();
    void textEdited( const QString& path );

  private:
    QString mFilePath;
    bool mButtonVisible;
    bool mUseLink;
    bool mFullUrl;
    QString mDialogTitle;
    QString mFilter;
    QString mDefaultRoot;
    StorageMode mStorageMode;
    RelativeStorage mRelativeStorage;

    QLabel* mLinkLabel;
    QgsFilterLineEdit* mLineEdit;
    QToolButton* mFileWidgetButton;

    //! returns a HTML code with a link to the given file path
    QString toUrl( const QString& path ) const;

    //! Returns a filePath with relative path options applied (or not) !
    QString relativePath( const QString& filePath, bool removeRelative ) const;

    friend class TestQgsFileWidget;
};

#endif // QGSFILEWIDGET_H
