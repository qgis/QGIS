/***************************************************************************
    qgsapplication.h - Accessors for application-wide data
     --------------------------------------
    Date                 : 02-Jan-2006
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
/* $Id$ */
#ifndef QGSAPPLICATION_H
#define QGSAPPLICATION_H

#include <QApplication>

/** \ingroup core
 * Extends QApplication to provide access to QGIS specific resources such
 * as theme paths, database paths etc.
 */
class CORE_EXPORT QgsApplication: public QApplication
{
    Q_OBJECT
  public:
    //! @note customConfigDir parameter added in v1.6
    QgsApplication( int & argc, char ** argv, bool GUIenabled, QString customConfigPath = QString() );
    virtual ~QgsApplication();

    //! Catch exceptions when sending event to receiver.
    virtual bool notify( QObject * receiver, QEvent * event );

    /** Set the active theme to the specified theme.
     * The theme name should be a single word e.g. 'default','classic'.
     * The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
     * but plugin writers etc can use themeName() as a basis for searching
     * for resources in their own datastores e.g. a Qt4 resource bundle.
     * @note A basic test will be carried out to ensure the theme search path
     * based on the supplied theme name exists. If it does not the theme name will
     * be reverted to 'default'.
     */
    static void setThemeName( const QString theThemeName );

    /** Set the active theme to the specified theme.
     * The theme name should be a single word e.g. 'default','classic'.
     * The theme search path usually will be pkgDataPath + "/themes/" + themName + "/"
     * but plugin writers etc can use this method as a basis for searching
     * for resources in their own datastores e.g. a Qt4 resource bundle.
     */
    static const QString themeName() ;

    //! Returns the path to the authors file.
    static const QString authorsFilePath();

    /** Returns the path to the contributors file.
     * Contributors are people who have submitted patches
     * but don't have svn write access.
     * @note this function was added in version 1.3 */
    static const QString contributorsFilePath();

    /**Returns the path to the sponsors file.
      @note this function was added in version 1.2*/
    static const QString sponsorsFilePath();

    /** Returns the path to the donors file.
      @note this function was added in version 1.2*/
    static const QString donorsFilePath();

    /**
     * Returns the path to the sponsors file.
     * @note This was added in QGIS 1.1
     */
    static const QString translatorsFilePath();

    //! Returns the path to the developer image directory.
    static const QString developerPath();

    //! Returns the path to the help application.
    static const QString helpAppPath();

    //! Returns the path to the mapserver export application.
    static const QString msexportAppPath();

    //! Returns the path to the translation directory.
    static const QString i18nPath();

    //! Returns the path to the master qgis.db file.
    static const QString qgisMasterDbFilePath();

    //! Returns the path to the spatialite template db file.
    //! @note added in 1.5
    static const QString qgisSpatialiteDbTemplatePath();

    //! Returns the path to the settings directory in user's home dir
    static const QString qgisSettingsDirPath();

    //! Returns the path to the user qgis.db file.
    static const QString qgisUserDbFilePath();

    //! Returns the path to the splash screen image directory.
    static const QString splashPath();

    //! Returns the path to the icons image directory.
    static const QString iconsPath();

    //! Returns the path to the srs.db file.
    static const QString srsDbFilePath();

    //! Returns the pathes to svg directories.
    //! @note added in 1.4
    static const QStringList svgPaths();

    //! Returns the paths to svg applications svg directory.
    //! @note deprecated since 1.4
    static const QString svgPath();

    //! Returns the path to the application prefix directory.
    static const QString prefixPath();

    //! Returns the path to the application plugin directory.
    static const QString pluginPath();

    //! Returns the common root path of all application data directories.
    static const QString pkgDataPath();

    //! Returns the path to the currently active theme directory.
    static const QString activeThemePath();

    //! Returns the path to the default theme directory.
    static const QString defaultThemePath();

    //! Returns path to the desired icon file.
    //! First it tries to use the active theme path, then default theme path
    //! @note Added in 1.5
    static QString iconPath( QString iconFile );

    //! Returns the path to user's style. Added in QGIS 1.4
    static const QString userStyleV2Path();

    //! Returns the path to default style (works as a starting point). Added in QGIS 1.4
    static const QString defaultStyleV2Path();

    //! Alters prefix path - used by 3rd party apps
    static void setPrefixPath( const QString thePrefixPath, bool useDefaultPaths = false );

    //! Alters plugin path - used by 3rd party apps
    static void setPluginPath( const QString thePluginPath );

    //! Alters pkg data path - used by 3rd party apps
    static void setPkgDataPath( const QString thePkgDataPath );

    //! Alters default svg paths - used by 3rd party apps. Added in QGIS 1.5
    static void setDefaultSvgPaths( const QStringList& pathList );

    //! loads providers
    static void initQgis();

    //! deletes provider registry and map layer registry
    static void exitQgis();

    /** constants for endian-ness */
    typedef enum ENDIAN
    {
      XDR = 0,  // network, or big-endian, byte order
      NDR = 1   // little-endian byte order
    }
    endian_t;

    //! Returns whether this machine uses big or little endian
    static endian_t endian();

    /** \brief get a standard css style sheet for reports.
     * Typically you will use this method by doing:
     * QString myStyle = QgsApplication::reportStyleSheet();
     * textBrowserReport->document()->setDefaultStyleSheet(myStyle);
     * @return QString containing the CSS 2.1 compliant stylesheet.
     * @note you can use the special Qt extensions too, for example
     * the gradient fills for backgrounds.
     */
    static QString reportStyleSheet();
    /** Convenience function to get a summary of the paths used in this
     * application instance useful for debugging mainly.*/
    static QString showSettings();
    /** Register OGR drivers ensuring this only happens once.
     * This is a workaround for an issue with older gdal versions that
     * caused duplicate driver name entries to appear in the list
     * of registered drivers when QgsApplication::registerOgrDrivers was called multiple
     * times.
     */
    static void registerOgrDrivers();

  private:
    static QString mPrefixPath;
    static QString mPluginPath;
    static QString mPkgDataPath;
    static QString mThemeName;
    static QStringList mDefaultSvgPaths;

    static QString mConfigPath;
};

#endif
