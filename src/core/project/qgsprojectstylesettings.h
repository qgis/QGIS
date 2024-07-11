/***************************************************************************
    qgsprojectstylesettings.h
    ---------------------------
    begin                : May 2022
    copyright            : (C) 2022 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTSTYLESETTINGS_H
#define QGSPROJECTSTYLESETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgstextformat.h"
#include "qgswkbtypes.h"

#include <memory.h>
#include <QAbstractListModel>
#include <QColorSpace>
#include <QSortFilterProxyModel>
#include <QPointer>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;
class QgsProject;
class QgsSymbol;
class QgsColorRamp;
class QgsStyle;
class QgsCombinedStyleModel;

/**
 * \brief Contains settings and properties relating to how a QgsProject should handle
 * styling.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProjectStyleSettings : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectStyleSettings for the specified \a project.
     *
     * Ownership is transferred to the \a project.
     */
    QgsProjectStyleSettings( QgsProject *project = nullptr );

    ~QgsProjectStyleSettings() override;

    /**
     * Returns the project default symbol for a given type.
     * \param symbolType the symbol type
     * \returns a symbol pointer or NULL if there is no default set
     * \note the symbol ownership is transferred to the caller
     */
    QgsSymbol *defaultSymbol( Qgis::SymbolType symbolType ) const SIP_FACTORY;

    /**
     * Sets the project default symbol for a given type.
     * \param symbolType the symbol type
     * \param symbol the symbol pointer, set to NULL to clear default
     * \note the symbol ownership is not transferred
     */
    void setDefaultSymbol( Qgis::SymbolType symbolType, QgsSymbol *symbol );

    /**
     * Returns the project default color ramp.
     * \returns a color ramp pointer or NULL if there is no default set
     * \note the color ramp ownership is transferred to the caller
     */
    QgsColorRamp *defaultColorRamp() const SIP_FACTORY;

    /**
     * Sets the project default color ramp.
     * \param colorRamp the color ramp, set to NULL to clear default
     * \note the color ramp ownership is not transferred
     */
    void setDefaultColorRamp( QgsColorRamp *colorRamp );

    /**
     * Returns the project default text format.
     * \note if no default is defined, the returned format will be invalid
     */
    QgsTextFormat defaultTextFormat() const;

    /**
     * Sets the project default text format.
     * \param textFormat the text format, an invalid format is interpreted as no default
     */
    void setDefaultTextFormat( const QgsTextFormat &textFormat );

    /**
     * Returns whether the default symbol fill color is randomized.
     */
    bool randomizeDefaultSymbolColor() const { return mRandomizeDefaultSymbolColor; }

    /**
     * Sets whether the default symbol fill color is randomized.
     */
    void setRandomizeDefaultSymbolColor( bool randomized ) { mRandomizeDefaultSymbolColor = randomized; }

    /**
     * Returns the default symbol opacity.
     */
    double defaultSymbolOpacity() const { return mDefaultSymbolOpacity; }

    /**
     * Sets the default symbol opacity.
     */
    void setDefaultSymbolOpacity( double opacity ) { mDefaultSymbolOpacity = opacity; }

    /**
     * Resets the settings to a default state.
     */
    void reset();

    /**
     * Removes and deletes the project style database.
     *
     * \since QGIS 3.32
     */
    void removeProjectStyle();

    /**
     * Sets the style database to use for the project style.
     *
     * \see projectStyle()
     */
    void setProjectStyle( QgsStyle *style SIP_TRANSFER );

    /**
     * Returns the style database to use for project specific styles.
     *
     * \see setProjectStyle()
     */
    QgsStyle *projectStyle();

    /**
     * Set the project's color model to \a colorModel
     *
     * This sets the default color model used when selecting a color in the whole application.
     * Any color defined in a different color model than the one specified here will be converted to
     * this color model when exporting a layout.
     *
     * If a color space has already been set and its color model differs from \a colorModel, the project
     * color space will be reset to invalid one (This is only true if QGIS is built against
     * Qt 6.8.0 or greater). \see setColorSpace() colorSpace()
     *
     * The color model defaults to Qgis::ColorModel::Rgb
     *
     * \see colorModel()
     * \since QGIS 3.40
     */
    void setColorModel( Qgis::ColorModel colorModel );

    /**
     * Returns the project's color model
     *
     * This model is used as the default color model when selecting a color in the whole application.
     * Any color defined in a different color model than the returned model will be converted to
     * this color model when exporting a layout.
     *
     * The color model defaults to Qgis::ColorModel::Rgb
     *
     * \see setColorModel()
     * \since QGIS 3.40
     */
    Qgis::ColorModel colorModel() const;

    /**
     * Set the project's current color space to \a colorSpace. \a colorSpace must be a valid RGB or CMYK color space.
     * The color space's ICC profile will be added as a project attached file.
     *
     * The project color's space will be added to PDF layout exports when it is defined (i.e. it is different from
     * the default invalid QColorSpace).
     *
     * If a color model has already been set and it differs from \a colorSpace's color model, the project's
     * color model is set to match the color space's color model (This is only true if QGIS is built against
     * Qt 6.8.0 or greater). \see setColorModel() colorModel()
     *
     * The color space defaults to an invalid color space.
     *
     * \see colorSpace()
     * \since QGIS 3.40
     */
    void setColorSpace( const QColorSpace &colorSpace );

    /**
     * Returns the project's color space.
     *
     * The project color's space will be added to PDF layout exports when it is defined (i.e. it is different from
     * the default invalid QColorSpace).
     *
     * The color space defaults to an invalid color space.
     *
     * \see setColorSpace()
     * \since QGIS 3.40
     */
    QColorSpace colorSpace() const;

    /**
     * Reads the settings's state from a DOM element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context, Qgis::ProjectReadFlags flags = Qgis::ProjectReadFlags() );

    /**
     * Returns a DOM element representing the settings.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

    /**
     * Returns a list of all style databases (file paths) associated with the project.
     *
     * The file paths returned by this method will always be absolute paths. Depending on the
     * project settings, they may be converted to relative paths when the project is saved.
     *
     * \see styles()
     * \see addStyleDatabasePath()
     * \see setStyleDatabasePaths()
     */
    QStringList styleDatabasePaths() const { return mStyleDatabases; }

    /**
     * Returns a list of all the styles associated with the project.
     *
     * \see styleDatabasePaths()
     */
    QList< QgsStyle * > styles() const;

    /**
     * Returns a reference to the style database associated with the project with matching file \a path.
     */
    QgsStyle *styleAtPath( const QString &path );

    /**
     * Adds a style database \a path to the project.
     *
     * Paths can be either style .db databases, or .xml exports of style databases.
     *
     * The file path added by this method must always be absolute paths. Depending on the
     * project settings, they may be converted to relative paths when the project is saved.
     *
     * \see styleDatabasePaths()
     * \see setStyleDatabasePaths()
     * \see styleDatabasesChanged()
     */
    void addStyleDatabasePath( const QString &path );

    /**
     * Sets the \a paths to all style databases associated with the project.
     *
     * Paths can be either style .db databases, or .xml exports of style databases.
     *
     * The file paths set by this method must always be absolute paths. Depending on the
     * project settings, they may be converted to relative paths when the project is saved.
     *
     * \see addStyleDatabasePath()
     * \see styleDatabasePaths()
     * \see styleDatabasesChanged()
     */
    void setStyleDatabasePaths( const QStringList &paths );

    /**
     * Returns the combined style model which includes all style databases
     * associated with the project.
     *
     * This model also includes QgsStyle::defaultStyle().
     *
     * \note This is only available on builds based on Qt 5.13 or later.
     *
     * \see styles()
     */
    QgsCombinedStyleModel *combinedStyleModel();

  signals:

    /**
     * Emitted whenever the set of style databases associated with the project is changed.
     */
    void styleDatabasesChanged();

