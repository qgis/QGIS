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

#include "qgsapplication.h"
#include "qgsblockingnetworkrequest.h"
#include "qgsfileutils.h"
#include "qgsreadwritelocker.h"
#include "qgssetrequestinitiator_p.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"
#include "qgsziputils.h"

#include <QDir>
#include <QFontDatabase>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTemporaryDir>
#include <QTemporaryFile>

#include "moc_qgsfontmanager.cpp"

const QgsSettingsEntryStringList *QgsFontManager::settingsFontFamilyReplacements = new QgsSettingsEntryStringList( u"fontFamilyReplacements"_s, QgsSettingsTree::sTreeFonts, QStringList(), u"Automatic font family replacements"_s );

const QgsSettingsEntryBool *QgsFontManager::settingsDownloadMissingFonts = new QgsSettingsEntryBool( u"downloadMissingFonts"_s, QgsSettingsTree::sTreeFonts, true, u"Automatically download missing fonts whenever possible"_s );

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
  const thread_local QRegularExpression charsToRemove( u"[^a-z]"_s );
  const thread_local QRegularExpression styleNames( u"(?:normal|regular|light|bold|black|demi|italic|oblique|medium|thin)"_s );

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
    const thread_local QRegularExpression rxReplacement( u"(.*?):(.*)"_s );
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
    replacements << u"%1:%2"_s.arg( it.key(), it.value() );
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
      QgsDebugError( u"Cannot create local fonts dir: %1"_s.arg( dir ) );
      return;
    }

    installFontsFromDirectory( dir );
  }
}

void QgsFontManager::installFontsFromDirectory( const QString &dir )
{
  const QFileInfoList fileInfoList = QDir( dir ).entryInfoList( QStringList( u"*"_s ), QDir::Files );
  QFileInfoList::const_iterator infoIt = fileInfoList.constBegin();
  for ( ; infoIt != fileInfoList.constEnd(); ++infoIt )
  {
    const int id = QFontDatabase::addApplicationFont( infoIt->filePath() );
    if ( id == -1 )
    {
      QgsDebugError( u"The user font %1 could not be installed"_s.arg( infoIt->filePath() ) );
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
    fontUrls.append( u"https://github.com/google/fonts/raw/main/%1"_s.arg( path ) );
  }
  return QgsFontDownloadDetails(
           family,
           fontUrls,
           !licensePath.isEmpty() ? u"https://github.com/google/fonts/raw/main/%1"_s.arg( licensePath ) : QString()
         );
}

QgsFontDownloadDetails QgsFontManager::detailsForFontDownload( const QString &family, QString &matchedFamily ) const
{
  // this list is built using scripts/process_google_fonts.py
  const thread_local std::vector< QgsFontDownloadDetails > sGoogleFonts
  {
    GoogleFontDetails( u"ABeeZee"_s, { u"ofl/abeezee/ABeeZee-Regular.ttf"_s, u"ofl/abeezee/ABeeZee-Italic.ttf"_s }, u"ofl/abeezee/OFL.txt"_s ),
    GoogleFontDetails( u"ADLaM Display"_s, { u"ofl/adlamdisplay/ADLaMDisplay-Regular.ttf"_s }, u"ofl/adlamdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Abel"_s, { u"ofl/abel/Abel-Regular.ttf"_s }, u"ofl/abel/OFL.txt"_s ),
    GoogleFontDetails( u"Abhaya Libre"_s, { u"ofl/abhayalibre/AbhayaLibre-Regular.ttf"_s, u"ofl/abhayalibre/AbhayaLibre-Medium.ttf"_s, u"ofl/abhayalibre/AbhayaLibre-SemiBold.ttf"_s, u"ofl/abhayalibre/AbhayaLibre-Bold.ttf"_s, u"ofl/abhayalibre/AbhayaLibre-ExtraBold.ttf"_s }, u"ofl/abhayalibre/OFL.txt"_s ),
    GoogleFontDetails( u"Aboreto"_s, { u"ofl/aboreto/Aboreto-Regular.ttf"_s }, u"ofl/aboreto/OFL.txt"_s ),
    GoogleFontDetails( u"Abril Fatface"_s, { u"ofl/abrilfatface/AbrilFatface-Regular.ttf"_s }, u"ofl/abrilfatface/OFL.txt"_s ),
    GoogleFontDetails( u"Abyssinica SIL"_s, { u"ofl/abyssinicasil/AbyssinicaSIL-Regular.ttf"_s }, u"ofl/abyssinicasil/OFL.txt"_s ),
    GoogleFontDetails( u"Aclonica"_s, { u"apache/aclonica/Aclonica-Regular.ttf"_s }, u"apache/aclonica/LICENSE.txt"_s ),
    GoogleFontDetails( u"Acme"_s, { u"ofl/acme/Acme-Regular.ttf"_s }, u"ofl/acme/OFL.txt"_s ),
    GoogleFontDetails( u"Actor"_s, { u"ofl/actor/Actor-Regular.ttf"_s }, u"ofl/actor/OFL.txt"_s ),
    GoogleFontDetails( u"Adamina"_s, { u"ofl/adamina/Adamina-Regular.ttf"_s }, u"ofl/adamina/OFL.txt"_s ),
    GoogleFontDetails( u"Advent Pro"_s, { u"ofl/adventpro/AdventPro%5Bwdth,wght%5D.ttf"_s, u"ofl/adventpro/AdventPro-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/adventpro/OFL.txt"_s ),
    GoogleFontDetails( u"Agdasima"_s, { u"ofl/agdasima/Agdasima-Regular.ttf"_s, u"ofl/agdasima/Agdasima-Bold.ttf"_s }, u"ofl/agdasima/OFL.txt"_s ),
    GoogleFontDetails( u"Aguafina Script"_s, { u"ofl/aguafinascript/AguafinaScript-Regular.ttf"_s }, u"ofl/aguafinascript/OFL.txt"_s ),
    GoogleFontDetails( u"Akatab"_s, { u"ofl/akatab/Akatab-Regular.ttf"_s, u"ofl/akatab/Akatab-Medium.ttf"_s, u"ofl/akatab/Akatab-SemiBold.ttf"_s, u"ofl/akatab/Akatab-Bold.ttf"_s, u"ofl/akatab/Akatab-ExtraBold.ttf"_s, u"ofl/akatab/Akatab-Black.ttf"_s }, u"ofl/akatab/OFL.txt"_s ),
    GoogleFontDetails( u"Akaya Kanadaka"_s, { u"ofl/akayakanadaka/AkayaKanadaka-Regular.ttf"_s }, u"ofl/akayakanadaka/OFL.txt"_s ),
    GoogleFontDetails( u"Akaya Telivigala"_s, { u"ofl/akayatelivigala/AkayaTelivigala-Regular.ttf"_s }, u"ofl/akayatelivigala/OFL.txt"_s ),
    GoogleFontDetails( u"Akronim"_s, { u"ofl/akronim/Akronim-Regular.ttf"_s }, u"ofl/akronim/OFL.txt"_s ),
    GoogleFontDetails( u"Akshar"_s, { u"ofl/akshar/Akshar%5Bwght%5D.ttf"_s }, u"ofl/akshar/OFL.txt"_s ),
    GoogleFontDetails( u"Aladin"_s, { u"ofl/aladin/Aladin-Regular.ttf"_s }, u"ofl/aladin/OFL.txt"_s ),
    GoogleFontDetails( u"Alata"_s, { u"ofl/alata/Alata-Regular.ttf"_s }, u"ofl/alata/OFL.txt"_s ),
    GoogleFontDetails( u"Alatsi"_s, { u"ofl/alatsi/Alatsi-Regular.ttf"_s }, u"ofl/alatsi/OFL.txt"_s ),
    GoogleFontDetails( u"Albert Sans"_s, { u"ofl/albertsans/AlbertSans%5Bwght%5D.ttf"_s, u"ofl/albertsans/AlbertSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/albertsans/OFL.txt"_s ),
    GoogleFontDetails( u"Aldrich"_s, { u"ofl/aldrich/Aldrich-Regular.ttf"_s }, u"ofl/aldrich/OFL.txt"_s ),
    GoogleFontDetails( u"Alef"_s, { u"ofl/alef/Alef-Regular.ttf"_s, u"ofl/alef/Alef-Bold.ttf"_s }, u"ofl/alef/OFL.txt"_s ),
    GoogleFontDetails( u"Alegreya"_s, { u"ofl/alegreya/Alegreya%5Bwght%5D.ttf"_s, u"ofl/alegreya/Alegreya-Italic%5Bwght%5D.ttf"_s }, u"ofl/alegreya/OFL.txt"_s ),
    GoogleFontDetails( u"Alegreya SC"_s, { u"ofl/alegreyasc/AlegreyaSC-Regular.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-Italic.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-Medium.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-MediumItalic.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-Bold.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-BoldItalic.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-ExtraBold.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-ExtraBoldItalic.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-Black.ttf"_s, u"ofl/alegreyasc/AlegreyaSC-BlackItalic.ttf"_s }, u"ofl/alegreyasc/OFL.txt"_s ),
    GoogleFontDetails( u"Alegreya Sans"_s, { u"ofl/alegreyasans/AlegreyaSans-Thin.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-ThinItalic.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-Light.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-LightItalic.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-Regular.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-Italic.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-Medium.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-MediumItalic.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-Bold.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-BoldItalic.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-ExtraBold.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-ExtraBoldItalic.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-Black.ttf"_s, u"ofl/alegreyasans/AlegreyaSans-BlackItalic.ttf"_s }, u"ofl/alegreyasans/OFL.txt"_s ),
    GoogleFontDetails( u"Alegreya Sans SC"_s, { u"ofl/alegreyasanssc/AlegreyaSansSC-Thin.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-ThinItalic.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-Light.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-LightItalic.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-Regular.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-Italic.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-Medium.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-MediumItalic.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-Bold.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-BoldItalic.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-ExtraBold.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-ExtraBoldItalic.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-Black.ttf"_s, u"ofl/alegreyasanssc/AlegreyaSansSC-BlackItalic.ttf"_s }, u"ofl/alegreyasanssc/OFL.txt"_s ),
    GoogleFontDetails( u"Aleo"_s, { u"ofl/aleo/Aleo%5Bwght%5D.ttf"_s, u"ofl/aleo/Aleo-Italic%5Bwght%5D.ttf"_s }, u"ofl/aleo/OFL.txt"_s ),
    GoogleFontDetails( u"Alex Brush"_s, { u"ofl/alexbrush/AlexBrush-Regular.ttf"_s }, u"ofl/alexbrush/OFL.txt"_s ),
    GoogleFontDetails( u"Alexandria"_s, { u"ofl/alexandria/Alexandria%5Bwght%5D.ttf"_s }, u"ofl/alexandria/OFL.txt"_s ),
    GoogleFontDetails( u"Alfa Slab One"_s, { u"ofl/alfaslabone/AlfaSlabOne-Regular.ttf"_s }, u"ofl/alfaslabone/OFL.txt"_s ),
    GoogleFontDetails( u"Alice"_s, { u"ofl/alice/Alice-Regular.ttf"_s }, u"ofl/alice/OFL.txt"_s ),
    GoogleFontDetails( u"Alike"_s, { u"ofl/alike/Alike-Regular.ttf"_s }, u"ofl/alike/OFL.txt"_s ),
    GoogleFontDetails( u"Alike Angular"_s, { u"ofl/alikeangular/AlikeAngular-Regular.ttf"_s }, u"ofl/alikeangular/OFL.txt"_s ),
    GoogleFontDetails( u"Alkalami"_s, { u"ofl/alkalami/Alkalami-Regular.ttf"_s }, u"ofl/alkalami/OFL.txt"_s ),
    GoogleFontDetails( u"Alkatra"_s, { u"ofl/alkatra/Alkatra%5Bwght%5D.ttf"_s }, u"ofl/alkatra/OFL.txt"_s ),
    GoogleFontDetails( u"Allan"_s, { u"ofl/allan/Allan-Regular.ttf"_s, u"ofl/allan/Allan-Bold.ttf"_s }, u"ofl/allan/OFL.txt"_s ),
    GoogleFontDetails( u"Allerta"_s, { u"ofl/allerta/Allerta-Regular.ttf"_s }, u"ofl/allerta/OFL.txt"_s ),
    GoogleFontDetails( u"Allerta Stencil"_s, { u"ofl/allertastencil/AllertaStencil-Regular.ttf"_s }, u"ofl/allertastencil/OFL.txt"_s ),
    GoogleFontDetails( u"Allison"_s, { u"ofl/allison/Allison-Regular.ttf"_s }, u"ofl/allison/OFL.txt"_s ),
    GoogleFontDetails( u"Allura"_s, { u"ofl/allura/Allura-Regular.ttf"_s }, u"ofl/allura/OFL.txt"_s ),
    GoogleFontDetails( u"Almarai"_s, { u"ofl/almarai/Almarai-Light.ttf"_s, u"ofl/almarai/Almarai-Regular.ttf"_s, u"ofl/almarai/Almarai-Bold.ttf"_s, u"ofl/almarai/Almarai-ExtraBold.ttf"_s }, u"ofl/almarai/OFL.txt"_s ),
    GoogleFontDetails( u"Almendra"_s, { u"ofl/almendra/Almendra-Regular.ttf"_s, u"ofl/almendra/Almendra-Italic.ttf"_s, u"ofl/almendra/Almendra-Bold.ttf"_s, u"ofl/almendra/Almendra-BoldItalic.ttf"_s }, u"ofl/almendra/OFL.txt"_s ),
    GoogleFontDetails( u"Almendra Display"_s, { u"ofl/almendradisplay/AlmendraDisplay-Regular.ttf"_s }, u"ofl/almendradisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Almendra SC"_s, { u"ofl/almendrasc/AlmendraSC-Regular.ttf"_s }, u"ofl/almendrasc/OFL.txt"_s ),
    GoogleFontDetails( u"Alumni Sans"_s, { u"ofl/alumnisans/AlumniSans%5Bwght%5D.ttf"_s, u"ofl/alumnisans/AlumniSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/alumnisans/OFL.txt"_s ),
    GoogleFontDetails( u"Alumni Sans Collegiate One"_s, { u"ofl/alumnisanscollegiateone/AlumniSansCollegiateOne-Regular.ttf"_s, u"ofl/alumnisanscollegiateone/AlumniSansCollegiateOne-Italic.ttf"_s }, u"ofl/alumnisanscollegiateone/OFL.txt"_s ),
    GoogleFontDetails( u"Alumni Sans Inline One"_s, { u"ofl/alumnisansinlineone/AlumniSansInlineOne-Regular.ttf"_s, u"ofl/alumnisansinlineone/AlumniSansInlineOne-Italic.ttf"_s }, u"ofl/alumnisansinlineone/OFL.txt"_s ),
    GoogleFontDetails( u"Alumni Sans Pinstripe"_s, { u"ofl/alumnisanspinstripe/AlumniSansPinstripe-Regular.ttf"_s, u"ofl/alumnisanspinstripe/AlumniSansPinstripe-Italic.ttf"_s }, u"ofl/alumnisanspinstripe/OFL.txt"_s ),
    GoogleFontDetails( u"Amarante"_s, { u"ofl/amarante/Amarante-Regular.ttf"_s }, u"ofl/amarante/OFL.txt"_s ),
    GoogleFontDetails( u"Amaranth"_s, { u"ofl/amaranth/Amaranth-Regular.ttf"_s, u"ofl/amaranth/Amaranth-Italic.ttf"_s, u"ofl/amaranth/Amaranth-Bold.ttf"_s, u"ofl/amaranth/Amaranth-BoldItalic.ttf"_s }, u"ofl/amaranth/OFL.txt"_s ),
    GoogleFontDetails( u"Amatic SC"_s, { u"ofl/amaticsc/AmaticSC-Regular.ttf"_s, u"ofl/amaticsc/AmaticSC-Bold.ttf"_s }, u"ofl/amaticsc/OFL.txt"_s ),
    GoogleFontDetails( u"Amethysta"_s, { u"ofl/amethysta/Amethysta-Regular.ttf"_s }, u"ofl/amethysta/OFL.txt"_s ),
    GoogleFontDetails( u"Amiko"_s, { u"ofl/amiko/Amiko-Regular.ttf"_s, u"ofl/amiko/Amiko-SemiBold.ttf"_s, u"ofl/amiko/Amiko-Bold.ttf"_s }, u"ofl/amiko/OFL.txt"_s ),
    GoogleFontDetails( u"Amiri"_s, { u"ofl/amiri/Amiri-Regular.ttf"_s, u"ofl/amiri/Amiri-Italic.ttf"_s, u"ofl/amiri/Amiri-Bold.ttf"_s, u"ofl/amiri/Amiri-BoldItalic.ttf"_s }, u"ofl/amiri/OFL.txt"_s ),
    GoogleFontDetails( u"Amiri Quran"_s, { u"ofl/amiriquran/AmiriQuran-Regular.ttf"_s }, u"ofl/amiriquran/OFL.txt"_s ),
    GoogleFontDetails( u"Amita"_s, { u"ofl/amita/Amita-Regular.ttf"_s, u"ofl/amita/Amita-Bold.ttf"_s }, u"ofl/amita/OFL.txt"_s ),
    GoogleFontDetails( u"Anaheim"_s, { u"ofl/anaheim/Anaheim%5Bwght%5D.ttf"_s }, u"ofl/anaheim/OFL.txt"_s ),
    GoogleFontDetails( u"Andada Pro"_s, { u"ofl/andadapro/AndadaPro%5Bwght%5D.ttf"_s, u"ofl/andadapro/AndadaPro-Italic%5Bwght%5D.ttf"_s }, u"ofl/andadapro/OFL.txt"_s ),
    GoogleFontDetails( u"Andika"_s, { u"ofl/andika/Andika-Regular.ttf"_s, u"ofl/andika/Andika-Italic.ttf"_s, u"ofl/andika/Andika-Bold.ttf"_s, u"ofl/andika/Andika-BoldItalic.ttf"_s }, u"ofl/andika/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Bangla"_s, { u"ofl/anekbangla/AnekBangla%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekbangla/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Devanagari"_s, { u"ofl/anekdevanagari/AnekDevanagari%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekdevanagari/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Gujarati"_s, { u"ofl/anekgujarati/AnekGujarati%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekgujarati/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Gurmukhi"_s, { u"ofl/anekgurmukhi/AnekGurmukhi%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekgurmukhi/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Kannada"_s, { u"ofl/anekkannada/AnekKannada%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekkannada/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Latin"_s, { u"ofl/aneklatin/AnekLatin%5Bwdth,wght%5D.ttf"_s }, u"ofl/aneklatin/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Malayalam"_s, { u"ofl/anekmalayalam/AnekMalayalam%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekmalayalam/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Odia"_s, { u"ofl/anekodia/AnekOdia%5Bwdth,wght%5D.ttf"_s }, u"ofl/anekodia/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Tamil"_s, { u"ofl/anektamil/AnekTamil%5Bwdth,wght%5D.ttf"_s }, u"ofl/anektamil/OFL.txt"_s ),
    GoogleFontDetails( u"Anek Telugu"_s, { u"ofl/anektelugu/AnekTelugu%5Bwdth,wght%5D.ttf"_s }, u"ofl/anektelugu/OFL.txt"_s ),
    GoogleFontDetails( u"Angkor"_s, { u"ofl/angkor/Angkor-Regular.ttf"_s }, u"ofl/angkor/OFL.txt"_s ),
    GoogleFontDetails( u"Annie Use Your Telescope"_s, { u"ofl/annieuseyourtelescope/AnnieUseYourTelescope-Regular.ttf"_s }, u"ofl/annieuseyourtelescope/OFL.txt"_s ),
    GoogleFontDetails( u"Anonymous Pro"_s, { u"ofl/anonymouspro/AnonymousPro-Regular.ttf"_s, u"ofl/anonymouspro/AnonymousPro-Italic.ttf"_s, u"ofl/anonymouspro/AnonymousPro-Bold.ttf"_s, u"ofl/anonymouspro/AnonymousPro-BoldItalic.ttf"_s }, u"ofl/anonymouspro/OFL.txt"_s ),
    GoogleFontDetails( u"Antic"_s, { u"ofl/antic/Antic-Regular.ttf"_s }, u"ofl/antic/OFL.txt"_s ),
    GoogleFontDetails( u"Antic Didone"_s, { u"ofl/anticdidone/AnticDidone-Regular.ttf"_s }, u"ofl/anticdidone/OFL.txt"_s ),
    GoogleFontDetails( u"Antic Slab"_s, { u"ofl/anticslab/AnticSlab-Regular.ttf"_s }, u"ofl/anticslab/OFL.txt"_s ),
    GoogleFontDetails( u"Anton"_s, { u"ofl/anton/Anton-Regular.ttf"_s }, u"ofl/anton/OFL.txt"_s ),
    GoogleFontDetails( u"Antonio"_s, { u"ofl/antonio/Antonio%5Bwght%5D.ttf"_s }, u"ofl/antonio/OFL.txt"_s ),
    GoogleFontDetails( u"Anuphan"_s, { u"ofl/anuphan/Anuphan%5Bwght%5D.ttf"_s }, u"ofl/anuphan/OFL.txt"_s ),
    GoogleFontDetails( u"Anybody"_s, { u"ofl/anybody/Anybody%5Bwdth,wght%5D.ttf"_s, u"ofl/anybody/Anybody-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/anybody/OFL.txt"_s ),
    GoogleFontDetails( u"Aoboshi One"_s, { u"ofl/aoboshione/AoboshiOne-Regular.ttf"_s }, u"ofl/aoboshione/OFL.txt"_s ),
    GoogleFontDetails( u"Arapey"_s, { u"ofl/arapey/Arapey-Regular.ttf"_s, u"ofl/arapey/Arapey-Italic.ttf"_s }, u"ofl/arapey/OFL.txt"_s ),
    GoogleFontDetails( u"Arbutus"_s, { u"ofl/arbutus/Arbutus-Regular.ttf"_s }, u"ofl/arbutus/OFL.txt"_s ),
    GoogleFontDetails( u"Arbutus Slab"_s, { u"ofl/arbutusslab/ArbutusSlab-Regular.ttf"_s }, u"ofl/arbutusslab/OFL.txt"_s ),
    GoogleFontDetails( u"Architects Daughter"_s, { u"ofl/architectsdaughter/ArchitectsDaughter-Regular.ttf"_s }, u"ofl/architectsdaughter/OFL.txt"_s ),
    GoogleFontDetails( u"Archivo"_s, { u"ofl/archivo/Archivo%5Bwdth,wght%5D.ttf"_s, u"ofl/archivo/Archivo-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/archivo/OFL.txt"_s ),
    GoogleFontDetails( u"Archivo Black"_s, { u"ofl/archivoblack/ArchivoBlack-Regular.ttf"_s }, u"ofl/archivoblack/OFL.txt"_s ),
    GoogleFontDetails( u"Archivo Narrow"_s, { u"ofl/archivonarrow/ArchivoNarrow%5Bwght%5D.ttf"_s, u"ofl/archivonarrow/ArchivoNarrow-Italic%5Bwght%5D.ttf"_s }, u"ofl/archivonarrow/OFL.txt"_s ),
    GoogleFontDetails( u"Are You Serious"_s, { u"ofl/areyouserious/AreYouSerious-Regular.ttf"_s }, u"ofl/areyouserious/OFL.txt"_s ),
    GoogleFontDetails( u"Aref Ruqaa"_s, { u"ofl/arefruqaa/ArefRuqaa-Regular.ttf"_s, u"ofl/arefruqaa/ArefRuqaa-Bold.ttf"_s }, u"ofl/arefruqaa/OFL.txt"_s ),
    GoogleFontDetails( u"Aref Ruqaa Ink"_s, { u"ofl/arefruqaaink/ArefRuqaaInk-Regular.ttf"_s, u"ofl/arefruqaaink/ArefRuqaaInk-Bold.ttf"_s }, u"ofl/arefruqaaink/OFL.txt"_s ),
    GoogleFontDetails( u"Arima"_s, { u"ofl/arima/Arima%5Bwght%5D.ttf"_s }, u"ofl/arima/OFL.txt"_s ),
    GoogleFontDetails( u"Arimo"_s, { u"apache/arimo/Arimo%5Bwght%5D.ttf"_s, u"apache/arimo/Arimo-Italic%5Bwght%5D.ttf"_s }, u"apache/arimo/LICENSE.txt"_s ),
    GoogleFontDetails( u"Arizonia"_s, { u"ofl/arizonia/Arizonia-Regular.ttf"_s }, u"ofl/arizonia/OFL.txt"_s ),
    GoogleFontDetails( u"Armata"_s, { u"ofl/armata/Armata-Regular.ttf"_s }, u"ofl/armata/OFL.txt"_s ),
    GoogleFontDetails( u"Arsenal"_s, { u"ofl/arsenal/Arsenal-Regular.ttf"_s, u"ofl/arsenal/Arsenal-Italic.ttf"_s, u"ofl/arsenal/Arsenal-Bold.ttf"_s, u"ofl/arsenal/Arsenal-BoldItalic.ttf"_s }, u"ofl/arsenal/OFL.txt"_s ),
    GoogleFontDetails( u"Artifika"_s, { u"ofl/artifika/Artifika-Regular.ttf"_s }, u"ofl/artifika/OFL.txt"_s ),
    GoogleFontDetails( u"Arvo"_s, { u"ofl/arvo/Arvo-Regular.ttf"_s, u"ofl/arvo/Arvo-Italic.ttf"_s, u"ofl/arvo/Arvo-Bold.ttf"_s, u"ofl/arvo/Arvo-BoldItalic.ttf"_s }, u"ofl/arvo/OFL.txt"_s ),
    GoogleFontDetails( u"Arya"_s, { u"ofl/arya/Arya-Regular.ttf"_s, u"ofl/arya/Arya-Bold.ttf"_s }, u"ofl/arya/OFL.txt"_s ),
    GoogleFontDetails( u"Asap"_s, { u"ofl/asap/Asap%5Bwdth,wght%5D.ttf"_s, u"ofl/asap/Asap-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/asap/OFL.txt"_s ),
    GoogleFontDetails( u"Asap Condensed"_s, { u"ofl/asapcondensed/AsapCondensed-ExtraLight.ttf"_s, u"ofl/asapcondensed/AsapCondensed-ExtraLightItalic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-Light.ttf"_s, u"ofl/asapcondensed/AsapCondensed-LightItalic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-Regular.ttf"_s, u"ofl/asapcondensed/AsapCondensed-Italic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-Medium.ttf"_s, u"ofl/asapcondensed/AsapCondensed-MediumItalic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-SemiBold.ttf"_s, u"ofl/asapcondensed/AsapCondensed-SemiBoldItalic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-Bold.ttf"_s, u"ofl/asapcondensed/AsapCondensed-BoldItalic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-ExtraBold.ttf"_s, u"ofl/asapcondensed/AsapCondensed-ExtraBoldItalic.ttf"_s, u"ofl/asapcondensed/AsapCondensed-Black.ttf"_s, u"ofl/asapcondensed/AsapCondensed-BlackItalic.ttf"_s }, u"ofl/asapcondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Asar"_s, { u"ofl/asar/Asar-Regular.ttf"_s }, u"ofl/asar/OFL.txt"_s ),
    GoogleFontDetails( u"Asset"_s, { u"ofl/asset/Asset-Regular.ttf"_s }, u"ofl/asset/OFL.txt"_s ),
    GoogleFontDetails( u"Assistant"_s, { u"ofl/assistant/Assistant%5Bwght%5D.ttf"_s }, u"ofl/assistant/OFL.txt"_s ),
    GoogleFontDetails( u"Astloch"_s, { u"ofl/astloch/Astloch-Regular.ttf"_s, u"ofl/astloch/Astloch-Bold.ttf"_s }, u"ofl/astloch/OFL.txt"_s ),
    GoogleFontDetails( u"Asul"_s, { u"ofl/asul/Asul-Regular.ttf"_s, u"ofl/asul/Asul-Bold.ttf"_s }, u"ofl/asul/OFL.txt"_s ),
    GoogleFontDetails( u"Athiti"_s, { u"ofl/athiti/Athiti-ExtraLight.ttf"_s, u"ofl/athiti/Athiti-Light.ttf"_s, u"ofl/athiti/Athiti-Regular.ttf"_s, u"ofl/athiti/Athiti-Medium.ttf"_s, u"ofl/athiti/Athiti-SemiBold.ttf"_s, u"ofl/athiti/Athiti-Bold.ttf"_s }, u"ofl/athiti/OFL.txt"_s ),
    GoogleFontDetails( u"Atkinson Hyperlegible"_s, { u"ofl/atkinsonhyperlegible/AtkinsonHyperlegible-Regular.ttf"_s, u"ofl/atkinsonhyperlegible/AtkinsonHyperlegible-Italic.ttf"_s, u"ofl/atkinsonhyperlegible/AtkinsonHyperlegible-Bold.ttf"_s, u"ofl/atkinsonhyperlegible/AtkinsonHyperlegible-BoldItalic.ttf"_s }, u"ofl/atkinsonhyperlegible/OFL.txt"_s ),
    GoogleFontDetails( u"Atomic Age"_s, { u"ofl/atomicage/AtomicAge-Regular.ttf"_s }, u"ofl/atomicage/OFL.txt"_s ),
    GoogleFontDetails( u"Aubrey"_s, { u"ofl/aubrey/Aubrey-Regular.ttf"_s }, u"ofl/aubrey/OFL.txt"_s ),
    GoogleFontDetails( u"Audiowide"_s, { u"ofl/audiowide/Audiowide-Regular.ttf"_s }, u"ofl/audiowide/OFL.txt"_s ),
    GoogleFontDetails( u"Autour One"_s, { u"ofl/autourone/AutourOne-Regular.ttf"_s }, u"ofl/autourone/OFL.txt"_s ),
    GoogleFontDetails( u"Average"_s, { u"ofl/average/Average-Regular.ttf"_s }, u"ofl/average/OFL.txt"_s ),
    GoogleFontDetails( u"Average Sans"_s, { u"ofl/averagesans/AverageSans-Regular.ttf"_s }, u"ofl/averagesans/OFL.txt"_s ),
    GoogleFontDetails( u"Averia Gruesa Libre"_s, { u"ofl/averiagruesalibre/AveriaGruesaLibre-Regular.ttf"_s }, u"ofl/averiagruesalibre/OFL.txt"_s ),
    GoogleFontDetails( u"Averia Libre"_s, { u"ofl/averialibre/AveriaLibre-Light.ttf"_s, u"ofl/averialibre/AveriaLibre-LightItalic.ttf"_s, u"ofl/averialibre/AveriaLibre-Regular.ttf"_s, u"ofl/averialibre/AveriaLibre-Italic.ttf"_s, u"ofl/averialibre/AveriaLibre-Bold.ttf"_s, u"ofl/averialibre/AveriaLibre-BoldItalic.ttf"_s }, u"ofl/averialibre/OFL.txt"_s ),
    GoogleFontDetails( u"Averia Sans Libre"_s, { u"ofl/averiasanslibre/AveriaSansLibre-Light.ttf"_s, u"ofl/averiasanslibre/AveriaSansLibre-LightItalic.ttf"_s, u"ofl/averiasanslibre/AveriaSansLibre-Regular.ttf"_s, u"ofl/averiasanslibre/AveriaSansLibre-Italic.ttf"_s, u"ofl/averiasanslibre/AveriaSansLibre-Bold.ttf"_s, u"ofl/averiasanslibre/AveriaSansLibre-BoldItalic.ttf"_s }, u"ofl/averiasanslibre/OFL.txt"_s ),
    GoogleFontDetails( u"Averia Serif Libre"_s, { u"ofl/averiaseriflibre/AveriaSerifLibre-Light.ttf"_s, u"ofl/averiaseriflibre/AveriaSerifLibre-LightItalic.ttf"_s, u"ofl/averiaseriflibre/AveriaSerifLibre-Regular.ttf"_s, u"ofl/averiaseriflibre/AveriaSerifLibre-Italic.ttf"_s, u"ofl/averiaseriflibre/AveriaSerifLibre-Bold.ttf"_s, u"ofl/averiaseriflibre/AveriaSerifLibre-BoldItalic.ttf"_s }, u"ofl/averiaseriflibre/OFL.txt"_s ),
    GoogleFontDetails( u"Azeret Mono"_s, { u"ofl/azeretmono/AzeretMono%5Bwght%5D.ttf"_s, u"ofl/azeretmono/AzeretMono-Italic%5Bwght%5D.ttf"_s }, u"ofl/azeretmono/OFL.txt"_s ),
    GoogleFontDetails( u"B612"_s, { u"ofl/b612/B612-Regular.ttf"_s, u"ofl/b612/B612-Italic.ttf"_s, u"ofl/b612/B612-Bold.ttf"_s, u"ofl/b612/B612-BoldItalic.ttf"_s }, u"ofl/b612/OFL.txt"_s ),
    GoogleFontDetails( u"B612 Mono"_s, { u"ofl/b612mono/B612Mono-Regular.ttf"_s, u"ofl/b612mono/B612Mono-Italic.ttf"_s, u"ofl/b612mono/B612Mono-Bold.ttf"_s, u"ofl/b612mono/B612Mono-BoldItalic.ttf"_s }, u"ofl/b612mono/OFL.txt"_s ),
    GoogleFontDetails( u"BIZ UDGothic"_s, { u"ofl/bizudgothic/BIZUDGothic-Regular.ttf"_s, u"ofl/bizudgothic/BIZUDGothic-Bold.ttf"_s }, u"ofl/bizudgothic/OFL.txt"_s ),
    GoogleFontDetails( u"BIZ UDMincho"_s, { u"ofl/bizudmincho/BIZUDMincho-Regular.ttf"_s, u"ofl/bizudmincho/BIZUDMincho-Bold.ttf"_s }, u"ofl/bizudmincho/OFL.txt"_s ),
    GoogleFontDetails( u"BIZ UDPGothic"_s, { u"ofl/bizudpgothic/BIZUDPGothic-Regular.ttf"_s, u"ofl/bizudpgothic/BIZUDPGothic-Bold.ttf"_s }, u"ofl/bizudpgothic/OFL.txt"_s ),
    GoogleFontDetails( u"BIZ UDPMincho"_s, { u"ofl/bizudpmincho/BIZUDPMincho-Regular.ttf"_s, u"ofl/bizudpmincho/BIZUDPMincho-Bold.ttf"_s }, u"ofl/bizudpmincho/OFL.txt"_s ),
    GoogleFontDetails( u"Babylonica"_s, { u"ofl/babylonica/Babylonica-Regular.ttf"_s }, u"ofl/babylonica/OFL.txt"_s ),
    GoogleFontDetails( u"Bacasime Antique"_s, { u"ofl/bacasimeantique/BacasimeAntique-Regular.ttf"_s }, u"ofl/bacasimeantique/OFL.txt"_s ),
    GoogleFontDetails( u"Bad Script"_s, { u"ofl/badscript/BadScript-Regular.ttf"_s }, u"ofl/badscript/OFL.txt"_s ),
    GoogleFontDetails( u"Bagel Fat One"_s, { u"ofl/bagelfatone/BagelFatOne-Regular.ttf"_s }, u"ofl/bagelfatone/OFL.txt"_s ),
    GoogleFontDetails( u"Bahiana"_s, { u"ofl/bahiana/Bahiana-Regular.ttf"_s }, u"ofl/bahiana/OFL.txt"_s ),
    GoogleFontDetails( u"Bahianita"_s, { u"ofl/bahianita/Bahianita-Regular.ttf"_s }, u"ofl/bahianita/OFL.txt"_s ),
    GoogleFontDetails( u"Bai Jamjuree"_s, { u"ofl/baijamjuree/BaiJamjuree-ExtraLight.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-ExtraLightItalic.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-Light.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-LightItalic.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-Regular.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-Italic.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-Medium.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-MediumItalic.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-SemiBold.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-SemiBoldItalic.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-Bold.ttf"_s, u"ofl/baijamjuree/BaiJamjuree-BoldItalic.ttf"_s }, u"ofl/baijamjuree/OFL.txt"_s ),
    GoogleFontDetails( u"Bakbak One"_s, { u"ofl/bakbakone/BakbakOne-Regular.ttf"_s }, u"ofl/bakbakone/OFL.txt"_s ),
    GoogleFontDetails( u"Ballet"_s, { u"ofl/ballet/Ballet%5Bopsz%5D.ttf"_s }, u"ofl/ballet/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo 2"_s, { u"ofl/baloo2/Baloo2%5Bwght%5D.ttf"_s }, u"ofl/baloo2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Bhai 2"_s, { u"ofl/baloobhai2/BalooBhai2%5Bwght%5D.ttf"_s }, u"ofl/baloobhai2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Bhaijaan 2"_s, { u"ofl/baloobhaijaan2/BalooBhaijaan2%5Bwght%5D.ttf"_s }, u"ofl/baloobhaijaan2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Bhaina 2"_s, { u"ofl/baloobhaina2/BalooBhaina2%5Bwght%5D.ttf"_s }, u"ofl/baloobhaina2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Chettan 2"_s, { u"ofl/baloochettan2/BalooChettan2%5Bwght%5D.ttf"_s }, u"ofl/baloochettan2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Da 2"_s, { u"ofl/balooda2/BalooDa2%5Bwght%5D.ttf"_s }, u"ofl/balooda2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Paaji 2"_s, { u"ofl/baloopaaji2/BalooPaaji2%5Bwght%5D.ttf"_s }, u"ofl/baloopaaji2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Tamma 2"_s, { u"ofl/balootamma2/BalooTamma2%5Bwght%5D.ttf"_s }, u"ofl/balootamma2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Tammudu 2"_s, { u"ofl/balootammudu2/BalooTammudu2%5Bwght%5D.ttf"_s }, u"ofl/balootammudu2/OFL.txt"_s ),
    GoogleFontDetails( u"Baloo Thambi 2"_s, { u"ofl/baloothambi2/BalooThambi2%5Bwght%5D.ttf"_s }, u"ofl/baloothambi2/OFL.txt"_s ),
    GoogleFontDetails( u"Balsamiq Sans"_s, { u"ofl/balsamiqsans/BalsamiqSans-Regular.ttf"_s, u"ofl/balsamiqsans/BalsamiqSans-Italic.ttf"_s, u"ofl/balsamiqsans/BalsamiqSans-Bold.ttf"_s, u"ofl/balsamiqsans/BalsamiqSans-BoldItalic.ttf"_s }, u"ofl/balsamiqsans/OFL.txt"_s ),
    GoogleFontDetails( u"Balthazar"_s, { u"ofl/balthazar/Balthazar-Regular.ttf"_s }, u"ofl/balthazar/OFL.txt"_s ),
    GoogleFontDetails( u"Bangers"_s, { u"ofl/bangers/Bangers-Regular.ttf"_s }, u"ofl/bangers/OFL.txt"_s ),
    GoogleFontDetails( u"Barlow"_s, { u"ofl/barlow/Barlow-Thin.ttf"_s, u"ofl/barlow/Barlow-ThinItalic.ttf"_s, u"ofl/barlow/Barlow-ExtraLight.ttf"_s, u"ofl/barlow/Barlow-ExtraLightItalic.ttf"_s, u"ofl/barlow/Barlow-Light.ttf"_s, u"ofl/barlow/Barlow-LightItalic.ttf"_s, u"ofl/barlow/Barlow-Regular.ttf"_s, u"ofl/barlow/Barlow-Italic.ttf"_s, u"ofl/barlow/Barlow-Medium.ttf"_s, u"ofl/barlow/Barlow-MediumItalic.ttf"_s, u"ofl/barlow/Barlow-SemiBold.ttf"_s, u"ofl/barlow/Barlow-SemiBoldItalic.ttf"_s, u"ofl/barlow/Barlow-Bold.ttf"_s, u"ofl/barlow/Barlow-BoldItalic.ttf"_s, u"ofl/barlow/Barlow-ExtraBold.ttf"_s, u"ofl/barlow/Barlow-ExtraBoldItalic.ttf"_s, u"ofl/barlow/Barlow-Black.ttf"_s, u"ofl/barlow/Barlow-BlackItalic.ttf"_s }, u"ofl/barlow/OFL.txt"_s ),
    GoogleFontDetails( u"Barlow Condensed"_s, { u"ofl/barlowcondensed/BarlowCondensed-Thin.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-ThinItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-ExtraLight.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-ExtraLightItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-Light.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-LightItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-Regular.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-Italic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-Medium.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-MediumItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-SemiBold.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-SemiBoldItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-Bold.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-BoldItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-ExtraBold.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-ExtraBoldItalic.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-Black.ttf"_s, u"ofl/barlowcondensed/BarlowCondensed-BlackItalic.ttf"_s }, u"ofl/barlowcondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Barlow Semi Condensed"_s, { u"ofl/barlowsemicondensed/BarlowSemiCondensed-Thin.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-ThinItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraLight.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraLightItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-Light.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-LightItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-Regular.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-Italic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-Medium.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-MediumItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-SemiBold.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-SemiBoldItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-Bold.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-BoldItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraBold.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-ExtraBoldItalic.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-Black.ttf"_s, u"ofl/barlowsemicondensed/BarlowSemiCondensed-BlackItalic.ttf"_s }, u"ofl/barlowsemicondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Barriecito"_s, { u"ofl/barriecito/Barriecito-Regular.ttf"_s }, u"ofl/barriecito/OFL.txt"_s ),
    GoogleFontDetails( u"Barrio"_s, { u"ofl/barrio/Barrio-Regular.ttf"_s }, u"ofl/barrio/OFL.txt"_s ),
    GoogleFontDetails( u"Basic"_s, { u"ofl/basic/Basic-Regular.ttf"_s }, u"ofl/basic/OFL.txt"_s ),
    GoogleFontDetails( u"Baskervville"_s, { u"ofl/baskervville/Baskervville-Regular.ttf"_s, u"ofl/baskervville/Baskervville-Italic.ttf"_s }, u"ofl/baskervville/OFL.txt"_s ),
    GoogleFontDetails( u"Battambang"_s, { u"ofl/battambang/Battambang-Thin.ttf"_s, u"ofl/battambang/Battambang-Light.ttf"_s, u"ofl/battambang/Battambang-Regular.ttf"_s, u"ofl/battambang/Battambang-Bold.ttf"_s, u"ofl/battambang/Battambang-Black.ttf"_s }, u"ofl/battambang/OFL.txt"_s ),
    GoogleFontDetails( u"Baumans"_s, { u"ofl/baumans/Baumans-Regular.ttf"_s }, u"ofl/baumans/OFL.txt"_s ),
    GoogleFontDetails( u"Bayon"_s, { u"ofl/bayon/Bayon-Regular.ttf"_s }, u"ofl/bayon/OFL.txt"_s ),
    GoogleFontDetails( u"Be Vietnam Pro"_s, { u"ofl/bevietnampro/BeVietnamPro-Thin.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-ThinItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-ExtraLight.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-ExtraLightItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-Light.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-LightItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-Regular.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-Italic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-Medium.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-MediumItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-SemiBold.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-SemiBoldItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-Bold.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-BoldItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-ExtraBold.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-ExtraBoldItalic.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-Black.ttf"_s, u"ofl/bevietnampro/BeVietnamPro-BlackItalic.ttf"_s }, u"ofl/bevietnampro/OFL.txt"_s ),
    GoogleFontDetails( u"Beau Rivage"_s, { u"ofl/beaurivage/BeauRivage-Regular.ttf"_s }, u"ofl/beaurivage/OFL.txt"_s ),
    GoogleFontDetails( u"Bebas Neue"_s, { u"ofl/bebasneue/BebasNeue-Regular.ttf"_s }, u"ofl/bebasneue/OFL.txt"_s ),
    GoogleFontDetails( u"Belanosima"_s, { u"ofl/belanosima/Belanosima-Regular.ttf"_s, u"ofl/belanosima/Belanosima-SemiBold.ttf"_s, u"ofl/belanosima/Belanosima-Bold.ttf"_s }, u"ofl/belanosima/OFL.txt"_s ),
    GoogleFontDetails( u"Belgrano"_s, { u"ofl/belgrano/Belgrano-Regular.ttf"_s }, u"ofl/belgrano/OFL.txt"_s ),
    GoogleFontDetails( u"Bellefair"_s, { u"ofl/bellefair/Bellefair-Regular.ttf"_s }, u"ofl/bellefair/OFL.txt"_s ),
    GoogleFontDetails( u"Belleza"_s, { u"ofl/belleza/Belleza-Regular.ttf"_s }, u"ofl/belleza/OFL.txt"_s ),
    GoogleFontDetails( u"Bellota"_s, { u"ofl/bellota/Bellota-Light.ttf"_s, u"ofl/bellota/Bellota-LightItalic.ttf"_s, u"ofl/bellota/Bellota-Regular.ttf"_s, u"ofl/bellota/Bellota-Italic.ttf"_s, u"ofl/bellota/Bellota-Bold.ttf"_s, u"ofl/bellota/Bellota-BoldItalic.ttf"_s }, u"ofl/bellota/OFL.txt"_s ),
    GoogleFontDetails( u"Bellota Text"_s, { u"ofl/bellotatext/BellotaText-Light.ttf"_s, u"ofl/bellotatext/BellotaText-LightItalic.ttf"_s, u"ofl/bellotatext/BellotaText-Regular.ttf"_s, u"ofl/bellotatext/BellotaText-Italic.ttf"_s, u"ofl/bellotatext/BellotaText-Bold.ttf"_s, u"ofl/bellotatext/BellotaText-BoldItalic.ttf"_s }, u"ofl/bellotatext/OFL.txt"_s ),
    GoogleFontDetails( u"BenchNine"_s, { u"ofl/benchnine/BenchNine-Light.ttf"_s, u"ofl/benchnine/BenchNine-Regular.ttf"_s, u"ofl/benchnine/BenchNine-Bold.ttf"_s }, u"ofl/benchnine/OFL.txt"_s ),
    GoogleFontDetails( u"Benne"_s, { u"ofl/benne/Benne-Regular.ttf"_s }, u"ofl/benne/OFL.txt"_s ),
    GoogleFontDetails( u"Bentham"_s, { u"ofl/bentham/Bentham-Regular.ttf"_s }, u"ofl/bentham/OFL.txt"_s ),
    GoogleFontDetails( u"Berkshire Swash"_s, { u"ofl/berkshireswash/BerkshireSwash-Regular.ttf"_s }, u"ofl/berkshireswash/OFL.txt"_s ),
    GoogleFontDetails( u"Besley"_s, { u"ofl/besley/Besley%5Bwght%5D.ttf"_s, u"ofl/besley/Besley-Italic%5Bwght%5D.ttf"_s }, u"ofl/besley/OFL.txt"_s ),
    GoogleFontDetails( u"Beth Ellen"_s, { u"ofl/bethellen/BethEllen-Regular.ttf"_s }, u"ofl/bethellen/OFL.txt"_s ),
    GoogleFontDetails( u"Bevan"_s, { u"ofl/bevan/Bevan-Regular.ttf"_s, u"ofl/bevan/Bevan-Italic.ttf"_s }, u"ofl/bevan/OFL.txt"_s ),
    GoogleFontDetails( u"BhuTuka Expanded One"_s, { u"ofl/bhutukaexpandedone/BhuTukaExpandedOne-Regular.ttf"_s }, u"ofl/bhutukaexpandedone/OFL.txt"_s ),
    GoogleFontDetails( u"Big Shoulders Display"_s, { u"ofl/bigshouldersdisplay/BigShouldersDisplay%5Bwght%5D.ttf"_s }, u"ofl/bigshouldersdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Big Shoulders Inline Display"_s, { u"ofl/bigshouldersinlinedisplay/BigShouldersInlineDisplay%5Bwght%5D.ttf"_s }, u"ofl/bigshouldersinlinedisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Big Shoulders Inline Text"_s, { u"ofl/bigshouldersinlinetext/BigShouldersInlineText%5Bwght%5D.ttf"_s }, u"ofl/bigshouldersinlinetext/OFL.txt"_s ),
    GoogleFontDetails( u"Big Shoulders Stencil Display"_s, { u"ofl/bigshouldersstencildisplay/BigShouldersStencilDisplay%5Bwght%5D.ttf"_s }, u"ofl/bigshouldersstencildisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Big Shoulders Stencil Text"_s, { u"ofl/bigshouldersstenciltext/BigShouldersStencilText%5Bwght%5D.ttf"_s }, u"ofl/bigshouldersstenciltext/OFL.txt"_s ),
    GoogleFontDetails( u"Big Shoulders Text"_s, { u"ofl/bigshoulderstext/BigShouldersText%5Bwght%5D.ttf"_s }, u"ofl/bigshoulderstext/OFL.txt"_s ),
    GoogleFontDetails( u"Bigelow Rules"_s, { u"ofl/bigelowrules/BigelowRules-Regular.ttf"_s }, u"ofl/bigelowrules/OFL.txt"_s ),
    GoogleFontDetails( u"Bigshot One"_s, { u"ofl/bigshotone/BigshotOne-Regular.ttf"_s }, u"ofl/bigshotone/OFL.txt"_s ),
    GoogleFontDetails( u"Bilbo"_s, { u"ofl/bilbo/Bilbo-Regular.ttf"_s }, u"ofl/bilbo/OFL.txt"_s ),
    GoogleFontDetails( u"Bilbo Swash Caps"_s, { u"ofl/bilboswashcaps/BilboSwashCaps-Regular.ttf"_s }, u"ofl/bilboswashcaps/OFL.txt"_s ),
    GoogleFontDetails( u"BioRhyme"_s, { u"ofl/biorhyme/BioRhyme%5Bwdth,wght%5D.ttf"_s }, u"ofl/biorhyme/OFL.txt"_s ),
    GoogleFontDetails( u"BioRhyme Expanded"_s, { u"ofl/biorhymeexpanded/BioRhymeExpanded-ExtraLight.ttf"_s, u"ofl/biorhymeexpanded/BioRhymeExpanded-Light.ttf"_s, u"ofl/biorhymeexpanded/BioRhymeExpanded-Regular.ttf"_s, u"ofl/biorhymeexpanded/BioRhymeExpanded-Bold.ttf"_s, u"ofl/biorhymeexpanded/BioRhymeExpanded-ExtraBold.ttf"_s }, u"ofl/biorhymeexpanded/OFL.txt"_s ),
    GoogleFontDetails( u"Birthstone"_s, { u"ofl/birthstone/Birthstone-Regular.ttf"_s }, u"ofl/birthstone/OFL.txt"_s ),
    GoogleFontDetails( u"Birthstone Bounce"_s, { u"ofl/birthstonebounce/BirthstoneBounce-Regular.ttf"_s, u"ofl/birthstonebounce/BirthstoneBounce-Medium.ttf"_s }, u"ofl/birthstonebounce/OFL.txt"_s ),
    GoogleFontDetails( u"Biryani"_s, { u"ofl/biryani/Biryani-ExtraLight.ttf"_s, u"ofl/biryani/Biryani-Light.ttf"_s, u"ofl/biryani/Biryani-Regular.ttf"_s, u"ofl/biryani/Biryani-SemiBold.ttf"_s, u"ofl/biryani/Biryani-Bold.ttf"_s, u"ofl/biryani/Biryani-ExtraBold.ttf"_s, u"ofl/biryani/Biryani-Black.ttf"_s }, u"ofl/biryani/OFL.txt"_s ),
    GoogleFontDetails( u"Bitter"_s, { u"ofl/bitter/Bitter%5Bwght%5D.ttf"_s, u"ofl/bitter/Bitter-Italic%5Bwght%5D.ttf"_s }, u"ofl/bitter/OFL.txt"_s ),
    GoogleFontDetails( u"Black And White Picture"_s, { u"ofl/blackandwhitepicture/BlackAndWhitePicture-Regular.ttf"_s }, u"ofl/blackandwhitepicture/OFL.txt"_s ),
    GoogleFontDetails( u"Black Han Sans"_s, { u"ofl/blackhansans/BlackHanSans-Regular.ttf"_s }, u"ofl/blackhansans/OFL.txt"_s ),
    GoogleFontDetails( u"Black Ops One"_s, { u"ofl/blackopsone/BlackOpsOne-Regular.ttf"_s }, u"ofl/blackopsone/OFL.txt"_s ),
    GoogleFontDetails( u"Blaka"_s, { u"ofl/blaka/Blaka-Regular.ttf"_s }, u"ofl/blaka/OFL.txt"_s ),
    GoogleFontDetails( u"Blaka Hollow"_s, { u"ofl/blakahollow/BlakaHollow-Regular.ttf"_s }, u"ofl/blakahollow/OFL.txt"_s ),
    GoogleFontDetails( u"Blaka Ink"_s, { u"ofl/blakaink/BlakaInk-Regular.ttf"_s }, u"ofl/blakaink/OFL.txt"_s ),
    GoogleFontDetails( u"Bodoni Moda"_s, { u"ofl/bodonimoda/BodoniModa%5Bopsz,wght%5D.ttf"_s, u"ofl/bodonimoda/BodoniModa-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/bodonimoda/OFL.txt"_s ),
    GoogleFontDetails( u"Bokor"_s, { u"ofl/bokor/Bokor-Regular.ttf"_s }, u"ofl/bokor/OFL.txt"_s ),
    GoogleFontDetails( u"Bona Nova"_s, { u"ofl/bonanova/BonaNova-Regular.ttf"_s, u"ofl/bonanova/BonaNova-Italic.ttf"_s, u"ofl/bonanova/BonaNova-Bold.ttf"_s }, u"ofl/bonanova/OFL.txt"_s ),
    GoogleFontDetails( u"Bonbon"_s, { u"ofl/bonbon/Bonbon-Regular.ttf"_s }, u"ofl/bonbon/OFL.txt"_s ),
    GoogleFontDetails( u"Bonheur Royale"_s, { u"ofl/bonheurroyale/BonheurRoyale-Regular.ttf"_s }, u"ofl/bonheurroyale/OFL.txt"_s ),
    GoogleFontDetails( u"Boogaloo"_s, { u"ofl/boogaloo/Boogaloo-Regular.ttf"_s }, u"ofl/boogaloo/OFL.txt"_s ),
    GoogleFontDetails( u"Borel"_s, { u"ofl/borel/Borel-Regular.ttf"_s }, u"ofl/borel/OFL.txt"_s ),
    GoogleFontDetails( u"Bowlby One"_s, { u"ofl/bowlbyone/BowlbyOne-Regular.ttf"_s }, u"ofl/bowlbyone/OFL.txt"_s ),
    GoogleFontDetails( u"Bowlby One SC"_s, { u"ofl/bowlbyonesc/BowlbyOneSC-Regular.ttf"_s }, u"ofl/bowlbyonesc/OFL.txt"_s ),
    GoogleFontDetails( u"Braah One"_s, { u"ofl/braahone/BraahOne-Regular.ttf"_s }, u"ofl/braahone/OFL.txt"_s ),
    GoogleFontDetails( u"Brawler"_s, { u"ofl/brawler/Brawler-Regular.ttf"_s, u"ofl/brawler/Brawler-Bold.ttf"_s }, u"ofl/brawler/OFL.txt"_s ),
    GoogleFontDetails( u"Bree Serif"_s, { u"ofl/breeserif/BreeSerif-Regular.ttf"_s }, u"ofl/breeserif/OFL.txt"_s ),
    GoogleFontDetails( u"Bricolage Grotesque"_s, { u"ofl/bricolagegrotesque/BricolageGrotesque%5Bopsz,wdth,wght%5D.ttf"_s }, u"ofl/bricolagegrotesque/OFL.txt"_s ),
    GoogleFontDetails( u"Bruno Ace"_s, { u"ofl/brunoace/BrunoAce-Regular.ttf"_s }, u"ofl/brunoace/OFL.txt"_s ),
    GoogleFontDetails( u"Bruno Ace SC"_s, { u"ofl/brunoacesc/BrunoAceSC-Regular.ttf"_s }, u"ofl/brunoacesc/OFL.txt"_s ),
    GoogleFontDetails( u"Brygada 1918"_s, { u"ofl/brygada1918/Brygada1918%5Bwght%5D.ttf"_s, u"ofl/brygada1918/Brygada1918-Italic%5Bwght%5D.ttf"_s }, u"ofl/brygada1918/OFL.txt"_s ),
    GoogleFontDetails( u"Bubblegum Sans"_s, { u"ofl/bubblegumsans/BubblegumSans-Regular.ttf"_s }, u"ofl/bubblegumsans/OFL.txt"_s ),
    GoogleFontDetails( u"Bubbler One"_s, { u"ofl/bubblerone/BubblerOne-Regular.ttf"_s }, u"ofl/bubblerone/OFL.txt"_s ),
    GoogleFontDetails( u"Buda"_s, { u"ofl/buda/Buda-Light.ttf"_s }, u"ofl/buda/OFL.txt"_s ),
    GoogleFontDetails( u"Buenard"_s, { u"ofl/buenard/Buenard-Regular.ttf"_s, u"ofl/buenard/Buenard-Bold.ttf"_s }, u"ofl/buenard/OFL.txt"_s ),
    GoogleFontDetails( u"Bungee"_s, { u"ofl/bungee/Bungee-Regular.ttf"_s }, u"ofl/bungee/OFL.txt"_s ),
    GoogleFontDetails( u"Bungee Hairline"_s, { u"ofl/bungeehairline/BungeeHairline-Regular.ttf"_s }, u"ofl/bungeehairline/OFL.txt"_s ),
    GoogleFontDetails( u"Bungee Inline"_s, { u"ofl/bungeeinline/BungeeInline-Regular.ttf"_s }, u"ofl/bungeeinline/OFL.txt"_s ),
    GoogleFontDetails( u"Bungee Outline"_s, { u"ofl/bungeeoutline/BungeeOutline-Regular.ttf"_s }, u"ofl/bungeeoutline/OFL.txt"_s ),
    GoogleFontDetails( u"Bungee Shade"_s, { u"ofl/bungeeshade/BungeeShade-Regular.ttf"_s }, u"ofl/bungeeshade/OFL.txt"_s ),
    GoogleFontDetails( u"Bungee Spice"_s, { u"ofl/bungeespice/BungeeSpice-Regular.ttf"_s }, u"ofl/bungeespice/OFL.txt"_s ),
    GoogleFontDetails( u"Butcherman"_s, { u"ofl/butcherman/Butcherman-Regular.ttf"_s }, u"ofl/butcherman/OFL.txt"_s ),
    GoogleFontDetails( u"Butterfly Kids"_s, { u"ofl/butterflykids/ButterflyKids-Regular.ttf"_s }, u"ofl/butterflykids/OFL.txt"_s ),
    GoogleFontDetails( u"Cabin"_s, { u"ofl/cabin/Cabin%5Bwdth,wght%5D.ttf"_s, u"ofl/cabin/Cabin-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/cabin/OFL.txt"_s ),
    GoogleFontDetails( u"Cabin Condensed"_s, { u"ofl/cabincondensed/CabinCondensed-Regular.ttf"_s, u"ofl/cabincondensed/CabinCondensed-Medium.ttf"_s, u"ofl/cabincondensed/CabinCondensed-SemiBold.ttf"_s, u"ofl/cabincondensed/CabinCondensed-Bold.ttf"_s }, u"ofl/cabincondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Cabin Sketch"_s, { u"ofl/cabinsketch/CabinSketch-Regular.ttf"_s, u"ofl/cabinsketch/CabinSketch-Bold.ttf"_s }, u"ofl/cabinsketch/OFL.txt"_s ),
    GoogleFontDetails( u"Caesar Dressing"_s, { u"ofl/caesardressing/CaesarDressing-Regular.ttf"_s }, u"ofl/caesardressing/OFL.txt"_s ),
    GoogleFontDetails( u"Cagliostro"_s, { u"ofl/cagliostro/Cagliostro-Regular.ttf"_s }, u"ofl/cagliostro/OFL.txt"_s ),
    GoogleFontDetails( u"Cairo"_s, { u"ofl/cairo/Cairo%5Bslnt,wght%5D.ttf"_s }, u"ofl/cairo/OFL.txt"_s ),
    GoogleFontDetails( u"Cairo Play"_s, { u"ofl/cairoplay/CairoPlay%5Bslnt,wght%5D.ttf"_s }, u"ofl/cairoplay/OFL.txt"_s ),
    GoogleFontDetails( u"Caladea"_s, { u"ofl/caladea/Caladea-Regular.ttf"_s, u"ofl/caladea/Caladea-Italic.ttf"_s, u"ofl/caladea/Caladea-Bold.ttf"_s, u"ofl/caladea/Caladea-BoldItalic.ttf"_s }, u"ofl/caladea/OFL.txt"_s ),
    GoogleFontDetails( u"Calistoga"_s, { u"ofl/calistoga/Calistoga-Regular.ttf"_s }, u"ofl/calistoga/OFL.txt"_s ),
    GoogleFontDetails( u"Calligraffitti"_s, { u"apache/calligraffitti/Calligraffitti-Regular.ttf"_s }, u"apache/calligraffitti/LICENSE.txt"_s ),
    GoogleFontDetails( u"Cambay"_s, { u"ofl/cambay/Cambay-Regular.ttf"_s, u"ofl/cambay/Cambay-Italic.ttf"_s, u"ofl/cambay/Cambay-Bold.ttf"_s, u"ofl/cambay/Cambay-BoldItalic.ttf"_s }, u"ofl/cambay/OFL.txt"_s ),
    GoogleFontDetails( u"Cambo"_s, { u"ofl/cambo/Cambo-Regular.ttf"_s }, u"ofl/cambo/OFL.txt"_s ),
    GoogleFontDetails( u"Candal"_s, { u"ofl/candal/Candal.ttf"_s }, u"ofl/candal/OFL.txt"_s ),
    GoogleFontDetails( u"Cantarell"_s, { u"ofl/cantarell/Cantarell-Regular.ttf"_s, u"ofl/cantarell/Cantarell-Italic.ttf"_s, u"ofl/cantarell/Cantarell-Bold.ttf"_s, u"ofl/cantarell/Cantarell-BoldItalic.ttf"_s }, u"ofl/cantarell/OFL.txt"_s ),
    GoogleFontDetails( u"Cantata One"_s, { u"ofl/cantataone/CantataOne-Regular.ttf"_s }, u"ofl/cantataone/OFL.txt"_s ),
    GoogleFontDetails( u"Cantora One"_s, { u"ofl/cantoraone/CantoraOne-Regular.ttf"_s }, u"ofl/cantoraone/OFL.txt"_s ),
    GoogleFontDetails( u"Caprasimo"_s, { u"ofl/caprasimo/Caprasimo-Regular.ttf"_s }, u"ofl/caprasimo/OFL.txt"_s ),
    GoogleFontDetails( u"Capriola"_s, { u"ofl/capriola/Capriola-Regular.ttf"_s }, u"ofl/capriola/OFL.txt"_s ),
    GoogleFontDetails( u"Caramel"_s, { u"ofl/caramel/Caramel-Regular.ttf"_s }, u"ofl/caramel/OFL.txt"_s ),
    GoogleFontDetails( u"Carattere"_s, { u"ofl/carattere/Carattere-Regular.ttf"_s }, u"ofl/carattere/OFL.txt"_s ),
    GoogleFontDetails( u"Cardo"_s, { u"ofl/cardo/Cardo-Regular.ttf"_s, u"ofl/cardo/Cardo-Italic.ttf"_s, u"ofl/cardo/Cardo-Bold.ttf"_s }, u"ofl/cardo/OFL.txt"_s ),
    GoogleFontDetails( u"Carlito"_s, { u"ofl/carlito/Carlito-Regular.ttf"_s, u"ofl/carlito/Carlito-Italic.ttf"_s, u"ofl/carlito/Carlito-Bold.ttf"_s, u"ofl/carlito/Carlito-BoldItalic.ttf"_s }, u"ofl/carlito/OFL.txt"_s ),
    GoogleFontDetails( u"Carme"_s, { u"ofl/carme/Carme-Regular.ttf"_s }, u"ofl/carme/OFL.txt"_s ),
    GoogleFontDetails( u"Carrois Gothic"_s, { u"ofl/carroisgothic/CarroisGothic-Regular.ttf"_s }, u"ofl/carroisgothic/OFL.txt"_s ),
    GoogleFontDetails( u"Carrois Gothic SC"_s, { u"ofl/carroisgothicsc/CarroisGothicSC-Regular.ttf"_s }, u"ofl/carroisgothicsc/OFL.txt"_s ),
    GoogleFontDetails( u"Carter One"_s, { u"ofl/carterone/CarterOne.ttf"_s }, u"ofl/carterone/OFL.txt"_s ),
    GoogleFontDetails( u"Castoro"_s, { u"ofl/castoro/Castoro-Regular.ttf"_s, u"ofl/castoro/Castoro-Italic.ttf"_s }, u"ofl/castoro/OFL.txt"_s ),
    GoogleFontDetails( u"Castoro Titling"_s, { u"ofl/castorotitling/CastoroTitling-Regular.ttf"_s }, u"ofl/castorotitling/OFL.txt"_s ),
    GoogleFontDetails( u"Catamaran"_s, { u"ofl/catamaran/Catamaran%5Bwght%5D.ttf"_s }, u"ofl/catamaran/OFL.txt"_s ),
    GoogleFontDetails( u"Caudex"_s, { u"ofl/caudex/Caudex-Regular.ttf"_s, u"ofl/caudex/Caudex-Italic.ttf"_s, u"ofl/caudex/Caudex-Bold.ttf"_s, u"ofl/caudex/Caudex-BoldItalic.ttf"_s }, u"ofl/caudex/OFL.txt"_s ),
    GoogleFontDetails( u"Caveat"_s, { u"ofl/caveat/Caveat%5Bwght%5D.ttf"_s }, u"ofl/caveat/OFL.txt"_s ),
    GoogleFontDetails( u"Caveat Brush"_s, { u"ofl/caveatbrush/CaveatBrush-Regular.ttf"_s }, u"ofl/caveatbrush/OFL.txt"_s ),
    GoogleFontDetails( u"Cedarville Cursive"_s, { u"ofl/cedarvillecursive/Cedarville-Cursive.ttf"_s }, u"ofl/cedarvillecursive/OFL.txt"_s ),
    GoogleFontDetails( u"Ceviche One"_s, { u"ofl/cevicheone/CevicheOne-Regular.ttf"_s }, u"ofl/cevicheone/OFL.txt"_s ),
    GoogleFontDetails( u"Chakra Petch"_s, { u"ofl/chakrapetch/ChakraPetch-Light.ttf"_s, u"ofl/chakrapetch/ChakraPetch-LightItalic.ttf"_s, u"ofl/chakrapetch/ChakraPetch-Regular.ttf"_s, u"ofl/chakrapetch/ChakraPetch-Italic.ttf"_s, u"ofl/chakrapetch/ChakraPetch-Medium.ttf"_s, u"ofl/chakrapetch/ChakraPetch-MediumItalic.ttf"_s, u"ofl/chakrapetch/ChakraPetch-SemiBold.ttf"_s, u"ofl/chakrapetch/ChakraPetch-SemiBoldItalic.ttf"_s, u"ofl/chakrapetch/ChakraPetch-Bold.ttf"_s, u"ofl/chakrapetch/ChakraPetch-BoldItalic.ttf"_s }, u"ofl/chakrapetch/OFL.txt"_s ),
    GoogleFontDetails( u"Changa"_s, { u"ofl/changa/Changa%5Bwght%5D.ttf"_s }, u"ofl/changa/OFL.txt"_s ),
    GoogleFontDetails( u"Changa One"_s, { u"ofl/changaone/ChangaOne-Regular.ttf"_s, u"ofl/changaone/ChangaOne-Italic.ttf"_s }, u"ofl/changaone/OFL.txt"_s ),
    GoogleFontDetails( u"Chango"_s, { u"ofl/chango/Chango-Regular.ttf"_s }, u"ofl/chango/OFL.txt"_s ),
    GoogleFontDetails( u"Charis SIL"_s, { u"ofl/charissil/CharisSIL-Regular.ttf"_s, u"ofl/charissil/CharisSIL-Italic.ttf"_s, u"ofl/charissil/CharisSIL-Bold.ttf"_s, u"ofl/charissil/CharisSIL-BoldItalic.ttf"_s }, u"ofl/charissil/OFL.txt"_s ),
    GoogleFontDetails( u"Charm"_s, { u"ofl/charm/Charm-Regular.ttf"_s, u"ofl/charm/Charm-Bold.ttf"_s }, u"ofl/charm/OFL.txt"_s ),
    GoogleFontDetails( u"Charmonman"_s, { u"ofl/charmonman/Charmonman-Regular.ttf"_s, u"ofl/charmonman/Charmonman-Bold.ttf"_s }, u"ofl/charmonman/OFL.txt"_s ),
    GoogleFontDetails( u"Chau Philomene One"_s, { u"ofl/chauphilomeneone/ChauPhilomeneOne-Regular.ttf"_s, u"ofl/chauphilomeneone/ChauPhilomeneOne-Italic.ttf"_s }, u"ofl/chauphilomeneone/OFL.txt"_s ),
    GoogleFontDetails( u"Chela One"_s, { u"ofl/chelaone/ChelaOne-Regular.ttf"_s }, u"ofl/chelaone/OFL.txt"_s ),
    GoogleFontDetails( u"Chelsea Market"_s, { u"ofl/chelseamarket/ChelseaMarket-Regular.ttf"_s }, u"ofl/chelseamarket/OFL.txt"_s ),
    GoogleFontDetails( u"Chenla"_s, { u"ofl/chenla/Chenla.ttf"_s }, u"ofl/chenla/OFL.txt"_s ),
    GoogleFontDetails( u"Cherish"_s, { u"ofl/cherish/Cherish-Regular.ttf"_s }, u"ofl/cherish/OFL.txt"_s ),
    GoogleFontDetails( u"Cherry Bomb One"_s, { u"ofl/cherrybombone/CherryBombOne-Regular.ttf"_s }, u"ofl/cherrybombone/OFL.txt"_s ),
    GoogleFontDetails( u"Cherry Cream Soda"_s, { u"apache/cherrycreamsoda/CherryCreamSoda-Regular.ttf"_s }, u"apache/cherrycreamsoda/LICENSE.txt"_s ),
    GoogleFontDetails( u"Cherry Swash"_s, { u"ofl/cherryswash/CherrySwash-Regular.ttf"_s, u"ofl/cherryswash/CherrySwash-Bold.ttf"_s }, u"ofl/cherryswash/OFL.txt"_s ),
    GoogleFontDetails( u"Chewy"_s, { u"apache/chewy/Chewy-Regular.ttf"_s }, u"apache/chewy/LICENSE.txt"_s ),
    GoogleFontDetails( u"Chicle"_s, { u"ofl/chicle/Chicle-Regular.ttf"_s }, u"ofl/chicle/OFL.txt"_s ),
    GoogleFontDetails( u"Chilanka"_s, { u"ofl/chilanka/Chilanka-Regular.ttf"_s }, u"ofl/chilanka/OFL.txt"_s ),
    GoogleFontDetails( u"Chivo"_s, { u"ofl/chivo/Chivo%5Bwght%5D.ttf"_s, u"ofl/chivo/Chivo-Italic%5Bwght%5D.ttf"_s }, u"ofl/chivo/OFL.txt"_s ),
    GoogleFontDetails( u"Chivo Mono"_s, { u"ofl/chivomono/ChivoMono%5Bwght%5D.ttf"_s, u"ofl/chivomono/ChivoMono-Italic%5Bwght%5D.ttf"_s }, u"ofl/chivomono/OFL.txt"_s ),
    GoogleFontDetails( u"Chokokutai"_s, { u"ofl/chokokutai/Chokokutai-Regular.ttf"_s }, u"ofl/chokokutai/OFL.txt"_s ),
    GoogleFontDetails( u"Chonburi"_s, { u"ofl/chonburi/Chonburi-Regular.ttf"_s }, u"ofl/chonburi/OFL.txt"_s ),
    GoogleFontDetails( u"Cinzel"_s, { u"ofl/cinzel/Cinzel%5Bwght%5D.ttf"_s }, u"ofl/cinzel/OFL.txt"_s ),
    GoogleFontDetails( u"Cinzel Decorative"_s, { u"ofl/cinzeldecorative/CinzelDecorative-Regular.ttf"_s, u"ofl/cinzeldecorative/CinzelDecorative-Bold.ttf"_s, u"ofl/cinzeldecorative/CinzelDecorative-Black.ttf"_s }, u"ofl/cinzeldecorative/OFL.txt"_s ),
    GoogleFontDetails( u"Clicker Script"_s, { u"ofl/clickerscript/ClickerScript-Regular.ttf"_s }, u"ofl/clickerscript/OFL.txt"_s ),
    GoogleFontDetails( u"Climate Crisis"_s, { u"ofl/climatecrisis/ClimateCrisis%5BYEAR%5D.ttf"_s }, u"ofl/climatecrisis/OFL.txt"_s ),
    GoogleFontDetails( u"Coda"_s, { u"ofl/coda/Coda-Regular.ttf"_s, u"ofl/coda/Coda-ExtraBold.ttf"_s }, u"ofl/coda/OFL.txt"_s ),
    GoogleFontDetails( u"Codystar"_s, { u"ofl/codystar/Codystar-Light.ttf"_s, u"ofl/codystar/Codystar-Regular.ttf"_s }, u"ofl/codystar/OFL.txt"_s ),
    GoogleFontDetails( u"Coiny"_s, { u"ofl/coiny/Coiny-Regular.ttf"_s }, u"ofl/coiny/OFL.txt"_s ),
    GoogleFontDetails( u"Combo"_s, { u"ofl/combo/Combo-Regular.ttf"_s }, u"ofl/combo/OFL.txt"_s ),
    GoogleFontDetails( u"Comfortaa"_s, { u"ofl/comfortaa/Comfortaa%5Bwght%5D.ttf"_s }, u"ofl/comfortaa/OFL.txt"_s ),
    GoogleFontDetails( u"Comforter"_s, { u"ofl/comforter/Comforter-Regular.ttf"_s }, u"ofl/comforter/OFL.txt"_s ),
    GoogleFontDetails( u"Comforter Brush"_s, { u"ofl/comforterbrush/ComforterBrush-Regular.ttf"_s }, u"ofl/comforterbrush/OFL.txt"_s ),
    GoogleFontDetails( u"Comic Neue"_s, { u"ofl/comicneue/ComicNeue-Light.ttf"_s, u"ofl/comicneue/ComicNeue-LightItalic.ttf"_s, u"ofl/comicneue/ComicNeue-Regular.ttf"_s, u"ofl/comicneue/ComicNeue-Italic.ttf"_s, u"ofl/comicneue/ComicNeue-Bold.ttf"_s, u"ofl/comicneue/ComicNeue-BoldItalic.ttf"_s }, u"ofl/comicneue/OFL.txt"_s ),
    GoogleFontDetails( u"Coming Soon"_s, { u"apache/comingsoon/ComingSoon-Regular.ttf"_s }, u"apache/comingsoon/LICENSE.txt"_s ),
    GoogleFontDetails( u"Comme"_s, { u"ofl/comme/Comme%5Bwght%5D.ttf"_s }, u"ofl/comme/OFL.txt"_s ),
    GoogleFontDetails( u"Commissioner"_s, { u"ofl/commissioner/Commissioner%5BFLAR,VOLM,slnt,wght%5D.ttf"_s }, u"ofl/commissioner/OFL.txt"_s ),
    GoogleFontDetails( u"Concert One"_s, { u"ofl/concertone/ConcertOne-Regular.ttf"_s }, u"ofl/concertone/OFL.txt"_s ),
    GoogleFontDetails( u"Condiment"_s, { u"ofl/condiment/Condiment-Regular.ttf"_s }, u"ofl/condiment/OFL.txt"_s ),
    GoogleFontDetails( u"Content"_s, { u"ofl/content/Content-Regular.ttf"_s, u"ofl/content/Content-Bold.ttf"_s }, u"ofl/content/OFL.txt"_s ),
    GoogleFontDetails( u"Contrail One"_s, { u"ofl/contrailone/ContrailOne-Regular.ttf"_s }, u"ofl/contrailone/OFL.txt"_s ),
    GoogleFontDetails( u"Convergence"_s, { u"ofl/convergence/Convergence-Regular.ttf"_s }, u"ofl/convergence/OFL.txt"_s ),
    GoogleFontDetails( u"Cookie"_s, { u"ofl/cookie/Cookie-Regular.ttf"_s }, u"ofl/cookie/OFL.txt"_s ),
    GoogleFontDetails( u"Copse"_s, { u"ofl/copse/Copse-Regular.ttf"_s }, u"ofl/copse/OFL.txt"_s ),
    GoogleFontDetails( u"Corben"_s, { u"ofl/corben/Corben-Regular.ttf"_s, u"ofl/corben/Corben-Bold.ttf"_s }, u"ofl/corben/OFL.txt"_s ),
    GoogleFontDetails( u"Corinthia"_s, { u"ofl/corinthia/Corinthia-Regular.ttf"_s, u"ofl/corinthia/Corinthia-Bold.ttf"_s }, u"ofl/corinthia/OFL.txt"_s ),
    GoogleFontDetails( u"Cormorant"_s, { u"ofl/cormorant/Cormorant%5Bwght%5D.ttf"_s, u"ofl/cormorant/Cormorant-Italic%5Bwght%5D.ttf"_s }, u"ofl/cormorant/OFL.txt"_s ),
    GoogleFontDetails( u"Cormorant Garamond"_s, { u"ofl/cormorantgaramond/CormorantGaramond-Light.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-LightItalic.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-Regular.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-Italic.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-Medium.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-MediumItalic.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-SemiBold.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-SemiBoldItalic.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-Bold.ttf"_s, u"ofl/cormorantgaramond/CormorantGaramond-BoldItalic.ttf"_s }, u"ofl/cormorantgaramond/OFL.txt"_s ),
    GoogleFontDetails( u"Cormorant Infant"_s, { u"ofl/cormorantinfant/CormorantInfant-Light.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-LightItalic.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-Regular.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-Italic.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-Medium.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-MediumItalic.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-SemiBold.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-SemiBoldItalic.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-Bold.ttf"_s, u"ofl/cormorantinfant/CormorantInfant-BoldItalic.ttf"_s }, u"ofl/cormorantinfant/OFL.txt"_s ),
    GoogleFontDetails( u"Cormorant SC"_s, { u"ofl/cormorantsc/CormorantSC-Light.ttf"_s, u"ofl/cormorantsc/CormorantSC-Regular.ttf"_s, u"ofl/cormorantsc/CormorantSC-Medium.ttf"_s, u"ofl/cormorantsc/CormorantSC-SemiBold.ttf"_s, u"ofl/cormorantsc/CormorantSC-Bold.ttf"_s }, u"ofl/cormorantsc/OFL.txt"_s ),
    GoogleFontDetails( u"Cormorant Unicase"_s, { u"ofl/cormorantunicase/CormorantUnicase-Light.ttf"_s, u"ofl/cormorantunicase/CormorantUnicase-Regular.ttf"_s, u"ofl/cormorantunicase/CormorantUnicase-Medium.ttf"_s, u"ofl/cormorantunicase/CormorantUnicase-SemiBold.ttf"_s, u"ofl/cormorantunicase/CormorantUnicase-Bold.ttf"_s }, u"ofl/cormorantunicase/OFL.txt"_s ),
    GoogleFontDetails( u"Cormorant Upright"_s, { u"ofl/cormorantupright/CormorantUpright-Light.ttf"_s, u"ofl/cormorantupright/CormorantUpright-Regular.ttf"_s, u"ofl/cormorantupright/CormorantUpright-Medium.ttf"_s, u"ofl/cormorantupright/CormorantUpright-SemiBold.ttf"_s, u"ofl/cormorantupright/CormorantUpright-Bold.ttf"_s }, u"ofl/cormorantupright/OFL.txt"_s ),
    GoogleFontDetails( u"Courgette"_s, { u"ofl/courgette/Courgette-Regular.ttf"_s }, u"ofl/courgette/OFL.txt"_s ),
    GoogleFontDetails( u"Courier Prime"_s, { u"ofl/courierprime/CourierPrime-Regular.ttf"_s, u"ofl/courierprime/CourierPrime-Italic.ttf"_s, u"ofl/courierprime/CourierPrime-Bold.ttf"_s, u"ofl/courierprime/CourierPrime-BoldItalic.ttf"_s }, u"ofl/courierprime/OFL.txt"_s ),
    GoogleFontDetails( u"Cousine"_s, { u"apache/cousine/Cousine-Regular.ttf"_s, u"apache/cousine/Cousine-Italic.ttf"_s, u"apache/cousine/Cousine-Bold.ttf"_s, u"apache/cousine/Cousine-BoldItalic.ttf"_s }, u"apache/cousine/LICENSE.txt"_s ),
    GoogleFontDetails( u"Coustard"_s, { u"ofl/coustard/Coustard-Regular.ttf"_s, u"ofl/coustard/Coustard-Black.ttf"_s }, u"ofl/coustard/OFL.txt"_s ),
    GoogleFontDetails( u"Covered By Your Grace"_s, { u"ofl/coveredbyyourgrace/CoveredByYourGrace.ttf"_s }, u"ofl/coveredbyyourgrace/OFL.txt"_s ),
    GoogleFontDetails( u"Crafty Girls"_s, { u"apache/craftygirls/CraftyGirls-Regular.ttf"_s }, u"apache/craftygirls/LICENSE.txt"_s ),
    GoogleFontDetails( u"Creepster"_s, { u"ofl/creepster/Creepster-Regular.ttf"_s }, u"ofl/creepster/OFL.txt"_s ),
    GoogleFontDetails( u"Crete Round"_s, { u"ofl/creteround/CreteRound-Regular.ttf"_s, u"ofl/creteround/CreteRound-Italic.ttf"_s }, u"ofl/creteround/OFL.txt"_s ),
    GoogleFontDetails( u"Crimson Pro"_s, { u"ofl/crimsonpro/CrimsonPro%5Bwght%5D.ttf"_s, u"ofl/crimsonpro/CrimsonPro-Italic%5Bwght%5D.ttf"_s }, u"ofl/crimsonpro/OFL.txt"_s ),
    GoogleFontDetails( u"Crimson Text"_s, { u"ofl/crimsontext/CrimsonText-Regular.ttf"_s, u"ofl/crimsontext/CrimsonText-Italic.ttf"_s, u"ofl/crimsontext/CrimsonText-SemiBold.ttf"_s, u"ofl/crimsontext/CrimsonText-SemiBoldItalic.ttf"_s, u"ofl/crimsontext/CrimsonText-Bold.ttf"_s, u"ofl/crimsontext/CrimsonText-BoldItalic.ttf"_s }, u"ofl/crimsontext/OFL.txt"_s ),
    GoogleFontDetails( u"Croissant One"_s, { u"ofl/croissantone/CroissantOne-Regular.ttf"_s }, u"ofl/croissantone/OFL.txt"_s ),
    GoogleFontDetails( u"Crushed"_s, { u"apache/crushed/Crushed-Regular.ttf"_s }, u"apache/crushed/LICENSE.txt"_s ),
    GoogleFontDetails( u"Cuprum"_s, { u"ofl/cuprum/Cuprum%5Bwght%5D.ttf"_s, u"ofl/cuprum/Cuprum-Italic%5Bwght%5D.ttf"_s }, u"ofl/cuprum/OFL.txt"_s ),
    GoogleFontDetails( u"Cute Font"_s, { u"ofl/cutefont/CuteFont-Regular.ttf"_s }, u"ofl/cutefont/OFL.txt"_s ),
    GoogleFontDetails( u"Cutive"_s, { u"ofl/cutive/Cutive-Regular.ttf"_s }, u"ofl/cutive/OFL.txt"_s ),
    GoogleFontDetails( u"Cutive Mono"_s, { u"ofl/cutivemono/CutiveMono-Regular.ttf"_s }, u"ofl/cutivemono/OFL.txt"_s ),
    GoogleFontDetails( u"DM Mono"_s, { u"ofl/dmmono/DMMono-Light.ttf"_s, u"ofl/dmmono/DMMono-LightItalic.ttf"_s, u"ofl/dmmono/DMMono-Regular.ttf"_s, u"ofl/dmmono/DMMono-Italic.ttf"_s, u"ofl/dmmono/DMMono-Medium.ttf"_s, u"ofl/dmmono/DMMono-MediumItalic.ttf"_s }, u"ofl/dmmono/OFL.txt"_s ),
    GoogleFontDetails( u"DM Sans"_s, { u"ofl/dmsans/DMSans%5Bopsz,wght%5D.ttf"_s, u"ofl/dmsans/DMSans-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/dmsans/OFL.txt"_s ),
    GoogleFontDetails( u"DM Serif Display"_s, { u"ofl/dmserifdisplay/DMSerifDisplay-Regular.ttf"_s, u"ofl/dmserifdisplay/DMSerifDisplay-Italic.ttf"_s }, u"ofl/dmserifdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"DM Serif Text"_s, { u"ofl/dmseriftext/DMSerifText-Regular.ttf"_s, u"ofl/dmseriftext/DMSerifText-Italic.ttf"_s }, u"ofl/dmseriftext/OFL.txt"_s ),
    GoogleFontDetails( u"Dai Banna SIL"_s, { u"ofl/daibannasil/DaiBannaSIL-Light.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-LightItalic.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-Regular.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-Italic.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-Medium.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-MediumItalic.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-SemiBold.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-SemiBoldItalic.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-Bold.ttf"_s, u"ofl/daibannasil/DaiBannaSIL-BoldItalic.ttf"_s }, u"ofl/daibannasil/OFL.txt"_s ),
    GoogleFontDetails( u"Damion"_s, { u"ofl/damion/Damion-Regular.ttf"_s }, u"ofl/damion/OFL.txt"_s ),
    GoogleFontDetails( u"Dancing Script"_s, { u"ofl/dancingscript/DancingScript%5Bwght%5D.ttf"_s }, u"ofl/dancingscript/OFL.txt"_s ),
    GoogleFontDetails( u"Dangrek"_s, { u"ofl/dangrek/Dangrek-Regular.ttf"_s }, u"ofl/dangrek/OFL.txt"_s ),
    GoogleFontDetails( u"Darker Grotesque"_s, { u"ofl/darkergrotesque/DarkerGrotesque%5Bwght%5D.ttf"_s }, u"ofl/darkergrotesque/OFL.txt"_s ),
    GoogleFontDetails( u"Darumadrop One"_s, { u"ofl/darumadropone/DarumadropOne-Regular.ttf"_s }, u"ofl/darumadropone/OFL.txt"_s ),
    GoogleFontDetails( u"David Libre"_s, { u"ofl/davidlibre/DavidLibre-Regular.ttf"_s, u"ofl/davidlibre/DavidLibre-Medium.ttf"_s, u"ofl/davidlibre/DavidLibre-Bold.ttf"_s }, u"ofl/davidlibre/OFL.txt"_s ),
    GoogleFontDetails( u"Dawning of a New Day"_s, { u"ofl/dawningofanewday/DawningofaNewDay.ttf"_s }, u"ofl/dawningofanewday/OFL.txt"_s ),
    GoogleFontDetails( u"Days One"_s, { u"ofl/daysone/DaysOne-Regular.ttf"_s }, u"ofl/daysone/OFL.txt"_s ),
    GoogleFontDetails( u"Dekko"_s, { u"ofl/dekko/Dekko-Regular.ttf"_s }, u"ofl/dekko/OFL.txt"_s ),
    GoogleFontDetails( u"Delicious Handrawn"_s, { u"ofl/delicioushandrawn/DeliciousHandrawn-Regular.ttf"_s }, u"ofl/delicioushandrawn/OFL.txt"_s ),
    GoogleFontDetails( u"Delius"_s, { u"ofl/delius/Delius-Regular.ttf"_s }, u"ofl/delius/OFL.txt"_s ),
    GoogleFontDetails( u"Delius Swash Caps"_s, { u"ofl/deliusswashcaps/DeliusSwashCaps-Regular.ttf"_s }, u"ofl/deliusswashcaps/OFL.txt"_s ),
    GoogleFontDetails( u"Delius Unicase"_s, { u"ofl/deliusunicase/DeliusUnicase-Regular.ttf"_s, u"ofl/deliusunicase/DeliusUnicase-Bold.ttf"_s }, u"ofl/deliusunicase/OFL.txt"_s ),
    GoogleFontDetails( u"Della Respira"_s, { u"ofl/dellarespira/DellaRespira-Regular.ttf"_s }, u"ofl/dellarespira/OFL.txt"_s ),
    GoogleFontDetails( u"Denk One"_s, { u"ofl/denkone/DenkOne-Regular.ttf"_s }, u"ofl/denkone/OFL.txt"_s ),
    GoogleFontDetails( u"Devonshire"_s, { u"ofl/devonshire/Devonshire-Regular.ttf"_s }, u"ofl/devonshire/OFL.txt"_s ),
    GoogleFontDetails( u"Dhurjati"_s, { u"ofl/dhurjati/Dhurjati-Regular.ttf"_s }, u"ofl/dhurjati/OFL.txt"_s ),
    GoogleFontDetails( u"Didact Gothic"_s, { u"ofl/didactgothic/DidactGothic-Regular.ttf"_s }, u"ofl/didactgothic/OFL.txt"_s ),
    GoogleFontDetails( u"Diphylleia"_s, { u"ofl/diphylleia/Diphylleia-Regular.ttf"_s }, u"ofl/diphylleia/OFL.txt"_s ),
    GoogleFontDetails( u"Diplomata"_s, { u"ofl/diplomata/Diplomata-Regular.ttf"_s }, u"ofl/diplomata/OFL.txt"_s ),
    GoogleFontDetails( u"Diplomata SC"_s, { u"ofl/diplomatasc/DiplomataSC-Regular.ttf"_s }, u"ofl/diplomatasc/OFL.txt"_s ),
    GoogleFontDetails( u"Do Hyeon"_s, { u"ofl/dohyeon/DoHyeon-Regular.ttf"_s }, u"ofl/dohyeon/OFL.txt"_s ),
    GoogleFontDetails( u"Dokdo"_s, { u"ofl/dokdo/Dokdo-Regular.ttf"_s }, u"ofl/dokdo/OFL.txt"_s ),
    GoogleFontDetails( u"Domine"_s, { u"ofl/domine/Domine%5Bwght%5D.ttf"_s }, u"ofl/domine/OFL.txt"_s ),
    GoogleFontDetails( u"Donegal One"_s, { u"ofl/donegalone/DonegalOne-Regular.ttf"_s }, u"ofl/donegalone/OFL.txt"_s ),
    GoogleFontDetails( u"Dongle"_s, { u"ofl/dongle/Dongle-Light.ttf"_s, u"ofl/dongle/Dongle-Regular.ttf"_s, u"ofl/dongle/Dongle-Bold.ttf"_s }, u"ofl/dongle/OFL.txt"_s ),
    GoogleFontDetails( u"Doppio One"_s, { u"ofl/doppioone/DoppioOne-Regular.ttf"_s }, u"ofl/doppioone/OFL.txt"_s ),
    GoogleFontDetails( u"Dorsa"_s, { u"ofl/dorsa/Dorsa-Regular.ttf"_s }, u"ofl/dorsa/OFL.txt"_s ),
    GoogleFontDetails( u"Dosis"_s, { u"ofl/dosis/Dosis%5Bwght%5D.ttf"_s }, u"ofl/dosis/OFL.txt"_s ),
    GoogleFontDetails( u"DotGothic16"_s, { u"ofl/dotgothic16/DotGothic16-Regular.ttf"_s }, u"ofl/dotgothic16/OFL.txt"_s ),
    GoogleFontDetails( u"Dr Sugiyama"_s, { u"ofl/drsugiyama/DrSugiyama-Regular.ttf"_s }, u"ofl/drsugiyama/OFL.txt"_s ),
    GoogleFontDetails( u"Duru Sans"_s, { u"ofl/durusans/DuruSans-Regular.ttf"_s }, u"ofl/durusans/OFL.txt"_s ),
    GoogleFontDetails( u"DynaPuff"_s, { u"ofl/dynapuff/DynaPuff%5Bwdth,wght%5D.ttf"_s }, u"ofl/dynapuff/OFL.txt"_s ),
    GoogleFontDetails( u"Dynalight"_s, { u"ofl/dynalight/Dynalight-Regular.ttf"_s }, u"ofl/dynalight/OFL.txt"_s ),
    GoogleFontDetails( u"EB Garamond"_s, { u"ofl/ebgaramond/EBGaramond%5Bwght%5D.ttf"_s, u"ofl/ebgaramond/EBGaramond-Italic%5Bwght%5D.ttf"_s }, u"ofl/ebgaramond/OFL.txt"_s ),
    GoogleFontDetails( u"Eagle Lake"_s, { u"ofl/eaglelake/EagleLake-Regular.ttf"_s }, u"ofl/eaglelake/OFL.txt"_s ),
    GoogleFontDetails( u"East Sea Dokdo"_s, { u"ofl/eastseadokdo/EastSeaDokdo-Regular.ttf"_s }, u"ofl/eastseadokdo/OFL.txt"_s ),
    GoogleFontDetails( u"Eater"_s, { u"ofl/eater/Eater-Regular.ttf"_s }, u"ofl/eater/OFL.txt"_s ),
    GoogleFontDetails( u"Economica"_s, { u"ofl/economica/Economica-Regular.ttf"_s, u"ofl/economica/Economica-Italic.ttf"_s, u"ofl/economica/Economica-Bold.ttf"_s, u"ofl/economica/Economica-BoldItalic.ttf"_s }, u"ofl/economica/OFL.txt"_s ),
    GoogleFontDetails( u"Eczar"_s, { u"ofl/eczar/Eczar%5Bwght%5D.ttf"_s }, u"ofl/eczar/OFL.txt"_s ),
    GoogleFontDetails( u"Edu NSW ACT Foundation"_s, { u"ofl/edunswactfoundation/EduNSWACTFoundation%5Bwght%5D.ttf"_s }, u"ofl/edunswactfoundation/OFL.txt"_s ),
    GoogleFontDetails( u"Edu QLD Beginner"_s, { u"ofl/eduqldbeginner/EduQLDBeginner%5Bwght%5D.ttf"_s }, u"ofl/eduqldbeginner/OFL.txt"_s ),
    GoogleFontDetails( u"Edu SA Beginner"_s, { u"ofl/edusabeginner/EduSABeginner%5Bwght%5D.ttf"_s }, u"ofl/edusabeginner/OFL.txt"_s ),
    GoogleFontDetails( u"Edu TAS Beginner"_s, { u"ofl/edutasbeginner/EduTASBeginner%5Bwght%5D.ttf"_s }, u"ofl/edutasbeginner/OFL.txt"_s ),
    GoogleFontDetails( u"Edu VIC WA NT Beginner"_s, { u"ofl/eduvicwantbeginner/EduVICWANTBeginner%5Bwght%5D.ttf"_s }, u"ofl/eduvicwantbeginner/OFL.txt"_s ),
    GoogleFontDetails( u"Ek Mukta"_s, { u"ofl/ekmukta/EkMukta-ExtraLight.ttf"_s, u"ofl/ekmukta/EkMukta-Light.ttf"_s, u"ofl/ekmukta/EkMukta-Regular.ttf"_s, u"ofl/ekmukta/EkMukta-Medium.ttf"_s, u"ofl/ekmukta/EkMukta-SemiBold.ttf"_s, u"ofl/ekmukta/EkMukta-Bold.ttf"_s, u"ofl/ekmukta/EkMukta-ExtraBold.ttf"_s }, u"ofl/ekmukta/OFL.txt"_s ),
    GoogleFontDetails( u"El Messiri"_s, { u"ofl/elmessiri/ElMessiri%5Bwght%5D.ttf"_s }, u"ofl/elmessiri/OFL.txt"_s ),
    GoogleFontDetails( u"Electrolize"_s, { u"ofl/electrolize/Electrolize-Regular.ttf"_s }, u"ofl/electrolize/OFL.txt"_s ),
    GoogleFontDetails( u"Elsie"_s, { u"ofl/elsie/Elsie-Regular.ttf"_s, u"ofl/elsie/Elsie-Black.ttf"_s }, u"ofl/elsie/OFL.txt"_s ),
    GoogleFontDetails( u"Elsie Swash Caps"_s, { u"ofl/elsieswashcaps/ElsieSwashCaps-Regular.ttf"_s, u"ofl/elsieswashcaps/ElsieSwashCaps-Black.ttf"_s }, u"ofl/elsieswashcaps/OFL.txt"_s ),
    GoogleFontDetails( u"Emblema One"_s, { u"ofl/emblemaone/EmblemaOne-Regular.ttf"_s }, u"ofl/emblemaone/OFL.txt"_s ),
    GoogleFontDetails( u"Emilys Candy"_s, { u"ofl/emilyscandy/EmilysCandy-Regular.ttf"_s }, u"ofl/emilyscandy/OFL.txt"_s ),
    GoogleFontDetails( u"Encode Sans"_s, { u"ofl/encodesans/EncodeSans%5Bwdth,wght%5D.ttf"_s }, u"ofl/encodesans/OFL.txt"_s ),
    GoogleFontDetails( u"Encode Sans Condensed"_s, { u"ofl/encodesanscondensed/EncodeSansCondensed-Thin.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-ExtraLight.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-Light.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-Regular.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-Medium.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-SemiBold.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-Bold.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-ExtraBold.ttf"_s, u"ofl/encodesanscondensed/EncodeSansCondensed-Black.ttf"_s }, u"ofl/encodesanscondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Encode Sans Expanded"_s, { u"ofl/encodesansexpanded/EncodeSansExpanded-Thin.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-ExtraLight.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-Light.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-Regular.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-Medium.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-SemiBold.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-Bold.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-ExtraBold.ttf"_s, u"ofl/encodesansexpanded/EncodeSansExpanded-Black.ttf"_s }, u"ofl/encodesansexpanded/OFL.txt"_s ),
    GoogleFontDetails( u"Encode Sans SC"_s, { u"ofl/encodesanssc/EncodeSansSC%5Bwdth,wght%5D.ttf"_s }, u"ofl/encodesanssc/OFL.txt"_s ),
    GoogleFontDetails( u"Encode Sans Semi Condensed"_s, { u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Thin.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-ExtraLight.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Light.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Regular.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Medium.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-SemiBold.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Bold.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-ExtraBold.ttf"_s, u"ofl/encodesanssemicondensed/EncodeSansSemiCondensed-Black.ttf"_s }, u"ofl/encodesanssemicondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Encode Sans Semi Expanded"_s, { u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Thin.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-ExtraLight.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Light.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Regular.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Medium.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-SemiBold.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Bold.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-ExtraBold.ttf"_s, u"ofl/encodesanssemiexpanded/EncodeSansSemiExpanded-Black.ttf"_s }, u"ofl/encodesanssemiexpanded/OFL.txt"_s ),
    GoogleFontDetails( u"Engagement"_s, { u"ofl/engagement/Engagement-Regular.ttf"_s }, u"ofl/engagement/OFL.txt"_s ),
    GoogleFontDetails( u"Englebert"_s, { u"ofl/englebert/Englebert-Regular.ttf"_s }, u"ofl/englebert/OFL.txt"_s ),
    GoogleFontDetails( u"Enriqueta"_s, { u"ofl/enriqueta/Enriqueta-Regular.ttf"_s, u"ofl/enriqueta/Enriqueta-Medium.ttf"_s, u"ofl/enriqueta/Enriqueta-SemiBold.ttf"_s, u"ofl/enriqueta/Enriqueta-Bold.ttf"_s }, u"ofl/enriqueta/OFL.txt"_s ),
    GoogleFontDetails( u"Ephesis"_s, { u"ofl/ephesis/Ephesis-Regular.ttf"_s }, u"ofl/ephesis/OFL.txt"_s ),
    GoogleFontDetails( u"Epilogue"_s, { u"ofl/epilogue/Epilogue%5Bwght%5D.ttf"_s, u"ofl/epilogue/Epilogue-Italic%5Bwght%5D.ttf"_s }, u"ofl/epilogue/OFL.txt"_s ),
    GoogleFontDetails( u"Erica One"_s, { u"ofl/ericaone/EricaOne-Regular.ttf"_s }, u"ofl/ericaone/OFL.txt"_s ),
    GoogleFontDetails( u"Esteban"_s, { u"ofl/esteban/Esteban-Regular.ttf"_s }, u"ofl/esteban/OFL.txt"_s ),
    GoogleFontDetails( u"Estonia"_s, { u"ofl/estonia/Estonia-Regular.ttf"_s }, u"ofl/estonia/OFL.txt"_s ),
    GoogleFontDetails( u"Euphoria Script"_s, { u"ofl/euphoriascript/EuphoriaScript-Regular.ttf"_s }, u"ofl/euphoriascript/OFL.txt"_s ),
    GoogleFontDetails( u"Ewert"_s, { u"ofl/ewert/Ewert-Regular.ttf"_s }, u"ofl/ewert/OFL.txt"_s ),
    GoogleFontDetails( u"Exo"_s, { u"ofl/exo/Exo%5Bwght%5D.ttf"_s, u"ofl/exo/Exo-Italic%5Bwght%5D.ttf"_s }, u"ofl/exo/OFL.txt"_s ),
    GoogleFontDetails( u"Exo 2"_s, { u"ofl/exo2/Exo2%5Bwght%5D.ttf"_s, u"ofl/exo2/Exo2-Italic%5Bwght%5D.ttf"_s }, u"ofl/exo2/OFL.txt"_s ),
    GoogleFontDetails( u"Expletus Sans"_s, { u"ofl/expletussans/ExpletusSans%5Bwght%5D.ttf"_s, u"ofl/expletussans/ExpletusSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/expletussans/OFL.txt"_s ),
    GoogleFontDetails( u"Explora"_s, { u"ofl/explora/Explora-Regular.ttf"_s }, u"ofl/explora/OFL.txt"_s ),
    GoogleFontDetails( u"Fahkwang"_s, { u"ofl/fahkwang/Fahkwang-ExtraLight.ttf"_s, u"ofl/fahkwang/Fahkwang-ExtraLightItalic.ttf"_s, u"ofl/fahkwang/Fahkwang-Light.ttf"_s, u"ofl/fahkwang/Fahkwang-LightItalic.ttf"_s, u"ofl/fahkwang/Fahkwang-Regular.ttf"_s, u"ofl/fahkwang/Fahkwang-Italic.ttf"_s, u"ofl/fahkwang/Fahkwang-Medium.ttf"_s, u"ofl/fahkwang/Fahkwang-MediumItalic.ttf"_s, u"ofl/fahkwang/Fahkwang-SemiBold.ttf"_s, u"ofl/fahkwang/Fahkwang-SemiBoldItalic.ttf"_s, u"ofl/fahkwang/Fahkwang-Bold.ttf"_s, u"ofl/fahkwang/Fahkwang-BoldItalic.ttf"_s }, u"ofl/fahkwang/OFL.txt"_s ),
    GoogleFontDetails( u"Familjen Grotesk"_s, { u"ofl/familjengrotesk/FamiljenGrotesk%5Bwght%5D.ttf"_s, u"ofl/familjengrotesk/FamiljenGrotesk-Italic%5Bwght%5D.ttf"_s }, u"ofl/familjengrotesk/OFL.txt"_s ),
    GoogleFontDetails( u"Fanwood Text"_s, { u"ofl/fanwoodtext/FanwoodText-Regular.ttf"_s, u"ofl/fanwoodtext/FanwoodText-Italic.ttf"_s }, u"ofl/fanwoodtext/OFL.txt"_s ),
    GoogleFontDetails( u"Farro"_s, { u"ofl/farro/Farro-Light.ttf"_s, u"ofl/farro/Farro-Regular.ttf"_s, u"ofl/farro/Farro-Medium.ttf"_s, u"ofl/farro/Farro-Bold.ttf"_s }, u"ofl/farro/OFL.txt"_s ),
    GoogleFontDetails( u"Farsan"_s, { u"ofl/farsan/Farsan-Regular.ttf"_s }, u"ofl/farsan/OFL.txt"_s ),
    GoogleFontDetails( u"Fascinate"_s, { u"ofl/fascinate/Fascinate-Regular.ttf"_s }, u"ofl/fascinate/OFL.txt"_s ),
    GoogleFontDetails( u"Fascinate Inline"_s, { u"ofl/fascinateinline/FascinateInline-Regular.ttf"_s }, u"ofl/fascinateinline/OFL.txt"_s ),
    GoogleFontDetails( u"Faster One"_s, { u"ofl/fasterone/FasterOne-Regular.ttf"_s }, u"ofl/fasterone/OFL.txt"_s ),
    GoogleFontDetails( u"Fasthand"_s, { u"ofl/fasthand/Fasthand-Regular.ttf"_s }, u"ofl/fasthand/OFL.txt"_s ),
    GoogleFontDetails( u"Fauna One"_s, { u"ofl/faunaone/FaunaOne-Regular.ttf"_s }, u"ofl/faunaone/OFL.txt"_s ),
    GoogleFontDetails( u"Faustina"_s, { u"ofl/faustina/Faustina%5Bwght%5D.ttf"_s, u"ofl/faustina/Faustina-Italic%5Bwght%5D.ttf"_s }, u"ofl/faustina/OFL.txt"_s ),
    GoogleFontDetails( u"Federant"_s, { u"ofl/federant/Federant-Regular.ttf"_s }, u"ofl/federant/OFL.txt"_s ),
    GoogleFontDetails( u"Federo"_s, { u"ofl/federo/Federo-Regular.ttf"_s }, u"ofl/federo/OFL.txt"_s ),
    GoogleFontDetails( u"Felipa"_s, { u"ofl/felipa/Felipa-Regular.ttf"_s }, u"ofl/felipa/OFL.txt"_s ),
    GoogleFontDetails( u"Fenix"_s, { u"ofl/fenix/Fenix-Regular.ttf"_s }, u"ofl/fenix/OFL.txt"_s ),
    GoogleFontDetails( u"Festive"_s, { u"ofl/festive/Festive-Regular.ttf"_s }, u"ofl/festive/OFL.txt"_s ),
    GoogleFontDetails( u"Figtree"_s, { u"ofl/figtree/Figtree%5Bwght%5D.ttf"_s, u"ofl/figtree/Figtree-Italic%5Bwght%5D.ttf"_s }, u"ofl/figtree/OFL.txt"_s ),
    GoogleFontDetails( u"Finger Paint"_s, { u"ofl/fingerpaint/FingerPaint-Regular.ttf"_s }, u"ofl/fingerpaint/OFL.txt"_s ),
    GoogleFontDetails( u"Finlandica"_s, { u"ofl/finlandica/Finlandica%5Bwght%5D.ttf"_s, u"ofl/finlandica/Finlandica-Italic%5Bwght%5D.ttf"_s }, u"ofl/finlandica/OFL.txt"_s ),
    GoogleFontDetails( u"Fira Code"_s, { u"ofl/firacode/FiraCode%5Bwght%5D.ttf"_s }, u"ofl/firacode/OFL.txt"_s ),
    GoogleFontDetails( u"Fira Mono"_s, { u"ofl/firamono/FiraMono-Regular.ttf"_s, u"ofl/firamono/FiraMono-Medium.ttf"_s, u"ofl/firamono/FiraMono-Bold.ttf"_s }, u"ofl/firamono/OFL.txt"_s ),
    GoogleFontDetails( u"Fira Sans"_s, { u"ofl/firasans/FiraSans-Thin.ttf"_s, u"ofl/firasans/FiraSans-ThinItalic.ttf"_s, u"ofl/firasans/FiraSans-ExtraLight.ttf"_s, u"ofl/firasans/FiraSans-ExtraLightItalic.ttf"_s, u"ofl/firasans/FiraSans-Light.ttf"_s, u"ofl/firasans/FiraSans-LightItalic.ttf"_s, u"ofl/firasans/FiraSans-Regular.ttf"_s, u"ofl/firasans/FiraSans-Italic.ttf"_s, u"ofl/firasans/FiraSans-Medium.ttf"_s, u"ofl/firasans/FiraSans-MediumItalic.ttf"_s, u"ofl/firasans/FiraSans-SemiBold.ttf"_s, u"ofl/firasans/FiraSans-SemiBoldItalic.ttf"_s, u"ofl/firasans/FiraSans-Bold.ttf"_s, u"ofl/firasans/FiraSans-BoldItalic.ttf"_s, u"ofl/firasans/FiraSans-ExtraBold.ttf"_s, u"ofl/firasans/FiraSans-ExtraBoldItalic.ttf"_s, u"ofl/firasans/FiraSans-Black.ttf"_s, u"ofl/firasans/FiraSans-BlackItalic.ttf"_s }, u"ofl/firasans/OFL.txt"_s ),
    GoogleFontDetails( u"Fira Sans Condensed"_s, { u"ofl/firasanscondensed/FiraSansCondensed-Thin.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-ThinItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-ExtraLight.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-ExtraLightItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-Light.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-LightItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-Regular.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-Italic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-Medium.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-MediumItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-SemiBold.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-SemiBoldItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-Bold.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-BoldItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-ExtraBold.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-ExtraBoldItalic.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-Black.ttf"_s, u"ofl/firasanscondensed/FiraSansCondensed-BlackItalic.ttf"_s }, u"ofl/firasanscondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Fira Sans Extra Condensed"_s, { u"ofl/firasansextracondensed/FiraSansExtraCondensed-Thin.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ThinItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLight.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLightItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Light.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-LightItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Regular.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Italic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Medium.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-MediumItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBold.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBoldItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Bold.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-BoldItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBold.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBoldItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Black.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-BlackItalic.ttf"_s }, u"ofl/firasansextracondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Fira Sans Extra Condensed"_s, { u"ofl/firasansextracondensed/FiraSansExtraCondensed-Thin.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ThinItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLight.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraLightItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Light.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-LightItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Regular.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Italic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Medium.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-MediumItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBold.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-SemiBoldItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Bold.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-BoldItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBold.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-ExtraBoldItalic.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-Black.ttf"_s, u"ofl/firasansextracondensed/FiraSansExtraCondensed-BlackItalic.ttf"_s }, u"ofl/firasansextracondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Fjalla One"_s, { u"ofl/fjallaone/FjallaOne-Regular.ttf"_s }, u"ofl/fjallaone/OFL.txt"_s ),
    GoogleFontDetails( u"Fjord One"_s, { u"ofl/fjordone/FjordOne-Regular.ttf"_s }, u"ofl/fjordone/OFL.txt"_s ),
    GoogleFontDetails( u"Flamenco"_s, { u"ofl/flamenco/Flamenco-Light.ttf"_s, u"ofl/flamenco/Flamenco-Regular.ttf"_s }, u"ofl/flamenco/OFL.txt"_s ),
    GoogleFontDetails( u"Flavors"_s, { u"ofl/flavors/Flavors-Regular.ttf"_s }, u"ofl/flavors/OFL.txt"_s ),
    GoogleFontDetails( u"Fleur De Leah"_s, { u"ofl/fleurdeleah/FleurDeLeah-Regular.ttf"_s }, u"ofl/fleurdeleah/OFL.txt"_s ),
    GoogleFontDetails( u"Flow Block"_s, { u"ofl/flowblock/FlowBlock-Regular.ttf"_s }, u"ofl/flowblock/OFL.txt"_s ),
    GoogleFontDetails( u"Flow Circular"_s, { u"ofl/flowcircular/FlowCircular-Regular.ttf"_s }, u"ofl/flowcircular/OFL.txt"_s ),
    GoogleFontDetails( u"Flow Rounded"_s, { u"ofl/flowrounded/FlowRounded-Regular.ttf"_s }, u"ofl/flowrounded/OFL.txt"_s ),
    GoogleFontDetails( u"Foldit"_s, { u"ofl/foldit/Foldit%5Bwght%5D.ttf"_s }, u"ofl/foldit/OFL.txt"_s ),
    GoogleFontDetails( u"Fondamento"_s, { u"ofl/fondamento/Fondamento-Regular.ttf"_s, u"ofl/fondamento/Fondamento-Italic.ttf"_s }, u"ofl/fondamento/OFL.txt"_s ),
    GoogleFontDetails( u"Fontdiner Swanky"_s, { u"apache/fontdinerswanky/FontdinerSwanky-Regular.ttf"_s }, u"apache/fontdinerswanky/LICENSE.txt"_s ),
    GoogleFontDetails( u"Forum"_s, { u"ofl/forum/Forum-Regular.ttf"_s }, u"ofl/forum/OFL.txt"_s ),
    GoogleFontDetails( u"Fragment Mono"_s, { u"ofl/fragmentmono/FragmentMono-Regular.ttf"_s, u"ofl/fragmentmono/FragmentMono-Italic.ttf"_s }, u"ofl/fragmentmono/OFL.txt"_s ),
    GoogleFontDetails( u"Francois One"_s, { u"ofl/francoisone/FrancoisOne-Regular.ttf"_s }, u"ofl/francoisone/OFL.txt"_s ),
    GoogleFontDetails( u"Frank Ruhl Libre"_s, { u"ofl/frankruhllibre/FrankRuhlLibre%5Bwght%5D.ttf"_s }, u"ofl/frankruhllibre/OFL.txt"_s ),
    GoogleFontDetails( u"Fraunces"_s, { u"ofl/fraunces/Fraunces%5BSOFT,WONK,opsz,wght%5D.ttf"_s, u"ofl/fraunces/Fraunces-Italic%5BSOFT,WONK,opsz,wght%5D.ttf"_s }, u"ofl/fraunces/OFL.txt"_s ),
    GoogleFontDetails( u"Freckle Face"_s, { u"ofl/freckleface/FreckleFace-Regular.ttf"_s }, u"ofl/freckleface/OFL.txt"_s ),
    GoogleFontDetails( u"Fredericka the Great"_s, { u"ofl/frederickathegreat/FrederickatheGreat-Regular.ttf"_s }, u"ofl/frederickathegreat/OFL.txt"_s ),
    GoogleFontDetails( u"Fredoka"_s, { u"ofl/fredoka/Fredoka%5Bwdth,wght%5D.ttf"_s }, u"ofl/fredoka/OFL.txt"_s ),
    GoogleFontDetails( u"Freehand"_s, { u"ofl/freehand/Freehand-Regular.ttf"_s }, u"ofl/freehand/OFL.txt"_s ),
    GoogleFontDetails( u"Fresca"_s, { u"ofl/fresca/Fresca-Regular.ttf"_s }, u"ofl/fresca/OFL.txt"_s ),
    GoogleFontDetails( u"Frijole"_s, { u"ofl/frijole/Frijole-Regular.ttf"_s }, u"ofl/frijole/OFL.txt"_s ),
    GoogleFontDetails( u"Fruktur"_s, { u"ofl/fruktur/Fruktur-Regular.ttf"_s, u"ofl/fruktur/Fruktur-Italic.ttf"_s }, u"ofl/fruktur/OFL.txt"_s ),
    GoogleFontDetails( u"Fugaz One"_s, { u"ofl/fugazone/FugazOne-Regular.ttf"_s }, u"ofl/fugazone/OFL.txt"_s ),
    GoogleFontDetails( u"Fuggles"_s, { u"ofl/fuggles/Fuggles-Regular.ttf"_s }, u"ofl/fuggles/OFL.txt"_s ),
    GoogleFontDetails( u"Fuzzy Bubbles"_s, { u"ofl/fuzzybubbles/FuzzyBubbles-Regular.ttf"_s, u"ofl/fuzzybubbles/FuzzyBubbles-Bold.ttf"_s }, u"ofl/fuzzybubbles/OFL.txt"_s ),
    GoogleFontDetails( u"GFS Didot"_s, { u"ofl/gfsdidot/GFSDidot-Regular.ttf"_s }, u"ofl/gfsdidot/OFL.txt"_s ),
    GoogleFontDetails( u"GFS Neohellenic"_s, { u"ofl/gfsneohellenic/GFSNeohellenic.ttf"_s, u"ofl/gfsneohellenic/GFSNeohellenicItalic.ttf"_s, u"ofl/gfsneohellenic/GFSNeohellenicBold.ttf"_s, u"ofl/gfsneohellenic/GFSNeohellenicBoldItalic.ttf"_s }, u"ofl/gfsneohellenic/OFL.txt"_s ),
    GoogleFontDetails( u"Gabriela"_s, { u"ofl/gabriela/Gabriela-Regular.ttf"_s }, u"ofl/gabriela/OFL.txt"_s ),
    GoogleFontDetails( u"Gaegu"_s, { u"ofl/gaegu/Gaegu-Light.ttf"_s, u"ofl/gaegu/Gaegu-Regular.ttf"_s, u"ofl/gaegu/Gaegu-Bold.ttf"_s }, u"ofl/gaegu/OFL.txt"_s ),
    GoogleFontDetails( u"Gafata"_s, { u"ofl/gafata/Gafata-Regular.ttf"_s }, u"ofl/gafata/OFL.txt"_s ),
    GoogleFontDetails( u"Gajraj One"_s, { u"ofl/gajrajone/GajrajOne-Regular.ttf"_s }, u"ofl/gajrajone/OFL.txt"_s ),
    GoogleFontDetails( u"Galada"_s, { u"ofl/galada/Galada-Regular.ttf"_s }, u"ofl/galada/OFL.txt"_s ),
    GoogleFontDetails( u"Galdeano"_s, { u"ofl/galdeano/Galdeano-Regular.ttf"_s }, u"ofl/galdeano/OFL.txt"_s ),
    GoogleFontDetails( u"Galindo"_s, { u"ofl/galindo/Galindo-Regular.ttf"_s }, u"ofl/galindo/OFL.txt"_s ),
    GoogleFontDetails( u"Gamja Flower"_s, { u"ofl/gamjaflower/GamjaFlower-Regular.ttf"_s }, u"ofl/gamjaflower/OFL.txt"_s ),
    GoogleFontDetails( u"Gantari"_s, { u"ofl/gantari/Gantari%5Bwght%5D.ttf"_s, u"ofl/gantari/Gantari-Italic%5Bwght%5D.ttf"_s }, u"ofl/gantari/OFL.txt"_s ),
    GoogleFontDetails( u"Gasoek One"_s, { u"ofl/gasoekone/GasoekOne-Regular.ttf"_s }, u"ofl/gasoekone/OFL.txt"_s ),
    GoogleFontDetails( u"Gayathri"_s, { u"ofl/gayathri/Gayathri-Thin.ttf"_s, u"ofl/gayathri/Gayathri-Regular.ttf"_s, u"ofl/gayathri/Gayathri-Bold.ttf"_s }, u"ofl/gayathri/OFL.txt"_s ),
    GoogleFontDetails( u"Gelasio"_s, { u"ofl/gelasio/Gelasio%5Bwght%5D.ttf"_s, u"ofl/gelasio/Gelasio-Italic%5Bwght%5D.ttf"_s }, u"ofl/gelasio/OFL.txt"_s ),
    GoogleFontDetails( u"Gemunu Libre"_s, { u"ofl/gemunulibre/GemunuLibre%5Bwght%5D.ttf"_s }, u"ofl/gemunulibre/OFL.txt"_s ),
    GoogleFontDetails( u"Genos"_s, { u"ofl/genos/Genos%5Bwght%5D.ttf"_s, u"ofl/genos/Genos-Italic%5Bwght%5D.ttf"_s }, u"ofl/genos/OFL.txt"_s ),
    GoogleFontDetails( u"Geo"_s, { u"ofl/geo/Geo-Regular.ttf"_s, u"ofl/geo/Geo-Oblique.ttf"_s }, u"ofl/geo/OFL.txt"_s ),
    GoogleFontDetails( u"Geologica"_s, { u"ofl/geologica/Geologica%5BCRSV,SHRP,slnt,wght%5D.ttf"_s }, u"ofl/geologica/OFL.txt"_s ),
    GoogleFontDetails( u"Georama"_s, { u"ofl/georama/Georama%5Bwdth,wght%5D.ttf"_s, u"ofl/georama/Georama-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/georama/OFL.txt"_s ),
    GoogleFontDetails( u"Geostar"_s, { u"ofl/geostar/Geostar-Regular.ttf"_s }, u"ofl/geostar/OFL.txt"_s ),
    GoogleFontDetails( u"Geostar Fill"_s, { u"ofl/geostarfill/GeostarFill-Regular.ttf"_s }, u"ofl/geostarfill/OFL.txt"_s ),
    GoogleFontDetails( u"Germania One"_s, { u"ofl/germaniaone/GermaniaOne-Regular.ttf"_s }, u"ofl/germaniaone/OFL.txt"_s ),
    GoogleFontDetails( u"Gideon Roman"_s, { u"ofl/gideonroman/GideonRoman-Regular.ttf"_s }, u"ofl/gideonroman/OFL.txt"_s ),
    GoogleFontDetails( u"Gidugu"_s, { u"ofl/gidugu/Gidugu-Regular.ttf"_s }, u"ofl/gidugu/OFL.txt"_s ),
    GoogleFontDetails( u"Gilda Display"_s, { u"ofl/gildadisplay/GildaDisplay-Regular.ttf"_s }, u"ofl/gildadisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Girassol"_s, { u"ofl/girassol/Girassol-Regular.ttf"_s }, u"ofl/girassol/OFL.txt"_s ),
    GoogleFontDetails( u"Give You Glory"_s, { u"ofl/giveyouglory/GiveYouGlory.ttf"_s }, u"ofl/giveyouglory/OFL.txt"_s ),
    GoogleFontDetails( u"Glass Antiqua"_s, { u"ofl/glassantiqua/GlassAntiqua-Regular.ttf"_s }, u"ofl/glassantiqua/OFL.txt"_s ),
    GoogleFontDetails( u"Glegoo"_s, { u"ofl/glegoo/Glegoo-Regular.ttf"_s, u"ofl/glegoo/Glegoo-Bold.ttf"_s }, u"ofl/glegoo/OFL.txt"_s ),
    GoogleFontDetails( u"Gloock"_s, { u"ofl/gloock/Gloock-Regular.ttf"_s }, u"ofl/gloock/OFL.txt"_s ),
    GoogleFontDetails( u"Gloria Hallelujah"_s, { u"ofl/gloriahallelujah/GloriaHallelujah.ttf"_s }, u"ofl/gloriahallelujah/OFL.txt"_s ),
    GoogleFontDetails( u"Glory"_s, { u"ofl/glory/Glory%5Bwght%5D.ttf"_s, u"ofl/glory/Glory-Italic%5Bwght%5D.ttf"_s }, u"ofl/glory/OFL.txt"_s ),
    GoogleFontDetails( u"Gluten"_s, { u"ofl/gluten/Gluten%5Bslnt,wght%5D.ttf"_s }, u"ofl/gluten/OFL.txt"_s ),
    GoogleFontDetails( u"Goblin One"_s, { u"ofl/goblinone/GoblinOne.ttf"_s }, u"ofl/goblinone/OFL.txt"_s ),
    GoogleFontDetails( u"Gochi Hand"_s, { u"ofl/gochihand/GochiHand-Regular.ttf"_s }, u"ofl/gochihand/OFL.txt"_s ),
    GoogleFontDetails( u"Goldman"_s, { u"ofl/goldman/Goldman-Regular.ttf"_s, u"ofl/goldman/Goldman-Bold.ttf"_s }, u"ofl/goldman/OFL.txt"_s ),
    GoogleFontDetails( u"Golos Text"_s, { u"ofl/golostext/GolosText%5Bwght%5D.ttf"_s }, u"ofl/golostext/OFL.txt"_s ),
    GoogleFontDetails( u"Gorditas"_s, { u"ofl/gorditas/Gorditas-Regular.ttf"_s, u"ofl/gorditas/Gorditas-Bold.ttf"_s }, u"ofl/gorditas/OFL.txt"_s ),
    GoogleFontDetails( u"Gothic A1"_s, { u"ofl/gothica1/GothicA1-Thin.ttf"_s, u"ofl/gothica1/GothicA1-ExtraLight.ttf"_s, u"ofl/gothica1/GothicA1-Light.ttf"_s, u"ofl/gothica1/GothicA1-Regular.ttf"_s, u"ofl/gothica1/GothicA1-Medium.ttf"_s, u"ofl/gothica1/GothicA1-SemiBold.ttf"_s, u"ofl/gothica1/GothicA1-Bold.ttf"_s, u"ofl/gothica1/GothicA1-ExtraBold.ttf"_s, u"ofl/gothica1/GothicA1-Black.ttf"_s }, u"ofl/gothica1/OFL.txt"_s ),
    GoogleFontDetails( u"Gotu"_s, { u"ofl/gotu/Gotu-Regular.ttf"_s }, u"ofl/gotu/OFL.txt"_s ),
    GoogleFontDetails( u"Goudy Bookletter 1911"_s, { u"ofl/goudybookletter1911/GoudyBookletter1911.ttf"_s }, u"ofl/goudybookletter1911/OFL.txt"_s ),
    GoogleFontDetails( u"Gowun Batang"_s, { u"ofl/gowunbatang/GowunBatang-Regular.ttf"_s, u"ofl/gowunbatang/GowunBatang-Bold.ttf"_s }, u"ofl/gowunbatang/OFL.txt"_s ),
    GoogleFontDetails( u"Gowun Dodum"_s, { u"ofl/gowundodum/GowunDodum-Regular.ttf"_s }, u"ofl/gowundodum/OFL.txt"_s ),
    GoogleFontDetails( u"Graduate"_s, { u"ofl/graduate/Graduate-Regular.ttf"_s }, u"ofl/graduate/OFL.txt"_s ),
    GoogleFontDetails( u"Grand Hotel"_s, { u"ofl/grandhotel/GrandHotel-Regular.ttf"_s }, u"ofl/grandhotel/OFL.txt"_s ),
    GoogleFontDetails( u"Grandiflora One"_s, { u"ofl/grandifloraone/GrandifloraOne-Regular.ttf"_s }, u"ofl/grandifloraone/OFL.txt"_s ),
    GoogleFontDetails( u"Grandstander"_s, { u"ofl/grandstander/Grandstander%5Bwght%5D.ttf"_s, u"ofl/grandstander/Grandstander-Italic%5Bwght%5D.ttf"_s }, u"ofl/grandstander/OFL.txt"_s ),
    GoogleFontDetails( u"Grape Nuts"_s, { u"ofl/grapenuts/GrapeNuts-Regular.ttf"_s }, u"ofl/grapenuts/OFL.txt"_s ),
    GoogleFontDetails( u"Gravitas One"_s, { u"ofl/gravitasone/GravitasOne.ttf"_s }, u"ofl/gravitasone/OFL.txt"_s ),
    GoogleFontDetails( u"Great Vibes"_s, { u"ofl/greatvibes/GreatVibes-Regular.ttf"_s }, u"ofl/greatvibes/OFL.txt"_s ),
    GoogleFontDetails( u"Grechen Fuemen"_s, { u"ofl/grechenfuemen/GrechenFuemen-Regular.ttf"_s }, u"ofl/grechenfuemen/OFL.txt"_s ),
    GoogleFontDetails( u"Grenze"_s, { u"ofl/grenze/Grenze-Thin.ttf"_s, u"ofl/grenze/Grenze-ThinItalic.ttf"_s, u"ofl/grenze/Grenze-ExtraLight.ttf"_s, u"ofl/grenze/Grenze-ExtraLightItalic.ttf"_s, u"ofl/grenze/Grenze-Light.ttf"_s, u"ofl/grenze/Grenze-LightItalic.ttf"_s, u"ofl/grenze/Grenze-Regular.ttf"_s, u"ofl/grenze/Grenze-Italic.ttf"_s, u"ofl/grenze/Grenze-Medium.ttf"_s, u"ofl/grenze/Grenze-MediumItalic.ttf"_s, u"ofl/grenze/Grenze-SemiBold.ttf"_s, u"ofl/grenze/Grenze-SemiBoldItalic.ttf"_s, u"ofl/grenze/Grenze-Bold.ttf"_s, u"ofl/grenze/Grenze-BoldItalic.ttf"_s, u"ofl/grenze/Grenze-ExtraBold.ttf"_s, u"ofl/grenze/Grenze-ExtraBoldItalic.ttf"_s, u"ofl/grenze/Grenze-Black.ttf"_s, u"ofl/grenze/Grenze-BlackItalic.ttf"_s }, u"ofl/grenze/OFL.txt"_s ),
    GoogleFontDetails( u"Grenze Gotisch"_s, { u"ofl/grenzegotisch/GrenzeGotisch%5Bwght%5D.ttf"_s }, u"ofl/grenzegotisch/OFL.txt"_s ),
    GoogleFontDetails( u"Grey Qo"_s, { u"ofl/greyqo/GreyQo-Regular.ttf"_s }, u"ofl/greyqo/OFL.txt"_s ),
    GoogleFontDetails( u"Griffy"_s, { u"ofl/griffy/Griffy-Regular.ttf"_s }, u"ofl/griffy/OFL.txt"_s ),
    GoogleFontDetails( u"Gruppo"_s, { u"ofl/gruppo/Gruppo-Regular.ttf"_s }, u"ofl/gruppo/OFL.txt"_s ),
    GoogleFontDetails( u"Gudea"_s, { u"ofl/gudea/Gudea-Regular.ttf"_s, u"ofl/gudea/Gudea-Italic.ttf"_s, u"ofl/gudea/Gudea-Bold.ttf"_s }, u"ofl/gudea/OFL.txt"_s ),
    GoogleFontDetails( u"Gugi"_s, { u"ofl/gugi/Gugi-Regular.ttf"_s }, u"ofl/gugi/OFL.txt"_s ),
    GoogleFontDetails( u"Gulzar"_s, { u"ofl/gulzar/Gulzar-Regular.ttf"_s }, u"ofl/gulzar/OFL.txt"_s ),
    GoogleFontDetails( u"Gupter"_s, { u"ofl/gupter/Gupter-Regular.ttf"_s, u"ofl/gupter/Gupter-Medium.ttf"_s, u"ofl/gupter/Gupter-Bold.ttf"_s }, u"ofl/gupter/OFL.txt"_s ),
    GoogleFontDetails( u"Gurajada"_s, { u"ofl/gurajada/Gurajada-Regular.ttf"_s }, u"ofl/gurajada/OFL.txt"_s ),
    GoogleFontDetails( u"Gwendolyn"_s, { u"ofl/gwendolyn/Gwendolyn-Regular.ttf"_s, u"ofl/gwendolyn/Gwendolyn-Bold.ttf"_s }, u"ofl/gwendolyn/OFL.txt"_s ),
    GoogleFontDetails( u"Habibi"_s, { u"ofl/habibi/Habibi-Regular.ttf"_s }, u"ofl/habibi/OFL.txt"_s ),
    GoogleFontDetails( u"Hachi Maru Pop"_s, { u"ofl/hachimarupop/HachiMaruPop-Regular.ttf"_s }, u"ofl/hachimarupop/OFL.txt"_s ),
    GoogleFontDetails( u"Hahmlet"_s, { u"ofl/hahmlet/Hahmlet%5Bwght%5D.ttf"_s }, u"ofl/hahmlet/OFL.txt"_s ),
    GoogleFontDetails( u"Halant"_s, { u"ofl/halant/Halant-Light.ttf"_s, u"ofl/halant/Halant-Regular.ttf"_s, u"ofl/halant/Halant-Medium.ttf"_s, u"ofl/halant/Halant-SemiBold.ttf"_s, u"ofl/halant/Halant-Bold.ttf"_s }, u"ofl/halant/OFL.txt"_s ),
    GoogleFontDetails( u"Hammersmith One"_s, { u"ofl/hammersmithone/HammersmithOne-Regular.ttf"_s }, u"ofl/hammersmithone/OFL.txt"_s ),
    GoogleFontDetails( u"Hanalei"_s, { u"ofl/hanalei/Hanalei-Regular.ttf"_s }, u"ofl/hanalei/OFL.txt"_s ),
    GoogleFontDetails( u"Hanalei Fill"_s, { u"ofl/hanaleifill/HanaleiFill-Regular.ttf"_s }, u"ofl/hanaleifill/OFL.txt"_s ),
    GoogleFontDetails( u"Handjet"_s, { u"ofl/handjet/Handjet%5BELGR,ELSH,wght%5D.ttf"_s }, u"ofl/handjet/OFL.txt"_s ),
    GoogleFontDetails( u"Handlee"_s, { u"ofl/handlee/Handlee-Regular.ttf"_s }, u"ofl/handlee/OFL.txt"_s ),
    GoogleFontDetails( u"Hanken Grotesk"_s, { u"ofl/hankengrotesk/HankenGrotesk%5Bwght%5D.ttf"_s, u"ofl/hankengrotesk/HankenGrotesk-Italic%5Bwght%5D.ttf"_s }, u"ofl/hankengrotesk/OFL.txt"_s ),
    GoogleFontDetails( u"Hanuman"_s, { u"ofl/hanuman/Hanuman-Thin.ttf"_s, u"ofl/hanuman/Hanuman-Light.ttf"_s, u"ofl/hanuman/Hanuman-Regular.ttf"_s, u"ofl/hanuman/Hanuman-Bold.ttf"_s, u"ofl/hanuman/Hanuman-Black.ttf"_s }, u"ofl/hanuman/OFL.txt"_s ),
    GoogleFontDetails( u"Happy Monkey"_s, { u"ofl/happymonkey/HappyMonkey-Regular.ttf"_s }, u"ofl/happymonkey/OFL.txt"_s ),
    GoogleFontDetails( u"Harmattan"_s, { u"ofl/harmattan/Harmattan-Regular.ttf"_s, u"ofl/harmattan/Harmattan-Medium.ttf"_s, u"ofl/harmattan/Harmattan-SemiBold.ttf"_s, u"ofl/harmattan/Harmattan-Bold.ttf"_s }, u"ofl/harmattan/OFL.txt"_s ),
    GoogleFontDetails( u"Headland One"_s, { u"ofl/headlandone/HeadlandOne-Regular.ttf"_s }, u"ofl/headlandone/OFL.txt"_s ),
    GoogleFontDetails( u"Heebo"_s, { u"ofl/heebo/Heebo%5Bwght%5D.ttf"_s }, u"ofl/heebo/OFL.txt"_s ),
    GoogleFontDetails( u"Henny Penny"_s, { u"ofl/hennypenny/HennyPenny-Regular.ttf"_s }, u"ofl/hennypenny/OFL.txt"_s ),
    GoogleFontDetails( u"Hepta Slab"_s, { u"ofl/heptaslab/HeptaSlab%5Bwght%5D.ttf"_s }, u"ofl/heptaslab/OFL.txt"_s ),
    GoogleFontDetails( u"Herr Von Muellerhoff"_s, { u"ofl/herrvonmuellerhoff/HerrVonMuellerhoff-Regular.ttf"_s }, u"ofl/herrvonmuellerhoff/OFL.txt"_s ),
    GoogleFontDetails( u"Hi Melody"_s, { u"ofl/himelody/HiMelody-Regular.ttf"_s }, u"ofl/himelody/OFL.txt"_s ),
    GoogleFontDetails( u"Hina Mincho"_s, { u"ofl/hinamincho/HinaMincho-Regular.ttf"_s }, u"ofl/hinamincho/OFL.txt"_s ),
    GoogleFontDetails( u"Hind"_s, { u"ofl/hind/Hind-Light.ttf"_s, u"ofl/hind/Hind-Regular.ttf"_s, u"ofl/hind/Hind-Medium.ttf"_s, u"ofl/hind/Hind-SemiBold.ttf"_s, u"ofl/hind/Hind-Bold.ttf"_s }, u"ofl/hind/OFL.txt"_s ),
    GoogleFontDetails( u"Hind Guntur"_s, { u"ofl/hindguntur/HindGuntur-Light.ttf"_s, u"ofl/hindguntur/HindGuntur-Regular.ttf"_s, u"ofl/hindguntur/HindGuntur-Medium.ttf"_s, u"ofl/hindguntur/HindGuntur-SemiBold.ttf"_s, u"ofl/hindguntur/HindGuntur-Bold.ttf"_s }, u"ofl/hindguntur/OFL.txt"_s ),
    GoogleFontDetails( u"Hind Madurai"_s, { u"ofl/hindmadurai/HindMadurai-Light.ttf"_s, u"ofl/hindmadurai/HindMadurai-Regular.ttf"_s, u"ofl/hindmadurai/HindMadurai-Medium.ttf"_s, u"ofl/hindmadurai/HindMadurai-SemiBold.ttf"_s, u"ofl/hindmadurai/HindMadurai-Bold.ttf"_s }, u"ofl/hindmadurai/OFL.txt"_s ),
    GoogleFontDetails( u"Hind Siliguri"_s, { u"ofl/hindsiliguri/HindSiliguri-Light.ttf"_s, u"ofl/hindsiliguri/HindSiliguri-Regular.ttf"_s, u"ofl/hindsiliguri/HindSiliguri-Medium.ttf"_s, u"ofl/hindsiliguri/HindSiliguri-SemiBold.ttf"_s, u"ofl/hindsiliguri/HindSiliguri-Bold.ttf"_s }, u"ofl/hindsiliguri/OFL.txt"_s ),
    GoogleFontDetails( u"Hind Vadodara"_s, { u"ofl/hindvadodara/HindVadodara-Light.ttf"_s, u"ofl/hindvadodara/HindVadodara-Regular.ttf"_s, u"ofl/hindvadodara/HindVadodara-Medium.ttf"_s, u"ofl/hindvadodara/HindVadodara-SemiBold.ttf"_s, u"ofl/hindvadodara/HindVadodara-Bold.ttf"_s }, u"ofl/hindvadodara/OFL.txt"_s ),
    GoogleFontDetails( u"Holtwood One SC"_s, { u"ofl/holtwoodonesc/HoltwoodOneSC-Regular.ttf"_s }, u"ofl/holtwoodonesc/OFL.txt"_s ),
    GoogleFontDetails( u"Homemade Apple"_s, { u"apache/homemadeapple/HomemadeApple-Regular.ttf"_s }, u"apache/homemadeapple/LICENSE.txt"_s ),
    GoogleFontDetails( u"Homenaje"_s, { u"ofl/homenaje/Homenaje-Regular.ttf"_s }, u"ofl/homenaje/OFL.txt"_s ),
    GoogleFontDetails( u"Hubballi"_s, { u"ofl/hubballi/Hubballi-Regular.ttf"_s }, u"ofl/hubballi/OFL.txt"_s ),
    GoogleFontDetails( u"Hurricane"_s, { u"ofl/hurricane/Hurricane-Regular.ttf"_s }, u"ofl/hurricane/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Mono"_s, { u"ofl/ibmplexmono/IBMPlexMono-Thin.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-ThinItalic.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-ExtraLight.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-ExtraLightItalic.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-Light.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-LightItalic.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-Regular.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-Italic.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-Medium.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-MediumItalic.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-SemiBold.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-SemiBoldItalic.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-Bold.ttf"_s, u"ofl/ibmplexmono/IBMPlexMono-BoldItalic.ttf"_s }, u"ofl/ibmplexmono/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans"_s, { u"ofl/ibmplexsans/IBMPlexSans-Thin.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-ThinItalic.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-ExtraLight.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-ExtraLightItalic.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-Light.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-LightItalic.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-Regular.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-Italic.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-Medium.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-MediumItalic.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-SemiBold.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-SemiBoldItalic.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-Bold.ttf"_s, u"ofl/ibmplexsans/IBMPlexSans-BoldItalic.ttf"_s }, u"ofl/ibmplexsans/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans Arabic"_s, { u"ofl/ibmplexsansarabic/IBMPlexSansArabic-Thin.ttf"_s, u"ofl/ibmplexsansarabic/IBMPlexSansArabic-ExtraLight.ttf"_s, u"ofl/ibmplexsansarabic/IBMPlexSansArabic-Light.ttf"_s, u"ofl/ibmplexsansarabic/IBMPlexSansArabic-Regular.ttf"_s, u"ofl/ibmplexsansarabic/IBMPlexSansArabic-Medium.ttf"_s, u"ofl/ibmplexsansarabic/IBMPlexSansArabic-SemiBold.ttf"_s, u"ofl/ibmplexsansarabic/IBMPlexSansArabic-Bold.ttf"_s }, u"ofl/ibmplexsansarabic/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans Condensed"_s, { u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Thin.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-ThinItalic.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-ExtraLight.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-ExtraLightItalic.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Light.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-LightItalic.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Regular.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Italic.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Medium.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-MediumItalic.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-SemiBold.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-SemiBoldItalic.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-Bold.ttf"_s, u"ofl/ibmplexsanscondensed/IBMPlexSansCondensed-BoldItalic.ttf"_s }, u"ofl/ibmplexsanscondensed/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans Devanagari"_s, { u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Thin.ttf"_s, u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-ExtraLight.ttf"_s, u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Light.ttf"_s, u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Regular.ttf"_s, u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Medium.ttf"_s, u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-SemiBold.ttf"_s, u"ofl/ibmplexsansdevanagari/IBMPlexSansDevanagari-Bold.ttf"_s }, u"ofl/ibmplexsansdevanagari/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans Hebrew"_s, { u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Thin.ttf"_s, u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-ExtraLight.ttf"_s, u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Light.ttf"_s, u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Regular.ttf"_s, u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Medium.ttf"_s, u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-SemiBold.ttf"_s, u"ofl/ibmplexsanshebrew/IBMPlexSansHebrew-Bold.ttf"_s }, u"ofl/ibmplexsanshebrew/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans JP"_s, { u"ofl/ibmplexsansjp/IBMPlexSansJP-Thin.ttf"_s, u"ofl/ibmplexsansjp/IBMPlexSansJP-ExtraLight.ttf"_s, u"ofl/ibmplexsansjp/IBMPlexSansJP-Light.ttf"_s, u"ofl/ibmplexsansjp/IBMPlexSansJP-Regular.ttf"_s, u"ofl/ibmplexsansjp/IBMPlexSansJP-Medium.ttf"_s, u"ofl/ibmplexsansjp/IBMPlexSansJP-SemiBold.ttf"_s, u"ofl/ibmplexsansjp/IBMPlexSansJP-Bold.ttf"_s }, u"ofl/ibmplexsansjp/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans KR"_s, { u"ofl/ibmplexsanskr/IBMPlexSansKR-Thin.ttf"_s, u"ofl/ibmplexsanskr/IBMPlexSansKR-ExtraLight.ttf"_s, u"ofl/ibmplexsanskr/IBMPlexSansKR-Light.ttf"_s, u"ofl/ibmplexsanskr/IBMPlexSansKR-Regular.ttf"_s, u"ofl/ibmplexsanskr/IBMPlexSansKR-Medium.ttf"_s, u"ofl/ibmplexsanskr/IBMPlexSansKR-SemiBold.ttf"_s, u"ofl/ibmplexsanskr/IBMPlexSansKR-Bold.ttf"_s }, u"ofl/ibmplexsanskr/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans Thai"_s, { u"ofl/ibmplexsansthai/IBMPlexSansThai-Thin.ttf"_s, u"ofl/ibmplexsansthai/IBMPlexSansThai-ExtraLight.ttf"_s, u"ofl/ibmplexsansthai/IBMPlexSansThai-Light.ttf"_s, u"ofl/ibmplexsansthai/IBMPlexSansThai-Regular.ttf"_s, u"ofl/ibmplexsansthai/IBMPlexSansThai-Medium.ttf"_s, u"ofl/ibmplexsansthai/IBMPlexSansThai-SemiBold.ttf"_s, u"ofl/ibmplexsansthai/IBMPlexSansThai-Bold.ttf"_s }, u"ofl/ibmplexsansthai/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Sans Thai Looped"_s, { u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Thin.ttf"_s, u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-ExtraLight.ttf"_s, u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Light.ttf"_s, u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Regular.ttf"_s, u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Medium.ttf"_s, u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-SemiBold.ttf"_s, u"ofl/ibmplexsansthailooped/IBMPlexSansThaiLooped-Bold.ttf"_s }, u"ofl/ibmplexsansthailooped/OFL.txt"_s ),
    GoogleFontDetails( u"IBM Plex Serif"_s, { u"ofl/ibmplexserif/IBMPlexSerif-Thin.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-ThinItalic.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-ExtraLight.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-ExtraLightItalic.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-Light.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-LightItalic.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-Regular.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-Italic.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-Medium.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-MediumItalic.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-SemiBold.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-SemiBoldItalic.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-Bold.ttf"_s, u"ofl/ibmplexserif/IBMPlexSerif-BoldItalic.ttf"_s }, u"ofl/ibmplexserif/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell DW Pica"_s, { u"ofl/imfelldwpica/IMFePIrm28P.ttf"_s, u"ofl/imfelldwpica/IMFePIit28P.ttf"_s }, u"ofl/imfelldwpica/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell DW Pica SC"_s, { u"ofl/imfelldwpicasc/IMFePIsc28P.ttf"_s }, u"ofl/imfelldwpicasc/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell Double Pica"_s, { u"ofl/imfelldoublepica/IMFELLDoublePica-Regular.ttf"_s, u"ofl/imfelldoublepica/IMFELLDoublePica-Italic.ttf"_s }, u"ofl/imfelldoublepica/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell Double Pica SC"_s, { u"ofl/imfelldoublepicasc/IMFeDPsc28P.ttf"_s }, u"ofl/imfelldoublepicasc/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell English"_s, { u"ofl/imfellenglish/IMFeENrm28P.ttf"_s, u"ofl/imfellenglish/IMFeENit28P.ttf"_s }, u"ofl/imfellenglish/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell English SC"_s, { u"ofl/imfellenglishsc/IMFeENsc28P.ttf"_s }, u"ofl/imfellenglishsc/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell French Canon"_s, { u"ofl/imfellfrenchcanon/IMFeFCrm28P.ttf"_s, u"ofl/imfellfrenchcanon/IMFeFCit28P.ttf"_s }, u"ofl/imfellfrenchcanon/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell French Canon SC"_s, { u"ofl/imfellfrenchcanonsc/IMFeFCsc28P.ttf"_s }, u"ofl/imfellfrenchcanonsc/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell Great Primer"_s, { u"ofl/imfellgreatprimer/IMFeGPrm28P.ttf"_s, u"ofl/imfellgreatprimer/IMFeGPit28P.ttf"_s }, u"ofl/imfellgreatprimer/OFL.txt"_s ),
    GoogleFontDetails( u"IM Fell Great Primer SC"_s, { u"ofl/imfellgreatprimersc/IMFeGPsc28P.ttf"_s }, u"ofl/imfellgreatprimersc/OFL.txt"_s ),
    GoogleFontDetails( u"Ibarra Real Nova"_s, { u"ofl/ibarrarealnova/IbarraRealNova%5Bwght%5D.ttf"_s, u"ofl/ibarrarealnova/IbarraRealNova-Italic%5Bwght%5D.ttf"_s }, u"ofl/ibarrarealnova/OFL.txt"_s ),
    GoogleFontDetails( u"Iceberg"_s, { u"ofl/iceberg/Iceberg-Regular.ttf"_s }, u"ofl/iceberg/OFL.txt"_s ),
    GoogleFontDetails( u"Iceland"_s, { u"ofl/iceland/Iceland-Regular.ttf"_s }, u"ofl/iceland/OFL.txt"_s ),
    GoogleFontDetails( u"Imbue"_s, { u"ofl/imbue/Imbue%5Bopsz,wght%5D.ttf"_s }, u"ofl/imbue/OFL.txt"_s ),
    GoogleFontDetails( u"Imperial Script"_s, { u"ofl/imperialscript/ImperialScript-Regular.ttf"_s }, u"ofl/imperialscript/OFL.txt"_s ),
    GoogleFontDetails( u"Imprima"_s, { u"ofl/imprima/Imprima-Regular.ttf"_s }, u"ofl/imprima/OFL.txt"_s ),
    GoogleFontDetails( u"Inconsolata"_s, { u"ofl/inconsolata/Inconsolata%5Bwdth,wght%5D.ttf"_s }, u"ofl/inconsolata/OFL.txt"_s ),
    GoogleFontDetails( u"Inder"_s, { u"ofl/inder/Inder-Regular.ttf"_s }, u"ofl/inder/OFL.txt"_s ),
    GoogleFontDetails( u"Indie Flower"_s, { u"ofl/indieflower/IndieFlower-Regular.ttf"_s }, u"ofl/indieflower/OFL.txt"_s ),
    GoogleFontDetails( u"Ingrid Darling"_s, { u"ofl/ingriddarling/IngridDarling-Regular.ttf"_s }, u"ofl/ingriddarling/OFL.txt"_s ),
    GoogleFontDetails( u"Inika"_s, { u"ofl/inika/Inika-Regular.ttf"_s, u"ofl/inika/Inika-Bold.ttf"_s }, u"ofl/inika/OFL.txt"_s ),
    GoogleFontDetails( u"Inknut Antiqua"_s, { u"ofl/inknutantiqua/InknutAntiqua-Light.ttf"_s, u"ofl/inknutantiqua/InknutAntiqua-Regular.ttf"_s, u"ofl/inknutantiqua/InknutAntiqua-Medium.ttf"_s, u"ofl/inknutantiqua/InknutAntiqua-SemiBold.ttf"_s, u"ofl/inknutantiqua/InknutAntiqua-Bold.ttf"_s, u"ofl/inknutantiqua/InknutAntiqua-ExtraBold.ttf"_s, u"ofl/inknutantiqua/InknutAntiqua-Black.ttf"_s }, u"ofl/inknutantiqua/OFL.txt"_s ),
    GoogleFontDetails( u"Inria Sans"_s, { u"ofl/inriasans/InriaSans-Light.ttf"_s, u"ofl/inriasans/InriaSans-LightItalic.ttf"_s, u"ofl/inriasans/InriaSans-Regular.ttf"_s, u"ofl/inriasans/InriaSans-Italic.ttf"_s, u"ofl/inriasans/InriaSans-Bold.ttf"_s, u"ofl/inriasans/InriaSans-BoldItalic.ttf"_s }, u"ofl/inriasans/OFL.txt"_s ),
    GoogleFontDetails( u"Inria Serif"_s, { u"ofl/inriaserif/InriaSerif-Light.ttf"_s, u"ofl/inriaserif/InriaSerif-LightItalic.ttf"_s, u"ofl/inriaserif/InriaSerif-Regular.ttf"_s, u"ofl/inriaserif/InriaSerif-Italic.ttf"_s, u"ofl/inriaserif/InriaSerif-Bold.ttf"_s, u"ofl/inriaserif/InriaSerif-BoldItalic.ttf"_s }, u"ofl/inriaserif/OFL.txt"_s ),
    GoogleFontDetails( u"Inspiration"_s, { u"ofl/inspiration/Inspiration-Regular.ttf"_s }, u"ofl/inspiration/OFL.txt"_s ),
    GoogleFontDetails( u"Instrument Sans"_s, { u"ofl/instrumentsans/InstrumentSans%5Bwdth,wght%5D.ttf"_s, u"ofl/instrumentsans/InstrumentSans-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/instrumentsans/OFL.txt"_s ),
    GoogleFontDetails( u"Instrument Serif"_s, { u"ofl/instrumentserif/InstrumentSerif-Regular.ttf"_s, u"ofl/instrumentserif/InstrumentSerif-Italic.ttf"_s }, u"ofl/instrumentserif/OFL.txt"_s ),
    GoogleFontDetails( u"Inter"_s, { u"ofl/inter/Inter%5Bopsz,wght%5D.ttf"_s, u"ofl/inter/Inter-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/inter/OFL.txt"_s ),
    GoogleFontDetails( u"Inter Tight"_s, { u"ofl/intertight/InterTight%5Bwght%5D.ttf"_s, u"ofl/intertight/InterTight-Italic%5Bwght%5D.ttf"_s }, u"ofl/intertight/OFL.txt"_s ),
    GoogleFontDetails( u"Irish Grover"_s, { u"apache/irishgrover/IrishGrover-Regular.ttf"_s }, u"apache/irishgrover/LICENSE.txt"_s ),
    GoogleFontDetails( u"Island Moments"_s, { u"ofl/islandmoments/IslandMoments-Regular.ttf"_s }, u"ofl/islandmoments/OFL.txt"_s ),
    GoogleFontDetails( u"Istok Web"_s, { u"ofl/istokweb/IstokWeb-Regular.ttf"_s, u"ofl/istokweb/IstokWeb-Italic.ttf"_s, u"ofl/istokweb/IstokWeb-Bold.ttf"_s, u"ofl/istokweb/IstokWeb-BoldItalic.ttf"_s }, u"ofl/istokweb/OFL.txt"_s ),
    GoogleFontDetails( u"Italiana"_s, { u"ofl/italiana/Italiana-Regular.ttf"_s }, u"ofl/italiana/OFL.txt"_s ),
    GoogleFontDetails( u"Italianno"_s, { u"ofl/italianno/Italianno-Regular.ttf"_s }, u"ofl/italianno/OFL.txt"_s ),
    GoogleFontDetails( u"Itim"_s, { u"ofl/itim/Itim-Regular.ttf"_s }, u"ofl/itim/OFL.txt"_s ),
    GoogleFontDetails( u"Jacques Francois"_s, { u"ofl/jacquesfrancois/JacquesFrancois-Regular.ttf"_s }, u"ofl/jacquesfrancois/OFL.txt"_s ),
    GoogleFontDetails( u"Jacques Francois Shadow"_s, { u"ofl/jacquesfrancoisshadow/JacquesFrancoisShadow-Regular.ttf"_s }, u"ofl/jacquesfrancoisshadow/OFL.txt"_s ),
    GoogleFontDetails( u"Jaldi"_s, { u"ofl/jaldi/Jaldi-Regular.ttf"_s, u"ofl/jaldi/Jaldi-Bold.ttf"_s }, u"ofl/jaldi/OFL.txt"_s ),
    GoogleFontDetails( u"JetBrains Mono"_s, { u"ofl/jetbrainsmono/JetBrainsMono%5Bwght%5D.ttf"_s, u"ofl/jetbrainsmono/JetBrainsMono-Italic%5Bwght%5D.ttf"_s }, u"ofl/jetbrainsmono/OFL.txt"_s ),
    GoogleFontDetails( u"Jim Nightshade"_s, { u"ofl/jimnightshade/JimNightshade-Regular.ttf"_s }, u"ofl/jimnightshade/OFL.txt"_s ),
    GoogleFontDetails( u"Joan"_s, { u"ofl/joan/Joan-Regular.ttf"_s }, u"ofl/joan/OFL.txt"_s ),
    GoogleFontDetails( u"Jockey One"_s, { u"ofl/jockeyone/JockeyOne-Regular.ttf"_s }, u"ofl/jockeyone/OFL.txt"_s ),
    GoogleFontDetails( u"Jolly Lodger"_s, { u"ofl/jollylodger/JollyLodger-Regular.ttf"_s }, u"ofl/jollylodger/OFL.txt"_s ),
    GoogleFontDetails( u"Jomhuria"_s, { u"ofl/jomhuria/Jomhuria-Regular.ttf"_s }, u"ofl/jomhuria/OFL.txt"_s ),
    GoogleFontDetails( u"Jomolhari"_s, { u"ofl/jomolhari/Jomolhari-Regular.ttf"_s }, u"ofl/jomolhari/OFL.txt"_s ),
    GoogleFontDetails( u"Josefin Sans"_s, { u"ofl/josefinsans/JosefinSans%5Bwght%5D.ttf"_s, u"ofl/josefinsans/JosefinSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/josefinsans/OFL.txt"_s ),
    GoogleFontDetails( u"Josefin Slab"_s, { u"ofl/josefinslab/JosefinSlab%5Bwght%5D.ttf"_s, u"ofl/josefinslab/JosefinSlab-Italic%5Bwght%5D.ttf"_s }, u"ofl/josefinslab/OFL.txt"_s ),
    GoogleFontDetails( u"Jost"_s, { u"ofl/jost/Jost%5Bwght%5D.ttf"_s, u"ofl/jost/Jost-Italic%5Bwght%5D.ttf"_s }, u"ofl/jost/OFL.txt"_s ),
    GoogleFontDetails( u"Joti One"_s, { u"ofl/jotione/JotiOne-Regular.ttf"_s }, u"ofl/jotione/OFL.txt"_s ),
    GoogleFontDetails( u"Jua"_s, { u"ofl/jua/Jua-Regular.ttf"_s }, u"ofl/jua/OFL.txt"_s ),
    GoogleFontDetails( u"Judson"_s, { u"ofl/judson/Judson-Regular.ttf"_s, u"ofl/judson/Judson-Italic.ttf"_s, u"ofl/judson/Judson-Bold.ttf"_s }, u"ofl/judson/OFL.txt"_s ),
    GoogleFontDetails( u"Julee"_s, { u"ofl/julee/Julee-Regular.ttf"_s }, u"ofl/julee/OFL.txt"_s ),
    GoogleFontDetails( u"Julius Sans One"_s, { u"ofl/juliussansone/JuliusSansOne-Regular.ttf"_s }, u"ofl/juliussansone/OFL.txt"_s ),
    GoogleFontDetails( u"Junge"_s, { u"ofl/junge/Junge-Regular.ttf"_s }, u"ofl/junge/OFL.txt"_s ),
    GoogleFontDetails( u"Jura"_s, { u"ofl/jura/Jura%5Bwght%5D.ttf"_s }, u"ofl/jura/OFL.txt"_s ),
    GoogleFontDetails( u"Just Another Hand"_s, { u"apache/justanotherhand/JustAnotherHand-Regular.ttf"_s }, u"apache/justanotherhand/LICENSE.txt"_s ),
    GoogleFontDetails( u"Just Me Again Down Here"_s, { u"ofl/justmeagaindownhere/JustMeAgainDownHere.ttf"_s }, u"ofl/justmeagaindownhere/OFL.txt"_s ),
    GoogleFontDetails( u"K2D"_s, { u"ofl/k2d/K2D-Thin.ttf"_s, u"ofl/k2d/K2D-ThinItalic.ttf"_s, u"ofl/k2d/K2D-ExtraLight.ttf"_s, u"ofl/k2d/K2D-ExtraLightItalic.ttf"_s, u"ofl/k2d/K2D-Light.ttf"_s, u"ofl/k2d/K2D-LightItalic.ttf"_s, u"ofl/k2d/K2D-Regular.ttf"_s, u"ofl/k2d/K2D-Italic.ttf"_s, u"ofl/k2d/K2D-Medium.ttf"_s, u"ofl/k2d/K2D-MediumItalic.ttf"_s, u"ofl/k2d/K2D-SemiBold.ttf"_s, u"ofl/k2d/K2D-SemiBoldItalic.ttf"_s, u"ofl/k2d/K2D-Bold.ttf"_s, u"ofl/k2d/K2D-BoldItalic.ttf"_s, u"ofl/k2d/K2D-ExtraBold.ttf"_s, u"ofl/k2d/K2D-ExtraBoldItalic.ttf"_s }, u"ofl/k2d/OFL.txt"_s ),
    GoogleFontDetails( u"Kablammo"_s, { u"ofl/kablammo/Kablammo%5BMORF%5D.ttf"_s }, u"ofl/kablammo/OFL.txt"_s ),
    GoogleFontDetails( u"Kadwa"_s, { u"ofl/kadwa/Kadwa-Regular.ttf"_s, u"ofl/kadwa/Kadwa-Bold.ttf"_s }, u"ofl/kadwa/OFL.txt"_s ),
    GoogleFontDetails( u"Kaisei Decol"_s, { u"ofl/kaiseidecol/KaiseiDecol-Regular.ttf"_s, u"ofl/kaiseidecol/KaiseiDecol-Medium.ttf"_s, u"ofl/kaiseidecol/KaiseiDecol-Bold.ttf"_s }, u"ofl/kaiseidecol/OFL.txt"_s ),
    GoogleFontDetails( u"Kaisei HarunoUmi"_s, { u"ofl/kaiseiharunoumi/KaiseiHarunoUmi-Regular.ttf"_s, u"ofl/kaiseiharunoumi/KaiseiHarunoUmi-Medium.ttf"_s, u"ofl/kaiseiharunoumi/KaiseiHarunoUmi-Bold.ttf"_s }, u"ofl/kaiseiharunoumi/OFL.txt"_s ),
    GoogleFontDetails( u"Kaisei Opti"_s, { u"ofl/kaiseiopti/KaiseiOpti-Regular.ttf"_s, u"ofl/kaiseiopti/KaiseiOpti-Medium.ttf"_s, u"ofl/kaiseiopti/KaiseiOpti-Bold.ttf"_s }, u"ofl/kaiseiopti/OFL.txt"_s ),
    GoogleFontDetails( u"Kaisei Tokumin"_s, { u"ofl/kaiseitokumin/KaiseiTokumin-Regular.ttf"_s, u"ofl/kaiseitokumin/KaiseiTokumin-Medium.ttf"_s, u"ofl/kaiseitokumin/KaiseiTokumin-Bold.ttf"_s, u"ofl/kaiseitokumin/KaiseiTokumin-ExtraBold.ttf"_s }, u"ofl/kaiseitokumin/OFL.txt"_s ),
    GoogleFontDetails( u"Kalam"_s, { u"ofl/kalam/Kalam-Light.ttf"_s, u"ofl/kalam/Kalam-Regular.ttf"_s, u"ofl/kalam/Kalam-Bold.ttf"_s }, u"ofl/kalam/OFL.txt"_s ),
    GoogleFontDetails( u"Kameron"_s, { u"ofl/kameron/Kameron%5Bwght%5D.ttf"_s }, u"ofl/kameron/OFL.txt"_s ),
    GoogleFontDetails( u"Kanit"_s, { u"ofl/kanit/Kanit-Thin.ttf"_s, u"ofl/kanit/Kanit-ThinItalic.ttf"_s, u"ofl/kanit/Kanit-ExtraLight.ttf"_s, u"ofl/kanit/Kanit-ExtraLightItalic.ttf"_s, u"ofl/kanit/Kanit-Light.ttf"_s, u"ofl/kanit/Kanit-LightItalic.ttf"_s, u"ofl/kanit/Kanit-Regular.ttf"_s, u"ofl/kanit/Kanit-Italic.ttf"_s, u"ofl/kanit/Kanit-Medium.ttf"_s, u"ofl/kanit/Kanit-MediumItalic.ttf"_s, u"ofl/kanit/Kanit-SemiBold.ttf"_s, u"ofl/kanit/Kanit-SemiBoldItalic.ttf"_s, u"ofl/kanit/Kanit-Bold.ttf"_s, u"ofl/kanit/Kanit-BoldItalic.ttf"_s, u"ofl/kanit/Kanit-ExtraBold.ttf"_s, u"ofl/kanit/Kanit-ExtraBoldItalic.ttf"_s, u"ofl/kanit/Kanit-Black.ttf"_s, u"ofl/kanit/Kanit-BlackItalic.ttf"_s }, u"ofl/kanit/OFL.txt"_s ),
    GoogleFontDetails( u"Kantumruy Pro"_s, { u"ofl/kantumruypro/KantumruyPro%5Bwght%5D.ttf"_s, u"ofl/kantumruypro/KantumruyPro-Italic%5Bwght%5D.ttf"_s }, u"ofl/kantumruypro/OFL.txt"_s ),
    GoogleFontDetails( u"Karantina"_s, { u"ofl/karantina/Karantina-Light.ttf"_s, u"ofl/karantina/Karantina-Regular.ttf"_s, u"ofl/karantina/Karantina-Bold.ttf"_s }, u"ofl/karantina/OFL.txt"_s ),
    GoogleFontDetails( u"Karla"_s, { u"ofl/karla/Karla%5Bwght%5D.ttf"_s, u"ofl/karla/Karla-Italic%5Bwght%5D.ttf"_s }, u"ofl/karla/OFL.txt"_s ),
    GoogleFontDetails( u"Karma"_s, { u"ofl/karma/Karma-Light.ttf"_s, u"ofl/karma/Karma-Regular.ttf"_s, u"ofl/karma/Karma-Medium.ttf"_s, u"ofl/karma/Karma-SemiBold.ttf"_s, u"ofl/karma/Karma-Bold.ttf"_s }, u"ofl/karma/OFL.txt"_s ),
    GoogleFontDetails( u"Katibeh"_s, { u"ofl/katibeh/Katibeh-Regular.ttf"_s }, u"ofl/katibeh/OFL.txt"_s ),
    GoogleFontDetails( u"Kaushan Script"_s, { u"ofl/kaushanscript/KaushanScript-Regular.ttf"_s }, u"ofl/kaushanscript/OFL.txt"_s ),
    GoogleFontDetails( u"Kavivanar"_s, { u"ofl/kavivanar/Kavivanar-Regular.ttf"_s }, u"ofl/kavivanar/OFL.txt"_s ),
    GoogleFontDetails( u"Kavoon"_s, { u"ofl/kavoon/Kavoon-Regular.ttf"_s }, u"ofl/kavoon/OFL.txt"_s ),
    GoogleFontDetails( u"Kdam Thmor Pro"_s, { u"ofl/kdamthmorpro/KdamThmorPro-Regular.ttf"_s }, u"ofl/kdamthmorpro/OFL.txt"_s ),
    GoogleFontDetails( u"Keania One"_s, { u"ofl/keaniaone/KeaniaOne-Regular.ttf"_s }, u"ofl/keaniaone/OFL.txt"_s ),
    GoogleFontDetails( u"Kelly Slab"_s, { u"ofl/kellyslab/KellySlab-Regular.ttf"_s }, u"ofl/kellyslab/OFL.txt"_s ),
    GoogleFontDetails( u"Kenia"_s, { u"ofl/kenia/Kenia-Regular.ttf"_s }, u"ofl/kenia/OFL.txt"_s ),
    GoogleFontDetails( u"Khand"_s, { u"ofl/khand/Khand-Light.ttf"_s, u"ofl/khand/Khand-Regular.ttf"_s, u"ofl/khand/Khand-Medium.ttf"_s, u"ofl/khand/Khand-SemiBold.ttf"_s, u"ofl/khand/Khand-Bold.ttf"_s }, u"ofl/khand/OFL.txt"_s ),
    GoogleFontDetails( u"Khmer"_s, { u"ofl/khmer/Khmer.ttf"_s }, u"ofl/khmer/OFL.txt"_s ),
    GoogleFontDetails( u"Khula"_s, { u"ofl/khula/Khula-Light.ttf"_s, u"ofl/khula/Khula-Regular.ttf"_s, u"ofl/khula/Khula-SemiBold.ttf"_s, u"ofl/khula/Khula-Bold.ttf"_s, u"ofl/khula/Khula-ExtraBold.ttf"_s }, u"ofl/khula/OFL.txt"_s ),
    GoogleFontDetails( u"Kings"_s, { u"ofl/kings/Kings-Regular.ttf"_s }, u"ofl/kings/OFL.txt"_s ),
    GoogleFontDetails( u"Kirang Haerang"_s, { u"ofl/kiranghaerang/KirangHaerang-Regular.ttf"_s }, u"ofl/kiranghaerang/OFL.txt"_s ),
    GoogleFontDetails( u"Kite One"_s, { u"ofl/kiteone/KiteOne-Regular.ttf"_s }, u"ofl/kiteone/OFL.txt"_s ),
    GoogleFontDetails( u"Kiwi Maru"_s, { u"ofl/kiwimaru/KiwiMaru-Light.ttf"_s, u"ofl/kiwimaru/KiwiMaru-Regular.ttf"_s, u"ofl/kiwimaru/KiwiMaru-Medium.ttf"_s }, u"ofl/kiwimaru/OFL.txt"_s ),
    GoogleFontDetails( u"Klee One"_s, { u"ofl/kleeone/KleeOne-Regular.ttf"_s, u"ofl/kleeone/KleeOne-SemiBold.ttf"_s }, u"ofl/kleeone/OFL.txt"_s ),
    GoogleFontDetails( u"Knewave"_s, { u"ofl/knewave/Knewave-Regular.ttf"_s }, u"ofl/knewave/OFL.txt"_s ),
    GoogleFontDetails( u"KoHo"_s, { u"ofl/koho/KoHo-ExtraLight.ttf"_s, u"ofl/koho/KoHo-ExtraLightItalic.ttf"_s, u"ofl/koho/KoHo-Light.ttf"_s, u"ofl/koho/KoHo-LightItalic.ttf"_s, u"ofl/koho/KoHo-Regular.ttf"_s, u"ofl/koho/KoHo-Italic.ttf"_s, u"ofl/koho/KoHo-Medium.ttf"_s, u"ofl/koho/KoHo-MediumItalic.ttf"_s, u"ofl/koho/KoHo-SemiBold.ttf"_s, u"ofl/koho/KoHo-SemiBoldItalic.ttf"_s, u"ofl/koho/KoHo-Bold.ttf"_s, u"ofl/koho/KoHo-BoldItalic.ttf"_s }, u"ofl/koho/OFL.txt"_s ),
    GoogleFontDetails( u"Kodchasan"_s, { u"ofl/kodchasan/Kodchasan-ExtraLight.ttf"_s, u"ofl/kodchasan/Kodchasan-ExtraLightItalic.ttf"_s, u"ofl/kodchasan/Kodchasan-Light.ttf"_s, u"ofl/kodchasan/Kodchasan-LightItalic.ttf"_s, u"ofl/kodchasan/Kodchasan-Regular.ttf"_s, u"ofl/kodchasan/Kodchasan-Italic.ttf"_s, u"ofl/kodchasan/Kodchasan-Medium.ttf"_s, u"ofl/kodchasan/Kodchasan-MediumItalic.ttf"_s, u"ofl/kodchasan/Kodchasan-SemiBold.ttf"_s, u"ofl/kodchasan/Kodchasan-SemiBoldItalic.ttf"_s, u"ofl/kodchasan/Kodchasan-Bold.ttf"_s, u"ofl/kodchasan/Kodchasan-BoldItalic.ttf"_s }, u"ofl/kodchasan/OFL.txt"_s ),
    GoogleFontDetails( u"Koh Santepheap"_s, { u"ofl/kohsantepheap/KohSantepheap-Thin.ttf"_s, u"ofl/kohsantepheap/KohSantepheap-Light.ttf"_s, u"ofl/kohsantepheap/KohSantepheap-Regular.ttf"_s, u"ofl/kohsantepheap/KohSantepheap-Bold.ttf"_s, u"ofl/kohsantepheap/KohSantepheap-Black.ttf"_s }, u"ofl/kohsantepheap/OFL.txt"_s ),
    GoogleFontDetails( u"Kolker Brush"_s, { u"ofl/kolkerbrush/KolkerBrush-Regular.ttf"_s }, u"ofl/kolkerbrush/OFL.txt"_s ),
    GoogleFontDetails( u"Konkhmer Sleokchher"_s, { u"ofl/konkhmersleokchher/KonkhmerSleokchher-Regular.ttf"_s }, u"ofl/konkhmersleokchher/OFL.txt"_s ),
    GoogleFontDetails( u"Kosugi"_s, { u"apache/kosugi/Kosugi-Regular.ttf"_s }, u"apache/kosugi/LICENSE.txt"_s ),
    GoogleFontDetails( u"Kosugi Maru"_s, { u"apache/kosugimaru/KosugiMaru-Regular.ttf"_s }, u"apache/kosugimaru/LICENSE.txt"_s ),
    GoogleFontDetails( u"Kotta One"_s, { u"ofl/kottaone/KottaOne-Regular.ttf"_s }, u"ofl/kottaone/OFL.txt"_s ),
    GoogleFontDetails( u"Koulen"_s, { u"ofl/koulen/Koulen-Regular.ttf"_s }, u"ofl/koulen/OFL.txt"_s ),
    GoogleFontDetails( u"Kranky"_s, { u"apache/kranky/Kranky-Regular.ttf"_s }, u"apache/kranky/LICENSE.txt"_s ),
    GoogleFontDetails( u"Kreon"_s, { u"ofl/kreon/Kreon%5Bwght%5D.ttf"_s }, u"ofl/kreon/OFL.txt"_s ),
    GoogleFontDetails( u"Kristi"_s, { u"ofl/kristi/Kristi-Regular.ttf"_s }, u"ofl/kristi/OFL.txt"_s ),
    GoogleFontDetails( u"Krona One"_s, { u"ofl/kronaone/KronaOne-Regular.ttf"_s }, u"ofl/kronaone/OFL.txt"_s ),
    GoogleFontDetails( u"Krub"_s, { u"ofl/krub/Krub-ExtraLight.ttf"_s, u"ofl/krub/Krub-ExtraLightItalic.ttf"_s, u"ofl/krub/Krub-Light.ttf"_s, u"ofl/krub/Krub-LightItalic.ttf"_s, u"ofl/krub/Krub-Regular.ttf"_s, u"ofl/krub/Krub-Italic.ttf"_s, u"ofl/krub/Krub-Medium.ttf"_s, u"ofl/krub/Krub-MediumItalic.ttf"_s, u"ofl/krub/Krub-SemiBold.ttf"_s, u"ofl/krub/Krub-SemiBoldItalic.ttf"_s, u"ofl/krub/Krub-Bold.ttf"_s, u"ofl/krub/Krub-BoldItalic.ttf"_s }, u"ofl/krub/OFL.txt"_s ),
    GoogleFontDetails( u"Kufam"_s, { u"ofl/kufam/Kufam%5Bwght%5D.ttf"_s, u"ofl/kufam/Kufam-Italic%5Bwght%5D.ttf"_s }, u"ofl/kufam/OFL.txt"_s ),
    GoogleFontDetails( u"Kumar One"_s, { u"ofl/kumarone/KumarOne-Regular.ttf"_s }, u"ofl/kumarone/OFL.txt"_s ),
    GoogleFontDetails( u"Kumbh Sans"_s, { u"ofl/kumbhsans/KumbhSans%5BYOPQ,wght%5D.ttf"_s }, u"ofl/kumbhsans/OFL.txt"_s ),
    GoogleFontDetails( u"Kurale"_s, { u"ofl/kurale/Kurale-Regular.ttf"_s }, u"ofl/kurale/OFL.txt"_s ),
    GoogleFontDetails( u"La Belle Aurore"_s, { u"ofl/labelleaurore/LaBelleAurore.ttf"_s }, u"ofl/labelleaurore/OFL.txt"_s ),
    GoogleFontDetails( u"Labrada"_s, { u"ofl/labrada/Labrada%5Bwght%5D.ttf"_s, u"ofl/labrada/Labrada-Italic%5Bwght%5D.ttf"_s }, u"ofl/labrada/OFL.txt"_s ),
    GoogleFontDetails( u"Lacquer"_s, { u"ofl/lacquer/Lacquer-Regular.ttf"_s }, u"ofl/lacquer/OFL.txt"_s ),
    GoogleFontDetails( u"Laila"_s, { u"ofl/laila/Laila-Light.ttf"_s, u"ofl/laila/Laila-Regular.ttf"_s, u"ofl/laila/Laila-Medium.ttf"_s, u"ofl/laila/Laila-SemiBold.ttf"_s, u"ofl/laila/Laila-Bold.ttf"_s }, u"ofl/laila/OFL.txt"_s ),
    GoogleFontDetails( u"Lakki Reddy"_s, { u"ofl/lakkireddy/LakkiReddy-Regular.ttf"_s }, u"ofl/lakkireddy/OFL.txt"_s ),
    GoogleFontDetails( u"Lalezar"_s, { u"ofl/lalezar/Lalezar-Regular.ttf"_s }, u"ofl/lalezar/OFL.txt"_s ),
    GoogleFontDetails( u"Lancelot"_s, { u"ofl/lancelot/Lancelot-Regular.ttf"_s }, u"ofl/lancelot/OFL.txt"_s ),
    GoogleFontDetails( u"Langar"_s, { u"ofl/langar/Langar-Regular.ttf"_s }, u"ofl/langar/OFL.txt"_s ),
    GoogleFontDetails( u"Lateef"_s, { u"ofl/lateef/Lateef-ExtraLight.ttf"_s, u"ofl/lateef/Lateef-Light.ttf"_s, u"ofl/lateef/Lateef-Regular.ttf"_s, u"ofl/lateef/Lateef-Medium.ttf"_s, u"ofl/lateef/Lateef-SemiBold.ttf"_s, u"ofl/lateef/Lateef-Bold.ttf"_s, u"ofl/lateef/Lateef-ExtraBold.ttf"_s }, u"ofl/lateef/OFL.txt"_s ),
    GoogleFontDetails( u"Lato"_s, { u"ofl/lato/Lato-Thin.ttf"_s, u"ofl/lato/Lato-ThinItalic.ttf"_s, u"ofl/lato/Lato-ExtraLight.ttf"_s, u"ofl/lato/Lato-ExtraLightItalic.ttf"_s, u"ofl/lato/Lato-Light.ttf"_s, u"ofl/lato/Lato-LightItalic.ttf"_s, u"ofl/lato/Lato-Regular.ttf"_s, u"ofl/lato/Lato-Italic.ttf"_s, u"ofl/lato/Lato-Medium.ttf"_s, u"ofl/lato/Lato-MediumItalic.ttf"_s, u"ofl/lato/Lato-SemiBold.ttf"_s, u"ofl/lato/Lato-SemiBoldItalic.ttf"_s, u"ofl/lato/Lato-Bold.ttf"_s, u"ofl/lato/Lato-BoldItalic.ttf"_s, u"ofl/lato/Lato-ExtraBold.ttf"_s, u"ofl/lato/Lato-ExtraBoldItalic.ttf"_s, u"ofl/lato/Lato-Black.ttf"_s, u"ofl/lato/Lato-BlackItalic.ttf"_s }, u"ofl/lato/OFL.txt"_s ),
    GoogleFontDetails( u"Lavishly Yours"_s, { u"ofl/lavishlyyours/LavishlyYours-Regular.ttf"_s }, u"ofl/lavishlyyours/OFL.txt"_s ),
    GoogleFontDetails( u"League Gothic"_s, { u"ofl/leaguegothic/LeagueGothic%5Bwdth%5D.ttf"_s }, u"ofl/leaguegothic/OFL.txt"_s ),
    GoogleFontDetails( u"League Script"_s, { u"ofl/leaguescript/LeagueScript-Regular.ttf"_s }, u"ofl/leaguescript/OFL.txt"_s ),
    GoogleFontDetails( u"League Spartan"_s, { u"ofl/leaguespartan/LeagueSpartan%5Bwght%5D.ttf"_s }, u"ofl/leaguespartan/OFL.txt"_s ),
    GoogleFontDetails( u"Leckerli One"_s, { u"ofl/leckerlione/LeckerliOne-Regular.ttf"_s }, u"ofl/leckerlione/OFL.txt"_s ),
    GoogleFontDetails( u"Ledger"_s, { u"ofl/ledger/Ledger-Regular.ttf"_s }, u"ofl/ledger/OFL.txt"_s ),
    GoogleFontDetails( u"Lekton"_s, { u"ofl/lekton/Lekton-Regular.ttf"_s, u"ofl/lekton/Lekton-Italic.ttf"_s, u"ofl/lekton/Lekton-Bold.ttf"_s }, u"ofl/lekton/OFL.txt"_s ),
    GoogleFontDetails( u"Lemon"_s, { u"ofl/lemon/Lemon-Regular.ttf"_s }, u"ofl/lemon/OFL.txt"_s ),
    GoogleFontDetails( u"Lemonada"_s, { u"ofl/lemonada/Lemonada%5Bwght%5D.ttf"_s }, u"ofl/lemonada/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend"_s, { u"ofl/lexend/Lexend%5Bwght%5D.ttf"_s }, u"ofl/lexend/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Deca"_s, { u"ofl/lexenddeca/LexendDeca%5Bwght%5D.ttf"_s }, u"ofl/lexenddeca/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Exa"_s, { u"ofl/lexendexa/LexendExa%5Bwght%5D.ttf"_s }, u"ofl/lexendexa/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Giga"_s, { u"ofl/lexendgiga/LexendGiga%5Bwght%5D.ttf"_s }, u"ofl/lexendgiga/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Mega"_s, { u"ofl/lexendmega/LexendMega%5Bwght%5D.ttf"_s }, u"ofl/lexendmega/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Peta"_s, { u"ofl/lexendpeta/LexendPeta%5Bwght%5D.ttf"_s }, u"ofl/lexendpeta/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Tera"_s, { u"ofl/lexendtera/LexendTera%5Bwght%5D.ttf"_s }, u"ofl/lexendtera/OFL.txt"_s ),
    GoogleFontDetails( u"Lexend Zetta"_s, { u"ofl/lexendzetta/LexendZetta%5Bwght%5D.ttf"_s }, u"ofl/lexendzetta/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode 128"_s, { u"ofl/librebarcode128/LibreBarcode128-Regular.ttf"_s }, u"ofl/librebarcode128/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode 128 Text"_s, { u"ofl/librebarcode128text/LibreBarcode128Text-Regular.ttf"_s }, u"ofl/librebarcode128text/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode 39"_s, { u"ofl/librebarcode39/LibreBarcode39-Regular.ttf"_s }, u"ofl/librebarcode39/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode 39 Extended"_s, { u"ofl/librebarcode39extended/LibreBarcode39Extended-Regular.ttf"_s }, u"ofl/librebarcode39extended/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode 39 Extended Text"_s, { u"ofl/librebarcode39extendedtext/LibreBarcode39ExtendedText-Regular.ttf"_s }, u"ofl/librebarcode39extendedtext/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode 39 Text"_s, { u"ofl/librebarcode39text/LibreBarcode39Text-Regular.ttf"_s }, u"ofl/librebarcode39text/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Barcode EAN13 Text"_s, { u"ofl/librebarcodeean13text/LibreBarcodeEAN13Text-Regular.ttf"_s }, u"ofl/librebarcodeean13text/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Baskerville"_s, { u"ofl/librebaskerville/LibreBaskerville-Regular.ttf"_s, u"ofl/librebaskerville/LibreBaskerville-Italic.ttf"_s, u"ofl/librebaskerville/LibreBaskerville-Bold.ttf"_s }, u"ofl/librebaskerville/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Bodoni"_s, { u"ofl/librebodoni/LibreBodoni%5Bwght%5D.ttf"_s, u"ofl/librebodoni/LibreBodoni-Italic%5Bwght%5D.ttf"_s }, u"ofl/librebodoni/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Caslon Display"_s, { u"ofl/librecaslondisplay/LibreCaslonDisplay-Regular.ttf"_s }, u"ofl/librecaslondisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Caslon Text"_s, { u"ofl/librecaslontext/LibreCaslonText%5Bwght%5D.ttf"_s, u"ofl/librecaslontext/LibreCaslonText-Italic%5Bwght%5D.ttf"_s }, u"ofl/librecaslontext/OFL.txt"_s ),
    GoogleFontDetails( u"Libre Franklin"_s, { u"ofl/librefranklin/LibreFranklin%5Bwght%5D.ttf"_s, u"ofl/librefranklin/LibreFranklin-Italic%5Bwght%5D.ttf"_s }, u"ofl/librefranklin/OFL.txt"_s ),
    GoogleFontDetails( u"Licorice"_s, { u"ofl/licorice/Licorice-Regular.ttf"_s }, u"ofl/licorice/OFL.txt"_s ),
    GoogleFontDetails( u"Life Savers"_s, { u"ofl/lifesavers/LifeSavers-Regular.ttf"_s, u"ofl/lifesavers/LifeSavers-Bold.ttf"_s, u"ofl/lifesavers/LifeSavers-ExtraBold.ttf"_s }, u"ofl/lifesavers/OFL.txt"_s ),
    GoogleFontDetails( u"Lilita One"_s, { u"ofl/lilitaone/LilitaOne-Regular.ttf"_s }, u"ofl/lilitaone/OFL.txt"_s ),
    GoogleFontDetails( u"Lily Script One"_s, { u"ofl/lilyscriptone/LilyScriptOne-Regular.ttf"_s }, u"ofl/lilyscriptone/OFL.txt"_s ),
    GoogleFontDetails( u"Limelight"_s, { u"ofl/limelight/Limelight-Regular.ttf"_s }, u"ofl/limelight/OFL.txt"_s ),
    GoogleFontDetails( u"Linden Hill"_s, { u"ofl/lindenhill/LindenHill-Regular.ttf"_s, u"ofl/lindenhill/LindenHill-Italic.ttf"_s }, u"ofl/lindenhill/OFL.txt"_s ),
    GoogleFontDetails( u"Lisu Bosa"_s, { u"ofl/lisubosa/LisuBosa-ExtraLight.ttf"_s, u"ofl/lisubosa/LisuBosa-ExtraLightItalic.ttf"_s, u"ofl/lisubosa/LisuBosa-Light.ttf"_s, u"ofl/lisubosa/LisuBosa-LightItalic.ttf"_s, u"ofl/lisubosa/LisuBosa-Regular.ttf"_s, u"ofl/lisubosa/LisuBosa-Italic.ttf"_s, u"ofl/lisubosa/LisuBosa-Medium.ttf"_s, u"ofl/lisubosa/LisuBosa-MediumItalic.ttf"_s, u"ofl/lisubosa/LisuBosa-SemiBold.ttf"_s, u"ofl/lisubosa/LisuBosa-SemiBoldItalic.ttf"_s, u"ofl/lisubosa/LisuBosa-Bold.ttf"_s, u"ofl/lisubosa/LisuBosa-BoldItalic.ttf"_s, u"ofl/lisubosa/LisuBosa-ExtraBold.ttf"_s, u"ofl/lisubosa/LisuBosa-ExtraBoldItalic.ttf"_s, u"ofl/lisubosa/LisuBosa-Black.ttf"_s, u"ofl/lisubosa/LisuBosa-BlackItalic.ttf"_s }, u"ofl/lisubosa/OFL.txt"_s ),
    GoogleFontDetails( u"Literata"_s, { u"ofl/literata/Literata%5Bopsz,wght%5D.ttf"_s, u"ofl/literata/Literata-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/literata/OFL.txt"_s ),
    GoogleFontDetails( u"Liu Jian Mao Cao"_s, { u"ofl/liujianmaocao/LiuJianMaoCao-Regular.ttf"_s }, u"ofl/liujianmaocao/OFL.txt"_s ),
    GoogleFontDetails( u"Livvic"_s, { u"ofl/livvic/Livvic-Thin.ttf"_s, u"ofl/livvic/Livvic-ThinItalic.ttf"_s, u"ofl/livvic/Livvic-ExtraLight.ttf"_s, u"ofl/livvic/Livvic-ExtraLightItalic.ttf"_s, u"ofl/livvic/Livvic-Light.ttf"_s, u"ofl/livvic/Livvic-LightItalic.ttf"_s, u"ofl/livvic/Livvic-Regular.ttf"_s, u"ofl/livvic/Livvic-Italic.ttf"_s, u"ofl/livvic/Livvic-Medium.ttf"_s, u"ofl/livvic/Livvic-MediumItalic.ttf"_s, u"ofl/livvic/Livvic-SemiBold.ttf"_s, u"ofl/livvic/Livvic-SemiBoldItalic.ttf"_s, u"ofl/livvic/Livvic-Bold.ttf"_s, u"ofl/livvic/Livvic-BoldItalic.ttf"_s, u"ofl/livvic/Livvic-Black.ttf"_s, u"ofl/livvic/Livvic-BlackItalic.ttf"_s }, u"ofl/livvic/OFL.txt"_s ),
    GoogleFontDetails( u"Lobster"_s, { u"ofl/lobster/Lobster-Regular.ttf"_s }, u"ofl/lobster/OFL.txt"_s ),
    GoogleFontDetails( u"Lobster Two"_s, { u"ofl/lobstertwo/LobsterTwo-Regular.ttf"_s, u"ofl/lobstertwo/LobsterTwo-Italic.ttf"_s, u"ofl/lobstertwo/LobsterTwo-Bold.ttf"_s, u"ofl/lobstertwo/LobsterTwo-BoldItalic.ttf"_s }, u"ofl/lobstertwo/OFL.txt"_s ),
    GoogleFontDetails( u"Londrina Outline"_s, { u"ofl/londrinaoutline/LondrinaOutline-Regular.ttf"_s }, u"ofl/londrinaoutline/OFL.txt"_s ),
    GoogleFontDetails( u"Londrina Shadow"_s, { u"ofl/londrinashadow/LondrinaShadow-Regular.ttf"_s }, u"ofl/londrinashadow/OFL.txt"_s ),
    GoogleFontDetails( u"Londrina Sketch"_s, { u"ofl/londrinasketch/LondrinaSketch-Regular.ttf"_s }, u"ofl/londrinasketch/OFL.txt"_s ),
    GoogleFontDetails( u"Londrina Solid"_s, { u"ofl/londrinasolid/LondrinaSolid-Thin.ttf"_s, u"ofl/londrinasolid/LondrinaSolid-Light.ttf"_s, u"ofl/londrinasolid/LondrinaSolid-Regular.ttf"_s, u"ofl/londrinasolid/LondrinaSolid-Black.ttf"_s }, u"ofl/londrinasolid/OFL.txt"_s ),
    GoogleFontDetails( u"Long Cang"_s, { u"ofl/longcang/LongCang-Regular.ttf"_s }, u"ofl/longcang/OFL.txt"_s ),
    GoogleFontDetails( u"Lora"_s, { u"ofl/lora/Lora%5Bwght%5D.ttf"_s, u"ofl/lora/Lora-Italic%5Bwght%5D.ttf"_s }, u"ofl/lora/OFL.txt"_s ),
    GoogleFontDetails( u"Love Light"_s, { u"ofl/lovelight/LoveLight-Regular.ttf"_s }, u"ofl/lovelight/OFL.txt"_s ),
    GoogleFontDetails( u"Love Ya Like A Sister"_s, { u"ofl/loveyalikeasister/LoveYaLikeASister.ttf"_s }, u"ofl/loveyalikeasister/OFL.txt"_s ),
    GoogleFontDetails( u"Loved by the King"_s, { u"ofl/lovedbytheking/LovedbytheKing.ttf"_s }, u"ofl/lovedbytheking/OFL.txt"_s ),
    GoogleFontDetails( u"Lovers Quarrel"_s, { u"ofl/loversquarrel/LoversQuarrel-Regular.ttf"_s }, u"ofl/loversquarrel/OFL.txt"_s ),
    GoogleFontDetails( u"Luckiest Guy"_s, { u"apache/luckiestguy/LuckiestGuy-Regular.ttf"_s }, u"apache/luckiestguy/LICENSE.txt"_s ),
    GoogleFontDetails( u"Lugrasimo"_s, { u"ofl/lugrasimo/Lugrasimo-Regular.ttf"_s }, u"ofl/lugrasimo/OFL.txt"_s ),
    GoogleFontDetails( u"Lumanosimo"_s, { u"ofl/lumanosimo/Lumanosimo-Regular.ttf"_s }, u"ofl/lumanosimo/OFL.txt"_s ),
    GoogleFontDetails( u"Lunasima"_s, { u"ofl/lunasima/Lunasima-Regular.ttf"_s, u"ofl/lunasima/Lunasima-Bold.ttf"_s }, u"ofl/lunasima/OFL.txt"_s ),
    GoogleFontDetails( u"Lusitana"_s, { u"ofl/lusitana/Lusitana-Regular.ttf"_s, u"ofl/lusitana/Lusitana-Bold.ttf"_s }, u"ofl/lusitana/OFL.txt"_s ),
    GoogleFontDetails( u"Lustria"_s, { u"ofl/lustria/Lustria-Regular.ttf"_s }, u"ofl/lustria/OFL.txt"_s ),
    GoogleFontDetails( u"Luxurious Roman"_s, { u"ofl/luxuriousroman/LuxuriousRoman-Regular.ttf"_s }, u"ofl/luxuriousroman/OFL.txt"_s ),
    GoogleFontDetails( u"Luxurious Script"_s, { u"ofl/luxuriousscript/LuxuriousScript-Regular.ttf"_s }, u"ofl/luxuriousscript/OFL.txt"_s ),
    GoogleFontDetails( u"M PLUS 1"_s, { u"ofl/mplus1/MPLUS1%5Bwght%5D.ttf"_s }, u"ofl/mplus1/OFL.txt"_s ),
    GoogleFontDetails( u"M PLUS 1 Code"_s, { u"ofl/mplus1code/MPLUS1Code%5Bwght%5D.ttf"_s }, u"ofl/mplus1code/OFL.txt"_s ),
    GoogleFontDetails( u"M PLUS 1p"_s, { u"ofl/mplus1p/MPLUS1p-Thin.ttf"_s, u"ofl/mplus1p/MPLUS1p-Light.ttf"_s, u"ofl/mplus1p/MPLUS1p-Regular.ttf"_s, u"ofl/mplus1p/MPLUS1p-Medium.ttf"_s, u"ofl/mplus1p/MPLUS1p-Bold.ttf"_s, u"ofl/mplus1p/MPLUS1p-ExtraBold.ttf"_s, u"ofl/mplus1p/MPLUS1p-Black.ttf"_s }, u"ofl/mplus1p/OFL.txt"_s ),
    GoogleFontDetails( u"M PLUS 2"_s, { u"ofl/mplus2/MPLUS2%5Bwght%5D.ttf"_s }, u"ofl/mplus2/OFL.txt"_s ),
    GoogleFontDetails( u"M PLUS Code Latin"_s, { u"ofl/mpluscodelatin/MPLUSCodeLatin%5Bwdth,wght%5D.ttf"_s }, u"ofl/mpluscodelatin/OFL.txt"_s ),
    GoogleFontDetails( u"Ma Shan Zheng"_s, { u"ofl/mashanzheng/MaShanZheng-Regular.ttf"_s }, u"ofl/mashanzheng/OFL.txt"_s ),
    GoogleFontDetails( u"Macondo"_s, { u"ofl/macondo/Macondo-Regular.ttf"_s }, u"ofl/macondo/OFL.txt"_s ),
    GoogleFontDetails( u"Macondo Swash Caps"_s, { u"ofl/macondoswashcaps/MacondoSwashCaps-Regular.ttf"_s }, u"ofl/macondoswashcaps/OFL.txt"_s ),
    GoogleFontDetails( u"Mada"_s, { u"ofl/mada/Mada%5Bwght%5D.ttf"_s }, u"ofl/mada/OFL.txt"_s ),
    GoogleFontDetails( u"Magra"_s, { u"ofl/magra/Magra-Regular.ttf"_s, u"ofl/magra/Magra-Bold.ttf"_s }, u"ofl/magra/OFL.txt"_s ),
    GoogleFontDetails( u"Maiden Orange"_s, { u"apache/maidenorange/MaidenOrange-Regular.ttf"_s }, u"apache/maidenorange/LICENSE.txt"_s ),
    GoogleFontDetails( u"Maitree"_s, { u"ofl/maitree/Maitree-ExtraLight.ttf"_s, u"ofl/maitree/Maitree-Light.ttf"_s, u"ofl/maitree/Maitree-Regular.ttf"_s, u"ofl/maitree/Maitree-Medium.ttf"_s, u"ofl/maitree/Maitree-SemiBold.ttf"_s, u"ofl/maitree/Maitree-Bold.ttf"_s }, u"ofl/maitree/OFL.txt"_s ),
    GoogleFontDetails( u"Major Mono Display"_s, { u"ofl/majormonodisplay/MajorMonoDisplay-Regular.ttf"_s }, u"ofl/majormonodisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Mako"_s, { u"ofl/mako/Mako-Regular.ttf"_s }, u"ofl/mako/OFL.txt"_s ),
    GoogleFontDetails( u"Mali"_s, { u"ofl/mali/Mali-ExtraLight.ttf"_s, u"ofl/mali/Mali-ExtraLightItalic.ttf"_s, u"ofl/mali/Mali-Light.ttf"_s, u"ofl/mali/Mali-LightItalic.ttf"_s, u"ofl/mali/Mali-Regular.ttf"_s, u"ofl/mali/Mali-Italic.ttf"_s, u"ofl/mali/Mali-Medium.ttf"_s, u"ofl/mali/Mali-MediumItalic.ttf"_s, u"ofl/mali/Mali-SemiBold.ttf"_s, u"ofl/mali/Mali-SemiBoldItalic.ttf"_s, u"ofl/mali/Mali-Bold.ttf"_s, u"ofl/mali/Mali-BoldItalic.ttf"_s }, u"ofl/mali/OFL.txt"_s ),
    GoogleFontDetails( u"Mallanna"_s, { u"ofl/mallanna/Mallanna-Regular.ttf"_s }, u"ofl/mallanna/OFL.txt"_s ),
    GoogleFontDetails( u"Mandali"_s, { u"ofl/mandali/Mandali-Regular.ttf"_s }, u"ofl/mandali/OFL.txt"_s ),
    GoogleFontDetails( u"Manjari"_s, { u"ofl/manjari/Manjari-Thin.ttf"_s, u"ofl/manjari/Manjari-Regular.ttf"_s, u"ofl/manjari/Manjari-Bold.ttf"_s }, u"ofl/manjari/OFL.txt"_s ),
    GoogleFontDetails( u"Manrope"_s, { u"ofl/manrope/Manrope%5Bwght%5D.ttf"_s }, u"ofl/manrope/OFL.txt"_s ),
    GoogleFontDetails( u"Mansalva"_s, { u"ofl/mansalva/Mansalva-Regular.ttf"_s }, u"ofl/mansalva/OFL.txt"_s ),
    GoogleFontDetails( u"Manuale"_s, { u"ofl/manuale/Manuale%5Bwght%5D.ttf"_s, u"ofl/manuale/Manuale-Italic%5Bwght%5D.ttf"_s }, u"ofl/manuale/OFL.txt"_s ),
    GoogleFontDetails( u"Marcellus"_s, { u"ofl/marcellus/Marcellus-Regular.ttf"_s }, u"ofl/marcellus/OFL.txt"_s ),
    GoogleFontDetails( u"Marcellus SC"_s, { u"ofl/marcellussc/MarcellusSC-Regular.ttf"_s }, u"ofl/marcellussc/OFL.txt"_s ),
    GoogleFontDetails( u"Marck Script"_s, { u"ofl/marckscript/MarckScript-Regular.ttf"_s }, u"ofl/marckscript/OFL.txt"_s ),
    GoogleFontDetails( u"Margarine"_s, { u"ofl/margarine/Margarine-Regular.ttf"_s }, u"ofl/margarine/OFL.txt"_s ),
    GoogleFontDetails( u"Marhey"_s, { u"ofl/marhey/Marhey%5Bwght%5D.ttf"_s }, u"ofl/marhey/OFL.txt"_s ),
    GoogleFontDetails( u"Markazi Text"_s, { u"ofl/markazitext/MarkaziText%5Bwght%5D.ttf"_s }, u"ofl/markazitext/OFL.txt"_s ),
    GoogleFontDetails( u"Marko One"_s, { u"ofl/markoone/MarkoOne-Regular.ttf"_s }, u"ofl/markoone/OFL.txt"_s ),
    GoogleFontDetails( u"Marmelad"_s, { u"ofl/marmelad/Marmelad-Regular.ttf"_s }, u"ofl/marmelad/OFL.txt"_s ),
    GoogleFontDetails( u"Martel"_s, { u"ofl/martel/Martel-UltraLight.ttf"_s, u"ofl/martel/Martel-Light.ttf"_s, u"ofl/martel/Martel-Regular.ttf"_s, u"ofl/martel/Martel-DemiBold.ttf"_s, u"ofl/martel/Martel-Bold.ttf"_s, u"ofl/martel/Martel-ExtraBold.ttf"_s, u"ofl/martel/Martel-Heavy.ttf"_s }, u"ofl/martel/OFL.txt"_s ),
    GoogleFontDetails( u"Martel Sans"_s, { u"ofl/martelsans/MartelSans-ExtraLight.ttf"_s, u"ofl/martelsans/MartelSans-Light.ttf"_s, u"ofl/martelsans/MartelSans-Regular.ttf"_s, u"ofl/martelsans/MartelSans-SemiBold.ttf"_s, u"ofl/martelsans/MartelSans-Bold.ttf"_s, u"ofl/martelsans/MartelSans-ExtraBold.ttf"_s, u"ofl/martelsans/MartelSans-Black.ttf"_s }, u"ofl/martelsans/OFL.txt"_s ),
    GoogleFontDetails( u"Martian Mono"_s, { u"ofl/martianmono/MartianMono%5Bwdth,wght%5D.ttf"_s }, u"ofl/martianmono/OFL.txt"_s ),
    GoogleFontDetails( u"Marvel"_s, { u"ofl/marvel/Marvel-Regular.ttf"_s, u"ofl/marvel/Marvel-Italic.ttf"_s, u"ofl/marvel/Marvel-Bold.ttf"_s, u"ofl/marvel/Marvel-BoldItalic.ttf"_s }, u"ofl/marvel/OFL.txt"_s ),
    GoogleFontDetails( u"Mate"_s, { u"ofl/mate/Mate-Regular.ttf"_s, u"ofl/mate/Mate-Italic.ttf"_s }, u"ofl/mate/OFL.txt"_s ),
    GoogleFontDetails( u"Mate SC"_s, { u"ofl/matesc/MateSC-Regular.ttf"_s }, u"ofl/matesc/OFL.txt"_s ),
    GoogleFontDetails( u"Maven Pro"_s, { u"ofl/mavenpro/MavenPro%5Bwght%5D.ttf"_s }, u"ofl/mavenpro/OFL.txt"_s ),
    GoogleFontDetails( u"McLaren"_s, { u"ofl/mclaren/McLaren-Regular.ttf"_s }, u"ofl/mclaren/OFL.txt"_s ),
    GoogleFontDetails( u"Mea Culpa"_s, { u"ofl/meaculpa/MeaCulpa-Regular.ttf"_s }, u"ofl/meaculpa/OFL.txt"_s ),
    GoogleFontDetails( u"Meddon"_s, { u"ofl/meddon/Meddon.ttf"_s }, u"ofl/meddon/OFL.txt"_s ),
    GoogleFontDetails( u"MedievalSharp"_s, { u"ofl/medievalsharp/MedievalSharp.ttf"_s }, u"ofl/medievalsharp/OFL.txt"_s ),
    GoogleFontDetails( u"Medula One"_s, { u"ofl/medulaone/MedulaOne-Regular.ttf"_s }, u"ofl/medulaone/OFL.txt"_s ),
    GoogleFontDetails( u"Meera Inimai"_s, { u"ofl/meerainimai/MeeraInimai-Regular.ttf"_s }, u"ofl/meerainimai/OFL.txt"_s ),
    GoogleFontDetails( u"Megrim"_s, { u"ofl/megrim/Megrim.ttf"_s }, u"ofl/megrim/OFL.txt"_s ),
    GoogleFontDetails( u"Meie Script"_s, { u"ofl/meiescript/MeieScript-Regular.ttf"_s }, u"ofl/meiescript/OFL.txt"_s ),
    GoogleFontDetails( u"Meow Script"_s, { u"ofl/meowscript/MeowScript-Regular.ttf"_s }, u"ofl/meowscript/OFL.txt"_s ),
    GoogleFontDetails( u"Merienda"_s, { u"ofl/merienda/Merienda%5Bwght%5D.ttf"_s }, u"ofl/merienda/OFL.txt"_s ),
    GoogleFontDetails( u"Merriweather"_s, { u"ofl/merriweather/Merriweather-Light.ttf"_s, u"ofl/merriweather/Merriweather-LightItalic.ttf"_s, u"ofl/merriweather/Merriweather-Regular.ttf"_s, u"ofl/merriweather/Merriweather-Italic.ttf"_s, u"ofl/merriweather/Merriweather-Bold.ttf"_s, u"ofl/merriweather/Merriweather-BoldItalic.ttf"_s, u"ofl/merriweather/Merriweather-Black.ttf"_s, u"ofl/merriweather/Merriweather-BlackItalic.ttf"_s }, u"ofl/merriweather/OFL.txt"_s ),
    GoogleFontDetails( u"Merriweather Sans"_s, { u"ofl/merriweathersans/MerriweatherSans%5Bwght%5D.ttf"_s, u"ofl/merriweathersans/MerriweatherSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/merriweathersans/OFL.txt"_s ),
    GoogleFontDetails( u"Metal"_s, { u"ofl/metal/Metal-Regular.ttf"_s }, u"ofl/metal/OFL.txt"_s ),
    GoogleFontDetails( u"Metal Mania"_s, { u"ofl/metalmania/MetalMania-Regular.ttf"_s }, u"ofl/metalmania/OFL.txt"_s ),
    GoogleFontDetails( u"Metamorphous"_s, { u"ofl/metamorphous/Metamorphous-Regular.ttf"_s }, u"ofl/metamorphous/OFL.txt"_s ),
    GoogleFontDetails( u"Metrophobic"_s, { u"ofl/metrophobic/Metrophobic-Regular.ttf"_s }, u"ofl/metrophobic/OFL.txt"_s ),
    GoogleFontDetails( u"Michroma"_s, { u"ofl/michroma/Michroma-Regular.ttf"_s }, u"ofl/michroma/OFL.txt"_s ),
    GoogleFontDetails( u"Milonga"_s, { u"ofl/milonga/Milonga-Regular.ttf"_s }, u"ofl/milonga/OFL.txt"_s ),
    GoogleFontDetails( u"Miltonian"_s, { u"ofl/miltonian/Miltonian-Regular.ttf"_s }, u"ofl/miltonian/OFL.txt"_s ),
    GoogleFontDetails( u"Miltonian Tattoo"_s, { u"ofl/miltoniantattoo/MiltonianTattoo-Regular.ttf"_s }, u"ofl/miltoniantattoo/OFL.txt"_s ),
    GoogleFontDetails( u"Mina"_s, { u"ofl/mina/Mina-Regular.ttf"_s, u"ofl/mina/Mina-Bold.ttf"_s }, u"ofl/mina/OFL.txt"_s ),
    GoogleFontDetails( u"Mingzat"_s, { u"ofl/mingzat/Mingzat-Regular.ttf"_s }, u"ofl/mingzat/OFL.txt"_s ),
    GoogleFontDetails( u"Miniver"_s, { u"ofl/miniver/Miniver-Regular.ttf"_s }, u"ofl/miniver/OFL.txt"_s ),
    GoogleFontDetails( u"Miriam Libre"_s, { u"ofl/miriamlibre/MiriamLibre%5Bwght%5D.ttf"_s }, u"ofl/miriamlibre/OFL.txt"_s ),
    GoogleFontDetails( u"Miss Fajardose"_s, { u"ofl/missfajardose/MissFajardose-Regular.ttf"_s }, u"ofl/missfajardose/OFL.txt"_s ),
    GoogleFontDetails( u"Mochiy Pop One"_s, { u"ofl/mochiypopone/MochiyPopOne-Regular.ttf"_s }, u"ofl/mochiypopone/OFL.txt"_s ),
    GoogleFontDetails( u"Mochiy Pop P One"_s, { u"ofl/mochiypoppone/MochiyPopPOne-Regular.ttf"_s }, u"ofl/mochiypoppone/OFL.txt"_s ),
    GoogleFontDetails( u"Modak"_s, { u"ofl/modak/Modak-Regular.ttf"_s }, u"ofl/modak/OFL.txt"_s ),
    GoogleFontDetails( u"Modern Antiqua"_s, { u"ofl/modernantiqua/ModernAntiqua-Regular.ttf"_s }, u"ofl/modernantiqua/OFL.txt"_s ),
    GoogleFontDetails( u"Mohave"_s, { u"ofl/mohave/Mohave%5Bwght%5D.ttf"_s, u"ofl/mohave/Mohave-Italic%5Bwght%5D.ttf"_s }, u"ofl/mohave/OFL.txt"_s ),
    GoogleFontDetails( u"Moirai One"_s, { u"ofl/moiraione/MoiraiOne-Regular.ttf"_s }, u"ofl/moiraione/OFL.txt"_s ),
    GoogleFontDetails( u"Molengo"_s, { u"ofl/molengo/Molengo-Regular.ttf"_s }, u"ofl/molengo/OFL.txt"_s ),
    GoogleFontDetails( u"Molle"_s, { u"ofl/molle/Molle-Regular.ttf"_s }, u"ofl/molle/OFL.txt"_s ),
    GoogleFontDetails( u"Monda"_s, { u"ofl/monda/Monda%5Bwght%5D.ttf"_s }, u"ofl/monda/OFL.txt"_s ),
    GoogleFontDetails( u"Monofett"_s, { u"ofl/monofett/Monofett-Regular.ttf"_s }, u"ofl/monofett/OFL.txt"_s ),
    GoogleFontDetails( u"Monomaniac One"_s, { u"ofl/monomaniacone/MonomaniacOne-Regular.ttf"_s }, u"ofl/monomaniacone/OFL.txt"_s ),
    GoogleFontDetails( u"Monoton"_s, { u"ofl/monoton/Monoton-Regular.ttf"_s }, u"ofl/monoton/OFL.txt"_s ),
    GoogleFontDetails( u"Monsieur La Doulaise"_s, { u"ofl/monsieurladoulaise/MonsieurLaDoulaise-Regular.ttf"_s }, u"ofl/monsieurladoulaise/OFL.txt"_s ),
    GoogleFontDetails( u"Montaga"_s, { u"ofl/montaga/Montaga-Regular.ttf"_s }, u"ofl/montaga/OFL.txt"_s ),
    GoogleFontDetails( u"Montagu Slab"_s, { u"ofl/montaguslab/MontaguSlab%5Bopsz,wght%5D.ttf"_s }, u"ofl/montaguslab/OFL.txt"_s ),
    GoogleFontDetails( u"MonteCarlo"_s, { u"ofl/montecarlo/MonteCarlo-Regular.ttf"_s }, u"ofl/montecarlo/OFL.txt"_s ),
    GoogleFontDetails( u"Montez"_s, { u"apache/montez/Montez-Regular.ttf"_s }, u"apache/montez/LICENSE.txt"_s ),
    GoogleFontDetails( u"Montserrat"_s, { u"ofl/montserrat/Montserrat%5Bwght%5D.ttf"_s, u"ofl/montserrat/Montserrat-Italic%5Bwght%5D.ttf"_s }, u"ofl/montserrat/OFL.txt"_s ),
    GoogleFontDetails( u"Montserrat Alternates"_s, { u"ofl/montserratalternates/MontserratAlternates-Thin.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-ThinItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-ExtraLight.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-ExtraLightItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-Light.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-LightItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-Regular.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-Italic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-Medium.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-MediumItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-SemiBold.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-SemiBoldItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-Bold.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-BoldItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-ExtraBold.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-ExtraBoldItalic.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-Black.ttf"_s, u"ofl/montserratalternates/MontserratAlternates-BlackItalic.ttf"_s }, u"ofl/montserratalternates/OFL.txt"_s ),
    GoogleFontDetails( u"Montserrat Subrayada"_s, { u"ofl/montserratsubrayada/MontserratSubrayada-Regular.ttf"_s, u"ofl/montserratsubrayada/MontserratSubrayada-Bold.ttf"_s }, u"ofl/montserratsubrayada/OFL.txt"_s ),
    GoogleFontDetails( u"Moo Lah Lah"_s, { u"ofl/moolahlah/MooLahLah-Regular.ttf"_s }, u"ofl/moolahlah/OFL.txt"_s ),
    GoogleFontDetails( u"Moon Dance"_s, { u"ofl/moondance/MoonDance-Regular.ttf"_s }, u"ofl/moondance/OFL.txt"_s ),
    GoogleFontDetails( u"Moul"_s, { u"ofl/moul/Moul-Regular.ttf"_s }, u"ofl/moul/OFL.txt"_s ),
    GoogleFontDetails( u"Moulpali"_s, { u"ofl/moulpali/Moulpali-Regular.ttf"_s }, u"ofl/moulpali/OFL.txt"_s ),
    GoogleFontDetails( u"Mountains of Christmas"_s, { u"apache/mountainsofchristmas/MountainsofChristmas-Regular.ttf"_s, u"apache/mountainsofchristmas/MountainsofChristmas-Bold.ttf"_s }, u"apache/mountainsofchristmas/LICENSE.txt"_s ),
    GoogleFontDetails( u"Mouse Memoirs"_s, { u"ofl/mousememoirs/MouseMemoirs-Regular.ttf"_s }, u"ofl/mousememoirs/OFL.txt"_s ),
    GoogleFontDetails( u"Mr Bedfort"_s, { u"ofl/mrbedfort/MrBedfort-Regular.ttf"_s }, u"ofl/mrbedfort/OFL.txt"_s ),
    GoogleFontDetails( u"Mr Dafoe"_s, { u"ofl/mrdafoe/MrDafoe-Regular.ttf"_s }, u"ofl/mrdafoe/OFL.txt"_s ),
    GoogleFontDetails( u"Mr De Haviland"_s, { u"ofl/mrdehaviland/MrDeHaviland-Regular.ttf"_s }, u"ofl/mrdehaviland/OFL.txt"_s ),
    GoogleFontDetails( u"Mrs Saint Delafield"_s, { u"ofl/mrssaintdelafield/MrsSaintDelafield-Regular.ttf"_s }, u"ofl/mrssaintdelafield/OFL.txt"_s ),
    GoogleFontDetails( u"Mrs Sheppards"_s, { u"ofl/mrssheppards/MrsSheppards-Regular.ttf"_s }, u"ofl/mrssheppards/OFL.txt"_s ),
    GoogleFontDetails( u"Ms Madi"_s, { u"ofl/msmadi/MsMadi-Regular.ttf"_s }, u"ofl/msmadi/OFL.txt"_s ),
    GoogleFontDetails( u"Mukta"_s, { u"ofl/mukta/Mukta-ExtraLight.ttf"_s, u"ofl/mukta/Mukta-Light.ttf"_s, u"ofl/mukta/Mukta-Regular.ttf"_s, u"ofl/mukta/Mukta-Medium.ttf"_s, u"ofl/mukta/Mukta-SemiBold.ttf"_s, u"ofl/mukta/Mukta-Bold.ttf"_s, u"ofl/mukta/Mukta-ExtraBold.ttf"_s }, u"ofl/mukta/OFL.txt"_s ),
    GoogleFontDetails( u"Mukta Mahee"_s, { u"ofl/muktamahee/MuktaMahee-ExtraLight.ttf"_s, u"ofl/muktamahee/MuktaMahee-Light.ttf"_s, u"ofl/muktamahee/MuktaMahee-Regular.ttf"_s, u"ofl/muktamahee/MuktaMahee-Medium.ttf"_s, u"ofl/muktamahee/MuktaMahee-SemiBold.ttf"_s, u"ofl/muktamahee/MuktaMahee-Bold.ttf"_s, u"ofl/muktamahee/MuktaMahee-ExtraBold.ttf"_s }, u"ofl/muktamahee/OFL.txt"_s ),
    GoogleFontDetails( u"Mukta Malar"_s, { u"ofl/muktamalar/MuktaMalar-ExtraLight.ttf"_s, u"ofl/muktamalar/MuktaMalar-Light.ttf"_s, u"ofl/muktamalar/MuktaMalar-Regular.ttf"_s, u"ofl/muktamalar/MuktaMalar-Medium.ttf"_s, u"ofl/muktamalar/MuktaMalar-SemiBold.ttf"_s, u"ofl/muktamalar/MuktaMalar-Bold.ttf"_s, u"ofl/muktamalar/MuktaMalar-ExtraBold.ttf"_s }, u"ofl/muktamalar/OFL.txt"_s ),
    GoogleFontDetails( u"Mukta Vaani"_s, { u"ofl/muktavaani/MuktaVaani-ExtraLight.ttf"_s, u"ofl/muktavaani/MuktaVaani-Light.ttf"_s, u"ofl/muktavaani/MuktaVaani-Regular.ttf"_s, u"ofl/muktavaani/MuktaVaani-Medium.ttf"_s, u"ofl/muktavaani/MuktaVaani-SemiBold.ttf"_s, u"ofl/muktavaani/MuktaVaani-Bold.ttf"_s, u"ofl/muktavaani/MuktaVaani-ExtraBold.ttf"_s }, u"ofl/muktavaani/OFL.txt"_s ),
    GoogleFontDetails( u"Mulish"_s, { u"ofl/mulish/Mulish%5Bwght%5D.ttf"_s, u"ofl/mulish/Mulish-Italic%5Bwght%5D.ttf"_s }, u"ofl/mulish/OFL.txt"_s ),
    GoogleFontDetails( u"Murecho"_s, { u"ofl/murecho/Murecho%5Bwght%5D.ttf"_s }, u"ofl/murecho/OFL.txt"_s ),
    GoogleFontDetails( u"MuseoModerno"_s, { u"ofl/museomoderno/MuseoModerno%5Bwght%5D.ttf"_s, u"ofl/museomoderno/MuseoModerno-Italic%5Bwght%5D.ttf"_s }, u"ofl/museomoderno/OFL.txt"_s ),
    GoogleFontDetails( u"My Soul"_s, { u"ofl/mysoul/MySoul-Regular.ttf"_s }, u"ofl/mysoul/OFL.txt"_s ),
    GoogleFontDetails( u"Mynerve"_s, { u"ofl/mynerve/Mynerve-Regular.ttf"_s }, u"ofl/mynerve/OFL.txt"_s ),
    GoogleFontDetails( u"Mystery Quest"_s, { u"ofl/mysteryquest/MysteryQuest-Regular.ttf"_s }, u"ofl/mysteryquest/OFL.txt"_s ),
    GoogleFontDetails( u"NTR"_s, { u"ofl/ntr/NTR-Regular.ttf"_s }, u"ofl/ntr/OFL.txt"_s ),
    GoogleFontDetails( u"Nabla"_s, { u"ofl/nabla/Nabla%5BEDPT,EHLT%5D.ttf"_s }, u"ofl/nabla/OFL.txt"_s ),
    GoogleFontDetails( u"Nanum Brush Script"_s, { u"ofl/nanumbrushscript/NanumBrushScript-Regular.ttf"_s }, u"ofl/nanumbrushscript/OFL.txt"_s ),
    GoogleFontDetails( u"Nanum Gothic"_s, { u"ofl/nanumgothic/NanumGothic-Regular.ttf"_s, u"ofl/nanumgothic/NanumGothic-Bold.ttf"_s, u"ofl/nanumgothic/NanumGothic-ExtraBold.ttf"_s }, u"ofl/nanumgothic/OFL.txt"_s ),
    GoogleFontDetails( u"Nanum Gothic Coding"_s, { u"ofl/nanumgothiccoding/NanumGothicCoding-Regular.ttf"_s, u"ofl/nanumgothiccoding/NanumGothicCoding-Bold.ttf"_s }, u"ofl/nanumgothiccoding/OFL.txt"_s ),
    GoogleFontDetails( u"Nanum Myeongjo"_s, { u"ofl/nanummyeongjo/NanumMyeongjo-Regular.ttf"_s, u"ofl/nanummyeongjo/NanumMyeongjo-Bold.ttf"_s, u"ofl/nanummyeongjo/NanumMyeongjo-ExtraBold.ttf"_s }, u"ofl/nanummyeongjo/OFL.txt"_s ),
    GoogleFontDetails( u"Nanum Pen Script"_s, { u"ofl/nanumpenscript/NanumPenScript-Regular.ttf"_s }, u"ofl/nanumpenscript/OFL.txt"_s ),
    GoogleFontDetails( u"Narnoor"_s, { u"ofl/narnoor/Narnoor-Regular.ttf"_s, u"ofl/narnoor/Narnoor-Medium.ttf"_s, u"ofl/narnoor/Narnoor-SemiBold.ttf"_s, u"ofl/narnoor/Narnoor-Bold.ttf"_s, u"ofl/narnoor/Narnoor-ExtraBold.ttf"_s }, u"ofl/narnoor/OFL.txt"_s ),
    GoogleFontDetails( u"Neonderthaw"_s, { u"ofl/neonderthaw/Neonderthaw-Regular.ttf"_s }, u"ofl/neonderthaw/OFL.txt"_s ),
    GoogleFontDetails( u"Nerko One"_s, { u"ofl/nerkoone/NerkoOne-Regular.ttf"_s }, u"ofl/nerkoone/OFL.txt"_s ),
    GoogleFontDetails( u"Neucha"_s, { u"ofl/neucha/Neucha.ttf"_s }, u"ofl/neucha/OFL.txt"_s ),
    GoogleFontDetails( u"Neuton"_s, { u"ofl/neuton/Neuton-ExtraLight.ttf"_s, u"ofl/neuton/Neuton-Light.ttf"_s, u"ofl/neuton/Neuton-Regular.ttf"_s, u"ofl/neuton/Neuton-Italic.ttf"_s, u"ofl/neuton/Neuton-Bold.ttf"_s, u"ofl/neuton/Neuton-ExtraBold.ttf"_s }, u"ofl/neuton/OFL.txt"_s ),
    GoogleFontDetails( u"New Rocker"_s, { u"ofl/newrocker/NewRocker-Regular.ttf"_s }, u"ofl/newrocker/OFL.txt"_s ),
    GoogleFontDetails( u"New Tegomin"_s, { u"ofl/newtegomin/NewTegomin-Regular.ttf"_s }, u"ofl/newtegomin/OFL.txt"_s ),
    GoogleFontDetails( u"News Cycle"_s, { u"ofl/newscycle/NewsCycle-Regular.ttf"_s, u"ofl/newscycle/NewsCycle-Bold.ttf"_s }, u"ofl/newscycle/OFL.txt"_s ),
    GoogleFontDetails( u"Newsreader"_s, { u"ofl/newsreader/Newsreader%5Bopsz,wght%5D.ttf"_s, u"ofl/newsreader/Newsreader-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/newsreader/OFL.txt"_s ),
    GoogleFontDetails( u"Niconne"_s, { u"ofl/niconne/Niconne-Regular.ttf"_s }, u"ofl/niconne/OFL.txt"_s ),
    GoogleFontDetails( u"Niramit"_s, { u"ofl/niramit/Niramit-ExtraLight.ttf"_s, u"ofl/niramit/Niramit-ExtraLightItalic.ttf"_s, u"ofl/niramit/Niramit-Light.ttf"_s, u"ofl/niramit/Niramit-LightItalic.ttf"_s, u"ofl/niramit/Niramit-Regular.ttf"_s, u"ofl/niramit/Niramit-Italic.ttf"_s, u"ofl/niramit/Niramit-Medium.ttf"_s, u"ofl/niramit/Niramit-MediumItalic.ttf"_s, u"ofl/niramit/Niramit-SemiBold.ttf"_s, u"ofl/niramit/Niramit-SemiBoldItalic.ttf"_s, u"ofl/niramit/Niramit-Bold.ttf"_s, u"ofl/niramit/Niramit-BoldItalic.ttf"_s }, u"ofl/niramit/OFL.txt"_s ),
    GoogleFontDetails( u"Nixie One"_s, { u"ofl/nixieone/NixieOne-Regular.ttf"_s }, u"ofl/nixieone/OFL.txt"_s ),
    GoogleFontDetails( u"Nobile"_s, { u"ofl/nobile/Nobile-Regular.ttf"_s, u"ofl/nobile/Nobile-Italic.ttf"_s, u"ofl/nobile/Nobile-Medium.ttf"_s, u"ofl/nobile/Nobile-MediumItalic.ttf"_s, u"ofl/nobile/Nobile-Bold.ttf"_s, u"ofl/nobile/Nobile-BoldItalic.ttf"_s }, u"ofl/nobile/OFL.txt"_s ),
    GoogleFontDetails( u"Nokora"_s, { u"ofl/nokora/Nokora-Thin.ttf"_s, u"ofl/nokora/Nokora-Light.ttf"_s, u"ofl/nokora/Nokora-Regular.ttf"_s, u"ofl/nokora/Nokora-Bold.ttf"_s, u"ofl/nokora/Nokora-Black.ttf"_s }, u"ofl/nokora/OFL.txt"_s ),
    GoogleFontDetails( u"Norican"_s, { u"ofl/norican/Norican-Regular.ttf"_s }, u"ofl/norican/OFL.txt"_s ),
    GoogleFontDetails( u"Nosifer"_s, { u"ofl/nosifer/Nosifer-Regular.ttf"_s }, u"ofl/nosifer/OFL.txt"_s ),
    GoogleFontDetails( u"Notable"_s, { u"ofl/notable/Notable-Regular.ttf"_s }, u"ofl/notable/OFL.txt"_s ),
    GoogleFontDetails( u"Nothing You Could Do"_s, { u"ofl/nothingyoucoulddo/NothingYouCouldDo.ttf"_s }, u"ofl/nothingyoucoulddo/OFL.txt"_s ),
    GoogleFontDetails( u"Noticia Text"_s, { u"ofl/noticiatext/NoticiaText-Regular.ttf"_s, u"ofl/noticiatext/NoticiaText-Italic.ttf"_s, u"ofl/noticiatext/NoticiaText-Bold.ttf"_s, u"ofl/noticiatext/NoticiaText-BoldItalic.ttf"_s }, u"ofl/noticiatext/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Color Emoji"_s, { u"ofl/notocoloremoji/NotoColorEmoji-Regular.ttf"_s }, u"ofl/notocoloremoji/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Emoji"_s, { u"ofl/notoemoji/NotoEmoji%5Bwght%5D.ttf"_s }, u"ofl/notoemoji/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Kufi Arabic"_s, { u"ofl/notokufiarabic/NotoKufiArabic%5Bwght%5D.ttf"_s }, u"ofl/notokufiarabic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Music"_s, { u"ofl/notomusic/NotoMusic-Regular.ttf"_s }, u"ofl/notomusic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Naskh Arabic"_s, { u"ofl/notonaskharabic/NotoNaskhArabic%5Bwght%5D.ttf"_s }, u"ofl/notonaskharabic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Nastaliq Urdu"_s, { u"ofl/notonastaliqurdu/NotoNastaliqUrdu%5Bwght%5D.ttf"_s }, u"ofl/notonastaliqurdu/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Rashi Hebrew"_s, { u"ofl/notorashihebrew/NotoRashiHebrew%5Bwght%5D.ttf"_s }, u"ofl/notorashihebrew/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans"_s, { u"ofl/notosans/NotoSans%5Bwdth,wght%5D.ttf"_s, u"ofl/notosans/NotoSans-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosans/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Adlam"_s, { u"ofl/notosansadlam/NotoSansAdlam%5Bwght%5D.ttf"_s }, u"ofl/notosansadlam/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Adlam Unjoined"_s, { u"ofl/notosansadlamunjoined/NotoSansAdlamUnjoined%5Bwght%5D.ttf"_s }, u"ofl/notosansadlamunjoined/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Anatolian Hieroglyphs"_s, { u"ofl/notosansanatolianhieroglyphs/NotoSansAnatolianHieroglyphs-Regular.ttf"_s }, u"ofl/notosansanatolianhieroglyphs/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Arabic"_s, { u"ofl/notosansarabic/NotoSansArabic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansarabic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Armenian"_s, { u"ofl/notosansarmenian/NotoSansArmenian%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansarmenian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Avestan"_s, { u"ofl/notosansavestan/NotoSansAvestan-Regular.ttf"_s }, u"ofl/notosansavestan/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Balinese"_s, { u"ofl/notosansbalinese/NotoSansBalinese%5Bwght%5D.ttf"_s }, u"ofl/notosansbalinese/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Bamum"_s, { u"ofl/notosansbamum/NotoSansBamum%5Bwght%5D.ttf"_s }, u"ofl/notosansbamum/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Bassa Vah"_s, { u"ofl/notosansbassavah/NotoSansBassaVah%5Bwght%5D.ttf"_s }, u"ofl/notosansbassavah/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Batak"_s, { u"ofl/notosansbatak/NotoSansBatak-Regular.ttf"_s }, u"ofl/notosansbatak/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Bengali"_s, { u"ofl/notosansbengali/NotoSansBengali%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansbengali/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Bhaiksuki"_s, { u"ofl/notosansbhaiksuki/NotoSansBhaiksuki-Regular.ttf"_s }, u"ofl/notosansbhaiksuki/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Brahmi"_s, { u"ofl/notosansbrahmi/NotoSansBrahmi-Regular.ttf"_s }, u"ofl/notosansbrahmi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Buginese"_s, { u"ofl/notosansbuginese/NotoSansBuginese-Regular.ttf"_s }, u"ofl/notosansbuginese/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Buhid"_s, { u"ofl/notosansbuhid/NotoSansBuhid-Regular.ttf"_s }, u"ofl/notosansbuhid/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Canadian Aboriginal"_s, { u"ofl/notosanscanadianaboriginal/NotoSansCanadianAboriginal%5Bwght%5D.ttf"_s }, u"ofl/notosanscanadianaboriginal/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Carian"_s, { u"ofl/notosanscarian/NotoSansCarian-Regular.ttf"_s }, u"ofl/notosanscarian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Caucasian Albanian"_s, { u"ofl/notosanscaucasianalbanian/NotoSansCaucasianAlbanian-Regular.ttf"_s }, u"ofl/notosanscaucasianalbanian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Chakma"_s, { u"ofl/notosanschakma/NotoSansChakma-Regular.ttf"_s }, u"ofl/notosanschakma/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Cham"_s, { u"ofl/notosanscham/NotoSansCham%5Bwght%5D.ttf"_s }, u"ofl/notosanscham/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Cherokee"_s, { u"ofl/notosanscherokee/NotoSansCherokee%5Bwght%5D.ttf"_s }, u"ofl/notosanscherokee/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Chorasmian"_s, { u"ofl/notosanschorasmian/NotoSansChorasmian-Regular.ttf"_s }, u"ofl/notosanschorasmian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Coptic"_s, { u"ofl/notosanscoptic/NotoSansCoptic-Regular.ttf"_s }, u"ofl/notosanscoptic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Cuneiform"_s, { u"ofl/notosanscuneiform/NotoSansCuneiform-Regular.ttf"_s }, u"ofl/notosanscuneiform/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Cypriot"_s, { u"ofl/notosanscypriot/NotoSansCypriot-Regular.ttf"_s }, u"ofl/notosanscypriot/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Cypro Minoan"_s, { u"ofl/notosanscyprominoan/NotoSansCyproMinoan-Regular.ttf"_s }, u"ofl/notosanscyprominoan/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Deseret"_s, { u"ofl/notosansdeseret/NotoSansDeseret-Regular.ttf"_s }, u"ofl/notosansdeseret/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Devanagari"_s, { u"ofl/notosansdevanagari/NotoSansDevanagari%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansdevanagari/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Display"_s, { u"ofl/notosansdisplay/NotoSansDisplay%5Bwdth,wght%5D.ttf"_s, u"ofl/notosansdisplay/NotoSansDisplay-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Duployan"_s, { u"ofl/notosansduployan/NotoSansDuployan-Regular.ttf"_s, u"ofl/notosansduployan/NotoSansDuployan-Bold.ttf"_s }, u"ofl/notosansduployan/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Egyptian Hieroglyphs"_s, { u"ofl/notosansegyptianhieroglyphs/NotoSansEgyptianHieroglyphs-Regular.ttf"_s }, u"ofl/notosansegyptianhieroglyphs/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Elbasan"_s, { u"ofl/notosanselbasan/NotoSansElbasan-Regular.ttf"_s }, u"ofl/notosanselbasan/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Elymaic"_s, { u"ofl/notosanselymaic/NotoSansElymaic-Regular.ttf"_s }, u"ofl/notosanselymaic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Ethiopic"_s, { u"ofl/notosansethiopic/NotoSansEthiopic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansethiopic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Georgian"_s, { u"ofl/notosansgeorgian/NotoSansGeorgian%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansgeorgian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Glagolitic"_s, { u"ofl/notosansglagolitic/NotoSansGlagolitic-Regular.ttf"_s }, u"ofl/notosansglagolitic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Gothic"_s, { u"ofl/notosansgothic/NotoSansGothic-Regular.ttf"_s }, u"ofl/notosansgothic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Grantha"_s, { u"ofl/notosansgrantha/NotoSansGrantha-Regular.ttf"_s }, u"ofl/notosansgrantha/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Gujarati"_s, { u"ofl/notosansgujarati/NotoSansGujarati%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansgujarati/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Gunjala Gondi"_s, { u"ofl/notosansgunjalagondi/NotoSansGunjalaGondi%5Bwght%5D.ttf"_s }, u"ofl/notosansgunjalagondi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Gurmukhi"_s, { u"ofl/notosansgurmukhi/NotoSansGurmukhi%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansgurmukhi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans HK"_s, { u"ofl/notosanshk/NotoSansHK%5Bwght%5D.ttf"_s }, u"ofl/notosanshk/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Hanifi Rohingya"_s, { u"ofl/notosanshanifirohingya/NotoSansHanifiRohingya%5Bwght%5D.ttf"_s }, u"ofl/notosanshanifirohingya/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Hanunoo"_s, { u"ofl/notosanshanunoo/NotoSansHanunoo-Regular.ttf"_s }, u"ofl/notosanshanunoo/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Hatran"_s, { u"ofl/notosanshatran/NotoSansHatran-Regular.ttf"_s }, u"ofl/notosanshatran/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Hebrew"_s, { u"ofl/notosanshebrew/NotoSansHebrew%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanshebrew/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Imperial Aramaic"_s, { u"ofl/notosansimperialaramaic/NotoSansImperialAramaic-Regular.ttf"_s }, u"ofl/notosansimperialaramaic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Indic Siyaq Numbers"_s, { u"ofl/notosansindicsiyaqnumbers/NotoSansIndicSiyaqNumbers-Regular.ttf"_s }, u"ofl/notosansindicsiyaqnumbers/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Inscriptional Pahlavi"_s, { u"ofl/notosansinscriptionalpahlavi/NotoSansInscriptionalPahlavi-Regular.ttf"_s }, u"ofl/notosansinscriptionalpahlavi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Inscriptional Parthian"_s, { u"ofl/notosansinscriptionalparthian/NotoSansInscriptionalParthian-Regular.ttf"_s }, u"ofl/notosansinscriptionalparthian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans JP"_s, { u"ofl/notosansjp/NotoSansJP%5Bwght%5D.ttf"_s }, u"ofl/notosansjp/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Javanese"_s, { u"ofl/notosansjavanese/NotoSansJavanese%5Bwght%5D.ttf"_s }, u"ofl/notosansjavanese/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans KR"_s, { u"ofl/notosanskr/NotoSansKR%5Bwght%5D.ttf"_s }, u"ofl/notosanskr/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Kaithi"_s, { u"ofl/notosanskaithi/NotoSansKaithi-Regular.ttf"_s }, u"ofl/notosanskaithi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Kannada"_s, { u"ofl/notosanskannada/NotoSansKannada%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanskannada/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Kayah Li"_s, { u"ofl/notosanskayahli/NotoSansKayahLi%5Bwght%5D.ttf"_s }, u"ofl/notosanskayahli/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Kharoshthi"_s, { u"ofl/notosanskharoshthi/NotoSansKharoshthi-Regular.ttf"_s }, u"ofl/notosanskharoshthi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Khmer"_s, { u"ofl/notosanskhmer/NotoSansKhmer%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanskhmer/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Khojki"_s, { u"ofl/notosanskhojki/NotoSansKhojki-Regular.ttf"_s }, u"ofl/notosanskhojki/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Khudawadi"_s, { u"ofl/notosanskhudawadi/NotoSansKhudawadi-Regular.ttf"_s }, u"ofl/notosanskhudawadi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Lao"_s, { u"ofl/notosanslao/NotoSansLao%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanslao/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Lao Looped"_s, { u"ofl/notosanslaolooped/NotoSansLaoLooped%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanslaolooped/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Lepcha"_s, { u"ofl/notosanslepcha/NotoSansLepcha-Regular.ttf"_s }, u"ofl/notosanslepcha/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Limbu"_s, { u"ofl/notosanslimbu/NotoSansLimbu-Regular.ttf"_s }, u"ofl/notosanslimbu/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Linear A"_s, { u"ofl/notosanslineara/NotoSansLinearA-Regular.ttf"_s }, u"ofl/notosanslineara/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Linear B"_s, { u"ofl/notosanslinearb/NotoSansLinearB-Regular.ttf"_s }, u"ofl/notosanslinearb/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Lisu"_s, { u"ofl/notosanslisu/NotoSansLisu%5Bwght%5D.ttf"_s }, u"ofl/notosanslisu/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Lycian"_s, { u"ofl/notosanslycian/NotoSansLycian-Regular.ttf"_s }, u"ofl/notosanslycian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Lydian"_s, { u"ofl/notosanslydian/NotoSansLydian-Regular.ttf"_s }, u"ofl/notosanslydian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mahajani"_s, { u"ofl/notosansmahajani/NotoSansMahajani-Regular.ttf"_s }, u"ofl/notosansmahajani/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Malayalam"_s, { u"ofl/notosansmalayalam/NotoSansMalayalam%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansmalayalam/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mandaic"_s, { u"ofl/notosansmandaic/NotoSansMandaic-Regular.ttf"_s }, u"ofl/notosansmandaic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Manichaean"_s, { u"ofl/notosansmanichaean/NotoSansManichaean-Regular.ttf"_s }, u"ofl/notosansmanichaean/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Marchen"_s, { u"ofl/notosansmarchen/NotoSansMarchen-Regular.ttf"_s }, u"ofl/notosansmarchen/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Masaram Gondi"_s, { u"ofl/notosansmasaramgondi/NotoSansMasaramGondi-Regular.ttf"_s }, u"ofl/notosansmasaramgondi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Math"_s, { u"ofl/notosansmath/NotoSansMath-Regular.ttf"_s }, u"ofl/notosansmath/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mayan Numerals"_s, { u"ofl/notosansmayannumerals/NotoSansMayanNumerals-Regular.ttf"_s }, u"ofl/notosansmayannumerals/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Medefaidrin"_s, { u"ofl/notosansmedefaidrin/NotoSansMedefaidrin%5Bwght%5D.ttf"_s }, u"ofl/notosansmedefaidrin/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Meetei Mayek"_s, { u"ofl/notosansmeeteimayek/NotoSansMeeteiMayek%5Bwght%5D.ttf"_s }, u"ofl/notosansmeeteimayek/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mende Kikakui"_s, { u"ofl/notosansmendekikakui/NotoSansMendeKikakui-Regular.ttf"_s }, u"ofl/notosansmendekikakui/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Meroitic"_s, { u"ofl/notosansmeroitic/NotoSansMeroitic-Regular.ttf"_s }, u"ofl/notosansmeroitic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Miao"_s, { u"ofl/notosansmiao/NotoSansMiao-Regular.ttf"_s }, u"ofl/notosansmiao/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Modi"_s, { u"ofl/notosansmodi/NotoSansModi-Regular.ttf"_s }, u"ofl/notosansmodi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mongolian"_s, { u"ofl/notosansmongolian/NotoSansMongolian-Regular.ttf"_s }, u"ofl/notosansmongolian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mono"_s, { u"ofl/notosansmono/NotoSansMono%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansmono/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Mro"_s, { u"ofl/notosansmro/NotoSansMro-Regular.ttf"_s }, u"ofl/notosansmro/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Multani"_s, { u"ofl/notosansmultani/NotoSansMultani-Regular.ttf"_s }, u"ofl/notosansmultani/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Myanmar"_s, { u"ofl/notosansmyanmar/NotoSansMyanmar%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansmyanmar/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans NKo"_s, { u"ofl/notosansnko/NotoSansNKo-Regular.ttf"_s }, u"ofl/notosansnko/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Nabataean"_s, { u"ofl/notosansnabataean/NotoSansNabataean-Regular.ttf"_s }, u"ofl/notosansnabataean/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Nag Mundari"_s, { u"ofl/notosansnagmundari/NotoSansNagMundari%5Bwght%5D.ttf"_s }, u"ofl/notosansnagmundari/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Nandinagari"_s, { u"ofl/notosansnandinagari/NotoSansNandinagari-Regular.ttf"_s }, u"ofl/notosansnandinagari/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans New Tai Lue"_s, { u"ofl/notosansnewtailue/NotoSansNewTaiLue%5Bwght%5D.ttf"_s }, u"ofl/notosansnewtailue/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Newa"_s, { u"ofl/notosansnewa/NotoSansNewa-Regular.ttf"_s }, u"ofl/notosansnewa/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Nushu"_s, { u"ofl/notosansnushu/NotoSansNushu-Regular.ttf"_s }, u"ofl/notosansnushu/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Ogham"_s, { u"ofl/notosansogham/NotoSansOgham-Regular.ttf"_s }, u"ofl/notosansogham/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Ol Chiki"_s, { u"ofl/notosansolchiki/NotoSansOlChiki%5Bwght%5D.ttf"_s }, u"ofl/notosansolchiki/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old Hungarian"_s, { u"ofl/notosansoldhungarian/NotoSansOldHungarian-Regular.ttf"_s }, u"ofl/notosansoldhungarian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old Italic"_s, { u"ofl/notosansolditalic/NotoSansOldItalic-Regular.ttf"_s }, u"ofl/notosansolditalic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old North Arabian"_s, { u"ofl/notosansoldnortharabian/NotoSansOldNorthArabian-Regular.ttf"_s }, u"ofl/notosansoldnortharabian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old Permic"_s, { u"ofl/notosansoldpermic/NotoSansOldPermic-Regular.ttf"_s }, u"ofl/notosansoldpermic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old Persian"_s, { u"ofl/notosansoldpersian/NotoSansOldPersian-Regular.ttf"_s }, u"ofl/notosansoldpersian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old Sogdian"_s, { u"ofl/notosansoldsogdian/NotoSansOldSogdian-Regular.ttf"_s }, u"ofl/notosansoldsogdian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old South Arabian"_s, { u"ofl/notosansoldsoutharabian/NotoSansOldSouthArabian-Regular.ttf"_s }, u"ofl/notosansoldsoutharabian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Old Turkic"_s, { u"ofl/notosansoldturkic/NotoSansOldTurkic-Regular.ttf"_s }, u"ofl/notosansoldturkic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Oriya"_s, { u"ofl/notosansoriya/NotoSansOriya%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansoriya/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Osage"_s, { u"ofl/notosansosage/NotoSansOsage-Regular.ttf"_s }, u"ofl/notosansosage/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Osmanya"_s, { u"ofl/notosansosmanya/NotoSansOsmanya-Regular.ttf"_s }, u"ofl/notosansosmanya/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Pahawh Hmong"_s, { u"ofl/notosanspahawhhmong/NotoSansPahawhHmong-Regular.ttf"_s }, u"ofl/notosanspahawhhmong/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Palmyrene"_s, { u"ofl/notosanspalmyrene/NotoSansPalmyrene-Regular.ttf"_s }, u"ofl/notosanspalmyrene/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Pau Cin Hau"_s, { u"ofl/notosanspaucinhau/NotoSansPauCinHau-Regular.ttf"_s }, u"ofl/notosanspaucinhau/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Phoenician"_s, { u"ofl/notosansphoenician/NotoSansPhoenician-Regular.ttf"_s }, u"ofl/notosansphoenician/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Psalter Pahlavi"_s, { u"ofl/notosanspsalterpahlavi/NotoSansPsalterPahlavi-Regular.ttf"_s }, u"ofl/notosanspsalterpahlavi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Rejang"_s, { u"ofl/notosansrejang/NotoSansRejang-Regular.ttf"_s }, u"ofl/notosansrejang/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Runic"_s, { u"ofl/notosansrunic/NotoSansRunic-Regular.ttf"_s }, u"ofl/notosansrunic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans SC"_s, { u"ofl/notosanssc/NotoSansSC%5Bwght%5D.ttf"_s }, u"ofl/notosanssc/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Samaritan"_s, { u"ofl/notosanssamaritan/NotoSansSamaritan-Regular.ttf"_s }, u"ofl/notosanssamaritan/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Saurashtra"_s, { u"ofl/notosanssaurashtra/NotoSansSaurashtra-Regular.ttf"_s }, u"ofl/notosanssaurashtra/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Sharada"_s, { u"ofl/notosanssharada/NotoSansSharada-Regular.ttf"_s }, u"ofl/notosanssharada/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Shavian"_s, { u"ofl/notosansshavian/NotoSansShavian-Regular.ttf"_s }, u"ofl/notosansshavian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Siddham"_s, { u"ofl/notosanssiddham/NotoSansSiddham-Regular.ttf"_s }, u"ofl/notosanssiddham/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans SignWriting"_s, { u"ofl/notosanssignwriting/NotoSansSignWriting-Regular.ttf"_s }, u"ofl/notosanssignwriting/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Sinhala"_s, { u"ofl/notosanssinhala/NotoSansSinhala%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanssinhala/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Sogdian"_s, { u"ofl/notosanssogdian/NotoSansSogdian-Regular.ttf"_s }, u"ofl/notosanssogdian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Sora Sompeng"_s, { u"ofl/notosanssorasompeng/NotoSansSoraSompeng%5Bwght%5D.ttf"_s }, u"ofl/notosanssorasompeng/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Soyombo"_s, { u"ofl/notosanssoyombo/NotoSansSoyombo-Regular.ttf"_s }, u"ofl/notosanssoyombo/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Sundanese"_s, { u"ofl/notosanssundanese/NotoSansSundanese%5Bwght%5D.ttf"_s }, u"ofl/notosanssundanese/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Syloti Nagri"_s, { u"ofl/notosanssylotinagri/NotoSansSylotiNagri-Regular.ttf"_s }, u"ofl/notosanssylotinagri/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Symbols"_s, { u"ofl/notosanssymbols/NotoSansSymbols%5Bwght%5D.ttf"_s }, u"ofl/notosanssymbols/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Symbols 2"_s, { u"ofl/notosanssymbols2/NotoSansSymbols2-Regular.ttf"_s }, u"ofl/notosanssymbols2/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Syriac"_s, { u"ofl/notosanssyriac/NotoSansSyriac%5Bwght%5D.ttf"_s }, u"ofl/notosanssyriac/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Syriac Eastern"_s, { u"ofl/notosanssyriaceastern/NotoSansSyriacEastern%5Bwght%5D.ttf"_s }, u"ofl/notosanssyriaceastern/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans TC"_s, { u"ofl/notosanstc/NotoSansTC%5Bwght%5D.ttf"_s }, u"ofl/notosanstc/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tagalog"_s, { u"ofl/notosanstagalog/NotoSansTagalog-Regular.ttf"_s }, u"ofl/notosanstagalog/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tagbanwa"_s, { u"ofl/notosanstagbanwa/NotoSansTagbanwa-Regular.ttf"_s }, u"ofl/notosanstagbanwa/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tai Le"_s, { u"ofl/notosanstaile/NotoSansTaiLe-Regular.ttf"_s }, u"ofl/notosanstaile/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tai Tham"_s, { u"ofl/notosanstaitham/NotoSansTaiTham%5Bwght%5D.ttf"_s }, u"ofl/notosanstaitham/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tai Viet"_s, { u"ofl/notosanstaiviet/NotoSansTaiViet-Regular.ttf"_s }, u"ofl/notosanstaiviet/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Takri"_s, { u"ofl/notosanstakri/NotoSansTakri-Regular.ttf"_s }, u"ofl/notosanstakri/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tamil"_s, { u"ofl/notosanstamil/NotoSansTamil%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanstamil/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tamil Supplement"_s, { u"ofl/notosanstamilsupplement/NotoSansTamilSupplement-Regular.ttf"_s }, u"ofl/notosanstamilsupplement/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tangsa"_s, { u"ofl/notosanstangsa/NotoSansTangsa%5Bwght%5D.ttf"_s }, u"ofl/notosanstangsa/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Telugu"_s, { u"ofl/notosanstelugu/NotoSansTelugu%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosanstelugu/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Thaana"_s, { u"ofl/notosansthaana/NotoSansThaana%5Bwght%5D.ttf"_s }, u"ofl/notosansthaana/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Thai"_s, { u"ofl/notosansthai/NotoSansThai%5Bwdth,wght%5D.ttf"_s }, u"ofl/notosansthai/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Thai Looped"_s, { u"ofl/notosansthailooped/NotoSansThaiLooped-Thin.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-ExtraLight.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-Light.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-Regular.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-Medium.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-SemiBold.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-Bold.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-ExtraBold.ttf"_s, u"ofl/notosansthailooped/NotoSansThaiLooped-Black.ttf"_s }, u"ofl/notosansthailooped/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tifinagh"_s, { u"ofl/notosanstifinagh/NotoSansTifinagh-Regular.ttf"_s }, u"ofl/notosanstifinagh/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Tirhuta"_s, { u"ofl/notosanstirhuta/NotoSansTirhuta-Regular.ttf"_s }, u"ofl/notosanstirhuta/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Ugaritic"_s, { u"ofl/notosansugaritic/NotoSansUgaritic-Regular.ttf"_s }, u"ofl/notosansugaritic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Vai"_s, { u"ofl/notosansvai/NotoSansVai-Regular.ttf"_s }, u"ofl/notosansvai/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Vithkuqi"_s, { u"ofl/notosansvithkuqi/NotoSansVithkuqi%5Bwght%5D.ttf"_s }, u"ofl/notosansvithkuqi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Wancho"_s, { u"ofl/notosanswancho/NotoSansWancho-Regular.ttf"_s }, u"ofl/notosanswancho/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Warang Citi"_s, { u"ofl/notosanswarangciti/NotoSansWarangCiti-Regular.ttf"_s }, u"ofl/notosanswarangciti/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Yi"_s, { u"ofl/notosansyi/NotoSansYi-Regular.ttf"_s }, u"ofl/notosansyi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Sans Zanabazar Square"_s, { u"ofl/notosanszanabazarsquare/NotoSansZanabazarSquare-Regular.ttf"_s }, u"ofl/notosanszanabazarsquare/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif"_s, { u"ofl/notoserif/NotoSerif%5Bwdth,wght%5D.ttf"_s, u"ofl/notoserif/NotoSerif-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserif/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Ahom"_s, { u"ofl/notoserifahom/NotoSerifAhom-Regular.ttf"_s }, u"ofl/notoserifahom/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Armenian"_s, { u"ofl/notoserifarmenian/NotoSerifArmenian%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifarmenian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Balinese"_s, { u"ofl/notoserifbalinese/NotoSerifBalinese-Regular.ttf"_s }, u"ofl/notoserifbalinese/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Bengali"_s, { u"ofl/notoserifbengali/NotoSerifBengali%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifbengali/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Devanagari"_s, { u"ofl/notoserifdevanagari/NotoSerifDevanagari%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifdevanagari/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Display"_s, { u"ofl/notoserifdisplay/NotoSerifDisplay%5Bwdth,wght%5D.ttf"_s, u"ofl/notoserifdisplay/NotoSerifDisplay-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Dogra"_s, { u"ofl/notoserifdogra/NotoSerifDogra-Regular.ttf"_s }, u"ofl/notoserifdogra/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Ethiopic"_s, { u"ofl/notoserifethiopic/NotoSerifEthiopic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifethiopic/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Georgian"_s, { u"ofl/notoserifgeorgian/NotoSerifGeorgian%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifgeorgian/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Grantha"_s, { u"ofl/notoserifgrantha/NotoSerifGrantha-Regular.ttf"_s }, u"ofl/notoserifgrantha/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Gujarati"_s, { u"ofl/notoserifgujarati/NotoSerifGujarati%5Bwght%5D.ttf"_s }, u"ofl/notoserifgujarati/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Gurmukhi"_s, { u"ofl/notoserifgurmukhi/NotoSerifGurmukhi%5Bwght%5D.ttf"_s }, u"ofl/notoserifgurmukhi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif HK"_s, { u"ofl/notoserifhk/NotoSerifHK%5Bwght%5D.ttf"_s }, u"ofl/notoserifhk/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Hebrew"_s, { u"ofl/notoserifhebrew/NotoSerifHebrew%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifhebrew/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif JP"_s, { u"ofl/notoserifjp/NotoSerifJP%5Bwght%5D.ttf"_s }, u"ofl/notoserifjp/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif KR"_s, { u"ofl/notoserifkr/NotoSerifKR%5Bwght%5D.ttf"_s }, u"ofl/notoserifkr/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Kannada"_s, { u"ofl/notoserifkannada/NotoSerifKannada%5Bwght%5D.ttf"_s }, u"ofl/notoserifkannada/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Khitan Small Script"_s, { u"ofl/notoserifkhitansmallscript/NotoSerifKhitanSmallScript-Regular.ttf"_s }, u"ofl/notoserifkhitansmallscript/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Khmer"_s, { u"ofl/notoserifkhmer/NotoSerifKhmer%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifkhmer/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Khojki"_s, { u"ofl/notoserifkhojki/NotoSerifKhojki%5Bwght%5D.ttf"_s }, u"ofl/notoserifkhojki/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Lao"_s, { u"ofl/notoseriflao/NotoSerifLao%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoseriflao/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Makasar"_s, { u"ofl/notoserifmakasar/NotoSerifMakasar-Regular.ttf"_s }, u"ofl/notoserifmakasar/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Malayalam"_s, { u"ofl/notoserifmalayalam/NotoSerifMalayalam%5Bwght%5D.ttf"_s }, u"ofl/notoserifmalayalam/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Myanmar"_s, { u"ofl/notoserifmyanmar/NotoSerifMyanmar-Thin.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-ExtraLight.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-Light.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-Regular.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-Medium.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-SemiBold.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-Bold.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-ExtraBold.ttf"_s, u"ofl/notoserifmyanmar/NotoSerifMyanmar-Black.ttf"_s }, u"ofl/notoserifmyanmar/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif NP Hmong"_s, { u"ofl/notoserifnphmong/NotoSerifNPHmong%5Bwght%5D.ttf"_s }, u"ofl/notoserifnphmong/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Oriya"_s, { u"ofl/notoseriforiya/NotoSerifOriya%5Bwght%5D.ttf"_s }, u"ofl/notoseriforiya/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Ottoman Siyaq"_s, { u"ofl/notoserifottomansiyaq/NotoSerifOttomanSiyaq-Regular.ttf"_s }, u"ofl/notoserifottomansiyaq/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif SC"_s, { u"ofl/notoserifsc/NotoSerifSC%5Bwght%5D.ttf"_s }, u"ofl/notoserifsc/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Sinhala"_s, { u"ofl/notoserifsinhala/NotoSerifSinhala%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifsinhala/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif TC"_s, { u"ofl/notoseriftc/NotoSerifTC%5Bwght%5D.ttf"_s }, u"ofl/notoseriftc/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Tamil"_s, { u"ofl/notoseriftamil/NotoSerifTamil%5Bwdth,wght%5D.ttf"_s, u"ofl/notoseriftamil/NotoSerifTamil-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoseriftamil/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Tangut"_s, { u"ofl/notoseriftangut/NotoSerifTangut-Regular.ttf"_s }, u"ofl/notoseriftangut/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Telugu"_s, { u"ofl/notoseriftelugu/NotoSerifTelugu%5Bwght%5D.ttf"_s }, u"ofl/notoseriftelugu/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Thai"_s, { u"ofl/notoserifthai/NotoSerifThai%5Bwdth,wght%5D.ttf"_s }, u"ofl/notoserifthai/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Tibetan"_s, { u"ofl/notoseriftibetan/NotoSerifTibetan%5Bwght%5D.ttf"_s }, u"ofl/notoseriftibetan/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Toto"_s, { u"ofl/notoseriftoto/NotoSerifToto%5Bwght%5D.ttf"_s }, u"ofl/notoseriftoto/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Vithkuqi"_s, { u"ofl/notoserifvithkuqi/NotoSerifVithkuqi%5Bwght%5D.ttf"_s }, u"ofl/notoserifvithkuqi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Serif Yezidi"_s, { u"ofl/notoserifyezidi/NotoSerifYezidi%5Bwght%5D.ttf"_s }, u"ofl/notoserifyezidi/OFL.txt"_s ),
    GoogleFontDetails( u"Noto Traditional Nushu"_s, { u"ofl/nototraditionalnushu/NotoTraditionalNushu%5Bwght%5D.ttf"_s }, u"ofl/nototraditionalnushu/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Cut"_s, { u"ofl/novacut/NovaCut.ttf"_s }, u"ofl/novacut/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Flat"_s, { u"ofl/novaflat/NovaFlat.ttf"_s }, u"ofl/novaflat/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Mono"_s, { u"ofl/novamono/NovaMono.ttf"_s }, u"ofl/novamono/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Oval"_s, { u"ofl/novaoval/NovaOval.ttf"_s }, u"ofl/novaoval/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Round"_s, { u"ofl/novaround/NovaRound.ttf"_s }, u"ofl/novaround/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Script"_s, { u"ofl/novascript/NovaScript-Regular.ttf"_s }, u"ofl/novascript/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Slim"_s, { u"ofl/novaslim/NovaSlim.ttf"_s }, u"ofl/novaslim/OFL.txt"_s ),
    GoogleFontDetails( u"Nova Square"_s, { u"ofl/novasquare/NovaSquare.ttf"_s }, u"ofl/novasquare/OFL.txt"_s ),
    GoogleFontDetails( u"Numans"_s, { u"ofl/numans/Numans-Regular.ttf"_s }, u"ofl/numans/OFL.txt"_s ),
    GoogleFontDetails( u"Nunito"_s, { u"ofl/nunito/Nunito%5Bwght%5D.ttf"_s, u"ofl/nunito/Nunito-Italic%5Bwght%5D.ttf"_s }, u"ofl/nunito/OFL.txt"_s ),
    GoogleFontDetails( u"Nunito Sans"_s, { u"ofl/nunitosans/NunitoSans%5BYTLC,opsz,wdth,wght%5D.ttf"_s, u"ofl/nunitosans/NunitoSans-Italic%5BYTLC,opsz,wdth,wght%5D.ttf"_s }, u"ofl/nunitosans/OFL.txt"_s ),
    GoogleFontDetails( u"Nuosu SIL"_s, { u"ofl/nuosusil/NuosuSIL-Regular.ttf"_s }, u"ofl/nuosusil/OFL.txt"_s ),
    GoogleFontDetails( u"Odibee Sans"_s, { u"ofl/odibeesans/OdibeeSans-Regular.ttf"_s }, u"ofl/odibeesans/OFL.txt"_s ),
    GoogleFontDetails( u"Odor Mean Chey"_s, { u"ofl/odormeanchey/OdorMeanChey-Regular.ttf"_s }, u"ofl/odormeanchey/OFL.txt"_s ),
    GoogleFontDetails( u"Offside"_s, { u"ofl/offside/Offside-Regular.ttf"_s }, u"ofl/offside/OFL.txt"_s ),
    GoogleFontDetails( u"Oi"_s, { u"ofl/oi/Oi-Regular.ttf"_s }, u"ofl/oi/OFL.txt"_s ),
    GoogleFontDetails( u"Old Standard TT"_s, { u"ofl/oldstandardtt/OldStandard-Regular.ttf"_s, u"ofl/oldstandardtt/OldStandard-Italic.ttf"_s, u"ofl/oldstandardtt/OldStandard-Bold.ttf"_s }, u"ofl/oldstandardtt/OFL.txt"_s ),
    GoogleFontDetails( u"Oldenburg"_s, { u"ofl/oldenburg/Oldenburg-Regular.ttf"_s }, u"ofl/oldenburg/OFL.txt"_s ),
    GoogleFontDetails( u"Ole"_s, { u"ofl/ole/Ole-Regular.ttf"_s }, u"ofl/ole/OFL.txt"_s ),
    GoogleFontDetails( u"Oleo Script"_s, { u"ofl/oleoscript/OleoScript-Regular.ttf"_s, u"ofl/oleoscript/OleoScript-Bold.ttf"_s }, u"ofl/oleoscript/OFL.txt"_s ),
    GoogleFontDetails( u"Oleo Script Swash Caps"_s, { u"ofl/oleoscriptswashcaps/OleoScriptSwashCaps-Regular.ttf"_s, u"ofl/oleoscriptswashcaps/OleoScriptSwashCaps-Bold.ttf"_s }, u"ofl/oleoscriptswashcaps/OFL.txt"_s ),
    GoogleFontDetails( u"Oooh Baby"_s, { u"ofl/ooohbaby/OoohBaby-Regular.ttf"_s }, u"ofl/ooohbaby/OFL.txt"_s ),
    GoogleFontDetails( u"Open Sans"_s, { u"ofl/opensans/OpenSans%5Bwdth,wght%5D.ttf"_s, u"ofl/opensans/OpenSans-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/opensans/OFL.txt"_s ),
    GoogleFontDetails( u"Oranienbaum"_s, { u"ofl/oranienbaum/Oranienbaum-Regular.ttf"_s }, u"ofl/oranienbaum/OFL.txt"_s ),
    GoogleFontDetails( u"Orbit"_s, { u"ofl/orbit/Orbit-Regular.ttf"_s }, u"ofl/orbit/OFL.txt"_s ),
    GoogleFontDetails( u"Orbitron"_s, { u"ofl/orbitron/Orbitron%5Bwght%5D.ttf"_s }, u"ofl/orbitron/OFL.txt"_s ),
    GoogleFontDetails( u"Oregano"_s, { u"ofl/oregano/Oregano-Regular.ttf"_s, u"ofl/oregano/Oregano-Italic.ttf"_s }, u"ofl/oregano/OFL.txt"_s ),
    GoogleFontDetails( u"Orelega One"_s, { u"ofl/orelegaone/OrelegaOne-Regular.ttf"_s }, u"ofl/orelegaone/OFL.txt"_s ),
    GoogleFontDetails( u"Orienta"_s, { u"ofl/orienta/Orienta-Regular.ttf"_s }, u"ofl/orienta/OFL.txt"_s ),
    GoogleFontDetails( u"Original Surfer"_s, { u"ofl/originalsurfer/OriginalSurfer-Regular.ttf"_s }, u"ofl/originalsurfer/OFL.txt"_s ),
    GoogleFontDetails( u"Oswald"_s, { u"ofl/oswald/Oswald%5Bwght%5D.ttf"_s }, u"ofl/oswald/OFL.txt"_s ),
    GoogleFontDetails( u"Outfit"_s, { u"ofl/outfit/Outfit%5Bwght%5D.ttf"_s }, u"ofl/outfit/OFL.txt"_s ),
    GoogleFontDetails( u"Over the Rainbow"_s, { u"ofl/overtherainbow/OvertheRainbow.ttf"_s }, u"ofl/overtherainbow/OFL.txt"_s ),
    GoogleFontDetails( u"Overlock"_s, { u"ofl/overlock/Overlock-Regular.ttf"_s, u"ofl/overlock/Overlock-Italic.ttf"_s, u"ofl/overlock/Overlock-Bold.ttf"_s, u"ofl/overlock/Overlock-BoldItalic.ttf"_s, u"ofl/overlock/Overlock-Black.ttf"_s, u"ofl/overlock/Overlock-BlackItalic.ttf"_s }, u"ofl/overlock/OFL.txt"_s ),
    GoogleFontDetails( u"Overlock SC"_s, { u"ofl/overlocksc/OverlockSC-Regular.ttf"_s }, u"ofl/overlocksc/OFL.txt"_s ),
    GoogleFontDetails( u"Overpass"_s, { u"ofl/overpass/Overpass%5Bwght%5D.ttf"_s, u"ofl/overpass/Overpass-Italic%5Bwght%5D.ttf"_s }, u"ofl/overpass/OFL.txt"_s ),
    GoogleFontDetails( u"Overpass Mono"_s, { u"ofl/overpassmono/OverpassMono%5Bwght%5D.ttf"_s }, u"ofl/overpassmono/OFL.txt"_s ),
    GoogleFontDetails( u"Ovo"_s, { u"ofl/ovo/Ovo-Regular.ttf"_s }, u"ofl/ovo/OFL.txt"_s ),
    GoogleFontDetails( u"Oxanium"_s, { u"ofl/oxanium/Oxanium%5Bwght%5D.ttf"_s }, u"ofl/oxanium/OFL.txt"_s ),
    GoogleFontDetails( u"Oxygen"_s, { u"ofl/oxygen/Oxygen-Light.ttf"_s, u"ofl/oxygen/Oxygen-Regular.ttf"_s, u"ofl/oxygen/Oxygen-Bold.ttf"_s }, u"ofl/oxygen/OFL.txt"_s ),
    GoogleFontDetails( u"Oxygen Mono"_s, { u"ofl/oxygenmono/OxygenMono-Regular.ttf"_s }, u"ofl/oxygenmono/OFL.txt"_s ),
    GoogleFontDetails( u"PT Mono"_s, { u"ofl/ptmono/PTM55FT.ttf"_s }, u"ofl/ptmono/OFL.txt"_s ),
    GoogleFontDetails( u"PT Sans"_s, { u"ofl/ptsans/PT_Sans-Web-Regular.ttf"_s, u"ofl/ptsans/PT_Sans-Web-Italic.ttf"_s, u"ofl/ptsans/PT_Sans-Web-Bold.ttf"_s, u"ofl/ptsans/PT_Sans-Web-BoldItalic.ttf"_s }, u"ofl/ptsans/OFL.txt"_s ),
    GoogleFontDetails( u"PT Sans Caption"_s, { u"ofl/ptsanscaption/PT_Sans-Caption-Web-Regular.ttf"_s, u"ofl/ptsanscaption/PT_Sans-Caption-Web-Bold.ttf"_s }, u"ofl/ptsanscaption/OFL.txt"_s ),
    GoogleFontDetails( u"PT Sans Narrow"_s, { u"ofl/ptsansnarrow/PT_Sans-Narrow-Web-Regular.ttf"_s, u"ofl/ptsansnarrow/PT_Sans-Narrow-Web-Bold.ttf"_s }, u"ofl/ptsansnarrow/OFL.txt"_s ),
    GoogleFontDetails( u"PT Serif"_s, { u"ofl/ptserif/PT_Serif-Web-Regular.ttf"_s, u"ofl/ptserif/PT_Serif-Web-Italic.ttf"_s, u"ofl/ptserif/PT_Serif-Web-Bold.ttf"_s, u"ofl/ptserif/PT_Serif-Web-BoldItalic.ttf"_s }, u"ofl/ptserif/OFL.txt"_s ),
    GoogleFontDetails( u"PT Serif Caption"_s, { u"ofl/ptserifcaption/PT_Serif-Caption-Web-Regular.ttf"_s, u"ofl/ptserifcaption/PT_Serif-Caption-Web-Italic.ttf"_s }, u"ofl/ptserifcaption/OFL.txt"_s ),
    GoogleFontDetails( u"Pacifico"_s, { u"ofl/pacifico/Pacifico-Regular.ttf"_s }, u"ofl/pacifico/OFL.txt"_s ),
    GoogleFontDetails( u"Padauk"_s, { u"ofl/padauk/Padauk-Regular.ttf"_s, u"ofl/padauk/Padauk-Bold.ttf"_s }, u"ofl/padauk/OFL.txt"_s ),
    GoogleFontDetails( u"Padyakke Expanded One"_s, { u"ofl/padyakkeexpandedone/PadyakkeExpandedOne-Regular.ttf"_s }, u"ofl/padyakkeexpandedone/OFL.txt"_s ),
    GoogleFontDetails( u"Palanquin"_s, { u"ofl/palanquin/Palanquin-Thin.ttf"_s, u"ofl/palanquin/Palanquin-ExtraLight.ttf"_s, u"ofl/palanquin/Palanquin-Light.ttf"_s, u"ofl/palanquin/Palanquin-Regular.ttf"_s, u"ofl/palanquin/Palanquin-Medium.ttf"_s, u"ofl/palanquin/Palanquin-SemiBold.ttf"_s, u"ofl/palanquin/Palanquin-Bold.ttf"_s }, u"ofl/palanquin/OFL.txt"_s ),
    GoogleFontDetails( u"Palanquin Dark"_s, { u"ofl/palanquindark/PalanquinDark-Regular.ttf"_s, u"ofl/palanquindark/PalanquinDark-Medium.ttf"_s, u"ofl/palanquindark/PalanquinDark-SemiBold.ttf"_s, u"ofl/palanquindark/PalanquinDark-Bold.ttf"_s }, u"ofl/palanquindark/OFL.txt"_s ),
    GoogleFontDetails( u"Palette Mosaic"_s, { u"ofl/palettemosaic/PaletteMosaic-Regular.ttf"_s }, u"ofl/palettemosaic/OFL.txt"_s ),
    GoogleFontDetails( u"Pangolin"_s, { u"ofl/pangolin/Pangolin-Regular.ttf"_s }, u"ofl/pangolin/OFL.txt"_s ),
    GoogleFontDetails( u"Paprika"_s, { u"ofl/paprika/Paprika-Regular.ttf"_s }, u"ofl/paprika/OFL.txt"_s ),
    GoogleFontDetails( u"Parisienne"_s, { u"ofl/parisienne/Parisienne-Regular.ttf"_s }, u"ofl/parisienne/OFL.txt"_s ),
    GoogleFontDetails( u"Passero One"_s, { u"ofl/passeroone/PasseroOne-Regular.ttf"_s }, u"ofl/passeroone/OFL.txt"_s ),
    GoogleFontDetails( u"Passion One"_s, { u"ofl/passionone/PassionOne-Regular.ttf"_s, u"ofl/passionone/PassionOne-Bold.ttf"_s, u"ofl/passionone/PassionOne-Black.ttf"_s }, u"ofl/passionone/OFL.txt"_s ),
    GoogleFontDetails( u"Passions Conflict"_s, { u"ofl/passionsconflict/PassionsConflict-Regular.ttf"_s }, u"ofl/passionsconflict/OFL.txt"_s ),
    GoogleFontDetails( u"Pathway Extreme"_s, { u"ofl/pathwayextreme/PathwayExtreme%5Bopsz,wdth,wght%5D.ttf"_s, u"ofl/pathwayextreme/PathwayExtreme-Italic%5Bopsz,wdth,wght%5D.ttf"_s }, u"ofl/pathwayextreme/OFL.txt"_s ),
    GoogleFontDetails( u"Pathway Gothic One"_s, { u"ofl/pathwaygothicone/PathwayGothicOne-Regular.ttf"_s }, u"ofl/pathwaygothicone/OFL.txt"_s ),
    GoogleFontDetails( u"Patrick Hand"_s, { u"ofl/patrickhand/PatrickHand-Regular.ttf"_s }, u"ofl/patrickhand/OFL.txt"_s ),
    GoogleFontDetails( u"Patrick Hand SC"_s, { u"ofl/patrickhandsc/PatrickHandSC-Regular.ttf"_s }, u"ofl/patrickhandsc/OFL.txt"_s ),
    GoogleFontDetails( u"Pattaya"_s, { u"ofl/pattaya/Pattaya-Regular.ttf"_s }, u"ofl/pattaya/OFL.txt"_s ),
    GoogleFontDetails( u"Patua One"_s, { u"ofl/patuaone/PatuaOne-Regular.ttf"_s }, u"ofl/patuaone/OFL.txt"_s ),
    GoogleFontDetails( u"Pavanam"_s, { u"ofl/pavanam/Pavanam-Regular.ttf"_s }, u"ofl/pavanam/OFL.txt"_s ),
    GoogleFontDetails( u"Paytone One"_s, { u"ofl/paytoneone/PaytoneOne-Regular.ttf"_s }, u"ofl/paytoneone/OFL.txt"_s ),
    GoogleFontDetails( u"Peddana"_s, { u"ofl/peddana/Peddana-Regular.ttf"_s }, u"ofl/peddana/OFL.txt"_s ),
    GoogleFontDetails( u"Peralta"_s, { u"ofl/peralta/Peralta-Regular.ttf"_s }, u"ofl/peralta/OFL.txt"_s ),
    GoogleFontDetails( u"Permanent Marker"_s, { u"apache/permanentmarker/PermanentMarker-Regular.ttf"_s }, u"apache/permanentmarker/LICENSE.txt"_s ),
    GoogleFontDetails( u"Petemoss"_s, { u"ofl/petemoss/Petemoss-Regular.ttf"_s }, u"ofl/petemoss/OFL.txt"_s ),
    GoogleFontDetails( u"Petit Formal Script"_s, { u"ofl/petitformalscript/PetitFormalScript-Regular.ttf"_s }, u"ofl/petitformalscript/OFL.txt"_s ),
    GoogleFontDetails( u"Petrona"_s, { u"ofl/petrona/Petrona%5Bwght%5D.ttf"_s, u"ofl/petrona/Petrona-Italic%5Bwght%5D.ttf"_s }, u"ofl/petrona/OFL.txt"_s ),
    GoogleFontDetails( u"Philosopher"_s, { u"ofl/philosopher/Philosopher-Regular.ttf"_s, u"ofl/philosopher/Philosopher-Italic.ttf"_s, u"ofl/philosopher/Philosopher-Bold.ttf"_s, u"ofl/philosopher/Philosopher-BoldItalic.ttf"_s }, u"ofl/philosopher/OFL.txt"_s ),
    GoogleFontDetails( u"Phudu"_s, { u"ofl/phudu/Phudu%5Bwght%5D.ttf"_s }, u"ofl/phudu/OFL.txt"_s ),
    GoogleFontDetails( u"Piazzolla"_s, { u"ofl/piazzolla/Piazzolla%5Bopsz,wght%5D.ttf"_s, u"ofl/piazzolla/Piazzolla-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/piazzolla/OFL.txt"_s ),
    GoogleFontDetails( u"Piedra"_s, { u"ofl/piedra/Piedra-Regular.ttf"_s }, u"ofl/piedra/OFL.txt"_s ),
    GoogleFontDetails( u"Pinyon Script"_s, { u"ofl/pinyonscript/PinyonScript-Regular.ttf"_s }, u"ofl/pinyonscript/OFL.txt"_s ),
    GoogleFontDetails( u"Pirata One"_s, { u"ofl/pirataone/PirataOne-Regular.ttf"_s }, u"ofl/pirataone/OFL.txt"_s ),
    GoogleFontDetails( u"Plaster"_s, { u"ofl/plaster/Plaster-Regular.ttf"_s }, u"ofl/plaster/OFL.txt"_s ),
    GoogleFontDetails( u"Play"_s, { u"ofl/play/Play-Regular.ttf"_s, u"ofl/play/Play-Bold.ttf"_s }, u"ofl/play/OFL.txt"_s ),
    GoogleFontDetails( u"Playball"_s, { u"ofl/playball/Playball-Regular.ttf"_s }, u"ofl/playball/OFL.txt"_s ),
    GoogleFontDetails( u"Playfair"_s, { u"ofl/playfair/Playfair%5Bopsz,wdth,wght%5D.ttf"_s, u"ofl/playfair/Playfair-Italic%5Bopsz,wdth,wght%5D.ttf"_s }, u"ofl/playfair/OFL.txt"_s ),
    GoogleFontDetails( u"Playfair Display"_s, { u"ofl/playfairdisplay/PlayfairDisplay%5Bwght%5D.ttf"_s, u"ofl/playfairdisplay/PlayfairDisplay-Italic%5Bwght%5D.ttf"_s }, u"ofl/playfairdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Playfair Display SC"_s, { u"ofl/playfairdisplaysc/PlayfairDisplaySC-Regular.ttf"_s, u"ofl/playfairdisplaysc/PlayfairDisplaySC-Italic.ttf"_s, u"ofl/playfairdisplaysc/PlayfairDisplaySC-Bold.ttf"_s, u"ofl/playfairdisplaysc/PlayfairDisplaySC-BoldItalic.ttf"_s, u"ofl/playfairdisplaysc/PlayfairDisplaySC-Black.ttf"_s, u"ofl/playfairdisplaysc/PlayfairDisplaySC-BlackItalic.ttf"_s }, u"ofl/playfairdisplaysc/OFL.txt"_s ),
    GoogleFontDetails( u"Plus Jakarta Sans"_s, { u"ofl/plusjakartasans/PlusJakartaSans%5Bwght%5D.ttf"_s, u"ofl/plusjakartasans/PlusJakartaSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/plusjakartasans/OFL.txt"_s ),
    GoogleFontDetails( u"Podkova"_s, { u"ofl/podkova/Podkova%5Bwght%5D.ttf"_s }, u"ofl/podkova/OFL.txt"_s ),
    GoogleFontDetails( u"Poiret One"_s, { u"ofl/poiretone/PoiretOne-Regular.ttf"_s }, u"ofl/poiretone/OFL.txt"_s ),
    GoogleFontDetails( u"Poller One"_s, { u"ofl/pollerone/PollerOne.ttf"_s }, u"ofl/pollerone/OFL.txt"_s ),
    GoogleFontDetails( u"Poltawski Nowy"_s, { u"ofl/poltawskinowy/PoltawskiNowy%5Bwght%5D.ttf"_s, u"ofl/poltawskinowy/PoltawskiNowy-Italic%5Bwght%5D.ttf"_s }, u"ofl/poltawskinowy/OFL.txt"_s ),
    GoogleFontDetails( u"Poly"_s, { u"ofl/poly/Poly-Regular.ttf"_s, u"ofl/poly/Poly-Italic.ttf"_s }, u"ofl/poly/OFL.txt"_s ),
    GoogleFontDetails( u"Pompiere"_s, { u"ofl/pompiere/Pompiere-Regular.ttf"_s }, u"ofl/pompiere/OFL.txt"_s ),
    GoogleFontDetails( u"Pontano Sans"_s, { u"ofl/pontanosans/PontanoSans%5Bwght%5D.ttf"_s }, u"ofl/pontanosans/OFL.txt"_s ),
    GoogleFontDetails( u"Poor Story"_s, { u"ofl/poorstory/PoorStory-Regular.ttf"_s }, u"ofl/poorstory/OFL.txt"_s ),
    GoogleFontDetails( u"Poppins"_s, { u"ofl/poppins/Poppins-Thin.ttf"_s, u"ofl/poppins/Poppins-ThinItalic.ttf"_s, u"ofl/poppins/Poppins-ExtraLight.ttf"_s, u"ofl/poppins/Poppins-ExtraLightItalic.ttf"_s, u"ofl/poppins/Poppins-Light.ttf"_s, u"ofl/poppins/Poppins-LightItalic.ttf"_s, u"ofl/poppins/Poppins-Regular.ttf"_s, u"ofl/poppins/Poppins-Italic.ttf"_s, u"ofl/poppins/Poppins-Medium.ttf"_s, u"ofl/poppins/Poppins-MediumItalic.ttf"_s, u"ofl/poppins/Poppins-SemiBold.ttf"_s, u"ofl/poppins/Poppins-SemiBoldItalic.ttf"_s, u"ofl/poppins/Poppins-Bold.ttf"_s, u"ofl/poppins/Poppins-BoldItalic.ttf"_s, u"ofl/poppins/Poppins-ExtraBold.ttf"_s, u"ofl/poppins/Poppins-ExtraBoldItalic.ttf"_s, u"ofl/poppins/Poppins-Black.ttf"_s, u"ofl/poppins/Poppins-BlackItalic.ttf"_s }, u"ofl/poppins/OFL.txt"_s ),
    GoogleFontDetails( u"Port Lligat Sans"_s, { u"ofl/portlligatsans/PortLligatSans-Regular.ttf"_s }, u"ofl/portlligatsans/OFL.txt"_s ),
    GoogleFontDetails( u"Port Lligat Slab"_s, { u"ofl/portlligatslab/PortLligatSlab-Regular.ttf"_s }, u"ofl/portlligatslab/OFL.txt"_s ),
    GoogleFontDetails( u"Potta One"_s, { u"ofl/pottaone/PottaOne-Regular.ttf"_s }, u"ofl/pottaone/OFL.txt"_s ),
    GoogleFontDetails( u"Pragati Narrow"_s, { u"ofl/pragatinarrow/PragatiNarrow-Regular.ttf"_s, u"ofl/pragatinarrow/PragatiNarrow-Bold.ttf"_s }, u"ofl/pragatinarrow/OFL.txt"_s ),
    GoogleFontDetails( u"Praise"_s, { u"ofl/praise/Praise-Regular.ttf"_s }, u"ofl/praise/OFL.txt"_s ),
    GoogleFontDetails( u"Preahvihear"_s, { u"ofl/preahvihear/Preahvihear-Regular.ttf"_s }, u"ofl/preahvihear/OFL.txt"_s ),
    GoogleFontDetails( u"Press Start 2P"_s, { u"ofl/pressstart2p/PressStart2P-Regular.ttf"_s }, u"ofl/pressstart2p/OFL.txt"_s ),
    GoogleFontDetails( u"Pridi"_s, { u"ofl/pridi/Pridi-ExtraLight.ttf"_s, u"ofl/pridi/Pridi-Light.ttf"_s, u"ofl/pridi/Pridi-Regular.ttf"_s, u"ofl/pridi/Pridi-Medium.ttf"_s, u"ofl/pridi/Pridi-SemiBold.ttf"_s, u"ofl/pridi/Pridi-Bold.ttf"_s }, u"ofl/pridi/OFL.txt"_s ),
    GoogleFontDetails( u"Princess Sofia"_s, { u"ofl/princesssofia/PrincessSofia-Regular.ttf"_s }, u"ofl/princesssofia/OFL.txt"_s ),
    GoogleFontDetails( u"Prociono"_s, { u"ofl/prociono/Prociono-Regular.ttf"_s }, u"ofl/prociono/OFL.txt"_s ),
    GoogleFontDetails( u"Prompt"_s, { u"ofl/prompt/Prompt-Thin.ttf"_s, u"ofl/prompt/Prompt-ThinItalic.ttf"_s, u"ofl/prompt/Prompt-ExtraLight.ttf"_s, u"ofl/prompt/Prompt-ExtraLightItalic.ttf"_s, u"ofl/prompt/Prompt-Light.ttf"_s, u"ofl/prompt/Prompt-LightItalic.ttf"_s, u"ofl/prompt/Prompt-Regular.ttf"_s, u"ofl/prompt/Prompt-Italic.ttf"_s, u"ofl/prompt/Prompt-Medium.ttf"_s, u"ofl/prompt/Prompt-MediumItalic.ttf"_s, u"ofl/prompt/Prompt-SemiBold.ttf"_s, u"ofl/prompt/Prompt-SemiBoldItalic.ttf"_s, u"ofl/prompt/Prompt-Bold.ttf"_s, u"ofl/prompt/Prompt-BoldItalic.ttf"_s, u"ofl/prompt/Prompt-ExtraBold.ttf"_s, u"ofl/prompt/Prompt-ExtraBoldItalic.ttf"_s, u"ofl/prompt/Prompt-Black.ttf"_s, u"ofl/prompt/Prompt-BlackItalic.ttf"_s }, u"ofl/prompt/OFL.txt"_s ),
    GoogleFontDetails( u"Prosto One"_s, { u"ofl/prostoone/ProstoOne-Regular.ttf"_s }, u"ofl/prostoone/OFL.txt"_s ),
    GoogleFontDetails( u"Proza Libre"_s, { u"ofl/prozalibre/ProzaLibre-Regular.ttf"_s, u"ofl/prozalibre/ProzaLibre-Italic.ttf"_s, u"ofl/prozalibre/ProzaLibre-Medium.ttf"_s, u"ofl/prozalibre/ProzaLibre-MediumItalic.ttf"_s, u"ofl/prozalibre/ProzaLibre-SemiBold.ttf"_s, u"ofl/prozalibre/ProzaLibre-SemiBoldItalic.ttf"_s, u"ofl/prozalibre/ProzaLibre-Bold.ttf"_s, u"ofl/prozalibre/ProzaLibre-BoldItalic.ttf"_s, u"ofl/prozalibre/ProzaLibre-ExtraBold.ttf"_s, u"ofl/prozalibre/ProzaLibre-ExtraBoldItalic.ttf"_s }, u"ofl/prozalibre/OFL.txt"_s ),
    GoogleFontDetails( u"Public Sans"_s, { u"ofl/publicsans/PublicSans%5Bwght%5D.ttf"_s, u"ofl/publicsans/PublicSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/publicsans/OFL.txt"_s ),
    GoogleFontDetails( u"Puppies Play"_s, { u"ofl/puppiesplay/PuppiesPlay-Regular.ttf"_s }, u"ofl/puppiesplay/OFL.txt"_s ),
    GoogleFontDetails( u"Puritan"_s, { u"ofl/puritan/Puritan-Regular.ttf"_s, u"ofl/puritan/Puritan-Italic.ttf"_s, u"ofl/puritan/Puritan-Bold.ttf"_s, u"ofl/puritan/Puritan-BoldItalic.ttf"_s }, u"ofl/puritan/OFL.txt"_s ),
    GoogleFontDetails( u"Purple Purse"_s, { u"ofl/purplepurse/PurplePurse-Regular.ttf"_s }, u"ofl/purplepurse/OFL.txt"_s ),
    GoogleFontDetails( u"Qahiri"_s, { u"ofl/qahiri/Qahiri-Regular.ttf"_s }, u"ofl/qahiri/OFL.txt"_s ),
    GoogleFontDetails( u"Quando"_s, { u"ofl/quando/Quando-Regular.ttf"_s }, u"ofl/quando/OFL.txt"_s ),
    GoogleFontDetails( u"Quantico"_s, { u"ofl/quantico/Quantico-Regular.ttf"_s, u"ofl/quantico/Quantico-Italic.ttf"_s, u"ofl/quantico/Quantico-Bold.ttf"_s, u"ofl/quantico/Quantico-BoldItalic.ttf"_s }, u"ofl/quantico/OFL.txt"_s ),
    GoogleFontDetails( u"Quattrocento"_s, { u"ofl/quattrocento/Quattrocento-Regular.ttf"_s, u"ofl/quattrocento/Quattrocento-Bold.ttf"_s }, u"ofl/quattrocento/OFL.txt"_s ),
    GoogleFontDetails( u"Quattrocento Sans"_s, { u"ofl/quattrocentosans/QuattrocentoSans-Regular.ttf"_s, u"ofl/quattrocentosans/QuattrocentoSans-Italic.ttf"_s, u"ofl/quattrocentosans/QuattrocentoSans-Bold.ttf"_s, u"ofl/quattrocentosans/QuattrocentoSans-BoldItalic.ttf"_s }, u"ofl/quattrocentosans/OFL.txt"_s ),
    GoogleFontDetails( u"Questrial"_s, { u"ofl/questrial/Questrial-Regular.ttf"_s }, u"ofl/questrial/OFL.txt"_s ),
    GoogleFontDetails( u"Quicksand"_s, { u"ofl/quicksand/Quicksand%5Bwght%5D.ttf"_s }, u"ofl/quicksand/OFL.txt"_s ),
    GoogleFontDetails( u"Quintessential"_s, { u"ofl/quintessential/Quintessential-Regular.ttf"_s }, u"ofl/quintessential/OFL.txt"_s ),
    GoogleFontDetails( u"Qwigley"_s, { u"ofl/qwigley/Qwigley-Regular.ttf"_s }, u"ofl/qwigley/OFL.txt"_s ),
    GoogleFontDetails( u"Qwitcher Grypen"_s, { u"ofl/qwitchergrypen/QwitcherGrypen-Regular.ttf"_s, u"ofl/qwitchergrypen/QwitcherGrypen-Bold.ttf"_s }, u"ofl/qwitchergrypen/OFL.txt"_s ),
    GoogleFontDetails( u"REM"_s, { u"ofl/rem/REM%5Bwght%5D.ttf"_s, u"ofl/rem/REM-Italic%5Bwght%5D.ttf"_s }, u"ofl/rem/OFL.txt"_s ),
    GoogleFontDetails( u"Racing Sans One"_s, { u"ofl/racingsansone/RacingSansOne-Regular.ttf"_s }, u"ofl/racingsansone/OFL.txt"_s ),
    GoogleFontDetails( u"Radio Canada"_s, { u"ofl/radiocanada/RadioCanada%5Bwdth,wght%5D.ttf"_s, u"ofl/radiocanada/RadioCanada-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/radiocanada/OFL.txt"_s ),
    GoogleFontDetails( u"Radley"_s, { u"ofl/radley/Radley-Regular.ttf"_s, u"ofl/radley/Radley-Italic.ttf"_s }, u"ofl/radley/OFL.txt"_s ),
    GoogleFontDetails( u"Rajdhani"_s, { u"ofl/rajdhani/Rajdhani-Light.ttf"_s, u"ofl/rajdhani/Rajdhani-Regular.ttf"_s, u"ofl/rajdhani/Rajdhani-Medium.ttf"_s, u"ofl/rajdhani/Rajdhani-SemiBold.ttf"_s, u"ofl/rajdhani/Rajdhani-Bold.ttf"_s }, u"ofl/rajdhani/OFL.txt"_s ),
    GoogleFontDetails( u"Rakkas"_s, { u"ofl/rakkas/Rakkas-Regular.ttf"_s }, u"ofl/rakkas/OFL.txt"_s ),
    GoogleFontDetails( u"Raleway"_s, { u"ofl/raleway/Raleway%5Bwght%5D.ttf"_s, u"ofl/raleway/Raleway-Italic%5Bwght%5D.ttf"_s }, u"ofl/raleway/OFL.txt"_s ),
    GoogleFontDetails( u"Raleway Dots"_s, { u"ofl/ralewaydots/RalewayDots-Regular.ttf"_s }, u"ofl/ralewaydots/OFL.txt"_s ),
    GoogleFontDetails( u"Ramabhadra"_s, { u"ofl/ramabhadra/Ramabhadra-Regular.ttf"_s }, u"ofl/ramabhadra/OFL.txt"_s ),
    GoogleFontDetails( u"Ramaraja"_s, { u"ofl/ramaraja/Ramaraja-Regular.ttf"_s }, u"ofl/ramaraja/OFL.txt"_s ),
    GoogleFontDetails( u"Rambla"_s, { u"ofl/rambla/Rambla-Regular.ttf"_s, u"ofl/rambla/Rambla-Italic.ttf"_s, u"ofl/rambla/Rambla-Bold.ttf"_s, u"ofl/rambla/Rambla-BoldItalic.ttf"_s }, u"ofl/rambla/OFL.txt"_s ),
    GoogleFontDetails( u"Rammetto One"_s, { u"ofl/rammettoone/RammettoOne-Regular.ttf"_s }, u"ofl/rammettoone/OFL.txt"_s ),
    GoogleFontDetails( u"Rampart One"_s, { u"ofl/rampartone/RampartOne-Regular.ttf"_s }, u"ofl/rampartone/OFL.txt"_s ),
    GoogleFontDetails( u"Ranchers"_s, { u"ofl/ranchers/Ranchers-Regular.ttf"_s }, u"ofl/ranchers/OFL.txt"_s ),
    GoogleFontDetails( u"Rancho"_s, { u"apache/rancho/Rancho-Regular.ttf"_s }, u"apache/rancho/LICENSE.txt"_s ),
    GoogleFontDetails( u"Ranga"_s, { u"ofl/ranga/Ranga-Regular.ttf"_s, u"ofl/ranga/Ranga-Bold.ttf"_s }, u"ofl/ranga/OFL.txt"_s ),
    GoogleFontDetails( u"Rasa"_s, { u"ofl/rasa/Rasa%5Bwght%5D.ttf"_s, u"ofl/rasa/Rasa-Italic%5Bwght%5D.ttf"_s }, u"ofl/rasa/OFL.txt"_s ),
    GoogleFontDetails( u"Rationale"_s, { u"ofl/rationale/Rationale-Regular.ttf"_s }, u"ofl/rationale/OFL.txt"_s ),
    GoogleFontDetails( u"Ravi Prakash"_s, { u"ofl/raviprakash/RaviPrakash-Regular.ttf"_s }, u"ofl/raviprakash/OFL.txt"_s ),
    GoogleFontDetails( u"Readex Pro"_s, { u"ofl/readexpro/ReadexPro%5BHEXP,wght%5D.ttf"_s }, u"ofl/readexpro/OFL.txt"_s ),
    GoogleFontDetails( u"Recursive"_s, { u"ofl/recursive/Recursive%5BCASL,CRSV,MONO,slnt,wght%5D.ttf"_s }, u"ofl/recursive/OFL.txt"_s ),
    GoogleFontDetails( u"Red Hat Display"_s, { u"ofl/redhatdisplay/RedHatDisplay%5Bwght%5D.ttf"_s, u"ofl/redhatdisplay/RedHatDisplay-Italic%5Bwght%5D.ttf"_s }, u"ofl/redhatdisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Red Hat Mono"_s, { u"ofl/redhatmono/RedHatMono%5Bwght%5D.ttf"_s, u"ofl/redhatmono/RedHatMono-Italic%5Bwght%5D.ttf"_s }, u"ofl/redhatmono/OFL.txt"_s ),
    GoogleFontDetails( u"Red Hat Text"_s, { u"ofl/redhattext/RedHatText%5Bwght%5D.ttf"_s, u"ofl/redhattext/RedHatText-Italic%5Bwght%5D.ttf"_s }, u"ofl/redhattext/OFL.txt"_s ),
    GoogleFontDetails( u"Red Rose"_s, { u"ofl/redrose/RedRose%5Bwght%5D.ttf"_s }, u"ofl/redrose/OFL.txt"_s ),
    GoogleFontDetails( u"Redacted"_s, { u"ofl/redacted/Redacted-Regular.ttf"_s }, u"ofl/redacted/OFL.txt"_s ),
    GoogleFontDetails( u"Redacted Script"_s, { u"ofl/redactedscript/RedactedScript-Light.ttf"_s, u"ofl/redactedscript/RedactedScript-Regular.ttf"_s, u"ofl/redactedscript/RedactedScript-Bold.ttf"_s }, u"ofl/redactedscript/OFL.txt"_s ),
    GoogleFontDetails( u"Redressed"_s, { u"apache/redressed/Redressed-Regular.ttf"_s }, u"apache/redressed/LICENSE.txt"_s ),
    GoogleFontDetails( u"Reem Kufi"_s, { u"ofl/reemkufi/ReemKufi%5Bwght%5D.ttf"_s }, u"ofl/reemkufi/OFL.txt"_s ),
    GoogleFontDetails( u"Reem Kufi Fun"_s, { u"ofl/reemkufifun/ReemKufiFun%5Bwght%5D.ttf"_s }, u"ofl/reemkufifun/OFL.txt"_s ),
    GoogleFontDetails( u"Reem Kufi Ink"_s, { u"ofl/reemkufiink/ReemKufiInk-Regular.ttf"_s }, u"ofl/reemkufiink/OFL.txt"_s ),
    GoogleFontDetails( u"Reenie Beanie"_s, { u"ofl/reeniebeanie/ReenieBeanie.ttf"_s }, u"ofl/reeniebeanie/OFL.txt"_s ),
    GoogleFontDetails( u"Reggae One"_s, { u"ofl/reggaeone/ReggaeOne-Regular.ttf"_s }, u"ofl/reggaeone/OFL.txt"_s ),
    GoogleFontDetails( u"Revalia"_s, { u"ofl/revalia/Revalia-Regular.ttf"_s }, u"ofl/revalia/OFL.txt"_s ),
    GoogleFontDetails( u"Rhodium Libre"_s, { u"ofl/rhodiumlibre/RhodiumLibre-Regular.ttf"_s }, u"ofl/rhodiumlibre/OFL.txt"_s ),
    GoogleFontDetails( u"Ribeye"_s, { u"ofl/ribeye/Ribeye-Regular.ttf"_s }, u"ofl/ribeye/OFL.txt"_s ),
    GoogleFontDetails( u"Ribeye Marrow"_s, { u"ofl/ribeyemarrow/RibeyeMarrow-Regular.ttf"_s }, u"ofl/ribeyemarrow/OFL.txt"_s ),
    GoogleFontDetails( u"Righteous"_s, { u"ofl/righteous/Righteous-Regular.ttf"_s }, u"ofl/righteous/OFL.txt"_s ),
    GoogleFontDetails( u"Risque"_s, { u"ofl/risque/Risque-Regular.ttf"_s }, u"ofl/risque/OFL.txt"_s ),
    GoogleFontDetails( u"Road Rage"_s, { u"ofl/roadrage/RoadRage-Regular.ttf"_s }, u"ofl/roadrage/OFL.txt"_s ),
    GoogleFontDetails( u"Roboto"_s, { u"ofl/roboto/Roboto%5Bwdth,wght%5D.ttf"_s, u"ofl/roboto/Roboto-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/roboto/OFL.txt"_s ),
    GoogleFontDetails( u"Roboto Condensed"_s, { u"ofl/robotocondensed/RobotoCondensed%5Bwght%5D.ttf"_s, u"ofl/robotocondensed/RobotoCondensed-Italic%5Bwght%5D.ttf"_s }, u"ofl/robotocondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Roboto Flex"_s, { u"ofl/robotoflex/RobotoFlex%5BGRAD,XOPQ,XTRA,YOPQ,YTAS,YTDE,YTFI,YTLC,YTUC,opsz,slnt,wdth,wght%5D.ttf"_s }, u"ofl/robotoflex/OFL.txt"_s ),
    GoogleFontDetails( u"Roboto Mono"_s, { u"apache/robotomono/RobotoMono%5Bwght%5D.ttf"_s, u"apache/robotomono/RobotoMono-Italic%5Bwght%5D.ttf"_s }, u"apache/robotomono/LICENSE.txt"_s ),
    GoogleFontDetails( u"Roboto Serif"_s, { u"ofl/robotoserif/RobotoSerif%5BGRAD,opsz,wdth,wght%5D.ttf"_s, u"ofl/robotoserif/RobotoSerif-Italic%5BGRAD,opsz,wdth,wght%5D.ttf"_s }, u"ofl/robotoserif/OFL.txt"_s ),
    GoogleFontDetails( u"Roboto Slab"_s, { u"apache/robotoslab/RobotoSlab%5Bwght%5D.ttf"_s }, u"apache/robotoslab/LICENSE.txt"_s ),
    GoogleFontDetails( u"Rochester"_s, { u"apache/rochester/Rochester-Regular.ttf"_s }, u"apache/rochester/LICENSE.txt"_s ),
    GoogleFontDetails( u"Rock 3D"_s, { u"ofl/rock3d/Rock3D-Regular.ttf"_s }, u"ofl/rock3d/OFL.txt"_s ),
    GoogleFontDetails( u"Rock Salt"_s, { u"apache/rocksalt/RockSalt-Regular.ttf"_s }, u"apache/rocksalt/LICENSE.txt"_s ),
    GoogleFontDetails( u"RocknRoll One"_s, { u"ofl/rocknrollone/RocknRollOne-Regular.ttf"_s }, u"ofl/rocknrollone/OFL.txt"_s ),
    GoogleFontDetails( u"Rokkitt"_s, { u"ofl/rokkitt/Rokkitt%5Bwght%5D.ttf"_s, u"ofl/rokkitt/Rokkitt-Italic%5Bwght%5D.ttf"_s }, u"ofl/rokkitt/OFL.txt"_s ),
    GoogleFontDetails( u"Romanesco"_s, { u"ofl/romanesco/Romanesco-Regular.ttf"_s }, u"ofl/romanesco/OFL.txt"_s ),
    GoogleFontDetails( u"Ropa Sans"_s, { u"ofl/ropasans/RopaSans-Regular.ttf"_s, u"ofl/ropasans/RopaSans-Italic.ttf"_s }, u"ofl/ropasans/OFL.txt"_s ),
    GoogleFontDetails( u"Rosario"_s, { u"ofl/rosario/Rosario%5Bwght%5D.ttf"_s, u"ofl/rosario/Rosario-Italic%5Bwght%5D.ttf"_s }, u"ofl/rosario/OFL.txt"_s ),
    GoogleFontDetails( u"Rosarivo"_s, { u"ofl/rosarivo/Rosarivo-Regular.ttf"_s, u"ofl/rosarivo/Rosarivo-Italic.ttf"_s }, u"ofl/rosarivo/OFL.txt"_s ),
    GoogleFontDetails( u"Rouge Script"_s, { u"ofl/rougescript/RougeScript-Regular.ttf"_s }, u"ofl/rougescript/OFL.txt"_s ),
    GoogleFontDetails( u"Rowdies"_s, { u"ofl/rowdies/Rowdies-Light.ttf"_s, u"ofl/rowdies/Rowdies-Regular.ttf"_s, u"ofl/rowdies/Rowdies-Bold.ttf"_s }, u"ofl/rowdies/OFL.txt"_s ),
    GoogleFontDetails( u"Rozha One"_s, { u"ofl/rozhaone/RozhaOne-Regular.ttf"_s }, u"ofl/rozhaone/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik"_s, { u"ofl/rubik/Rubik%5Bwght%5D.ttf"_s, u"ofl/rubik/Rubik-Italic%5Bwght%5D.ttf"_s }, u"ofl/rubik/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik 80s Fade"_s, { u"ofl/rubik80sfade/Rubik80sFade-Regular.ttf"_s }, u"ofl/rubik80sfade/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Beastly"_s, { u"ofl/rubikbeastly/RubikBeastly-Regular.ttf"_s }, u"ofl/rubikbeastly/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Bubbles"_s, { u"ofl/rubikbubbles/RubikBubbles-Regular.ttf"_s }, u"ofl/rubikbubbles/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Burned"_s, { u"ofl/rubikburned/RubikBurned-Regular.ttf"_s }, u"ofl/rubikburned/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Dirt"_s, { u"ofl/rubikdirt/RubikDirt-Regular.ttf"_s }, u"ofl/rubikdirt/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Distressed"_s, { u"ofl/rubikdistressed/RubikDistressed-Regular.ttf"_s }, u"ofl/rubikdistressed/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Gemstones"_s, { u"ofl/rubikgemstones/RubikGemstones-Regular.ttf"_s }, u"ofl/rubikgemstones/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Glitch"_s, { u"ofl/rubikglitch/RubikGlitch-Regular.ttf"_s }, u"ofl/rubikglitch/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Iso"_s, { u"ofl/rubikiso/RubikIso-Regular.ttf"_s }, u"ofl/rubikiso/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Marker Hatch"_s, { u"ofl/rubikmarkerhatch/RubikMarkerHatch-Regular.ttf"_s }, u"ofl/rubikmarkerhatch/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Maze"_s, { u"ofl/rubikmaze/RubikMaze-Regular.ttf"_s }, u"ofl/rubikmaze/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Microbe"_s, { u"ofl/rubikmicrobe/RubikMicrobe-Regular.ttf"_s }, u"ofl/rubikmicrobe/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Mono One"_s, { u"ofl/rubikmonoone/RubikMonoOne-Regular.ttf"_s }, u"ofl/rubikmonoone/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Moonrocks"_s, { u"ofl/rubikmoonrocks/RubikMoonrocks-Regular.ttf"_s }, u"ofl/rubikmoonrocks/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik One"_s, { u"ofl/rubikone/RubikOne-Regular.ttf"_s }, u"ofl/rubikone/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Pixels"_s, { u"ofl/rubikpixels/RubikPixels-Regular.ttf"_s }, u"ofl/rubikpixels/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Puddles"_s, { u"ofl/rubikpuddles/RubikPuddles-Regular.ttf"_s }, u"ofl/rubikpuddles/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Spray Paint"_s, { u"ofl/rubikspraypaint/RubikSprayPaint-Regular.ttf"_s }, u"ofl/rubikspraypaint/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Storm"_s, { u"ofl/rubikstorm/RubikStorm-Regular.ttf"_s }, u"ofl/rubikstorm/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Vinyl"_s, { u"ofl/rubikvinyl/RubikVinyl-Regular.ttf"_s }, u"ofl/rubikvinyl/OFL.txt"_s ),
    GoogleFontDetails( u"Rubik Wet Paint"_s, { u"ofl/rubikwetpaint/RubikWetPaint-Regular.ttf"_s }, u"ofl/rubikwetpaint/OFL.txt"_s ),
    GoogleFontDetails( u"Ruda"_s, { u"ofl/ruda/Ruda%5Bwght%5D.ttf"_s }, u"ofl/ruda/OFL.txt"_s ),
    GoogleFontDetails( u"Rufina"_s, { u"ofl/rufina/Rufina-Regular.ttf"_s, u"ofl/rufina/Rufina-Bold.ttf"_s }, u"ofl/rufina/OFL.txt"_s ),
    GoogleFontDetails( u"Ruge Boogie"_s, { u"ofl/rugeboogie/RugeBoogie-Regular.ttf"_s }, u"ofl/rugeboogie/OFL.txt"_s ),
    GoogleFontDetails( u"Ruluko"_s, { u"ofl/ruluko/Ruluko-Regular.ttf"_s }, u"ofl/ruluko/OFL.txt"_s ),
    GoogleFontDetails( u"Rum Raisin"_s, { u"ofl/rumraisin/RumRaisin-Regular.ttf"_s }, u"ofl/rumraisin/OFL.txt"_s ),
    GoogleFontDetails( u"Ruslan Display"_s, { u"ofl/ruslandisplay/RuslanDisplay-Regular.ttf"_s }, u"ofl/ruslandisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Russo One"_s, { u"ofl/russoone/RussoOne-Regular.ttf"_s }, u"ofl/russoone/OFL.txt"_s ),
    GoogleFontDetails( u"Ruthie"_s, { u"ofl/ruthie/Ruthie-Regular.ttf"_s }, u"ofl/ruthie/OFL.txt"_s ),
    GoogleFontDetails( u"Ruwudu"_s, { u"ofl/ruwudu/Ruwudu-Regular.ttf"_s, u"ofl/ruwudu/Ruwudu-Medium.ttf"_s, u"ofl/ruwudu/Ruwudu-SemiBold.ttf"_s, u"ofl/ruwudu/Ruwudu-Bold.ttf"_s }, u"ofl/ruwudu/OFL.txt"_s ),
    GoogleFontDetails( u"Rye"_s, { u"ofl/rye/Rye-Regular.ttf"_s }, u"ofl/rye/OFL.txt"_s ),
    GoogleFontDetails( u"STIX Two Text"_s, { u"ofl/stixtwotext/STIXTwoText%5Bwght%5D.ttf"_s, u"ofl/stixtwotext/STIXTwoText-Italic%5Bwght%5D.ttf"_s }, u"ofl/stixtwotext/OFL.txt"_s ),
    GoogleFontDetails( u"Sacramento"_s, { u"ofl/sacramento/Sacramento-Regular.ttf"_s }, u"ofl/sacramento/OFL.txt"_s ),
    GoogleFontDetails( u"Sahitya"_s, { u"ofl/sahitya/Sahitya-Regular.ttf"_s, u"ofl/sahitya/Sahitya-Bold.ttf"_s }, u"ofl/sahitya/OFL.txt"_s ),
    GoogleFontDetails( u"Sail"_s, { u"ofl/sail/Sail-Regular.ttf"_s }, u"ofl/sail/OFL.txt"_s ),
    GoogleFontDetails( u"Saira"_s, { u"ofl/saira/Saira%5Bwdth,wght%5D.ttf"_s, u"ofl/saira/Saira-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/saira/OFL.txt"_s ),
    GoogleFontDetails( u"Saira Condensed"_s, { u"ofl/sairacondensed/SairaCondensed-Thin.ttf"_s, u"ofl/sairacondensed/SairaCondensed-ExtraLight.ttf"_s, u"ofl/sairacondensed/SairaCondensed-Light.ttf"_s, u"ofl/sairacondensed/SairaCondensed-Regular.ttf"_s, u"ofl/sairacondensed/SairaCondensed-Medium.ttf"_s, u"ofl/sairacondensed/SairaCondensed-SemiBold.ttf"_s, u"ofl/sairacondensed/SairaCondensed-Bold.ttf"_s, u"ofl/sairacondensed/SairaCondensed-ExtraBold.ttf"_s, u"ofl/sairacondensed/SairaCondensed-Black.ttf"_s }, u"ofl/sairacondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Saira Extra Condensed"_s, { u"ofl/sairaextracondensed/SairaExtraCondensed-Thin.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-ExtraLight.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-Light.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-Regular.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-Medium.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-SemiBold.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-Bold.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-ExtraBold.ttf"_s, u"ofl/sairaextracondensed/SairaExtraCondensed-Black.ttf"_s }, u"ofl/sairaextracondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Saira Semi Condensed"_s, { u"ofl/sairasemicondensed/SairaSemiCondensed-Thin.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-ExtraLight.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-Light.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-Regular.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-Medium.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-SemiBold.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-Bold.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-ExtraBold.ttf"_s, u"ofl/sairasemicondensed/SairaSemiCondensed-Black.ttf"_s }, u"ofl/sairasemicondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Saira Stencil One"_s, { u"ofl/sairastencilone/SairaStencilOne-Regular.ttf"_s }, u"ofl/sairastencilone/OFL.txt"_s ),
    GoogleFontDetails( u"Salsa"_s, { u"ofl/salsa/Salsa-Regular.ttf"_s }, u"ofl/salsa/OFL.txt"_s ),
    GoogleFontDetails( u"Sanchez"_s, { u"ofl/sanchez/Sanchez-Regular.ttf"_s, u"ofl/sanchez/Sanchez-Italic.ttf"_s }, u"ofl/sanchez/OFL.txt"_s ),
    GoogleFontDetails( u"Sancreek"_s, { u"ofl/sancreek/Sancreek-Regular.ttf"_s }, u"ofl/sancreek/OFL.txt"_s ),
    GoogleFontDetails( u"Sansita"_s, { u"ofl/sansita/Sansita-Regular.ttf"_s, u"ofl/sansita/Sansita-Italic.ttf"_s, u"ofl/sansita/Sansita-Bold.ttf"_s, u"ofl/sansita/Sansita-BoldItalic.ttf"_s, u"ofl/sansita/Sansita-ExtraBold.ttf"_s, u"ofl/sansita/Sansita-ExtraBoldItalic.ttf"_s, u"ofl/sansita/Sansita-Black.ttf"_s, u"ofl/sansita/Sansita-BlackItalic.ttf"_s }, u"ofl/sansita/OFL.txt"_s ),
    GoogleFontDetails( u"Sansita One"_s, { u"ofl/sansitaone/SansitaOne-Regular.ttf"_s }, u"ofl/sansitaone/OFL.txt"_s ),
    GoogleFontDetails( u"Sansita Swashed"_s, { u"ofl/sansitaswashed/SansitaSwashed%5Bwght%5D.ttf"_s }, u"ofl/sansitaswashed/OFL.txt"_s ),
    GoogleFontDetails( u"Sarabun"_s, { u"ofl/sarabun/Sarabun-Thin.ttf"_s, u"ofl/sarabun/Sarabun-ThinItalic.ttf"_s, u"ofl/sarabun/Sarabun-ExtraLight.ttf"_s, u"ofl/sarabun/Sarabun-ExtraLightItalic.ttf"_s, u"ofl/sarabun/Sarabun-Light.ttf"_s, u"ofl/sarabun/Sarabun-LightItalic.ttf"_s, u"ofl/sarabun/Sarabun-Regular.ttf"_s, u"ofl/sarabun/Sarabun-Italic.ttf"_s, u"ofl/sarabun/Sarabun-Medium.ttf"_s, u"ofl/sarabun/Sarabun-MediumItalic.ttf"_s, u"ofl/sarabun/Sarabun-SemiBold.ttf"_s, u"ofl/sarabun/Sarabun-SemiBoldItalic.ttf"_s, u"ofl/sarabun/Sarabun-Bold.ttf"_s, u"ofl/sarabun/Sarabun-BoldItalic.ttf"_s, u"ofl/sarabun/Sarabun-ExtraBold.ttf"_s, u"ofl/sarabun/Sarabun-ExtraBoldItalic.ttf"_s }, u"ofl/sarabun/OFL.txt"_s ),
    GoogleFontDetails( u"Sarala"_s, { u"ofl/sarala/Sarala-Regular.ttf"_s, u"ofl/sarala/Sarala-Bold.ttf"_s }, u"ofl/sarala/OFL.txt"_s ),
    GoogleFontDetails( u"Sarina"_s, { u"ofl/sarina/Sarina-Regular.ttf"_s }, u"ofl/sarina/OFL.txt"_s ),
    GoogleFontDetails( u"Sarpanch"_s, { u"ofl/sarpanch/Sarpanch-Regular.ttf"_s, u"ofl/sarpanch/Sarpanch-Medium.ttf"_s, u"ofl/sarpanch/Sarpanch-SemiBold.ttf"_s, u"ofl/sarpanch/Sarpanch-Bold.ttf"_s, u"ofl/sarpanch/Sarpanch-ExtraBold.ttf"_s, u"ofl/sarpanch/Sarpanch-Black.ttf"_s }, u"ofl/sarpanch/OFL.txt"_s ),
    GoogleFontDetails( u"Sassy Frass"_s, { u"ofl/sassyfrass/SassyFrass-Regular.ttf"_s }, u"ofl/sassyfrass/OFL.txt"_s ),
    GoogleFontDetails( u"Satisfy"_s, { u"apache/satisfy/Satisfy-Regular.ttf"_s }, u"apache/satisfy/LICENSE.txt"_s ),
    GoogleFontDetails( u"Sawarabi Mincho"_s, { u"ofl/sawarabimincho/SawarabiMincho-Regular.ttf"_s }, u"ofl/sawarabimincho/OFL.txt"_s ),
    GoogleFontDetails( u"Scada"_s, { u"ofl/scada/Scada-Regular.ttf"_s, u"ofl/scada/Scada-Italic.ttf"_s, u"ofl/scada/Scada-Bold.ttf"_s, u"ofl/scada/Scada-BoldItalic.ttf"_s }, u"ofl/scada/OFL.txt"_s ),
    GoogleFontDetails( u"Scheherazade New"_s, { u"ofl/scheherazadenew/ScheherazadeNew-Regular.ttf"_s, u"ofl/scheherazadenew/ScheherazadeNew-Medium.ttf"_s, u"ofl/scheherazadenew/ScheherazadeNew-SemiBold.ttf"_s, u"ofl/scheherazadenew/ScheherazadeNew-Bold.ttf"_s }, u"ofl/scheherazadenew/OFL.txt"_s ),
    GoogleFontDetails( u"Schibsted Grotesk"_s, { u"ofl/schibstedgrotesk/SchibstedGrotesk%5Bwght%5D.ttf"_s, u"ofl/schibstedgrotesk/SchibstedGrotesk-Italic%5Bwght%5D.ttf"_s }, u"ofl/schibstedgrotesk/OFL.txt"_s ),
    GoogleFontDetails( u"Schoolbell"_s, { u"apache/schoolbell/Schoolbell-Regular.ttf"_s }, u"apache/schoolbell/LICENSE.txt"_s ),
    GoogleFontDetails( u"Scope One"_s, { u"ofl/scopeone/ScopeOne-Regular.ttf"_s }, u"ofl/scopeone/OFL.txt"_s ),
    GoogleFontDetails( u"Seaweed Script"_s, { u"ofl/seaweedscript/SeaweedScript-Regular.ttf"_s }, u"ofl/seaweedscript/OFL.txt"_s ),
    GoogleFontDetails( u"Secular One"_s, { u"ofl/secularone/SecularOne-Regular.ttf"_s }, u"ofl/secularone/OFL.txt"_s ),
    GoogleFontDetails( u"Sedgwick Ave"_s, { u"ofl/sedgwickave/SedgwickAve-Regular.ttf"_s }, u"ofl/sedgwickave/OFL.txt"_s ),
    GoogleFontDetails( u"Sedgwick Ave Display"_s, { u"ofl/sedgwickavedisplay/SedgwickAveDisplay-Regular.ttf"_s }, u"ofl/sedgwickavedisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Sen"_s, { u"ofl/sen/Sen%5Bwght%5D.ttf"_s }, u"ofl/sen/OFL.txt"_s ),
    GoogleFontDetails( u"Send Flowers"_s, { u"ofl/sendflowers/SendFlowers-Regular.ttf"_s }, u"ofl/sendflowers/OFL.txt"_s ),
    GoogleFontDetails( u"Sevillana"_s, { u"ofl/sevillana/Sevillana-Regular.ttf"_s }, u"ofl/sevillana/OFL.txt"_s ),
    GoogleFontDetails( u"Seymour One"_s, { u"ofl/seymourone/SeymourOne-Regular.ttf"_s }, u"ofl/seymourone/OFL.txt"_s ),
    GoogleFontDetails( u"Shadows Into Light"_s, { u"ofl/shadowsintolight/ShadowsIntoLight.ttf"_s }, u"ofl/shadowsintolight/OFL.txt"_s ),
    GoogleFontDetails( u"Shadows Into Light Two"_s, { u"ofl/shadowsintolighttwo/ShadowsIntoLightTwo-Regular.ttf"_s }, u"ofl/shadowsintolighttwo/OFL.txt"_s ),
    GoogleFontDetails( u"Shalimar"_s, { u"ofl/shalimar/Shalimar-Regular.ttf"_s }, u"ofl/shalimar/OFL.txt"_s ),
    GoogleFontDetails( u"Shantell Sans"_s, { u"ofl/shantellsans/ShantellSans%5BBNCE,INFM,SPAC,wght%5D.ttf"_s, u"ofl/shantellsans/ShantellSans-Italic%5BBNCE,INFM,SPAC,wght%5D.ttf"_s }, u"ofl/shantellsans/OFL.txt"_s ),
    GoogleFontDetails( u"Shanti"_s, { u"ofl/shanti/Shanti-Regular.ttf"_s }, u"ofl/shanti/OFL.txt"_s ),
    GoogleFontDetails( u"Share"_s, { u"ofl/share/Share-Regular.ttf"_s, u"ofl/share/Share-Italic.ttf"_s, u"ofl/share/Share-Bold.ttf"_s, u"ofl/share/Share-BoldItalic.ttf"_s }, u"ofl/share/OFL.txt"_s ),
    GoogleFontDetails( u"Share Tech"_s, { u"ofl/sharetech/ShareTech-Regular.ttf"_s }, u"ofl/sharetech/OFL.txt"_s ),
    GoogleFontDetails( u"Share Tech Mono"_s, { u"ofl/sharetechmono/ShareTechMono-Regular.ttf"_s }, u"ofl/sharetechmono/OFL.txt"_s ),
    GoogleFontDetails( u"Shippori Antique"_s, { u"ofl/shipporiantique/ShipporiAntique-Regular.ttf"_s }, u"ofl/shipporiantique/OFL.txt"_s ),
    GoogleFontDetails( u"Shippori Antique B1"_s, { u"ofl/shipporiantiqueb1/ShipporiAntiqueB1-Regular.ttf"_s }, u"ofl/shipporiantiqueb1/OFL.txt"_s ),
    GoogleFontDetails( u"Shippori Mincho"_s, { u"ofl/shipporimincho/ShipporiMincho-Regular.ttf"_s, u"ofl/shipporimincho/ShipporiMincho-Medium.ttf"_s, u"ofl/shipporimincho/ShipporiMincho-SemiBold.ttf"_s, u"ofl/shipporimincho/ShipporiMincho-Bold.ttf"_s, u"ofl/shipporimincho/ShipporiMincho-ExtraBold.ttf"_s }, u"ofl/shipporimincho/OFL.txt"_s ),
    GoogleFontDetails( u"Shippori Mincho B1"_s, { u"ofl/shipporiminchob1/ShipporiMinchoB1-Regular.ttf"_s, u"ofl/shipporiminchob1/ShipporiMinchoB1-Medium.ttf"_s, u"ofl/shipporiminchob1/ShipporiMinchoB1-SemiBold.ttf"_s, u"ofl/shipporiminchob1/ShipporiMinchoB1-Bold.ttf"_s, u"ofl/shipporiminchob1/ShipporiMinchoB1-ExtraBold.ttf"_s }, u"ofl/shipporiminchob1/OFL.txt"_s ),
    GoogleFontDetails( u"Shizuru"_s, { u"ofl/shizuru/Shizuru-Regular.ttf"_s }, u"ofl/shizuru/OFL.txt"_s ),
    GoogleFontDetails( u"Shojumaru"_s, { u"ofl/shojumaru/Shojumaru-Regular.ttf"_s }, u"ofl/shojumaru/OFL.txt"_s ),
    GoogleFontDetails( u"Short Stack"_s, { u"ofl/shortstack/ShortStack-Regular.ttf"_s }, u"ofl/shortstack/OFL.txt"_s ),
    GoogleFontDetails( u"Shrikhand"_s, { u"ofl/shrikhand/Shrikhand-Regular.ttf"_s }, u"ofl/shrikhand/OFL.txt"_s ),
    GoogleFontDetails( u"Siemreap"_s, { u"ofl/siemreap/Siemreap.ttf"_s }, u"ofl/siemreap/OFL.txt"_s ),
    GoogleFontDetails( u"Sigmar"_s, { u"ofl/sigmar/Sigmar-Regular.ttf"_s }, u"ofl/sigmar/OFL.txt"_s ),
    GoogleFontDetails( u"Sigmar One"_s, { u"ofl/sigmarone/SigmarOne-Regular.ttf"_s }, u"ofl/sigmarone/OFL.txt"_s ),
    GoogleFontDetails( u"Signika"_s, { u"ofl/signika/Signika%5BGRAD,wght%5D.ttf"_s }, u"ofl/signika/OFL.txt"_s ),
    GoogleFontDetails( u"Signika Negative"_s, { u"ofl/signikanegative/SignikaNegative%5Bwght%5D.ttf"_s }, u"ofl/signikanegative/OFL.txt"_s ),
    GoogleFontDetails( u"Silkscreen"_s, { u"ofl/silkscreen/Silkscreen-Regular.ttf"_s, u"ofl/silkscreen/Silkscreen-Bold.ttf"_s }, u"ofl/silkscreen/OFL.txt"_s ),
    GoogleFontDetails( u"Simonetta"_s, { u"ofl/simonetta/Simonetta-Regular.ttf"_s, u"ofl/simonetta/Simonetta-Italic.ttf"_s, u"ofl/simonetta/Simonetta-Black.ttf"_s, u"ofl/simonetta/Simonetta-BlackItalic.ttf"_s }, u"ofl/simonetta/OFL.txt"_s ),
    GoogleFontDetails( u"Single Day"_s, { u"ofl/singleday/SingleDay-Regular.ttf"_s }, u"ofl/singleday/OFL.txt"_s ),
    GoogleFontDetails( u"Sintony"_s, { u"ofl/sintony/Sintony-Regular.ttf"_s, u"ofl/sintony/Sintony-Bold.ttf"_s }, u"ofl/sintony/OFL.txt"_s ),
    GoogleFontDetails( u"Sirin Stencil"_s, { u"ofl/sirinstencil/SirinStencil-Regular.ttf"_s }, u"ofl/sirinstencil/OFL.txt"_s ),
    GoogleFontDetails( u"Six Caps"_s, { u"ofl/sixcaps/SixCaps.ttf"_s }, u"ofl/sixcaps/OFL.txt"_s ),
    GoogleFontDetails( u"Skranji"_s, { u"ofl/skranji/Skranji-Regular.ttf"_s, u"ofl/skranji/Skranji-Bold.ttf"_s }, u"ofl/skranji/OFL.txt"_s ),
    GoogleFontDetails( u"Slabo 13px"_s, { u"ofl/slabo13px/Slabo13px-Regular.ttf"_s }, u"ofl/slabo13px/OFL.txt"_s ),
    GoogleFontDetails( u"Slabo 27px"_s, { u"ofl/slabo27px/Slabo27px-Regular.ttf"_s }, u"ofl/slabo27px/OFL.txt"_s ),
    GoogleFontDetails( u"Slackey"_s, { u"apache/slackey/Slackey-Regular.ttf"_s }, u"apache/slackey/LICENSE.txt"_s ),
    GoogleFontDetails( u"Slackside One"_s, { u"ofl/slacksideone/SlacksideOne-Regular.ttf"_s }, u"ofl/slacksideone/OFL.txt"_s ),
    GoogleFontDetails( u"Smokum"_s, { u"apache/smokum/Smokum-Regular.ttf"_s }, u"apache/smokum/LICENSE.txt"_s ),
    GoogleFontDetails( u"Smooch"_s, { u"ofl/smooch/Smooch-Regular.ttf"_s }, u"ofl/smooch/OFL.txt"_s ),
    GoogleFontDetails( u"Smooch Sans"_s, { u"ofl/smoochsans/SmoochSans%5Bwght%5D.ttf"_s }, u"ofl/smoochsans/OFL.txt"_s ),
    GoogleFontDetails( u"Smythe"_s, { u"ofl/smythe/Smythe-Regular.ttf"_s }, u"ofl/smythe/OFL.txt"_s ),
    GoogleFontDetails( u"Sniglet"_s, { u"ofl/sniglet/Sniglet-Regular.ttf"_s, u"ofl/sniglet/Sniglet-ExtraBold.ttf"_s }, u"ofl/sniglet/OFL.txt"_s ),
    GoogleFontDetails( u"Snippet"_s, { u"ofl/snippet/Snippet.ttf"_s }, u"ofl/snippet/OFL.txt"_s ),
    GoogleFontDetails( u"Snowburst One"_s, { u"ofl/snowburstone/SnowburstOne-Regular.ttf"_s }, u"ofl/snowburstone/OFL.txt"_s ),
    GoogleFontDetails( u"Sofadi One"_s, { u"ofl/sofadione/SofadiOne-Regular.ttf"_s }, u"ofl/sofadione/OFL.txt"_s ),
    GoogleFontDetails( u"Sofia"_s, { u"ofl/sofia/Sofia-Regular.ttf"_s }, u"ofl/sofia/OFL.txt"_s ),
    GoogleFontDetails( u"Sofia Sans"_s, { u"ofl/sofiasans/SofiaSans%5Bwght%5D.ttf"_s, u"ofl/sofiasans/SofiaSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/sofiasans/OFL.txt"_s ),
    GoogleFontDetails( u"Sofia Sans Condensed"_s, { u"ofl/sofiasanscondensed/SofiaSansCondensed%5Bwght%5D.ttf"_s, u"ofl/sofiasanscondensed/SofiaSansCondensed-Italic%5Bwght%5D.ttf"_s }, u"ofl/sofiasanscondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Sofia Sans Extra Condensed"_s, { u"ofl/sofiasansextracondensed/SofiaSansExtraCondensed%5Bwght%5D.ttf"_s, u"ofl/sofiasansextracondensed/SofiaSansExtraCondensed-Italic%5Bwght%5D.ttf"_s }, u"ofl/sofiasansextracondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Sofia Sans Semi Condensed"_s, { u"ofl/sofiasanssemicondensed/SofiaSansSemiCondensed%5Bwght%5D.ttf"_s, u"ofl/sofiasanssemicondensed/SofiaSansSemiCondensed-Italic%5Bwght%5D.ttf"_s }, u"ofl/sofiasanssemicondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Solitreo"_s, { u"ofl/solitreo/Solitreo-Regular.ttf"_s }, u"ofl/solitreo/OFL.txt"_s ),
    GoogleFontDetails( u"Solway"_s, { u"ofl/solway/Solway-Light.ttf"_s, u"ofl/solway/Solway-Regular.ttf"_s, u"ofl/solway/Solway-Medium.ttf"_s, u"ofl/solway/Solway-Bold.ttf"_s, u"ofl/solway/Solway-ExtraBold.ttf"_s }, u"ofl/solway/OFL.txt"_s ),
    GoogleFontDetails( u"Song Myung"_s, { u"ofl/songmyung/SongMyung-Regular.ttf"_s }, u"ofl/songmyung/OFL.txt"_s ),
    GoogleFontDetails( u"Sono"_s, { u"ofl/sono/Sono%5BMONO,wght%5D.ttf"_s }, u"ofl/sono/OFL.txt"_s ),
    GoogleFontDetails( u"Sonsie One"_s, { u"ofl/sonsieone/SonsieOne-Regular.ttf"_s }, u"ofl/sonsieone/OFL.txt"_s ),
    GoogleFontDetails( u"Sora"_s, { u"ofl/sora/Sora%5Bwght%5D.ttf"_s }, u"ofl/sora/OFL.txt"_s ),
    GoogleFontDetails( u"Sorts Mill Goudy"_s, { u"ofl/sortsmillgoudy/SortsMillGoudy-Regular.ttf"_s, u"ofl/sortsmillgoudy/SortsMillGoudy-Italic.ttf"_s }, u"ofl/sortsmillgoudy/OFL.txt"_s ),
    GoogleFontDetails( u"Source Code Pro"_s, { u"ofl/sourcecodepro/SourceCodePro%5Bwght%5D.ttf"_s, u"ofl/sourcecodepro/SourceCodePro-Italic%5Bwght%5D.ttf"_s }, u"ofl/sourcecodepro/OFL.txt"_s ),
    GoogleFontDetails( u"Source Sans 3"_s, { u"ofl/sourcesans3/SourceSans3%5Bwght%5D.ttf"_s, u"ofl/sourcesans3/SourceSans3-Italic%5Bwght%5D.ttf"_s }, u"ofl/sourcesans3/OFL.txt"_s ),
    GoogleFontDetails( u"Space Grotesk"_s, { u"ofl/spacegrotesk/SpaceGrotesk%5Bwght%5D.ttf"_s }, u"ofl/spacegrotesk/OFL.txt"_s ),
    GoogleFontDetails( u"Space Mono"_s, { u"ofl/spacemono/SpaceMono-Regular.ttf"_s, u"ofl/spacemono/SpaceMono-Italic.ttf"_s, u"ofl/spacemono/SpaceMono-Bold.ttf"_s, u"ofl/spacemono/SpaceMono-BoldItalic.ttf"_s }, u"ofl/spacemono/OFL.txt"_s ),
    GoogleFontDetails( u"Special Elite"_s, { u"apache/specialelite/SpecialElite-Regular.ttf"_s }, u"apache/specialelite/LICENSE.txt"_s ),
    GoogleFontDetails( u"Spectral"_s, { u"ofl/spectral/Spectral-ExtraLight.ttf"_s, u"ofl/spectral/Spectral-ExtraLightItalic.ttf"_s, u"ofl/spectral/Spectral-Light.ttf"_s, u"ofl/spectral/Spectral-LightItalic.ttf"_s, u"ofl/spectral/Spectral-Regular.ttf"_s, u"ofl/spectral/Spectral-Italic.ttf"_s, u"ofl/spectral/Spectral-Medium.ttf"_s, u"ofl/spectral/Spectral-MediumItalic.ttf"_s, u"ofl/spectral/Spectral-SemiBold.ttf"_s, u"ofl/spectral/Spectral-SemiBoldItalic.ttf"_s, u"ofl/spectral/Spectral-Bold.ttf"_s, u"ofl/spectral/Spectral-BoldItalic.ttf"_s, u"ofl/spectral/Spectral-ExtraBold.ttf"_s, u"ofl/spectral/Spectral-ExtraBoldItalic.ttf"_s }, u"ofl/spectral/OFL.txt"_s ),
    GoogleFontDetails( u"Spicy Rice"_s, { u"ofl/spicyrice/SpicyRice-Regular.ttf"_s }, u"ofl/spicyrice/OFL.txt"_s ),
    GoogleFontDetails( u"Spinnaker"_s, { u"ofl/spinnaker/Spinnaker-Regular.ttf"_s }, u"ofl/spinnaker/OFL.txt"_s ),
    GoogleFontDetails( u"Spirax"_s, { u"ofl/spirax/Spirax-Regular.ttf"_s }, u"ofl/spirax/OFL.txt"_s ),
    GoogleFontDetails( u"Splash"_s, { u"ofl/splash/Splash-Regular.ttf"_s }, u"ofl/splash/OFL.txt"_s ),
    GoogleFontDetails( u"Spline Sans"_s, { u"ofl/splinesans/SplineSans%5Bwght%5D.ttf"_s }, u"ofl/splinesans/OFL.txt"_s ),
    GoogleFontDetails( u"Spline Sans Mono"_s, { u"ofl/splinesansmono/SplineSansMono%5Bwght%5D.ttf"_s, u"ofl/splinesansmono/SplineSansMono-Italic%5Bwght%5D.ttf"_s }, u"ofl/splinesansmono/OFL.txt"_s ),
    GoogleFontDetails( u"Squada One"_s, { u"ofl/squadaone/SquadaOne-Regular.ttf"_s }, u"ofl/squadaone/OFL.txt"_s ),
    GoogleFontDetails( u"Square Peg"_s, { u"ofl/squarepeg/SquarePeg-Regular.ttf"_s }, u"ofl/squarepeg/OFL.txt"_s ),
    GoogleFontDetails( u"Sree Krushnadevaraya"_s, { u"ofl/sreekrushnadevaraya/SreeKrushnadevaraya-Regular.ttf"_s }, u"ofl/sreekrushnadevaraya/OFL.txt"_s ),
    GoogleFontDetails( u"Sriracha"_s, { u"ofl/sriracha/Sriracha-Regular.ttf"_s }, u"ofl/sriracha/OFL.txt"_s ),
    GoogleFontDetails( u"Srisakdi"_s, { u"ofl/srisakdi/Srisakdi-Regular.ttf"_s, u"ofl/srisakdi/Srisakdi-Bold.ttf"_s }, u"ofl/srisakdi/OFL.txt"_s ),
    GoogleFontDetails( u"Staatliches"_s, { u"ofl/staatliches/Staatliches-Regular.ttf"_s }, u"ofl/staatliches/OFL.txt"_s ),
    GoogleFontDetails( u"Stalemate"_s, { u"ofl/stalemate/Stalemate-Regular.ttf"_s }, u"ofl/stalemate/OFL.txt"_s ),
    GoogleFontDetails( u"Stalinist One"_s, { u"ofl/stalinistone/StalinistOne-Regular.ttf"_s }, u"ofl/stalinistone/OFL.txt"_s ),
    GoogleFontDetails( u"Stardos Stencil"_s, { u"ofl/stardosstencil/StardosStencil-Regular.ttf"_s, u"ofl/stardosstencil/StardosStencil-Bold.ttf"_s }, u"ofl/stardosstencil/OFL.txt"_s ),
    GoogleFontDetails( u"Stick"_s, { u"ofl/stick/Stick-Regular.ttf"_s }, u"ofl/stick/OFL.txt"_s ),
    GoogleFontDetails( u"Stick No Bills"_s, { u"ofl/sticknobills/StickNoBills%5Bwght%5D.ttf"_s }, u"ofl/sticknobills/OFL.txt"_s ),
    GoogleFontDetails( u"Stint Ultra Condensed"_s, { u"ofl/stintultracondensed/StintUltraCondensed-Regular.ttf"_s }, u"ofl/stintultracondensed/OFL.txt"_s ),
    GoogleFontDetails( u"Stint Ultra Expanded"_s, { u"ofl/stintultraexpanded/StintUltraExpanded-Regular.ttf"_s }, u"ofl/stintultraexpanded/OFL.txt"_s ),
    GoogleFontDetails( u"Stoke"_s, { u"ofl/stoke/Stoke-Light.ttf"_s, u"ofl/stoke/Stoke-Regular.ttf"_s }, u"ofl/stoke/OFL.txt"_s ),
    GoogleFontDetails( u"Strait"_s, { u"ofl/strait/Strait-Regular.ttf"_s }, u"ofl/strait/OFL.txt"_s ),
    GoogleFontDetails( u"Style Script"_s, { u"ofl/stylescript/StyleScript-Regular.ttf"_s }, u"ofl/stylescript/OFL.txt"_s ),
    GoogleFontDetails( u"Stylish"_s, { u"ofl/stylish/Stylish-Regular.ttf"_s }, u"ofl/stylish/OFL.txt"_s ),
    GoogleFontDetails( u"Sue Ellen Francisco"_s, { u"ofl/sueellenfrancisco/SueEllenFrancisco-Regular.ttf"_s }, u"ofl/sueellenfrancisco/OFL.txt"_s ),
    GoogleFontDetails( u"Suez One"_s, { u"ofl/suezone/SuezOne-Regular.ttf"_s }, u"ofl/suezone/OFL.txt"_s ),
    GoogleFontDetails( u"Sulphur Point"_s, { u"ofl/sulphurpoint/SulphurPoint-Light.ttf"_s, u"ofl/sulphurpoint/SulphurPoint-Regular.ttf"_s, u"ofl/sulphurpoint/SulphurPoint-Bold.ttf"_s }, u"ofl/sulphurpoint/OFL.txt"_s ),
    GoogleFontDetails( u"Sumana"_s, { u"ofl/sumana/Sumana-Regular.ttf"_s, u"ofl/sumana/Sumana-Bold.ttf"_s }, u"ofl/sumana/OFL.txt"_s ),
    GoogleFontDetails( u"Sunflower"_s, { u"ofl/sunflower/Sunflower-Light.ttf"_s, u"ofl/sunflower/Sunflower-Medium.ttf"_s, u"ofl/sunflower/Sunflower-Bold.ttf"_s }, u"ofl/sunflower/OFL.txt"_s ),
    GoogleFontDetails( u"Sunshiney"_s, { u"apache/sunshiney/Sunshiney-Regular.ttf"_s }, u"apache/sunshiney/LICENSE.txt"_s ),
    GoogleFontDetails( u"Supermercado One"_s, { u"ofl/supermercadoone/SupermercadoOne-Regular.ttf"_s }, u"ofl/supermercadoone/OFL.txt"_s ),
    GoogleFontDetails( u"Sura"_s, { u"ofl/sura/Sura-Regular.ttf"_s, u"ofl/sura/Sura-Bold.ttf"_s }, u"ofl/sura/OFL.txt"_s ),
    GoogleFontDetails( u"Suranna"_s, { u"ofl/suranna/Suranna-Regular.ttf"_s }, u"ofl/suranna/OFL.txt"_s ),
    GoogleFontDetails( u"Suravaram"_s, { u"ofl/suravaram/Suravaram-Regular.ttf"_s }, u"ofl/suravaram/OFL.txt"_s ),
    GoogleFontDetails( u"Suwannaphum"_s, { u"ofl/suwannaphum/Suwannaphum-Thin.ttf"_s, u"ofl/suwannaphum/Suwannaphum-Light.ttf"_s, u"ofl/suwannaphum/Suwannaphum-Regular.ttf"_s, u"ofl/suwannaphum/Suwannaphum-Bold.ttf"_s, u"ofl/suwannaphum/Suwannaphum-Black.ttf"_s }, u"ofl/suwannaphum/OFL.txt"_s ),
    GoogleFontDetails( u"Swanky and Moo Moo"_s, { u"ofl/swankyandmoomoo/SwankyandMooMoo.ttf"_s }, u"ofl/swankyandmoomoo/OFL.txt"_s ),
    GoogleFontDetails( u"Syncopate"_s, { u"apache/syncopate/Syncopate-Regular.ttf"_s, u"apache/syncopate/Syncopate-Bold.ttf"_s }, u"apache/syncopate/LICENSE.txt"_s ),
    GoogleFontDetails( u"Syne"_s, { u"ofl/syne/Syne%5Bwght%5D.ttf"_s }, u"ofl/syne/OFL.txt"_s ),
    GoogleFontDetails( u"Syne Mono"_s, { u"ofl/synemono/SyneMono-Regular.ttf"_s }, u"ofl/synemono/OFL.txt"_s ),
    GoogleFontDetails( u"Syne Tactile"_s, { u"ofl/synetactile/SyneTactile-Regular.ttf"_s }, u"ofl/synetactile/OFL.txt"_s ),
    GoogleFontDetails( u"Tai Heritage Pro"_s, { u"ofl/taiheritagepro/TaiHeritagePro-Regular.ttf"_s, u"ofl/taiheritagepro/TaiHeritagePro-Bold.ttf"_s }, u"ofl/taiheritagepro/OFL.txt"_s ),
    GoogleFontDetails( u"Tajawal"_s, { u"ofl/tajawal/Tajawal-ExtraLight.ttf"_s, u"ofl/tajawal/Tajawal-Light.ttf"_s, u"ofl/tajawal/Tajawal-Regular.ttf"_s, u"ofl/tajawal/Tajawal-Medium.ttf"_s, u"ofl/tajawal/Tajawal-Bold.ttf"_s, u"ofl/tajawal/Tajawal-ExtraBold.ttf"_s, u"ofl/tajawal/Tajawal-Black.ttf"_s }, u"ofl/tajawal/OFL.txt"_s ),
    GoogleFontDetails( u"Tangerine"_s, { u"ofl/tangerine/Tangerine-Regular.ttf"_s, u"ofl/tangerine/Tangerine-Bold.ttf"_s }, u"ofl/tangerine/OFL.txt"_s ),
    GoogleFontDetails( u"Tapestry"_s, { u"ofl/tapestry/Tapestry-Regular.ttf"_s }, u"ofl/tapestry/OFL.txt"_s ),
    GoogleFontDetails( u"Taprom"_s, { u"ofl/taprom/Taprom-Regular.ttf"_s }, u"ofl/taprom/OFL.txt"_s ),
    GoogleFontDetails( u"Tauri"_s, { u"ofl/tauri/Tauri-Regular.ttf"_s }, u"ofl/tauri/OFL.txt"_s ),
    GoogleFontDetails( u"Taviraj"_s, { u"ofl/taviraj/Taviraj-Thin.ttf"_s, u"ofl/taviraj/Taviraj-ThinItalic.ttf"_s, u"ofl/taviraj/Taviraj-ExtraLight.ttf"_s, u"ofl/taviraj/Taviraj-ExtraLightItalic.ttf"_s, u"ofl/taviraj/Taviraj-Light.ttf"_s, u"ofl/taviraj/Taviraj-LightItalic.ttf"_s, u"ofl/taviraj/Taviraj-Regular.ttf"_s, u"ofl/taviraj/Taviraj-Italic.ttf"_s, u"ofl/taviraj/Taviraj-Medium.ttf"_s, u"ofl/taviraj/Taviraj-MediumItalic.ttf"_s, u"ofl/taviraj/Taviraj-SemiBold.ttf"_s, u"ofl/taviraj/Taviraj-SemiBoldItalic.ttf"_s, u"ofl/taviraj/Taviraj-Bold.ttf"_s, u"ofl/taviraj/Taviraj-BoldItalic.ttf"_s, u"ofl/taviraj/Taviraj-ExtraBold.ttf"_s, u"ofl/taviraj/Taviraj-ExtraBoldItalic.ttf"_s, u"ofl/taviraj/Taviraj-Black.ttf"_s, u"ofl/taviraj/Taviraj-BlackItalic.ttf"_s }, u"ofl/taviraj/OFL.txt"_s ),
    GoogleFontDetails( u"Teko"_s, { u"ofl/teko/Teko%5Bwght%5D.ttf"_s }, u"ofl/teko/OFL.txt"_s ),
    GoogleFontDetails( u"Tektur"_s, { u"ofl/tektur/Tektur%5Bwdth,wght%5D.ttf"_s }, u"ofl/tektur/OFL.txt"_s ),
    GoogleFontDetails( u"Telex"_s, { u"ofl/telex/Telex-Regular.ttf"_s }, u"ofl/telex/OFL.txt"_s ),
    GoogleFontDetails( u"Tenali Ramakrishna"_s, { u"ofl/tenaliramakrishna/TenaliRamakrishna-Regular.ttf"_s }, u"ofl/tenaliramakrishna/OFL.txt"_s ),
    GoogleFontDetails( u"Tenor Sans"_s, { u"ofl/tenorsans/TenorSans-Regular.ttf"_s }, u"ofl/tenorsans/OFL.txt"_s ),
    GoogleFontDetails( u"Text Me One"_s, { u"ofl/textmeone/TextMeOne-Regular.ttf"_s }, u"ofl/textmeone/OFL.txt"_s ),
    GoogleFontDetails( u"Texturina"_s, { u"ofl/texturina/Texturina%5Bopsz,wght%5D.ttf"_s, u"ofl/texturina/Texturina-Italic%5Bopsz,wght%5D.ttf"_s }, u"ofl/texturina/OFL.txt"_s ),
    GoogleFontDetails( u"Thasadith"_s, { u"ofl/thasadith/Thasadith-Regular.ttf"_s, u"ofl/thasadith/Thasadith-Italic.ttf"_s, u"ofl/thasadith/Thasadith-Bold.ttf"_s, u"ofl/thasadith/Thasadith-BoldItalic.ttf"_s }, u"ofl/thasadith/OFL.txt"_s ),
    GoogleFontDetails( u"The Girl Next Door"_s, { u"ofl/thegirlnextdoor/TheGirlNextDoor.ttf"_s }, u"ofl/thegirlnextdoor/OFL.txt"_s ),
    GoogleFontDetails( u"The Nautigal"_s, { u"ofl/thenautigal/TheNautigal-Regular.ttf"_s, u"ofl/thenautigal/TheNautigal-Bold.ttf"_s }, u"ofl/thenautigal/OFL.txt"_s ),
    GoogleFontDetails( u"Tienne"_s, { u"ofl/tienne/Tienne-Regular.ttf"_s, u"ofl/tienne/Tienne-Bold.ttf"_s, u"ofl/tienne/Tienne-Black.ttf"_s }, u"ofl/tienne/OFL.txt"_s ),
    GoogleFontDetails( u"Tillana"_s, { u"ofl/tillana/Tillana-Regular.ttf"_s, u"ofl/tillana/Tillana-Medium.ttf"_s, u"ofl/tillana/Tillana-SemiBold.ttf"_s, u"ofl/tillana/Tillana-Bold.ttf"_s, u"ofl/tillana/Tillana-ExtraBold.ttf"_s }, u"ofl/tillana/OFL.txt"_s ),
    GoogleFontDetails( u"Tilt Neon"_s, { u"ofl/tiltneon/TiltNeon%5BXROT,YROT%5D.ttf"_s }, u"ofl/tiltneon/OFL.txt"_s ),
    GoogleFontDetails( u"Tilt Prism"_s, { u"ofl/tiltprism/TiltPrism%5BXROT,YROT%5D.ttf"_s }, u"ofl/tiltprism/OFL.txt"_s ),
    GoogleFontDetails( u"Tilt Warp"_s, { u"ofl/tiltwarp/TiltWarp%5BXROT,YROT%5D.ttf"_s }, u"ofl/tiltwarp/OFL.txt"_s ),
    GoogleFontDetails( u"Timmana"_s, { u"ofl/timmana/Timmana-Regular.ttf"_s }, u"ofl/timmana/OFL.txt"_s ),
    GoogleFontDetails( u"Tinos"_s, { u"apache/tinos/Tinos-Regular.ttf"_s, u"apache/tinos/Tinos-Italic.ttf"_s, u"apache/tinos/Tinos-Bold.ttf"_s, u"apache/tinos/Tinos-BoldItalic.ttf"_s }, u"apache/tinos/LICENSE.txt"_s ),
    GoogleFontDetails( u"Tiro Bangla"_s, { u"ofl/tirobangla/TiroBangla-Regular.ttf"_s, u"ofl/tirobangla/TiroBangla-Italic.ttf"_s }, u"ofl/tirobangla/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Devanagari Hindi"_s, { u"ofl/tirodevanagarihindi/TiroDevanagariHindi-Regular.ttf"_s, u"ofl/tirodevanagarihindi/TiroDevanagariHindi-Italic.ttf"_s }, u"ofl/tirodevanagarihindi/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Devanagari Marathi"_s, { u"ofl/tirodevanagarimarathi/TiroDevanagariMarathi-Regular.ttf"_s, u"ofl/tirodevanagarimarathi/TiroDevanagariMarathi-Italic.ttf"_s }, u"ofl/tirodevanagarimarathi/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Devanagari Sanskrit"_s, { u"ofl/tirodevanagarisanskrit/TiroDevanagariSanskrit-Regular.ttf"_s, u"ofl/tirodevanagarisanskrit/TiroDevanagariSanskrit-Italic.ttf"_s }, u"ofl/tirodevanagarisanskrit/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Gurmukhi"_s, { u"ofl/tirogurmukhi/TiroGurmukhi-Regular.ttf"_s, u"ofl/tirogurmukhi/TiroGurmukhi-Italic.ttf"_s }, u"ofl/tirogurmukhi/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Kannada"_s, { u"ofl/tirokannada/TiroKannada-Regular.ttf"_s, u"ofl/tirokannada/TiroKannada-Italic.ttf"_s }, u"ofl/tirokannada/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Tamil"_s, { u"ofl/tirotamil/TiroTamil-Regular.ttf"_s, u"ofl/tirotamil/TiroTamil-Italic.ttf"_s }, u"ofl/tirotamil/OFL.txt"_s ),
    GoogleFontDetails( u"Tiro Telugu"_s, { u"ofl/tirotelugu/TiroTelugu-Regular.ttf"_s, u"ofl/tirotelugu/TiroTelugu-Italic.ttf"_s }, u"ofl/tirotelugu/OFL.txt"_s ),
    GoogleFontDetails( u"Titan One"_s, { u"ofl/titanone/TitanOne-Regular.ttf"_s }, u"ofl/titanone/OFL.txt"_s ),
    GoogleFontDetails( u"Titillium Web"_s, { u"ofl/titilliumweb/TitilliumWeb-ExtraLight.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-ExtraLightItalic.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-Light.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-LightItalic.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-Regular.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-Italic.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-SemiBold.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-SemiBoldItalic.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-Bold.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-BoldItalic.ttf"_s, u"ofl/titilliumweb/TitilliumWeb-Black.ttf"_s }, u"ofl/titilliumweb/OFL.txt"_s ),
    GoogleFontDetails( u"Tomorrow"_s, { u"ofl/tomorrow/Tomorrow-Thin.ttf"_s, u"ofl/tomorrow/Tomorrow-ThinItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-ExtraLight.ttf"_s, u"ofl/tomorrow/Tomorrow-ExtraLightItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-Light.ttf"_s, u"ofl/tomorrow/Tomorrow-LightItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-Regular.ttf"_s, u"ofl/tomorrow/Tomorrow-Italic.ttf"_s, u"ofl/tomorrow/Tomorrow-Medium.ttf"_s, u"ofl/tomorrow/Tomorrow-MediumItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-SemiBold.ttf"_s, u"ofl/tomorrow/Tomorrow-SemiBoldItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-Bold.ttf"_s, u"ofl/tomorrow/Tomorrow-BoldItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-ExtraBold.ttf"_s, u"ofl/tomorrow/Tomorrow-ExtraBoldItalic.ttf"_s, u"ofl/tomorrow/Tomorrow-Black.ttf"_s, u"ofl/tomorrow/Tomorrow-BlackItalic.ttf"_s }, u"ofl/tomorrow/OFL.txt"_s ),
    GoogleFontDetails( u"Tourney"_s, { u"ofl/tourney/Tourney%5Bwdth,wght%5D.ttf"_s, u"ofl/tourney/Tourney-Italic%5Bwdth,wght%5D.ttf"_s }, u"ofl/tourney/OFL.txt"_s ),
    GoogleFontDetails( u"Trade Winds"_s, { u"ofl/tradewinds/TradeWinds-Regular.ttf"_s }, u"ofl/tradewinds/OFL.txt"_s ),
    GoogleFontDetails( u"Train One"_s, { u"ofl/trainone/TrainOne-Regular.ttf"_s }, u"ofl/trainone/OFL.txt"_s ),
    GoogleFontDetails( u"Trirong"_s, { u"ofl/trirong/Trirong-Thin.ttf"_s, u"ofl/trirong/Trirong-ThinItalic.ttf"_s, u"ofl/trirong/Trirong-ExtraLight.ttf"_s, u"ofl/trirong/Trirong-ExtraLightItalic.ttf"_s, u"ofl/trirong/Trirong-Light.ttf"_s, u"ofl/trirong/Trirong-LightItalic.ttf"_s, u"ofl/trirong/Trirong-Regular.ttf"_s, u"ofl/trirong/Trirong-Italic.ttf"_s, u"ofl/trirong/Trirong-Medium.ttf"_s, u"ofl/trirong/Trirong-MediumItalic.ttf"_s, u"ofl/trirong/Trirong-SemiBold.ttf"_s, u"ofl/trirong/Trirong-SemiBoldItalic.ttf"_s, u"ofl/trirong/Trirong-Bold.ttf"_s, u"ofl/trirong/Trirong-BoldItalic.ttf"_s, u"ofl/trirong/Trirong-ExtraBold.ttf"_s, u"ofl/trirong/Trirong-ExtraBoldItalic.ttf"_s, u"ofl/trirong/Trirong-Black.ttf"_s, u"ofl/trirong/Trirong-BlackItalic.ttf"_s }, u"ofl/trirong/OFL.txt"_s ),
    GoogleFontDetails( u"Trispace"_s, { u"ofl/trispace/Trispace%5Bwdth,wght%5D.ttf"_s }, u"ofl/trispace/OFL.txt"_s ),
    GoogleFontDetails( u"Trocchi"_s, { u"ofl/trocchi/Trocchi-Regular.ttf"_s }, u"ofl/trocchi/OFL.txt"_s ),
    GoogleFontDetails( u"Trochut"_s, { u"ofl/trochut/Trochut-Regular.ttf"_s, u"ofl/trochut/Trochut-Italic.ttf"_s, u"ofl/trochut/Trochut-Bold.ttf"_s }, u"ofl/trochut/OFL.txt"_s ),
    GoogleFontDetails( u"Truculenta"_s, { u"ofl/truculenta/Truculenta%5Bopsz,wdth,wght%5D.ttf"_s }, u"ofl/truculenta/OFL.txt"_s ),
    GoogleFontDetails( u"Trykker"_s, { u"ofl/trykker/Trykker-Regular.ttf"_s }, u"ofl/trykker/OFL.txt"_s ),
    GoogleFontDetails( u"Tsukimi Rounded"_s, { u"ofl/tsukimirounded/TsukimiRounded-Light.ttf"_s, u"ofl/tsukimirounded/TsukimiRounded-Regular.ttf"_s, u"ofl/tsukimirounded/TsukimiRounded-Medium.ttf"_s, u"ofl/tsukimirounded/TsukimiRounded-SemiBold.ttf"_s, u"ofl/tsukimirounded/TsukimiRounded-Bold.ttf"_s }, u"ofl/tsukimirounded/OFL.txt"_s ),
    GoogleFontDetails( u"Tulpen One"_s, { u"ofl/tulpenone/TulpenOne-Regular.ttf"_s }, u"ofl/tulpenone/OFL.txt"_s ),
    GoogleFontDetails( u"Turret Road"_s, { u"ofl/turretroad/TurretRoad-ExtraLight.ttf"_s, u"ofl/turretroad/TurretRoad-Light.ttf"_s, u"ofl/turretroad/TurretRoad-Regular.ttf"_s, u"ofl/turretroad/TurretRoad-Medium.ttf"_s, u"ofl/turretroad/TurretRoad-Bold.ttf"_s, u"ofl/turretroad/TurretRoad-ExtraBold.ttf"_s }, u"ofl/turretroad/OFL.txt"_s ),
    GoogleFontDetails( u"Twinkle Star"_s, { u"ofl/twinklestar/TwinkleStar-Regular.ttf"_s }, u"ofl/twinklestar/OFL.txt"_s ),
    GoogleFontDetails( u"Ubuntu"_s, { u"ufl/ubuntu/Ubuntu-Light.ttf"_s, u"ufl/ubuntu/Ubuntu-LightItalic.ttf"_s, u"ufl/ubuntu/Ubuntu-Regular.ttf"_s, u"ufl/ubuntu/Ubuntu-Italic.ttf"_s, u"ufl/ubuntu/Ubuntu-Medium.ttf"_s, u"ufl/ubuntu/Ubuntu-MediumItalic.ttf"_s, u"ufl/ubuntu/Ubuntu-Bold.ttf"_s, u"ufl/ubuntu/Ubuntu-BoldItalic.ttf"_s }, u"ufl/ubuntu/UFL.txt"_s ),
    GoogleFontDetails( u"Ubuntu Condensed"_s, { u"ufl/ubuntucondensed/UbuntuCondensed-Regular.ttf"_s }, u"ufl/ubuntucondensed/UFL.txt"_s ),
    GoogleFontDetails( u"Ubuntu Mono"_s, { u"ufl/ubuntumono/UbuntuMono-Regular.ttf"_s, u"ufl/ubuntumono/UbuntuMono-Italic.ttf"_s, u"ufl/ubuntumono/UbuntuMono-Bold.ttf"_s, u"ufl/ubuntumono/UbuntuMono-BoldItalic.ttf"_s }, u"ufl/ubuntumono/UFL.txt"_s ),
    GoogleFontDetails( u"Uchen"_s, { u"ofl/uchen/Uchen-Regular.ttf"_s }, u"ofl/uchen/OFL.txt"_s ),
    GoogleFontDetails( u"Ultra"_s, { u"apache/ultra/Ultra-Regular.ttf"_s }, u"apache/ultra/LICENSE.txt"_s ),
    GoogleFontDetails( u"Unbounded"_s, { u"ofl/unbounded/Unbounded%5Bwght%5D.ttf"_s }, u"ofl/unbounded/OFL.txt"_s ),
    GoogleFontDetails( u"Uncial Antiqua"_s, { u"ofl/uncialantiqua/UncialAntiqua-Regular.ttf"_s }, u"ofl/uncialantiqua/OFL.txt"_s ),
    GoogleFontDetails( u"Underdog"_s, { u"ofl/underdog/Underdog-Regular.ttf"_s }, u"ofl/underdog/OFL.txt"_s ),
    GoogleFontDetails( u"Unica One"_s, { u"ofl/unicaone/UnicaOne-Regular.ttf"_s }, u"ofl/unicaone/OFL.txt"_s ),
    GoogleFontDetails( u"UnifrakturCook"_s, { u"ofl/unifrakturcook/UnifrakturCook-Bold.ttf"_s }, u"ofl/unifrakturcook/OFL.txt"_s ),
    GoogleFontDetails( u"UnifrakturMaguntia"_s, { u"ofl/unifrakturmaguntia/UnifrakturMaguntia-Book.ttf"_s }, u"ofl/unifrakturmaguntia/OFL.txt"_s ),
    GoogleFontDetails( u"Unkempt"_s, { u"apache/unkempt/Unkempt-Regular.ttf"_s, u"apache/unkempt/Unkempt-Bold.ttf"_s }, u"apache/unkempt/LICENSE.txt"_s ),
    GoogleFontDetails( u"Unlock"_s, { u"ofl/unlock/Unlock-Regular.ttf"_s }, u"ofl/unlock/OFL.txt"_s ),
    GoogleFontDetails( u"Unna"_s, { u"ofl/unna/Unna-Regular.ttf"_s, u"ofl/unna/Unna-Italic.ttf"_s, u"ofl/unna/Unna-Bold.ttf"_s, u"ofl/unna/Unna-BoldItalic.ttf"_s }, u"ofl/unna/OFL.txt"_s ),
    GoogleFontDetails( u"Updock"_s, { u"ofl/updock/Updock-Regular.ttf"_s }, u"ofl/updock/OFL.txt"_s ),
    GoogleFontDetails( u"Urbanist"_s, { u"ofl/urbanist/Urbanist%5Bwght%5D.ttf"_s, u"ofl/urbanist/Urbanist-Italic%5Bwght%5D.ttf"_s }, u"ofl/urbanist/OFL.txt"_s ),
    GoogleFontDetails( u"VT323"_s, { u"ofl/vt323/VT323-Regular.ttf"_s }, u"ofl/vt323/OFL.txt"_s ),
    GoogleFontDetails( u"Vampiro One"_s, { u"ofl/vampiroone/VampiroOne-Regular.ttf"_s }, u"ofl/vampiroone/OFL.txt"_s ),
    GoogleFontDetails( u"Varela"_s, { u"ofl/varela/Varela-Regular.ttf"_s }, u"ofl/varela/OFL.txt"_s ),
    GoogleFontDetails( u"Varela Round"_s, { u"ofl/varelaround/VarelaRound-Regular.ttf"_s }, u"ofl/varelaround/OFL.txt"_s ),
    GoogleFontDetails( u"Varta"_s, { u"ofl/varta/Varta%5Bwght%5D.ttf"_s }, u"ofl/varta/OFL.txt"_s ),
    GoogleFontDetails( u"Vast Shadow"_s, { u"ofl/vastshadow/VastShadow-Regular.ttf"_s }, u"ofl/vastshadow/OFL.txt"_s ),
    GoogleFontDetails( u"Vazirmatn"_s, { u"ofl/vazirmatn/Vazirmatn%5Bwght%5D.ttf"_s }, u"ofl/vazirmatn/OFL.txt"_s ),
    GoogleFontDetails( u"Vesper Libre"_s, { u"ofl/vesperlibre/VesperLibre-Regular.ttf"_s, u"ofl/vesperlibre/VesperLibre-Medium.ttf"_s, u"ofl/vesperlibre/VesperLibre-Bold.ttf"_s, u"ofl/vesperlibre/VesperLibre-Heavy.ttf"_s }, u"ofl/vesperlibre/OFL.txt"_s ),
    GoogleFontDetails( u"Viaoda Libre"_s, { u"ofl/viaodalibre/ViaodaLibre-Regular.ttf"_s }, u"ofl/viaodalibre/OFL.txt"_s ),
    GoogleFontDetails( u"Vibes"_s, { u"ofl/vibes/Vibes-Regular.ttf"_s }, u"ofl/vibes/OFL.txt"_s ),
    GoogleFontDetails( u"Vibur"_s, { u"ofl/vibur/Vibur-Regular.ttf"_s }, u"ofl/vibur/OFL.txt"_s ),
    GoogleFontDetails( u"Victor Mono"_s, { u"ofl/victormono/VictorMono%5Bwght%5D.ttf"_s, u"ofl/victormono/VictorMono-Italic%5Bwght%5D.ttf"_s }, u"ofl/victormono/OFL.txt"_s ),
    GoogleFontDetails( u"Vidaloka"_s, { u"ofl/vidaloka/Vidaloka-Regular.ttf"_s }, u"ofl/vidaloka/OFL.txt"_s ),
    GoogleFontDetails( u"Viga"_s, { u"ofl/viga/Viga-Regular.ttf"_s }, u"ofl/viga/OFL.txt"_s ),
    GoogleFontDetails( u"Vina Sans"_s, { u"ofl/vinasans/VinaSans-Regular.ttf"_s }, u"ofl/vinasans/OFL.txt"_s ),
    GoogleFontDetails( u"Voces"_s, { u"ofl/voces/Voces-Regular.ttf"_s }, u"ofl/voces/OFL.txt"_s ),
    GoogleFontDetails( u"Volkhov"_s, { u"ofl/volkhov/Volkhov-Regular.ttf"_s, u"ofl/volkhov/Volkhov-Italic.ttf"_s, u"ofl/volkhov/Volkhov-Bold.ttf"_s, u"ofl/volkhov/Volkhov-BoldItalic.ttf"_s }, u"ofl/volkhov/OFL.txt"_s ),
    GoogleFontDetails( u"Vollkorn"_s, { u"ofl/vollkorn/Vollkorn%5Bwght%5D.ttf"_s, u"ofl/vollkorn/Vollkorn-Italic%5Bwght%5D.ttf"_s }, u"ofl/vollkorn/OFL.txt"_s ),
    GoogleFontDetails( u"Vollkorn SC"_s, { u"ofl/vollkornsc/VollkornSC-Regular.ttf"_s, u"ofl/vollkornsc/VollkornSC-SemiBold.ttf"_s, u"ofl/vollkornsc/VollkornSC-Bold.ttf"_s, u"ofl/vollkornsc/VollkornSC-Black.ttf"_s }, u"ofl/vollkornsc/OFL.txt"_s ),
    GoogleFontDetails( u"Voltaire"_s, { u"ofl/voltaire/Voltaire-Regular.ttf"_s }, u"ofl/voltaire/OFL.txt"_s ),
    GoogleFontDetails( u"Vujahday Script"_s, { u"ofl/vujahdayscript/VujahdayScript-Regular.ttf"_s }, u"ofl/vujahdayscript/OFL.txt"_s ),
    GoogleFontDetails( u"Waiting for the Sunrise"_s, { u"ofl/waitingforthesunrise/WaitingfortheSunrise.ttf"_s }, u"ofl/waitingforthesunrise/OFL.txt"_s ),
    GoogleFontDetails( u"Wallpoet"_s, { u"ofl/wallpoet/Wallpoet-Regular.ttf"_s }, u"ofl/wallpoet/OFL.txt"_s ),
    GoogleFontDetails( u"Walter Turncoat"_s, { u"apache/walterturncoat/WalterTurncoat-Regular.ttf"_s }, u"apache/walterturncoat/LICENSE.txt"_s ),
    GoogleFontDetails( u"Warnes"_s, { u"ofl/warnes/Warnes-Regular.ttf"_s }, u"ofl/warnes/OFL.txt"_s ),
    GoogleFontDetails( u"Water Brush"_s, { u"ofl/waterbrush/WaterBrush-Regular.ttf"_s }, u"ofl/waterbrush/OFL.txt"_s ),
    GoogleFontDetails( u"Waterfall"_s, { u"ofl/waterfall/Waterfall-Regular.ttf"_s }, u"ofl/waterfall/OFL.txt"_s ),
    GoogleFontDetails( u"Wavefont"_s, { u"ofl/wavefont/Wavefont%5BROND,YELA,wght%5D.ttf"_s }, u"ofl/wavefont/OFL.txt"_s ),
    GoogleFontDetails( u"Wellfleet"_s, { u"ofl/wellfleet/Wellfleet-Regular.ttf"_s }, u"ofl/wellfleet/OFL.txt"_s ),
    GoogleFontDetails( u"Wendy One"_s, { u"ofl/wendyone/WendyOne-Regular.ttf"_s }, u"ofl/wendyone/OFL.txt"_s ),
    GoogleFontDetails( u"Whisper"_s, { u"ofl/whisper/Whisper-Regular.ttf"_s }, u"ofl/whisper/OFL.txt"_s ),
    GoogleFontDetails( u"WindSong"_s, { u"ofl/windsong/WindSong-Regular.ttf"_s, u"ofl/windsong/WindSong-Medium.ttf"_s }, u"ofl/windsong/OFL.txt"_s ),
    GoogleFontDetails( u"Wire One"_s, { u"ofl/wireone/WireOne-Regular.ttf"_s }, u"ofl/wireone/OFL.txt"_s ),
    GoogleFontDetails( u"Wix Madefor Display"_s, { u"ofl/wixmadefordisplay/WixMadeforDisplay%5Bwght%5D.ttf"_s }, u"ofl/wixmadefordisplay/OFL.txt"_s ),
    GoogleFontDetails( u"Wix Madefor Text"_s, { u"ofl/wixmadefortext/WixMadeforText%5Bwght%5D.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-Italic%5Bwght%5D.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-Regular.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-Italic.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-Medium.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-MediumItalic.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-SemiBold.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-SemiBoldItalic.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-Bold.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-BoldItalic.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-ExtraBold.ttf"_s, u"ofl/wixmadefortext/WixMadeforText-ExtraBoldItalic.ttf"_s }, u"ofl/wixmadefortext/OFL.txt"_s ),
    GoogleFontDetails( u"Work Sans"_s, { u"ofl/worksans/WorkSans%5Bwght%5D.ttf"_s, u"ofl/worksans/WorkSans-Italic%5Bwght%5D.ttf"_s }, u"ofl/worksans/OFL.txt"_s ),
    GoogleFontDetails( u"Xanh Mono"_s, { u"ofl/xanhmono/XanhMono-Regular.ttf"_s, u"ofl/xanhmono/XanhMono-Italic.ttf"_s }, u"ofl/xanhmono/OFL.txt"_s ),
    GoogleFontDetails( u"Yaldevi"_s, { u"ofl/yaldevi/Yaldevi%5Bwght%5D.ttf"_s }, u"ofl/yaldevi/OFL.txt"_s ),
    GoogleFontDetails( u"Yanone Kaffeesatz"_s, { u"ofl/yanonekaffeesatz/YanoneKaffeesatz%5Bwght%5D.ttf"_s }, u"ofl/yanonekaffeesatz/OFL.txt"_s ),
    GoogleFontDetails( u"Yantramanav"_s, { u"ofl/yantramanav/Yantramanav-Thin.ttf"_s, u"ofl/yantramanav/Yantramanav-Light.ttf"_s, u"ofl/yantramanav/Yantramanav-Regular.ttf"_s, u"ofl/yantramanav/Yantramanav-Medium.ttf"_s, u"ofl/yantramanav/Yantramanav-Bold.ttf"_s, u"ofl/yantramanav/Yantramanav-Black.ttf"_s }, u"ofl/yantramanav/OFL.txt"_s ),
    GoogleFontDetails( u"Yatra One"_s, { u"ofl/yatraone/YatraOne-Regular.ttf"_s }, u"ofl/yatraone/OFL.txt"_s ),
    GoogleFontDetails( u"Yellowtail"_s, { u"apache/yellowtail/Yellowtail-Regular.ttf"_s }, u"apache/yellowtail/LICENSE.txt"_s ),
    GoogleFontDetails( u"Yeon Sung"_s, { u"ofl/yeonsung/YeonSung-Regular.ttf"_s }, u"ofl/yeonsung/OFL.txt"_s ),
    GoogleFontDetails( u"Yeseva One"_s, { u"ofl/yesevaone/YesevaOne-Regular.ttf"_s }, u"ofl/yesevaone/OFL.txt"_s ),
    GoogleFontDetails( u"Yesteryear"_s, { u"ofl/yesteryear/Yesteryear-Regular.ttf"_s }, u"ofl/yesteryear/OFL.txt"_s ),
    GoogleFontDetails( u"Yomogi"_s, { u"ofl/yomogi/Yomogi-Regular.ttf"_s }, u"ofl/yomogi/OFL.txt"_s ),
    GoogleFontDetails( u"Yrsa"_s, { u"ofl/yrsa/Yrsa%5Bwght%5D.ttf"_s, u"ofl/yrsa/Yrsa-Italic%5Bwght%5D.ttf"_s }, u"ofl/yrsa/OFL.txt"_s ),
    GoogleFontDetails( u"Ysabeau"_s, { u"ofl/ysabeau/Ysabeau%5Bwght%5D.ttf"_s, u"ofl/ysabeau/Ysabeau-Italic%5Bwght%5D.ttf"_s }, u"ofl/ysabeau/OFL.txt"_s ),
    GoogleFontDetails( u"Ysabeau Infant"_s, { u"ofl/ysabeauinfant/YsabeauInfant%5Bwght%5D.ttf"_s, u"ofl/ysabeauinfant/YsabeauInfant-Italic%5Bwght%5D.ttf"_s }, u"ofl/ysabeauinfant/OFL.txt"_s ),
    GoogleFontDetails( u"Ysabeau Office"_s, { u"ofl/ysabeauoffice/YsabeauOffice%5Bwght%5D.ttf"_s, u"ofl/ysabeauoffice/YsabeauOffice-Italic%5Bwght%5D.ttf"_s }, u"ofl/ysabeauoffice/OFL.txt"_s ),
    GoogleFontDetails( u"Ysabeau SC"_s, { u"ofl/ysabeausc/YsabeauSC%5Bwght%5D.ttf"_s }, u"ofl/ysabeausc/OFL.txt"_s ),
    GoogleFontDetails( u"Yuji Boku"_s, { u"ofl/yujiboku/YujiBoku-Regular.ttf"_s }, u"ofl/yujiboku/OFL.txt"_s ),
    GoogleFontDetails( u"Yuji Hentaigana Akari"_s, { u"ofl/yujihentaiganaakari/YujiHentaiganaAkari-Regular.ttf"_s }, u"ofl/yujihentaiganaakari/OFL.txt"_s ),
    GoogleFontDetails( u"Yuji Hentaigana Akebono"_s, { u"ofl/yujihentaiganaakebono/YujiHentaiganaAkebono-Regular.ttf"_s }, u"ofl/yujihentaiganaakebono/OFL.txt"_s ),
    GoogleFontDetails( u"Yuji Mai"_s, { u"ofl/yujimai/YujiMai-Regular.ttf"_s }, u"ofl/yujimai/OFL.txt"_s ),
    GoogleFontDetails( u"Yuji Syuku"_s, { u"ofl/yujisyuku/YujiSyuku-Regular.ttf"_s }, u"ofl/yujisyuku/OFL.txt"_s ),
    GoogleFontDetails( u"Yusei Magic"_s, { u"ofl/yuseimagic/YuseiMagic-Regular.ttf"_s }, u"ofl/yuseimagic/OFL.txt"_s ),
    GoogleFontDetails( u"ZCOOL KuaiLe"_s, { u"ofl/zcoolkuaile/ZCOOLKuaiLe-Regular.ttf"_s }, u"ofl/zcoolkuaile/OFL.txt"_s ),
    GoogleFontDetails( u"ZCOOL QingKe HuangYou"_s, { u"ofl/zcoolqingkehuangyou/ZCOOLQingKeHuangYou-Regular.ttf"_s }, u"ofl/zcoolqingkehuangyou/OFL.txt"_s ),
    GoogleFontDetails( u"ZCOOL XiaoWei"_s, { u"ofl/zcoolxiaowei/ZCOOLXiaoWei-Regular.ttf"_s }, u"ofl/zcoolxiaowei/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Antique"_s, { u"ofl/zenantique/ZenAntique-Regular.ttf"_s }, u"ofl/zenantique/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Antique Soft"_s, { u"ofl/zenantiquesoft/ZenAntiqueSoft-Regular.ttf"_s }, u"ofl/zenantiquesoft/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Dots"_s, { u"ofl/zendots/ZenDots-Regular.ttf"_s }, u"ofl/zendots/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Kaku Gothic Antique"_s, { u"ofl/zenkakugothicantique/ZenKakuGothicAntique-Light.ttf"_s, u"ofl/zenkakugothicantique/ZenKakuGothicAntique-Regular.ttf"_s, u"ofl/zenkakugothicantique/ZenKakuGothicAntique-Medium.ttf"_s, u"ofl/zenkakugothicantique/ZenKakuGothicAntique-Bold.ttf"_s, u"ofl/zenkakugothicantique/ZenKakuGothicAntique-Black.ttf"_s }, u"ofl/zenkakugothicantique/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Kaku Gothic New"_s, { u"ofl/zenkakugothicnew/ZenKakuGothicNew-Light.ttf"_s, u"ofl/zenkakugothicnew/ZenKakuGothicNew-Regular.ttf"_s, u"ofl/zenkakugothicnew/ZenKakuGothicNew-Medium.ttf"_s, u"ofl/zenkakugothicnew/ZenKakuGothicNew-Bold.ttf"_s, u"ofl/zenkakugothicnew/ZenKakuGothicNew-Black.ttf"_s }, u"ofl/zenkakugothicnew/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Kurenaido"_s, { u"ofl/zenkurenaido/ZenKurenaido-Regular.ttf"_s }, u"ofl/zenkurenaido/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Loop"_s, { u"ofl/zenloop/ZenLoop-Regular.ttf"_s, u"ofl/zenloop/ZenLoop-Italic.ttf"_s }, u"ofl/zenloop/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Maru Gothic"_s, { u"ofl/zenmarugothic/ZenMaruGothic-Light.ttf"_s, u"ofl/zenmarugothic/ZenMaruGothic-Regular.ttf"_s, u"ofl/zenmarugothic/ZenMaruGothic-Medium.ttf"_s, u"ofl/zenmarugothic/ZenMaruGothic-Bold.ttf"_s, u"ofl/zenmarugothic/ZenMaruGothic-Black.ttf"_s }, u"ofl/zenmarugothic/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Old Mincho"_s, { u"ofl/zenoldmincho/ZenOldMincho-Regular.ttf"_s, u"ofl/zenoldmincho/ZenOldMincho-Medium.ttf"_s, u"ofl/zenoldmincho/ZenOldMincho-SemiBold.ttf"_s, u"ofl/zenoldmincho/ZenOldMincho-Bold.ttf"_s, u"ofl/zenoldmincho/ZenOldMincho-Black.ttf"_s }, u"ofl/zenoldmincho/OFL.txt"_s ),
    GoogleFontDetails( u"Zen Tokyo Zoo"_s, { u"ofl/zentokyozoo/ZenTokyoZoo-Regular.ttf"_s }, u"ofl/zentokyozoo/OFL.txt"_s ),
    GoogleFontDetails( u"Zeyada"_s, { u"ofl/zeyada/Zeyada.ttf"_s }, u"ofl/zeyada/OFL.txt"_s ),
    GoogleFontDetails( u"Zhi Mang Xing"_s, { u"ofl/zhimangxing/ZhiMangXing-Regular.ttf"_s }, u"ofl/zhimangxing/OFL.txt"_s ),
    GoogleFontDetails( u"Zilla Slab"_s, { u"ofl/zillaslab/ZillaSlab-Light.ttf"_s, u"ofl/zillaslab/ZillaSlab-LightItalic.ttf"_s, u"ofl/zillaslab/ZillaSlab-Regular.ttf"_s, u"ofl/zillaslab/ZillaSlab-Italic.ttf"_s, u"ofl/zillaslab/ZillaSlab-Medium.ttf"_s, u"ofl/zillaslab/ZillaSlab-MediumItalic.ttf"_s, u"ofl/zillaslab/ZillaSlab-SemiBold.ttf"_s, u"ofl/zillaslab/ZillaSlab-SemiBoldItalic.ttf"_s, u"ofl/zillaslab/ZillaSlab-Bold.ttf"_s, u"ofl/zillaslab/ZillaSlab-BoldItalic.ttf"_s }, u"ofl/zillaslab/OFL.txt"_s ),
    GoogleFontDetails( u"Zilla Slab Highlight"_s, { u"ofl/zillaslabhighlight/ZillaSlabHighlight-Regular.ttf"_s, u"ofl/zillaslabhighlight/ZillaSlabHighlight-Bold.ttf"_s }, u"ofl/zillaslabhighlight/OFL.txt"_s ),
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
        const thread_local QRegularExpression rxExtension( u"^.*\\.(\\w+?)$"_s );
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
    tempFile.setFileTemplate( u"%1/XXXXXX.%2"_s.arg( QDir::tempPath(), cleanedExtension ) );
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

    QgsDebugMsgLevel( u"Found fonts %1"_s.arg( foundFamilies.join( ',' ) ), 2 );
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
        if ( fi.fileName().compare( "OFL.txt"_L1, Qt::CaseInsensitive ) == 0
             || fi.fileName().compare( "LICENSE.txt"_L1, Qt::CaseInsensitive ) == 0 )
        {
          QFile licenseFile( file );
          if ( licenseFile.open( QIODevice::ReadOnly ) )
          {
            QTextStream in( &licenseFile );
            const QString license = in.readAll();
            licenseDetails.append( license );
          }
        }
        else if ( fi.suffix().compare( "ttf"_L1, Qt::CaseInsensitive ) == 0 ||
                  fi.suffix().compare( "otf"_L1, Qt::CaseInsensitive ) == 0 )
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
    QgsDebugError( u"Cannot create local fonts dir: %1"_s.arg( directory ) );
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
    QgsSetRequestInitiatorClass( networkRequest, u"QgsFontDownloadTask"_s );
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
    QgsSetRequestInitiatorClass( networkRequest, u"QgsFontDownloadTask"_s );
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
