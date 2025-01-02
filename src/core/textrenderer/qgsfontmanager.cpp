/***************************************************************************
    qgsfontmanager.cpp
    ------------------
    Date                 : June 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsfontmanager.h"
#include "moc_qgsfontmanager.cpp"
#include "qgsreadwritelocker.h"
#include "qgsapplication.h"
#include "qgsziputils.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsblockingnetworkrequest.h"
#include "qgssetrequestinitiator_p.h"
#include "qgsfileutils.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QFontDatabase>
#include <QTemporaryFile>
#include <QTemporaryDir>

const QgsSettingsEntryStringList *QgsFontManager::settingsFontFamilyReplacements = new QgsSettingsEntryStringList( QStringLiteral( "fontFamilyReplacements" ), QgsSettingsTree::sTreeFonts, QStringList(), QStringLiteral( "Automatic font family replacements" ) );

const QgsSettingsEntryBool *QgsFontManager::settingsDownloadMissingFonts = new QgsSettingsEntryBool( QStringLiteral( "downloadMissingFonts" ), QgsSettingsTree::sTreeFonts, true, QStringLiteral( "Automatically download missing fonts whenever possible" ) );

//
// QgsFontDownloadDetails
//

QgsFontDownloadDetails::QgsFontDownloadDetails() = default;

QgsFontDownloadDetails::QgsFontDownloadDetails( const QString &family, const QStringList &fontUrls, const QString &licenseUrl )
  : mFamily( family )
  , mStandardizedFamily( standardizeFamily( family ) )
  , mFontUrls( fontUrls )
  , mLicenseUrl( licenseUrl )
{
}

QString QgsFontDownloadDetails::standardizeFamily( const QString &family )
{
  const thread_local QRegularExpression charsToRemove( QStringLiteral( "[^a-z]" ) );
  const thread_local QRegularExpression styleNames( QStringLiteral( "(?:normal|regular|light|bold|black|demi|italic|oblique|medium|thin)" ) );

  QString processed = family.toLower();
  processed.replace( styleNames, QString() );
  return processed.replace( charsToRemove, QString() );
}

//
// QgsFontManager
//

QgsFontManager::QgsFontManager( QObject *parent )
  : QObject( parent )
{
  const QStringList replacements = settingsFontFamilyReplacements->value();
  for ( const QString &replacement : replacements )
  {
    const thread_local QRegularExpression rxReplacement( QStringLiteral( "(.*?):(.*)" ) );
    const QRegularExpressionMatch match = rxReplacement.match( replacement );
    if ( match.hasMatch() )
    {
      mFamilyReplacements.insert( match.captured( 1 ), match.captured( 2 ) );
      mLowerCaseFamilyReplacements.insert( match.captured( 1 ).toLower(), match.captured( 2 ) );
    }
  }
}

QMap<QString, QString> QgsFontManager::fontFamilyReplacements() const
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  return mFamilyReplacements;
}

void QgsFontManager::addFontFamilyReplacement( const QString &original, const QString &replacement )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  if ( !replacement.isEmpty() )
  {
    mFamilyReplacements.insert( original, replacement );
    mLowerCaseFamilyReplacements.insert( original.toLower(), replacement );
  }
  else
  {
    mFamilyReplacements.remove( original );
    mLowerCaseFamilyReplacements.remove( original.toLower() );
  }
  storeFamilyReplacements();
}

void QgsFontManager::setFontFamilyReplacements( const QMap<QString, QString> &replacements )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  mFamilyReplacements = replacements;
  mLowerCaseFamilyReplacements.clear();
  for ( auto it = mFamilyReplacements.constBegin(); it != mFamilyReplacements.constEnd(); ++it )
    mLowerCaseFamilyReplacements.insert( it.key().toLower(), it.value() );

  storeFamilyReplacements();
}

QString QgsFontManager::processFontFamilyName( const QString &name ) const
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  auto it = mLowerCaseFamilyReplacements.constFind( name.toLower() );
  if ( it != mLowerCaseFamilyReplacements.constEnd() )
    return it.value();
  else
    return name;
}

void QgsFontManager::storeFamilyReplacements()
{
  QStringList replacements;
  for ( auto it = mFamilyReplacements.constBegin(); it != mFamilyReplacements.constEnd(); ++it )
    replacements << QStringLiteral( "%1:%2" ).arg( it.key(), it.value() );
  settingsFontFamilyReplacements->setValue( replacements );
}

void QgsFontManager::installUserFonts()
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  const QString userProfileFontsDir = QgsApplication::qgisSettingsDirPath() + "fonts";
  QStringList fontDirs { userProfileFontsDir };

  fontDirs.append( mUserFontDirectories );

  for ( const QString &dir : std::as_const( fontDirs ) )
  {
    if ( !QFile::exists( dir ) && !QDir().mkpath( dir ) )
    {
      QgsDebugError( QStringLiteral( "Cannot create local fonts dir: %1" ).arg( dir ) );
      return;
    }

    installFontsFromDirectory( dir );
  }
}

void QgsFontManager::installFontsFromDirectory( const QString &dir )
{
  const QFileInfoList fileInfoList = QDir( dir ).entryInfoList( QStringList( QStringLiteral( "*" ) ), QDir::Files );
  QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
  for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
  {
    const int id = QFontDatabase::addApplicationFont( infoIt->filePath() );
    if ( id == -1 )
    {
      QgsDebugError( QStringLiteral( "The user font %1 could not be installed" ).arg( infoIt->filePath() ) );
      mUserFontToFamilyMap.remove( infoIt->filePath() );
      mUserFontToIdMap.remove( infoIt->filePath() );
    }
    else
    {
      mUserFontToFamilyMap.insert( infoIt->filePath(), QFontDatabase::applicationFontFamilies( id ) );
      mUserFontToIdMap.insert( infoIt->filePath(), id );
    }
  }
}

bool QgsFontManager::tryToDownloadFontFamily( const QString &family, QString &matchedFamily )
{
  matchedFamily.clear();
  if ( !settingsDownloadMissingFonts->value() )
    return false;

  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  auto it = mPendingFontDownloads.constFind( family );
  if ( it != mPendingFontDownloads.constEnd() )
  {
    matchedFamily = it.value();
    return true;
  }
  locker.unlock();

  const QgsFontDownloadDetails details = detailsForFontDownload( family, matchedFamily );
  if ( !details.isValid() )
    return false;

  // It's possible that the font family laundering applied in urlForFontDownload has cleaned up the font
  // family to a valid font which already exists on the system. In this case we shouldn't try to download
  // the font again.
  const QFont testFont( matchedFamily );
  if ( testFont.exactMatch() )
    return true;

  locker.changeMode( QgsReadWriteLocker::Write );
  mPendingFontDownloads.insert( family, matchedFamily );
  if ( !mEnableFontDownloads )
  {
    mDeferredFontDownloads.insert( matchedFamily, details );
  }
  else
  {
    locker.unlock();
    downloadAndInstallFont( details, family );
  }
  return true;
}

void QgsFontManager::enableFontDownloadsForSession()
{
  if ( mEnableFontDownloads )
    return;

  mEnableFontDownloads = true;
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  if ( !mDeferredFontDownloads.isEmpty() )
  {
    locker.changeMode( QgsReadWriteLocker::Write );
    for ( auto it = mDeferredFontDownloads.constBegin(); it != mDeferredFontDownloads.constEnd(); ++it )
    {
      downloadAndInstallFont( it.value(), it.key() );
    }
    mDeferredFontDownloads.clear();
  }
}

QgsFontDownloadDetails GoogleFontDetails( const QString &family, const QStringList &downloadPaths, const QString &licensePath = QString() )
{
  QStringList fontUrls;
  fontUrls.reserve( downloadPaths.size( ) );
  for ( const QString &path : downloadPaths )
  {
    fontUrls.append( QStringLiteral( "https://github.com/google/fonts/raw/main/%1" ).arg( path ) );
  }
  return QgsFontDownloadDetails(
           family,
           fontUrls,
           !licensePath.isEmpty() ? QStringLiteral( "https://github.com/google/fonts/raw/main/%1" ).arg( licensePath ) : QString()
         );
}

QgsFontDownloadDetails QgsFontManager::detailsForFontDownload( const QString &family, QString &matchedFamily ) const
{
  // this list is built using scripts/process_google_fonts.py
  const thread_local std::vector< QgsFontDownloadDetails > sGoogleFonts
  {
    GoogleFontDetails( QStringLiteral( "ABeeZee" ), { QStringLiteral( "ofl/abeezee/ABeeZee-Regular.ttf" ), QStringLiteral( "ofl/abeezee/ABeeZee-Italic.ttf" ) }, QStringLiteral( "ofl/abeezee/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "ADLaM Display" ), { QStringLiteral( "ofl/adlamdisplay/ADLaMDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/adlamdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Abel" ), { QStringLiteral( "ofl/abel/Abel-Regular.ttf" ) }, QStringLiteral( "ofl/abel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Abhaya Libre" ), { QStringLiteral( "ofl/abhayalibre/AbhayaLibre-Regular.ttf" ), QStringLiteral( "ofl/abhayalibre/AbhayaLibre-Medium.ttf" ), QStringLiteral( "ofl/abhayalibre/AbhayaLibre-SemiBold.ttf" ), QStringLiteral( "ofl/abhayalibre/AbhayaLibre-Bold.ttf" ), QStringLiteral( "ofl/abhayalibre/AbhayaLibre-ExtraBold.ttf" ) }, QStringLiteral( "ofl/abhayalibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aboreto" ), { QStringLiteral( "ofl/aboreto/Aboreto-Regular.ttf" ) }, QStringLiteral( "ofl/aboreto/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Abril Fatface" ), { QStringLiteral( "ofl/abrilfatface/AbrilFatface-Regular.ttf" ) }, QStringLiteral( "ofl/abrilfatface/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Abyssinica SIL" ), { QStringLiteral( "ofl/abyssinicasil/AbyssinicaSIL-Regular.ttf" ) }, QStringLiteral( "ofl/abyssinicasil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aclonica" ), { QStringLiteral( "apache/aclonica/Aclonica-Regular.ttf" ) }, QStringLiteral( "apache/aclonica/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Acme" ), { QStringLiteral( "ofl/acme/Acme-Regular.ttf" ) }, QStringLiteral( "ofl/acme/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Actor" ), { QStringLiteral( "ofl/actor/Actor-Regular.ttf" ) }, QStringLiteral( "ofl/actor/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Adamina" ), { QStringLiteral( "ofl/adamina/Adamina-Regular.ttf" ) }, QStringLiteral( "ofl/adamina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Advent Pro" ), { QStringLiteral( "ofl/adventpro/AdventPro%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/adventpro/AdventPro-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/adventpro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Agdasima" ), { QStringLiteral( "ofl/agdasima/Agdasima-Regular.ttf" ), QStringLiteral( "ofl/agdasima/Agdasima-Bold.ttf" ) }, QStringLiteral( "ofl/agdasima/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aguafina Script" ), { QStringLiteral( "ofl/aguafinascript/AguafinaScript-Regular.ttf" ) }, QStringLiteral( "ofl/aguafinascript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Akatab" ), { QStringLiteral( "ofl/akatab/Akatab-Regular.ttf" ), QStringLiteral( "ofl/akatab/Akatab-Medium.ttf" ), QStringLiteral( "ofl/akatab/Akatab-SemiBold.ttf" ), QStringLiteral( "ofl/akatab/Akatab-Bold.ttf" ), QStringLiteral( "ofl/akatab/Akatab-ExtraBold.ttf" ), QStringLiteral( "ofl/akatab/Akatab-Black.ttf" ) }, QStringLiteral( "ofl/akatab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Akaya Kanadaka" ), { QStringLiteral( "ofl/akayakanadaka/AkayaKanadaka-Regular.ttf" ) }, QStringLiteral( "ofl/akayakanadaka/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Akaya Telivigala" ), { QStringLiteral( "ofl/akayatelivigala/AkayaTelivigala-Regular.ttf" ) }, QStringLiteral( "ofl/akayatelivigala/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Akronim" ), { QStringLiteral( "ofl/akronim/Akronim-Regular.ttf" ) }, QStringLiteral( "ofl/akronim/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Akshar" ), { QStringLiteral( "ofl/akshar/Akshar%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/akshar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aladin" ), { QStringLiteral( "ofl/aladin/Aladin-Regular.ttf" ) }, QStringLiteral( "ofl/aladin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alata" ), { QStringLiteral( "ofl/alata/Alata-Regular.ttf" ) }, QStringLiteral( "ofl/alata/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alatsi" ), { QStringLiteral( "ofl/alatsi/Alatsi-Regular.ttf" ) }, QStringLiteral( "ofl/alatsi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Albert Sans" ), { QStringLiteral( "ofl/albertsans/AlbertSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/albertsans/AlbertSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/albertsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aldrich" ), { QStringLiteral( "ofl/aldrich/Aldrich-Regular.ttf" ) }, QStringLiteral( "ofl/aldrich/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alef" ), { QStringLiteral( "ofl/alef/Alef-Regular.ttf" ), QStringLiteral( "ofl/alef/Alef-Bold.ttf" ) }, QStringLiteral( "ofl/alef/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alegreya" ), { QStringLiteral( "ofl/alegreya/Alegreya%5Bwght%5D.ttf" ), QStringLiteral( "ofl/alegreya/Alegreya-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/alegreya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alegreya SC" ), { QStringLiteral( "ofl/alegreyasc/AlegreyaSC-Regular.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-Italic.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-Medium.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-MediumItalic.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-Bold.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-BoldItalic.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-ExtraBold.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-Black.ttf" ), QStringLiteral( "ofl/alegreyasc/AlegreyaSC-BlackItalic.ttf" ) }, QStringLiteral( "ofl/alegreyasc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alegreya Sans" ), { QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Thin.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-ThinItalic.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Light.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-LightItalic.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Regular.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Italic.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Medium.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-MediumItalic.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Bold.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-BoldItalic.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-ExtraBold.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-Black.ttf" ), QStringLiteral( "ofl/alegreyasans/AlegreyaSans-BlackItalic.ttf" ) }, QStringLiteral( "ofl/alegreyasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alegreya Sans SC" ), { QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Thin.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-ThinItalic.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Light.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-LightItalic.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Regular.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Italic.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Medium.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-MediumItalic.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Bold.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-BoldItalic.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-ExtraBold.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-Black.ttf" ), QStringLiteral( "ofl/alegreyasanssc/AlegreyaSansSC-BlackItalic.ttf" ) }, QStringLiteral( "ofl/alegreyasanssc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aleo" ), { QStringLiteral( "ofl/aleo/Aleo%5Bwght%5D.ttf" ), QStringLiteral( "ofl/aleo/Aleo-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/aleo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alex Brush" ), { QStringLiteral( "ofl/alexbrush/AlexBrush-Regular.ttf" ) }, QStringLiteral( "ofl/alexbrush/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alexandria" ), { QStringLiteral( "ofl/alexandria/Alexandria%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/alexandria/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alfa Slab One" ), { QStringLiteral( "ofl/alfaslabone/AlfaSlabOne-Regular.ttf" ) }, QStringLiteral( "ofl/alfaslabone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alice" ), { QStringLiteral( "ofl/alice/Alice-Regular.ttf" ) }, QStringLiteral( "ofl/alice/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alike" ), { QStringLiteral( "ofl/alike/Alike-Regular.ttf" ) }, QStringLiteral( "ofl/alike/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alike Angular" ), { QStringLiteral( "ofl/alikeangular/AlikeAngular-Regular.ttf" ) }, QStringLiteral( "ofl/alikeangular/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alkalami" ), { QStringLiteral( "ofl/alkalami/Alkalami-Regular.ttf" ) }, QStringLiteral( "ofl/alkalami/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alkatra" ), { QStringLiteral( "ofl/alkatra/Alkatra%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/alkatra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Allan" ), { QStringLiteral( "ofl/allan/Allan-Regular.ttf" ), QStringLiteral( "ofl/allan/Allan-Bold.ttf" ) }, QStringLiteral( "ofl/allan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Allerta" ), { QStringLiteral( "ofl/allerta/Allerta-Regular.ttf" ) }, QStringLiteral( "ofl/allerta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Allerta Stencil" ), { QStringLiteral( "ofl/allertastencil/AllertaStencil-Regular.ttf" ) }, QStringLiteral( "ofl/allertastencil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Allison" ), { QStringLiteral( "ofl/allison/Allison-Regular.ttf" ) }, QStringLiteral( "ofl/allison/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Allura" ), { QStringLiteral( "ofl/allura/Allura-Regular.ttf" ) }, QStringLiteral( "ofl/allura/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Almarai" ), { QStringLiteral( "ofl/almarai/Almarai-Light.ttf" ), QStringLiteral( "ofl/almarai/Almarai-Regular.ttf" ), QStringLiteral( "ofl/almarai/Almarai-Bold.ttf" ), QStringLiteral( "ofl/almarai/Almarai-ExtraBold.ttf" ) }, QStringLiteral( "ofl/almarai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Almendra" ), { QStringLiteral( "ofl/almendra/Almendra-Regular.ttf" ), QStringLiteral( "ofl/almendra/Almendra-Italic.ttf" ), QStringLiteral( "ofl/almendra/Almendra-Bold.ttf" ), QStringLiteral( "ofl/almendra/Almendra-BoldItalic.ttf" ) }, QStringLiteral( "ofl/almendra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Almendra Display" ), { QStringLiteral( "ofl/almendradisplay/AlmendraDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/almendradisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Almendra SC" ), { QStringLiteral( "ofl/almendrasc/AlmendraSC-Regular.ttf" ) }, QStringLiteral( "ofl/almendrasc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alumni Sans" ), { QStringLiteral( "ofl/alumnisans/AlumniSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/alumnisans/AlumniSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/alumnisans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alumni Sans Collegiate One" ), { QStringLiteral( "ofl/alumnisanscollegiateone/AlumniSansCollegiateOne-Regular.ttf" ), QStringLiteral( "ofl/alumnisanscollegiateone/AlumniSansCollegiateOne-Italic.ttf" ) }, QStringLiteral( "ofl/alumnisanscollegiateone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alumni Sans Inline One" ), { QStringLiteral( "ofl/alumnisansinlineone/AlumniSansInlineOne-Regular.ttf" ), QStringLiteral( "ofl/alumnisansinlineone/AlumniSansInlineOne-Italic.ttf" ) }, QStringLiteral( "ofl/alumnisansinlineone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Alumni Sans Pinstripe" ), { QStringLiteral( "ofl/alumnisanspinstripe/AlumniSansPinstripe-Regular.ttf" ), QStringLiteral( "ofl/alumnisanspinstripe/AlumniSansPinstripe-Italic.ttf" ) }, QStringLiteral( "ofl/alumnisanspinstripe/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amarante" ), { QStringLiteral( "ofl/amarante/Amarante-Regular.ttf" ) }, QStringLiteral( "ofl/amarante/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amaranth" ), { QStringLiteral( "ofl/amaranth/Amaranth-Regular.ttf" ), QStringLiteral( "ofl/amaranth/Amaranth-Italic.ttf" ), QStringLiteral( "ofl/amaranth/Amaranth-Bold.ttf" ), QStringLiteral( "ofl/amaranth/Amaranth-BoldItalic.ttf" ) }, QStringLiteral( "ofl/amaranth/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amatic SC" ), { QStringLiteral( "ofl/amaticsc/AmaticSC-Regular.ttf" ), QStringLiteral( "ofl/amaticsc/AmaticSC-Bold.ttf" ) }, QStringLiteral( "ofl/amaticsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amethysta" ), { QStringLiteral( "ofl/amethysta/Amethysta-Regular.ttf" ) }, QStringLiteral( "ofl/amethysta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amiko" ), { QStringLiteral( "ofl/amiko/Amiko-Regular.ttf" ), QStringLiteral( "ofl/amiko/Amiko-SemiBold.ttf" ), QStringLiteral( "ofl/amiko/Amiko-Bold.ttf" ) }, QStringLiteral( "ofl/amiko/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amiri" ), { QStringLiteral( "ofl/amiri/Amiri-Regular.ttf" ), QStringLiteral( "ofl/amiri/Amiri-Italic.ttf" ), QStringLiteral( "ofl/amiri/Amiri-Bold.ttf" ), QStringLiteral( "ofl/amiri/Amiri-BoldItalic.ttf" ) }, QStringLiteral( "ofl/amiri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amiri Quran" ), { QStringLiteral( "ofl/amiriquran/AmiriQuran-Regular.ttf" ) }, QStringLiteral( "ofl/amiriquran/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Amita" ), { QStringLiteral( "ofl/amita/Amita-Regular.ttf" ), QStringLiteral( "ofl/amita/Amita-Bold.ttf" ) }, QStringLiteral( "ofl/amita/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anaheim" ), { QStringLiteral( "ofl/anaheim/Anaheim%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/anaheim/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Andada Pro" ), { QStringLiteral( "ofl/andadapro/AndadaPro%5Bwght%5D.ttf" ), QStringLiteral( "ofl/andadapro/AndadaPro-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/andadapro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Andika" ), { QStringLiteral( "ofl/andika/Andika-Regular.ttf" ), QStringLiteral( "ofl/andika/Andika-Italic.ttf" ), QStringLiteral( "ofl/andika/Andika-Bold.ttf" ), QStringLiteral( "ofl/andika/Andika-BoldItalic.ttf" ) }, QStringLiteral( "ofl/andika/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Bangla" ), { QStringLiteral( "ofl/anekbangla/AnekBangla%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekbangla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Devanagari" ), { QStringLiteral( "ofl/anekdevanagari/AnekDevanagari%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekdevanagari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Gujarati" ), { QStringLiteral( "ofl/anekgujarati/AnekGujarati%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekgujarati/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Gurmukhi" ), { QStringLiteral( "ofl/anekgurmukhi/AnekGurmukhi%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekgurmukhi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Kannada" ), { QStringLiteral( "ofl/anekkannada/AnekKannada%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekkannada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Latin" ), { QStringLiteral( "ofl/aneklatin/AnekLatin%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/aneklatin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Malayalam" ), { QStringLiteral( "ofl/anekmalayalam/AnekMalayalam%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekmalayalam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Odia" ), { QStringLiteral( "ofl/anekodia/AnekOdia%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anekodia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Tamil" ), { QStringLiteral( "ofl/anektamil/AnekTamil%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anektamil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anek Telugu" ), { QStringLiteral( "ofl/anektelugu/AnekTelugu%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anektelugu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Angkor" ), { QStringLiteral( "ofl/angkor/Angkor-Regular.ttf" ) }, QStringLiteral( "ofl/angkor/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Annie Use Your Telescope" ), { QStringLiteral( "ofl/annieuseyourtelescope/AnnieUseYourTelescope-Regular.ttf" ) }, QStringLiteral( "ofl/annieuseyourtelescope/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anonymous Pro" ), { QStringLiteral( "ofl/anonymouspro/AnonymousPro-Regular.ttf" ), QStringLiteral( "ofl/anonymouspro/AnonymousPro-Italic.ttf" ), QStringLiteral( "ofl/anonymouspro/AnonymousPro-Bold.ttf" ), QStringLiteral( "ofl/anonymouspro/AnonymousPro-BoldItalic.ttf" ) }, QStringLiteral( "ofl/anonymouspro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Antic" ), { QStringLiteral( "ofl/antic/Antic-Regular.ttf" ) }, QStringLiteral( "ofl/antic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Antic Didone" ), { QStringLiteral( "ofl/anticdidone/AnticDidone-Regular.ttf" ) }, QStringLiteral( "ofl/anticdidone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Antic Slab" ), { QStringLiteral( "ofl/anticslab/AnticSlab-Regular.ttf" ) }, QStringLiteral( "ofl/anticslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anton" ), { QStringLiteral( "ofl/anton/Anton-Regular.ttf" ) }, QStringLiteral( "ofl/anton/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Antonio" ), { QStringLiteral( "ofl/antonio/Antonio%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/antonio/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anuphan" ), { QStringLiteral( "ofl/anuphan/Anuphan%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/anuphan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Anybody" ), { QStringLiteral( "ofl/anybody/Anybody%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/anybody/Anybody-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/anybody/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aoboshi One" ), { QStringLiteral( "ofl/aoboshione/AoboshiOne-Regular.ttf" ) }, QStringLiteral( "ofl/aoboshione/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arapey" ), { QStringLiteral( "ofl/arapey/Arapey-Regular.ttf" ), QStringLiteral( "ofl/arapey/Arapey-Italic.ttf" ) }, QStringLiteral( "ofl/arapey/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arbutus" ), { QStringLiteral( "ofl/arbutus/Arbutus-Regular.ttf" ) }, QStringLiteral( "ofl/arbutus/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arbutus Slab" ), { QStringLiteral( "ofl/arbutusslab/ArbutusSlab-Regular.ttf" ) }, QStringLiteral( "ofl/arbutusslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Architects Daughter" ), { QStringLiteral( "ofl/architectsdaughter/ArchitectsDaughter-Regular.ttf" ) }, QStringLiteral( "ofl/architectsdaughter/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Archivo" ), { QStringLiteral( "ofl/archivo/Archivo%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/archivo/Archivo-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/archivo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Archivo Black" ), { QStringLiteral( "ofl/archivoblack/ArchivoBlack-Regular.ttf" ) }, QStringLiteral( "ofl/archivoblack/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Archivo Narrow" ), { QStringLiteral( "ofl/archivonarrow/ArchivoNarrow%5Bwght%5D.ttf" ), QStringLiteral( "ofl/archivonarrow/ArchivoNarrow-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/archivonarrow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Are You Serious" ), { QStringLiteral( "ofl/areyouserious/AreYouSerious-Regular.ttf" ) }, QStringLiteral( "ofl/areyouserious/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aref Ruqaa" ), { QStringLiteral( "ofl/arefruqaa/ArefRuqaa-Regular.ttf" ), QStringLiteral( "ofl/arefruqaa/ArefRuqaa-Bold.ttf" ) }, QStringLiteral( "ofl/arefruqaa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aref Ruqaa Ink" ), { QStringLiteral( "ofl/arefruqaaink/ArefRuqaaInk-Regular.ttf" ), QStringLiteral( "ofl/arefruqaaink/ArefRuqaaInk-Bold.ttf" ) }, QStringLiteral( "ofl/arefruqaaink/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arima" ), { QStringLiteral( "ofl/arima/Arima%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/arima/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arimo" ), { QStringLiteral( "apache/arimo/Arimo%5Bwght%5D.ttf" ), QStringLiteral( "apache/arimo/Arimo-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "apache/arimo/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arizonia" ), { QStringLiteral( "ofl/arizonia/Arizonia-Regular.ttf" ) }, QStringLiteral( "ofl/arizonia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Armata" ), { QStringLiteral( "ofl/armata/Armata-Regular.ttf" ) }, QStringLiteral( "ofl/armata/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arsenal" ), { QStringLiteral( "ofl/arsenal/Arsenal-Regular.ttf" ), QStringLiteral( "ofl/arsenal/Arsenal-Italic.ttf" ), QStringLiteral( "ofl/arsenal/Arsenal-Bold.ttf" ), QStringLiteral( "ofl/arsenal/Arsenal-BoldItalic.ttf" ) }, QStringLiteral( "ofl/arsenal/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Artifika" ), { QStringLiteral( "ofl/artifika/Artifika-Regular.ttf" ) }, QStringLiteral( "ofl/artifika/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arvo" ), { QStringLiteral( "ofl/arvo/Arvo-Regular.ttf" ), QStringLiteral( "ofl/arvo/Arvo-Italic.ttf" ), QStringLiteral( "ofl/arvo/Arvo-Bold.ttf" ), QStringLiteral( "ofl/arvo/Arvo-BoldItalic.ttf" ) }, QStringLiteral( "ofl/arvo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Arya" ), { QStringLiteral( "ofl/arya/Arya-Regular.ttf" ), QStringLiteral( "ofl/arya/Arya-Bold.ttf" ) }, QStringLiteral( "ofl/arya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Asap" ), { QStringLiteral( "ofl/asap/Asap%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/asap/Asap-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/asap/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Asap Condensed" ), { QStringLiteral( "ofl/asapcondensed/AsapCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-Light.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-Regular.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-Italic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-Medium.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-Bold.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-BoldItalic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-Black.ttf" ), QStringLiteral( "ofl/asapcondensed/AsapCondensed-BlackItalic.ttf" ) }, QStringLiteral( "ofl/asapcondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Asar" ), { QStringLiteral( "ofl/asar/Asar-Regular.ttf" ) }, QStringLiteral( "ofl/asar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Asset" ), { QStringLiteral( "ofl/asset/Asset-Regular.ttf" ) }, QStringLiteral( "ofl/asset/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Assistant" ), { QStringLiteral( "ofl/assistant/Assistant%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/assistant/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Astloch" ), { QStringLiteral( "ofl/astloch/Astloch-Regular.ttf" ), QStringLiteral( "ofl/astloch/Astloch-Bold.ttf" ) }, QStringLiteral( "ofl/astloch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Asul" ), { QStringLiteral( "ofl/asul/Asul-Regular.ttf" ), QStringLiteral( "ofl/asul/Asul-Bold.ttf" ) }, QStringLiteral( "ofl/asul/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Athiti" ), { QStringLiteral( "ofl/athiti/Athiti-ExtraLight.ttf" ), QStringLiteral( "ofl/athiti/Athiti-Light.ttf" ), QStringLiteral( "ofl/athiti/Athiti-Regular.ttf" ), QStringLiteral( "ofl/athiti/Athiti-Medium.ttf" ), QStringLiteral( "ofl/athiti/Athiti-SemiBold.ttf" ), QStringLiteral( "ofl/athiti/Athiti-Bold.ttf" ) }, QStringLiteral( "ofl/athiti/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Atkinson Hyperlegible" ), { QStringLiteral( "ofl/atkinsonhyperlegible/AtkinsonHyperlegible-Regular.ttf" ), QStringLiteral( "ofl/atkinsonhyperlegible/AtkinsonHyperlegible-Italic.ttf" ), QStringLiteral( "ofl/atkinsonhyperlegible/AtkinsonHyperlegible-Bold.ttf" ), QStringLiteral( "ofl/atkinsonhyperlegible/AtkinsonHyperlegible-BoldItalic.ttf" ) }, QStringLiteral( "ofl/atkinsonhyperlegible/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Atomic Age" ), { QStringLiteral( "ofl/atomicage/AtomicAge-Regular.ttf" ) }, QStringLiteral( "ofl/atomicage/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Aubrey" ), { QStringLiteral( "ofl/aubrey/Aubrey-Regular.ttf" ) }, QStringLiteral( "ofl/aubrey/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Audiowide" ), { QStringLiteral( "ofl/audiowide/Audiowide-Regular.ttf" ) }, QStringLiteral( "ofl/audiowide/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Autour One" ), { QStringLiteral( "ofl/autourone/AutourOne-Regular.ttf" ) }, QStringLiteral( "ofl/autourone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Average" ), { QStringLiteral( "ofl/average/Average-Regular.ttf" ) }, QStringLiteral( "ofl/average/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Average Sans" ), { QStringLiteral( "ofl/averagesans/AverageSans-Regular.ttf" ) }, QStringLiteral( "ofl/averagesans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Averia Gruesa Libre" ), { QStringLiteral( "ofl/averiagruesalibre/AveriaGruesaLibre-Regular.ttf" ) }, QStringLiteral( "ofl/averiagruesalibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Averia Libre" ), { QStringLiteral( "ofl/averialibre/AveriaLibre-Light.ttf" ), QStringLiteral( "ofl/averialibre/AveriaLibre-LightItalic.ttf" ), QStringLiteral( "ofl/averialibre/AveriaLibre-Regular.ttf" ), QStringLiteral( "ofl/averialibre/AveriaLibre-Italic.ttf" ), QStringLiteral( "ofl/averialibre/AveriaLibre-Bold.ttf" ), QStringLiteral( "ofl/averialibre/AveriaLibre-BoldItalic.ttf" ) }, QStringLiteral( "ofl/averialibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Averia Sans Libre" ), { QStringLiteral( "ofl/averiasanslibre/AveriaSansLibre-Light.ttf" ), QStringLiteral( "ofl/averiasanslibre/AveriaSansLibre-LightItalic.ttf" ), QStringLiteral( "ofl/averiasanslibre/AveriaSansLibre-Regular.ttf" ), QStringLiteral( "ofl/averiasanslibre/AveriaSansLibre-Italic.ttf" ), QStringLiteral( "ofl/averiasanslibre/AveriaSansLibre-Bold.ttf" ), QStringLiteral( "ofl/averiasanslibre/AveriaSansLibre-BoldItalic.ttf" ) }, QStringLiteral( "ofl/averiasanslibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Averia Serif Libre" ), { QStringLiteral( "ofl/averiaseriflibre/AveriaSerifLibre-Light.ttf" ), QStringLiteral( "ofl/averiaseriflibre/AveriaSerifLibre-LightItalic.ttf" ), QStringLiteral( "ofl/averiaseriflibre/AveriaSerifLibre-Regular.ttf" ), QStringLiteral( "ofl/averiaseriflibre/AveriaSerifLibre-Italic.ttf" ), QStringLiteral( "ofl/averiaseriflibre/AveriaSerifLibre-Bold.ttf" ), QStringLiteral( "ofl/averiaseriflibre/AveriaSerifLibre-BoldItalic.ttf" ) }, QStringLiteral( "ofl/averiaseriflibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Azeret Mono" ), { QStringLiteral( "ofl/azeretmono/AzeretMono%5Bwght%5D.ttf" ), QStringLiteral( "ofl/azeretmono/AzeretMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/azeretmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "B612" ), { QStringLiteral( "ofl/b612/B612-Regular.ttf" ), QStringLiteral( "ofl/b612/B612-Italic.ttf" ), QStringLiteral( "ofl/b612/B612-Bold.ttf" ), QStringLiteral( "ofl/b612/B612-BoldItalic.ttf" ) }, QStringLiteral( "ofl/b612/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "B612 Mono" ), { QStringLiteral( "ofl/b612mono/B612Mono-Regular.ttf" ), QStringLiteral( "ofl/b612mono/B612Mono-Italic.ttf" ), QStringLiteral( "ofl/b612mono/B612Mono-Bold.ttf" ), QStringLiteral( "ofl/b612mono/B612Mono-BoldItalic.ttf" ) }, QStringLiteral( "ofl/b612mono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BIZ UDGothic" ), { QStringLiteral( "ofl/bizudgothic/BIZUDGothic-Regular.ttf" ), QStringLiteral( "ofl/bizudgothic/BIZUDGothic-Bold.ttf" ) }, QStringLiteral( "ofl/bizudgothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BIZ UDMincho" ), { QStringLiteral( "ofl/bizudmincho/BIZUDMincho-Regular.ttf" ), QStringLiteral( "ofl/bizudmincho/BIZUDMincho-Bold.ttf" ) }, QStringLiteral( "ofl/bizudmincho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BIZ UDPGothic" ), { QStringLiteral( "ofl/bizudpgothic/BIZUDPGothic-Regular.ttf" ), QStringLiteral( "ofl/bizudpgothic/BIZUDPGothic-Bold.ttf" ) }, QStringLiteral( "ofl/bizudpgothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BIZ UDPMincho" ), { QStringLiteral( "ofl/bizudpmincho/BIZUDPMincho-Regular.ttf" ), QStringLiteral( "ofl/bizudpmincho/BIZUDPMincho-Bold.ttf" ) }, QStringLiteral( "ofl/bizudpmincho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Babylonica" ), { QStringLiteral( "ofl/babylonica/Babylonica-Regular.ttf" ) }, QStringLiteral( "ofl/babylonica/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bacasime Antique" ), { QStringLiteral( "ofl/bacasimeantique/BacasimeAntique-Regular.ttf" ) }, QStringLiteral( "ofl/bacasimeantique/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bad Script" ), { QStringLiteral( "ofl/badscript/BadScript-Regular.ttf" ) }, QStringLiteral( "ofl/badscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bagel Fat One" ), { QStringLiteral( "ofl/bagelfatone/BagelFatOne-Regular.ttf" ) }, QStringLiteral( "ofl/bagelfatone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bahiana" ), { QStringLiteral( "ofl/bahiana/Bahiana-Regular.ttf" ) }, QStringLiteral( "ofl/bahiana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bahianita" ), { QStringLiteral( "ofl/bahianita/Bahianita-Regular.ttf" ) }, QStringLiteral( "ofl/bahianita/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bai Jamjuree" ), { QStringLiteral( "ofl/baijamjuree/BaiJamjuree-ExtraLight.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-Light.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-LightItalic.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-Regular.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-Italic.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-Medium.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-MediumItalic.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-SemiBold.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-Bold.ttf" ), QStringLiteral( "ofl/baijamjuree/BaiJamjuree-BoldItalic.ttf" ) }, QStringLiteral( "ofl/baijamjuree/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bakbak One" ), { QStringLiteral( "ofl/bakbakone/BakbakOne-Regular.ttf" ) }, QStringLiteral( "ofl/bakbakone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ballet" ), { QStringLiteral( "ofl/ballet/Ballet%5Bopsz%5D.ttf" ) }, QStringLiteral( "ofl/ballet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo 2" ), { QStringLiteral( "ofl/baloo2/Baloo2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloo2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Bhai 2" ), { QStringLiteral( "ofl/baloobhai2/BalooBhai2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloobhai2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Bhaijaan 2" ), { QStringLiteral( "ofl/baloobhaijaan2/BalooBhaijaan2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloobhaijaan2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Bhaina 2" ), { QStringLiteral( "ofl/baloobhaina2/BalooBhaina2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloobhaina2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Chettan 2" ), { QStringLiteral( "ofl/baloochettan2/BalooChettan2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloochettan2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Da 2" ), { QStringLiteral( "ofl/balooda2/BalooDa2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/balooda2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Paaji 2" ), { QStringLiteral( "ofl/baloopaaji2/BalooPaaji2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloopaaji2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Tamma 2" ), { QStringLiteral( "ofl/balootamma2/BalooTamma2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/balootamma2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Tammudu 2" ), { QStringLiteral( "ofl/balootammudu2/BalooTammudu2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/balootammudu2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baloo Thambi 2" ), { QStringLiteral( "ofl/baloothambi2/BalooThambi2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/baloothambi2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Balsamiq Sans" ), { QStringLiteral( "ofl/balsamiqsans/BalsamiqSans-Regular.ttf" ), QStringLiteral( "ofl/balsamiqsans/BalsamiqSans-Italic.ttf" ), QStringLiteral( "ofl/balsamiqsans/BalsamiqSans-Bold.ttf" ), QStringLiteral( "ofl/balsamiqsans/BalsamiqSans-BoldItalic.ttf" ) }, QStringLiteral( "ofl/balsamiqsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Balthazar" ), { QStringLiteral( "ofl/balthazar/Balthazar-Regular.ttf" ) }, QStringLiteral( "ofl/balthazar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bangers" ), { QStringLiteral( "ofl/bangers/Bangers-Regular.ttf" ) }, QStringLiteral( "ofl/bangers/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Barlow" ), { QStringLiteral( "ofl/barlow/Barlow-Thin.ttf" ), QStringLiteral( "ofl/barlow/Barlow-ThinItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-ExtraLight.ttf" ), QStringLiteral( "ofl/barlow/Barlow-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-Light.ttf" ), QStringLiteral( "ofl/barlow/Barlow-LightItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-Regular.ttf" ), QStringLiteral( "ofl/barlow/Barlow-Italic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-Medium.ttf" ), QStringLiteral( "ofl/barlow/Barlow-MediumItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-SemiBold.ttf" ), QStringLiteral( "ofl/barlow/Barlow-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-Bold.ttf" ), QStringLiteral( "ofl/barlow/Barlow-BoldItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-ExtraBold.ttf" ), QStringLiteral( "ofl/barlow/Barlow-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/barlow/Barlow-Black.ttf" ), QStringLiteral( "ofl/barlow/Barlow-BlackItalic.ttf" ) }, QStringLiteral( "ofl/barlow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Barlow Condensed" ), { QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Thin.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-ThinItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Light.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Regular.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Italic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Medium.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Bold.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-BoldItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-Black.ttf" ), QStringLiteral( "ofl/barlowcondensed/BarlowCondensed-BlackItalic.ttf" ) }, QStringLiteral( "ofl/barlowcondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Barlow Semi Condensed" ), { QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Thin.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-ThinItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Light.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Regular.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Italic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Medium.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Bold.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-BoldItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-Black.ttf" ), QStringLiteral( "ofl/barlowsemicondensed/BarlowSemiCondensed-BlackItalic.ttf" ) }, QStringLiteral( "ofl/barlowsemicondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Barriecito" ), { QStringLiteral( "ofl/barriecito/Barriecito-Regular.ttf" ) }, QStringLiteral( "ofl/barriecito/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Barrio" ), { QStringLiteral( "ofl/barrio/Barrio-Regular.ttf" ) }, QStringLiteral( "ofl/barrio/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Basic" ), { QStringLiteral( "ofl/basic/Basic-Regular.ttf" ) }, QStringLiteral( "ofl/basic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baskervville" ), { QStringLiteral( "ofl/baskervville/Baskervville-Regular.ttf" ), QStringLiteral( "ofl/baskervville/Baskervville-Italic.ttf" ) }, QStringLiteral( "ofl/baskervville/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Battambang" ), { QStringLiteral( "ofl/battambang/Battambang-Thin.ttf" ), QStringLiteral( "ofl/battambang/Battambang-Light.ttf" ), QStringLiteral( "ofl/battambang/Battambang-Regular.ttf" ), QStringLiteral( "ofl/battambang/Battambang-Bold.ttf" ), QStringLiteral( "ofl/battambang/Battambang-Black.ttf" ) }, QStringLiteral( "ofl/battambang/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Baumans" ), { QStringLiteral( "ofl/baumans/Baumans-Regular.ttf" ) }, QStringLiteral( "ofl/baumans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bayon" ), { QStringLiteral( "ofl/bayon/Bayon-Regular.ttf" ) }, QStringLiteral( "ofl/bayon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Be Vietnam Pro" ), { QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Thin.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-ThinItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-ExtraLight.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Light.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-LightItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Regular.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Italic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Medium.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-MediumItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-SemiBold.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Bold.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-BoldItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-ExtraBold.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-Black.ttf" ), QStringLiteral( "ofl/bevietnampro/BeVietnamPro-BlackItalic.ttf" ) }, QStringLiteral( "ofl/bevietnampro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Beau Rivage" ), { QStringLiteral( "ofl/beaurivage/BeauRivage-Regular.ttf" ) }, QStringLiteral( "ofl/beaurivage/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bebas Neue" ), { QStringLiteral( "ofl/bebasneue/BebasNeue-Regular.ttf" ) }, QStringLiteral( "ofl/bebasneue/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Belanosima" ), { QStringLiteral( "ofl/belanosima/Belanosima-Regular.ttf" ), QStringLiteral( "ofl/belanosima/Belanosima-SemiBold.ttf" ), QStringLiteral( "ofl/belanosima/Belanosima-Bold.ttf" ) }, QStringLiteral( "ofl/belanosima/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Belgrano" ), { QStringLiteral( "ofl/belgrano/Belgrano-Regular.ttf" ) }, QStringLiteral( "ofl/belgrano/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bellefair" ), { QStringLiteral( "ofl/bellefair/Bellefair-Regular.ttf" ) }, QStringLiteral( "ofl/bellefair/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Belleza" ), { QStringLiteral( "ofl/belleza/Belleza-Regular.ttf" ) }, QStringLiteral( "ofl/belleza/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bellota" ), { QStringLiteral( "ofl/bellota/Bellota-Light.ttf" ), QStringLiteral( "ofl/bellota/Bellota-LightItalic.ttf" ), QStringLiteral( "ofl/bellota/Bellota-Regular.ttf" ), QStringLiteral( "ofl/bellota/Bellota-Italic.ttf" ), QStringLiteral( "ofl/bellota/Bellota-Bold.ttf" ), QStringLiteral( "ofl/bellota/Bellota-BoldItalic.ttf" ) }, QStringLiteral( "ofl/bellota/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bellota Text" ), { QStringLiteral( "ofl/bellotatext/BellotaText-Light.ttf" ), QStringLiteral( "ofl/bellotatext/BellotaText-LightItalic.ttf" ), QStringLiteral( "ofl/bellotatext/BellotaText-Regular.ttf" ), QStringLiteral( "ofl/bellotatext/BellotaText-Italic.ttf" ), QStringLiteral( "ofl/bellotatext/BellotaText-Bold.ttf" ), QStringLiteral( "ofl/bellotatext/BellotaText-BoldItalic.ttf" ) }, QStringLiteral( "ofl/bellotatext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BenchNine" ), { QStringLiteral( "ofl/benchnine/BenchNine-Light.ttf" ), QStringLiteral( "ofl/benchnine/BenchNine-Regular.ttf" ), QStringLiteral( "ofl/benchnine/BenchNine-Bold.ttf" ) }, QStringLiteral( "ofl/benchnine/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Benne" ), { QStringLiteral( "ofl/benne/Benne-Regular.ttf" ) }, QStringLiteral( "ofl/benne/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bentham" ), { QStringLiteral( "ofl/bentham/Bentham-Regular.ttf" ) }, QStringLiteral( "ofl/bentham/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Berkshire Swash" ), { QStringLiteral( "ofl/berkshireswash/BerkshireSwash-Regular.ttf" ) }, QStringLiteral( "ofl/berkshireswash/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Besley" ), { QStringLiteral( "ofl/besley/Besley%5Bwght%5D.ttf" ), QStringLiteral( "ofl/besley/Besley-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/besley/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Beth Ellen" ), { QStringLiteral( "ofl/bethellen/BethEllen-Regular.ttf" ) }, QStringLiteral( "ofl/bethellen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bevan" ), { QStringLiteral( "ofl/bevan/Bevan-Regular.ttf" ), QStringLiteral( "ofl/bevan/Bevan-Italic.ttf" ) }, QStringLiteral( "ofl/bevan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BhuTuka Expanded One" ), { QStringLiteral( "ofl/bhutukaexpandedone/BhuTukaExpandedOne-Regular.ttf" ) }, QStringLiteral( "ofl/bhutukaexpandedone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Big Shoulders Display" ), { QStringLiteral( "ofl/bigshouldersdisplay/BigShouldersDisplay%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bigshouldersdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Big Shoulders Inline Display" ), { QStringLiteral( "ofl/bigshouldersinlinedisplay/BigShouldersInlineDisplay%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bigshouldersinlinedisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Big Shoulders Inline Text" ), { QStringLiteral( "ofl/bigshouldersinlinetext/BigShouldersInlineText%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bigshouldersinlinetext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Big Shoulders Stencil Display" ), { QStringLiteral( "ofl/bigshouldersstencildisplay/BigShouldersStencilDisplay%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bigshouldersstencildisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Big Shoulders Stencil Text" ), { QStringLiteral( "ofl/bigshouldersstenciltext/BigShouldersStencilText%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bigshouldersstenciltext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Big Shoulders Text" ), { QStringLiteral( "ofl/bigshoulderstext/BigShouldersText%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bigshoulderstext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bigelow Rules" ), { QStringLiteral( "ofl/bigelowrules/BigelowRules-Regular.ttf" ) }, QStringLiteral( "ofl/bigelowrules/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bigshot One" ), { QStringLiteral( "ofl/bigshotone/BigshotOne-Regular.ttf" ) }, QStringLiteral( "ofl/bigshotone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bilbo" ), { QStringLiteral( "ofl/bilbo/Bilbo-Regular.ttf" ) }, QStringLiteral( "ofl/bilbo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bilbo Swash Caps" ), { QStringLiteral( "ofl/bilboswashcaps/BilboSwashCaps-Regular.ttf" ) }, QStringLiteral( "ofl/bilboswashcaps/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BioRhyme" ), { QStringLiteral( "ofl/biorhyme/BioRhyme%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/biorhyme/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "BioRhyme Expanded" ), { QStringLiteral( "ofl/biorhymeexpanded/BioRhymeExpanded-ExtraLight.ttf" ), QStringLiteral( "ofl/biorhymeexpanded/BioRhymeExpanded-Light.ttf" ), QStringLiteral( "ofl/biorhymeexpanded/BioRhymeExpanded-Regular.ttf" ), QStringLiteral( "ofl/biorhymeexpanded/BioRhymeExpanded-Bold.ttf" ), QStringLiteral( "ofl/biorhymeexpanded/BioRhymeExpanded-ExtraBold.ttf" ) }, QStringLiteral( "ofl/biorhymeexpanded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Birthstone" ), { QStringLiteral( "ofl/birthstone/Birthstone-Regular.ttf" ) }, QStringLiteral( "ofl/birthstone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Birthstone Bounce" ), { QStringLiteral( "ofl/birthstonebounce/BirthstoneBounce-Regular.ttf" ), QStringLiteral( "ofl/birthstonebounce/BirthstoneBounce-Medium.ttf" ) }, QStringLiteral( "ofl/birthstonebounce/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Biryani" ), { QStringLiteral( "ofl/biryani/Biryani-ExtraLight.ttf" ), QStringLiteral( "ofl/biryani/Biryani-Light.ttf" ), QStringLiteral( "ofl/biryani/Biryani-Regular.ttf" ), QStringLiteral( "ofl/biryani/Biryani-SemiBold.ttf" ), QStringLiteral( "ofl/biryani/Biryani-Bold.ttf" ), QStringLiteral( "ofl/biryani/Biryani-ExtraBold.ttf" ), QStringLiteral( "ofl/biryani/Biryani-Black.ttf" ) }, QStringLiteral( "ofl/biryani/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bitter" ), { QStringLiteral( "ofl/bitter/Bitter%5Bwght%5D.ttf" ), QStringLiteral( "ofl/bitter/Bitter-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/bitter/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Black And White Picture" ), { QStringLiteral( "ofl/blackandwhitepicture/BlackAndWhitePicture-Regular.ttf" ) }, QStringLiteral( "ofl/blackandwhitepicture/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Black Han Sans" ), { QStringLiteral( "ofl/blackhansans/BlackHanSans-Regular.ttf" ) }, QStringLiteral( "ofl/blackhansans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Black Ops One" ), { QStringLiteral( "ofl/blackopsone/BlackOpsOne-Regular.ttf" ) }, QStringLiteral( "ofl/blackopsone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Blaka" ), { QStringLiteral( "ofl/blaka/Blaka-Regular.ttf" ) }, QStringLiteral( "ofl/blaka/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Blaka Hollow" ), { QStringLiteral( "ofl/blakahollow/BlakaHollow-Regular.ttf" ) }, QStringLiteral( "ofl/blakahollow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Blaka Ink" ), { QStringLiteral( "ofl/blakaink/BlakaInk-Regular.ttf" ) }, QStringLiteral( "ofl/blakaink/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bodoni Moda" ), { QStringLiteral( "ofl/bodonimoda/BodoniModa%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/bodonimoda/BodoniModa-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/bodonimoda/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bokor" ), { QStringLiteral( "ofl/bokor/Bokor-Regular.ttf" ) }, QStringLiteral( "ofl/bokor/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bona Nova" ), { QStringLiteral( "ofl/bonanova/BonaNova-Regular.ttf" ), QStringLiteral( "ofl/bonanova/BonaNova-Italic.ttf" ), QStringLiteral( "ofl/bonanova/BonaNova-Bold.ttf" ) }, QStringLiteral( "ofl/bonanova/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bonbon" ), { QStringLiteral( "ofl/bonbon/Bonbon-Regular.ttf" ) }, QStringLiteral( "ofl/bonbon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bonheur Royale" ), { QStringLiteral( "ofl/bonheurroyale/BonheurRoyale-Regular.ttf" ) }, QStringLiteral( "ofl/bonheurroyale/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Boogaloo" ), { QStringLiteral( "ofl/boogaloo/Boogaloo-Regular.ttf" ) }, QStringLiteral( "ofl/boogaloo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Borel" ), { QStringLiteral( "ofl/borel/Borel-Regular.ttf" ) }, QStringLiteral( "ofl/borel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bowlby One" ), { QStringLiteral( "ofl/bowlbyone/BowlbyOne-Regular.ttf" ) }, QStringLiteral( "ofl/bowlbyone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bowlby One SC" ), { QStringLiteral( "ofl/bowlbyonesc/BowlbyOneSC-Regular.ttf" ) }, QStringLiteral( "ofl/bowlbyonesc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Braah One" ), { QStringLiteral( "ofl/braahone/BraahOne-Regular.ttf" ) }, QStringLiteral( "ofl/braahone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Brawler" ), { QStringLiteral( "ofl/brawler/Brawler-Regular.ttf" ), QStringLiteral( "ofl/brawler/Brawler-Bold.ttf" ) }, QStringLiteral( "ofl/brawler/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bree Serif" ), { QStringLiteral( "ofl/breeserif/BreeSerif-Regular.ttf" ) }, QStringLiteral( "ofl/breeserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bricolage Grotesque" ), { QStringLiteral( "ofl/bricolagegrotesque/BricolageGrotesque%5Bopsz,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/bricolagegrotesque/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bruno Ace" ), { QStringLiteral( "ofl/brunoace/BrunoAce-Regular.ttf" ) }, QStringLiteral( "ofl/brunoace/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bruno Ace SC" ), { QStringLiteral( "ofl/brunoacesc/BrunoAceSC-Regular.ttf" ) }, QStringLiteral( "ofl/brunoacesc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Brygada 1918" ), { QStringLiteral( "ofl/brygada1918/Brygada1918%5Bwght%5D.ttf" ), QStringLiteral( "ofl/brygada1918/Brygada1918-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/brygada1918/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bubblegum Sans" ), { QStringLiteral( "ofl/bubblegumsans/BubblegumSans-Regular.ttf" ) }, QStringLiteral( "ofl/bubblegumsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bubbler One" ), { QStringLiteral( "ofl/bubblerone/BubblerOne-Regular.ttf" ) }, QStringLiteral( "ofl/bubblerone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Buda" ), { QStringLiteral( "ofl/buda/Buda-Light.ttf" ) }, QStringLiteral( "ofl/buda/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Buenard" ), { QStringLiteral( "ofl/buenard/Buenard-Regular.ttf" ), QStringLiteral( "ofl/buenard/Buenard-Bold.ttf" ) }, QStringLiteral( "ofl/buenard/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bungee" ), { QStringLiteral( "ofl/bungee/Bungee-Regular.ttf" ) }, QStringLiteral( "ofl/bungee/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bungee Hairline" ), { QStringLiteral( "ofl/bungeehairline/BungeeHairline-Regular.ttf" ) }, QStringLiteral( "ofl/bungeehairline/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bungee Inline" ), { QStringLiteral( "ofl/bungeeinline/BungeeInline-Regular.ttf" ) }, QStringLiteral( "ofl/bungeeinline/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bungee Outline" ), { QStringLiteral( "ofl/bungeeoutline/BungeeOutline-Regular.ttf" ) }, QStringLiteral( "ofl/bungeeoutline/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bungee Shade" ), { QStringLiteral( "ofl/bungeeshade/BungeeShade-Regular.ttf" ) }, QStringLiteral( "ofl/bungeeshade/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Bungee Spice" ), { QStringLiteral( "ofl/bungeespice/BungeeSpice-Regular.ttf" ) }, QStringLiteral( "ofl/bungeespice/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Butcherman" ), { QStringLiteral( "ofl/butcherman/Butcherman-Regular.ttf" ) }, QStringLiteral( "ofl/butcherman/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Butterfly Kids" ), { QStringLiteral( "ofl/butterflykids/ButterflyKids-Regular.ttf" ) }, QStringLiteral( "ofl/butterflykids/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cabin" ), { QStringLiteral( "ofl/cabin/Cabin%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/cabin/Cabin-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/cabin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cabin Condensed" ), { QStringLiteral( "ofl/cabincondensed/CabinCondensed-Regular.ttf" ), QStringLiteral( "ofl/cabincondensed/CabinCondensed-Medium.ttf" ), QStringLiteral( "ofl/cabincondensed/CabinCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/cabincondensed/CabinCondensed-Bold.ttf" ) }, QStringLiteral( "ofl/cabincondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cabin Sketch" ), { QStringLiteral( "ofl/cabinsketch/CabinSketch-Regular.ttf" ), QStringLiteral( "ofl/cabinsketch/CabinSketch-Bold.ttf" ) }, QStringLiteral( "ofl/cabinsketch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caesar Dressing" ), { QStringLiteral( "ofl/caesardressing/CaesarDressing-Regular.ttf" ) }, QStringLiteral( "ofl/caesardressing/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cagliostro" ), { QStringLiteral( "ofl/cagliostro/Cagliostro-Regular.ttf" ) }, QStringLiteral( "ofl/cagliostro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cairo" ), { QStringLiteral( "ofl/cairo/Cairo%5Bslnt,wght%5D.ttf" ) }, QStringLiteral( "ofl/cairo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cairo Play" ), { QStringLiteral( "ofl/cairoplay/CairoPlay%5Bslnt,wght%5D.ttf" ) }, QStringLiteral( "ofl/cairoplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caladea" ), { QStringLiteral( "ofl/caladea/Caladea-Regular.ttf" ), QStringLiteral( "ofl/caladea/Caladea-Italic.ttf" ), QStringLiteral( "ofl/caladea/Caladea-Bold.ttf" ), QStringLiteral( "ofl/caladea/Caladea-BoldItalic.ttf" ) }, QStringLiteral( "ofl/caladea/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Calistoga" ), { QStringLiteral( "ofl/calistoga/Calistoga-Regular.ttf" ) }, QStringLiteral( "ofl/calistoga/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Calligraffitti" ), { QStringLiteral( "apache/calligraffitti/Calligraffitti-Regular.ttf" ) }, QStringLiteral( "apache/calligraffitti/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cambay" ), { QStringLiteral( "ofl/cambay/Cambay-Regular.ttf" ), QStringLiteral( "ofl/cambay/Cambay-Italic.ttf" ), QStringLiteral( "ofl/cambay/Cambay-Bold.ttf" ), QStringLiteral( "ofl/cambay/Cambay-BoldItalic.ttf" ) }, QStringLiteral( "ofl/cambay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cambo" ), { QStringLiteral( "ofl/cambo/Cambo-Regular.ttf" ) }, QStringLiteral( "ofl/cambo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Candal" ), { QStringLiteral( "ofl/candal/Candal.ttf" ) }, QStringLiteral( "ofl/candal/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cantarell" ), { QStringLiteral( "ofl/cantarell/Cantarell-Regular.ttf" ), QStringLiteral( "ofl/cantarell/Cantarell-Italic.ttf" ), QStringLiteral( "ofl/cantarell/Cantarell-Bold.ttf" ), QStringLiteral( "ofl/cantarell/Cantarell-BoldItalic.ttf" ) }, QStringLiteral( "ofl/cantarell/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cantata One" ), { QStringLiteral( "ofl/cantataone/CantataOne-Regular.ttf" ) }, QStringLiteral( "ofl/cantataone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cantora One" ), { QStringLiteral( "ofl/cantoraone/CantoraOne-Regular.ttf" ) }, QStringLiteral( "ofl/cantoraone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caprasimo" ), { QStringLiteral( "ofl/caprasimo/Caprasimo-Regular.ttf" ) }, QStringLiteral( "ofl/caprasimo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Capriola" ), { QStringLiteral( "ofl/capriola/Capriola-Regular.ttf" ) }, QStringLiteral( "ofl/capriola/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caramel" ), { QStringLiteral( "ofl/caramel/Caramel-Regular.ttf" ) }, QStringLiteral( "ofl/caramel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Carattere" ), { QStringLiteral( "ofl/carattere/Carattere-Regular.ttf" ) }, QStringLiteral( "ofl/carattere/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cardo" ), { QStringLiteral( "ofl/cardo/Cardo-Regular.ttf" ), QStringLiteral( "ofl/cardo/Cardo-Italic.ttf" ), QStringLiteral( "ofl/cardo/Cardo-Bold.ttf" ) }, QStringLiteral( "ofl/cardo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Carlito" ), { QStringLiteral( "ofl/carlito/Carlito-Regular.ttf" ), QStringLiteral( "ofl/carlito/Carlito-Italic.ttf" ), QStringLiteral( "ofl/carlito/Carlito-Bold.ttf" ), QStringLiteral( "ofl/carlito/Carlito-BoldItalic.ttf" ) }, QStringLiteral( "ofl/carlito/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Carme" ), { QStringLiteral( "ofl/carme/Carme-Regular.ttf" ) }, QStringLiteral( "ofl/carme/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Carrois Gothic" ), { QStringLiteral( "ofl/carroisgothic/CarroisGothic-Regular.ttf" ) }, QStringLiteral( "ofl/carroisgothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Carrois Gothic SC" ), { QStringLiteral( "ofl/carroisgothicsc/CarroisGothicSC-Regular.ttf" ) }, QStringLiteral( "ofl/carroisgothicsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Carter One" ), { QStringLiteral( "ofl/carterone/CarterOne.ttf" ) }, QStringLiteral( "ofl/carterone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Castoro" ), { QStringLiteral( "ofl/castoro/Castoro-Regular.ttf" ), QStringLiteral( "ofl/castoro/Castoro-Italic.ttf" ) }, QStringLiteral( "ofl/castoro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Castoro Titling" ), { QStringLiteral( "ofl/castorotitling/CastoroTitling-Regular.ttf" ) }, QStringLiteral( "ofl/castorotitling/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Catamaran" ), { QStringLiteral( "ofl/catamaran/Catamaran%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/catamaran/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caudex" ), { QStringLiteral( "ofl/caudex/Caudex-Regular.ttf" ), QStringLiteral( "ofl/caudex/Caudex-Italic.ttf" ), QStringLiteral( "ofl/caudex/Caudex-Bold.ttf" ), QStringLiteral( "ofl/caudex/Caudex-BoldItalic.ttf" ) }, QStringLiteral( "ofl/caudex/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caveat" ), { QStringLiteral( "ofl/caveat/Caveat%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/caveat/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Caveat Brush" ), { QStringLiteral( "ofl/caveatbrush/CaveatBrush-Regular.ttf" ) }, QStringLiteral( "ofl/caveatbrush/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cedarville Cursive" ), { QStringLiteral( "ofl/cedarvillecursive/Cedarville-Cursive.ttf" ) }, QStringLiteral( "ofl/cedarvillecursive/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ceviche One" ), { QStringLiteral( "ofl/cevicheone/CevicheOne-Regular.ttf" ) }, QStringLiteral( "ofl/cevicheone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chakra Petch" ), { QStringLiteral( "ofl/chakrapetch/ChakraPetch-Light.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-LightItalic.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-Regular.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-Italic.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-Medium.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-MediumItalic.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-SemiBold.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-Bold.ttf" ), QStringLiteral( "ofl/chakrapetch/ChakraPetch-BoldItalic.ttf" ) }, QStringLiteral( "ofl/chakrapetch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Changa" ), { QStringLiteral( "ofl/changa/Changa%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/changa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Changa One" ), { QStringLiteral( "ofl/changaone/ChangaOne-Regular.ttf" ), QStringLiteral( "ofl/changaone/ChangaOne-Italic.ttf" ) }, QStringLiteral( "ofl/changaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chango" ), { QStringLiteral( "ofl/chango/Chango-Regular.ttf" ) }, QStringLiteral( "ofl/chango/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Charis SIL" ), { QStringLiteral( "ofl/charissil/CharisSIL-Regular.ttf" ), QStringLiteral( "ofl/charissil/CharisSIL-Italic.ttf" ), QStringLiteral( "ofl/charissil/CharisSIL-Bold.ttf" ), QStringLiteral( "ofl/charissil/CharisSIL-BoldItalic.ttf" ) }, QStringLiteral( "ofl/charissil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Charm" ), { QStringLiteral( "ofl/charm/Charm-Regular.ttf" ), QStringLiteral( "ofl/charm/Charm-Bold.ttf" ) }, QStringLiteral( "ofl/charm/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Charmonman" ), { QStringLiteral( "ofl/charmonman/Charmonman-Regular.ttf" ), QStringLiteral( "ofl/charmonman/Charmonman-Bold.ttf" ) }, QStringLiteral( "ofl/charmonman/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chau Philomene One" ), { QStringLiteral( "ofl/chauphilomeneone/ChauPhilomeneOne-Regular.ttf" ), QStringLiteral( "ofl/chauphilomeneone/ChauPhilomeneOne-Italic.ttf" ) }, QStringLiteral( "ofl/chauphilomeneone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chela One" ), { QStringLiteral( "ofl/chelaone/ChelaOne-Regular.ttf" ) }, QStringLiteral( "ofl/chelaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chelsea Market" ), { QStringLiteral( "ofl/chelseamarket/ChelseaMarket-Regular.ttf" ) }, QStringLiteral( "ofl/chelseamarket/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chenla" ), { QStringLiteral( "ofl/chenla/Chenla.ttf" ) }, QStringLiteral( "ofl/chenla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cherish" ), { QStringLiteral( "ofl/cherish/Cherish-Regular.ttf" ) }, QStringLiteral( "ofl/cherish/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cherry Bomb One" ), { QStringLiteral( "ofl/cherrybombone/CherryBombOne-Regular.ttf" ) }, QStringLiteral( "ofl/cherrybombone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cherry Cream Soda" ), { QStringLiteral( "apache/cherrycreamsoda/CherryCreamSoda-Regular.ttf" ) }, QStringLiteral( "apache/cherrycreamsoda/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cherry Swash" ), { QStringLiteral( "ofl/cherryswash/CherrySwash-Regular.ttf" ), QStringLiteral( "ofl/cherryswash/CherrySwash-Bold.ttf" ) }, QStringLiteral( "ofl/cherryswash/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chewy" ), { QStringLiteral( "apache/chewy/Chewy-Regular.ttf" ) }, QStringLiteral( "apache/chewy/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chicle" ), { QStringLiteral( "ofl/chicle/Chicle-Regular.ttf" ) }, QStringLiteral( "ofl/chicle/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chilanka" ), { QStringLiteral( "ofl/chilanka/Chilanka-Regular.ttf" ) }, QStringLiteral( "ofl/chilanka/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chivo" ), { QStringLiteral( "ofl/chivo/Chivo%5Bwght%5D.ttf" ), QStringLiteral( "ofl/chivo/Chivo-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/chivo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chivo Mono" ), { QStringLiteral( "ofl/chivomono/ChivoMono%5Bwght%5D.ttf" ), QStringLiteral( "ofl/chivomono/ChivoMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/chivomono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chokokutai" ), { QStringLiteral( "ofl/chokokutai/Chokokutai-Regular.ttf" ) }, QStringLiteral( "ofl/chokokutai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Chonburi" ), { QStringLiteral( "ofl/chonburi/Chonburi-Regular.ttf" ) }, QStringLiteral( "ofl/chonburi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cinzel" ), { QStringLiteral( "ofl/cinzel/Cinzel%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/cinzel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cinzel Decorative" ), { QStringLiteral( "ofl/cinzeldecorative/CinzelDecorative-Regular.ttf" ), QStringLiteral( "ofl/cinzeldecorative/CinzelDecorative-Bold.ttf" ), QStringLiteral( "ofl/cinzeldecorative/CinzelDecorative-Black.ttf" ) }, QStringLiteral( "ofl/cinzeldecorative/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Clicker Script" ), { QStringLiteral( "ofl/clickerscript/ClickerScript-Regular.ttf" ) }, QStringLiteral( "ofl/clickerscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Climate Crisis" ), { QStringLiteral( "ofl/climatecrisis/ClimateCrisis%5BYEAR%5D.ttf" ) }, QStringLiteral( "ofl/climatecrisis/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Coda" ), { QStringLiteral( "ofl/coda/Coda-Regular.ttf" ), QStringLiteral( "ofl/coda/Coda-ExtraBold.ttf" ) }, QStringLiteral( "ofl/coda/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Codystar" ), { QStringLiteral( "ofl/codystar/Codystar-Light.ttf" ), QStringLiteral( "ofl/codystar/Codystar-Regular.ttf" ) }, QStringLiteral( "ofl/codystar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Coiny" ), { QStringLiteral( "ofl/coiny/Coiny-Regular.ttf" ) }, QStringLiteral( "ofl/coiny/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Combo" ), { QStringLiteral( "ofl/combo/Combo-Regular.ttf" ) }, QStringLiteral( "ofl/combo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Comfortaa" ), { QStringLiteral( "ofl/comfortaa/Comfortaa%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/comfortaa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Comforter" ), { QStringLiteral( "ofl/comforter/Comforter-Regular.ttf" ) }, QStringLiteral( "ofl/comforter/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Comforter Brush" ), { QStringLiteral( "ofl/comforterbrush/ComforterBrush-Regular.ttf" ) }, QStringLiteral( "ofl/comforterbrush/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Comic Neue" ), { QStringLiteral( "ofl/comicneue/ComicNeue-Light.ttf" ), QStringLiteral( "ofl/comicneue/ComicNeue-LightItalic.ttf" ), QStringLiteral( "ofl/comicneue/ComicNeue-Regular.ttf" ), QStringLiteral( "ofl/comicneue/ComicNeue-Italic.ttf" ), QStringLiteral( "ofl/comicneue/ComicNeue-Bold.ttf" ), QStringLiteral( "ofl/comicneue/ComicNeue-BoldItalic.ttf" ) }, QStringLiteral( "ofl/comicneue/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Coming Soon" ), { QStringLiteral( "apache/comingsoon/ComingSoon-Regular.ttf" ) }, QStringLiteral( "apache/comingsoon/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Comme" ), { QStringLiteral( "ofl/comme/Comme%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/comme/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Commissioner" ), { QStringLiteral( "ofl/commissioner/Commissioner%5BFLAR,VOLM,slnt,wght%5D.ttf" ) }, QStringLiteral( "ofl/commissioner/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Concert One" ), { QStringLiteral( "ofl/concertone/ConcertOne-Regular.ttf" ) }, QStringLiteral( "ofl/concertone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Condiment" ), { QStringLiteral( "ofl/condiment/Condiment-Regular.ttf" ) }, QStringLiteral( "ofl/condiment/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Content" ), { QStringLiteral( "ofl/content/Content-Regular.ttf" ), QStringLiteral( "ofl/content/Content-Bold.ttf" ) }, QStringLiteral( "ofl/content/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Contrail One" ), { QStringLiteral( "ofl/contrailone/ContrailOne-Regular.ttf" ) }, QStringLiteral( "ofl/contrailone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Convergence" ), { QStringLiteral( "ofl/convergence/Convergence-Regular.ttf" ) }, QStringLiteral( "ofl/convergence/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cookie" ), { QStringLiteral( "ofl/cookie/Cookie-Regular.ttf" ) }, QStringLiteral( "ofl/cookie/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Copse" ), { QStringLiteral( "ofl/copse/Copse-Regular.ttf" ) }, QStringLiteral( "ofl/copse/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Corben" ), { QStringLiteral( "ofl/corben/Corben-Regular.ttf" ), QStringLiteral( "ofl/corben/Corben-Bold.ttf" ) }, QStringLiteral( "ofl/corben/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Corinthia" ), { QStringLiteral( "ofl/corinthia/Corinthia-Regular.ttf" ), QStringLiteral( "ofl/corinthia/Corinthia-Bold.ttf" ) }, QStringLiteral( "ofl/corinthia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cormorant" ), { QStringLiteral( "ofl/cormorant/Cormorant%5Bwght%5D.ttf" ), QStringLiteral( "ofl/cormorant/Cormorant-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/cormorant/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cormorant Garamond" ), { QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-Light.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-LightItalic.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-Regular.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-Italic.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-Medium.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-MediumItalic.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-SemiBold.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-Bold.ttf" ), QStringLiteral( "ofl/cormorantgaramond/CormorantGaramond-BoldItalic.ttf" ) }, QStringLiteral( "ofl/cormorantgaramond/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cormorant Infant" ), { QStringLiteral( "ofl/cormorantinfant/CormorantInfant-Light.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-LightItalic.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-Regular.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-Italic.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-Medium.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-MediumItalic.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-SemiBold.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-Bold.ttf" ), QStringLiteral( "ofl/cormorantinfant/CormorantInfant-BoldItalic.ttf" ) }, QStringLiteral( "ofl/cormorantinfant/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cormorant SC" ), { QStringLiteral( "ofl/cormorantsc/CormorantSC-Light.ttf" ), QStringLiteral( "ofl/cormorantsc/CormorantSC-Regular.ttf" ), QStringLiteral( "ofl/cormorantsc/CormorantSC-Medium.ttf" ), QStringLiteral( "ofl/cormorantsc/CormorantSC-SemiBold.ttf" ), QStringLiteral( "ofl/cormorantsc/CormorantSC-Bold.ttf" ) }, QStringLiteral( "ofl/cormorantsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cormorant Unicase" ), { QStringLiteral( "ofl/cormorantunicase/CormorantUnicase-Light.ttf" ), QStringLiteral( "ofl/cormorantunicase/CormorantUnicase-Regular.ttf" ), QStringLiteral( "ofl/cormorantunicase/CormorantUnicase-Medium.ttf" ), QStringLiteral( "ofl/cormorantunicase/CormorantUnicase-SemiBold.ttf" ), QStringLiteral( "ofl/cormorantunicase/CormorantUnicase-Bold.ttf" ) }, QStringLiteral( "ofl/cormorantunicase/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cormorant Upright" ), { QStringLiteral( "ofl/cormorantupright/CormorantUpright-Light.ttf" ), QStringLiteral( "ofl/cormorantupright/CormorantUpright-Regular.ttf" ), QStringLiteral( "ofl/cormorantupright/CormorantUpright-Medium.ttf" ), QStringLiteral( "ofl/cormorantupright/CormorantUpright-SemiBold.ttf" ), QStringLiteral( "ofl/cormorantupright/CormorantUpright-Bold.ttf" ) }, QStringLiteral( "ofl/cormorantupright/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Courgette" ), { QStringLiteral( "ofl/courgette/Courgette-Regular.ttf" ) }, QStringLiteral( "ofl/courgette/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Courier Prime" ), { QStringLiteral( "ofl/courierprime/CourierPrime-Regular.ttf" ), QStringLiteral( "ofl/courierprime/CourierPrime-Italic.ttf" ), QStringLiteral( "ofl/courierprime/CourierPrime-Bold.ttf" ), QStringLiteral( "ofl/courierprime/CourierPrime-BoldItalic.ttf" ) }, QStringLiteral( "ofl/courierprime/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cousine" ), { QStringLiteral( "apache/cousine/Cousine-Regular.ttf" ), QStringLiteral( "apache/cousine/Cousine-Italic.ttf" ), QStringLiteral( "apache/cousine/Cousine-Bold.ttf" ), QStringLiteral( "apache/cousine/Cousine-BoldItalic.ttf" ) }, QStringLiteral( "apache/cousine/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Coustard" ), { QStringLiteral( "ofl/coustard/Coustard-Regular.ttf" ), QStringLiteral( "ofl/coustard/Coustard-Black.ttf" ) }, QStringLiteral( "ofl/coustard/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Covered By Your Grace" ), { QStringLiteral( "ofl/coveredbyyourgrace/CoveredByYourGrace.ttf" ) }, QStringLiteral( "ofl/coveredbyyourgrace/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Crafty Girls" ), { QStringLiteral( "apache/craftygirls/CraftyGirls-Regular.ttf" ) }, QStringLiteral( "apache/craftygirls/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Creepster" ), { QStringLiteral( "ofl/creepster/Creepster-Regular.ttf" ) }, QStringLiteral( "ofl/creepster/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Crete Round" ), { QStringLiteral( "ofl/creteround/CreteRound-Regular.ttf" ), QStringLiteral( "ofl/creteround/CreteRound-Italic.ttf" ) }, QStringLiteral( "ofl/creteround/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Crimson Pro" ), { QStringLiteral( "ofl/crimsonpro/CrimsonPro%5Bwght%5D.ttf" ), QStringLiteral( "ofl/crimsonpro/CrimsonPro-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/crimsonpro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Crimson Text" ), { QStringLiteral( "ofl/crimsontext/CrimsonText-Regular.ttf" ), QStringLiteral( "ofl/crimsontext/CrimsonText-Italic.ttf" ), QStringLiteral( "ofl/crimsontext/CrimsonText-SemiBold.ttf" ), QStringLiteral( "ofl/crimsontext/CrimsonText-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/crimsontext/CrimsonText-Bold.ttf" ), QStringLiteral( "ofl/crimsontext/CrimsonText-BoldItalic.ttf" ) }, QStringLiteral( "ofl/crimsontext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Croissant One" ), { QStringLiteral( "ofl/croissantone/CroissantOne-Regular.ttf" ) }, QStringLiteral( "ofl/croissantone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Crushed" ), { QStringLiteral( "apache/crushed/Crushed-Regular.ttf" ) }, QStringLiteral( "apache/crushed/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cuprum" ), { QStringLiteral( "ofl/cuprum/Cuprum%5Bwght%5D.ttf" ), QStringLiteral( "ofl/cuprum/Cuprum-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/cuprum/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cute Font" ), { QStringLiteral( "ofl/cutefont/CuteFont-Regular.ttf" ) }, QStringLiteral( "ofl/cutefont/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cutive" ), { QStringLiteral( "ofl/cutive/Cutive-Regular.ttf" ) }, QStringLiteral( "ofl/cutive/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Cutive Mono" ), { QStringLiteral( "ofl/cutivemono/CutiveMono-Regular.ttf" ) }, QStringLiteral( "ofl/cutivemono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "DM Mono" ), { QStringLiteral( "ofl/dmmono/DMMono-Light.ttf" ), QStringLiteral( "ofl/dmmono/DMMono-LightItalic.ttf" ), QStringLiteral( "ofl/dmmono/DMMono-Regular.ttf" ), QStringLiteral( "ofl/dmmono/DMMono-Italic.ttf" ), QStringLiteral( "ofl/dmmono/DMMono-Medium.ttf" ), QStringLiteral( "ofl/dmmono/DMMono-MediumItalic.ttf" ) }, QStringLiteral( "ofl/dmmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "DM Sans" ), { QStringLiteral( "ofl/dmsans/DMSans%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/dmsans/DMSans-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/dmsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "DM Serif Display" ), { QStringLiteral( "ofl/dmserifdisplay/DMSerifDisplay-Regular.ttf" ), QStringLiteral( "ofl/dmserifdisplay/DMSerifDisplay-Italic.ttf" ) }, QStringLiteral( "ofl/dmserifdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "DM Serif Text" ), { QStringLiteral( "ofl/dmseriftext/DMSerifText-Regular.ttf" ), QStringLiteral( "ofl/dmseriftext/DMSerifText-Italic.ttf" ) }, QStringLiteral( "ofl/dmseriftext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dai Banna SIL" ), { QStringLiteral( "ofl/daibannasil/DaiBannaSIL-Light.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-LightItalic.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-Regular.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-Italic.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-Medium.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-MediumItalic.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-SemiBold.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-Bold.ttf" ), QStringLiteral( "ofl/daibannasil/DaiBannaSIL-BoldItalic.ttf" ) }, QStringLiteral( "ofl/daibannasil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Damion" ), { QStringLiteral( "ofl/damion/Damion-Regular.ttf" ) }, QStringLiteral( "ofl/damion/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dancing Script" ), { QStringLiteral( "ofl/dancingscript/DancingScript%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/dancingscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dangrek" ), { QStringLiteral( "ofl/dangrek/Dangrek-Regular.ttf" ) }, QStringLiteral( "ofl/dangrek/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Darker Grotesque" ), { QStringLiteral( "ofl/darkergrotesque/DarkerGrotesque%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/darkergrotesque/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Darumadrop One" ), { QStringLiteral( "ofl/darumadropone/DarumadropOne-Regular.ttf" ) }, QStringLiteral( "ofl/darumadropone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "David Libre" ), { QStringLiteral( "ofl/davidlibre/DavidLibre-Regular.ttf" ), QStringLiteral( "ofl/davidlibre/DavidLibre-Medium.ttf" ), QStringLiteral( "ofl/davidlibre/DavidLibre-Bold.ttf" ) }, QStringLiteral( "ofl/davidlibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dawning of a New Day" ), { QStringLiteral( "ofl/dawningofanewday/DawningofaNewDay.ttf" ) }, QStringLiteral( "ofl/dawningofanewday/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Days One" ), { QStringLiteral( "ofl/daysone/DaysOne-Regular.ttf" ) }, QStringLiteral( "ofl/daysone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dekko" ), { QStringLiteral( "ofl/dekko/Dekko-Regular.ttf" ) }, QStringLiteral( "ofl/dekko/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Delicious Handrawn" ), { QStringLiteral( "ofl/delicioushandrawn/DeliciousHandrawn-Regular.ttf" ) }, QStringLiteral( "ofl/delicioushandrawn/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Delius" ), { QStringLiteral( "ofl/delius/Delius-Regular.ttf" ) }, QStringLiteral( "ofl/delius/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Delius Swash Caps" ), { QStringLiteral( "ofl/deliusswashcaps/DeliusSwashCaps-Regular.ttf" ) }, QStringLiteral( "ofl/deliusswashcaps/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Delius Unicase" ), { QStringLiteral( "ofl/deliusunicase/DeliusUnicase-Regular.ttf" ), QStringLiteral( "ofl/deliusunicase/DeliusUnicase-Bold.ttf" ) }, QStringLiteral( "ofl/deliusunicase/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Della Respira" ), { QStringLiteral( "ofl/dellarespira/DellaRespira-Regular.ttf" ) }, QStringLiteral( "ofl/dellarespira/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Denk One" ), { QStringLiteral( "ofl/denkone/DenkOne-Regular.ttf" ) }, QStringLiteral( "ofl/denkone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Devonshire" ), { QStringLiteral( "ofl/devonshire/Devonshire-Regular.ttf" ) }, QStringLiteral( "ofl/devonshire/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dhurjati" ), { QStringLiteral( "ofl/dhurjati/Dhurjati-Regular.ttf" ) }, QStringLiteral( "ofl/dhurjati/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Didact Gothic" ), { QStringLiteral( "ofl/didactgothic/DidactGothic-Regular.ttf" ) }, QStringLiteral( "ofl/didactgothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Diphylleia" ), { QStringLiteral( "ofl/diphylleia/Diphylleia-Regular.ttf" ) }, QStringLiteral( "ofl/diphylleia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Diplomata" ), { QStringLiteral( "ofl/diplomata/Diplomata-Regular.ttf" ) }, QStringLiteral( "ofl/diplomata/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Diplomata SC" ), { QStringLiteral( "ofl/diplomatasc/DiplomataSC-Regular.ttf" ) }, QStringLiteral( "ofl/diplomatasc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Do Hyeon" ), { QStringLiteral( "ofl/dohyeon/DoHyeon-Regular.ttf" ) }, QStringLiteral( "ofl/dohyeon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dokdo" ), { QStringLiteral( "ofl/dokdo/Dokdo-Regular.ttf" ) }, QStringLiteral( "ofl/dokdo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Domine" ), { QStringLiteral( "ofl/domine/Domine%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/domine/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Donegal One" ), { QStringLiteral( "ofl/donegalone/DonegalOne-Regular.ttf" ) }, QStringLiteral( "ofl/donegalone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dongle" ), { QStringLiteral( "ofl/dongle/Dongle-Light.ttf" ), QStringLiteral( "ofl/dongle/Dongle-Regular.ttf" ), QStringLiteral( "ofl/dongle/Dongle-Bold.ttf" ) }, QStringLiteral( "ofl/dongle/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Doppio One" ), { QStringLiteral( "ofl/doppioone/DoppioOne-Regular.ttf" ) }, QStringLiteral( "ofl/doppioone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dorsa" ), { QStringLiteral( "ofl/dorsa/Dorsa-Regular.ttf" ) }, QStringLiteral( "ofl/dorsa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dosis" ), { QStringLiteral( "ofl/dosis/Dosis%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/dosis/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "DotGothic16" ), { QStringLiteral( "ofl/dotgothic16/DotGothic16-Regular.ttf" ) }, QStringLiteral( "ofl/dotgothic16/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dr Sugiyama" ), { QStringLiteral( "ofl/drsugiyama/DrSugiyama-Regular.ttf" ) }, QStringLiteral( "ofl/drsugiyama/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Duru Sans" ), { QStringLiteral( "ofl/durusans/DuruSans-Regular.ttf" ) }, QStringLiteral( "ofl/durusans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "DynaPuff" ), { QStringLiteral( "ofl/dynapuff/DynaPuff%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/dynapuff/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Dynalight" ), { QStringLiteral( "ofl/dynalight/Dynalight-Regular.ttf" ) }, QStringLiteral( "ofl/dynalight/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "EB Garamond" ), { QStringLiteral( "ofl/ebgaramond/EBGaramond%5Bwght%5D.ttf" ), QStringLiteral( "ofl/ebgaramond/EBGaramond-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ebgaramond/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Eagle Lake" ), { QStringLiteral( "ofl/eaglelake/EagleLake-Regular.ttf" ) }, QStringLiteral( "ofl/eaglelake/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "East Sea Dokdo" ), { QStringLiteral( "ofl/eastseadokdo/EastSeaDokdo-Regular.ttf" ) }, QStringLiteral( "ofl/eastseadokdo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Eater" ), { QStringLiteral( "ofl/eater/Eater-Regular.ttf" ) }, QStringLiteral( "ofl/eater/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Economica" ), { QStringLiteral( "ofl/economica/Economica-Regular.ttf" ), QStringLiteral( "ofl/economica/Economica-Italic.ttf" ), QStringLiteral( "ofl/economica/Economica-Bold.ttf" ), QStringLiteral( "ofl/economica/Economica-BoldItalic.ttf" ) }, QStringLiteral( "ofl/economica/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Eczar" ), { QStringLiteral( "ofl/eczar/Eczar%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/eczar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Edu NSW ACT Foundation" ), { QStringLiteral( "ofl/edunswactfoundation/EduNSWACTFoundation%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/edunswactfoundation/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Edu QLD Beginner" ), { QStringLiteral( "ofl/eduqldbeginner/EduQLDBeginner%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/eduqldbeginner/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Edu SA Beginner" ), { QStringLiteral( "ofl/edusabeginner/EduSABeginner%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/edusabeginner/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Edu TAS Beginner" ), { QStringLiteral( "ofl/edutasbeginner/EduTASBeginner%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/edutasbeginner/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Edu VIC WA NT Beginner" ), { QStringLiteral( "ofl/eduvicwantbeginner/EduVICWANTBeginner%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/eduvicwantbeginner/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ek Mukta" ), { QStringLiteral( "ofl/ekmukta/EkMukta-ExtraLight.ttf" ), QStringLiteral( "ofl/ekmukta/EkMukta-Light.ttf" ), QStringLiteral( "ofl/ekmukta/EkMukta-Regular.ttf" ), QStringLiteral( "ofl/ekmukta/EkMukta-Medium.ttf" ), QStringLiteral( "ofl/ekmukta/EkMukta-SemiBold.ttf" ), QStringLiteral( "ofl/ekmukta/EkMukta-Bold.ttf" ), QStringLiteral( "ofl/ekmukta/EkMukta-ExtraBold.ttf" ) }, QStringLiteral( "ofl/ekmukta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "El Messiri" ), { QStringLiteral( "ofl/elmessiri/ElMessiri%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/elmessiri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Electrolize" ), { QStringLiteral( "ofl/electrolize/Electrolize-Regular.ttf" ) }, QStringLiteral( "ofl/electrolize/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Elsie" ), { QStringLiteral( "ofl/elsie/Elsie-Regular.ttf" ), QStringLiteral( "ofl/elsie/Elsie-Black.ttf" ) }, QStringLiteral( "ofl/elsie/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Elsie Swash Caps" ), { QStringLiteral( "ofl/elsieswashcaps/ElsieSwashCaps-Regular.ttf" ), QStringLiteral( "ofl/elsieswashcaps/ElsieSwashCaps-Black.ttf" ) }, QStringLiteral( "ofl/elsieswashcaps/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Emblema One" ), { QStringLiteral( "ofl/emblemaone/EmblemaOne-Regular.ttf" ) }, QStringLiteral( "ofl/emblemaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Emilys Candy" ), { QStringLiteral( "ofl/emilyscandy/EmilysCandy-Regular.ttf" ) }, QStringLiteral( "ofl/emilyscandy/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Encode Sans" ), { QStringLiteral( "ofl/encodesans/EncodeSans%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/encodesans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Encode Sans Condensed" ), { QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-Thin.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-Light.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-Regular.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-Medium.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-Bold.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/encodesanscondensed/EncodeSansCondensed-Black.ttf" ) }, QStringLiteral( "ofl/encodesanscondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Encode Sans Expanded" ), { QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-Thin.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-ExtraLight.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-Light.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-Regular.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-Medium.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-SemiBold.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-Bold.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-ExtraBold.ttf" ), QStringLiteral( "ofl/encodesansexpanded/EncodeSansExpanded-Black.ttf" ) }, QStringLiteral( "ofl/encodesansexpanded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Encode Sans SC" ), { QStringLiteral( "ofl/encodesanssc/EncodeSansSC%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/encodesanssc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Encode Sans Semi Condensed" ), { QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Thin.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Light.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Regular.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Medium.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Bold.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Black.ttf" ) }, QStringLiteral( "ofl/encodesanssemicondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Encode Sans Semi Expanded" ), { QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Thin.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-ExtraLight.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Light.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Regular.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Medium.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-SemiBold.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Bold.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-ExtraBold.ttf" ), QStringLiteral( "ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Black.ttf" ) }, QStringLiteral( "ofl/encodesanssemiexpanded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Engagement" ), { QStringLiteral( "ofl/engagement/Engagement-Regular.ttf" ) }, QStringLiteral( "ofl/engagement/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Englebert" ), { QStringLiteral( "ofl/englebert/Englebert-Regular.ttf" ) }, QStringLiteral( "ofl/englebert/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Enriqueta" ), { QStringLiteral( "ofl/enriqueta/Enriqueta-Regular.ttf" ), QStringLiteral( "ofl/enriqueta/Enriqueta-Medium.ttf" ), QStringLiteral( "ofl/enriqueta/Enriqueta-SemiBold.ttf" ), QStringLiteral( "ofl/enriqueta/Enriqueta-Bold.ttf" ) }, QStringLiteral( "ofl/enriqueta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ephesis" ), { QStringLiteral( "ofl/ephesis/Ephesis-Regular.ttf" ) }, QStringLiteral( "ofl/ephesis/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Epilogue" ), { QStringLiteral( "ofl/epilogue/Epilogue%5Bwght%5D.ttf" ), QStringLiteral( "ofl/epilogue/Epilogue-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/epilogue/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Erica One" ), { QStringLiteral( "ofl/ericaone/EricaOne-Regular.ttf" ) }, QStringLiteral( "ofl/ericaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Esteban" ), { QStringLiteral( "ofl/esteban/Esteban-Regular.ttf" ) }, QStringLiteral( "ofl/esteban/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Estonia" ), { QStringLiteral( "ofl/estonia/Estonia-Regular.ttf" ) }, QStringLiteral( "ofl/estonia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Euphoria Script" ), { QStringLiteral( "ofl/euphoriascript/EuphoriaScript-Regular.ttf" ) }, QStringLiteral( "ofl/euphoriascript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ewert" ), { QStringLiteral( "ofl/ewert/Ewert-Regular.ttf" ) }, QStringLiteral( "ofl/ewert/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Exo" ), { QStringLiteral( "ofl/exo/Exo%5Bwght%5D.ttf" ), QStringLiteral( "ofl/exo/Exo-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/exo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Exo 2" ), { QStringLiteral( "ofl/exo2/Exo2%5Bwght%5D.ttf" ), QStringLiteral( "ofl/exo2/Exo2-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/exo2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Expletus Sans" ), { QStringLiteral( "ofl/expletussans/ExpletusSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/expletussans/ExpletusSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/expletussans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Explora" ), { QStringLiteral( "ofl/explora/Explora-Regular.ttf" ) }, QStringLiteral( "ofl/explora/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fahkwang" ), { QStringLiteral( "ofl/fahkwang/Fahkwang-ExtraLight.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-Light.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-LightItalic.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-Regular.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-Italic.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-Medium.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-MediumItalic.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-SemiBold.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-Bold.ttf" ), QStringLiteral( "ofl/fahkwang/Fahkwang-BoldItalic.ttf" ) }, QStringLiteral( "ofl/fahkwang/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Familjen Grotesk" ), { QStringLiteral( "ofl/familjengrotesk/FamiljenGrotesk%5Bwght%5D.ttf" ), QStringLiteral( "ofl/familjengrotesk/FamiljenGrotesk-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/familjengrotesk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fanwood Text" ), { QStringLiteral( "ofl/fanwoodtext/FanwoodText-Regular.ttf" ), QStringLiteral( "ofl/fanwoodtext/FanwoodText-Italic.ttf" ) }, QStringLiteral( "ofl/fanwoodtext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Farro" ), { QStringLiteral( "ofl/farro/Farro-Light.ttf" ), QStringLiteral( "ofl/farro/Farro-Regular.ttf" ), QStringLiteral( "ofl/farro/Farro-Medium.ttf" ), QStringLiteral( "ofl/farro/Farro-Bold.ttf" ) }, QStringLiteral( "ofl/farro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Farsan" ), { QStringLiteral( "ofl/farsan/Farsan-Regular.ttf" ) }, QStringLiteral( "ofl/farsan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fascinate" ), { QStringLiteral( "ofl/fascinate/Fascinate-Regular.ttf" ) }, QStringLiteral( "ofl/fascinate/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fascinate Inline" ), { QStringLiteral( "ofl/fascinateinline/FascinateInline-Regular.ttf" ) }, QStringLiteral( "ofl/fascinateinline/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Faster One" ), { QStringLiteral( "ofl/fasterone/FasterOne-Regular.ttf" ) }, QStringLiteral( "ofl/fasterone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fasthand" ), { QStringLiteral( "ofl/fasthand/Fasthand-Regular.ttf" ) }, QStringLiteral( "ofl/fasthand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fauna One" ), { QStringLiteral( "ofl/faunaone/FaunaOne-Regular.ttf" ) }, QStringLiteral( "ofl/faunaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Faustina" ), { QStringLiteral( "ofl/faustina/Faustina%5Bwght%5D.ttf" ), QStringLiteral( "ofl/faustina/Faustina-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/faustina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Federant" ), { QStringLiteral( "ofl/federant/Federant-Regular.ttf" ) }, QStringLiteral( "ofl/federant/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Federo" ), { QStringLiteral( "ofl/federo/Federo-Regular.ttf" ) }, QStringLiteral( "ofl/federo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Felipa" ), { QStringLiteral( "ofl/felipa/Felipa-Regular.ttf" ) }, QStringLiteral( "ofl/felipa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fenix" ), { QStringLiteral( "ofl/fenix/Fenix-Regular.ttf" ) }, QStringLiteral( "ofl/fenix/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Festive" ), { QStringLiteral( "ofl/festive/Festive-Regular.ttf" ) }, QStringLiteral( "ofl/festive/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Figtree" ), { QStringLiteral( "ofl/figtree/Figtree%5Bwght%5D.ttf" ), QStringLiteral( "ofl/figtree/Figtree-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/figtree/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Finger Paint" ), { QStringLiteral( "ofl/fingerpaint/FingerPaint-Regular.ttf" ) }, QStringLiteral( "ofl/fingerpaint/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Finlandica" ), { QStringLiteral( "ofl/finlandica/Finlandica%5Bwght%5D.ttf" ), QStringLiteral( "ofl/finlandica/Finlandica-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/finlandica/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fira Code" ), { QStringLiteral( "ofl/firacode/FiraCode%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/firacode/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fira Mono" ), { QStringLiteral( "ofl/firamono/FiraMono-Regular.ttf" ), QStringLiteral( "ofl/firamono/FiraMono-Medium.ttf" ), QStringLiteral( "ofl/firamono/FiraMono-Bold.ttf" ) }, QStringLiteral( "ofl/firamono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fira Sans" ), { QStringLiteral( "ofl/firasans/FiraSans-Thin.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-ThinItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-ExtraLight.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-Light.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-LightItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-Regular.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-Italic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-Medium.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-MediumItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-SemiBold.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-Bold.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-BoldItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-ExtraBold.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-Black.ttf" ), QStringLiteral( "ofl/firasans/FiraSans-BlackItalic.ttf" ) }, QStringLiteral( "ofl/firasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fira Sans Condensed" ), { QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Thin.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-ThinItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Light.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Regular.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Italic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Medium.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Bold.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-BoldItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-Black.ttf" ), QStringLiteral( "ofl/firasanscondensed/FiraSansCondensed-BlackItalic.ttf" ) }, QStringLiteral( "ofl/firasanscondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fira Sans Extra Condensed" ), { QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Thin.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ThinItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Light.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Regular.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Italic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Medium.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Bold.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-BoldItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Black.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-BlackItalic.ttf" ) }, QStringLiteral( "ofl/firasansextracondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fira Sans Extra Condensed" ), { QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Thin.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ThinItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Light.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Regular.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Italic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Medium.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Bold.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-BoldItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-Black.ttf" ), QStringLiteral( "ofl/firasansextracondensed/FiraSansExtraCondensed-BlackItalic.ttf" ) }, QStringLiteral( "ofl/firasansextracondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fjalla One" ), { QStringLiteral( "ofl/fjallaone/FjallaOne-Regular.ttf" ) }, QStringLiteral( "ofl/fjallaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fjord One" ), { QStringLiteral( "ofl/fjordone/FjordOne-Regular.ttf" ) }, QStringLiteral( "ofl/fjordone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Flamenco" ), { QStringLiteral( "ofl/flamenco/Flamenco-Light.ttf" ), QStringLiteral( "ofl/flamenco/Flamenco-Regular.ttf" ) }, QStringLiteral( "ofl/flamenco/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Flavors" ), { QStringLiteral( "ofl/flavors/Flavors-Regular.ttf" ) }, QStringLiteral( "ofl/flavors/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fleur De Leah" ), { QStringLiteral( "ofl/fleurdeleah/FleurDeLeah-Regular.ttf" ) }, QStringLiteral( "ofl/fleurdeleah/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Flow Block" ), { QStringLiteral( "ofl/flowblock/FlowBlock-Regular.ttf" ) }, QStringLiteral( "ofl/flowblock/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Flow Circular" ), { QStringLiteral( "ofl/flowcircular/FlowCircular-Regular.ttf" ) }, QStringLiteral( "ofl/flowcircular/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Flow Rounded" ), { QStringLiteral( "ofl/flowrounded/FlowRounded-Regular.ttf" ) }, QStringLiteral( "ofl/flowrounded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Foldit" ), { QStringLiteral( "ofl/foldit/Foldit%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/foldit/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fondamento" ), { QStringLiteral( "ofl/fondamento/Fondamento-Regular.ttf" ), QStringLiteral( "ofl/fondamento/Fondamento-Italic.ttf" ) }, QStringLiteral( "ofl/fondamento/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fontdiner Swanky" ), { QStringLiteral( "apache/fontdinerswanky/FontdinerSwanky-Regular.ttf" ) }, QStringLiteral( "apache/fontdinerswanky/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Forum" ), { QStringLiteral( "ofl/forum/Forum-Regular.ttf" ) }, QStringLiteral( "ofl/forum/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fragment Mono" ), { QStringLiteral( "ofl/fragmentmono/FragmentMono-Regular.ttf" ), QStringLiteral( "ofl/fragmentmono/FragmentMono-Italic.ttf" ) }, QStringLiteral( "ofl/fragmentmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Francois One" ), { QStringLiteral( "ofl/francoisone/FrancoisOne-Regular.ttf" ) }, QStringLiteral( "ofl/francoisone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Frank Ruhl Libre" ), { QStringLiteral( "ofl/frankruhllibre/FrankRuhlLibre%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/frankruhllibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fraunces" ), { QStringLiteral( "ofl/fraunces/Fraunces%5BSOFT,WONK,opsz,wght%5D.ttf" ), QStringLiteral( "ofl/fraunces/Fraunces-Italic%5BSOFT,WONK,opsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/fraunces/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Freckle Face" ), { QStringLiteral( "ofl/freckleface/FreckleFace-Regular.ttf" ) }, QStringLiteral( "ofl/freckleface/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fredericka the Great" ), { QStringLiteral( "ofl/frederickathegreat/FrederickatheGreat-Regular.ttf" ) }, QStringLiteral( "ofl/frederickathegreat/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fredoka" ), { QStringLiteral( "ofl/fredoka/Fredoka%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/fredoka/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Freehand" ), { QStringLiteral( "ofl/freehand/Freehand-Regular.ttf" ) }, QStringLiteral( "ofl/freehand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fresca" ), { QStringLiteral( "ofl/fresca/Fresca-Regular.ttf" ) }, QStringLiteral( "ofl/fresca/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Frijole" ), { QStringLiteral( "ofl/frijole/Frijole-Regular.ttf" ) }, QStringLiteral( "ofl/frijole/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fruktur" ), { QStringLiteral( "ofl/fruktur/Fruktur-Regular.ttf" ), QStringLiteral( "ofl/fruktur/Fruktur-Italic.ttf" ) }, QStringLiteral( "ofl/fruktur/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fugaz One" ), { QStringLiteral( "ofl/fugazone/FugazOne-Regular.ttf" ) }, QStringLiteral( "ofl/fugazone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fuggles" ), { QStringLiteral( "ofl/fuggles/Fuggles-Regular.ttf" ) }, QStringLiteral( "ofl/fuggles/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Fuzzy Bubbles" ), { QStringLiteral( "ofl/fuzzybubbles/FuzzyBubbles-Regular.ttf" ), QStringLiteral( "ofl/fuzzybubbles/FuzzyBubbles-Bold.ttf" ) }, QStringLiteral( "ofl/fuzzybubbles/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "GFS Didot" ), { QStringLiteral( "ofl/gfsdidot/GFSDidot-Regular.ttf" ) }, QStringLiteral( "ofl/gfsdidot/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "GFS Neohellenic" ), { QStringLiteral( "ofl/gfsneohellenic/GFSNeohellenic.ttf" ), QStringLiteral( "ofl/gfsneohellenic/GFSNeohellenicItalic.ttf" ), QStringLiteral( "ofl/gfsneohellenic/GFSNeohellenicBold.ttf" ), QStringLiteral( "ofl/gfsneohellenic/GFSNeohellenicBoldItalic.ttf" ) }, QStringLiteral( "ofl/gfsneohellenic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gabriela" ), { QStringLiteral( "ofl/gabriela/Gabriela-Regular.ttf" ) }, QStringLiteral( "ofl/gabriela/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gaegu" ), { QStringLiteral( "ofl/gaegu/Gaegu-Light.ttf" ), QStringLiteral( "ofl/gaegu/Gaegu-Regular.ttf" ), QStringLiteral( "ofl/gaegu/Gaegu-Bold.ttf" ) }, QStringLiteral( "ofl/gaegu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gafata" ), { QStringLiteral( "ofl/gafata/Gafata-Regular.ttf" ) }, QStringLiteral( "ofl/gafata/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gajraj One" ), { QStringLiteral( "ofl/gajrajone/GajrajOne-Regular.ttf" ) }, QStringLiteral( "ofl/gajrajone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Galada" ), { QStringLiteral( "ofl/galada/Galada-Regular.ttf" ) }, QStringLiteral( "ofl/galada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Galdeano" ), { QStringLiteral( "ofl/galdeano/Galdeano-Regular.ttf" ) }, QStringLiteral( "ofl/galdeano/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Galindo" ), { QStringLiteral( "ofl/galindo/Galindo-Regular.ttf" ) }, QStringLiteral( "ofl/galindo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gamja Flower" ), { QStringLiteral( "ofl/gamjaflower/GamjaFlower-Regular.ttf" ) }, QStringLiteral( "ofl/gamjaflower/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gantari" ), { QStringLiteral( "ofl/gantari/Gantari%5Bwght%5D.ttf" ), QStringLiteral( "ofl/gantari/Gantari-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/gantari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gasoek One" ), { QStringLiteral( "ofl/gasoekone/GasoekOne-Regular.ttf" ) }, QStringLiteral( "ofl/gasoekone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gayathri" ), { QStringLiteral( "ofl/gayathri/Gayathri-Thin.ttf" ), QStringLiteral( "ofl/gayathri/Gayathri-Regular.ttf" ), QStringLiteral( "ofl/gayathri/Gayathri-Bold.ttf" ) }, QStringLiteral( "ofl/gayathri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gelasio" ), { QStringLiteral( "ofl/gelasio/Gelasio%5Bwght%5D.ttf" ), QStringLiteral( "ofl/gelasio/Gelasio-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/gelasio/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gemunu Libre" ), { QStringLiteral( "ofl/gemunulibre/GemunuLibre%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/gemunulibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Genos" ), { QStringLiteral( "ofl/genos/Genos%5Bwght%5D.ttf" ), QStringLiteral( "ofl/genos/Genos-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/genos/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Geo" ), { QStringLiteral( "ofl/geo/Geo-Regular.ttf" ), QStringLiteral( "ofl/geo/Geo-Oblique.ttf" ) }, QStringLiteral( "ofl/geo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Geologica" ), { QStringLiteral( "ofl/geologica/Geologica%5BCRSV,SHRP,slnt,wght%5D.ttf" ) }, QStringLiteral( "ofl/geologica/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Georama" ), { QStringLiteral( "ofl/georama/Georama%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/georama/Georama-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/georama/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Geostar" ), { QStringLiteral( "ofl/geostar/Geostar-Regular.ttf" ) }, QStringLiteral( "ofl/geostar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Geostar Fill" ), { QStringLiteral( "ofl/geostarfill/GeostarFill-Regular.ttf" ) }, QStringLiteral( "ofl/geostarfill/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Germania One" ), { QStringLiteral( "ofl/germaniaone/GermaniaOne-Regular.ttf" ) }, QStringLiteral( "ofl/germaniaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gideon Roman" ), { QStringLiteral( "ofl/gideonroman/GideonRoman-Regular.ttf" ) }, QStringLiteral( "ofl/gideonroman/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gidugu" ), { QStringLiteral( "ofl/gidugu/Gidugu-Regular.ttf" ) }, QStringLiteral( "ofl/gidugu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gilda Display" ), { QStringLiteral( "ofl/gildadisplay/GildaDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/gildadisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Girassol" ), { QStringLiteral( "ofl/girassol/Girassol-Regular.ttf" ) }, QStringLiteral( "ofl/girassol/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Give You Glory" ), { QStringLiteral( "ofl/giveyouglory/GiveYouGlory.ttf" ) }, QStringLiteral( "ofl/giveyouglory/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Glass Antiqua" ), { QStringLiteral( "ofl/glassantiqua/GlassAntiqua-Regular.ttf" ) }, QStringLiteral( "ofl/glassantiqua/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Glegoo" ), { QStringLiteral( "ofl/glegoo/Glegoo-Regular.ttf" ), QStringLiteral( "ofl/glegoo/Glegoo-Bold.ttf" ) }, QStringLiteral( "ofl/glegoo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gloock" ), { QStringLiteral( "ofl/gloock/Gloock-Regular.ttf" ) }, QStringLiteral( "ofl/gloock/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gloria Hallelujah" ), { QStringLiteral( "ofl/gloriahallelujah/GloriaHallelujah.ttf" ) }, QStringLiteral( "ofl/gloriahallelujah/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Glory" ), { QStringLiteral( "ofl/glory/Glory%5Bwght%5D.ttf" ), QStringLiteral( "ofl/glory/Glory-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/glory/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gluten" ), { QStringLiteral( "ofl/gluten/Gluten%5Bslnt,wght%5D.ttf" ) }, QStringLiteral( "ofl/gluten/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Goblin One" ), { QStringLiteral( "ofl/goblinone/GoblinOne.ttf" ) }, QStringLiteral( "ofl/goblinone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gochi Hand" ), { QStringLiteral( "ofl/gochihand/GochiHand-Regular.ttf" ) }, QStringLiteral( "ofl/gochihand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Goldman" ), { QStringLiteral( "ofl/goldman/Goldman-Regular.ttf" ), QStringLiteral( "ofl/goldman/Goldman-Bold.ttf" ) }, QStringLiteral( "ofl/goldman/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Golos Text" ), { QStringLiteral( "ofl/golostext/GolosText%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/golostext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gorditas" ), { QStringLiteral( "ofl/gorditas/Gorditas-Regular.ttf" ), QStringLiteral( "ofl/gorditas/Gorditas-Bold.ttf" ) }, QStringLiteral( "ofl/gorditas/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gothic A1" ), { QStringLiteral( "ofl/gothica1/GothicA1-Thin.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-ExtraLight.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-Light.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-Regular.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-Medium.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-SemiBold.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-Bold.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-ExtraBold.ttf" ), QStringLiteral( "ofl/gothica1/GothicA1-Black.ttf" ) }, QStringLiteral( "ofl/gothica1/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gotu" ), { QStringLiteral( "ofl/gotu/Gotu-Regular.ttf" ) }, QStringLiteral( "ofl/gotu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Goudy Bookletter 1911" ), { QStringLiteral( "ofl/goudybookletter1911/GoudyBookletter1911.ttf" ) }, QStringLiteral( "ofl/goudybookletter1911/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gowun Batang" ), { QStringLiteral( "ofl/gowunbatang/GowunBatang-Regular.ttf" ), QStringLiteral( "ofl/gowunbatang/GowunBatang-Bold.ttf" ) }, QStringLiteral( "ofl/gowunbatang/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gowun Dodum" ), { QStringLiteral( "ofl/gowundodum/GowunDodum-Regular.ttf" ) }, QStringLiteral( "ofl/gowundodum/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Graduate" ), { QStringLiteral( "ofl/graduate/Graduate-Regular.ttf" ) }, QStringLiteral( "ofl/graduate/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grand Hotel" ), { QStringLiteral( "ofl/grandhotel/GrandHotel-Regular.ttf" ) }, QStringLiteral( "ofl/grandhotel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grandiflora One" ), { QStringLiteral( "ofl/grandifloraone/GrandifloraOne-Regular.ttf" ) }, QStringLiteral( "ofl/grandifloraone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grandstander" ), { QStringLiteral( "ofl/grandstander/Grandstander%5Bwght%5D.ttf" ), QStringLiteral( "ofl/grandstander/Grandstander-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/grandstander/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grape Nuts" ), { QStringLiteral( "ofl/grapenuts/GrapeNuts-Regular.ttf" ) }, QStringLiteral( "ofl/grapenuts/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gravitas One" ), { QStringLiteral( "ofl/gravitasone/GravitasOne.ttf" ) }, QStringLiteral( "ofl/gravitasone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Great Vibes" ), { QStringLiteral( "ofl/greatvibes/GreatVibes-Regular.ttf" ) }, QStringLiteral( "ofl/greatvibes/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grechen Fuemen" ), { QStringLiteral( "ofl/grechenfuemen/GrechenFuemen-Regular.ttf" ) }, QStringLiteral( "ofl/grechenfuemen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grenze" ), { QStringLiteral( "ofl/grenze/Grenze-Thin.ttf" ), QStringLiteral( "ofl/grenze/Grenze-ThinItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-ExtraLight.ttf" ), QStringLiteral( "ofl/grenze/Grenze-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-Light.ttf" ), QStringLiteral( "ofl/grenze/Grenze-LightItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-Regular.ttf" ), QStringLiteral( "ofl/grenze/Grenze-Italic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-Medium.ttf" ), QStringLiteral( "ofl/grenze/Grenze-MediumItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-SemiBold.ttf" ), QStringLiteral( "ofl/grenze/Grenze-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-Bold.ttf" ), QStringLiteral( "ofl/grenze/Grenze-BoldItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-ExtraBold.ttf" ), QStringLiteral( "ofl/grenze/Grenze-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/grenze/Grenze-Black.ttf" ), QStringLiteral( "ofl/grenze/Grenze-BlackItalic.ttf" ) }, QStringLiteral( "ofl/grenze/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grenze Gotisch" ), { QStringLiteral( "ofl/grenzegotisch/GrenzeGotisch%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/grenzegotisch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Grey Qo" ), { QStringLiteral( "ofl/greyqo/GreyQo-Regular.ttf" ) }, QStringLiteral( "ofl/greyqo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Griffy" ), { QStringLiteral( "ofl/griffy/Griffy-Regular.ttf" ) }, QStringLiteral( "ofl/griffy/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gruppo" ), { QStringLiteral( "ofl/gruppo/Gruppo-Regular.ttf" ) }, QStringLiteral( "ofl/gruppo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gudea" ), { QStringLiteral( "ofl/gudea/Gudea-Regular.ttf" ), QStringLiteral( "ofl/gudea/Gudea-Italic.ttf" ), QStringLiteral( "ofl/gudea/Gudea-Bold.ttf" ) }, QStringLiteral( "ofl/gudea/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gugi" ), { QStringLiteral( "ofl/gugi/Gugi-Regular.ttf" ) }, QStringLiteral( "ofl/gugi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gulzar" ), { QStringLiteral( "ofl/gulzar/Gulzar-Regular.ttf" ) }, QStringLiteral( "ofl/gulzar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gupter" ), { QStringLiteral( "ofl/gupter/Gupter-Regular.ttf" ), QStringLiteral( "ofl/gupter/Gupter-Medium.ttf" ), QStringLiteral( "ofl/gupter/Gupter-Bold.ttf" ) }, QStringLiteral( "ofl/gupter/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gurajada" ), { QStringLiteral( "ofl/gurajada/Gurajada-Regular.ttf" ) }, QStringLiteral( "ofl/gurajada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Gwendolyn" ), { QStringLiteral( "ofl/gwendolyn/Gwendolyn-Regular.ttf" ), QStringLiteral( "ofl/gwendolyn/Gwendolyn-Bold.ttf" ) }, QStringLiteral( "ofl/gwendolyn/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Habibi" ), { QStringLiteral( "ofl/habibi/Habibi-Regular.ttf" ) }, QStringLiteral( "ofl/habibi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hachi Maru Pop" ), { QStringLiteral( "ofl/hachimarupop/HachiMaruPop-Regular.ttf" ) }, QStringLiteral( "ofl/hachimarupop/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hahmlet" ), { QStringLiteral( "ofl/hahmlet/Hahmlet%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/hahmlet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Halant" ), { QStringLiteral( "ofl/halant/Halant-Light.ttf" ), QStringLiteral( "ofl/halant/Halant-Regular.ttf" ), QStringLiteral( "ofl/halant/Halant-Medium.ttf" ), QStringLiteral( "ofl/halant/Halant-SemiBold.ttf" ), QStringLiteral( "ofl/halant/Halant-Bold.ttf" ) }, QStringLiteral( "ofl/halant/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hammersmith One" ), { QStringLiteral( "ofl/hammersmithone/HammersmithOne-Regular.ttf" ) }, QStringLiteral( "ofl/hammersmithone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hanalei" ), { QStringLiteral( "ofl/hanalei/Hanalei-Regular.ttf" ) }, QStringLiteral( "ofl/hanalei/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hanalei Fill" ), { QStringLiteral( "ofl/hanaleifill/HanaleiFill-Regular.ttf" ) }, QStringLiteral( "ofl/hanaleifill/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Handjet" ), { QStringLiteral( "ofl/handjet/Handjet%5BELGR,ELSH,wght%5D.ttf" ) }, QStringLiteral( "ofl/handjet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Handlee" ), { QStringLiteral( "ofl/handlee/Handlee-Regular.ttf" ) }, QStringLiteral( "ofl/handlee/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hanken Grotesk" ), { QStringLiteral( "ofl/hankengrotesk/HankenGrotesk%5Bwght%5D.ttf" ), QStringLiteral( "ofl/hankengrotesk/HankenGrotesk-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/hankengrotesk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hanuman" ), { QStringLiteral( "ofl/hanuman/Hanuman-Thin.ttf" ), QStringLiteral( "ofl/hanuman/Hanuman-Light.ttf" ), QStringLiteral( "ofl/hanuman/Hanuman-Regular.ttf" ), QStringLiteral( "ofl/hanuman/Hanuman-Bold.ttf" ), QStringLiteral( "ofl/hanuman/Hanuman-Black.ttf" ) }, QStringLiteral( "ofl/hanuman/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Happy Monkey" ), { QStringLiteral( "ofl/happymonkey/HappyMonkey-Regular.ttf" ) }, QStringLiteral( "ofl/happymonkey/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Harmattan" ), { QStringLiteral( "ofl/harmattan/Harmattan-Regular.ttf" ), QStringLiteral( "ofl/harmattan/Harmattan-Medium.ttf" ), QStringLiteral( "ofl/harmattan/Harmattan-SemiBold.ttf" ), QStringLiteral( "ofl/harmattan/Harmattan-Bold.ttf" ) }, QStringLiteral( "ofl/harmattan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Headland One" ), { QStringLiteral( "ofl/headlandone/HeadlandOne-Regular.ttf" ) }, QStringLiteral( "ofl/headlandone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Heebo" ), { QStringLiteral( "ofl/heebo/Heebo%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/heebo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Henny Penny" ), { QStringLiteral( "ofl/hennypenny/HennyPenny-Regular.ttf" ) }, QStringLiteral( "ofl/hennypenny/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hepta Slab" ), { QStringLiteral( "ofl/heptaslab/HeptaSlab%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/heptaslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Herr Von Muellerhoff" ), { QStringLiteral( "ofl/herrvonmuellerhoff/HerrVonMuellerhoff-Regular.ttf" ) }, QStringLiteral( "ofl/herrvonmuellerhoff/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hi Melody" ), { QStringLiteral( "ofl/himelody/HiMelody-Regular.ttf" ) }, QStringLiteral( "ofl/himelody/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hina Mincho" ), { QStringLiteral( "ofl/hinamincho/HinaMincho-Regular.ttf" ) }, QStringLiteral( "ofl/hinamincho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hind" ), { QStringLiteral( "ofl/hind/Hind-Light.ttf" ), QStringLiteral( "ofl/hind/Hind-Regular.ttf" ), QStringLiteral( "ofl/hind/Hind-Medium.ttf" ), QStringLiteral( "ofl/hind/Hind-SemiBold.ttf" ), QStringLiteral( "ofl/hind/Hind-Bold.ttf" ) }, QStringLiteral( "ofl/hind/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hind Guntur" ), { QStringLiteral( "ofl/hindguntur/HindGuntur-Light.ttf" ), QStringLiteral( "ofl/hindguntur/HindGuntur-Regular.ttf" ), QStringLiteral( "ofl/hindguntur/HindGuntur-Medium.ttf" ), QStringLiteral( "ofl/hindguntur/HindGuntur-SemiBold.ttf" ), QStringLiteral( "ofl/hindguntur/HindGuntur-Bold.ttf" ) }, QStringLiteral( "ofl/hindguntur/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hind Madurai" ), { QStringLiteral( "ofl/hindmadurai/HindMadurai-Light.ttf" ), QStringLiteral( "ofl/hindmadurai/HindMadurai-Regular.ttf" ), QStringLiteral( "ofl/hindmadurai/HindMadurai-Medium.ttf" ), QStringLiteral( "ofl/hindmadurai/HindMadurai-SemiBold.ttf" ), QStringLiteral( "ofl/hindmadurai/HindMadurai-Bold.ttf" ) }, QStringLiteral( "ofl/hindmadurai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hind Siliguri" ), { QStringLiteral( "ofl/hindsiliguri/HindSiliguri-Light.ttf" ), QStringLiteral( "ofl/hindsiliguri/HindSiliguri-Regular.ttf" ), QStringLiteral( "ofl/hindsiliguri/HindSiliguri-Medium.ttf" ), QStringLiteral( "ofl/hindsiliguri/HindSiliguri-SemiBold.ttf" ), QStringLiteral( "ofl/hindsiliguri/HindSiliguri-Bold.ttf" ) }, QStringLiteral( "ofl/hindsiliguri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hind Vadodara" ), { QStringLiteral( "ofl/hindvadodara/HindVadodara-Light.ttf" ), QStringLiteral( "ofl/hindvadodara/HindVadodara-Regular.ttf" ), QStringLiteral( "ofl/hindvadodara/HindVadodara-Medium.ttf" ), QStringLiteral( "ofl/hindvadodara/HindVadodara-SemiBold.ttf" ), QStringLiteral( "ofl/hindvadodara/HindVadodara-Bold.ttf" ) }, QStringLiteral( "ofl/hindvadodara/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Holtwood One SC" ), { QStringLiteral( "ofl/holtwoodonesc/HoltwoodOneSC-Regular.ttf" ) }, QStringLiteral( "ofl/holtwoodonesc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Homemade Apple" ), { QStringLiteral( "apache/homemadeapple/HomemadeApple-Regular.ttf" ) }, QStringLiteral( "apache/homemadeapple/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Homenaje" ), { QStringLiteral( "ofl/homenaje/Homenaje-Regular.ttf" ) }, QStringLiteral( "ofl/homenaje/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hubballi" ), { QStringLiteral( "ofl/hubballi/Hubballi-Regular.ttf" ) }, QStringLiteral( "ofl/hubballi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Hurricane" ), { QStringLiteral( "ofl/hurricane/Hurricane-Regular.ttf" ) }, QStringLiteral( "ofl/hurricane/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Mono" ), { QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-Thin.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-ThinItalic.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-Light.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-LightItalic.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-Regular.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-Italic.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-Medium.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-MediumItalic.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-Bold.ttf" ), QStringLiteral( "ofl/ibmplexmono/IBMPlexMono-BoldItalic.ttf" ) }, QStringLiteral( "ofl/ibmplexmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans" ), { QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-ThinItalic.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-Light.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-LightItalic.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-Italic.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-MediumItalic.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-Bold.ttf" ), QStringLiteral( "ofl/ibmplexsans/IBMPlexSans-BoldItalic.ttf" ) }, QStringLiteral( "ofl/ibmplexsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans Arabic" ), { QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-Light.ttf" ), QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsansarabic/IBMPlexSansArabic-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsansarabic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans Condensed" ), { QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-ThinItalic.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Light.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-LightItalic.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Italic.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-MediumItalic.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Bold.ttf" ), QStringLiteral( "ofl/ibmplexsanscondensed/IBMPlexSansCondensed-BoldItalic.ttf" ) }, QStringLiteral( "ofl/ibmplexsanscondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans Devanagari" ), { QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Light.ttf" ), QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsansdevanagari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans Hebrew" ), { QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Light.ttf" ), QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsanshebrew/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans JP" ), { QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-Light.ttf" ), QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsansjp/IBMPlexSansJP-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsansjp/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans KR" ), { QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-Light.ttf" ), QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsanskr/IBMPlexSansKR-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsanskr/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans Thai" ), { QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-Light.ttf" ), QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsansthai/IBMPlexSansThai-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsansthai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Sans Thai Looped" ), { QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Thin.ttf" ), QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Light.ttf" ), QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Regular.ttf" ), QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Medium.ttf" ), QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Bold.ttf" ) }, QStringLiteral( "ofl/ibmplexsansthailooped/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IBM Plex Serif" ), { QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-Thin.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-ThinItalic.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-ExtraLight.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-Light.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-LightItalic.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-Regular.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-Italic.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-Medium.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-MediumItalic.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-SemiBold.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-Bold.ttf" ), QStringLiteral( "ofl/ibmplexserif/IBMPlexSerif-BoldItalic.ttf" ) }, QStringLiteral( "ofl/ibmplexserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell DW Pica" ), { QStringLiteral( "ofl/imfelldwpica/IMFePIrm28P.ttf" ), QStringLiteral( "ofl/imfelldwpica/IMFePIit28P.ttf" ) }, QStringLiteral( "ofl/imfelldwpica/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell DW Pica SC" ), { QStringLiteral( "ofl/imfelldwpicasc/IMFePIsc28P.ttf" ) }, QStringLiteral( "ofl/imfelldwpicasc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell Double Pica" ), { QStringLiteral( "ofl/imfelldoublepica/IMFELLDoublePica-Regular.ttf" ), QStringLiteral( "ofl/imfelldoublepica/IMFELLDoublePica-Italic.ttf" ) }, QStringLiteral( "ofl/imfelldoublepica/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell Double Pica SC" ), { QStringLiteral( "ofl/imfelldoublepicasc/IMFeDPsc28P.ttf" ) }, QStringLiteral( "ofl/imfelldoublepicasc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell English" ), { QStringLiteral( "ofl/imfellenglish/IMFeENrm28P.ttf" ), QStringLiteral( "ofl/imfellenglish/IMFeENit28P.ttf" ) }, QStringLiteral( "ofl/imfellenglish/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell English SC" ), { QStringLiteral( "ofl/imfellenglishsc/IMFeENsc28P.ttf" ) }, QStringLiteral( "ofl/imfellenglishsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell French Canon" ), { QStringLiteral( "ofl/imfellfrenchcanon/IMFeFCrm28P.ttf" ), QStringLiteral( "ofl/imfellfrenchcanon/IMFeFCit28P.ttf" ) }, QStringLiteral( "ofl/imfellfrenchcanon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell French Canon SC" ), { QStringLiteral( "ofl/imfellfrenchcanonsc/IMFeFCsc28P.ttf" ) }, QStringLiteral( "ofl/imfellfrenchcanonsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell Great Primer" ), { QStringLiteral( "ofl/imfellgreatprimer/IMFeGPrm28P.ttf" ), QStringLiteral( "ofl/imfellgreatprimer/IMFeGPit28P.ttf" ) }, QStringLiteral( "ofl/imfellgreatprimer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "IM Fell Great Primer SC" ), { QStringLiteral( "ofl/imfellgreatprimersc/IMFeGPsc28P.ttf" ) }, QStringLiteral( "ofl/imfellgreatprimersc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ibarra Real Nova" ), { QStringLiteral( "ofl/ibarrarealnova/IbarraRealNova%5Bwght%5D.ttf" ), QStringLiteral( "ofl/ibarrarealnova/IbarraRealNova-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ibarrarealnova/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Iceberg" ), { QStringLiteral( "ofl/iceberg/Iceberg-Regular.ttf" ) }, QStringLiteral( "ofl/iceberg/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Iceland" ), { QStringLiteral( "ofl/iceland/Iceland-Regular.ttf" ) }, QStringLiteral( "ofl/iceland/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Imbue" ), { QStringLiteral( "ofl/imbue/Imbue%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/imbue/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Imperial Script" ), { QStringLiteral( "ofl/imperialscript/ImperialScript-Regular.ttf" ) }, QStringLiteral( "ofl/imperialscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Imprima" ), { QStringLiteral( "ofl/imprima/Imprima-Regular.ttf" ) }, QStringLiteral( "ofl/imprima/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inconsolata" ), { QStringLiteral( "ofl/inconsolata/Inconsolata%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/inconsolata/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inder" ), { QStringLiteral( "ofl/inder/Inder-Regular.ttf" ) }, QStringLiteral( "ofl/inder/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Indie Flower" ), { QStringLiteral( "ofl/indieflower/IndieFlower-Regular.ttf" ) }, QStringLiteral( "ofl/indieflower/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ingrid Darling" ), { QStringLiteral( "ofl/ingriddarling/IngridDarling-Regular.ttf" ) }, QStringLiteral( "ofl/ingriddarling/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inika" ), { QStringLiteral( "ofl/inika/Inika-Regular.ttf" ), QStringLiteral( "ofl/inika/Inika-Bold.ttf" ) }, QStringLiteral( "ofl/inika/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inknut Antiqua" ), { QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-Light.ttf" ), QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-Regular.ttf" ), QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-Medium.ttf" ), QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-SemiBold.ttf" ), QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-Bold.ttf" ), QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-ExtraBold.ttf" ), QStringLiteral( "ofl/inknutantiqua/InknutAntiqua-Black.ttf" ) }, QStringLiteral( "ofl/inknutantiqua/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inria Sans" ), { QStringLiteral( "ofl/inriasans/InriaSans-Light.ttf" ), QStringLiteral( "ofl/inriasans/InriaSans-LightItalic.ttf" ), QStringLiteral( "ofl/inriasans/InriaSans-Regular.ttf" ), QStringLiteral( "ofl/inriasans/InriaSans-Italic.ttf" ), QStringLiteral( "ofl/inriasans/InriaSans-Bold.ttf" ), QStringLiteral( "ofl/inriasans/InriaSans-BoldItalic.ttf" ) }, QStringLiteral( "ofl/inriasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inria Serif" ), { QStringLiteral( "ofl/inriaserif/InriaSerif-Light.ttf" ), QStringLiteral( "ofl/inriaserif/InriaSerif-LightItalic.ttf" ), QStringLiteral( "ofl/inriaserif/InriaSerif-Regular.ttf" ), QStringLiteral( "ofl/inriaserif/InriaSerif-Italic.ttf" ), QStringLiteral( "ofl/inriaserif/InriaSerif-Bold.ttf" ), QStringLiteral( "ofl/inriaserif/InriaSerif-BoldItalic.ttf" ) }, QStringLiteral( "ofl/inriaserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inspiration" ), { QStringLiteral( "ofl/inspiration/Inspiration-Regular.ttf" ) }, QStringLiteral( "ofl/inspiration/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Instrument Sans" ), { QStringLiteral( "ofl/instrumentsans/InstrumentSans%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/instrumentsans/InstrumentSans-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/instrumentsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Instrument Serif" ), { QStringLiteral( "ofl/instrumentserif/InstrumentSerif-Regular.ttf" ), QStringLiteral( "ofl/instrumentserif/InstrumentSerif-Italic.ttf" ) }, QStringLiteral( "ofl/instrumentserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inter" ), { QStringLiteral( "ofl/inter/Inter%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/inter/Inter-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/inter/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Inter Tight" ), { QStringLiteral( "ofl/intertight/InterTight%5Bwght%5D.ttf" ), QStringLiteral( "ofl/intertight/InterTight-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/intertight/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Irish Grover" ), { QStringLiteral( "apache/irishgrover/IrishGrover-Regular.ttf" ) }, QStringLiteral( "apache/irishgrover/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Island Moments" ), { QStringLiteral( "ofl/islandmoments/IslandMoments-Regular.ttf" ) }, QStringLiteral( "ofl/islandmoments/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Istok Web" ), { QStringLiteral( "ofl/istokweb/IstokWeb-Regular.ttf" ), QStringLiteral( "ofl/istokweb/IstokWeb-Italic.ttf" ), QStringLiteral( "ofl/istokweb/IstokWeb-Bold.ttf" ), QStringLiteral( "ofl/istokweb/IstokWeb-BoldItalic.ttf" ) }, QStringLiteral( "ofl/istokweb/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Italiana" ), { QStringLiteral( "ofl/italiana/Italiana-Regular.ttf" ) }, QStringLiteral( "ofl/italiana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Italianno" ), { QStringLiteral( "ofl/italianno/Italianno-Regular.ttf" ) }, QStringLiteral( "ofl/italianno/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Itim" ), { QStringLiteral( "ofl/itim/Itim-Regular.ttf" ) }, QStringLiteral( "ofl/itim/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jacques Francois" ), { QStringLiteral( "ofl/jacquesfrancois/JacquesFrancois-Regular.ttf" ) }, QStringLiteral( "ofl/jacquesfrancois/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jacques Francois Shadow" ), { QStringLiteral( "ofl/jacquesfrancoisshadow/JacquesFrancoisShadow-Regular.ttf" ) }, QStringLiteral( "ofl/jacquesfrancoisshadow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jaldi" ), { QStringLiteral( "ofl/jaldi/Jaldi-Regular.ttf" ), QStringLiteral( "ofl/jaldi/Jaldi-Bold.ttf" ) }, QStringLiteral( "ofl/jaldi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "JetBrains Mono" ), { QStringLiteral( "ofl/jetbrainsmono/JetBrainsMono%5Bwght%5D.ttf" ), QStringLiteral( "ofl/jetbrainsmono/JetBrainsMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/jetbrainsmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jim Nightshade" ), { QStringLiteral( "ofl/jimnightshade/JimNightshade-Regular.ttf" ) }, QStringLiteral( "ofl/jimnightshade/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Joan" ), { QStringLiteral( "ofl/joan/Joan-Regular.ttf" ) }, QStringLiteral( "ofl/joan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jockey One" ), { QStringLiteral( "ofl/jockeyone/JockeyOne-Regular.ttf" ) }, QStringLiteral( "ofl/jockeyone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jolly Lodger" ), { QStringLiteral( "ofl/jollylodger/JollyLodger-Regular.ttf" ) }, QStringLiteral( "ofl/jollylodger/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jomhuria" ), { QStringLiteral( "ofl/jomhuria/Jomhuria-Regular.ttf" ) }, QStringLiteral( "ofl/jomhuria/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jomolhari" ), { QStringLiteral( "ofl/jomolhari/Jomolhari-Regular.ttf" ) }, QStringLiteral( "ofl/jomolhari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Josefin Sans" ), { QStringLiteral( "ofl/josefinsans/JosefinSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/josefinsans/JosefinSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/josefinsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Josefin Slab" ), { QStringLiteral( "ofl/josefinslab/JosefinSlab%5Bwght%5D.ttf" ), QStringLiteral( "ofl/josefinslab/JosefinSlab-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/josefinslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jost" ), { QStringLiteral( "ofl/jost/Jost%5Bwght%5D.ttf" ), QStringLiteral( "ofl/jost/Jost-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/jost/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Joti One" ), { QStringLiteral( "ofl/jotione/JotiOne-Regular.ttf" ) }, QStringLiteral( "ofl/jotione/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jua" ), { QStringLiteral( "ofl/jua/Jua-Regular.ttf" ) }, QStringLiteral( "ofl/jua/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Judson" ), { QStringLiteral( "ofl/judson/Judson-Regular.ttf" ), QStringLiteral( "ofl/judson/Judson-Italic.ttf" ), QStringLiteral( "ofl/judson/Judson-Bold.ttf" ) }, QStringLiteral( "ofl/judson/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Julee" ), { QStringLiteral( "ofl/julee/Julee-Regular.ttf" ) }, QStringLiteral( "ofl/julee/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Julius Sans One" ), { QStringLiteral( "ofl/juliussansone/JuliusSansOne-Regular.ttf" ) }, QStringLiteral( "ofl/juliussansone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Junge" ), { QStringLiteral( "ofl/junge/Junge-Regular.ttf" ) }, QStringLiteral( "ofl/junge/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Jura" ), { QStringLiteral( "ofl/jura/Jura%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/jura/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Just Another Hand" ), { QStringLiteral( "apache/justanotherhand/JustAnotherHand-Regular.ttf" ) }, QStringLiteral( "apache/justanotherhand/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Just Me Again Down Here" ), { QStringLiteral( "ofl/justmeagaindownhere/JustMeAgainDownHere.ttf" ) }, QStringLiteral( "ofl/justmeagaindownhere/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "K2D" ), { QStringLiteral( "ofl/k2d/K2D-Thin.ttf" ), QStringLiteral( "ofl/k2d/K2D-ThinItalic.ttf" ), QStringLiteral( "ofl/k2d/K2D-ExtraLight.ttf" ), QStringLiteral( "ofl/k2d/K2D-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/k2d/K2D-Light.ttf" ), QStringLiteral( "ofl/k2d/K2D-LightItalic.ttf" ), QStringLiteral( "ofl/k2d/K2D-Regular.ttf" ), QStringLiteral( "ofl/k2d/K2D-Italic.ttf" ), QStringLiteral( "ofl/k2d/K2D-Medium.ttf" ), QStringLiteral( "ofl/k2d/K2D-MediumItalic.ttf" ), QStringLiteral( "ofl/k2d/K2D-SemiBold.ttf" ), QStringLiteral( "ofl/k2d/K2D-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/k2d/K2D-Bold.ttf" ), QStringLiteral( "ofl/k2d/K2D-BoldItalic.ttf" ), QStringLiteral( "ofl/k2d/K2D-ExtraBold.ttf" ), QStringLiteral( "ofl/k2d/K2D-ExtraBoldItalic.ttf" ) }, QStringLiteral( "ofl/k2d/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kablammo" ), { QStringLiteral( "ofl/kablammo/Kablammo%5BMORF%5D.ttf" ) }, QStringLiteral( "ofl/kablammo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kadwa" ), { QStringLiteral( "ofl/kadwa/Kadwa-Regular.ttf" ), QStringLiteral( "ofl/kadwa/Kadwa-Bold.ttf" ) }, QStringLiteral( "ofl/kadwa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kaisei Decol" ), { QStringLiteral( "ofl/kaiseidecol/KaiseiDecol-Regular.ttf" ), QStringLiteral( "ofl/kaiseidecol/KaiseiDecol-Medium.ttf" ), QStringLiteral( "ofl/kaiseidecol/KaiseiDecol-Bold.ttf" ) }, QStringLiteral( "ofl/kaiseidecol/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kaisei HarunoUmi" ), { QStringLiteral( "ofl/kaiseiharunoumi/KaiseiHarunoUmi-Regular.ttf" ), QStringLiteral( "ofl/kaiseiharunoumi/KaiseiHarunoUmi-Medium.ttf" ), QStringLiteral( "ofl/kaiseiharunoumi/KaiseiHarunoUmi-Bold.ttf" ) }, QStringLiteral( "ofl/kaiseiharunoumi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kaisei Opti" ), { QStringLiteral( "ofl/kaiseiopti/KaiseiOpti-Regular.ttf" ), QStringLiteral( "ofl/kaiseiopti/KaiseiOpti-Medium.ttf" ), QStringLiteral( "ofl/kaiseiopti/KaiseiOpti-Bold.ttf" ) }, QStringLiteral( "ofl/kaiseiopti/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kaisei Tokumin" ), { QStringLiteral( "ofl/kaiseitokumin/KaiseiTokumin-Regular.ttf" ), QStringLiteral( "ofl/kaiseitokumin/KaiseiTokumin-Medium.ttf" ), QStringLiteral( "ofl/kaiseitokumin/KaiseiTokumin-Bold.ttf" ), QStringLiteral( "ofl/kaiseitokumin/KaiseiTokumin-ExtraBold.ttf" ) }, QStringLiteral( "ofl/kaiseitokumin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kalam" ), { QStringLiteral( "ofl/kalam/Kalam-Light.ttf" ), QStringLiteral( "ofl/kalam/Kalam-Regular.ttf" ), QStringLiteral( "ofl/kalam/Kalam-Bold.ttf" ) }, QStringLiteral( "ofl/kalam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kameron" ), { QStringLiteral( "ofl/kameron/Kameron%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/kameron/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kanit" ), { QStringLiteral( "ofl/kanit/Kanit-Thin.ttf" ), QStringLiteral( "ofl/kanit/Kanit-ThinItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-ExtraLight.ttf" ), QStringLiteral( "ofl/kanit/Kanit-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-Light.ttf" ), QStringLiteral( "ofl/kanit/Kanit-LightItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-Regular.ttf" ), QStringLiteral( "ofl/kanit/Kanit-Italic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-Medium.ttf" ), QStringLiteral( "ofl/kanit/Kanit-MediumItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-SemiBold.ttf" ), QStringLiteral( "ofl/kanit/Kanit-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-Bold.ttf" ), QStringLiteral( "ofl/kanit/Kanit-BoldItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-ExtraBold.ttf" ), QStringLiteral( "ofl/kanit/Kanit-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/kanit/Kanit-Black.ttf" ), QStringLiteral( "ofl/kanit/Kanit-BlackItalic.ttf" ) }, QStringLiteral( "ofl/kanit/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kantumruy Pro" ), { QStringLiteral( "ofl/kantumruypro/KantumruyPro%5Bwght%5D.ttf" ), QStringLiteral( "ofl/kantumruypro/KantumruyPro-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/kantumruypro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Karantina" ), { QStringLiteral( "ofl/karantina/Karantina-Light.ttf" ), QStringLiteral( "ofl/karantina/Karantina-Regular.ttf" ), QStringLiteral( "ofl/karantina/Karantina-Bold.ttf" ) }, QStringLiteral( "ofl/karantina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Karla" ), { QStringLiteral( "ofl/karla/Karla%5Bwght%5D.ttf" ), QStringLiteral( "ofl/karla/Karla-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/karla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Karma" ), { QStringLiteral( "ofl/karma/Karma-Light.ttf" ), QStringLiteral( "ofl/karma/Karma-Regular.ttf" ), QStringLiteral( "ofl/karma/Karma-Medium.ttf" ), QStringLiteral( "ofl/karma/Karma-SemiBold.ttf" ), QStringLiteral( "ofl/karma/Karma-Bold.ttf" ) }, QStringLiteral( "ofl/karma/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Katibeh" ), { QStringLiteral( "ofl/katibeh/Katibeh-Regular.ttf" ) }, QStringLiteral( "ofl/katibeh/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kaushan Script" ), { QStringLiteral( "ofl/kaushanscript/KaushanScript-Regular.ttf" ) }, QStringLiteral( "ofl/kaushanscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kavivanar" ), { QStringLiteral( "ofl/kavivanar/Kavivanar-Regular.ttf" ) }, QStringLiteral( "ofl/kavivanar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kavoon" ), { QStringLiteral( "ofl/kavoon/Kavoon-Regular.ttf" ) }, QStringLiteral( "ofl/kavoon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kdam Thmor Pro" ), { QStringLiteral( "ofl/kdamthmorpro/KdamThmorPro-Regular.ttf" ) }, QStringLiteral( "ofl/kdamthmorpro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Keania One" ), { QStringLiteral( "ofl/keaniaone/KeaniaOne-Regular.ttf" ) }, QStringLiteral( "ofl/keaniaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kelly Slab" ), { QStringLiteral( "ofl/kellyslab/KellySlab-Regular.ttf" ) }, QStringLiteral( "ofl/kellyslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kenia" ), { QStringLiteral( "ofl/kenia/Kenia-Regular.ttf" ) }, QStringLiteral( "ofl/kenia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Khand" ), { QStringLiteral( "ofl/khand/Khand-Light.ttf" ), QStringLiteral( "ofl/khand/Khand-Regular.ttf" ), QStringLiteral( "ofl/khand/Khand-Medium.ttf" ), QStringLiteral( "ofl/khand/Khand-SemiBold.ttf" ), QStringLiteral( "ofl/khand/Khand-Bold.ttf" ) }, QStringLiteral( "ofl/khand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Khmer" ), { QStringLiteral( "ofl/khmer/Khmer.ttf" ) }, QStringLiteral( "ofl/khmer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Khula" ), { QStringLiteral( "ofl/khula/Khula-Light.ttf" ), QStringLiteral( "ofl/khula/Khula-Regular.ttf" ), QStringLiteral( "ofl/khula/Khula-SemiBold.ttf" ), QStringLiteral( "ofl/khula/Khula-Bold.ttf" ), QStringLiteral( "ofl/khula/Khula-ExtraBold.ttf" ) }, QStringLiteral( "ofl/khula/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kings" ), { QStringLiteral( "ofl/kings/Kings-Regular.ttf" ) }, QStringLiteral( "ofl/kings/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kirang Haerang" ), { QStringLiteral( "ofl/kiranghaerang/KirangHaerang-Regular.ttf" ) }, QStringLiteral( "ofl/kiranghaerang/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kite One" ), { QStringLiteral( "ofl/kiteone/KiteOne-Regular.ttf" ) }, QStringLiteral( "ofl/kiteone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kiwi Maru" ), { QStringLiteral( "ofl/kiwimaru/KiwiMaru-Light.ttf" ), QStringLiteral( "ofl/kiwimaru/KiwiMaru-Regular.ttf" ), QStringLiteral( "ofl/kiwimaru/KiwiMaru-Medium.ttf" ) }, QStringLiteral( "ofl/kiwimaru/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Klee One" ), { QStringLiteral( "ofl/kleeone/KleeOne-Regular.ttf" ), QStringLiteral( "ofl/kleeone/KleeOne-SemiBold.ttf" ) }, QStringLiteral( "ofl/kleeone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Knewave" ), { QStringLiteral( "ofl/knewave/Knewave-Regular.ttf" ) }, QStringLiteral( "ofl/knewave/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "KoHo" ), { QStringLiteral( "ofl/koho/KoHo-ExtraLight.ttf" ), QStringLiteral( "ofl/koho/KoHo-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/koho/KoHo-Light.ttf" ), QStringLiteral( "ofl/koho/KoHo-LightItalic.ttf" ), QStringLiteral( "ofl/koho/KoHo-Regular.ttf" ), QStringLiteral( "ofl/koho/KoHo-Italic.ttf" ), QStringLiteral( "ofl/koho/KoHo-Medium.ttf" ), QStringLiteral( "ofl/koho/KoHo-MediumItalic.ttf" ), QStringLiteral( "ofl/koho/KoHo-SemiBold.ttf" ), QStringLiteral( "ofl/koho/KoHo-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/koho/KoHo-Bold.ttf" ), QStringLiteral( "ofl/koho/KoHo-BoldItalic.ttf" ) }, QStringLiteral( "ofl/koho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kodchasan" ), { QStringLiteral( "ofl/kodchasan/Kodchasan-ExtraLight.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-Light.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-LightItalic.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-Regular.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-Italic.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-Medium.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-MediumItalic.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-SemiBold.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-Bold.ttf" ), QStringLiteral( "ofl/kodchasan/Kodchasan-BoldItalic.ttf" ) }, QStringLiteral( "ofl/kodchasan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Koh Santepheap" ), { QStringLiteral( "ofl/kohsantepheap/KohSantepheap-Thin.ttf" ), QStringLiteral( "ofl/kohsantepheap/KohSantepheap-Light.ttf" ), QStringLiteral( "ofl/kohsantepheap/KohSantepheap-Regular.ttf" ), QStringLiteral( "ofl/kohsantepheap/KohSantepheap-Bold.ttf" ), QStringLiteral( "ofl/kohsantepheap/KohSantepheap-Black.ttf" ) }, QStringLiteral( "ofl/kohsantepheap/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kolker Brush" ), { QStringLiteral( "ofl/kolkerbrush/KolkerBrush-Regular.ttf" ) }, QStringLiteral( "ofl/kolkerbrush/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Konkhmer Sleokchher" ), { QStringLiteral( "ofl/konkhmersleokchher/KonkhmerSleokchher-Regular.ttf" ) }, QStringLiteral( "ofl/konkhmersleokchher/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kosugi" ), { QStringLiteral( "apache/kosugi/Kosugi-Regular.ttf" ) }, QStringLiteral( "apache/kosugi/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kosugi Maru" ), { QStringLiteral( "apache/kosugimaru/KosugiMaru-Regular.ttf" ) }, QStringLiteral( "apache/kosugimaru/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kotta One" ), { QStringLiteral( "ofl/kottaone/KottaOne-Regular.ttf" ) }, QStringLiteral( "ofl/kottaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Koulen" ), { QStringLiteral( "ofl/koulen/Koulen-Regular.ttf" ) }, QStringLiteral( "ofl/koulen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kranky" ), { QStringLiteral( "apache/kranky/Kranky-Regular.ttf" ) }, QStringLiteral( "apache/kranky/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kreon" ), { QStringLiteral( "ofl/kreon/Kreon%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/kreon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kristi" ), { QStringLiteral( "ofl/kristi/Kristi-Regular.ttf" ) }, QStringLiteral( "ofl/kristi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Krona One" ), { QStringLiteral( "ofl/kronaone/KronaOne-Regular.ttf" ) }, QStringLiteral( "ofl/kronaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Krub" ), { QStringLiteral( "ofl/krub/Krub-ExtraLight.ttf" ), QStringLiteral( "ofl/krub/Krub-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/krub/Krub-Light.ttf" ), QStringLiteral( "ofl/krub/Krub-LightItalic.ttf" ), QStringLiteral( "ofl/krub/Krub-Regular.ttf" ), QStringLiteral( "ofl/krub/Krub-Italic.ttf" ), QStringLiteral( "ofl/krub/Krub-Medium.ttf" ), QStringLiteral( "ofl/krub/Krub-MediumItalic.ttf" ), QStringLiteral( "ofl/krub/Krub-SemiBold.ttf" ), QStringLiteral( "ofl/krub/Krub-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/krub/Krub-Bold.ttf" ), QStringLiteral( "ofl/krub/Krub-BoldItalic.ttf" ) }, QStringLiteral( "ofl/krub/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kufam" ), { QStringLiteral( "ofl/kufam/Kufam%5Bwght%5D.ttf" ), QStringLiteral( "ofl/kufam/Kufam-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/kufam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kumar One" ), { QStringLiteral( "ofl/kumarone/KumarOne-Regular.ttf" ) }, QStringLiteral( "ofl/kumarone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kumbh Sans" ), { QStringLiteral( "ofl/kumbhsans/KumbhSans%5BYOPQ,wght%5D.ttf" ) }, QStringLiteral( "ofl/kumbhsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Kurale" ), { QStringLiteral( "ofl/kurale/Kurale-Regular.ttf" ) }, QStringLiteral( "ofl/kurale/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "La Belle Aurore" ), { QStringLiteral( "ofl/labelleaurore/LaBelleAurore.ttf" ) }, QStringLiteral( "ofl/labelleaurore/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Labrada" ), { QStringLiteral( "ofl/labrada/Labrada%5Bwght%5D.ttf" ), QStringLiteral( "ofl/labrada/Labrada-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/labrada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lacquer" ), { QStringLiteral( "ofl/lacquer/Lacquer-Regular.ttf" ) }, QStringLiteral( "ofl/lacquer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Laila" ), { QStringLiteral( "ofl/laila/Laila-Light.ttf" ), QStringLiteral( "ofl/laila/Laila-Regular.ttf" ), QStringLiteral( "ofl/laila/Laila-Medium.ttf" ), QStringLiteral( "ofl/laila/Laila-SemiBold.ttf" ), QStringLiteral( "ofl/laila/Laila-Bold.ttf" ) }, QStringLiteral( "ofl/laila/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lakki Reddy" ), { QStringLiteral( "ofl/lakkireddy/LakkiReddy-Regular.ttf" ) }, QStringLiteral( "ofl/lakkireddy/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lalezar" ), { QStringLiteral( "ofl/lalezar/Lalezar-Regular.ttf" ) }, QStringLiteral( "ofl/lalezar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lancelot" ), { QStringLiteral( "ofl/lancelot/Lancelot-Regular.ttf" ) }, QStringLiteral( "ofl/lancelot/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Langar" ), { QStringLiteral( "ofl/langar/Langar-Regular.ttf" ) }, QStringLiteral( "ofl/langar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lateef" ), { QStringLiteral( "ofl/lateef/Lateef-ExtraLight.ttf" ), QStringLiteral( "ofl/lateef/Lateef-Light.ttf" ), QStringLiteral( "ofl/lateef/Lateef-Regular.ttf" ), QStringLiteral( "ofl/lateef/Lateef-Medium.ttf" ), QStringLiteral( "ofl/lateef/Lateef-SemiBold.ttf" ), QStringLiteral( "ofl/lateef/Lateef-Bold.ttf" ), QStringLiteral( "ofl/lateef/Lateef-ExtraBold.ttf" ) }, QStringLiteral( "ofl/lateef/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lato" ), { QStringLiteral( "ofl/lato/Lato-Thin.ttf" ), QStringLiteral( "ofl/lato/Lato-ThinItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-ExtraLight.ttf" ), QStringLiteral( "ofl/lato/Lato-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-Light.ttf" ), QStringLiteral( "ofl/lato/Lato-LightItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-Regular.ttf" ), QStringLiteral( "ofl/lato/Lato-Italic.ttf" ), QStringLiteral( "ofl/lato/Lato-Medium.ttf" ), QStringLiteral( "ofl/lato/Lato-MediumItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-SemiBold.ttf" ), QStringLiteral( "ofl/lato/Lato-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-Bold.ttf" ), QStringLiteral( "ofl/lato/Lato-BoldItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-ExtraBold.ttf" ), QStringLiteral( "ofl/lato/Lato-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/lato/Lato-Black.ttf" ), QStringLiteral( "ofl/lato/Lato-BlackItalic.ttf" ) }, QStringLiteral( "ofl/lato/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lavishly Yours" ), { QStringLiteral( "ofl/lavishlyyours/LavishlyYours-Regular.ttf" ) }, QStringLiteral( "ofl/lavishlyyours/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "League Gothic" ), { QStringLiteral( "ofl/leaguegothic/LeagueGothic%5Bwdth%5D.ttf" ) }, QStringLiteral( "ofl/leaguegothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "League Script" ), { QStringLiteral( "ofl/leaguescript/LeagueScript-Regular.ttf" ) }, QStringLiteral( "ofl/leaguescript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "League Spartan" ), { QStringLiteral( "ofl/leaguespartan/LeagueSpartan%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/leaguespartan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Leckerli One" ), { QStringLiteral( "ofl/leckerlione/LeckerliOne-Regular.ttf" ) }, QStringLiteral( "ofl/leckerlione/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ledger" ), { QStringLiteral( "ofl/ledger/Ledger-Regular.ttf" ) }, QStringLiteral( "ofl/ledger/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lekton" ), { QStringLiteral( "ofl/lekton/Lekton-Regular.ttf" ), QStringLiteral( "ofl/lekton/Lekton-Italic.ttf" ), QStringLiteral( "ofl/lekton/Lekton-Bold.ttf" ) }, QStringLiteral( "ofl/lekton/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lemon" ), { QStringLiteral( "ofl/lemon/Lemon-Regular.ttf" ) }, QStringLiteral( "ofl/lemon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lemonada" ), { QStringLiteral( "ofl/lemonada/Lemonada%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lemonada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend" ), { QStringLiteral( "ofl/lexend/Lexend%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexend/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Deca" ), { QStringLiteral( "ofl/lexenddeca/LexendDeca%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexenddeca/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Exa" ), { QStringLiteral( "ofl/lexendexa/LexendExa%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexendexa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Giga" ), { QStringLiteral( "ofl/lexendgiga/LexendGiga%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexendgiga/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Mega" ), { QStringLiteral( "ofl/lexendmega/LexendMega%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexendmega/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Peta" ), { QStringLiteral( "ofl/lexendpeta/LexendPeta%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexendpeta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Tera" ), { QStringLiteral( "ofl/lexendtera/LexendTera%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexendtera/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lexend Zetta" ), { QStringLiteral( "ofl/lexendzetta/LexendZetta%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lexendzetta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode 128" ), { QStringLiteral( "ofl/librebarcode128/LibreBarcode128-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcode128/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode 128 Text" ), { QStringLiteral( "ofl/librebarcode128text/LibreBarcode128Text-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcode128text/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode 39" ), { QStringLiteral( "ofl/librebarcode39/LibreBarcode39-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcode39/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode 39 Extended" ), { QStringLiteral( "ofl/librebarcode39extended/LibreBarcode39Extended-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcode39extended/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode 39 Extended Text" ), { QStringLiteral( "ofl/librebarcode39extendedtext/LibreBarcode39ExtendedText-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcode39extendedtext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode 39 Text" ), { QStringLiteral( "ofl/librebarcode39text/LibreBarcode39Text-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcode39text/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Barcode EAN13 Text" ), { QStringLiteral( "ofl/librebarcodeean13text/LibreBarcodeEAN13Text-Regular.ttf" ) }, QStringLiteral( "ofl/librebarcodeean13text/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Baskerville" ), { QStringLiteral( "ofl/librebaskerville/LibreBaskerville-Regular.ttf" ), QStringLiteral( "ofl/librebaskerville/LibreBaskerville-Italic.ttf" ), QStringLiteral( "ofl/librebaskerville/LibreBaskerville-Bold.ttf" ) }, QStringLiteral( "ofl/librebaskerville/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Bodoni" ), { QStringLiteral( "ofl/librebodoni/LibreBodoni%5Bwght%5D.ttf" ), QStringLiteral( "ofl/librebodoni/LibreBodoni-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/librebodoni/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Caslon Display" ), { QStringLiteral( "ofl/librecaslondisplay/LibreCaslonDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/librecaslondisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Caslon Text" ), { QStringLiteral( "ofl/librecaslontext/LibreCaslonText%5Bwght%5D.ttf" ), QStringLiteral( "ofl/librecaslontext/LibreCaslonText-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/librecaslontext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Libre Franklin" ), { QStringLiteral( "ofl/librefranklin/LibreFranklin%5Bwght%5D.ttf" ), QStringLiteral( "ofl/librefranklin/LibreFranklin-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/librefranklin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Licorice" ), { QStringLiteral( "ofl/licorice/Licorice-Regular.ttf" ) }, QStringLiteral( "ofl/licorice/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Life Savers" ), { QStringLiteral( "ofl/lifesavers/LifeSavers-Regular.ttf" ), QStringLiteral( "ofl/lifesavers/LifeSavers-Bold.ttf" ), QStringLiteral( "ofl/lifesavers/LifeSavers-ExtraBold.ttf" ) }, QStringLiteral( "ofl/lifesavers/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lilita One" ), { QStringLiteral( "ofl/lilitaone/LilitaOne-Regular.ttf" ) }, QStringLiteral( "ofl/lilitaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lily Script One" ), { QStringLiteral( "ofl/lilyscriptone/LilyScriptOne-Regular.ttf" ) }, QStringLiteral( "ofl/lilyscriptone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Limelight" ), { QStringLiteral( "ofl/limelight/Limelight-Regular.ttf" ) }, QStringLiteral( "ofl/limelight/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Linden Hill" ), { QStringLiteral( "ofl/lindenhill/LindenHill-Regular.ttf" ), QStringLiteral( "ofl/lindenhill/LindenHill-Italic.ttf" ) }, QStringLiteral( "ofl/lindenhill/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lisu Bosa" ), { QStringLiteral( "ofl/lisubosa/LisuBosa-ExtraLight.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-Light.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-LightItalic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-Regular.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-Italic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-Medium.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-MediumItalic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-SemiBold.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-Bold.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-BoldItalic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-ExtraBold.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-Black.ttf" ), QStringLiteral( "ofl/lisubosa/LisuBosa-BlackItalic.ttf" ) }, QStringLiteral( "ofl/lisubosa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Literata" ), { QStringLiteral( "ofl/literata/Literata%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/literata/Literata-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/literata/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Liu Jian Mao Cao" ), { QStringLiteral( "ofl/liujianmaocao/LiuJianMaoCao-Regular.ttf" ) }, QStringLiteral( "ofl/liujianmaocao/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Livvic" ), { QStringLiteral( "ofl/livvic/Livvic-Thin.ttf" ), QStringLiteral( "ofl/livvic/Livvic-ThinItalic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-ExtraLight.ttf" ), QStringLiteral( "ofl/livvic/Livvic-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-Light.ttf" ), QStringLiteral( "ofl/livvic/Livvic-LightItalic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-Regular.ttf" ), QStringLiteral( "ofl/livvic/Livvic-Italic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-Medium.ttf" ), QStringLiteral( "ofl/livvic/Livvic-MediumItalic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-SemiBold.ttf" ), QStringLiteral( "ofl/livvic/Livvic-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-Bold.ttf" ), QStringLiteral( "ofl/livvic/Livvic-BoldItalic.ttf" ), QStringLiteral( "ofl/livvic/Livvic-Black.ttf" ), QStringLiteral( "ofl/livvic/Livvic-BlackItalic.ttf" ) }, QStringLiteral( "ofl/livvic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lobster" ), { QStringLiteral( "ofl/lobster/Lobster-Regular.ttf" ) }, QStringLiteral( "ofl/lobster/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lobster Two" ), { QStringLiteral( "ofl/lobstertwo/LobsterTwo-Regular.ttf" ), QStringLiteral( "ofl/lobstertwo/LobsterTwo-Italic.ttf" ), QStringLiteral( "ofl/lobstertwo/LobsterTwo-Bold.ttf" ), QStringLiteral( "ofl/lobstertwo/LobsterTwo-BoldItalic.ttf" ) }, QStringLiteral( "ofl/lobstertwo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Londrina Outline" ), { QStringLiteral( "ofl/londrinaoutline/LondrinaOutline-Regular.ttf" ) }, QStringLiteral( "ofl/londrinaoutline/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Londrina Shadow" ), { QStringLiteral( "ofl/londrinashadow/LondrinaShadow-Regular.ttf" ) }, QStringLiteral( "ofl/londrinashadow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Londrina Sketch" ), { QStringLiteral( "ofl/londrinasketch/LondrinaSketch-Regular.ttf" ) }, QStringLiteral( "ofl/londrinasketch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Londrina Solid" ), { QStringLiteral( "ofl/londrinasolid/LondrinaSolid-Thin.ttf" ), QStringLiteral( "ofl/londrinasolid/LondrinaSolid-Light.ttf" ), QStringLiteral( "ofl/londrinasolid/LondrinaSolid-Regular.ttf" ), QStringLiteral( "ofl/londrinasolid/LondrinaSolid-Black.ttf" ) }, QStringLiteral( "ofl/londrinasolid/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Long Cang" ), { QStringLiteral( "ofl/longcang/LongCang-Regular.ttf" ) }, QStringLiteral( "ofl/longcang/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lora" ), { QStringLiteral( "ofl/lora/Lora%5Bwght%5D.ttf" ), QStringLiteral( "ofl/lora/Lora-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/lora/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Love Light" ), { QStringLiteral( "ofl/lovelight/LoveLight-Regular.ttf" ) }, QStringLiteral( "ofl/lovelight/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Love Ya Like A Sister" ), { QStringLiteral( "ofl/loveyalikeasister/LoveYaLikeASister.ttf" ) }, QStringLiteral( "ofl/loveyalikeasister/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Loved by the King" ), { QStringLiteral( "ofl/lovedbytheking/LovedbytheKing.ttf" ) }, QStringLiteral( "ofl/lovedbytheking/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lovers Quarrel" ), { QStringLiteral( "ofl/loversquarrel/LoversQuarrel-Regular.ttf" ) }, QStringLiteral( "ofl/loversquarrel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Luckiest Guy" ), { QStringLiteral( "apache/luckiestguy/LuckiestGuy-Regular.ttf" ) }, QStringLiteral( "apache/luckiestguy/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lugrasimo" ), { QStringLiteral( "ofl/lugrasimo/Lugrasimo-Regular.ttf" ) }, QStringLiteral( "ofl/lugrasimo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lumanosimo" ), { QStringLiteral( "ofl/lumanosimo/Lumanosimo-Regular.ttf" ) }, QStringLiteral( "ofl/lumanosimo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lunasima" ), { QStringLiteral( "ofl/lunasima/Lunasima-Regular.ttf" ), QStringLiteral( "ofl/lunasima/Lunasima-Bold.ttf" ) }, QStringLiteral( "ofl/lunasima/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lusitana" ), { QStringLiteral( "ofl/lusitana/Lusitana-Regular.ttf" ), QStringLiteral( "ofl/lusitana/Lusitana-Bold.ttf" ) }, QStringLiteral( "ofl/lusitana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Lustria" ), { QStringLiteral( "ofl/lustria/Lustria-Regular.ttf" ) }, QStringLiteral( "ofl/lustria/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Luxurious Roman" ), { QStringLiteral( "ofl/luxuriousroman/LuxuriousRoman-Regular.ttf" ) }, QStringLiteral( "ofl/luxuriousroman/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Luxurious Script" ), { QStringLiteral( "ofl/luxuriousscript/LuxuriousScript-Regular.ttf" ) }, QStringLiteral( "ofl/luxuriousscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "M PLUS 1" ), { QStringLiteral( "ofl/mplus1/MPLUS1%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mplus1/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "M PLUS 1 Code" ), { QStringLiteral( "ofl/mplus1code/MPLUS1Code%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mplus1code/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "M PLUS 1p" ), { QStringLiteral( "ofl/mplus1p/MPLUS1p-Thin.ttf" ), QStringLiteral( "ofl/mplus1p/MPLUS1p-Light.ttf" ), QStringLiteral( "ofl/mplus1p/MPLUS1p-Regular.ttf" ), QStringLiteral( "ofl/mplus1p/MPLUS1p-Medium.ttf" ), QStringLiteral( "ofl/mplus1p/MPLUS1p-Bold.ttf" ), QStringLiteral( "ofl/mplus1p/MPLUS1p-ExtraBold.ttf" ), QStringLiteral( "ofl/mplus1p/MPLUS1p-Black.ttf" ) }, QStringLiteral( "ofl/mplus1p/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "M PLUS 2" ), { QStringLiteral( "ofl/mplus2/MPLUS2%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mplus2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "M PLUS Code Latin" ), { QStringLiteral( "ofl/mpluscodelatin/MPLUSCodeLatin%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/mpluscodelatin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ma Shan Zheng" ), { QStringLiteral( "ofl/mashanzheng/MaShanZheng-Regular.ttf" ) }, QStringLiteral( "ofl/mashanzheng/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Macondo" ), { QStringLiteral( "ofl/macondo/Macondo-Regular.ttf" ) }, QStringLiteral( "ofl/macondo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Macondo Swash Caps" ), { QStringLiteral( "ofl/macondoswashcaps/MacondoSwashCaps-Regular.ttf" ) }, QStringLiteral( "ofl/macondoswashcaps/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mada" ), { QStringLiteral( "ofl/mada/Mada%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Magra" ), { QStringLiteral( "ofl/magra/Magra-Regular.ttf" ), QStringLiteral( "ofl/magra/Magra-Bold.ttf" ) }, QStringLiteral( "ofl/magra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Maiden Orange" ), { QStringLiteral( "apache/maidenorange/MaidenOrange-Regular.ttf" ) }, QStringLiteral( "apache/maidenorange/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Maitree" ), { QStringLiteral( "ofl/maitree/Maitree-ExtraLight.ttf" ), QStringLiteral( "ofl/maitree/Maitree-Light.ttf" ), QStringLiteral( "ofl/maitree/Maitree-Regular.ttf" ), QStringLiteral( "ofl/maitree/Maitree-Medium.ttf" ), QStringLiteral( "ofl/maitree/Maitree-SemiBold.ttf" ), QStringLiteral( "ofl/maitree/Maitree-Bold.ttf" ) }, QStringLiteral( "ofl/maitree/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Major Mono Display" ), { QStringLiteral( "ofl/majormonodisplay/MajorMonoDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/majormonodisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mako" ), { QStringLiteral( "ofl/mako/Mako-Regular.ttf" ) }, QStringLiteral( "ofl/mako/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mali" ), { QStringLiteral( "ofl/mali/Mali-ExtraLight.ttf" ), QStringLiteral( "ofl/mali/Mali-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/mali/Mali-Light.ttf" ), QStringLiteral( "ofl/mali/Mali-LightItalic.ttf" ), QStringLiteral( "ofl/mali/Mali-Regular.ttf" ), QStringLiteral( "ofl/mali/Mali-Italic.ttf" ), QStringLiteral( "ofl/mali/Mali-Medium.ttf" ), QStringLiteral( "ofl/mali/Mali-MediumItalic.ttf" ), QStringLiteral( "ofl/mali/Mali-SemiBold.ttf" ), QStringLiteral( "ofl/mali/Mali-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/mali/Mali-Bold.ttf" ), QStringLiteral( "ofl/mali/Mali-BoldItalic.ttf" ) }, QStringLiteral( "ofl/mali/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mallanna" ), { QStringLiteral( "ofl/mallanna/Mallanna-Regular.ttf" ) }, QStringLiteral( "ofl/mallanna/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mandali" ), { QStringLiteral( "ofl/mandali/Mandali-Regular.ttf" ) }, QStringLiteral( "ofl/mandali/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Manjari" ), { QStringLiteral( "ofl/manjari/Manjari-Thin.ttf" ), QStringLiteral( "ofl/manjari/Manjari-Regular.ttf" ), QStringLiteral( "ofl/manjari/Manjari-Bold.ttf" ) }, QStringLiteral( "ofl/manjari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Manrope" ), { QStringLiteral( "ofl/manrope/Manrope%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/manrope/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mansalva" ), { QStringLiteral( "ofl/mansalva/Mansalva-Regular.ttf" ) }, QStringLiteral( "ofl/mansalva/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Manuale" ), { QStringLiteral( "ofl/manuale/Manuale%5Bwght%5D.ttf" ), QStringLiteral( "ofl/manuale/Manuale-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/manuale/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marcellus" ), { QStringLiteral( "ofl/marcellus/Marcellus-Regular.ttf" ) }, QStringLiteral( "ofl/marcellus/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marcellus SC" ), { QStringLiteral( "ofl/marcellussc/MarcellusSC-Regular.ttf" ) }, QStringLiteral( "ofl/marcellussc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marck Script" ), { QStringLiteral( "ofl/marckscript/MarckScript-Regular.ttf" ) }, QStringLiteral( "ofl/marckscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Margarine" ), { QStringLiteral( "ofl/margarine/Margarine-Regular.ttf" ) }, QStringLiteral( "ofl/margarine/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marhey" ), { QStringLiteral( "ofl/marhey/Marhey%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/marhey/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Markazi Text" ), { QStringLiteral( "ofl/markazitext/MarkaziText%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/markazitext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marko One" ), { QStringLiteral( "ofl/markoone/MarkoOne-Regular.ttf" ) }, QStringLiteral( "ofl/markoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marmelad" ), { QStringLiteral( "ofl/marmelad/Marmelad-Regular.ttf" ) }, QStringLiteral( "ofl/marmelad/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Martel" ), { QStringLiteral( "ofl/martel/Martel-UltraLight.ttf" ), QStringLiteral( "ofl/martel/Martel-Light.ttf" ), QStringLiteral( "ofl/martel/Martel-Regular.ttf" ), QStringLiteral( "ofl/martel/Martel-DemiBold.ttf" ), QStringLiteral( "ofl/martel/Martel-Bold.ttf" ), QStringLiteral( "ofl/martel/Martel-ExtraBold.ttf" ), QStringLiteral( "ofl/martel/Martel-Heavy.ttf" ) }, QStringLiteral( "ofl/martel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Martel Sans" ), { QStringLiteral( "ofl/martelsans/MartelSans-ExtraLight.ttf" ), QStringLiteral( "ofl/martelsans/MartelSans-Light.ttf" ), QStringLiteral( "ofl/martelsans/MartelSans-Regular.ttf" ), QStringLiteral( "ofl/martelsans/MartelSans-SemiBold.ttf" ), QStringLiteral( "ofl/martelsans/MartelSans-Bold.ttf" ), QStringLiteral( "ofl/martelsans/MartelSans-ExtraBold.ttf" ), QStringLiteral( "ofl/martelsans/MartelSans-Black.ttf" ) }, QStringLiteral( "ofl/martelsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Martian Mono" ), { QStringLiteral( "ofl/martianmono/MartianMono%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/martianmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Marvel" ), { QStringLiteral( "ofl/marvel/Marvel-Regular.ttf" ), QStringLiteral( "ofl/marvel/Marvel-Italic.ttf" ), QStringLiteral( "ofl/marvel/Marvel-Bold.ttf" ), QStringLiteral( "ofl/marvel/Marvel-BoldItalic.ttf" ) }, QStringLiteral( "ofl/marvel/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mate" ), { QStringLiteral( "ofl/mate/Mate-Regular.ttf" ), QStringLiteral( "ofl/mate/Mate-Italic.ttf" ) }, QStringLiteral( "ofl/mate/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mate SC" ), { QStringLiteral( "ofl/matesc/MateSC-Regular.ttf" ) }, QStringLiteral( "ofl/matesc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Maven Pro" ), { QStringLiteral( "ofl/mavenpro/MavenPro%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mavenpro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "McLaren" ), { QStringLiteral( "ofl/mclaren/McLaren-Regular.ttf" ) }, QStringLiteral( "ofl/mclaren/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mea Culpa" ), { QStringLiteral( "ofl/meaculpa/MeaCulpa-Regular.ttf" ) }, QStringLiteral( "ofl/meaculpa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Meddon" ), { QStringLiteral( "ofl/meddon/Meddon.ttf" ) }, QStringLiteral( "ofl/meddon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "MedievalSharp" ), { QStringLiteral( "ofl/medievalsharp/MedievalSharp.ttf" ) }, QStringLiteral( "ofl/medievalsharp/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Medula One" ), { QStringLiteral( "ofl/medulaone/MedulaOne-Regular.ttf" ) }, QStringLiteral( "ofl/medulaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Meera Inimai" ), { QStringLiteral( "ofl/meerainimai/MeeraInimai-Regular.ttf" ) }, QStringLiteral( "ofl/meerainimai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Megrim" ), { QStringLiteral( "ofl/megrim/Megrim.ttf" ) }, QStringLiteral( "ofl/megrim/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Meie Script" ), { QStringLiteral( "ofl/meiescript/MeieScript-Regular.ttf" ) }, QStringLiteral( "ofl/meiescript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Meow Script" ), { QStringLiteral( "ofl/meowscript/MeowScript-Regular.ttf" ) }, QStringLiteral( "ofl/meowscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Merienda" ), { QStringLiteral( "ofl/merienda/Merienda%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/merienda/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Merriweather" ), { QStringLiteral( "ofl/merriweather/Merriweather-Light.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-LightItalic.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-Regular.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-Italic.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-Bold.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-BoldItalic.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-Black.ttf" ), QStringLiteral( "ofl/merriweather/Merriweather-BlackItalic.ttf" ) }, QStringLiteral( "ofl/merriweather/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Merriweather Sans" ), { QStringLiteral( "ofl/merriweathersans/MerriweatherSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/merriweathersans/MerriweatherSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/merriweathersans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Metal" ), { QStringLiteral( "ofl/metal/Metal-Regular.ttf" ) }, QStringLiteral( "ofl/metal/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Metal Mania" ), { QStringLiteral( "ofl/metalmania/MetalMania-Regular.ttf" ) }, QStringLiteral( "ofl/metalmania/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Metamorphous" ), { QStringLiteral( "ofl/metamorphous/Metamorphous-Regular.ttf" ) }, QStringLiteral( "ofl/metamorphous/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Metrophobic" ), { QStringLiteral( "ofl/metrophobic/Metrophobic-Regular.ttf" ) }, QStringLiteral( "ofl/metrophobic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Michroma" ), { QStringLiteral( "ofl/michroma/Michroma-Regular.ttf" ) }, QStringLiteral( "ofl/michroma/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Milonga" ), { QStringLiteral( "ofl/milonga/Milonga-Regular.ttf" ) }, QStringLiteral( "ofl/milonga/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Miltonian" ), { QStringLiteral( "ofl/miltonian/Miltonian-Regular.ttf" ) }, QStringLiteral( "ofl/miltonian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Miltonian Tattoo" ), { QStringLiteral( "ofl/miltoniantattoo/MiltonianTattoo-Regular.ttf" ) }, QStringLiteral( "ofl/miltoniantattoo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mina" ), { QStringLiteral( "ofl/mina/Mina-Regular.ttf" ), QStringLiteral( "ofl/mina/Mina-Bold.ttf" ) }, QStringLiteral( "ofl/mina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mingzat" ), { QStringLiteral( "ofl/mingzat/Mingzat-Regular.ttf" ) }, QStringLiteral( "ofl/mingzat/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Miniver" ), { QStringLiteral( "ofl/miniver/Miniver-Regular.ttf" ) }, QStringLiteral( "ofl/miniver/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Miriam Libre" ), { QStringLiteral( "ofl/miriamlibre/MiriamLibre%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/miriamlibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Miss Fajardose" ), { QStringLiteral( "ofl/missfajardose/MissFajardose-Regular.ttf" ) }, QStringLiteral( "ofl/missfajardose/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mochiy Pop One" ), { QStringLiteral( "ofl/mochiypopone/MochiyPopOne-Regular.ttf" ) }, QStringLiteral( "ofl/mochiypopone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mochiy Pop P One" ), { QStringLiteral( "ofl/mochiypoppone/MochiyPopPOne-Regular.ttf" ) }, QStringLiteral( "ofl/mochiypoppone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Modak" ), { QStringLiteral( "ofl/modak/Modak-Regular.ttf" ) }, QStringLiteral( "ofl/modak/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Modern Antiqua" ), { QStringLiteral( "ofl/modernantiqua/ModernAntiqua-Regular.ttf" ) }, QStringLiteral( "ofl/modernantiqua/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mohave" ), { QStringLiteral( "ofl/mohave/Mohave%5Bwght%5D.ttf" ), QStringLiteral( "ofl/mohave/Mohave-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mohave/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Moirai One" ), { QStringLiteral( "ofl/moiraione/MoiraiOne-Regular.ttf" ) }, QStringLiteral( "ofl/moiraione/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Molengo" ), { QStringLiteral( "ofl/molengo/Molengo-Regular.ttf" ) }, QStringLiteral( "ofl/molengo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Molle" ), { QStringLiteral( "ofl/molle/Molle-Regular.ttf" ) }, QStringLiteral( "ofl/molle/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Monda" ), { QStringLiteral( "ofl/monda/Monda%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/monda/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Monofett" ), { QStringLiteral( "ofl/monofett/Monofett-Regular.ttf" ) }, QStringLiteral( "ofl/monofett/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Monomaniac One" ), { QStringLiteral( "ofl/monomaniacone/MonomaniacOne-Regular.ttf" ) }, QStringLiteral( "ofl/monomaniacone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Monoton" ), { QStringLiteral( "ofl/monoton/Monoton-Regular.ttf" ) }, QStringLiteral( "ofl/monoton/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Monsieur La Doulaise" ), { QStringLiteral( "ofl/monsieurladoulaise/MonsieurLaDoulaise-Regular.ttf" ) }, QStringLiteral( "ofl/monsieurladoulaise/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Montaga" ), { QStringLiteral( "ofl/montaga/Montaga-Regular.ttf" ) }, QStringLiteral( "ofl/montaga/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Montagu Slab" ), { QStringLiteral( "ofl/montaguslab/MontaguSlab%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/montaguslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "MonteCarlo" ), { QStringLiteral( "ofl/montecarlo/MonteCarlo-Regular.ttf" ) }, QStringLiteral( "ofl/montecarlo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Montez" ), { QStringLiteral( "apache/montez/Montez-Regular.ttf" ) }, QStringLiteral( "apache/montez/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Montserrat" ), { QStringLiteral( "ofl/montserrat/Montserrat%5Bwght%5D.ttf" ), QStringLiteral( "ofl/montserrat/Montserrat-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/montserrat/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Montserrat Alternates" ), { QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Thin.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-ThinItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-ExtraLight.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Light.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-LightItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Regular.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Italic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Medium.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-MediumItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-SemiBold.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Bold.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-BoldItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-ExtraBold.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-Black.ttf" ), QStringLiteral( "ofl/montserratalternates/MontserratAlternates-BlackItalic.ttf" ) }, QStringLiteral( "ofl/montserratalternates/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Montserrat Subrayada" ), { QStringLiteral( "ofl/montserratsubrayada/MontserratSubrayada-Regular.ttf" ), QStringLiteral( "ofl/montserratsubrayada/MontserratSubrayada-Bold.ttf" ) }, QStringLiteral( "ofl/montserratsubrayada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Moo Lah Lah" ), { QStringLiteral( "ofl/moolahlah/MooLahLah-Regular.ttf" ) }, QStringLiteral( "ofl/moolahlah/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Moon Dance" ), { QStringLiteral( "ofl/moondance/MoonDance-Regular.ttf" ) }, QStringLiteral( "ofl/moondance/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Moul" ), { QStringLiteral( "ofl/moul/Moul-Regular.ttf" ) }, QStringLiteral( "ofl/moul/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Moulpali" ), { QStringLiteral( "ofl/moulpali/Moulpali-Regular.ttf" ) }, QStringLiteral( "ofl/moulpali/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mountains of Christmas" ), { QStringLiteral( "apache/mountainsofchristmas/MountainsofChristmas-Regular.ttf" ), QStringLiteral( "apache/mountainsofchristmas/MountainsofChristmas-Bold.ttf" ) }, QStringLiteral( "apache/mountainsofchristmas/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mouse Memoirs" ), { QStringLiteral( "ofl/mousememoirs/MouseMemoirs-Regular.ttf" ) }, QStringLiteral( "ofl/mousememoirs/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mr Bedfort" ), { QStringLiteral( "ofl/mrbedfort/MrBedfort-Regular.ttf" ) }, QStringLiteral( "ofl/mrbedfort/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mr Dafoe" ), { QStringLiteral( "ofl/mrdafoe/MrDafoe-Regular.ttf" ) }, QStringLiteral( "ofl/mrdafoe/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mr De Haviland" ), { QStringLiteral( "ofl/mrdehaviland/MrDeHaviland-Regular.ttf" ) }, QStringLiteral( "ofl/mrdehaviland/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mrs Saint Delafield" ), { QStringLiteral( "ofl/mrssaintdelafield/MrsSaintDelafield-Regular.ttf" ) }, QStringLiteral( "ofl/mrssaintdelafield/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mrs Sheppards" ), { QStringLiteral( "ofl/mrssheppards/MrsSheppards-Regular.ttf" ) }, QStringLiteral( "ofl/mrssheppards/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ms Madi" ), { QStringLiteral( "ofl/msmadi/MsMadi-Regular.ttf" ) }, QStringLiteral( "ofl/msmadi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mukta" ), { QStringLiteral( "ofl/mukta/Mukta-ExtraLight.ttf" ), QStringLiteral( "ofl/mukta/Mukta-Light.ttf" ), QStringLiteral( "ofl/mukta/Mukta-Regular.ttf" ), QStringLiteral( "ofl/mukta/Mukta-Medium.ttf" ), QStringLiteral( "ofl/mukta/Mukta-SemiBold.ttf" ), QStringLiteral( "ofl/mukta/Mukta-Bold.ttf" ), QStringLiteral( "ofl/mukta/Mukta-ExtraBold.ttf" ) }, QStringLiteral( "ofl/mukta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mukta Mahee" ), { QStringLiteral( "ofl/muktamahee/MuktaMahee-ExtraLight.ttf" ), QStringLiteral( "ofl/muktamahee/MuktaMahee-Light.ttf" ), QStringLiteral( "ofl/muktamahee/MuktaMahee-Regular.ttf" ), QStringLiteral( "ofl/muktamahee/MuktaMahee-Medium.ttf" ), QStringLiteral( "ofl/muktamahee/MuktaMahee-SemiBold.ttf" ), QStringLiteral( "ofl/muktamahee/MuktaMahee-Bold.ttf" ), QStringLiteral( "ofl/muktamahee/MuktaMahee-ExtraBold.ttf" ) }, QStringLiteral( "ofl/muktamahee/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mukta Malar" ), { QStringLiteral( "ofl/muktamalar/MuktaMalar-ExtraLight.ttf" ), QStringLiteral( "ofl/muktamalar/MuktaMalar-Light.ttf" ), QStringLiteral( "ofl/muktamalar/MuktaMalar-Regular.ttf" ), QStringLiteral( "ofl/muktamalar/MuktaMalar-Medium.ttf" ), QStringLiteral( "ofl/muktamalar/MuktaMalar-SemiBold.ttf" ), QStringLiteral( "ofl/muktamalar/MuktaMalar-Bold.ttf" ), QStringLiteral( "ofl/muktamalar/MuktaMalar-ExtraBold.ttf" ) }, QStringLiteral( "ofl/muktamalar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mukta Vaani" ), { QStringLiteral( "ofl/muktavaani/MuktaVaani-ExtraLight.ttf" ), QStringLiteral( "ofl/muktavaani/MuktaVaani-Light.ttf" ), QStringLiteral( "ofl/muktavaani/MuktaVaani-Regular.ttf" ), QStringLiteral( "ofl/muktavaani/MuktaVaani-Medium.ttf" ), QStringLiteral( "ofl/muktavaani/MuktaVaani-SemiBold.ttf" ), QStringLiteral( "ofl/muktavaani/MuktaVaani-Bold.ttf" ), QStringLiteral( "ofl/muktavaani/MuktaVaani-ExtraBold.ttf" ) }, QStringLiteral( "ofl/muktavaani/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mulish" ), { QStringLiteral( "ofl/mulish/Mulish%5Bwght%5D.ttf" ), QStringLiteral( "ofl/mulish/Mulish-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/mulish/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Murecho" ), { QStringLiteral( "ofl/murecho/Murecho%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/murecho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "MuseoModerno" ), { QStringLiteral( "ofl/museomoderno/MuseoModerno%5Bwght%5D.ttf" ), QStringLiteral( "ofl/museomoderno/MuseoModerno-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/museomoderno/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "My Soul" ), { QStringLiteral( "ofl/mysoul/MySoul-Regular.ttf" ) }, QStringLiteral( "ofl/mysoul/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mynerve" ), { QStringLiteral( "ofl/mynerve/Mynerve-Regular.ttf" ) }, QStringLiteral( "ofl/mynerve/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Mystery Quest" ), { QStringLiteral( "ofl/mysteryquest/MysteryQuest-Regular.ttf" ) }, QStringLiteral( "ofl/mysteryquest/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "NTR" ), { QStringLiteral( "ofl/ntr/NTR-Regular.ttf" ) }, QStringLiteral( "ofl/ntr/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nabla" ), { QStringLiteral( "ofl/nabla/Nabla%5BEDPT,EHLT%5D.ttf" ) }, QStringLiteral( "ofl/nabla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nanum Brush Script" ), { QStringLiteral( "ofl/nanumbrushscript/NanumBrushScript-Regular.ttf" ) }, QStringLiteral( "ofl/nanumbrushscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nanum Gothic" ), { QStringLiteral( "ofl/nanumgothic/NanumGothic-Regular.ttf" ), QStringLiteral( "ofl/nanumgothic/NanumGothic-Bold.ttf" ), QStringLiteral( "ofl/nanumgothic/NanumGothic-ExtraBold.ttf" ) }, QStringLiteral( "ofl/nanumgothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nanum Gothic Coding" ), { QStringLiteral( "ofl/nanumgothiccoding/NanumGothicCoding-Regular.ttf" ), QStringLiteral( "ofl/nanumgothiccoding/NanumGothicCoding-Bold.ttf" ) }, QStringLiteral( "ofl/nanumgothiccoding/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nanum Myeongjo" ), { QStringLiteral( "ofl/nanummyeongjo/NanumMyeongjo-Regular.ttf" ), QStringLiteral( "ofl/nanummyeongjo/NanumMyeongjo-Bold.ttf" ), QStringLiteral( "ofl/nanummyeongjo/NanumMyeongjo-ExtraBold.ttf" ) }, QStringLiteral( "ofl/nanummyeongjo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nanum Pen Script" ), { QStringLiteral( "ofl/nanumpenscript/NanumPenScript-Regular.ttf" ) }, QStringLiteral( "ofl/nanumpenscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Narnoor" ), { QStringLiteral( "ofl/narnoor/Narnoor-Regular.ttf" ), QStringLiteral( "ofl/narnoor/Narnoor-Medium.ttf" ), QStringLiteral( "ofl/narnoor/Narnoor-SemiBold.ttf" ), QStringLiteral( "ofl/narnoor/Narnoor-Bold.ttf" ), QStringLiteral( "ofl/narnoor/Narnoor-ExtraBold.ttf" ) }, QStringLiteral( "ofl/narnoor/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Neonderthaw" ), { QStringLiteral( "ofl/neonderthaw/Neonderthaw-Regular.ttf" ) }, QStringLiteral( "ofl/neonderthaw/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nerko One" ), { QStringLiteral( "ofl/nerkoone/NerkoOne-Regular.ttf" ) }, QStringLiteral( "ofl/nerkoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Neucha" ), { QStringLiteral( "ofl/neucha/Neucha.ttf" ) }, QStringLiteral( "ofl/neucha/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Neuton" ), { QStringLiteral( "ofl/neuton/Neuton-ExtraLight.ttf" ), QStringLiteral( "ofl/neuton/Neuton-Light.ttf" ), QStringLiteral( "ofl/neuton/Neuton-Regular.ttf" ), QStringLiteral( "ofl/neuton/Neuton-Italic.ttf" ), QStringLiteral( "ofl/neuton/Neuton-Bold.ttf" ), QStringLiteral( "ofl/neuton/Neuton-ExtraBold.ttf" ) }, QStringLiteral( "ofl/neuton/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "New Rocker" ), { QStringLiteral( "ofl/newrocker/NewRocker-Regular.ttf" ) }, QStringLiteral( "ofl/newrocker/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "New Tegomin" ), { QStringLiteral( "ofl/newtegomin/NewTegomin-Regular.ttf" ) }, QStringLiteral( "ofl/newtegomin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "News Cycle" ), { QStringLiteral( "ofl/newscycle/NewsCycle-Regular.ttf" ), QStringLiteral( "ofl/newscycle/NewsCycle-Bold.ttf" ) }, QStringLiteral( "ofl/newscycle/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Newsreader" ), { QStringLiteral( "ofl/newsreader/Newsreader%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/newsreader/Newsreader-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/newsreader/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Niconne" ), { QStringLiteral( "ofl/niconne/Niconne-Regular.ttf" ) }, QStringLiteral( "ofl/niconne/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Niramit" ), { QStringLiteral( "ofl/niramit/Niramit-ExtraLight.ttf" ), QStringLiteral( "ofl/niramit/Niramit-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/niramit/Niramit-Light.ttf" ), QStringLiteral( "ofl/niramit/Niramit-LightItalic.ttf" ), QStringLiteral( "ofl/niramit/Niramit-Regular.ttf" ), QStringLiteral( "ofl/niramit/Niramit-Italic.ttf" ), QStringLiteral( "ofl/niramit/Niramit-Medium.ttf" ), QStringLiteral( "ofl/niramit/Niramit-MediumItalic.ttf" ), QStringLiteral( "ofl/niramit/Niramit-SemiBold.ttf" ), QStringLiteral( "ofl/niramit/Niramit-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/niramit/Niramit-Bold.ttf" ), QStringLiteral( "ofl/niramit/Niramit-BoldItalic.ttf" ) }, QStringLiteral( "ofl/niramit/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nixie One" ), { QStringLiteral( "ofl/nixieone/NixieOne-Regular.ttf" ) }, QStringLiteral( "ofl/nixieone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nobile" ), { QStringLiteral( "ofl/nobile/Nobile-Regular.ttf" ), QStringLiteral( "ofl/nobile/Nobile-Italic.ttf" ), QStringLiteral( "ofl/nobile/Nobile-Medium.ttf" ), QStringLiteral( "ofl/nobile/Nobile-MediumItalic.ttf" ), QStringLiteral( "ofl/nobile/Nobile-Bold.ttf" ), QStringLiteral( "ofl/nobile/Nobile-BoldItalic.ttf" ) }, QStringLiteral( "ofl/nobile/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nokora" ), { QStringLiteral( "ofl/nokora/Nokora-Thin.ttf" ), QStringLiteral( "ofl/nokora/Nokora-Light.ttf" ), QStringLiteral( "ofl/nokora/Nokora-Regular.ttf" ), QStringLiteral( "ofl/nokora/Nokora-Bold.ttf" ), QStringLiteral( "ofl/nokora/Nokora-Black.ttf" ) }, QStringLiteral( "ofl/nokora/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Norican" ), { QStringLiteral( "ofl/norican/Norican-Regular.ttf" ) }, QStringLiteral( "ofl/norican/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nosifer" ), { QStringLiteral( "ofl/nosifer/Nosifer-Regular.ttf" ) }, QStringLiteral( "ofl/nosifer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Notable" ), { QStringLiteral( "ofl/notable/Notable-Regular.ttf" ) }, QStringLiteral( "ofl/notable/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nothing You Could Do" ), { QStringLiteral( "ofl/nothingyoucoulddo/NothingYouCouldDo.ttf" ) }, QStringLiteral( "ofl/nothingyoucoulddo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noticia Text" ), { QStringLiteral( "ofl/noticiatext/NoticiaText-Regular.ttf" ), QStringLiteral( "ofl/noticiatext/NoticiaText-Italic.ttf" ), QStringLiteral( "ofl/noticiatext/NoticiaText-Bold.ttf" ), QStringLiteral( "ofl/noticiatext/NoticiaText-BoldItalic.ttf" ) }, QStringLiteral( "ofl/noticiatext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Color Emoji" ), { QStringLiteral( "ofl/notocoloremoji/NotoColorEmoji-Regular.ttf" ) }, QStringLiteral( "ofl/notocoloremoji/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Emoji" ), { QStringLiteral( "ofl/notoemoji/NotoEmoji%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoemoji/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Kufi Arabic" ), { QStringLiteral( "ofl/notokufiarabic/NotoKufiArabic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notokufiarabic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Music" ), { QStringLiteral( "ofl/notomusic/NotoMusic-Regular.ttf" ) }, QStringLiteral( "ofl/notomusic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Naskh Arabic" ), { QStringLiteral( "ofl/notonaskharabic/NotoNaskhArabic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notonaskharabic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Nastaliq Urdu" ), { QStringLiteral( "ofl/notonastaliqurdu/NotoNastaliqUrdu%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notonastaliqurdu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Rashi Hebrew" ), { QStringLiteral( "ofl/notorashihebrew/NotoRashiHebrew%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notorashihebrew/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans" ), { QStringLiteral( "ofl/notosans/NotoSans%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/notosans/NotoSans-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Adlam" ), { QStringLiteral( "ofl/notosansadlam/NotoSansAdlam%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansadlam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Adlam Unjoined" ), { QStringLiteral( "ofl/notosansadlamunjoined/NotoSansAdlamUnjoined%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansadlamunjoined/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Anatolian Hieroglyphs" ), { QStringLiteral( "ofl/notosansanatolianhieroglyphs/NotoSansAnatolianHieroglyphs-Regular.ttf" ) }, QStringLiteral( "ofl/notosansanatolianhieroglyphs/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Arabic" ), { QStringLiteral( "ofl/notosansarabic/NotoSansArabic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansarabic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Armenian" ), { QStringLiteral( "ofl/notosansarmenian/NotoSansArmenian%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansarmenian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Avestan" ), { QStringLiteral( "ofl/notosansavestan/NotoSansAvestan-Regular.ttf" ) }, QStringLiteral( "ofl/notosansavestan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Balinese" ), { QStringLiteral( "ofl/notosansbalinese/NotoSansBalinese%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansbalinese/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Bamum" ), { QStringLiteral( "ofl/notosansbamum/NotoSansBamum%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansbamum/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Bassa Vah" ), { QStringLiteral( "ofl/notosansbassavah/NotoSansBassaVah%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansbassavah/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Batak" ), { QStringLiteral( "ofl/notosansbatak/NotoSansBatak-Regular.ttf" ) }, QStringLiteral( "ofl/notosansbatak/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Bengali" ), { QStringLiteral( "ofl/notosansbengali/NotoSansBengali%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansbengali/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Bhaiksuki" ), { QStringLiteral( "ofl/notosansbhaiksuki/NotoSansBhaiksuki-Regular.ttf" ) }, QStringLiteral( "ofl/notosansbhaiksuki/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Brahmi" ), { QStringLiteral( "ofl/notosansbrahmi/NotoSansBrahmi-Regular.ttf" ) }, QStringLiteral( "ofl/notosansbrahmi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Buginese" ), { QStringLiteral( "ofl/notosansbuginese/NotoSansBuginese-Regular.ttf" ) }, QStringLiteral( "ofl/notosansbuginese/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Buhid" ), { QStringLiteral( "ofl/notosansbuhid/NotoSansBuhid-Regular.ttf" ) }, QStringLiteral( "ofl/notosansbuhid/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Canadian Aboriginal" ), { QStringLiteral( "ofl/notosanscanadianaboriginal/NotoSansCanadianAboriginal%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanscanadianaboriginal/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Carian" ), { QStringLiteral( "ofl/notosanscarian/NotoSansCarian-Regular.ttf" ) }, QStringLiteral( "ofl/notosanscarian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Caucasian Albanian" ), { QStringLiteral( "ofl/notosanscaucasianalbanian/NotoSansCaucasianAlbanian-Regular.ttf" ) }, QStringLiteral( "ofl/notosanscaucasianalbanian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Chakma" ), { QStringLiteral( "ofl/notosanschakma/NotoSansChakma-Regular.ttf" ) }, QStringLiteral( "ofl/notosanschakma/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Cham" ), { QStringLiteral( "ofl/notosanscham/NotoSansCham%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanscham/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Cherokee" ), { QStringLiteral( "ofl/notosanscherokee/NotoSansCherokee%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanscherokee/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Chorasmian" ), { QStringLiteral( "ofl/notosanschorasmian/NotoSansChorasmian-Regular.ttf" ) }, QStringLiteral( "ofl/notosanschorasmian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Coptic" ), { QStringLiteral( "ofl/notosanscoptic/NotoSansCoptic-Regular.ttf" ) }, QStringLiteral( "ofl/notosanscoptic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Cuneiform" ), { QStringLiteral( "ofl/notosanscuneiform/NotoSansCuneiform-Regular.ttf" ) }, QStringLiteral( "ofl/notosanscuneiform/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Cypriot" ), { QStringLiteral( "ofl/notosanscypriot/NotoSansCypriot-Regular.ttf" ) }, QStringLiteral( "ofl/notosanscypriot/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Cypro Minoan" ), { QStringLiteral( "ofl/notosanscyprominoan/NotoSansCyproMinoan-Regular.ttf" ) }, QStringLiteral( "ofl/notosanscyprominoan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Deseret" ), { QStringLiteral( "ofl/notosansdeseret/NotoSansDeseret-Regular.ttf" ) }, QStringLiteral( "ofl/notosansdeseret/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Devanagari" ), { QStringLiteral( "ofl/notosansdevanagari/NotoSansDevanagari%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansdevanagari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Display" ), { QStringLiteral( "ofl/notosansdisplay/NotoSansDisplay%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/notosansdisplay/NotoSansDisplay-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Duployan" ), { QStringLiteral( "ofl/notosansduployan/NotoSansDuployan-Regular.ttf" ), QStringLiteral( "ofl/notosansduployan/NotoSansDuployan-Bold.ttf" ) }, QStringLiteral( "ofl/notosansduployan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Egyptian Hieroglyphs" ), { QStringLiteral( "ofl/notosansegyptianhieroglyphs/NotoSansEgyptianHieroglyphs-Regular.ttf" ) }, QStringLiteral( "ofl/notosansegyptianhieroglyphs/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Elbasan" ), { QStringLiteral( "ofl/notosanselbasan/NotoSansElbasan-Regular.ttf" ) }, QStringLiteral( "ofl/notosanselbasan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Elymaic" ), { QStringLiteral( "ofl/notosanselymaic/NotoSansElymaic-Regular.ttf" ) }, QStringLiteral( "ofl/notosanselymaic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Ethiopic" ), { QStringLiteral( "ofl/notosansethiopic/NotoSansEthiopic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansethiopic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Georgian" ), { QStringLiteral( "ofl/notosansgeorgian/NotoSansGeorgian%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansgeorgian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Glagolitic" ), { QStringLiteral( "ofl/notosansglagolitic/NotoSansGlagolitic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansglagolitic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Gothic" ), { QStringLiteral( "ofl/notosansgothic/NotoSansGothic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansgothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Grantha" ), { QStringLiteral( "ofl/notosansgrantha/NotoSansGrantha-Regular.ttf" ) }, QStringLiteral( "ofl/notosansgrantha/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Gujarati" ), { QStringLiteral( "ofl/notosansgujarati/NotoSansGujarati%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansgujarati/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Gunjala Gondi" ), { QStringLiteral( "ofl/notosansgunjalagondi/NotoSansGunjalaGondi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansgunjalagondi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Gurmukhi" ), { QStringLiteral( "ofl/notosansgurmukhi/NotoSansGurmukhi%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansgurmukhi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans HK" ), { QStringLiteral( "ofl/notosanshk/NotoSansHK%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanshk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Hanifi Rohingya" ), { QStringLiteral( "ofl/notosanshanifirohingya/NotoSansHanifiRohingya%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanshanifirohingya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Hanunoo" ), { QStringLiteral( "ofl/notosanshanunoo/NotoSansHanunoo-Regular.ttf" ) }, QStringLiteral( "ofl/notosanshanunoo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Hatran" ), { QStringLiteral( "ofl/notosanshatran/NotoSansHatran-Regular.ttf" ) }, QStringLiteral( "ofl/notosanshatran/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Hebrew" ), { QStringLiteral( "ofl/notosanshebrew/NotoSansHebrew%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanshebrew/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Imperial Aramaic" ), { QStringLiteral( "ofl/notosansimperialaramaic/NotoSansImperialAramaic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansimperialaramaic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Indic Siyaq Numbers" ), { QStringLiteral( "ofl/notosansindicsiyaqnumbers/NotoSansIndicSiyaqNumbers-Regular.ttf" ) }, QStringLiteral( "ofl/notosansindicsiyaqnumbers/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Inscriptional Pahlavi" ), { QStringLiteral( "ofl/notosansinscriptionalpahlavi/NotoSansInscriptionalPahlavi-Regular.ttf" ) }, QStringLiteral( "ofl/notosansinscriptionalpahlavi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Inscriptional Parthian" ), { QStringLiteral( "ofl/notosansinscriptionalparthian/NotoSansInscriptionalParthian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansinscriptionalparthian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans JP" ), { QStringLiteral( "ofl/notosansjp/NotoSansJP%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansjp/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Javanese" ), { QStringLiteral( "ofl/notosansjavanese/NotoSansJavanese%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansjavanese/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans KR" ), { QStringLiteral( "ofl/notosanskr/NotoSansKR%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanskr/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Kaithi" ), { QStringLiteral( "ofl/notosanskaithi/NotoSansKaithi-Regular.ttf" ) }, QStringLiteral( "ofl/notosanskaithi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Kannada" ), { QStringLiteral( "ofl/notosanskannada/NotoSansKannada%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanskannada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Kayah Li" ), { QStringLiteral( "ofl/notosanskayahli/NotoSansKayahLi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanskayahli/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Kharoshthi" ), { QStringLiteral( "ofl/notosanskharoshthi/NotoSansKharoshthi-Regular.ttf" ) }, QStringLiteral( "ofl/notosanskharoshthi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Khmer" ), { QStringLiteral( "ofl/notosanskhmer/NotoSansKhmer%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanskhmer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Khojki" ), { QStringLiteral( "ofl/notosanskhojki/NotoSansKhojki-Regular.ttf" ) }, QStringLiteral( "ofl/notosanskhojki/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Khudawadi" ), { QStringLiteral( "ofl/notosanskhudawadi/NotoSansKhudawadi-Regular.ttf" ) }, QStringLiteral( "ofl/notosanskhudawadi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Lao" ), { QStringLiteral( "ofl/notosanslao/NotoSansLao%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanslao/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Lao Looped" ), { QStringLiteral( "ofl/notosanslaolooped/NotoSansLaoLooped%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanslaolooped/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Lepcha" ), { QStringLiteral( "ofl/notosanslepcha/NotoSansLepcha-Regular.ttf" ) }, QStringLiteral( "ofl/notosanslepcha/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Limbu" ), { QStringLiteral( "ofl/notosanslimbu/NotoSansLimbu-Regular.ttf" ) }, QStringLiteral( "ofl/notosanslimbu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Linear A" ), { QStringLiteral( "ofl/notosanslineara/NotoSansLinearA-Regular.ttf" ) }, QStringLiteral( "ofl/notosanslineara/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Linear B" ), { QStringLiteral( "ofl/notosanslinearb/NotoSansLinearB-Regular.ttf" ) }, QStringLiteral( "ofl/notosanslinearb/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Lisu" ), { QStringLiteral( "ofl/notosanslisu/NotoSansLisu%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanslisu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Lycian" ), { QStringLiteral( "ofl/notosanslycian/NotoSansLycian-Regular.ttf" ) }, QStringLiteral( "ofl/notosanslycian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Lydian" ), { QStringLiteral( "ofl/notosanslydian/NotoSansLydian-Regular.ttf" ) }, QStringLiteral( "ofl/notosanslydian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mahajani" ), { QStringLiteral( "ofl/notosansmahajani/NotoSansMahajani-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmahajani/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Malayalam" ), { QStringLiteral( "ofl/notosansmalayalam/NotoSansMalayalam%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansmalayalam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mandaic" ), { QStringLiteral( "ofl/notosansmandaic/NotoSansMandaic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmandaic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Manichaean" ), { QStringLiteral( "ofl/notosansmanichaean/NotoSansManichaean-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmanichaean/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Marchen" ), { QStringLiteral( "ofl/notosansmarchen/NotoSansMarchen-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmarchen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Masaram Gondi" ), { QStringLiteral( "ofl/notosansmasaramgondi/NotoSansMasaramGondi-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmasaramgondi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Math" ), { QStringLiteral( "ofl/notosansmath/NotoSansMath-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmath/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mayan Numerals" ), { QStringLiteral( "ofl/notosansmayannumerals/NotoSansMayanNumerals-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmayannumerals/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Medefaidrin" ), { QStringLiteral( "ofl/notosansmedefaidrin/NotoSansMedefaidrin%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansmedefaidrin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Meetei Mayek" ), { QStringLiteral( "ofl/notosansmeeteimayek/NotoSansMeeteiMayek%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansmeeteimayek/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mende Kikakui" ), { QStringLiteral( "ofl/notosansmendekikakui/NotoSansMendeKikakui-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmendekikakui/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Meroitic" ), { QStringLiteral( "ofl/notosansmeroitic/NotoSansMeroitic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmeroitic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Miao" ), { QStringLiteral( "ofl/notosansmiao/NotoSansMiao-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmiao/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Modi" ), { QStringLiteral( "ofl/notosansmodi/NotoSansModi-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmodi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mongolian" ), { QStringLiteral( "ofl/notosansmongolian/NotoSansMongolian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmongolian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mono" ), { QStringLiteral( "ofl/notosansmono/NotoSansMono%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Mro" ), { QStringLiteral( "ofl/notosansmro/NotoSansMro-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Multani" ), { QStringLiteral( "ofl/notosansmultani/NotoSansMultani-Regular.ttf" ) }, QStringLiteral( "ofl/notosansmultani/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Myanmar" ), { QStringLiteral( "ofl/notosansmyanmar/NotoSansMyanmar%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansmyanmar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans NKo" ), { QStringLiteral( "ofl/notosansnko/NotoSansNKo-Regular.ttf" ) }, QStringLiteral( "ofl/notosansnko/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Nabataean" ), { QStringLiteral( "ofl/notosansnabataean/NotoSansNabataean-Regular.ttf" ) }, QStringLiteral( "ofl/notosansnabataean/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Nag Mundari" ), { QStringLiteral( "ofl/notosansnagmundari/NotoSansNagMundari%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansnagmundari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Nandinagari" ), { QStringLiteral( "ofl/notosansnandinagari/NotoSansNandinagari-Regular.ttf" ) }, QStringLiteral( "ofl/notosansnandinagari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans New Tai Lue" ), { QStringLiteral( "ofl/notosansnewtailue/NotoSansNewTaiLue%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansnewtailue/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Newa" ), { QStringLiteral( "ofl/notosansnewa/NotoSansNewa-Regular.ttf" ) }, QStringLiteral( "ofl/notosansnewa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Nushu" ), { QStringLiteral( "ofl/notosansnushu/NotoSansNushu-Regular.ttf" ) }, QStringLiteral( "ofl/notosansnushu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Ogham" ), { QStringLiteral( "ofl/notosansogham/NotoSansOgham-Regular.ttf" ) }, QStringLiteral( "ofl/notosansogham/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Ol Chiki" ), { QStringLiteral( "ofl/notosansolchiki/NotoSansOlChiki%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansolchiki/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old Hungarian" ), { QStringLiteral( "ofl/notosansoldhungarian/NotoSansOldHungarian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldhungarian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old Italic" ), { QStringLiteral( "ofl/notosansolditalic/NotoSansOldItalic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansolditalic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old North Arabian" ), { QStringLiteral( "ofl/notosansoldnortharabian/NotoSansOldNorthArabian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldnortharabian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old Permic" ), { QStringLiteral( "ofl/notosansoldpermic/NotoSansOldPermic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldpermic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old Persian" ), { QStringLiteral( "ofl/notosansoldpersian/NotoSansOldPersian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldpersian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old Sogdian" ), { QStringLiteral( "ofl/notosansoldsogdian/NotoSansOldSogdian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldsogdian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old South Arabian" ), { QStringLiteral( "ofl/notosansoldsoutharabian/NotoSansOldSouthArabian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldsoutharabian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Old Turkic" ), { QStringLiteral( "ofl/notosansoldturkic/NotoSansOldTurkic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansoldturkic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Oriya" ), { QStringLiteral( "ofl/notosansoriya/NotoSansOriya%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansoriya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Osage" ), { QStringLiteral( "ofl/notosansosage/NotoSansOsage-Regular.ttf" ) }, QStringLiteral( "ofl/notosansosage/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Osmanya" ), { QStringLiteral( "ofl/notosansosmanya/NotoSansOsmanya-Regular.ttf" ) }, QStringLiteral( "ofl/notosansosmanya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Pahawh Hmong" ), { QStringLiteral( "ofl/notosanspahawhhmong/NotoSansPahawhHmong-Regular.ttf" ) }, QStringLiteral( "ofl/notosanspahawhhmong/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Palmyrene" ), { QStringLiteral( "ofl/notosanspalmyrene/NotoSansPalmyrene-Regular.ttf" ) }, QStringLiteral( "ofl/notosanspalmyrene/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Pau Cin Hau" ), { QStringLiteral( "ofl/notosanspaucinhau/NotoSansPauCinHau-Regular.ttf" ) }, QStringLiteral( "ofl/notosanspaucinhau/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Phoenician" ), { QStringLiteral( "ofl/notosansphoenician/NotoSansPhoenician-Regular.ttf" ) }, QStringLiteral( "ofl/notosansphoenician/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Psalter Pahlavi" ), { QStringLiteral( "ofl/notosanspsalterpahlavi/NotoSansPsalterPahlavi-Regular.ttf" ) }, QStringLiteral( "ofl/notosanspsalterpahlavi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Rejang" ), { QStringLiteral( "ofl/notosansrejang/NotoSansRejang-Regular.ttf" ) }, QStringLiteral( "ofl/notosansrejang/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Runic" ), { QStringLiteral( "ofl/notosansrunic/NotoSansRunic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansrunic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans SC" ), { QStringLiteral( "ofl/notosanssc/NotoSansSC%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Samaritan" ), { QStringLiteral( "ofl/notosanssamaritan/NotoSansSamaritan-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssamaritan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Saurashtra" ), { QStringLiteral( "ofl/notosanssaurashtra/NotoSansSaurashtra-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssaurashtra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Sharada" ), { QStringLiteral( "ofl/notosanssharada/NotoSansSharada-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssharada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Shavian" ), { QStringLiteral( "ofl/notosansshavian/NotoSansShavian-Regular.ttf" ) }, QStringLiteral( "ofl/notosansshavian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Siddham" ), { QStringLiteral( "ofl/notosanssiddham/NotoSansSiddham-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssiddham/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans SignWriting" ), { QStringLiteral( "ofl/notosanssignwriting/NotoSansSignWriting-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssignwriting/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Sinhala" ), { QStringLiteral( "ofl/notosanssinhala/NotoSansSinhala%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssinhala/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Sogdian" ), { QStringLiteral( "ofl/notosanssogdian/NotoSansSogdian-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssogdian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Sora Sompeng" ), { QStringLiteral( "ofl/notosanssorasompeng/NotoSansSoraSompeng%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssorasompeng/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Soyombo" ), { QStringLiteral( "ofl/notosanssoyombo/NotoSansSoyombo-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssoyombo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Sundanese" ), { QStringLiteral( "ofl/notosanssundanese/NotoSansSundanese%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssundanese/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Syloti Nagri" ), { QStringLiteral( "ofl/notosanssylotinagri/NotoSansSylotiNagri-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssylotinagri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Symbols" ), { QStringLiteral( "ofl/notosanssymbols/NotoSansSymbols%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssymbols/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Symbols 2" ), { QStringLiteral( "ofl/notosanssymbols2/NotoSansSymbols2-Regular.ttf" ) }, QStringLiteral( "ofl/notosanssymbols2/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Syriac" ), { QStringLiteral( "ofl/notosanssyriac/NotoSansSyriac%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssyriac/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Syriac Eastern" ), { QStringLiteral( "ofl/notosanssyriaceastern/NotoSansSyriacEastern%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanssyriaceastern/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans TC" ), { QStringLiteral( "ofl/notosanstc/NotoSansTC%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanstc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tagalog" ), { QStringLiteral( "ofl/notosanstagalog/NotoSansTagalog-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstagalog/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tagbanwa" ), { QStringLiteral( "ofl/notosanstagbanwa/NotoSansTagbanwa-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstagbanwa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tai Le" ), { QStringLiteral( "ofl/notosanstaile/NotoSansTaiLe-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstaile/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tai Tham" ), { QStringLiteral( "ofl/notosanstaitham/NotoSansTaiTham%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanstaitham/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tai Viet" ), { QStringLiteral( "ofl/notosanstaiviet/NotoSansTaiViet-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstaiviet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Takri" ), { QStringLiteral( "ofl/notosanstakri/NotoSansTakri-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstakri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tamil" ), { QStringLiteral( "ofl/notosanstamil/NotoSansTamil%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanstamil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tamil Supplement" ), { QStringLiteral( "ofl/notosanstamilsupplement/NotoSansTamilSupplement-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstamilsupplement/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tangsa" ), { QStringLiteral( "ofl/notosanstangsa/NotoSansTangsa%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosanstangsa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Telugu" ), { QStringLiteral( "ofl/notosanstelugu/NotoSansTelugu%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosanstelugu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Thaana" ), { QStringLiteral( "ofl/notosansthaana/NotoSansThaana%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansthaana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Thai" ), { QStringLiteral( "ofl/notosansthai/NotoSansThai%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notosansthai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Thai Looped" ), { QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-Thin.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-ExtraLight.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-Light.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-Regular.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-Medium.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-SemiBold.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-Bold.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-ExtraBold.ttf" ), QStringLiteral( "ofl/notosansthailooped/NotoSansThaiLooped-Black.ttf" ) }, QStringLiteral( "ofl/notosansthailooped/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tifinagh" ), { QStringLiteral( "ofl/notosanstifinagh/NotoSansTifinagh-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstifinagh/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Tirhuta" ), { QStringLiteral( "ofl/notosanstirhuta/NotoSansTirhuta-Regular.ttf" ) }, QStringLiteral( "ofl/notosanstirhuta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Ugaritic" ), { QStringLiteral( "ofl/notosansugaritic/NotoSansUgaritic-Regular.ttf" ) }, QStringLiteral( "ofl/notosansugaritic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Vai" ), { QStringLiteral( "ofl/notosansvai/NotoSansVai-Regular.ttf" ) }, QStringLiteral( "ofl/notosansvai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Vithkuqi" ), { QStringLiteral( "ofl/notosansvithkuqi/NotoSansVithkuqi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notosansvithkuqi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Wancho" ), { QStringLiteral( "ofl/notosanswancho/NotoSansWancho-Regular.ttf" ) }, QStringLiteral( "ofl/notosanswancho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Warang Citi" ), { QStringLiteral( "ofl/notosanswarangciti/NotoSansWarangCiti-Regular.ttf" ) }, QStringLiteral( "ofl/notosanswarangciti/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Yi" ), { QStringLiteral( "ofl/notosansyi/NotoSansYi-Regular.ttf" ) }, QStringLiteral( "ofl/notosansyi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Sans Zanabazar Square" ), { QStringLiteral( "ofl/notosanszanabazarsquare/NotoSansZanabazarSquare-Regular.ttf" ) }, QStringLiteral( "ofl/notosanszanabazarsquare/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif" ), { QStringLiteral( "ofl/notoserif/NotoSerif%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/notoserif/NotoSerif-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Ahom" ), { QStringLiteral( "ofl/notoserifahom/NotoSerifAhom-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifahom/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Armenian" ), { QStringLiteral( "ofl/notoserifarmenian/NotoSerifArmenian%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifarmenian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Balinese" ), { QStringLiteral( "ofl/notoserifbalinese/NotoSerifBalinese-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifbalinese/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Bengali" ), { QStringLiteral( "ofl/notoserifbengali/NotoSerifBengali%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifbengali/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Devanagari" ), { QStringLiteral( "ofl/notoserifdevanagari/NotoSerifDevanagari%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifdevanagari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Display" ), { QStringLiteral( "ofl/notoserifdisplay/NotoSerifDisplay%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/notoserifdisplay/NotoSerifDisplay-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Dogra" ), { QStringLiteral( "ofl/notoserifdogra/NotoSerifDogra-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifdogra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Ethiopic" ), { QStringLiteral( "ofl/notoserifethiopic/NotoSerifEthiopic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifethiopic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Georgian" ), { QStringLiteral( "ofl/notoserifgeorgian/NotoSerifGeorgian%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifgeorgian/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Grantha" ), { QStringLiteral( "ofl/notoserifgrantha/NotoSerifGrantha-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifgrantha/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Gujarati" ), { QStringLiteral( "ofl/notoserifgujarati/NotoSerifGujarati%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifgujarati/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Gurmukhi" ), { QStringLiteral( "ofl/notoserifgurmukhi/NotoSerifGurmukhi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifgurmukhi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif HK" ), { QStringLiteral( "ofl/notoserifhk/NotoSerifHK%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifhk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Hebrew" ), { QStringLiteral( "ofl/notoserifhebrew/NotoSerifHebrew%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifhebrew/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif JP" ), { QStringLiteral( "ofl/notoserifjp/NotoSerifJP%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifjp/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif KR" ), { QStringLiteral( "ofl/notoserifkr/NotoSerifKR%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifkr/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Kannada" ), { QStringLiteral( "ofl/notoserifkannada/NotoSerifKannada%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifkannada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Khitan Small Script" ), { QStringLiteral( "ofl/notoserifkhitansmallscript/NotoSerifKhitanSmallScript-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifkhitansmallscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Khmer" ), { QStringLiteral( "ofl/notoserifkhmer/NotoSerifKhmer%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifkhmer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Khojki" ), { QStringLiteral( "ofl/notoserifkhojki/NotoSerifKhojki%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifkhojki/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Lao" ), { QStringLiteral( "ofl/notoseriflao/NotoSerifLao%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriflao/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Makasar" ), { QStringLiteral( "ofl/notoserifmakasar/NotoSerifMakasar-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifmakasar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Malayalam" ), { QStringLiteral( "ofl/notoserifmalayalam/NotoSerifMalayalam%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifmalayalam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Myanmar" ), { QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-Thin.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-ExtraLight.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-Light.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-Regular.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-Medium.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-SemiBold.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-Bold.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-ExtraBold.ttf" ), QStringLiteral( "ofl/notoserifmyanmar/NotoSerifMyanmar-Black.ttf" ) }, QStringLiteral( "ofl/notoserifmyanmar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif NP Hmong" ), { QStringLiteral( "ofl/notoserifnphmong/NotoSerifNPHmong%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifnphmong/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Oriya" ), { QStringLiteral( "ofl/notoseriforiya/NotoSerifOriya%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriforiya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Ottoman Siyaq" ), { QStringLiteral( "ofl/notoserifottomansiyaq/NotoSerifOttomanSiyaq-Regular.ttf" ) }, QStringLiteral( "ofl/notoserifottomansiyaq/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif SC" ), { QStringLiteral( "ofl/notoserifsc/NotoSerifSC%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Sinhala" ), { QStringLiteral( "ofl/notoserifsinhala/NotoSerifSinhala%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifsinhala/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif TC" ), { QStringLiteral( "ofl/notoseriftc/NotoSerifTC%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriftc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Tamil" ), { QStringLiteral( "ofl/notoseriftamil/NotoSerifTamil%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/notoseriftamil/NotoSerifTamil-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriftamil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Tangut" ), { QStringLiteral( "ofl/notoseriftangut/NotoSerifTangut-Regular.ttf" ) }, QStringLiteral( "ofl/notoseriftangut/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Telugu" ), { QStringLiteral( "ofl/notoseriftelugu/NotoSerifTelugu%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriftelugu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Thai" ), { QStringLiteral( "ofl/notoserifthai/NotoSerifThai%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifthai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Tibetan" ), { QStringLiteral( "ofl/notoseriftibetan/NotoSerifTibetan%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriftibetan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Toto" ), { QStringLiteral( "ofl/notoseriftoto/NotoSerifToto%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoseriftoto/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Vithkuqi" ), { QStringLiteral( "ofl/notoserifvithkuqi/NotoSerifVithkuqi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifvithkuqi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Serif Yezidi" ), { QStringLiteral( "ofl/notoserifyezidi/NotoSerifYezidi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/notoserifyezidi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Noto Traditional Nushu" ), { QStringLiteral( "ofl/nototraditionalnushu/NotoTraditionalNushu%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/nototraditionalnushu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Cut" ), { QStringLiteral( "ofl/novacut/NovaCut.ttf" ) }, QStringLiteral( "ofl/novacut/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Flat" ), { QStringLiteral( "ofl/novaflat/NovaFlat.ttf" ) }, QStringLiteral( "ofl/novaflat/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Mono" ), { QStringLiteral( "ofl/novamono/NovaMono.ttf" ) }, QStringLiteral( "ofl/novamono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Oval" ), { QStringLiteral( "ofl/novaoval/NovaOval.ttf" ) }, QStringLiteral( "ofl/novaoval/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Round" ), { QStringLiteral( "ofl/novaround/NovaRound.ttf" ) }, QStringLiteral( "ofl/novaround/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Script" ), { QStringLiteral( "ofl/novascript/NovaScript-Regular.ttf" ) }, QStringLiteral( "ofl/novascript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Slim" ), { QStringLiteral( "ofl/novaslim/NovaSlim.ttf" ) }, QStringLiteral( "ofl/novaslim/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nova Square" ), { QStringLiteral( "ofl/novasquare/NovaSquare.ttf" ) }, QStringLiteral( "ofl/novasquare/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Numans" ), { QStringLiteral( "ofl/numans/Numans-Regular.ttf" ) }, QStringLiteral( "ofl/numans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nunito" ), { QStringLiteral( "ofl/nunito/Nunito%5Bwght%5D.ttf" ), QStringLiteral( "ofl/nunito/Nunito-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/nunito/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nunito Sans" ), { QStringLiteral( "ofl/nunitosans/NunitoSans%5BYTLC,opsz,wdth,wght%5D.ttf" ), QStringLiteral( "ofl/nunitosans/NunitoSans-Italic%5BYTLC,opsz,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/nunitosans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Nuosu SIL" ), { QStringLiteral( "ofl/nuosusil/NuosuSIL-Regular.ttf" ) }, QStringLiteral( "ofl/nuosusil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Odibee Sans" ), { QStringLiteral( "ofl/odibeesans/OdibeeSans-Regular.ttf" ) }, QStringLiteral( "ofl/odibeesans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Odor Mean Chey" ), { QStringLiteral( "ofl/odormeanchey/OdorMeanChey-Regular.ttf" ) }, QStringLiteral( "ofl/odormeanchey/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Offside" ), { QStringLiteral( "ofl/offside/Offside-Regular.ttf" ) }, QStringLiteral( "ofl/offside/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oi" ), { QStringLiteral( "ofl/oi/Oi-Regular.ttf" ) }, QStringLiteral( "ofl/oi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Old Standard TT" ), { QStringLiteral( "ofl/oldstandardtt/OldStandard-Regular.ttf" ), QStringLiteral( "ofl/oldstandardtt/OldStandard-Italic.ttf" ), QStringLiteral( "ofl/oldstandardtt/OldStandard-Bold.ttf" ) }, QStringLiteral( "ofl/oldstandardtt/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oldenburg" ), { QStringLiteral( "ofl/oldenburg/Oldenburg-Regular.ttf" ) }, QStringLiteral( "ofl/oldenburg/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ole" ), { QStringLiteral( "ofl/ole/Ole-Regular.ttf" ) }, QStringLiteral( "ofl/ole/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oleo Script" ), { QStringLiteral( "ofl/oleoscript/OleoScript-Regular.ttf" ), QStringLiteral( "ofl/oleoscript/OleoScript-Bold.ttf" ) }, QStringLiteral( "ofl/oleoscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oleo Script Swash Caps" ), { QStringLiteral( "ofl/oleoscriptswashcaps/OleoScriptSwashCaps-Regular.ttf" ), QStringLiteral( "ofl/oleoscriptswashcaps/OleoScriptSwashCaps-Bold.ttf" ) }, QStringLiteral( "ofl/oleoscriptswashcaps/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oooh Baby" ), { QStringLiteral( "ofl/ooohbaby/OoohBaby-Regular.ttf" ) }, QStringLiteral( "ofl/ooohbaby/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Open Sans" ), { QStringLiteral( "ofl/opensans/OpenSans%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/opensans/OpenSans-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/opensans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oranienbaum" ), { QStringLiteral( "ofl/oranienbaum/Oranienbaum-Regular.ttf" ) }, QStringLiteral( "ofl/oranienbaum/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Orbit" ), { QStringLiteral( "ofl/orbit/Orbit-Regular.ttf" ) }, QStringLiteral( "ofl/orbit/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Orbitron" ), { QStringLiteral( "ofl/orbitron/Orbitron%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/orbitron/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oregano" ), { QStringLiteral( "ofl/oregano/Oregano-Regular.ttf" ), QStringLiteral( "ofl/oregano/Oregano-Italic.ttf" ) }, QStringLiteral( "ofl/oregano/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Orelega One" ), { QStringLiteral( "ofl/orelegaone/OrelegaOne-Regular.ttf" ) }, QStringLiteral( "ofl/orelegaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Orienta" ), { QStringLiteral( "ofl/orienta/Orienta-Regular.ttf" ) }, QStringLiteral( "ofl/orienta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Original Surfer" ), { QStringLiteral( "ofl/originalsurfer/OriginalSurfer-Regular.ttf" ) }, QStringLiteral( "ofl/originalsurfer/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oswald" ), { QStringLiteral( "ofl/oswald/Oswald%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/oswald/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Outfit" ), { QStringLiteral( "ofl/outfit/Outfit%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/outfit/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Over the Rainbow" ), { QStringLiteral( "ofl/overtherainbow/OvertheRainbow.ttf" ) }, QStringLiteral( "ofl/overtherainbow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Overlock" ), { QStringLiteral( "ofl/overlock/Overlock-Regular.ttf" ), QStringLiteral( "ofl/overlock/Overlock-Italic.ttf" ), QStringLiteral( "ofl/overlock/Overlock-Bold.ttf" ), QStringLiteral( "ofl/overlock/Overlock-BoldItalic.ttf" ), QStringLiteral( "ofl/overlock/Overlock-Black.ttf" ), QStringLiteral( "ofl/overlock/Overlock-BlackItalic.ttf" ) }, QStringLiteral( "ofl/overlock/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Overlock SC" ), { QStringLiteral( "ofl/overlocksc/OverlockSC-Regular.ttf" ) }, QStringLiteral( "ofl/overlocksc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Overpass" ), { QStringLiteral( "ofl/overpass/Overpass%5Bwght%5D.ttf" ), QStringLiteral( "ofl/overpass/Overpass-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/overpass/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Overpass Mono" ), { QStringLiteral( "ofl/overpassmono/OverpassMono%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/overpassmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ovo" ), { QStringLiteral( "ofl/ovo/Ovo-Regular.ttf" ) }, QStringLiteral( "ofl/ovo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oxanium" ), { QStringLiteral( "ofl/oxanium/Oxanium%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/oxanium/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oxygen" ), { QStringLiteral( "ofl/oxygen/Oxygen-Light.ttf" ), QStringLiteral( "ofl/oxygen/Oxygen-Regular.ttf" ), QStringLiteral( "ofl/oxygen/Oxygen-Bold.ttf" ) }, QStringLiteral( "ofl/oxygen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Oxygen Mono" ), { QStringLiteral( "ofl/oxygenmono/OxygenMono-Regular.ttf" ) }, QStringLiteral( "ofl/oxygenmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "PT Mono" ), { QStringLiteral( "ofl/ptmono/PTM55FT.ttf" ) }, QStringLiteral( "ofl/ptmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "PT Sans" ), { QStringLiteral( "ofl/ptsans/PT_Sans-Web-Regular.ttf" ), QStringLiteral( "ofl/ptsans/PT_Sans-Web-Italic.ttf" ), QStringLiteral( "ofl/ptsans/PT_Sans-Web-Bold.ttf" ), QStringLiteral( "ofl/ptsans/PT_Sans-Web-BoldItalic.ttf" ) }, QStringLiteral( "ofl/ptsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "PT Sans Caption" ), { QStringLiteral( "ofl/ptsanscaption/PT_Sans-Caption-Web-Regular.ttf" ), QStringLiteral( "ofl/ptsanscaption/PT_Sans-Caption-Web-Bold.ttf" ) }, QStringLiteral( "ofl/ptsanscaption/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "PT Sans Narrow" ), { QStringLiteral( "ofl/ptsansnarrow/PT_Sans-Narrow-Web-Regular.ttf" ), QStringLiteral( "ofl/ptsansnarrow/PT_Sans-Narrow-Web-Bold.ttf" ) }, QStringLiteral( "ofl/ptsansnarrow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "PT Serif" ), { QStringLiteral( "ofl/ptserif/PT_Serif-Web-Regular.ttf" ), QStringLiteral( "ofl/ptserif/PT_Serif-Web-Italic.ttf" ), QStringLiteral( "ofl/ptserif/PT_Serif-Web-Bold.ttf" ), QStringLiteral( "ofl/ptserif/PT_Serif-Web-BoldItalic.ttf" ) }, QStringLiteral( "ofl/ptserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "PT Serif Caption" ), { QStringLiteral( "ofl/ptserifcaption/PT_Serif-Caption-Web-Regular.ttf" ), QStringLiteral( "ofl/ptserifcaption/PT_Serif-Caption-Web-Italic.ttf" ) }, QStringLiteral( "ofl/ptserifcaption/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pacifico" ), { QStringLiteral( "ofl/pacifico/Pacifico-Regular.ttf" ) }, QStringLiteral( "ofl/pacifico/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Padauk" ), { QStringLiteral( "ofl/padauk/Padauk-Regular.ttf" ), QStringLiteral( "ofl/padauk/Padauk-Bold.ttf" ) }, QStringLiteral( "ofl/padauk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Padyakke Expanded One" ), { QStringLiteral( "ofl/padyakkeexpandedone/PadyakkeExpandedOne-Regular.ttf" ) }, QStringLiteral( "ofl/padyakkeexpandedone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Palanquin" ), { QStringLiteral( "ofl/palanquin/Palanquin-Thin.ttf" ), QStringLiteral( "ofl/palanquin/Palanquin-ExtraLight.ttf" ), QStringLiteral( "ofl/palanquin/Palanquin-Light.ttf" ), QStringLiteral( "ofl/palanquin/Palanquin-Regular.ttf" ), QStringLiteral( "ofl/palanquin/Palanquin-Medium.ttf" ), QStringLiteral( "ofl/palanquin/Palanquin-SemiBold.ttf" ), QStringLiteral( "ofl/palanquin/Palanquin-Bold.ttf" ) }, QStringLiteral( "ofl/palanquin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Palanquin Dark" ), { QStringLiteral( "ofl/palanquindark/PalanquinDark-Regular.ttf" ), QStringLiteral( "ofl/palanquindark/PalanquinDark-Medium.ttf" ), QStringLiteral( "ofl/palanquindark/PalanquinDark-SemiBold.ttf" ), QStringLiteral( "ofl/palanquindark/PalanquinDark-Bold.ttf" ) }, QStringLiteral( "ofl/palanquindark/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Palette Mosaic" ), { QStringLiteral( "ofl/palettemosaic/PaletteMosaic-Regular.ttf" ) }, QStringLiteral( "ofl/palettemosaic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pangolin" ), { QStringLiteral( "ofl/pangolin/Pangolin-Regular.ttf" ) }, QStringLiteral( "ofl/pangolin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Paprika" ), { QStringLiteral( "ofl/paprika/Paprika-Regular.ttf" ) }, QStringLiteral( "ofl/paprika/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Parisienne" ), { QStringLiteral( "ofl/parisienne/Parisienne-Regular.ttf" ) }, QStringLiteral( "ofl/parisienne/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Passero One" ), { QStringLiteral( "ofl/passeroone/PasseroOne-Regular.ttf" ) }, QStringLiteral( "ofl/passeroone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Passion One" ), { QStringLiteral( "ofl/passionone/PassionOne-Regular.ttf" ), QStringLiteral( "ofl/passionone/PassionOne-Bold.ttf" ), QStringLiteral( "ofl/passionone/PassionOne-Black.ttf" ) }, QStringLiteral( "ofl/passionone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Passions Conflict" ), { QStringLiteral( "ofl/passionsconflict/PassionsConflict-Regular.ttf" ) }, QStringLiteral( "ofl/passionsconflict/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pathway Extreme" ), { QStringLiteral( "ofl/pathwayextreme/PathwayExtreme%5Bopsz,wdth,wght%5D.ttf" ), QStringLiteral( "ofl/pathwayextreme/PathwayExtreme-Italic%5Bopsz,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/pathwayextreme/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pathway Gothic One" ), { QStringLiteral( "ofl/pathwaygothicone/PathwayGothicOne-Regular.ttf" ) }, QStringLiteral( "ofl/pathwaygothicone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Patrick Hand" ), { QStringLiteral( "ofl/patrickhand/PatrickHand-Regular.ttf" ) }, QStringLiteral( "ofl/patrickhand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Patrick Hand SC" ), { QStringLiteral( "ofl/patrickhandsc/PatrickHandSC-Regular.ttf" ) }, QStringLiteral( "ofl/patrickhandsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pattaya" ), { QStringLiteral( "ofl/pattaya/Pattaya-Regular.ttf" ) }, QStringLiteral( "ofl/pattaya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Patua One" ), { QStringLiteral( "ofl/patuaone/PatuaOne-Regular.ttf" ) }, QStringLiteral( "ofl/patuaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pavanam" ), { QStringLiteral( "ofl/pavanam/Pavanam-Regular.ttf" ) }, QStringLiteral( "ofl/pavanam/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Paytone One" ), { QStringLiteral( "ofl/paytoneone/PaytoneOne-Regular.ttf" ) }, QStringLiteral( "ofl/paytoneone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Peddana" ), { QStringLiteral( "ofl/peddana/Peddana-Regular.ttf" ) }, QStringLiteral( "ofl/peddana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Peralta" ), { QStringLiteral( "ofl/peralta/Peralta-Regular.ttf" ) }, QStringLiteral( "ofl/peralta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Permanent Marker" ), { QStringLiteral( "apache/permanentmarker/PermanentMarker-Regular.ttf" ) }, QStringLiteral( "apache/permanentmarker/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Petemoss" ), { QStringLiteral( "ofl/petemoss/Petemoss-Regular.ttf" ) }, QStringLiteral( "ofl/petemoss/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Petit Formal Script" ), { QStringLiteral( "ofl/petitformalscript/PetitFormalScript-Regular.ttf" ) }, QStringLiteral( "ofl/petitformalscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Petrona" ), { QStringLiteral( "ofl/petrona/Petrona%5Bwght%5D.ttf" ), QStringLiteral( "ofl/petrona/Petrona-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/petrona/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Philosopher" ), { QStringLiteral( "ofl/philosopher/Philosopher-Regular.ttf" ), QStringLiteral( "ofl/philosopher/Philosopher-Italic.ttf" ), QStringLiteral( "ofl/philosopher/Philosopher-Bold.ttf" ), QStringLiteral( "ofl/philosopher/Philosopher-BoldItalic.ttf" ) }, QStringLiteral( "ofl/philosopher/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Phudu" ), { QStringLiteral( "ofl/phudu/Phudu%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/phudu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Piazzolla" ), { QStringLiteral( "ofl/piazzolla/Piazzolla%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/piazzolla/Piazzolla-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/piazzolla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Piedra" ), { QStringLiteral( "ofl/piedra/Piedra-Regular.ttf" ) }, QStringLiteral( "ofl/piedra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pinyon Script" ), { QStringLiteral( "ofl/pinyonscript/PinyonScript-Regular.ttf" ) }, QStringLiteral( "ofl/pinyonscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pirata One" ), { QStringLiteral( "ofl/pirataone/PirataOne-Regular.ttf" ) }, QStringLiteral( "ofl/pirataone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Plaster" ), { QStringLiteral( "ofl/plaster/Plaster-Regular.ttf" ) }, QStringLiteral( "ofl/plaster/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Play" ), { QStringLiteral( "ofl/play/Play-Regular.ttf" ), QStringLiteral( "ofl/play/Play-Bold.ttf" ) }, QStringLiteral( "ofl/play/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Playball" ), { QStringLiteral( "ofl/playball/Playball-Regular.ttf" ) }, QStringLiteral( "ofl/playball/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Playfair" ), { QStringLiteral( "ofl/playfair/Playfair%5Bopsz,wdth,wght%5D.ttf" ), QStringLiteral( "ofl/playfair/Playfair-Italic%5Bopsz,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/playfair/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Playfair Display" ), { QStringLiteral( "ofl/playfairdisplay/PlayfairDisplay%5Bwght%5D.ttf" ), QStringLiteral( "ofl/playfairdisplay/PlayfairDisplay-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/playfairdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Playfair Display SC" ), { QStringLiteral( "ofl/playfairdisplaysc/PlayfairDisplaySC-Regular.ttf" ), QStringLiteral( "ofl/playfairdisplaysc/PlayfairDisplaySC-Italic.ttf" ), QStringLiteral( "ofl/playfairdisplaysc/PlayfairDisplaySC-Bold.ttf" ), QStringLiteral( "ofl/playfairdisplaysc/PlayfairDisplaySC-BoldItalic.ttf" ), QStringLiteral( "ofl/playfairdisplaysc/PlayfairDisplaySC-Black.ttf" ), QStringLiteral( "ofl/playfairdisplaysc/PlayfairDisplaySC-BlackItalic.ttf" ) }, QStringLiteral( "ofl/playfairdisplaysc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Plus Jakarta Sans" ), { QStringLiteral( "ofl/plusjakartasans/PlusJakartaSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/plusjakartasans/PlusJakartaSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/plusjakartasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Podkova" ), { QStringLiteral( "ofl/podkova/Podkova%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/podkova/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Poiret One" ), { QStringLiteral( "ofl/poiretone/PoiretOne-Regular.ttf" ) }, QStringLiteral( "ofl/poiretone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Poller One" ), { QStringLiteral( "ofl/pollerone/PollerOne.ttf" ) }, QStringLiteral( "ofl/pollerone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Poltawski Nowy" ), { QStringLiteral( "ofl/poltawskinowy/PoltawskiNowy%5Bwght%5D.ttf" ), QStringLiteral( "ofl/poltawskinowy/PoltawskiNowy-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/poltawskinowy/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Poly" ), { QStringLiteral( "ofl/poly/Poly-Regular.ttf" ), QStringLiteral( "ofl/poly/Poly-Italic.ttf" ) }, QStringLiteral( "ofl/poly/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pompiere" ), { QStringLiteral( "ofl/pompiere/Pompiere-Regular.ttf" ) }, QStringLiteral( "ofl/pompiere/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pontano Sans" ), { QStringLiteral( "ofl/pontanosans/PontanoSans%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/pontanosans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Poor Story" ), { QStringLiteral( "ofl/poorstory/PoorStory-Regular.ttf" ) }, QStringLiteral( "ofl/poorstory/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Poppins" ), { QStringLiteral( "ofl/poppins/Poppins-Thin.ttf" ), QStringLiteral( "ofl/poppins/Poppins-ThinItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-ExtraLight.ttf" ), QStringLiteral( "ofl/poppins/Poppins-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-Light.ttf" ), QStringLiteral( "ofl/poppins/Poppins-LightItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-Regular.ttf" ), QStringLiteral( "ofl/poppins/Poppins-Italic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-Medium.ttf" ), QStringLiteral( "ofl/poppins/Poppins-MediumItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-SemiBold.ttf" ), QStringLiteral( "ofl/poppins/Poppins-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-Bold.ttf" ), QStringLiteral( "ofl/poppins/Poppins-BoldItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-ExtraBold.ttf" ), QStringLiteral( "ofl/poppins/Poppins-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/poppins/Poppins-Black.ttf" ), QStringLiteral( "ofl/poppins/Poppins-BlackItalic.ttf" ) }, QStringLiteral( "ofl/poppins/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Port Lligat Sans" ), { QStringLiteral( "ofl/portlligatsans/PortLligatSans-Regular.ttf" ) }, QStringLiteral( "ofl/portlligatsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Port Lligat Slab" ), { QStringLiteral( "ofl/portlligatslab/PortLligatSlab-Regular.ttf" ) }, QStringLiteral( "ofl/portlligatslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Potta One" ), { QStringLiteral( "ofl/pottaone/PottaOne-Regular.ttf" ) }, QStringLiteral( "ofl/pottaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pragati Narrow" ), { QStringLiteral( "ofl/pragatinarrow/PragatiNarrow-Regular.ttf" ), QStringLiteral( "ofl/pragatinarrow/PragatiNarrow-Bold.ttf" ) }, QStringLiteral( "ofl/pragatinarrow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Praise" ), { QStringLiteral( "ofl/praise/Praise-Regular.ttf" ) }, QStringLiteral( "ofl/praise/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Preahvihear" ), { QStringLiteral( "ofl/preahvihear/Preahvihear-Regular.ttf" ) }, QStringLiteral( "ofl/preahvihear/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Press Start 2P" ), { QStringLiteral( "ofl/pressstart2p/PressStart2P-Regular.ttf" ) }, QStringLiteral( "ofl/pressstart2p/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Pridi" ), { QStringLiteral( "ofl/pridi/Pridi-ExtraLight.ttf" ), QStringLiteral( "ofl/pridi/Pridi-Light.ttf" ), QStringLiteral( "ofl/pridi/Pridi-Regular.ttf" ), QStringLiteral( "ofl/pridi/Pridi-Medium.ttf" ), QStringLiteral( "ofl/pridi/Pridi-SemiBold.ttf" ), QStringLiteral( "ofl/pridi/Pridi-Bold.ttf" ) }, QStringLiteral( "ofl/pridi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Princess Sofia" ), { QStringLiteral( "ofl/princesssofia/PrincessSofia-Regular.ttf" ) }, QStringLiteral( "ofl/princesssofia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Prociono" ), { QStringLiteral( "ofl/prociono/Prociono-Regular.ttf" ) }, QStringLiteral( "ofl/prociono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Prompt" ), { QStringLiteral( "ofl/prompt/Prompt-Thin.ttf" ), QStringLiteral( "ofl/prompt/Prompt-ThinItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-ExtraLight.ttf" ), QStringLiteral( "ofl/prompt/Prompt-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-Light.ttf" ), QStringLiteral( "ofl/prompt/Prompt-LightItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-Regular.ttf" ), QStringLiteral( "ofl/prompt/Prompt-Italic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-Medium.ttf" ), QStringLiteral( "ofl/prompt/Prompt-MediumItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-SemiBold.ttf" ), QStringLiteral( "ofl/prompt/Prompt-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-Bold.ttf" ), QStringLiteral( "ofl/prompt/Prompt-BoldItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-ExtraBold.ttf" ), QStringLiteral( "ofl/prompt/Prompt-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/prompt/Prompt-Black.ttf" ), QStringLiteral( "ofl/prompt/Prompt-BlackItalic.ttf" ) }, QStringLiteral( "ofl/prompt/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Prosto One" ), { QStringLiteral( "ofl/prostoone/ProstoOne-Regular.ttf" ) }, QStringLiteral( "ofl/prostoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Proza Libre" ), { QStringLiteral( "ofl/prozalibre/ProzaLibre-Regular.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-Italic.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-Medium.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-MediumItalic.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-SemiBold.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-Bold.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-BoldItalic.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-ExtraBold.ttf" ), QStringLiteral( "ofl/prozalibre/ProzaLibre-ExtraBoldItalic.ttf" ) }, QStringLiteral( "ofl/prozalibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Public Sans" ), { QStringLiteral( "ofl/publicsans/PublicSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/publicsans/PublicSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/publicsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Puppies Play" ), { QStringLiteral( "ofl/puppiesplay/PuppiesPlay-Regular.ttf" ) }, QStringLiteral( "ofl/puppiesplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Puritan" ), { QStringLiteral( "ofl/puritan/Puritan-Regular.ttf" ), QStringLiteral( "ofl/puritan/Puritan-Italic.ttf" ), QStringLiteral( "ofl/puritan/Puritan-Bold.ttf" ), QStringLiteral( "ofl/puritan/Puritan-BoldItalic.ttf" ) }, QStringLiteral( "ofl/puritan/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Purple Purse" ), { QStringLiteral( "ofl/purplepurse/PurplePurse-Regular.ttf" ) }, QStringLiteral( "ofl/purplepurse/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Qahiri" ), { QStringLiteral( "ofl/qahiri/Qahiri-Regular.ttf" ) }, QStringLiteral( "ofl/qahiri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Quando" ), { QStringLiteral( "ofl/quando/Quando-Regular.ttf" ) }, QStringLiteral( "ofl/quando/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Quantico" ), { QStringLiteral( "ofl/quantico/Quantico-Regular.ttf" ), QStringLiteral( "ofl/quantico/Quantico-Italic.ttf" ), QStringLiteral( "ofl/quantico/Quantico-Bold.ttf" ), QStringLiteral( "ofl/quantico/Quantico-BoldItalic.ttf" ) }, QStringLiteral( "ofl/quantico/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Quattrocento" ), { QStringLiteral( "ofl/quattrocento/Quattrocento-Regular.ttf" ), QStringLiteral( "ofl/quattrocento/Quattrocento-Bold.ttf" ) }, QStringLiteral( "ofl/quattrocento/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Quattrocento Sans" ), { QStringLiteral( "ofl/quattrocentosans/QuattrocentoSans-Regular.ttf" ), QStringLiteral( "ofl/quattrocentosans/QuattrocentoSans-Italic.ttf" ), QStringLiteral( "ofl/quattrocentosans/QuattrocentoSans-Bold.ttf" ), QStringLiteral( "ofl/quattrocentosans/QuattrocentoSans-BoldItalic.ttf" ) }, QStringLiteral( "ofl/quattrocentosans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Questrial" ), { QStringLiteral( "ofl/questrial/Questrial-Regular.ttf" ) }, QStringLiteral( "ofl/questrial/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Quicksand" ), { QStringLiteral( "ofl/quicksand/Quicksand%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/quicksand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Quintessential" ), { QStringLiteral( "ofl/quintessential/Quintessential-Regular.ttf" ) }, QStringLiteral( "ofl/quintessential/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Qwigley" ), { QStringLiteral( "ofl/qwigley/Qwigley-Regular.ttf" ) }, QStringLiteral( "ofl/qwigley/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Qwitcher Grypen" ), { QStringLiteral( "ofl/qwitchergrypen/QwitcherGrypen-Regular.ttf" ), QStringLiteral( "ofl/qwitchergrypen/QwitcherGrypen-Bold.ttf" ) }, QStringLiteral( "ofl/qwitchergrypen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "REM" ), { QStringLiteral( "ofl/rem/REM%5Bwght%5D.ttf" ), QStringLiteral( "ofl/rem/REM-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/rem/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Racing Sans One" ), { QStringLiteral( "ofl/racingsansone/RacingSansOne-Regular.ttf" ) }, QStringLiteral( "ofl/racingsansone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Radio Canada" ), { QStringLiteral( "ofl/radiocanada/RadioCanada%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/radiocanada/RadioCanada-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/radiocanada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Radley" ), { QStringLiteral( "ofl/radley/Radley-Regular.ttf" ), QStringLiteral( "ofl/radley/Radley-Italic.ttf" ) }, QStringLiteral( "ofl/radley/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rajdhani" ), { QStringLiteral( "ofl/rajdhani/Rajdhani-Light.ttf" ), QStringLiteral( "ofl/rajdhani/Rajdhani-Regular.ttf" ), QStringLiteral( "ofl/rajdhani/Rajdhani-Medium.ttf" ), QStringLiteral( "ofl/rajdhani/Rajdhani-SemiBold.ttf" ), QStringLiteral( "ofl/rajdhani/Rajdhani-Bold.ttf" ) }, QStringLiteral( "ofl/rajdhani/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rakkas" ), { QStringLiteral( "ofl/rakkas/Rakkas-Regular.ttf" ) }, QStringLiteral( "ofl/rakkas/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Raleway" ), { QStringLiteral( "ofl/raleway/Raleway%5Bwght%5D.ttf" ), QStringLiteral( "ofl/raleway/Raleway-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/raleway/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Raleway Dots" ), { QStringLiteral( "ofl/ralewaydots/RalewayDots-Regular.ttf" ) }, QStringLiteral( "ofl/ralewaydots/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ramabhadra" ), { QStringLiteral( "ofl/ramabhadra/Ramabhadra-Regular.ttf" ) }, QStringLiteral( "ofl/ramabhadra/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ramaraja" ), { QStringLiteral( "ofl/ramaraja/Ramaraja-Regular.ttf" ) }, QStringLiteral( "ofl/ramaraja/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rambla" ), { QStringLiteral( "ofl/rambla/Rambla-Regular.ttf" ), QStringLiteral( "ofl/rambla/Rambla-Italic.ttf" ), QStringLiteral( "ofl/rambla/Rambla-Bold.ttf" ), QStringLiteral( "ofl/rambla/Rambla-BoldItalic.ttf" ) }, QStringLiteral( "ofl/rambla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rammetto One" ), { QStringLiteral( "ofl/rammettoone/RammettoOne-Regular.ttf" ) }, QStringLiteral( "ofl/rammettoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rampart One" ), { QStringLiteral( "ofl/rampartone/RampartOne-Regular.ttf" ) }, QStringLiteral( "ofl/rampartone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ranchers" ), { QStringLiteral( "ofl/ranchers/Ranchers-Regular.ttf" ) }, QStringLiteral( "ofl/ranchers/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rancho" ), { QStringLiteral( "apache/rancho/Rancho-Regular.ttf" ) }, QStringLiteral( "apache/rancho/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ranga" ), { QStringLiteral( "ofl/ranga/Ranga-Regular.ttf" ), QStringLiteral( "ofl/ranga/Ranga-Bold.ttf" ) }, QStringLiteral( "ofl/ranga/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rasa" ), { QStringLiteral( "ofl/rasa/Rasa%5Bwght%5D.ttf" ), QStringLiteral( "ofl/rasa/Rasa-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/rasa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rationale" ), { QStringLiteral( "ofl/rationale/Rationale-Regular.ttf" ) }, QStringLiteral( "ofl/rationale/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ravi Prakash" ), { QStringLiteral( "ofl/raviprakash/RaviPrakash-Regular.ttf" ) }, QStringLiteral( "ofl/raviprakash/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Readex Pro" ), { QStringLiteral( "ofl/readexpro/ReadexPro%5BHEXP,wght%5D.ttf" ) }, QStringLiteral( "ofl/readexpro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Recursive" ), { QStringLiteral( "ofl/recursive/Recursive%5BCASL,CRSV,MONO,slnt,wght%5D.ttf" ) }, QStringLiteral( "ofl/recursive/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Red Hat Display" ), { QStringLiteral( "ofl/redhatdisplay/RedHatDisplay%5Bwght%5D.ttf" ), QStringLiteral( "ofl/redhatdisplay/RedHatDisplay-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/redhatdisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Red Hat Mono" ), { QStringLiteral( "ofl/redhatmono/RedHatMono%5Bwght%5D.ttf" ), QStringLiteral( "ofl/redhatmono/RedHatMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/redhatmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Red Hat Text" ), { QStringLiteral( "ofl/redhattext/RedHatText%5Bwght%5D.ttf" ), QStringLiteral( "ofl/redhattext/RedHatText-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/redhattext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Red Rose" ), { QStringLiteral( "ofl/redrose/RedRose%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/redrose/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Redacted" ), { QStringLiteral( "ofl/redacted/Redacted-Regular.ttf" ) }, QStringLiteral( "ofl/redacted/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Redacted Script" ), { QStringLiteral( "ofl/redactedscript/RedactedScript-Light.ttf" ), QStringLiteral( "ofl/redactedscript/RedactedScript-Regular.ttf" ), QStringLiteral( "ofl/redactedscript/RedactedScript-Bold.ttf" ) }, QStringLiteral( "ofl/redactedscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Redressed" ), { QStringLiteral( "apache/redressed/Redressed-Regular.ttf" ) }, QStringLiteral( "apache/redressed/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Reem Kufi" ), { QStringLiteral( "ofl/reemkufi/ReemKufi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/reemkufi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Reem Kufi Fun" ), { QStringLiteral( "ofl/reemkufifun/ReemKufiFun%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/reemkufifun/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Reem Kufi Ink" ), { QStringLiteral( "ofl/reemkufiink/ReemKufiInk-Regular.ttf" ) }, QStringLiteral( "ofl/reemkufiink/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Reenie Beanie" ), { QStringLiteral( "ofl/reeniebeanie/ReenieBeanie.ttf" ) }, QStringLiteral( "ofl/reeniebeanie/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Reggae One" ), { QStringLiteral( "ofl/reggaeone/ReggaeOne-Regular.ttf" ) }, QStringLiteral( "ofl/reggaeone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Revalia" ), { QStringLiteral( "ofl/revalia/Revalia-Regular.ttf" ) }, QStringLiteral( "ofl/revalia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rhodium Libre" ), { QStringLiteral( "ofl/rhodiumlibre/RhodiumLibre-Regular.ttf" ) }, QStringLiteral( "ofl/rhodiumlibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ribeye" ), { QStringLiteral( "ofl/ribeye/Ribeye-Regular.ttf" ) }, QStringLiteral( "ofl/ribeye/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ribeye Marrow" ), { QStringLiteral( "ofl/ribeyemarrow/RibeyeMarrow-Regular.ttf" ) }, QStringLiteral( "ofl/ribeyemarrow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Righteous" ), { QStringLiteral( "ofl/righteous/Righteous-Regular.ttf" ) }, QStringLiteral( "ofl/righteous/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Risque" ), { QStringLiteral( "ofl/risque/Risque-Regular.ttf" ) }, QStringLiteral( "ofl/risque/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Road Rage" ), { QStringLiteral( "ofl/roadrage/RoadRage-Regular.ttf" ) }, QStringLiteral( "ofl/roadrage/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Roboto" ), { QStringLiteral( "ofl/roboto/Roboto%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/roboto/Roboto-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/roboto/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Roboto Condensed" ), { QStringLiteral( "ofl/robotocondensed/RobotoCondensed%5Bwght%5D.ttf" ), QStringLiteral( "ofl/robotocondensed/RobotoCondensed-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/robotocondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Roboto Flex" ), { QStringLiteral( "ofl/robotoflex/RobotoFlex%5BGRAD,XOPQ,XTRA,YOPQ,YTAS,YTDE,YTFI,YTLC,YTUC,opsz,slnt,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/robotoflex/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Roboto Mono" ), { QStringLiteral( "apache/robotomono/RobotoMono%5Bwght%5D.ttf" ), QStringLiteral( "apache/robotomono/RobotoMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "apache/robotomono/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Roboto Serif" ), { QStringLiteral( "ofl/robotoserif/RobotoSerif%5BGRAD,opsz,wdth,wght%5D.ttf" ), QStringLiteral( "ofl/robotoserif/RobotoSerif-Italic%5BGRAD,opsz,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/robotoserif/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Roboto Slab" ), { QStringLiteral( "apache/robotoslab/RobotoSlab%5Bwght%5D.ttf" ) }, QStringLiteral( "apache/robotoslab/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rochester" ), { QStringLiteral( "apache/rochester/Rochester-Regular.ttf" ) }, QStringLiteral( "apache/rochester/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rock 3D" ), { QStringLiteral( "ofl/rock3d/Rock3D-Regular.ttf" ) }, QStringLiteral( "ofl/rock3d/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rock Salt" ), { QStringLiteral( "apache/rocksalt/RockSalt-Regular.ttf" ) }, QStringLiteral( "apache/rocksalt/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "RocknRoll One" ), { QStringLiteral( "ofl/rocknrollone/RocknRollOne-Regular.ttf" ) }, QStringLiteral( "ofl/rocknrollone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rokkitt" ), { QStringLiteral( "ofl/rokkitt/Rokkitt%5Bwght%5D.ttf" ), QStringLiteral( "ofl/rokkitt/Rokkitt-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/rokkitt/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Romanesco" ), { QStringLiteral( "ofl/romanesco/Romanesco-Regular.ttf" ) }, QStringLiteral( "ofl/romanesco/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ropa Sans" ), { QStringLiteral( "ofl/ropasans/RopaSans-Regular.ttf" ), QStringLiteral( "ofl/ropasans/RopaSans-Italic.ttf" ) }, QStringLiteral( "ofl/ropasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rosario" ), { QStringLiteral( "ofl/rosario/Rosario%5Bwght%5D.ttf" ), QStringLiteral( "ofl/rosario/Rosario-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/rosario/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rosarivo" ), { QStringLiteral( "ofl/rosarivo/Rosarivo-Regular.ttf" ), QStringLiteral( "ofl/rosarivo/Rosarivo-Italic.ttf" ) }, QStringLiteral( "ofl/rosarivo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rouge Script" ), { QStringLiteral( "ofl/rougescript/RougeScript-Regular.ttf" ) }, QStringLiteral( "ofl/rougescript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rowdies" ), { QStringLiteral( "ofl/rowdies/Rowdies-Light.ttf" ), QStringLiteral( "ofl/rowdies/Rowdies-Regular.ttf" ), QStringLiteral( "ofl/rowdies/Rowdies-Bold.ttf" ) }, QStringLiteral( "ofl/rowdies/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rozha One" ), { QStringLiteral( "ofl/rozhaone/RozhaOne-Regular.ttf" ) }, QStringLiteral( "ofl/rozhaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik" ), { QStringLiteral( "ofl/rubik/Rubik%5Bwght%5D.ttf" ), QStringLiteral( "ofl/rubik/Rubik-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/rubik/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik 80s Fade" ), { QStringLiteral( "ofl/rubik80sfade/Rubik80sFade-Regular.ttf" ) }, QStringLiteral( "ofl/rubik80sfade/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Beastly" ), { QStringLiteral( "ofl/rubikbeastly/RubikBeastly-Regular.ttf" ) }, QStringLiteral( "ofl/rubikbeastly/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Bubbles" ), { QStringLiteral( "ofl/rubikbubbles/RubikBubbles-Regular.ttf" ) }, QStringLiteral( "ofl/rubikbubbles/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Burned" ), { QStringLiteral( "ofl/rubikburned/RubikBurned-Regular.ttf" ) }, QStringLiteral( "ofl/rubikburned/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Dirt" ), { QStringLiteral( "ofl/rubikdirt/RubikDirt-Regular.ttf" ) }, QStringLiteral( "ofl/rubikdirt/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Distressed" ), { QStringLiteral( "ofl/rubikdistressed/RubikDistressed-Regular.ttf" ) }, QStringLiteral( "ofl/rubikdistressed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Gemstones" ), { QStringLiteral( "ofl/rubikgemstones/RubikGemstones-Regular.ttf" ) }, QStringLiteral( "ofl/rubikgemstones/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Glitch" ), { QStringLiteral( "ofl/rubikglitch/RubikGlitch-Regular.ttf" ) }, QStringLiteral( "ofl/rubikglitch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Iso" ), { QStringLiteral( "ofl/rubikiso/RubikIso-Regular.ttf" ) }, QStringLiteral( "ofl/rubikiso/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Marker Hatch" ), { QStringLiteral( "ofl/rubikmarkerhatch/RubikMarkerHatch-Regular.ttf" ) }, QStringLiteral( "ofl/rubikmarkerhatch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Maze" ), { QStringLiteral( "ofl/rubikmaze/RubikMaze-Regular.ttf" ) }, QStringLiteral( "ofl/rubikmaze/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Microbe" ), { QStringLiteral( "ofl/rubikmicrobe/RubikMicrobe-Regular.ttf" ) }, QStringLiteral( "ofl/rubikmicrobe/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Mono One" ), { QStringLiteral( "ofl/rubikmonoone/RubikMonoOne-Regular.ttf" ) }, QStringLiteral( "ofl/rubikmonoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Moonrocks" ), { QStringLiteral( "ofl/rubikmoonrocks/RubikMoonrocks-Regular.ttf" ) }, QStringLiteral( "ofl/rubikmoonrocks/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik One" ), { QStringLiteral( "ofl/rubikone/RubikOne-Regular.ttf" ) }, QStringLiteral( "ofl/rubikone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Pixels" ), { QStringLiteral( "ofl/rubikpixels/RubikPixels-Regular.ttf" ) }, QStringLiteral( "ofl/rubikpixels/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Puddles" ), { QStringLiteral( "ofl/rubikpuddles/RubikPuddles-Regular.ttf" ) }, QStringLiteral( "ofl/rubikpuddles/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Spray Paint" ), { QStringLiteral( "ofl/rubikspraypaint/RubikSprayPaint-Regular.ttf" ) }, QStringLiteral( "ofl/rubikspraypaint/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Storm" ), { QStringLiteral( "ofl/rubikstorm/RubikStorm-Regular.ttf" ) }, QStringLiteral( "ofl/rubikstorm/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Vinyl" ), { QStringLiteral( "ofl/rubikvinyl/RubikVinyl-Regular.ttf" ) }, QStringLiteral( "ofl/rubikvinyl/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rubik Wet Paint" ), { QStringLiteral( "ofl/rubikwetpaint/RubikWetPaint-Regular.ttf" ) }, QStringLiteral( "ofl/rubikwetpaint/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ruda" ), { QStringLiteral( "ofl/ruda/Ruda%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ruda/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rufina" ), { QStringLiteral( "ofl/rufina/Rufina-Regular.ttf" ), QStringLiteral( "ofl/rufina/Rufina-Bold.ttf" ) }, QStringLiteral( "ofl/rufina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ruge Boogie" ), { QStringLiteral( "ofl/rugeboogie/RugeBoogie-Regular.ttf" ) }, QStringLiteral( "ofl/rugeboogie/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ruluko" ), { QStringLiteral( "ofl/ruluko/Ruluko-Regular.ttf" ) }, QStringLiteral( "ofl/ruluko/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rum Raisin" ), { QStringLiteral( "ofl/rumraisin/RumRaisin-Regular.ttf" ) }, QStringLiteral( "ofl/rumraisin/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ruslan Display" ), { QStringLiteral( "ofl/ruslandisplay/RuslanDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/ruslandisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Russo One" ), { QStringLiteral( "ofl/russoone/RussoOne-Regular.ttf" ) }, QStringLiteral( "ofl/russoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ruthie" ), { QStringLiteral( "ofl/ruthie/Ruthie-Regular.ttf" ) }, QStringLiteral( "ofl/ruthie/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ruwudu" ), { QStringLiteral( "ofl/ruwudu/Ruwudu-Regular.ttf" ), QStringLiteral( "ofl/ruwudu/Ruwudu-Medium.ttf" ), QStringLiteral( "ofl/ruwudu/Ruwudu-SemiBold.ttf" ), QStringLiteral( "ofl/ruwudu/Ruwudu-Bold.ttf" ) }, QStringLiteral( "ofl/ruwudu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Rye" ), { QStringLiteral( "ofl/rye/Rye-Regular.ttf" ) }, QStringLiteral( "ofl/rye/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "STIX Two Text" ), { QStringLiteral( "ofl/stixtwotext/STIXTwoText%5Bwght%5D.ttf" ), QStringLiteral( "ofl/stixtwotext/STIXTwoText-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/stixtwotext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sacramento" ), { QStringLiteral( "ofl/sacramento/Sacramento-Regular.ttf" ) }, QStringLiteral( "ofl/sacramento/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sahitya" ), { QStringLiteral( "ofl/sahitya/Sahitya-Regular.ttf" ), QStringLiteral( "ofl/sahitya/Sahitya-Bold.ttf" ) }, QStringLiteral( "ofl/sahitya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sail" ), { QStringLiteral( "ofl/sail/Sail-Regular.ttf" ) }, QStringLiteral( "ofl/sail/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Saira" ), { QStringLiteral( "ofl/saira/Saira%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/saira/Saira-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/saira/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Saira Condensed" ), { QStringLiteral( "ofl/sairacondensed/SairaCondensed-Thin.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-Light.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-Regular.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-Medium.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-Bold.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/sairacondensed/SairaCondensed-Black.ttf" ) }, QStringLiteral( "ofl/sairacondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Saira Extra Condensed" ), { QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-Thin.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-Light.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-Regular.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-Medium.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-Bold.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/sairaextracondensed/SairaExtraCondensed-Black.ttf" ) }, QStringLiteral( "ofl/sairaextracondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Saira Semi Condensed" ), { QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-Thin.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-ExtraLight.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-Light.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-Regular.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-Medium.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-SemiBold.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-Bold.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-ExtraBold.ttf" ), QStringLiteral( "ofl/sairasemicondensed/SairaSemiCondensed-Black.ttf" ) }, QStringLiteral( "ofl/sairasemicondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Saira Stencil One" ), { QStringLiteral( "ofl/sairastencilone/SairaStencilOne-Regular.ttf" ) }, QStringLiteral( "ofl/sairastencilone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Salsa" ), { QStringLiteral( "ofl/salsa/Salsa-Regular.ttf" ) }, QStringLiteral( "ofl/salsa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sanchez" ), { QStringLiteral( "ofl/sanchez/Sanchez-Regular.ttf" ), QStringLiteral( "ofl/sanchez/Sanchez-Italic.ttf" ) }, QStringLiteral( "ofl/sanchez/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sancreek" ), { QStringLiteral( "ofl/sancreek/Sancreek-Regular.ttf" ) }, QStringLiteral( "ofl/sancreek/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sansita" ), { QStringLiteral( "ofl/sansita/Sansita-Regular.ttf" ), QStringLiteral( "ofl/sansita/Sansita-Italic.ttf" ), QStringLiteral( "ofl/sansita/Sansita-Bold.ttf" ), QStringLiteral( "ofl/sansita/Sansita-BoldItalic.ttf" ), QStringLiteral( "ofl/sansita/Sansita-ExtraBold.ttf" ), QStringLiteral( "ofl/sansita/Sansita-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/sansita/Sansita-Black.ttf" ), QStringLiteral( "ofl/sansita/Sansita-BlackItalic.ttf" ) }, QStringLiteral( "ofl/sansita/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sansita One" ), { QStringLiteral( "ofl/sansitaone/SansitaOne-Regular.ttf" ) }, QStringLiteral( "ofl/sansitaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sansita Swashed" ), { QStringLiteral( "ofl/sansitaswashed/SansitaSwashed%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sansitaswashed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sarabun" ), { QStringLiteral( "ofl/sarabun/Sarabun-Thin.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-ThinItalic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-ExtraLight.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-Light.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-LightItalic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-Regular.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-Italic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-Medium.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-MediumItalic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-SemiBold.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-Bold.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-BoldItalic.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-ExtraBold.ttf" ), QStringLiteral( "ofl/sarabun/Sarabun-ExtraBoldItalic.ttf" ) }, QStringLiteral( "ofl/sarabun/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sarala" ), { QStringLiteral( "ofl/sarala/Sarala-Regular.ttf" ), QStringLiteral( "ofl/sarala/Sarala-Bold.ttf" ) }, QStringLiteral( "ofl/sarala/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sarina" ), { QStringLiteral( "ofl/sarina/Sarina-Regular.ttf" ) }, QStringLiteral( "ofl/sarina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sarpanch" ), { QStringLiteral( "ofl/sarpanch/Sarpanch-Regular.ttf" ), QStringLiteral( "ofl/sarpanch/Sarpanch-Medium.ttf" ), QStringLiteral( "ofl/sarpanch/Sarpanch-SemiBold.ttf" ), QStringLiteral( "ofl/sarpanch/Sarpanch-Bold.ttf" ), QStringLiteral( "ofl/sarpanch/Sarpanch-ExtraBold.ttf" ), QStringLiteral( "ofl/sarpanch/Sarpanch-Black.ttf" ) }, QStringLiteral( "ofl/sarpanch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sassy Frass" ), { QStringLiteral( "ofl/sassyfrass/SassyFrass-Regular.ttf" ) }, QStringLiteral( "ofl/sassyfrass/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Satisfy" ), { QStringLiteral( "apache/satisfy/Satisfy-Regular.ttf" ) }, QStringLiteral( "apache/satisfy/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sawarabi Mincho" ), { QStringLiteral( "ofl/sawarabimincho/SawarabiMincho-Regular.ttf" ) }, QStringLiteral( "ofl/sawarabimincho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Scada" ), { QStringLiteral( "ofl/scada/Scada-Regular.ttf" ), QStringLiteral( "ofl/scada/Scada-Italic.ttf" ), QStringLiteral( "ofl/scada/Scada-Bold.ttf" ), QStringLiteral( "ofl/scada/Scada-BoldItalic.ttf" ) }, QStringLiteral( "ofl/scada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Scheherazade New" ), { QStringLiteral( "ofl/scheherazadenew/ScheherazadeNew-Regular.ttf" ), QStringLiteral( "ofl/scheherazadenew/ScheherazadeNew-Medium.ttf" ), QStringLiteral( "ofl/scheherazadenew/ScheherazadeNew-SemiBold.ttf" ), QStringLiteral( "ofl/scheherazadenew/ScheherazadeNew-Bold.ttf" ) }, QStringLiteral( "ofl/scheherazadenew/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Schibsted Grotesk" ), { QStringLiteral( "ofl/schibstedgrotesk/SchibstedGrotesk%5Bwght%5D.ttf" ), QStringLiteral( "ofl/schibstedgrotesk/SchibstedGrotesk-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/schibstedgrotesk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Schoolbell" ), { QStringLiteral( "apache/schoolbell/Schoolbell-Regular.ttf" ) }, QStringLiteral( "apache/schoolbell/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Scope One" ), { QStringLiteral( "ofl/scopeone/ScopeOne-Regular.ttf" ) }, QStringLiteral( "ofl/scopeone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Seaweed Script" ), { QStringLiteral( "ofl/seaweedscript/SeaweedScript-Regular.ttf" ) }, QStringLiteral( "ofl/seaweedscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Secular One" ), { QStringLiteral( "ofl/secularone/SecularOne-Regular.ttf" ) }, QStringLiteral( "ofl/secularone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sedgwick Ave" ), { QStringLiteral( "ofl/sedgwickave/SedgwickAve-Regular.ttf" ) }, QStringLiteral( "ofl/sedgwickave/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sedgwick Ave Display" ), { QStringLiteral( "ofl/sedgwickavedisplay/SedgwickAveDisplay-Regular.ttf" ) }, QStringLiteral( "ofl/sedgwickavedisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sen" ), { QStringLiteral( "ofl/sen/Sen%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Send Flowers" ), { QStringLiteral( "ofl/sendflowers/SendFlowers-Regular.ttf" ) }, QStringLiteral( "ofl/sendflowers/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sevillana" ), { QStringLiteral( "ofl/sevillana/Sevillana-Regular.ttf" ) }, QStringLiteral( "ofl/sevillana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Seymour One" ), { QStringLiteral( "ofl/seymourone/SeymourOne-Regular.ttf" ) }, QStringLiteral( "ofl/seymourone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shadows Into Light" ), { QStringLiteral( "ofl/shadowsintolight/ShadowsIntoLight.ttf" ) }, QStringLiteral( "ofl/shadowsintolight/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shadows Into Light Two" ), { QStringLiteral( "ofl/shadowsintolighttwo/ShadowsIntoLightTwo-Regular.ttf" ) }, QStringLiteral( "ofl/shadowsintolighttwo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shalimar" ), { QStringLiteral( "ofl/shalimar/Shalimar-Regular.ttf" ) }, QStringLiteral( "ofl/shalimar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shantell Sans" ), { QStringLiteral( "ofl/shantellsans/ShantellSans%5BBNCE,INFM,SPAC,wght%5D.ttf" ), QStringLiteral( "ofl/shantellsans/ShantellSans-Italic%5BBNCE,INFM,SPAC,wght%5D.ttf" ) }, QStringLiteral( "ofl/shantellsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shanti" ), { QStringLiteral( "ofl/shanti/Shanti-Regular.ttf" ) }, QStringLiteral( "ofl/shanti/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Share" ), { QStringLiteral( "ofl/share/Share-Regular.ttf" ), QStringLiteral( "ofl/share/Share-Italic.ttf" ), QStringLiteral( "ofl/share/Share-Bold.ttf" ), QStringLiteral( "ofl/share/Share-BoldItalic.ttf" ) }, QStringLiteral( "ofl/share/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Share Tech" ), { QStringLiteral( "ofl/sharetech/ShareTech-Regular.ttf" ) }, QStringLiteral( "ofl/sharetech/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Share Tech Mono" ), { QStringLiteral( "ofl/sharetechmono/ShareTechMono-Regular.ttf" ) }, QStringLiteral( "ofl/sharetechmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shippori Antique" ), { QStringLiteral( "ofl/shipporiantique/ShipporiAntique-Regular.ttf" ) }, QStringLiteral( "ofl/shipporiantique/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shippori Antique B1" ), { QStringLiteral( "ofl/shipporiantiqueb1/ShipporiAntiqueB1-Regular.ttf" ) }, QStringLiteral( "ofl/shipporiantiqueb1/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shippori Mincho" ), { QStringLiteral( "ofl/shipporimincho/ShipporiMincho-Regular.ttf" ), QStringLiteral( "ofl/shipporimincho/ShipporiMincho-Medium.ttf" ), QStringLiteral( "ofl/shipporimincho/ShipporiMincho-SemiBold.ttf" ), QStringLiteral( "ofl/shipporimincho/ShipporiMincho-Bold.ttf" ), QStringLiteral( "ofl/shipporimincho/ShipporiMincho-ExtraBold.ttf" ) }, QStringLiteral( "ofl/shipporimincho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shippori Mincho B1" ), { QStringLiteral( "ofl/shipporiminchob1/ShipporiMinchoB1-Regular.ttf" ), QStringLiteral( "ofl/shipporiminchob1/ShipporiMinchoB1-Medium.ttf" ), QStringLiteral( "ofl/shipporiminchob1/ShipporiMinchoB1-SemiBold.ttf" ), QStringLiteral( "ofl/shipporiminchob1/ShipporiMinchoB1-Bold.ttf" ), QStringLiteral( "ofl/shipporiminchob1/ShipporiMinchoB1-ExtraBold.ttf" ) }, QStringLiteral( "ofl/shipporiminchob1/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shizuru" ), { QStringLiteral( "ofl/shizuru/Shizuru-Regular.ttf" ) }, QStringLiteral( "ofl/shizuru/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shojumaru" ), { QStringLiteral( "ofl/shojumaru/Shojumaru-Regular.ttf" ) }, QStringLiteral( "ofl/shojumaru/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Short Stack" ), { QStringLiteral( "ofl/shortstack/ShortStack-Regular.ttf" ) }, QStringLiteral( "ofl/shortstack/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Shrikhand" ), { QStringLiteral( "ofl/shrikhand/Shrikhand-Regular.ttf" ) }, QStringLiteral( "ofl/shrikhand/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Siemreap" ), { QStringLiteral( "ofl/siemreap/Siemreap.ttf" ) }, QStringLiteral( "ofl/siemreap/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sigmar" ), { QStringLiteral( "ofl/sigmar/Sigmar-Regular.ttf" ) }, QStringLiteral( "ofl/sigmar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sigmar One" ), { QStringLiteral( "ofl/sigmarone/SigmarOne-Regular.ttf" ) }, QStringLiteral( "ofl/sigmarone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Signika" ), { QStringLiteral( "ofl/signika/Signika%5BGRAD,wght%5D.ttf" ) }, QStringLiteral( "ofl/signika/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Signika Negative" ), { QStringLiteral( "ofl/signikanegative/SignikaNegative%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/signikanegative/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Silkscreen" ), { QStringLiteral( "ofl/silkscreen/Silkscreen-Regular.ttf" ), QStringLiteral( "ofl/silkscreen/Silkscreen-Bold.ttf" ) }, QStringLiteral( "ofl/silkscreen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Simonetta" ), { QStringLiteral( "ofl/simonetta/Simonetta-Regular.ttf" ), QStringLiteral( "ofl/simonetta/Simonetta-Italic.ttf" ), QStringLiteral( "ofl/simonetta/Simonetta-Black.ttf" ), QStringLiteral( "ofl/simonetta/Simonetta-BlackItalic.ttf" ) }, QStringLiteral( "ofl/simonetta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Single Day" ), { QStringLiteral( "ofl/singleday/SingleDay-Regular.ttf" ) }, QStringLiteral( "ofl/singleday/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sintony" ), { QStringLiteral( "ofl/sintony/Sintony-Regular.ttf" ), QStringLiteral( "ofl/sintony/Sintony-Bold.ttf" ) }, QStringLiteral( "ofl/sintony/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sirin Stencil" ), { QStringLiteral( "ofl/sirinstencil/SirinStencil-Regular.ttf" ) }, QStringLiteral( "ofl/sirinstencil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Six Caps" ), { QStringLiteral( "ofl/sixcaps/SixCaps.ttf" ) }, QStringLiteral( "ofl/sixcaps/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Skranji" ), { QStringLiteral( "ofl/skranji/Skranji-Regular.ttf" ), QStringLiteral( "ofl/skranji/Skranji-Bold.ttf" ) }, QStringLiteral( "ofl/skranji/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Slabo 13px" ), { QStringLiteral( "ofl/slabo13px/Slabo13px-Regular.ttf" ) }, QStringLiteral( "ofl/slabo13px/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Slabo 27px" ), { QStringLiteral( "ofl/slabo27px/Slabo27px-Regular.ttf" ) }, QStringLiteral( "ofl/slabo27px/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Slackey" ), { QStringLiteral( "apache/slackey/Slackey-Regular.ttf" ) }, QStringLiteral( "apache/slackey/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Slackside One" ), { QStringLiteral( "ofl/slacksideone/SlacksideOne-Regular.ttf" ) }, QStringLiteral( "ofl/slacksideone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Smokum" ), { QStringLiteral( "apache/smokum/Smokum-Regular.ttf" ) }, QStringLiteral( "apache/smokum/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Smooch" ), { QStringLiteral( "ofl/smooch/Smooch-Regular.ttf" ) }, QStringLiteral( "ofl/smooch/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Smooch Sans" ), { QStringLiteral( "ofl/smoochsans/SmoochSans%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/smoochsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Smythe" ), { QStringLiteral( "ofl/smythe/Smythe-Regular.ttf" ) }, QStringLiteral( "ofl/smythe/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sniglet" ), { QStringLiteral( "ofl/sniglet/Sniglet-Regular.ttf" ), QStringLiteral( "ofl/sniglet/Sniglet-ExtraBold.ttf" ) }, QStringLiteral( "ofl/sniglet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Snippet" ), { QStringLiteral( "ofl/snippet/Snippet.ttf" ) }, QStringLiteral( "ofl/snippet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Snowburst One" ), { QStringLiteral( "ofl/snowburstone/SnowburstOne-Regular.ttf" ) }, QStringLiteral( "ofl/snowburstone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sofadi One" ), { QStringLiteral( "ofl/sofadione/SofadiOne-Regular.ttf" ) }, QStringLiteral( "ofl/sofadione/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sofia" ), { QStringLiteral( "ofl/sofia/Sofia-Regular.ttf" ) }, QStringLiteral( "ofl/sofia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sofia Sans" ), { QStringLiteral( "ofl/sofiasans/SofiaSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/sofiasans/SofiaSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sofiasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sofia Sans Condensed" ), { QStringLiteral( "ofl/sofiasanscondensed/SofiaSansCondensed%5Bwght%5D.ttf" ), QStringLiteral( "ofl/sofiasanscondensed/SofiaSansCondensed-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sofiasanscondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sofia Sans Extra Condensed" ), { QStringLiteral( "ofl/sofiasansextracondensed/SofiaSansExtraCondensed%5Bwght%5D.ttf" ), QStringLiteral( "ofl/sofiasansextracondensed/SofiaSansExtraCondensed-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sofiasansextracondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sofia Sans Semi Condensed" ), { QStringLiteral( "ofl/sofiasanssemicondensed/SofiaSansSemiCondensed%5Bwght%5D.ttf" ), QStringLiteral( "ofl/sofiasanssemicondensed/SofiaSansSemiCondensed-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sofiasanssemicondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Solitreo" ), { QStringLiteral( "ofl/solitreo/Solitreo-Regular.ttf" ) }, QStringLiteral( "ofl/solitreo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Solway" ), { QStringLiteral( "ofl/solway/Solway-Light.ttf" ), QStringLiteral( "ofl/solway/Solway-Regular.ttf" ), QStringLiteral( "ofl/solway/Solway-Medium.ttf" ), QStringLiteral( "ofl/solway/Solway-Bold.ttf" ), QStringLiteral( "ofl/solway/Solway-ExtraBold.ttf" ) }, QStringLiteral( "ofl/solway/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Song Myung" ), { QStringLiteral( "ofl/songmyung/SongMyung-Regular.ttf" ) }, QStringLiteral( "ofl/songmyung/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sono" ), { QStringLiteral( "ofl/sono/Sono%5BMONO,wght%5D.ttf" ) }, QStringLiteral( "ofl/sono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sonsie One" ), { QStringLiteral( "ofl/sonsieone/SonsieOne-Regular.ttf" ) }, QStringLiteral( "ofl/sonsieone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sora" ), { QStringLiteral( "ofl/sora/Sora%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sora/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sorts Mill Goudy" ), { QStringLiteral( "ofl/sortsmillgoudy/SortsMillGoudy-Regular.ttf" ), QStringLiteral( "ofl/sortsmillgoudy/SortsMillGoudy-Italic.ttf" ) }, QStringLiteral( "ofl/sortsmillgoudy/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Source Code Pro" ), { QStringLiteral( "ofl/sourcecodepro/SourceCodePro%5Bwght%5D.ttf" ), QStringLiteral( "ofl/sourcecodepro/SourceCodePro-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sourcecodepro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Source Sans 3" ), { QStringLiteral( "ofl/sourcesans3/SourceSans3%5Bwght%5D.ttf" ), QStringLiteral( "ofl/sourcesans3/SourceSans3-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sourcesans3/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Space Grotesk" ), { QStringLiteral( "ofl/spacegrotesk/SpaceGrotesk%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/spacegrotesk/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Space Mono" ), { QStringLiteral( "ofl/spacemono/SpaceMono-Regular.ttf" ), QStringLiteral( "ofl/spacemono/SpaceMono-Italic.ttf" ), QStringLiteral( "ofl/spacemono/SpaceMono-Bold.ttf" ), QStringLiteral( "ofl/spacemono/SpaceMono-BoldItalic.ttf" ) }, QStringLiteral( "ofl/spacemono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Special Elite" ), { QStringLiteral( "apache/specialelite/SpecialElite-Regular.ttf" ) }, QStringLiteral( "apache/specialelite/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Spectral" ), { QStringLiteral( "ofl/spectral/Spectral-ExtraLight.ttf" ), QStringLiteral( "ofl/spectral/Spectral-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/spectral/Spectral-Light.ttf" ), QStringLiteral( "ofl/spectral/Spectral-LightItalic.ttf" ), QStringLiteral( "ofl/spectral/Spectral-Regular.ttf" ), QStringLiteral( "ofl/spectral/Spectral-Italic.ttf" ), QStringLiteral( "ofl/spectral/Spectral-Medium.ttf" ), QStringLiteral( "ofl/spectral/Spectral-MediumItalic.ttf" ), QStringLiteral( "ofl/spectral/Spectral-SemiBold.ttf" ), QStringLiteral( "ofl/spectral/Spectral-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/spectral/Spectral-Bold.ttf" ), QStringLiteral( "ofl/spectral/Spectral-BoldItalic.ttf" ), QStringLiteral( "ofl/spectral/Spectral-ExtraBold.ttf" ), QStringLiteral( "ofl/spectral/Spectral-ExtraBoldItalic.ttf" ) }, QStringLiteral( "ofl/spectral/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Spicy Rice" ), { QStringLiteral( "ofl/spicyrice/SpicyRice-Regular.ttf" ) }, QStringLiteral( "ofl/spicyrice/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Spinnaker" ), { QStringLiteral( "ofl/spinnaker/Spinnaker-Regular.ttf" ) }, QStringLiteral( "ofl/spinnaker/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Spirax" ), { QStringLiteral( "ofl/spirax/Spirax-Regular.ttf" ) }, QStringLiteral( "ofl/spirax/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Splash" ), { QStringLiteral( "ofl/splash/Splash-Regular.ttf" ) }, QStringLiteral( "ofl/splash/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Spline Sans" ), { QStringLiteral( "ofl/splinesans/SplineSans%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/splinesans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Spline Sans Mono" ), { QStringLiteral( "ofl/splinesansmono/SplineSansMono%5Bwght%5D.ttf" ), QStringLiteral( "ofl/splinesansmono/SplineSansMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/splinesansmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Squada One" ), { QStringLiteral( "ofl/squadaone/SquadaOne-Regular.ttf" ) }, QStringLiteral( "ofl/squadaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Square Peg" ), { QStringLiteral( "ofl/squarepeg/SquarePeg-Regular.ttf" ) }, QStringLiteral( "ofl/squarepeg/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sree Krushnadevaraya" ), { QStringLiteral( "ofl/sreekrushnadevaraya/SreeKrushnadevaraya-Regular.ttf" ) }, QStringLiteral( "ofl/sreekrushnadevaraya/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sriracha" ), { QStringLiteral( "ofl/sriracha/Sriracha-Regular.ttf" ) }, QStringLiteral( "ofl/sriracha/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Srisakdi" ), { QStringLiteral( "ofl/srisakdi/Srisakdi-Regular.ttf" ), QStringLiteral( "ofl/srisakdi/Srisakdi-Bold.ttf" ) }, QStringLiteral( "ofl/srisakdi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Staatliches" ), { QStringLiteral( "ofl/staatliches/Staatliches-Regular.ttf" ) }, QStringLiteral( "ofl/staatliches/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stalemate" ), { QStringLiteral( "ofl/stalemate/Stalemate-Regular.ttf" ) }, QStringLiteral( "ofl/stalemate/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stalinist One" ), { QStringLiteral( "ofl/stalinistone/StalinistOne-Regular.ttf" ) }, QStringLiteral( "ofl/stalinistone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stardos Stencil" ), { QStringLiteral( "ofl/stardosstencil/StardosStencil-Regular.ttf" ), QStringLiteral( "ofl/stardosstencil/StardosStencil-Bold.ttf" ) }, QStringLiteral( "ofl/stardosstencil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stick" ), { QStringLiteral( "ofl/stick/Stick-Regular.ttf" ) }, QStringLiteral( "ofl/stick/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stick No Bills" ), { QStringLiteral( "ofl/sticknobills/StickNoBills%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/sticknobills/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stint Ultra Condensed" ), { QStringLiteral( "ofl/stintultracondensed/StintUltraCondensed-Regular.ttf" ) }, QStringLiteral( "ofl/stintultracondensed/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stint Ultra Expanded" ), { QStringLiteral( "ofl/stintultraexpanded/StintUltraExpanded-Regular.ttf" ) }, QStringLiteral( "ofl/stintultraexpanded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stoke" ), { QStringLiteral( "ofl/stoke/Stoke-Light.ttf" ), QStringLiteral( "ofl/stoke/Stoke-Regular.ttf" ) }, QStringLiteral( "ofl/stoke/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Strait" ), { QStringLiteral( "ofl/strait/Strait-Regular.ttf" ) }, QStringLiteral( "ofl/strait/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Style Script" ), { QStringLiteral( "ofl/stylescript/StyleScript-Regular.ttf" ) }, QStringLiteral( "ofl/stylescript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Stylish" ), { QStringLiteral( "ofl/stylish/Stylish-Regular.ttf" ) }, QStringLiteral( "ofl/stylish/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sue Ellen Francisco" ), { QStringLiteral( "ofl/sueellenfrancisco/SueEllenFrancisco-Regular.ttf" ) }, QStringLiteral( "ofl/sueellenfrancisco/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Suez One" ), { QStringLiteral( "ofl/suezone/SuezOne-Regular.ttf" ) }, QStringLiteral( "ofl/suezone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sulphur Point" ), { QStringLiteral( "ofl/sulphurpoint/SulphurPoint-Light.ttf" ), QStringLiteral( "ofl/sulphurpoint/SulphurPoint-Regular.ttf" ), QStringLiteral( "ofl/sulphurpoint/SulphurPoint-Bold.ttf" ) }, QStringLiteral( "ofl/sulphurpoint/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sumana" ), { QStringLiteral( "ofl/sumana/Sumana-Regular.ttf" ), QStringLiteral( "ofl/sumana/Sumana-Bold.ttf" ) }, QStringLiteral( "ofl/sumana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sunflower" ), { QStringLiteral( "ofl/sunflower/Sunflower-Light.ttf" ), QStringLiteral( "ofl/sunflower/Sunflower-Medium.ttf" ), QStringLiteral( "ofl/sunflower/Sunflower-Bold.ttf" ) }, QStringLiteral( "ofl/sunflower/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sunshiney" ), { QStringLiteral( "apache/sunshiney/Sunshiney-Regular.ttf" ) }, QStringLiteral( "apache/sunshiney/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Supermercado One" ), { QStringLiteral( "ofl/supermercadoone/SupermercadoOne-Regular.ttf" ) }, QStringLiteral( "ofl/supermercadoone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Sura" ), { QStringLiteral( "ofl/sura/Sura-Regular.ttf" ), QStringLiteral( "ofl/sura/Sura-Bold.ttf" ) }, QStringLiteral( "ofl/sura/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Suranna" ), { QStringLiteral( "ofl/suranna/Suranna-Regular.ttf" ) }, QStringLiteral( "ofl/suranna/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Suravaram" ), { QStringLiteral( "ofl/suravaram/Suravaram-Regular.ttf" ) }, QStringLiteral( "ofl/suravaram/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Suwannaphum" ), { QStringLiteral( "ofl/suwannaphum/Suwannaphum-Thin.ttf" ), QStringLiteral( "ofl/suwannaphum/Suwannaphum-Light.ttf" ), QStringLiteral( "ofl/suwannaphum/Suwannaphum-Regular.ttf" ), QStringLiteral( "ofl/suwannaphum/Suwannaphum-Bold.ttf" ), QStringLiteral( "ofl/suwannaphum/Suwannaphum-Black.ttf" ) }, QStringLiteral( "ofl/suwannaphum/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Swanky and Moo Moo" ), { QStringLiteral( "ofl/swankyandmoomoo/SwankyandMooMoo.ttf" ) }, QStringLiteral( "ofl/swankyandmoomoo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Syncopate" ), { QStringLiteral( "apache/syncopate/Syncopate-Regular.ttf" ), QStringLiteral( "apache/syncopate/Syncopate-Bold.ttf" ) }, QStringLiteral( "apache/syncopate/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Syne" ), { QStringLiteral( "ofl/syne/Syne%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/syne/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Syne Mono" ), { QStringLiteral( "ofl/synemono/SyneMono-Regular.ttf" ) }, QStringLiteral( "ofl/synemono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Syne Tactile" ), { QStringLiteral( "ofl/synetactile/SyneTactile-Regular.ttf" ) }, QStringLiteral( "ofl/synetactile/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tai Heritage Pro" ), { QStringLiteral( "ofl/taiheritagepro/TaiHeritagePro-Regular.ttf" ), QStringLiteral( "ofl/taiheritagepro/TaiHeritagePro-Bold.ttf" ) }, QStringLiteral( "ofl/taiheritagepro/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tajawal" ), { QStringLiteral( "ofl/tajawal/Tajawal-ExtraLight.ttf" ), QStringLiteral( "ofl/tajawal/Tajawal-Light.ttf" ), QStringLiteral( "ofl/tajawal/Tajawal-Regular.ttf" ), QStringLiteral( "ofl/tajawal/Tajawal-Medium.ttf" ), QStringLiteral( "ofl/tajawal/Tajawal-Bold.ttf" ), QStringLiteral( "ofl/tajawal/Tajawal-ExtraBold.ttf" ), QStringLiteral( "ofl/tajawal/Tajawal-Black.ttf" ) }, QStringLiteral( "ofl/tajawal/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tangerine" ), { QStringLiteral( "ofl/tangerine/Tangerine-Regular.ttf" ), QStringLiteral( "ofl/tangerine/Tangerine-Bold.ttf" ) }, QStringLiteral( "ofl/tangerine/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tapestry" ), { QStringLiteral( "ofl/tapestry/Tapestry-Regular.ttf" ) }, QStringLiteral( "ofl/tapestry/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Taprom" ), { QStringLiteral( "ofl/taprom/Taprom-Regular.ttf" ) }, QStringLiteral( "ofl/taprom/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tauri" ), { QStringLiteral( "ofl/tauri/Tauri-Regular.ttf" ) }, QStringLiteral( "ofl/tauri/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Taviraj" ), { QStringLiteral( "ofl/taviraj/Taviraj-Thin.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-ThinItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-ExtraLight.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-Light.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-LightItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-Regular.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-Italic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-Medium.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-MediumItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-SemiBold.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-Bold.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-BoldItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-ExtraBold.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-Black.ttf" ), QStringLiteral( "ofl/taviraj/Taviraj-BlackItalic.ttf" ) }, QStringLiteral( "ofl/taviraj/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Teko" ), { QStringLiteral( "ofl/teko/Teko%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/teko/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tektur" ), { QStringLiteral( "ofl/tektur/Tektur%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/tektur/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Telex" ), { QStringLiteral( "ofl/telex/Telex-Regular.ttf" ) }, QStringLiteral( "ofl/telex/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tenali Ramakrishna" ), { QStringLiteral( "ofl/tenaliramakrishna/TenaliRamakrishna-Regular.ttf" ) }, QStringLiteral( "ofl/tenaliramakrishna/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tenor Sans" ), { QStringLiteral( "ofl/tenorsans/TenorSans-Regular.ttf" ) }, QStringLiteral( "ofl/tenorsans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Text Me One" ), { QStringLiteral( "ofl/textmeone/TextMeOne-Regular.ttf" ) }, QStringLiteral( "ofl/textmeone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Texturina" ), { QStringLiteral( "ofl/texturina/Texturina%5Bopsz,wght%5D.ttf" ), QStringLiteral( "ofl/texturina/Texturina-Italic%5Bopsz,wght%5D.ttf" ) }, QStringLiteral( "ofl/texturina/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Thasadith" ), { QStringLiteral( "ofl/thasadith/Thasadith-Regular.ttf" ), QStringLiteral( "ofl/thasadith/Thasadith-Italic.ttf" ), QStringLiteral( "ofl/thasadith/Thasadith-Bold.ttf" ), QStringLiteral( "ofl/thasadith/Thasadith-BoldItalic.ttf" ) }, QStringLiteral( "ofl/thasadith/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "The Girl Next Door" ), { QStringLiteral( "ofl/thegirlnextdoor/TheGirlNextDoor.ttf" ) }, QStringLiteral( "ofl/thegirlnextdoor/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "The Nautigal" ), { QStringLiteral( "ofl/thenautigal/TheNautigal-Regular.ttf" ), QStringLiteral( "ofl/thenautigal/TheNautigal-Bold.ttf" ) }, QStringLiteral( "ofl/thenautigal/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tienne" ), { QStringLiteral( "ofl/tienne/Tienne-Regular.ttf" ), QStringLiteral( "ofl/tienne/Tienne-Bold.ttf" ), QStringLiteral( "ofl/tienne/Tienne-Black.ttf" ) }, QStringLiteral( "ofl/tienne/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tillana" ), { QStringLiteral( "ofl/tillana/Tillana-Regular.ttf" ), QStringLiteral( "ofl/tillana/Tillana-Medium.ttf" ), QStringLiteral( "ofl/tillana/Tillana-SemiBold.ttf" ), QStringLiteral( "ofl/tillana/Tillana-Bold.ttf" ), QStringLiteral( "ofl/tillana/Tillana-ExtraBold.ttf" ) }, QStringLiteral( "ofl/tillana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tilt Neon" ), { QStringLiteral( "ofl/tiltneon/TiltNeon%5BXROT,YROT%5D.ttf" ) }, QStringLiteral( "ofl/tiltneon/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tilt Prism" ), { QStringLiteral( "ofl/tiltprism/TiltPrism%5BXROT,YROT%5D.ttf" ) }, QStringLiteral( "ofl/tiltprism/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tilt Warp" ), { QStringLiteral( "ofl/tiltwarp/TiltWarp%5BXROT,YROT%5D.ttf" ) }, QStringLiteral( "ofl/tiltwarp/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Timmana" ), { QStringLiteral( "ofl/timmana/Timmana-Regular.ttf" ) }, QStringLiteral( "ofl/timmana/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tinos" ), { QStringLiteral( "apache/tinos/Tinos-Regular.ttf" ), QStringLiteral( "apache/tinos/Tinos-Italic.ttf" ), QStringLiteral( "apache/tinos/Tinos-Bold.ttf" ), QStringLiteral( "apache/tinos/Tinos-BoldItalic.ttf" ) }, QStringLiteral( "apache/tinos/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Bangla" ), { QStringLiteral( "ofl/tirobangla/TiroBangla-Regular.ttf" ), QStringLiteral( "ofl/tirobangla/TiroBangla-Italic.ttf" ) }, QStringLiteral( "ofl/tirobangla/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Devanagari Hindi" ), { QStringLiteral( "ofl/tirodevanagarihindi/TiroDevanagariHindi-Regular.ttf" ), QStringLiteral( "ofl/tirodevanagarihindi/TiroDevanagariHindi-Italic.ttf" ) }, QStringLiteral( "ofl/tirodevanagarihindi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Devanagari Marathi" ), { QStringLiteral( "ofl/tirodevanagarimarathi/TiroDevanagariMarathi-Regular.ttf" ), QStringLiteral( "ofl/tirodevanagarimarathi/TiroDevanagariMarathi-Italic.ttf" ) }, QStringLiteral( "ofl/tirodevanagarimarathi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Devanagari Sanskrit" ), { QStringLiteral( "ofl/tirodevanagarisanskrit/TiroDevanagariSanskrit-Regular.ttf" ), QStringLiteral( "ofl/tirodevanagarisanskrit/TiroDevanagariSanskrit-Italic.ttf" ) }, QStringLiteral( "ofl/tirodevanagarisanskrit/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Gurmukhi" ), { QStringLiteral( "ofl/tirogurmukhi/TiroGurmukhi-Regular.ttf" ), QStringLiteral( "ofl/tirogurmukhi/TiroGurmukhi-Italic.ttf" ) }, QStringLiteral( "ofl/tirogurmukhi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Kannada" ), { QStringLiteral( "ofl/tirokannada/TiroKannada-Regular.ttf" ), QStringLiteral( "ofl/tirokannada/TiroKannada-Italic.ttf" ) }, QStringLiteral( "ofl/tirokannada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Tamil" ), { QStringLiteral( "ofl/tirotamil/TiroTamil-Regular.ttf" ), QStringLiteral( "ofl/tirotamil/TiroTamil-Italic.ttf" ) }, QStringLiteral( "ofl/tirotamil/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tiro Telugu" ), { QStringLiteral( "ofl/tirotelugu/TiroTelugu-Regular.ttf" ), QStringLiteral( "ofl/tirotelugu/TiroTelugu-Italic.ttf" ) }, QStringLiteral( "ofl/tirotelugu/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Titan One" ), { QStringLiteral( "ofl/titanone/TitanOne-Regular.ttf" ) }, QStringLiteral( "ofl/titanone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Titillium Web" ), { QStringLiteral( "ofl/titilliumweb/TitilliumWeb-ExtraLight.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-Light.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-LightItalic.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-Regular.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-Italic.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-SemiBold.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-Bold.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-BoldItalic.ttf" ), QStringLiteral( "ofl/titilliumweb/TitilliumWeb-Black.ttf" ) }, QStringLiteral( "ofl/titilliumweb/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tomorrow" ), { QStringLiteral( "ofl/tomorrow/Tomorrow-Thin.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-ThinItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-ExtraLight.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-Light.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-LightItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-Regular.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-Italic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-Medium.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-MediumItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-SemiBold.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-Bold.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-BoldItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-ExtraBold.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-Black.ttf" ), QStringLiteral( "ofl/tomorrow/Tomorrow-BlackItalic.ttf" ) }, QStringLiteral( "ofl/tomorrow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tourney" ), { QStringLiteral( "ofl/tourney/Tourney%5Bwdth,wght%5D.ttf" ), QStringLiteral( "ofl/tourney/Tourney-Italic%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/tourney/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Trade Winds" ), { QStringLiteral( "ofl/tradewinds/TradeWinds-Regular.ttf" ) }, QStringLiteral( "ofl/tradewinds/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Train One" ), { QStringLiteral( "ofl/trainone/TrainOne-Regular.ttf" ) }, QStringLiteral( "ofl/trainone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Trirong" ), { QStringLiteral( "ofl/trirong/Trirong-Thin.ttf" ), QStringLiteral( "ofl/trirong/Trirong-ThinItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-ExtraLight.ttf" ), QStringLiteral( "ofl/trirong/Trirong-ExtraLightItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-Light.ttf" ), QStringLiteral( "ofl/trirong/Trirong-LightItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-Regular.ttf" ), QStringLiteral( "ofl/trirong/Trirong-Italic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-Medium.ttf" ), QStringLiteral( "ofl/trirong/Trirong-MediumItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-SemiBold.ttf" ), QStringLiteral( "ofl/trirong/Trirong-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-Bold.ttf" ), QStringLiteral( "ofl/trirong/Trirong-BoldItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-ExtraBold.ttf" ), QStringLiteral( "ofl/trirong/Trirong-ExtraBoldItalic.ttf" ), QStringLiteral( "ofl/trirong/Trirong-Black.ttf" ), QStringLiteral( "ofl/trirong/Trirong-BlackItalic.ttf" ) }, QStringLiteral( "ofl/trirong/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Trispace" ), { QStringLiteral( "ofl/trispace/Trispace%5Bwdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/trispace/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Trocchi" ), { QStringLiteral( "ofl/trocchi/Trocchi-Regular.ttf" ) }, QStringLiteral( "ofl/trocchi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Trochut" ), { QStringLiteral( "ofl/trochut/Trochut-Regular.ttf" ), QStringLiteral( "ofl/trochut/Trochut-Italic.ttf" ), QStringLiteral( "ofl/trochut/Trochut-Bold.ttf" ) }, QStringLiteral( "ofl/trochut/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Truculenta" ), { QStringLiteral( "ofl/truculenta/Truculenta%5Bopsz,wdth,wght%5D.ttf" ) }, QStringLiteral( "ofl/truculenta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Trykker" ), { QStringLiteral( "ofl/trykker/Trykker-Regular.ttf" ) }, QStringLiteral( "ofl/trykker/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tsukimi Rounded" ), { QStringLiteral( "ofl/tsukimirounded/TsukimiRounded-Light.ttf" ), QStringLiteral( "ofl/tsukimirounded/TsukimiRounded-Regular.ttf" ), QStringLiteral( "ofl/tsukimirounded/TsukimiRounded-Medium.ttf" ), QStringLiteral( "ofl/tsukimirounded/TsukimiRounded-SemiBold.ttf" ), QStringLiteral( "ofl/tsukimirounded/TsukimiRounded-Bold.ttf" ) }, QStringLiteral( "ofl/tsukimirounded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Tulpen One" ), { QStringLiteral( "ofl/tulpenone/TulpenOne-Regular.ttf" ) }, QStringLiteral( "ofl/tulpenone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Turret Road" ), { QStringLiteral( "ofl/turretroad/TurretRoad-ExtraLight.ttf" ), QStringLiteral( "ofl/turretroad/TurretRoad-Light.ttf" ), QStringLiteral( "ofl/turretroad/TurretRoad-Regular.ttf" ), QStringLiteral( "ofl/turretroad/TurretRoad-Medium.ttf" ), QStringLiteral( "ofl/turretroad/TurretRoad-Bold.ttf" ), QStringLiteral( "ofl/turretroad/TurretRoad-ExtraBold.ttf" ) }, QStringLiteral( "ofl/turretroad/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Twinkle Star" ), { QStringLiteral( "ofl/twinklestar/TwinkleStar-Regular.ttf" ) }, QStringLiteral( "ofl/twinklestar/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ubuntu" ), { QStringLiteral( "ufl/ubuntu/Ubuntu-Light.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-LightItalic.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-Regular.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-Italic.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-Medium.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-MediumItalic.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-Bold.ttf" ), QStringLiteral( "ufl/ubuntu/Ubuntu-BoldItalic.ttf" ) }, QStringLiteral( "ufl/ubuntu/UFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ubuntu Condensed" ), { QStringLiteral( "ufl/ubuntucondensed/UbuntuCondensed-Regular.ttf" ) }, QStringLiteral( "ufl/ubuntucondensed/UFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ubuntu Mono" ), { QStringLiteral( "ufl/ubuntumono/UbuntuMono-Regular.ttf" ), QStringLiteral( "ufl/ubuntumono/UbuntuMono-Italic.ttf" ), QStringLiteral( "ufl/ubuntumono/UbuntuMono-Bold.ttf" ), QStringLiteral( "ufl/ubuntumono/UbuntuMono-BoldItalic.ttf" ) }, QStringLiteral( "ufl/ubuntumono/UFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Uchen" ), { QStringLiteral( "ofl/uchen/Uchen-Regular.ttf" ) }, QStringLiteral( "ofl/uchen/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ultra" ), { QStringLiteral( "apache/ultra/Ultra-Regular.ttf" ) }, QStringLiteral( "apache/ultra/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Unbounded" ), { QStringLiteral( "ofl/unbounded/Unbounded%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/unbounded/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Uncial Antiqua" ), { QStringLiteral( "ofl/uncialantiqua/UncialAntiqua-Regular.ttf" ) }, QStringLiteral( "ofl/uncialantiqua/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Underdog" ), { QStringLiteral( "ofl/underdog/Underdog-Regular.ttf" ) }, QStringLiteral( "ofl/underdog/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Unica One" ), { QStringLiteral( "ofl/unicaone/UnicaOne-Regular.ttf" ) }, QStringLiteral( "ofl/unicaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "UnifrakturCook" ), { QStringLiteral( "ofl/unifrakturcook/UnifrakturCook-Bold.ttf" ) }, QStringLiteral( "ofl/unifrakturcook/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "UnifrakturMaguntia" ), { QStringLiteral( "ofl/unifrakturmaguntia/UnifrakturMaguntia-Book.ttf" ) }, QStringLiteral( "ofl/unifrakturmaguntia/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Unkempt" ), { QStringLiteral( "apache/unkempt/Unkempt-Regular.ttf" ), QStringLiteral( "apache/unkempt/Unkempt-Bold.ttf" ) }, QStringLiteral( "apache/unkempt/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Unlock" ), { QStringLiteral( "ofl/unlock/Unlock-Regular.ttf" ) }, QStringLiteral( "ofl/unlock/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Unna" ), { QStringLiteral( "ofl/unna/Unna-Regular.ttf" ), QStringLiteral( "ofl/unna/Unna-Italic.ttf" ), QStringLiteral( "ofl/unna/Unna-Bold.ttf" ), QStringLiteral( "ofl/unna/Unna-BoldItalic.ttf" ) }, QStringLiteral( "ofl/unna/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Updock" ), { QStringLiteral( "ofl/updock/Updock-Regular.ttf" ) }, QStringLiteral( "ofl/updock/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Urbanist" ), { QStringLiteral( "ofl/urbanist/Urbanist%5Bwght%5D.ttf" ), QStringLiteral( "ofl/urbanist/Urbanist-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/urbanist/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "VT323" ), { QStringLiteral( "ofl/vt323/VT323-Regular.ttf" ) }, QStringLiteral( "ofl/vt323/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vampiro One" ), { QStringLiteral( "ofl/vampiroone/VampiroOne-Regular.ttf" ) }, QStringLiteral( "ofl/vampiroone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Varela" ), { QStringLiteral( "ofl/varela/Varela-Regular.ttf" ) }, QStringLiteral( "ofl/varela/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Varela Round" ), { QStringLiteral( "ofl/varelaround/VarelaRound-Regular.ttf" ) }, QStringLiteral( "ofl/varelaround/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Varta" ), { QStringLiteral( "ofl/varta/Varta%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/varta/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vast Shadow" ), { QStringLiteral( "ofl/vastshadow/VastShadow-Regular.ttf" ) }, QStringLiteral( "ofl/vastshadow/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vazirmatn" ), { QStringLiteral( "ofl/vazirmatn/Vazirmatn%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/vazirmatn/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vesper Libre" ), { QStringLiteral( "ofl/vesperlibre/VesperLibre-Regular.ttf" ), QStringLiteral( "ofl/vesperlibre/VesperLibre-Medium.ttf" ), QStringLiteral( "ofl/vesperlibre/VesperLibre-Bold.ttf" ), QStringLiteral( "ofl/vesperlibre/VesperLibre-Heavy.ttf" ) }, QStringLiteral( "ofl/vesperlibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Viaoda Libre" ), { QStringLiteral( "ofl/viaodalibre/ViaodaLibre-Regular.ttf" ) }, QStringLiteral( "ofl/viaodalibre/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vibes" ), { QStringLiteral( "ofl/vibes/Vibes-Regular.ttf" ) }, QStringLiteral( "ofl/vibes/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vibur" ), { QStringLiteral( "ofl/vibur/Vibur-Regular.ttf" ) }, QStringLiteral( "ofl/vibur/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Victor Mono" ), { QStringLiteral( "ofl/victormono/VictorMono%5Bwght%5D.ttf" ), QStringLiteral( "ofl/victormono/VictorMono-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/victormono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vidaloka" ), { QStringLiteral( "ofl/vidaloka/Vidaloka-Regular.ttf" ) }, QStringLiteral( "ofl/vidaloka/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Viga" ), { QStringLiteral( "ofl/viga/Viga-Regular.ttf" ) }, QStringLiteral( "ofl/viga/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vina Sans" ), { QStringLiteral( "ofl/vinasans/VinaSans-Regular.ttf" ) }, QStringLiteral( "ofl/vinasans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Voces" ), { QStringLiteral( "ofl/voces/Voces-Regular.ttf" ) }, QStringLiteral( "ofl/voces/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Volkhov" ), { QStringLiteral( "ofl/volkhov/Volkhov-Regular.ttf" ), QStringLiteral( "ofl/volkhov/Volkhov-Italic.ttf" ), QStringLiteral( "ofl/volkhov/Volkhov-Bold.ttf" ), QStringLiteral( "ofl/volkhov/Volkhov-BoldItalic.ttf" ) }, QStringLiteral( "ofl/volkhov/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vollkorn" ), { QStringLiteral( "ofl/vollkorn/Vollkorn%5Bwght%5D.ttf" ), QStringLiteral( "ofl/vollkorn/Vollkorn-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/vollkorn/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vollkorn SC" ), { QStringLiteral( "ofl/vollkornsc/VollkornSC-Regular.ttf" ), QStringLiteral( "ofl/vollkornsc/VollkornSC-SemiBold.ttf" ), QStringLiteral( "ofl/vollkornsc/VollkornSC-Bold.ttf" ), QStringLiteral( "ofl/vollkornsc/VollkornSC-Black.ttf" ) }, QStringLiteral( "ofl/vollkornsc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Voltaire" ), { QStringLiteral( "ofl/voltaire/Voltaire-Regular.ttf" ) }, QStringLiteral( "ofl/voltaire/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Vujahday Script" ), { QStringLiteral( "ofl/vujahdayscript/VujahdayScript-Regular.ttf" ) }, QStringLiteral( "ofl/vujahdayscript/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Waiting for the Sunrise" ), { QStringLiteral( "ofl/waitingforthesunrise/WaitingfortheSunrise.ttf" ) }, QStringLiteral( "ofl/waitingforthesunrise/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wallpoet" ), { QStringLiteral( "ofl/wallpoet/Wallpoet-Regular.ttf" ) }, QStringLiteral( "ofl/wallpoet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Walter Turncoat" ), { QStringLiteral( "apache/walterturncoat/WalterTurncoat-Regular.ttf" ) }, QStringLiteral( "apache/walterturncoat/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Warnes" ), { QStringLiteral( "ofl/warnes/Warnes-Regular.ttf" ) }, QStringLiteral( "ofl/warnes/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Water Brush" ), { QStringLiteral( "ofl/waterbrush/WaterBrush-Regular.ttf" ) }, QStringLiteral( "ofl/waterbrush/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Waterfall" ), { QStringLiteral( "ofl/waterfall/Waterfall-Regular.ttf" ) }, QStringLiteral( "ofl/waterfall/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wavefont" ), { QStringLiteral( "ofl/wavefont/Wavefont%5BROND,YELA,wght%5D.ttf" ) }, QStringLiteral( "ofl/wavefont/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wellfleet" ), { QStringLiteral( "ofl/wellfleet/Wellfleet-Regular.ttf" ) }, QStringLiteral( "ofl/wellfleet/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wendy One" ), { QStringLiteral( "ofl/wendyone/WendyOne-Regular.ttf" ) }, QStringLiteral( "ofl/wendyone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Whisper" ), { QStringLiteral( "ofl/whisper/Whisper-Regular.ttf" ) }, QStringLiteral( "ofl/whisper/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "WindSong" ), { QStringLiteral( "ofl/windsong/WindSong-Regular.ttf" ), QStringLiteral( "ofl/windsong/WindSong-Medium.ttf" ) }, QStringLiteral( "ofl/windsong/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wire One" ), { QStringLiteral( "ofl/wireone/WireOne-Regular.ttf" ) }, QStringLiteral( "ofl/wireone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wix Madefor Display" ), { QStringLiteral( "ofl/wixmadefordisplay/WixMadeforDisplay%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/wixmadefordisplay/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Wix Madefor Text" ), { QStringLiteral( "ofl/wixmadefortext/WixMadeforText%5Bwght%5D.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-Italic%5Bwght%5D.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-Regular.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-Italic.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-Medium.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-MediumItalic.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-SemiBold.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-Bold.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-BoldItalic.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-ExtraBold.ttf" ), QStringLiteral( "ofl/wixmadefortext/WixMadeforText-ExtraBoldItalic.ttf" ) }, QStringLiteral( "ofl/wixmadefortext/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Work Sans" ), { QStringLiteral( "ofl/worksans/WorkSans%5Bwght%5D.ttf" ), QStringLiteral( "ofl/worksans/WorkSans-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/worksans/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Xanh Mono" ), { QStringLiteral( "ofl/xanhmono/XanhMono-Regular.ttf" ), QStringLiteral( "ofl/xanhmono/XanhMono-Italic.ttf" ) }, QStringLiteral( "ofl/xanhmono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yaldevi" ), { QStringLiteral( "ofl/yaldevi/Yaldevi%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/yaldevi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yanone Kaffeesatz" ), { QStringLiteral( "ofl/yanonekaffeesatz/YanoneKaffeesatz%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/yanonekaffeesatz/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yantramanav" ), { QStringLiteral( "ofl/yantramanav/Yantramanav-Thin.ttf" ), QStringLiteral( "ofl/yantramanav/Yantramanav-Light.ttf" ), QStringLiteral( "ofl/yantramanav/Yantramanav-Regular.ttf" ), QStringLiteral( "ofl/yantramanav/Yantramanav-Medium.ttf" ), QStringLiteral( "ofl/yantramanav/Yantramanav-Bold.ttf" ), QStringLiteral( "ofl/yantramanav/Yantramanav-Black.ttf" ) }, QStringLiteral( "ofl/yantramanav/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yatra One" ), { QStringLiteral( "ofl/yatraone/YatraOne-Regular.ttf" ) }, QStringLiteral( "ofl/yatraone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yellowtail" ), { QStringLiteral( "apache/yellowtail/Yellowtail-Regular.ttf" ) }, QStringLiteral( "apache/yellowtail/LICENSE.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yeon Sung" ), { QStringLiteral( "ofl/yeonsung/YeonSung-Regular.ttf" ) }, QStringLiteral( "ofl/yeonsung/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yeseva One" ), { QStringLiteral( "ofl/yesevaone/YesevaOne-Regular.ttf" ) }, QStringLiteral( "ofl/yesevaone/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yesteryear" ), { QStringLiteral( "ofl/yesteryear/Yesteryear-Regular.ttf" ) }, QStringLiteral( "ofl/yesteryear/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yomogi" ), { QStringLiteral( "ofl/yomogi/Yomogi-Regular.ttf" ) }, QStringLiteral( "ofl/yomogi/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yrsa" ), { QStringLiteral( "ofl/yrsa/Yrsa%5Bwght%5D.ttf" ), QStringLiteral( "ofl/yrsa/Yrsa-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/yrsa/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ysabeau" ), { QStringLiteral( "ofl/ysabeau/Ysabeau%5Bwght%5D.ttf" ), QStringLiteral( "ofl/ysabeau/Ysabeau-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ysabeau/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ysabeau Infant" ), { QStringLiteral( "ofl/ysabeauinfant/YsabeauInfant%5Bwght%5D.ttf" ), QStringLiteral( "ofl/ysabeauinfant/YsabeauInfant-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ysabeauinfant/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ysabeau Office" ), { QStringLiteral( "ofl/ysabeauoffice/YsabeauOffice%5Bwght%5D.ttf" ), QStringLiteral( "ofl/ysabeauoffice/YsabeauOffice-Italic%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ysabeauoffice/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Ysabeau SC" ), { QStringLiteral( "ofl/ysabeausc/YsabeauSC%5Bwght%5D.ttf" ) }, QStringLiteral( "ofl/ysabeausc/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yuji Boku" ), { QStringLiteral( "ofl/yujiboku/YujiBoku-Regular.ttf" ) }, QStringLiteral( "ofl/yujiboku/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yuji Hentaigana Akari" ), { QStringLiteral( "ofl/yujihentaiganaakari/YujiHentaiganaAkari-Regular.ttf" ) }, QStringLiteral( "ofl/yujihentaiganaakari/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yuji Hentaigana Akebono" ), { QStringLiteral( "ofl/yujihentaiganaakebono/YujiHentaiganaAkebono-Regular.ttf" ) }, QStringLiteral( "ofl/yujihentaiganaakebono/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yuji Mai" ), { QStringLiteral( "ofl/yujimai/YujiMai-Regular.ttf" ) }, QStringLiteral( "ofl/yujimai/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yuji Syuku" ), { QStringLiteral( "ofl/yujisyuku/YujiSyuku-Regular.ttf" ) }, QStringLiteral( "ofl/yujisyuku/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Yusei Magic" ), { QStringLiteral( "ofl/yuseimagic/YuseiMagic-Regular.ttf" ) }, QStringLiteral( "ofl/yuseimagic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "ZCOOL KuaiLe" ), { QStringLiteral( "ofl/zcoolkuaile/ZCOOLKuaiLe-Regular.ttf" ) }, QStringLiteral( "ofl/zcoolkuaile/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "ZCOOL QingKe HuangYou" ), { QStringLiteral( "ofl/zcoolqingkehuangyou/ZCOOLQingKeHuangYou-Regular.ttf" ) }, QStringLiteral( "ofl/zcoolqingkehuangyou/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "ZCOOL XiaoWei" ), { QStringLiteral( "ofl/zcoolxiaowei/ZCOOLXiaoWei-Regular.ttf" ) }, QStringLiteral( "ofl/zcoolxiaowei/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Antique" ), { QStringLiteral( "ofl/zenantique/ZenAntique-Regular.ttf" ) }, QStringLiteral( "ofl/zenantique/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Antique Soft" ), { QStringLiteral( "ofl/zenantiquesoft/ZenAntiqueSoft-Regular.ttf" ) }, QStringLiteral( "ofl/zenantiquesoft/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Dots" ), { QStringLiteral( "ofl/zendots/ZenDots-Regular.ttf" ) }, QStringLiteral( "ofl/zendots/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Kaku Gothic Antique" ), { QStringLiteral( "ofl/zenkakugothicantique/ZenKakuGothicAntique-Light.ttf" ), QStringLiteral( "ofl/zenkakugothicantique/ZenKakuGothicAntique-Regular.ttf" ), QStringLiteral( "ofl/zenkakugothicantique/ZenKakuGothicAntique-Medium.ttf" ), QStringLiteral( "ofl/zenkakugothicantique/ZenKakuGothicAntique-Bold.ttf" ), QStringLiteral( "ofl/zenkakugothicantique/ZenKakuGothicAntique-Black.ttf" ) }, QStringLiteral( "ofl/zenkakugothicantique/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Kaku Gothic New" ), { QStringLiteral( "ofl/zenkakugothicnew/ZenKakuGothicNew-Light.ttf" ), QStringLiteral( "ofl/zenkakugothicnew/ZenKakuGothicNew-Regular.ttf" ), QStringLiteral( "ofl/zenkakugothicnew/ZenKakuGothicNew-Medium.ttf" ), QStringLiteral( "ofl/zenkakugothicnew/ZenKakuGothicNew-Bold.ttf" ), QStringLiteral( "ofl/zenkakugothicnew/ZenKakuGothicNew-Black.ttf" ) }, QStringLiteral( "ofl/zenkakugothicnew/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Kurenaido" ), { QStringLiteral( "ofl/zenkurenaido/ZenKurenaido-Regular.ttf" ) }, QStringLiteral( "ofl/zenkurenaido/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Loop" ), { QStringLiteral( "ofl/zenloop/ZenLoop-Regular.ttf" ), QStringLiteral( "ofl/zenloop/ZenLoop-Italic.ttf" ) }, QStringLiteral( "ofl/zenloop/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Maru Gothic" ), { QStringLiteral( "ofl/zenmarugothic/ZenMaruGothic-Light.ttf" ), QStringLiteral( "ofl/zenmarugothic/ZenMaruGothic-Regular.ttf" ), QStringLiteral( "ofl/zenmarugothic/ZenMaruGothic-Medium.ttf" ), QStringLiteral( "ofl/zenmarugothic/ZenMaruGothic-Bold.ttf" ), QStringLiteral( "ofl/zenmarugothic/ZenMaruGothic-Black.ttf" ) }, QStringLiteral( "ofl/zenmarugothic/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Old Mincho" ), { QStringLiteral( "ofl/zenoldmincho/ZenOldMincho-Regular.ttf" ), QStringLiteral( "ofl/zenoldmincho/ZenOldMincho-Medium.ttf" ), QStringLiteral( "ofl/zenoldmincho/ZenOldMincho-SemiBold.ttf" ), QStringLiteral( "ofl/zenoldmincho/ZenOldMincho-Bold.ttf" ), QStringLiteral( "ofl/zenoldmincho/ZenOldMincho-Black.ttf" ) }, QStringLiteral( "ofl/zenoldmincho/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zen Tokyo Zoo" ), { QStringLiteral( "ofl/zentokyozoo/ZenTokyoZoo-Regular.ttf" ) }, QStringLiteral( "ofl/zentokyozoo/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zeyada" ), { QStringLiteral( "ofl/zeyada/Zeyada.ttf" ) }, QStringLiteral( "ofl/zeyada/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zhi Mang Xing" ), { QStringLiteral( "ofl/zhimangxing/ZhiMangXing-Regular.ttf" ) }, QStringLiteral( "ofl/zhimangxing/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zilla Slab" ), { QStringLiteral( "ofl/zillaslab/ZillaSlab-Light.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-LightItalic.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-Regular.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-Italic.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-Medium.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-MediumItalic.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-SemiBold.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-SemiBoldItalic.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-Bold.ttf" ), QStringLiteral( "ofl/zillaslab/ZillaSlab-BoldItalic.ttf" ) }, QStringLiteral( "ofl/zillaslab/OFL.txt" ) ),
    GoogleFontDetails( QStringLiteral( "Zilla Slab Highlight" ), { QStringLiteral( "ofl/zillaslabhighlight/ZillaSlabHighlight-Regular.ttf" ), QStringLiteral( "ofl/zillaslabhighlight/ZillaSlabHighlight-Bold.ttf" ) }, QStringLiteral( "ofl/zillaslabhighlight/OFL.txt" ) ),
  };

  matchedFamily.clear();
  const QString cleanedFamily = QgsFontDownloadDetails::standardizeFamily( family );

  for ( const QgsFontDownloadDetails &candidate : sGoogleFonts )
  {
    if ( candidate.standardizedFamily() == cleanedFamily )
    {
      matchedFamily = candidate.family();
      return candidate;
    }
  }

  return QgsFontDownloadDetails();
}

QString QgsFontManager::urlForFontDownload( const QString &family, QString &matchedFamily ) const
{
  const QgsFontDownloadDetails details = detailsForFontDownload( family, matchedFamily );
  return details.isValid() ? details.fontUrls().value( 0 ) : QString();
};

void QgsFontManager::downloadAndInstallFont( const QgsFontDownloadDetails &details, const QString &identifier )
{
  if ( !details.isValid() )
    return;

  QString description;
  if ( identifier.isEmpty() )
  {
    description = tr( "Installing %1" ).arg( details.family() );
  }
  else
  {
    description = tr( "Installing %1" ).arg( identifier );
  }

  QgsFontDownloadTask *task = new QgsFontDownloadTask( description, details );
  connect( task, &QgsFontDownloadTask::taskTerminated, this, [this, task, identifier]
  {
    QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
    mPendingFontDownloads.remove( identifier );
    locker.unlock();

    emit fontDownloadErrorOccurred( QUrl( task->failedUrl() ), identifier, task->errorMessage() );
  } );

  connect( task, &QgsFontDownloadTask::taskCompleted, this, [this, task, details, identifier]
  {
    const QList<QByteArray > allFontData = task->fontData();
    QStringList allFamilies;
    QStringList allLicenseDetails;

    QString errorMessage;
    for ( int i = 0; i < allFontData.size(); ++i )
    {
      QStringList thisUrlFamilies;
      const QByteArray fontData  = allFontData[i];
      const QString contentDispositionFilename = task->contentDispositionFilenames().at( i );
      QString extension;
      if ( contentDispositionFilename.isEmpty() )
      {
        const QUrl originalUrl = details.fontUrls().value( i );
        const thread_local QRegularExpression rxExtension( QStringLiteral( "^.*\\.(\\w+?)$" ) );
        extension = rxExtension.match( originalUrl.toString() ).captured( 1 );
      }
      QString thisLicenseDetails;
      if ( !installFontsFromData( fontData, errorMessage, thisUrlFamilies, thisLicenseDetails, contentDispositionFilename, extension ) )
      {
        QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
        mPendingFontDownloads.remove( identifier );
        locker.unlock();

        emit fontDownloadErrorOccurred( details.fontUrls().value( i ), identifier, errorMessage );
        return;
      }
      else
      {
        for ( const QString &family : std::as_const( thisUrlFamilies ) )
        {
          if ( !allFamilies.contains( family ) )
            allFamilies.append( family );
        }
        if ( !thisLicenseDetails.isEmpty() && !allLicenseDetails.contains( thisLicenseDetails ) )
        {
          allLicenseDetails.append( thisLicenseDetails );
        }
      }
    }

    if ( !task->licenseData().isEmpty() && !allLicenseDetails.contains( task->licenseData() ) )
    {
      allLicenseDetails.append( task->licenseData() );
    }

    QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
    mPendingFontDownloads.remove( identifier );
    locker.unlock();

    emit fontDownloaded( allFamilies, allLicenseDetails.isEmpty() ? QString() : allLicenseDetails.join( "\n\n" ) );
  }
         );

  QgsApplication::taskManager()->addTask( task );
}

void QgsFontManager::downloadAndInstallFont( const QUrl &url, const QString &identifier )
{
  downloadAndInstallFont( QgsFontDownloadDetails( identifier, { url.toString() } ) );
}

bool QgsFontManager::installFontsFromData( const QByteArray &data, QString &errorMessage, QStringList &families, QString &licenseDetails, const QString &filename, const QString &extension )
{
  errorMessage.clear();
  families.clear();
  licenseDetails.clear();

  QTemporaryFile tempFile;
  if ( !extension.isEmpty() )
  {
    QString cleanedExtension = extension;
    if ( cleanedExtension.startsWith( '.' ) )
      cleanedExtension = cleanedExtension.mid( 1 );
    tempFile.setFileTemplate( QStringLiteral( "%1/XXXXXX.%2" ).arg( QDir::tempPath(), cleanedExtension ) );
  }
  QTemporaryDir tempDir;

  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  const QString userFontsDir = mUserFontDirectories.empty() ? ( QgsApplication::qgisSettingsDirPath() + "fonts" ) : mUserFontDirectories.at( 0 );
  locker.unlock();

  const QDir fontsDir( userFontsDir );

  if ( !tempFile.open() )
  {
    errorMessage = tr( "Could not write font data to a temporary file" );
    return false;
  }

  tempFile.write( data );
  tempFile.close();

  QString sourcePath = tempFile.fileName();

  //try to install the data directly as a font
  int id = QFontDatabase::addApplicationFontFromData( data );
  if ( id != -1 )
  {
    // successfully loaded data as a font
    const QStringList foundFamilies = QFontDatabase::applicationFontFamilies( id );
    // remove the application font, as we'll copy it to the final destination and re-add from there
    QFontDatabase::removeApplicationFont( id );

    if ( foundFamilies.empty() )
    {
      errorMessage = tr( "Could not find any families in font" );
      return false;
    }

    QgsDebugMsgLevel( QStringLiteral( "Found fonts %1" ).arg( foundFamilies.join( ',' ) ), 2 );
    families = foundFamilies;
    // guess a good name for the file, by taking the first family name from the font
    const QString family = families.at( 0 );
    const QString destPath = QgsFileUtils::uniquePath( fontsDir.filePath( filename.isEmpty() ? family : filename ) );

    if ( !QFile::copy( sourcePath, destPath ) )
    {
      errorMessage = tr( "Could not copy font to %1" ).arg( destPath );
      return false;
    }

    id = QFontDatabase::addApplicationFont( destPath );
    if ( id == -1 )
    {
      errorMessage = tr( "Could not install font from %1" ).arg( destPath );
      return false;
    }
    else
    {
      locker.changeMode( QgsReadWriteLocker::Write );
      mUserFontToFamilyMap.insert( destPath, foundFamilies );
      mUserFontToIdMap.insert( destPath, id );
    }
    return true;
  }
  else
  {
    // font install failed, but maybe it's a zip file
    QStringList files;
    if ( QgsZipUtils::unzip( tempFile.fileName(), tempDir.path(), files ) )
    {
      locker.changeMode( QgsReadWriteLocker::Write );
      for ( const QString &file : std::as_const( files ) )
      {
        const QFileInfo fi( file );
        if ( fi.fileName().compare( QLatin1String( "OFL.txt" ), Qt::CaseInsensitive ) == 0
             || fi.fileName().compare( QLatin1String( "LICENSE.txt" ), Qt::CaseInsensitive ) == 0 )
        {
          QFile licenseFile( file );
          if ( licenseFile.open( QIODevice::ReadOnly ) )
          {
            QTextStream in( &licenseFile );
            const QString license = in.readAll();
            licenseDetails.append( license );
          }
        }
        else if ( fi.suffix().compare( QLatin1String( "ttf" ), Qt::CaseInsensitive ) == 0 ||
                  fi.suffix().compare( QLatin1String( "otf" ), Qt::CaseInsensitive ) == 0 )
        {
          sourcePath = file;
          id = QFontDatabase::addApplicationFont( sourcePath );
          if ( id != -1 )
          {
            QFontDatabase::removeApplicationFont( id );
            const QString destPath = fontsDir.filePath( fi.fileName() );
            // dest path may already exist for zip files -- e.g if a single zip contains a number of font variants
            if ( !QFile::exists( destPath ) && !QFile::copy( sourcePath, destPath ) )
            {
              errorMessage = tr( "Could not copy font to %1" ).arg( destPath );
              return false;
            }
            id = QFontDatabase::addApplicationFont( destPath );
            if ( id == -1 )
            {
              errorMessage = tr( "Could not install font from %1" ).arg( destPath );
              return false;
            }
            const QStringList foundFamilies = QFontDatabase::applicationFontFamilies( id );
            mUserFontToFamilyMap.insert( destPath, foundFamilies );
            mUserFontToIdMap.insert( destPath, id );
            for ( const QString &found : foundFamilies )
            {
              if ( !families.contains( found ) )
                families << found;
            }
          }
        }
      }
      return true;
    }
  }

  errorMessage = tr( "Could not read fonts from data" );
  return false;
}

void QgsFontManager::addUserFontDirectory( const QString &directory )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  if ( mUserFontDirectories.contains( directory ) )
    return;

  locker.changeMode( QgsReadWriteLocker::Write );
  mUserFontDirectories.append( directory );
  locker.unlock();

  if ( !QFile::exists( directory ) && !QDir().mkpath( directory ) )
  {
    QgsDebugError( QStringLiteral( "Cannot create local fonts dir: %1" ).arg( directory ) );
    return;
  }

  installFontsFromDirectory( directory );
}

QMap<QString, QStringList> QgsFontManager::userFontToFamilyMap() const
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Read );
  return mUserFontToFamilyMap;
}

bool QgsFontManager::removeUserFont( const QString &path )
{
  QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
  const int id = mUserFontToIdMap.value( path, -1 );
  if ( id != -1 )
    QFontDatabase::removeApplicationFont( id );
  QFile::remove( path );
  mUserFontToIdMap.remove( path );
  mUserFontToFamilyMap.remove( path );
  return true;
}

/// @cond PRIVATE
//
// QgsFontDownloadTask
//

QgsFontDownloadTask::QgsFontDownloadTask( const QString &description, const QgsFontDownloadDetails &details )
  : QgsTask( description, QgsTask::CanCancel )
  ,  mDetails( details )
{

}

bool QgsFontDownloadTask::run()
{
  mFeedback = std::make_unique< QgsFeedback >();
  mResult = true;

  for ( const QString &url : mDetails.fontUrls() )
  {
    // TODO: We should really do this async, but I'm trying to minimize the impact of this change for backport friendliness
    QgsBlockingNetworkRequest req;
    QNetworkRequest networkRequest( url );
    QgsSetRequestInitiatorClass( networkRequest, QStringLiteral( "QgsFontDownloadTask" ) );
    switch ( req.get( networkRequest, false, mFeedback.get() ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        mFontData.append( req.reply().content() );
        mContentDispositionFilenames.append( QgsNetworkReplyContent::extractFileNameFromContentDispositionHeader( req.reply().rawHeader( "Content-Disposition" ) ) );
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        mResult = false;
        mErrorMessage = req.errorMessage();
        mFailedUrl = url;
        break;
    }

    if ( !mResult )
      break;
  }

  if ( mResult && !mDetails.licenseUrl().isEmpty() )
  {
    QgsBlockingNetworkRequest req;
    QNetworkRequest networkRequest( mDetails.licenseUrl() );
    QgsSetRequestInitiatorClass( networkRequest, QStringLiteral( "QgsFontDownloadTask" ) );
    switch ( req.get( networkRequest, false, mFeedback.get() ) )
    {
      case QgsBlockingNetworkRequest::NoError:
        mLicenseData = req.reply().content();
        break;

      case QgsBlockingNetworkRequest::NetworkError:
      case QgsBlockingNetworkRequest::TimeoutError:
      case QgsBlockingNetworkRequest::ServerExceptionError:
        mResult = false;
        mErrorMessage = req.errorMessage();
        mFailedUrl = mDetails.licenseUrl();
        break;
    }
  }

  return mResult;
}

void QgsFontDownloadTask::cancel()
{
  if ( mFeedback )
    mFeedback->cancel();
  QgsTask::cancel();
}

///@endcond PRIVATE