#ifndef SIP_RUN

    /**
     * Emitted when a style database path is about to be added.
     *
     * \note Not available in Python bindings
     */
    void styleDatabaseAboutToBeAdded( const QString &path );

    /**
     * Emitted when a style database path is added.
     *
     * \note Not available in Python bindings
     */
    void styleDatabaseAdded( const QString &path );

    /**
     * Emitted when a style database path is about to be removed.
     *
     * \note Not available in Python bindings
     */
    void styleDatabaseAboutToBeRemoved( const QString &path );

    /**
     * Emitted when a style database path is removed.
     *
     * \note Not available in Python bindings
     */
    void styleDatabaseRemoved( const QString &path );

    /**
     * Emitted when the style returned by projectStyle() is changed.
     *
     * \note Not available in Python bindings
     */
    void projectStyleChanged();

#endif
  private:

    QgsProject *mProject = nullptr;

    std::unique_ptr< QgsSymbol > mDefaultMarkerSymbol;
    std::unique_ptr< QgsSymbol > mDefaultLineSymbol;
    std::unique_ptr< QgsSymbol > mDefaultFillSymbol;
    std::unique_ptr< QgsColorRamp > mDefaultColorRamp;
    QgsTextFormat mDefaultTextFormat;

    bool mRandomizeDefaultSymbolColor = true;
    double mDefaultSymbolOpacity = 1.0;

    QgsStyle *mProjectStyle = nullptr;
    QStringList mStyleDatabases;
    QList< QPointer< QgsStyle > > mStyles;

    QgsCombinedStyleModel *mCombinedStyleModel = nullptr;
    Qgis::ColorModel mColorModel = Qgis::ColorModel::Rgb;
    QColorSpace mColorSpace;
    QString mIccProfileFilePath;

    void loadStyleAtPath( const QString &path );
    void clearStyles();

    friend class TestQgsProjectProperties;
};

