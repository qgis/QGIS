/***************************************************************************
 qgsfilecontentsourcelineedit.h
 ---------------------
 begin                : July 2018
 copyright            : (C) 2018 by Nyall Dawson
 email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILECONTENTSOURCELINEEDIT_H
#define QGSFILECONTENTSOURCELINEEDIT_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QWidget>
#include <QString>

class QgsFilterLineEdit;
class QToolButton;
class QgsMessageBar;
class QgsPropertyOverrideButton;

/**
 * \ingroup gui
 * \class QgsAbstractFileContentSourceLineEdit
 * \brief Abstract base class for a widgets which allows users to select content from a file, embedding a file, etc.
 *
 * This class is designed to be used by content which is managed by a QgsAbstractContentCache,
 * i.e. it can handle either direct file paths, base64 encoded contents, or remote HTTP
 * urls.
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsAbstractFileContentSourceLineEdit : public QWidget SIP_ABSTRACT
{
    Q_OBJECT
    Q_PROPERTY( QString source READ source WRITE setSource NOTIFY sourceChanged )

  public:

    /**
     * Constructor for QgsAbstractFileContentSourceLineEdit, with the specified \a parent widget.
     */
    QgsAbstractFileContentSourceLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget.
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar() const;

    /**
     * Returns the current file source.
     * \see setSource()
     * \see sourceChanged()
     */
    QString source() const;

    /**
     * Sets a specific settings \a key to use when storing the last
     * used path for the file source.
     */
    void setLastPathSettingsKey( const QString &key );

    /**
     * Returns the property override tool button
     * \since QGIS 3.16
     */
    QgsPropertyOverrideButton *propertyOverrideToolButton() const {return mPropertyOverrideButton;}

    /**
     * Sets the visibility of the property override tool button
     * \since QGIS 3.16
     */
    void setPropertyOverrideToolButtonVisible( bool visible );

  public slots:

    /**
     * Sets a new \a source to show in the widget.
     * \see source()
     * \see sourceChanged()
     */
    void setSource( const QString &source );

  signals:

    /**
     * Emitted whenever the file source is changed in the widget.
     */
    void sourceChanged( const QString &source );

  private:

#ifndef SIP_RUN
///@cond PRIVATE

    /**
     * Returns the widget's file filter string.
     */
    virtual QString fileFilter() const = 0;

    /**
     * Returns the translated title to use for the select file dialog.
     */
    virtual QString selectFileTitle() const = 0;

    /**
     * Returns the translated title to use for the file from URL dialog.
     */
    virtual QString fileFromUrlTitle() const = 0;

    /**
     * Returns the translated descriptive text to use for the file from URL dialog.
     */
    virtual QString fileFromUrlText() const = 0;

    /**
     * Returns the translated title to use for the embed file dialog.
     */
    virtual QString embedFileTitle() const = 0;

    /**
     * Returns the translated title to use for the extract file dialog.
     */
    virtual QString extractFileTitle() const = 0;

    /**
     * Returns the default settings key to use for the widget's settings.
     */
    virtual QString defaultSettingsKey() const = 0;

///@endcond
#endif

  private slots:
    void selectFile();
    void selectUrl();
    void embedFile();
    void extractFile();
    void mFileLineEdit_textEdited( const QString &text );

  private:

    enum Mode
    {
      ModeFile,
      ModeBase64,
    };

    Mode mMode = ModeFile;
    bool mPropertyOverrideButtonVisible = false;

    QgsFilterLineEdit *mFileLineEdit = nullptr;
    QToolButton *mFileToolButton = nullptr;
    QgsPropertyOverrideButton *mPropertyOverrideButton = nullptr;
    QString mLastPathKey;
    QString mBase64;
    QgsMessageBar *mMessageBar = nullptr;

    QString defaultPath() const;
    QString settingsKey() const;

};



/**
 * \ingroup gui
 * \class QgsPictureSourceLineEditBase
 * \brief A line edit widget with toolbutton for setting a raster image path.
 *
 * \see QgsSvgSourceLineEdit
 *
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsPictureSourceLineEditBase : public QgsAbstractFileContentSourceLineEdit
{
    Q_OBJECT
  public:

    /**
     * Format of source image
     */
    enum Format
    {
      Svg, //!< SVG image
      Image, //!< Raster image
      AnimatedImage, //!< Animated image (since QGIS 3.26)
    };

    /**
     * Constructor for QgsImageSourceLineEdit, with the specified \a parent widget.
     * The default format is SVG.
     */
    QgsPictureSourceLineEditBase( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsAbstractFileContentSourceLineEdit( parent )
    {}

    //! Defines the mode of the source line edit
    void setMode( Format format ) {mFormat = format;}

  protected:

    /**
     * Constructor for QgsImageSourceLineEdit, with the specified \a parent widget.
     */
    QgsPictureSourceLineEditBase( Format format, QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsAbstractFileContentSourceLineEdit( parent )
      , mFormat( format )
    {}

  private:
    Format mFormat = Svg;

#ifndef SIP_RUN
///@cond PRIVATE
    QString fileFilter() const override;
    QString selectFileTitle() const override;
    QString fileFromUrlTitle() const override;
    QString fileFromUrlText() const override;
    QString embedFileTitle() const override;
    QString extractFileTitle() const override;
    QString defaultSettingsKey() const override;
    ///@endcond
#endif
};


/**
 * \ingroup gui
 * \class QgsSvgSourceLineEdit
 * \brief A line edit widget with toolbutton for setting an SVG image path.
 *
 * Designed for use with QgsSvgCache.
 *
 * \see QgsImageSourceLineEdit
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsSvgSourceLineEdit : public QgsPictureSourceLineEditBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsSvgSourceLineEdit, with the specified \a parent widget.
     */
    QgsSvgSourceLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsPictureSourceLineEditBase( Svg, parent )
    {}
};

/**
 * \ingroup gui
 * \class QgsImageSourceLineEdit
 * \brief A line edit widget with toolbutton for setting a raster image path.
 *
 * Designed for use with QgsImageCache.
 *
 * \see QgsSvgSourceLineEdit
 *
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsImageSourceLineEdit : public QgsPictureSourceLineEditBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsImageSourceLineEdit, with the specified \a parent widget.
     */
    QgsImageSourceLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsPictureSourceLineEditBase( Image, parent )
    {}
};


/**
 * \ingroup gui
 * \class QgsAnimatedImageSourceLineEdit
 * \brief A line edit widget with toolbutton for setting an animated raster image path.
 *
 * Designed for use with QgsImageCache.
 *
 * \see QgsImageSourceLineEdit
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsAnimatedImageSourceLineEdit : public QgsPictureSourceLineEditBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsAnimatedImageSourceLineEdit, with the specified \a parent widget.
     */
    QgsAnimatedImageSourceLineEdit( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QgsPictureSourceLineEditBase( AnimatedImage, parent )
    {}
};

#endif // QGSFILECONTENTSOURCELINEEDIT_H