/**
 * \ingroup core
 * \class QgsProjectStyleDatabaseModel
 *
 * \brief List model representing the style databases associated with a QgsProject.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProjectStyleDatabaseModel : public QAbstractListModel
{
    Q_OBJECT

  public:

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsProjectStyleDatabaseModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsProjectStyleDatabaseModel, Role ) : int
    {
      Style SIP_MONKEYPATCH_COMPAT_NAME( StyleRole ) = Qt::UserRole + 1, //!< Style object
      Path SIP_MONKEYPATCH_COMPAT_NAME(PathRole) //!< Style path
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsProjectStyleDatabaseModel, showing the styles from the specified \a settings.
     */
    explicit QgsProjectStyleDatabaseModel( QgsProjectStyleSettings *settings, QObject *parent SIP_TRANSFERTHIS = nullptr );

    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

    /**
     * Returns the style at the corresponding \a index.
     * \see indexFromStyle()
     */
    QgsStyle *styleFromIndex( const QModelIndex &index ) const;

    /**
     * Returns the model index corresponding to a \a style.
     * \see styleFromIndex()
     */
    QModelIndex indexFromStyle( QgsStyle *style ) const;

    /**
     * Sets whether the default style should also be included in the model.
     *
     * \see showDefaultStyle()
     */
    void setShowDefaultStyle( bool show );

    /**
     * Returns TRUE if the model includes the default style.
     *
     * \see setShowDefaultStyle()
     */
    bool showDefaultStyle() const { return mShowDefault; }

  private slots:
    void styleDatabaseAboutToBeAdded( const QString &path );
    void styleDatabaseAboutToBeRemoved( const QString &path );
    void styleDatabaseAdded( const QString &path );
    void styleDatabaseRemoved( const QString &path );

    void setProjectStyle( QgsStyle *style );
    void projectStyleAboutToBeDestroyed();
    void projectStyleDestroyed();
    void projectStyleChanged();

  private:
    QgsProjectStyleSettings *mSettings = nullptr;
    bool mShowDefault = false;
    QPointer< QgsStyle > mProjectStyle;
};

/**
 * \ingroup core
 * \class QgsProjectStyleDatabaseProxyModel
 *
 * \brief A proxy model for filtering QgsProjectStyleDatabaseModel.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProjectStyleDatabaseProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

  public:

    //! Available filter flags for filtering the model
    enum class Filter : int SIP_ENUM_BASETYPE( IntFlag )
    {
      FilterHideReadOnly = 1 << 0, //!< Hide read-only style databases
    };
    Q_ENUM( Filter )
    //! Available filter flags for filtering the model
    Q_DECLARE_FLAGS( Filters, Filter )
    Q_FLAG( Filters )

    /**
     * Constructor for QgsProjectStyleDatabaseProxyModel, for the specified style database \a model.
     */
    QgsProjectStyleDatabaseProxyModel( QgsProjectStyleDatabaseModel *model, QObject *parent SIP_TRANSFERTHIS = nullptr );

    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    /**
     * Returns the current filters used for filtering available style.
     *
     * \see setFilters()
     */
    QgsProjectStyleDatabaseProxyModel::Filters filters() const;

    /**
     * Sets the current \a filters used for filtering available styles.
     *
     * \see filters()
     */
    void setFilters( QgsProjectStyleDatabaseProxyModel::Filters filters );

  private:

    QgsProjectStyleDatabaseProxyModel::Filters mFilters;

};
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProjectStyleDatabaseProxyModel::Filters )


#endif // QGSPROJECTSTYLESETTINGS_H
