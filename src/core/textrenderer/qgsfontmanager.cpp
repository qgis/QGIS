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
#include "qgsreadwritelocker.h"
#include "qgsapplication.h"
#include "qgsnetworkcontentfetchertask.h"
#include "qgsziputils.h"
#include "qgssettingsentryimpl.h"
#include "qgssettingstree.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QFontDatabase>
#include <QTemporaryFile>
#include <QTemporaryDir>

const QgsSettingsEntryStringList *QgsFontManager::settingsFontFamilyReplacements = new QgsSettingsEntryStringList( QStringLiteral( "fontFamilyReplacements" ), QgsSettingsTree::sTreeFonts, QStringList(), QStringLiteral( "Automatic font family replacements" ) );

const QgsSettingsEntryBool *QgsFontManager::settingsDownloadMissingFonts = new QgsSettingsEntryBool( QStringLiteral( "downloadMissingFonts" ), QgsSettingsTree::sTreeFonts, true, QStringLiteral( "Automatically download missing fonts whenever possible" ) );

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

  const QString url = urlForFontDownload( family, matchedFamily );
  if ( url.isEmpty() )
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
    mDeferredFontDownloads.insert( url, family );
  }
  else
  {
    locker.unlock();
    downloadAndInstallFont( QUrl( url ), family );
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
      downloadAndInstallFont( QUrl( it.key() ), it.value() );
    }
    mDeferredFontDownloads.clear();
  }
}

QString QgsFontManager::urlForFontDownload( const QString &family, QString &matchedFamily ) const
{
  const thread_local QStringList sGoogleFonts
  {
    QStringLiteral( "ABeeZee" ),
    QStringLiteral( "ADLaM Display" ),
    QStringLiteral( "Abel" ),
    QStringLiteral( "Abhaya Libre" ),
    QStringLiteral( "Aboreto" ),
    QStringLiteral( "Abril Fatface" ),
    QStringLiteral( "Abyssinica SIL" ),
    QStringLiteral( "Aclonica" ),
    QStringLiteral( "Acme" ),
    QStringLiteral( "Actor" ),
    QStringLiteral( "Adamina" ),
    QStringLiteral( "Advent Pro" ),
    QStringLiteral( "Agdasima" ),
    QStringLiteral( "Aguafina Script" ),
    QStringLiteral( "Akatab" ),
    QStringLiteral( "Akaya Kanadaka" ),
    QStringLiteral( "Akaya Telivigala" ),
    QStringLiteral( "Akronim" ),
    QStringLiteral( "Akshar" ),
    QStringLiteral( "Aladin" ),
    QStringLiteral( "Alata" ),
    QStringLiteral( "Alatsi" ),
    QStringLiteral( "Albert Sans" ),
    QStringLiteral( "Aldrich" ),
    QStringLiteral( "Alef" ),
    QStringLiteral( "Alegreya" ),
    QStringLiteral( "Alegreya SC" ),
    QStringLiteral( "Alegreya Sans" ),
    QStringLiteral( "Alegreya Sans SC" ),
    QStringLiteral( "Aleo" ),
    QStringLiteral( "Alex Brush" ),
    QStringLiteral( "Alexandria" ),
    QStringLiteral( "Alfa Slab One" ),
    QStringLiteral( "Alice" ),
    QStringLiteral( "Alike" ),
    QStringLiteral( "Alike Angular" ),
    QStringLiteral( "Alkalami" ),
    QStringLiteral( "Alkatra" ),
    QStringLiteral( "Allan" ),
    QStringLiteral( "Allerta" ),
    QStringLiteral( "Allerta Stencil" ),
    QStringLiteral( "Allison" ),
    QStringLiteral( "Allura" ),
    QStringLiteral( "Almarai" ),
    QStringLiteral( "Almendra" ),
    QStringLiteral( "Almendra Display" ),
    QStringLiteral( "Almendra SC" ),
    QStringLiteral( "Alumni Sans" ),
    QStringLiteral( "Alumni Sans Collegiate One" ),
    QStringLiteral( "Alumni Sans Inline One" ),
    QStringLiteral( "Alumni Sans Pinstripe" ),
    QStringLiteral( "Amarante" ),
    QStringLiteral( "Amaranth" ),
    QStringLiteral( "Amatic SC" ),
    QStringLiteral( "Amethysta" ),
    QStringLiteral( "Amiko" ),
    QStringLiteral( "Amiri" ),
    QStringLiteral( "Amiri Quran" ),
    QStringLiteral( "Amita" ),
    QStringLiteral( "Anaheim" ),
    QStringLiteral( "Andada" ),
    QStringLiteral( "Andada Pro" ),
    QStringLiteral( "Andika" ),
    QStringLiteral( "Anek Bangla" ),
    QStringLiteral( "Anek Devanagari" ),
    QStringLiteral( "Anek Gujarati" ),
    QStringLiteral( "Anek Gurmukhi" ),
    QStringLiteral( "Anek Kannada" ),
    QStringLiteral( "Anek Latin" ),
    QStringLiteral( "Anek Malayalam" ),
    QStringLiteral( "Anek Odia" ),
    QStringLiteral( "Anek Tamil" ),
    QStringLiteral( "Anek Telugu" ),
    QStringLiteral( "Angkor" ),
    QStringLiteral( "Annie Use Your Telescope" ),
    QStringLiteral( "Anonymous Pro" ),
    QStringLiteral( "Antic" ),
    QStringLiteral( "Antic Didone" ),
    QStringLiteral( "Antic Slab" ),
    QStringLiteral( "Anton" ),
    QStringLiteral( "Antonio" ),
    QStringLiteral( "Anuphan" ),
    QStringLiteral( "Anybody" ),
    QStringLiteral( "Aoboshi One" ),
    QStringLiteral( "Arapey" ),
    QStringLiteral( "Arbutus" ),
    QStringLiteral( "Arbutus Slab" ),
    QStringLiteral( "Architects Daughter" ),
    QStringLiteral( "Archivo" ),
    QStringLiteral( "Archivo Black" ),
    QStringLiteral( "Archivo Narrow" ),
    QStringLiteral( "Are You Serious" ),
    QStringLiteral( "Aref Ruqaa" ),
    QStringLiteral( "Aref Ruqaa Ink" ),
    QStringLiteral( "Arima" ),
    QStringLiteral( "Arimo" ),
    QStringLiteral( "Arizonia" ),
    QStringLiteral( "Armata" ),
    QStringLiteral( "Arsenal" ),
    QStringLiteral( "Artifika" ),
    QStringLiteral( "Arvo" ),
    QStringLiteral( "Arya" ),
    QStringLiteral( "Asap" ),
    QStringLiteral( "Asap Condensed" ),
    QStringLiteral( "Asar" ),
    QStringLiteral( "Asset" ),
    QStringLiteral( "Assistant" ),
    QStringLiteral( "Astloch" ),
    QStringLiteral( "Asul" ),
    QStringLiteral( "Athiti" ),
    QStringLiteral( "Atkinson Hyperlegible" ),
    QStringLiteral( "Atma" ),
    QStringLiteral( "Atomic Age" ),
    QStringLiteral( "Aubrey" ),
    QStringLiteral( "Audiowide" ),
    QStringLiteral( "Autour One" ),
    QStringLiteral( "Average" ),
    QStringLiteral( "Average Sans" ),
    QStringLiteral( "Averia Gruesa Libre" ),
    QStringLiteral( "Averia Libre" ),
    QStringLiteral( "Averia Sans Libre" ),
    QStringLiteral( "Averia Serif Libre" ),
    QStringLiteral( "Azeret Mono" ),
    QStringLiteral( "B612" ),
    QStringLiteral( "B612 Mono" ),
    QStringLiteral( "BIZ UDGothic" ),
    QStringLiteral( "BIZ UDMincho" ),
    QStringLiteral( "BIZ UDPGothic" ),
    QStringLiteral( "BIZ UDPMincho" ),
    QStringLiteral( "Babylonica" ),
    QStringLiteral( "Bacasime Antique" ),
    QStringLiteral( "Bad Script" ),
    QStringLiteral( "Bagel Fat One" ),
    QStringLiteral( "Bahiana" ),
    QStringLiteral( "Bahianita" ),
    QStringLiteral( "Bai Jamjuree" ),
    QStringLiteral( "Bakbak One" ),
    QStringLiteral( "Ballet" ),
    QStringLiteral( "Baloo 2" ),
    QStringLiteral( "Baloo Bhai 2" ),
    QStringLiteral( "Baloo Bhaijaan 2" ),
    QStringLiteral( "Baloo Bhaina 2" ),
    QStringLiteral( "Baloo Chettan 2" ),
    QStringLiteral( "Baloo Da 2" ),
    QStringLiteral( "Baloo Paaji 2" ),
    QStringLiteral( "Baloo Tamma 2" ),
    QStringLiteral( "Baloo Tammudu 2" ),
    QStringLiteral( "Baloo Thambi 2" ),
    QStringLiteral( "Balsamiq Sans" ),
    QStringLiteral( "Balthazar" ),
    QStringLiteral( "Bangers" ),
    QStringLiteral( "Barlow" ),
    QStringLiteral( "Barlow Condensed" ),
    QStringLiteral( "Barlow Semi Condensed" ),
    QStringLiteral( "Barriecito" ),
    QStringLiteral( "Barrio" ),
    QStringLiteral( "Basic" ),
    QStringLiteral( "Baskervville" ),
    QStringLiteral( "Battambang" ),
    QStringLiteral( "Baumans" ),
    QStringLiteral( "Bayon" ),
    QStringLiteral( "Be Vietnam Pro" ),
    QStringLiteral( "Beau Rivage" ),
    QStringLiteral( "Bebas Neue" ),
    QStringLiteral( "Belanosima" ),
    QStringLiteral( "Belgrano" ),
    QStringLiteral( "Bellefair" ),
    QStringLiteral( "Belleza" ),
    QStringLiteral( "Bellota" ),
    QStringLiteral( "Bellota Text" ),
    QStringLiteral( "BenchNine" ),
    QStringLiteral( "Benne" ),
    QStringLiteral( "Bentham" ),
    QStringLiteral( "Berkshire Swash" ),
    QStringLiteral( "Besley" ),
    QStringLiteral( "Beth Ellen" ),
    QStringLiteral( "Bevan" ),
    QStringLiteral( "BhuTuka Expanded One" ),
    QStringLiteral( "Big Shoulders Display" ),
    QStringLiteral( "Big Shoulders Inline Display" ),
    QStringLiteral( "Big Shoulders Inline Text" ),
    QStringLiteral( "Big Shoulders Stencil Display" ),
    QStringLiteral( "Big Shoulders Stencil Text" ),
    QStringLiteral( "Big Shoulders Text" ),
    QStringLiteral( "Bigelow Rules" ),
    QStringLiteral( "Bigshot One" ),
    QStringLiteral( "Bilbo" ),
    QStringLiteral( "Bilbo Swash Caps" ),
    QStringLiteral( "BioRhyme" ),
    QStringLiteral( "BioRhyme Expanded" ),
    QStringLiteral( "Birthstone" ),
    QStringLiteral( "Birthstone Bounce" ),
    QStringLiteral( "Biryani" ),
    QStringLiteral( "Bitter" ),
    QStringLiteral( "Black And White Picture" ),
    QStringLiteral( "Black Han Sans" ),
    QStringLiteral( "Black Ops One" ),
    QStringLiteral( "Blaka" ),
    QStringLiteral( "Blaka Hollow" ),
    QStringLiteral( "Blaka Ink" ),
    QStringLiteral( "Blinker" ),
    QStringLiteral( "Bodoni Moda" ),
    QStringLiteral( "Bokor" ),
    QStringLiteral( "Bona Nova" ),
    QStringLiteral( "Bonbon" ),
    QStringLiteral( "Bonheur Royale" ),
    QStringLiteral( "Boogaloo" ),
    QStringLiteral( "Borel" ),
    QStringLiteral( "Bowlby One" ),
    QStringLiteral( "Bowlby One SC" ),
    QStringLiteral( "Braah One" ),
    QStringLiteral( "Brawler" ),
    QStringLiteral( "Bree Serif" ),
    QStringLiteral( "Bricolage Grotesque" ),
    QStringLiteral( "Bruno Ace" ),
    QStringLiteral( "Bruno Ace SC" ),
    QStringLiteral( "Brygada 1918" ),
    QStringLiteral( "Bubblegum Sans" ),
    QStringLiteral( "Bubbler One" ),
    QStringLiteral( "Buda" ),
    QStringLiteral( "Buenard" ),
    QStringLiteral( "Bungee" ),
    QStringLiteral( "Bungee Hairline" ),
    QStringLiteral( "Bungee Inline" ),
    QStringLiteral( "Bungee Outline" ),
    QStringLiteral( "Bungee Shade" ),
    QStringLiteral( "Bungee Spice" ),
    QStringLiteral( "Butcherman" ),
    QStringLiteral( "Butterfly Kids" ),
    QStringLiteral( "Cabin" ),
    QStringLiteral( "Cabin Condensed" ),
    QStringLiteral( "Cabin Sketch" ),
    QStringLiteral( "Caesar Dressing" ),
    QStringLiteral( "Cagliostro" ),
    QStringLiteral( "Cairo" ),
    QStringLiteral( "Cairo Play" ),
    QStringLiteral( "Caladea" ),
    QStringLiteral( "Calistoga" ),
    QStringLiteral( "Calligraffitti" ),
    QStringLiteral( "Cambay" ),
    QStringLiteral( "Cambo" ),
    QStringLiteral( "Candal" ),
    QStringLiteral( "Cantarell" ),
    QStringLiteral( "Cantata One" ),
    QStringLiteral( "Cantora One" ),
    QStringLiteral( "Caprasimo" ),
    QStringLiteral( "Capriola" ),
    QStringLiteral( "Caramel" ),
    QStringLiteral( "Carattere" ),
    QStringLiteral( "Cardo" ),
    QStringLiteral( "Carlito" ),
    QStringLiteral( "Carme" ),
    QStringLiteral( "Carrois Gothic" ),
    QStringLiteral( "Carrois Gothic SC" ),
    QStringLiteral( "Carter One" ),
    QStringLiteral( "Castoro" ),
    QStringLiteral( "Castoro Titling" ),
    QStringLiteral( "Catamaran" ),
    QStringLiteral( "Caudex" ),
    QStringLiteral( "Caveat" ),
    QStringLiteral( "Caveat Brush" ),
    QStringLiteral( "Cedarville Cursive" ),
    QStringLiteral( "Ceviche One" ),
    QStringLiteral( "Chakra Petch" ),
    QStringLiteral( "Changa" ),
    QStringLiteral( "Changa One" ),
    QStringLiteral( "Chango" ),
    QStringLiteral( "Charis SIL" ),
    QStringLiteral( "Charm" ),
    QStringLiteral( "Charmonman" ),
    QStringLiteral( "Chathura" ),
    QStringLiteral( "Chau Philomene One" ),
    QStringLiteral( "Chela One" ),
    QStringLiteral( "Chelsea Market" ),
    QStringLiteral( "Chenla" ),
    QStringLiteral( "Cherish" ),
    QStringLiteral( "Cherry Bomb One" ),
    QStringLiteral( "Cherry Cream Soda" ),
    QStringLiteral( "Cherry Swash" ),
    QStringLiteral( "Chewy" ),
    QStringLiteral( "Chicle" ),
    QStringLiteral( "Chilanka" ),
    QStringLiteral( "Chivo" ),
    QStringLiteral( "Chivo Mono" ),
    QStringLiteral( "Chokokutai" ),
    QStringLiteral( "Chonburi" ),
    QStringLiteral( "Cinzel" ),
    QStringLiteral( "Cinzel Decorative" ),
    QStringLiteral( "Clicker Script" ),
    QStringLiteral( "Climate Crisis" ),
    QStringLiteral( "Coda" ),
    QStringLiteral( "Coda Caption" ),
    QStringLiteral( "Codystar" ),
    QStringLiteral( "Coiny" ),
    QStringLiteral( "Combo" ),
    QStringLiteral( "Comfortaa" ),
    QStringLiteral( "Comforter" ),
    QStringLiteral( "Comforter Brush" ),
    QStringLiteral( "Comic Neue" ),
    QStringLiteral( "Coming Soon" ),
    QStringLiteral( "Comme" ),
    QStringLiteral( "Commissioner" ),
    QStringLiteral( "Concert One" ),
    QStringLiteral( "Condiment" ),
    QStringLiteral( "Content" ),
    QStringLiteral( "Contrail One" ),
    QStringLiteral( "Convergence" ),
    QStringLiteral( "Cookie" ),
    QStringLiteral( "Copse" ),
    QStringLiteral( "Corben" ),
    QStringLiteral( "Corinthia" ),
    QStringLiteral( "Cormorant" ),
    QStringLiteral( "Cormorant Garamond" ),
    QStringLiteral( "Cormorant Infant" ),
    QStringLiteral( "Cormorant SC" ),
    QStringLiteral( "Cormorant Unicase" ),
    QStringLiteral( "Cormorant Upright" ),
    QStringLiteral( "Courgette" ),
    QStringLiteral( "Courier Prime" ),
    QStringLiteral( "Cousine" ),
    QStringLiteral( "Coustard" ),
    QStringLiteral( "Covered By Your Grace" ),
    QStringLiteral( "Crafty Girls" ),
    QStringLiteral( "Creepster" ),
    QStringLiteral( "Crete Round" ),
    QStringLiteral( "Crimson Pro" ),
    QStringLiteral( "Crimson Text" ),
    QStringLiteral( "Croissant One" ),
    QStringLiteral( "Crushed" ),
    QStringLiteral( "Cuprum" ),
    QStringLiteral( "Cute Font" ),
    QStringLiteral( "Cutive" ),
    QStringLiteral( "Cutive Mono" ),
    QStringLiteral( "DM Mono" ),
    QStringLiteral( "DM Sans" ),
    QStringLiteral( "DM Serif Display" ),
    QStringLiteral( "DM Serif Text" ),
    QStringLiteral( "Dai Banna SIL" ),
    QStringLiteral( "Damion" ),
    QStringLiteral( "Dancing Script" ),
    QStringLiteral( "Dangrek" ),
    QStringLiteral( "Darker Grotesque" ),
    QStringLiteral( "Darumadrop One" ),
    QStringLiteral( "David Libre" ),
    QStringLiteral( "Dawning of a New Day" ),
    QStringLiteral( "Days One" ),
    QStringLiteral( "Dekko" ),
    QStringLiteral( "Dela Gothic One" ),
    QStringLiteral( "Delicious Handrawn" ),
    QStringLiteral( "Delius" ),
    QStringLiteral( "Delius Swash Caps" ),
    QStringLiteral( "Delius Unicase" ),
    QStringLiteral( "Della Respira" ),
    QStringLiteral( "Denk One" ),
    QStringLiteral( "Devonshire" ),
    QStringLiteral( "Dhurjati" ),
    QStringLiteral( "Didact Gothic" ),
    QStringLiteral( "Diphylleia" ),
    QStringLiteral( "Diplomata" ),
    QStringLiteral( "Diplomata SC" ),
    QStringLiteral( "Do Hyeon" ),
    QStringLiteral( "Dokdo" ),
    QStringLiteral( "Domine" ),
    QStringLiteral( "Donegal One" ),
    QStringLiteral( "Dongle" ),
    QStringLiteral( "Doppio One" ),
    QStringLiteral( "Dorsa" ),
    QStringLiteral( "Dosis" ),
    QStringLiteral( "DotGothic16" ),
    QStringLiteral( "Dr Sugiyama" ),
    QStringLiteral( "Droid Sans" ),
    QStringLiteral( "Droid Sans Mono" ),
    QStringLiteral( "Droid Serif" ),
    QStringLiteral( "Duru Sans" ),
    QStringLiteral( "DynaPuff" ),
    QStringLiteral( "Dynalight" ),
    QStringLiteral( "EB Garamond" ),
    QStringLiteral( "Eagle Lake" ),
    QStringLiteral( "East Sea Dokdo" ),
    QStringLiteral( "Eater" ),
    QStringLiteral( "Economica" ),
    QStringLiteral( "Eczar" ),
    QStringLiteral( "Edu NSW ACT Foundation" ),
    QStringLiteral( "Edu QLD Beginner" ),
    QStringLiteral( "Edu SA Beginner" ),
    QStringLiteral( "Edu TAS Beginner" ),
    QStringLiteral( "Edu VIC WA NT Beginner" ),
    QStringLiteral( "Ek Mukta" ),
    QStringLiteral( "El Messiri" ),
    QStringLiteral( "Electrolize" ),
    QStringLiteral( "Elsie" ),
    QStringLiteral( "Elsie Swash Caps" ),
    QStringLiteral( "Emblema One" ),
    QStringLiteral( "Emilys Candy" ),
    QStringLiteral( "Encode Sans" ),
    QStringLiteral( "Encode Sans Condensed" ),
    QStringLiteral( "Encode Sans Expanded" ),
    QStringLiteral( "Encode Sans SC" ),
    QStringLiteral( "Encode Sans Semi Condensed" ),
    QStringLiteral( "Encode Sans Semi Expanded" ),
    QStringLiteral( "Engagement" ),
    QStringLiteral( "Englebert" ),
    QStringLiteral( "Enriqueta" ),
    QStringLiteral( "Ephesis" ),
    QStringLiteral( "Epilogue" ),
    QStringLiteral( "Erica One" ),
    QStringLiteral( "Esteban" ),
    QStringLiteral( "Estonia" ),
    QStringLiteral( "Euphoria Script" ),
    QStringLiteral( "Ewert" ),
    QStringLiteral( "Exo" ),
    QStringLiteral( "Exo 2" ),
    QStringLiteral( "Expletus Sans" ),
    QStringLiteral( "Explora" ),
    QStringLiteral( "Fahkwang" ),
    QStringLiteral( "Familjen Grotesk" ),
    QStringLiteral( "Fanwood Text" ),
    QStringLiteral( "Farro" ),
    QStringLiteral( "Farsan" ),
    QStringLiteral( "Fascinate" ),
    QStringLiteral( "Fascinate Inline" ),
    QStringLiteral( "Faster One" ),
    QStringLiteral( "Fasthand" ),
    QStringLiteral( "Fauna One" ),
    QStringLiteral( "Faustina" ),
    QStringLiteral( "Federant" ),
    QStringLiteral( "Federo" ),
    QStringLiteral( "Felipa" ),
    QStringLiteral( "Fenix" ),
    QStringLiteral( "Festive" ),
    QStringLiteral( "Figtree" ),
    QStringLiteral( "Finger Paint" ),
    QStringLiteral( "Finlandica" ),
    QStringLiteral( "Fira Code" ),
    QStringLiteral( "Fira Mono" ),
    QStringLiteral( "Fira Sans" ),
    QStringLiteral( "Fira Sans Condensed" ),
    QStringLiteral( "Fira Sans Extra Condensed" ),
    QStringLiteral( "Fira Sans Extra Condensed " ),
    QStringLiteral( "Fjalla One" ),
    QStringLiteral( "Fjord One" ),
    QStringLiteral( "Flamenco" ),
    QStringLiteral( "Flavors" ),
    QStringLiteral( "Fleur De Leah" ),
    QStringLiteral( "Flow Block" ),
    QStringLiteral( "Flow Circular" ),
    QStringLiteral( "Flow Rounded" ),
    QStringLiteral( "Foldit" ),
    QStringLiteral( "Fondamento" ),
    QStringLiteral( "Fontdiner Swanky" ),
    QStringLiteral( "Forum" ),
    QStringLiteral( "Fragment Mono" ),
    QStringLiteral( "Francois One" ),
    QStringLiteral( "Frank Ruhl Libre" ),
    QStringLiteral( "Fraunces" ),
    QStringLiteral( "Freckle Face" ),
    QStringLiteral( "Fredericka the Great" ),
    QStringLiteral( "Fredoka" ),
    QStringLiteral( "Fredoka One" ),
    QStringLiteral( "Freehand" ),
    QStringLiteral( "Fresca" ),
    QStringLiteral( "Frijole" ),
    QStringLiteral( "Fruktur" ),
    QStringLiteral( "Fugaz One" ),
    QStringLiteral( "Fuggles" ),
    QStringLiteral( "Fuzzy Bubbles" ),
    QStringLiteral( "GFS Didot" ),
    QStringLiteral( "GFS Neohellenic" ),
    QStringLiteral( "Gabriela" ),
    QStringLiteral( "Gaegu" ),
    QStringLiteral( "Gafata" ),
    QStringLiteral( "Gajraj One" ),
    QStringLiteral( "Galada" ),
    QStringLiteral( "Galdeano" ),
    QStringLiteral( "Galindo" ),
    QStringLiteral( "Gamja Flower" ),
    QStringLiteral( "Gantari" ),
    QStringLiteral( "Gasoek One" ),
    QStringLiteral( "Gayathri" ),
    QStringLiteral( "Gelasio" ),
    QStringLiteral( "Gemunu Libre" ),
    QStringLiteral( "Genos" ),
    QStringLiteral( "Gentium Basic" ),
    QStringLiteral( "Gentium Book Basic" ),
    QStringLiteral( "Gentium Book Plus" ),
    QStringLiteral( "Gentium Plus" ),
    QStringLiteral( "Geo" ),
    QStringLiteral( "Geologica" ),
    QStringLiteral( "Georama" ),
    QStringLiteral( "Geostar" ),
    QStringLiteral( "Geostar Fill" ),
    QStringLiteral( "Germania One" ),
    QStringLiteral( "Gideon Roman" ),
    QStringLiteral( "Gidugu" ),
    QStringLiteral( "Gilda Display" ),
    QStringLiteral( "Girassol" ),
    QStringLiteral( "Give You Glory" ),
    QStringLiteral( "Glass Antiqua" ),
    QStringLiteral( "Glegoo" ),
    QStringLiteral( "Gloock" ),
    QStringLiteral( "Gloria Hallelujah" ),
    QStringLiteral( "Glory" ),
    QStringLiteral( "Gluten" ),
    QStringLiteral( "Goblin One" ),
    QStringLiteral( "Gochi Hand" ),
    QStringLiteral( "Goldman" ),
    QStringLiteral( "Golos Text" ),
    QStringLiteral( "Gorditas" ),
    QStringLiteral( "Gothic A1" ),
    QStringLiteral( "Gotu" ),
    QStringLiteral( "Goudy Bookletter 1911" ),
    QStringLiteral( "Gowun Batang" ),
    QStringLiteral( "Gowun Dodum" ),
    QStringLiteral( "Graduate" ),
    QStringLiteral( "Grand Hotel" ),
    QStringLiteral( "Grandiflora One" ),
    QStringLiteral( "Grandstander" ),
    QStringLiteral( "Grape Nuts" ),
    QStringLiteral( "Gravitas One" ),
    QStringLiteral( "Great Vibes" ),
    QStringLiteral( "Grechen Fuemen" ),
    QStringLiteral( "Grenze" ),
    QStringLiteral( "Grenze Gotisch" ),
    QStringLiteral( "Grey Qo" ),
    QStringLiteral( "Griffy" ),
    QStringLiteral( "Gruppo" ),
    QStringLiteral( "Gudea" ),
    QStringLiteral( "Gugi" ),
    QStringLiteral( "Gulzar" ),
    QStringLiteral( "Gupter" ),
    QStringLiteral( "Gurajada" ),
    QStringLiteral( "Gwendolyn" ),
    QStringLiteral( "Habibi" ),
    QStringLiteral( "Hachi Maru Pop" ),
    QStringLiteral( "Hahmlet" ),
    QStringLiteral( "Halant" ),
    QStringLiteral( "Hammersmith One" ),
    QStringLiteral( "Hanalei" ),
    QStringLiteral( "Hanalei Fill" ),
    QStringLiteral( "Handjet" ),
    QStringLiteral( "Handlee" ),
    QStringLiteral( "Hanken Grotesk" ),
    QStringLiteral( "Hanuman" ),
    QStringLiteral( "Happy Monkey" ),
    QStringLiteral( "Harmattan" ),
    QStringLiteral( "Headland One" ),
    QStringLiteral( "Heebo" ),
    QStringLiteral( "Henny Penny" ),
    QStringLiteral( "Hepta Slab" ),
    QStringLiteral( "Herr Von Muellerhoff" ),
    QStringLiteral( "Hi Melody" ),
    QStringLiteral( "Hina Mincho" ),
    QStringLiteral( "Hind" ),
    QStringLiteral( "Hind Guntur" ),
    QStringLiteral( "Hind Madurai" ),
    QStringLiteral( "Hind Siliguri" ),
    QStringLiteral( "Hind Vadodara" ),
    QStringLiteral( "Holtwood One SC" ),
    QStringLiteral( "Homemade Apple" ),
    QStringLiteral( "Homenaje" ),
    QStringLiteral( "Hubballi" ),
    QStringLiteral( "Hurricane" ),
    QStringLiteral( "IBM Plex Mono" ),
    QStringLiteral( "IBM Plex Sans" ),
    QStringLiteral( "IBM Plex Sans Arabic" ),
    QStringLiteral( "IBM Plex Sans Condensed" ),
    QStringLiteral( "IBM Plex Sans Devanagari" ),
    QStringLiteral( "IBM Plex Sans Hebrew" ),
    QStringLiteral( "IBM Plex Sans JP" ),
    QStringLiteral( "IBM Plex Sans KR" ),
    QStringLiteral( "IBM Plex Sans Thai" ),
    QStringLiteral( "IBM Plex Sans Thai Looped" ),
    QStringLiteral( "IBM Plex Serif" ),
    QStringLiteral( "IM Fell DW Pica" ),
    QStringLiteral( "IM Fell DW Pica SC" ),
    QStringLiteral( "IM Fell Double Pica" ),
    QStringLiteral( "IM Fell Double Pica SC" ),
    QStringLiteral( "IM Fell English" ),
    QStringLiteral( "IM Fell English SC" ),
    QStringLiteral( "IM Fell French Canon" ),
    QStringLiteral( "IM Fell French Canon SC" ),
    QStringLiteral( "IM Fell Great Primer" ),
    QStringLiteral( "IM Fell Great Primer SC" ),
    QStringLiteral( "Ibarra Real Nova" ),
    QStringLiteral( "Iceberg" ),
    QStringLiteral( "Iceland" ),
    QStringLiteral( "Imbue" ),
    QStringLiteral( "Imperial Script" ),
    QStringLiteral( "Imprima" ),
    QStringLiteral( "Inconsolata" ),
    QStringLiteral( "Inder" ),
    QStringLiteral( "Indie Flower" ),
    QStringLiteral( "Ingrid Darling" ),
    QStringLiteral( "Inika" ),
    QStringLiteral( "Inknut Antiqua" ),
    QStringLiteral( "Inria Sans" ),
    QStringLiteral( "Inria Serif" ),
    QStringLiteral( "Inspiration" ),
    QStringLiteral( "Instrument Sans" ),
    QStringLiteral( "Instrument Serif" ),
    QStringLiteral( "Inter" ),
    QStringLiteral( "Inter Tight" ),
    QStringLiteral( "Irish Grover" ),
    QStringLiteral( "Island Moments" ),
    QStringLiteral( "Istok Web" ),
    QStringLiteral( "Italiana" ),
    QStringLiteral( "Italianno" ),
    QStringLiteral( "Itim" ),
    QStringLiteral( "Jacques Francois" ),
    QStringLiteral( "Jacques Francois Shadow" ),
    QStringLiteral( "Jaldi" ),
    QStringLiteral( "JetBrains Mono" ),
    QStringLiteral( "Jim Nightshade" ),
    QStringLiteral( "Joan" ),
    QStringLiteral( "Jockey One" ),
    QStringLiteral( "Jolly Lodger" ),
    QStringLiteral( "Jomhuria" ),
    QStringLiteral( "Jomolhari" ),
    QStringLiteral( "Josefin Sans" ),
    QStringLiteral( "Josefin Slab" ),
    QStringLiteral( "Jost" ),
    QStringLiteral( "Joti One" ),
    QStringLiteral( "Jua" ),
    QStringLiteral( "Judson" ),
    QStringLiteral( "Julee" ),
    QStringLiteral( "Julius Sans One" ),
    QStringLiteral( "Junge" ),
    QStringLiteral( "Jura" ),
    QStringLiteral( "Just Another Hand" ),
    QStringLiteral( "Just Me Again Down Here" ),
    QStringLiteral( "K2D" ),
    QStringLiteral( "Kablammo" ),
    QStringLiteral( "Kadwa" ),
    QStringLiteral( "Kaisei Decol" ),
    QStringLiteral( "Kaisei HarunoUmi" ),
    QStringLiteral( "Kaisei Opti" ),
    QStringLiteral( "Kaisei Tokumin" ),
    QStringLiteral( "Kalam" ),
    QStringLiteral( "Kameron" ),
    QStringLiteral( "Kanit" ),
    QStringLiteral( "Kantumruy" ),
    QStringLiteral( "Kantumruy Pro" ),
    QStringLiteral( "Karantina" ),
    QStringLiteral( "Karla" ),
    QStringLiteral( "Karma" ),
    QStringLiteral( "Katibeh" ),
    QStringLiteral( "Kaushan Script" ),
    QStringLiteral( "Kavivanar" ),
    QStringLiteral( "Kavoon" ),
    QStringLiteral( "Kdam Thmor" ),
    QStringLiteral( "Kdam Thmor Pro" ),
    QStringLiteral( "Keania One" ),
    QStringLiteral( "Kelly Slab" ),
    QStringLiteral( "Kenia" ),
    QStringLiteral( "Khand" ),
    QStringLiteral( "Khmer" ),
    QStringLiteral( "Khula" ),
    QStringLiteral( "Kings" ),
    QStringLiteral( "Kirang Haerang" ),
    QStringLiteral( "Kite One" ),
    QStringLiteral( "Kiwi Maru" ),
    QStringLiteral( "Klee One" ),
    QStringLiteral( "Knewave" ),
    QStringLiteral( "KoHo" ),
    QStringLiteral( "Kodchasan" ),
    QStringLiteral( "Koh Santepheap" ),
    QStringLiteral( "Kolker Brush" ),
    QStringLiteral( "Konkhmer Sleokchher" ),
    QStringLiteral( "Kosugi" ),
    QStringLiteral( "Kosugi Maru" ),
    QStringLiteral( "Kotta One" ),
    QStringLiteral( "Koulen" ),
    QStringLiteral( "Kranky" ),
    QStringLiteral( "Kreon" ),
    QStringLiteral( "Kristi" ),
    QStringLiteral( "Krona One" ),
    QStringLiteral( "Krub" ),
    QStringLiteral( "Kufam" ),
    QStringLiteral( "Kulim Park" ),
    QStringLiteral( "Kumar One" ),
    QStringLiteral( "Kumar One Outline" ),
    QStringLiteral( "Kumbh Sans" ),
    QStringLiteral( "Kurale" ),
    QStringLiteral( "La Belle Aurore" ),
    QStringLiteral( "Labrada" ),
    QStringLiteral( "Lacquer" ),
    QStringLiteral( "Laila" ),
    QStringLiteral( "Lakki Reddy" ),
    QStringLiteral( "Lalezar" ),
    QStringLiteral( "Lancelot" ),
    QStringLiteral( "Langar" ),
    QStringLiteral( "Lateef" ),
    QStringLiteral( "Lato" ),
    QStringLiteral( "Lavishly Yours" ),
    QStringLiteral( "League Gothic" ),
    QStringLiteral( "League Script" ),
    QStringLiteral( "League Spartan" ),
    QStringLiteral( "Leckerli One" ),
    QStringLiteral( "Ledger" ),
    QStringLiteral( "Lekton" ),
    QStringLiteral( "Lemon" ),
    QStringLiteral( "Lemonada" ),
    QStringLiteral( "Lexend" ),
    QStringLiteral( "Lexend Deca" ),
    QStringLiteral( "Lexend Exa" ),
    QStringLiteral( "Lexend Giga" ),
    QStringLiteral( "Lexend Mega" ),
    QStringLiteral( "Lexend Peta" ),
    QStringLiteral( "Lexend Tera" ),
    QStringLiteral( "Lexend Zetta" ),
    QStringLiteral( "Libre Barcode 128" ),
    QStringLiteral( "Libre Barcode 128 Text" ),
    QStringLiteral( "Libre Barcode 39" ),
    QStringLiteral( "Libre Barcode 39 Extended" ),
    QStringLiteral( "Libre Barcode 39 Extended Text" ),
    QStringLiteral( "Libre Barcode 39 Text" ),
    QStringLiteral( "Libre Barcode EAN13 Text" ),
    QStringLiteral( "Libre Baskerville" ),
    QStringLiteral( "Libre Bodoni" ),
    QStringLiteral( "Libre Caslon Display" ),
    QStringLiteral( "Libre Caslon Text" ),
    QStringLiteral( "Libre Franklin" ),
    QStringLiteral( "Licorice" ),
    QStringLiteral( "Life Savers" ),
    QStringLiteral( "Lilita One" ),
    QStringLiteral( "Lily Script One" ),
    QStringLiteral( "Limelight" ),
    QStringLiteral( "Linden Hill" ),
    QStringLiteral( "Lisu Bosa" ),
    QStringLiteral( "Literata" ),
    QStringLiteral( "Liu Jian Mao Cao" ),
    QStringLiteral( "Livvic" ),
    QStringLiteral( "Lobster" ),
    QStringLiteral( "Lobster Two" ),
    QStringLiteral( "Londrina Outline" ),
    QStringLiteral( "Londrina Shadow" ),
    QStringLiteral( "Londrina Sketch" ),
    QStringLiteral( "Londrina Solid" ),
    QStringLiteral( "Long Cang" ),
    QStringLiteral( "Lora" ),
    QStringLiteral( "Love Light" ),
    QStringLiteral( "Love Ya Like A Sister" ),
    QStringLiteral( "Loved by the King" ),
    QStringLiteral( "Lovers Quarrel" ),
    QStringLiteral( "Luckiest Guy" ),
    QStringLiteral( "Lugrasimo" ),
    QStringLiteral( "Lumanosimo" ),
    QStringLiteral( "Lunasima" ),
    QStringLiteral( "Lusitana" ),
    QStringLiteral( "Lustria" ),
    QStringLiteral( "Luxurious Roman" ),
    QStringLiteral( "Luxurious Script" ),
    QStringLiteral( "M PLUS 1" ),
    QStringLiteral( "M PLUS 1 Code" ),
    QStringLiteral( "M PLUS 1p" ),
    QStringLiteral( "M PLUS 2" ),
    QStringLiteral( "M PLUS Code Latin" ),
    QStringLiteral( "M PLUS Rounded 1c" ),
    QStringLiteral( "Ma Shan Zheng" ),
    QStringLiteral( "Macondo" ),
    QStringLiteral( "Macondo Swash Caps" ),
    QStringLiteral( "Mada" ),
    QStringLiteral( "Magra" ),
    QStringLiteral( "Maiden Orange" ),
    QStringLiteral( "Maitree" ),
    QStringLiteral( "Major Mono Display" ),
    QStringLiteral( "Mako" ),
    QStringLiteral( "Mali" ),
    QStringLiteral( "Mallanna" ),
    QStringLiteral( "Mandali" ),
    QStringLiteral( "Manjari" ),
    QStringLiteral( "Manrope" ),
    QStringLiteral( "Mansalva" ),
    QStringLiteral( "Manuale" ),
    QStringLiteral( "Marcellus" ),
    QStringLiteral( "Marcellus SC" ),
    QStringLiteral( "Marck Script" ),
    QStringLiteral( "Margarine" ),
    QStringLiteral( "Marhey" ),
    QStringLiteral( "Markazi Text" ),
    QStringLiteral( "Marko One" ),
    QStringLiteral( "Marmelad" ),
    QStringLiteral( "Martel" ),
    QStringLiteral( "Martel Sans" ),
    QStringLiteral( "Martian Mono" ),
    QStringLiteral( "Marvel" ),
    QStringLiteral( "Mate" ),
    QStringLiteral( "Mate SC" ),
    QStringLiteral( "Material Icons" ),
    QStringLiteral( "Material Icons Outlined" ),
    QStringLiteral( "Material Icons Round" ),
    QStringLiteral( "Material Icons Sharp" ),
    QStringLiteral( "Material Icons Two Tone" ),
    QStringLiteral( "Material Symbols Outlined" ),
    QStringLiteral( "Material Symbols Rounded" ),
    QStringLiteral( "Material Symbols Sharp" ),
    QStringLiteral( "Maven Pro" ),
    QStringLiteral( "McLaren" ),
    QStringLiteral( "Mea Culpa" ),
    QStringLiteral( "Meddon" ),
    QStringLiteral( "MedievalSharp" ),
    QStringLiteral( "Medula One" ),
    QStringLiteral( "Meera Inimai" ),
    QStringLiteral( "Megrim" ),
    QStringLiteral( "Meie Script" ),
    QStringLiteral( "Meow Script" ),
    QStringLiteral( "Merienda" ),
    QStringLiteral( "Merienda One" ),
    QStringLiteral( "Merriweather" ),
    QStringLiteral( "Merriweather Sans" ),
    QStringLiteral( "Metal" ),
    QStringLiteral( "Metal Mania" ),
    QStringLiteral( "Metamorphous" ),
    QStringLiteral( "Metrophobic" ),
    QStringLiteral( "Michroma" ),
    QStringLiteral( "Milonga" ),
    QStringLiteral( "Miltonian" ),
    QStringLiteral( "Miltonian Tattoo" ),
    QStringLiteral( "Mina" ),
    QStringLiteral( "Mingzat" ),
    QStringLiteral( "Miniver" ),
    QStringLiteral( "Miriam Libre" ),
    QStringLiteral( "Mirza" ),
    QStringLiteral( "Miss Fajardose" ),
    QStringLiteral( "Mitr" ),
    QStringLiteral( "Mochiy Pop One" ),
    QStringLiteral( "Mochiy Pop P One" ),
    QStringLiteral( "Modak" ),
    QStringLiteral( "Modern Antiqua" ),
    QStringLiteral( "Mogra" ),
    QStringLiteral( "Mohave" ),
    QStringLiteral( "Moirai One" ),
    QStringLiteral( "Molengo" ),
    QStringLiteral( "Molle" ),
    QStringLiteral( "Monda" ),
    QStringLiteral( "Monofett" ),
    QStringLiteral( "Monomaniac One" ),
    QStringLiteral( "Monoton" ),
    QStringLiteral( "Monsieur La Doulaise" ),
    QStringLiteral( "Montaga" ),
    QStringLiteral( "Montagu Slab" ),
    QStringLiteral( "MonteCarlo" ),
    QStringLiteral( "Montez" ),
    QStringLiteral( "Montserrat" ),
    QStringLiteral( "Montserrat Alternates" ),
    QStringLiteral( "Montserrat Subrayada" ),
    QStringLiteral( "Moo Lah Lah" ),
    QStringLiteral( "Moon Dance" ),
    QStringLiteral( "Moul" ),
    QStringLiteral( "Moulpali" ),
    QStringLiteral( "Mountains of Christmas" ),
    QStringLiteral( "Mouse Memoirs" ),
    QStringLiteral( "Mr Bedfort" ),
    QStringLiteral( "Mr Dafoe" ),
    QStringLiteral( "Mr De Haviland" ),
    QStringLiteral( "Mrs Saint Delafield" ),
    QStringLiteral( "Mrs Sheppards" ),
    QStringLiteral( "Ms Madi" ),
    QStringLiteral( "Mukta" ),
    QStringLiteral( "Mukta Mahee" ),
    QStringLiteral( "Mukta Malar" ),
    QStringLiteral( "Mukta Vaani" ),
    QStringLiteral( "Muli" ),
    QStringLiteral( "Mulish" ),
    QStringLiteral( "Murecho" ),
    QStringLiteral( "MuseoModerno" ),
    QStringLiteral( "My Soul" ),
    QStringLiteral( "Mynerve" ),
    QStringLiteral( "Mystery Quest" ),
    QStringLiteral( "NTR" ),
    QStringLiteral( "Nabla" ),
    QStringLiteral( "Nanum Brush Script" ),
    QStringLiteral( "Nanum Gothic" ),
    QStringLiteral( "Nanum Gothic Coding" ),
    QStringLiteral( "Nanum Myeongjo" ),
    QStringLiteral( "Nanum Pen Script" ),
    QStringLiteral( "Narnoor" ),
    QStringLiteral( "Neonderthaw" ),
    QStringLiteral( "Nerko One" ),
    QStringLiteral( "Neucha" ),
    QStringLiteral( "Neuton" ),
    QStringLiteral( "New Rocker" ),
    QStringLiteral( "New Tegomin" ),
    QStringLiteral( "News Cycle" ),
    QStringLiteral( "Newsreader" ),
    QStringLiteral( "Niconne" ),
    QStringLiteral( "Niramit" ),
    QStringLiteral( "Nixie One" ),
    QStringLiteral( "Nobile" ),
    QStringLiteral( "Nokora" ),
    QStringLiteral( "Norican" ),
    QStringLiteral( "Nosifer" ),
    QStringLiteral( "Notable" ),
    QStringLiteral( "Nothing You Could Do" ),
    QStringLiteral( "Noticia Text" ),
    QStringLiteral( "Noto Color Emoji" ),
    QStringLiteral( "Noto Emoji" ),
    QStringLiteral( "Noto Kufi Arabic" ),
    QStringLiteral( "Noto Music" ),
    QStringLiteral( "Noto Naskh Arabic" ),
    QStringLiteral( "Noto Nastaliq Urdu" ),
    QStringLiteral( "Noto Rashi Hebrew" ),
    QStringLiteral( "Noto Sans" ),
    QStringLiteral( "Noto Sans Adlam" ),
    QStringLiteral( "Noto Sans Adlam Unjoined" ),
    QStringLiteral( "Noto Sans Anatolian Hieroglyphs" ),
    QStringLiteral( "Noto Sans Arabic" ),
    QStringLiteral( "Noto Sans Armenian" ),
    QStringLiteral( "Noto Sans Avestan" ),
    QStringLiteral( "Noto Sans Balinese" ),
    QStringLiteral( "Noto Sans Bamum" ),
    QStringLiteral( "Noto Sans Bassa Vah" ),
    QStringLiteral( "Noto Sans Batak" ),
    QStringLiteral( "Noto Sans Bengali" ),
    QStringLiteral( "Noto Sans Bhaiksuki" ),
    QStringLiteral( "Noto Sans Brahmi" ),
    QStringLiteral( "Noto Sans Buginese" ),
    QStringLiteral( "Noto Sans Buhid" ),
    QStringLiteral( "Noto Sans Canadian Aboriginal" ),
    QStringLiteral( "Noto Sans Carian" ),
    QStringLiteral( "Noto Sans Caucasian Albanian" ),
    QStringLiteral( "Noto Sans Chakma" ),
    QStringLiteral( "Noto Sans Cham" ),
    QStringLiteral( "Noto Sans Cherokee" ),
    QStringLiteral( "Noto Sans Chorasmian" ),
    QStringLiteral( "Noto Sans Coptic" ),
    QStringLiteral( "Noto Sans Cuneiform" ),
    QStringLiteral( "Noto Sans Cypriot" ),
    QStringLiteral( "Noto Sans Cypro Minoan" ),
    QStringLiteral( "Noto Sans Deseret" ),
    QStringLiteral( "Noto Sans Devanagari" ),
    QStringLiteral( "Noto Sans Display" ),
    QStringLiteral( "Noto Sans Duployan" ),
    QStringLiteral( "Noto Sans Egyptian Hieroglyphs" ),
    QStringLiteral( "Noto Sans Elbasan" ),
    QStringLiteral( "Noto Sans Elymaic" ),
    QStringLiteral( "Noto Sans Ethiopic" ),
    QStringLiteral( "Noto Sans Georgian" ),
    QStringLiteral( "Noto Sans Glagolitic" ),
    QStringLiteral( "Noto Sans Gothic" ),
    QStringLiteral( "Noto Sans Grantha" ),
    QStringLiteral( "Noto Sans Gujarati" ),
    QStringLiteral( "Noto Sans Gunjala Gondi" ),
    QStringLiteral( "Noto Sans Gurmukhi" ),
    QStringLiteral( "Noto Sans HK" ),
    QStringLiteral( "Noto Sans Hanifi Rohingya" ),
    QStringLiteral( "Noto Sans Hanunoo" ),
    QStringLiteral( "Noto Sans Hatran" ),
    QStringLiteral( "Noto Sans Hebrew" ),
    QStringLiteral( "Noto Sans Imperial Aramaic" ),
    QStringLiteral( "Noto Sans Indic Siyaq Numbers" ),
    QStringLiteral( "Noto Sans Inscriptional Pahlavi" ),
    QStringLiteral( "Noto Sans Inscriptional Parthian" ),
    QStringLiteral( "Noto Sans JP" ),
    QStringLiteral( "Noto Sans Japanese" ),
    QStringLiteral( "Noto Sans Javanese" ),
    QStringLiteral( "Noto Sans KR" ),
    QStringLiteral( "Noto Sans Kaithi" ),
    QStringLiteral( "Noto Sans Kannada" ),
    QStringLiteral( "Noto Sans Kayah Li" ),
    QStringLiteral( "Noto Sans Kharoshthi" ),
    QStringLiteral( "Noto Sans Khmer" ),
    QStringLiteral( "Noto Sans Khojki" ),
    QStringLiteral( "Noto Sans Khudawadi" ),
    QStringLiteral( "Noto Sans Korean" ),
    QStringLiteral( "Noto Sans Lao" ),
    QStringLiteral( "Noto Sans Lao Looped" ),
    QStringLiteral( "Noto Sans Lepcha" ),
    QStringLiteral( "Noto Sans Limbu" ),
    QStringLiteral( "Noto Sans Linear A" ),
    QStringLiteral( "Noto Sans Linear B" ),
    QStringLiteral( "Noto Sans Lisu" ),
    QStringLiteral( "Noto Sans Lycian" ),
    QStringLiteral( "Noto Sans Lydian" ),
    QStringLiteral( "Noto Sans Mahajani" ),
    QStringLiteral( "Noto Sans Malayalam" ),
    QStringLiteral( "Noto Sans Mandaic" ),
    QStringLiteral( "Noto Sans Manichaean" ),
    QStringLiteral( "Noto Sans Marchen" ),
    QStringLiteral( "Noto Sans Masaram Gondi" ),
    QStringLiteral( "Noto Sans Math" ),
    QStringLiteral( "Noto Sans Mayan Numerals" ),
    QStringLiteral( "Noto Sans Medefaidrin" ),
    QStringLiteral( "Noto Sans Meetei Mayek" ),
    QStringLiteral( "Noto Sans Mende Kikakui" ),
    QStringLiteral( "Noto Sans Meroitic" ),
    QStringLiteral( "Noto Sans Miao" ),
    QStringLiteral( "Noto Sans Modi" ),
    QStringLiteral( "Noto Sans Mongolian" ),
    QStringLiteral( "Noto Sans Mono" ),
    QStringLiteral( "Noto Sans Mro" ),
    QStringLiteral( "Noto Sans Multani" ),
    QStringLiteral( "Noto Sans Myanmar" ),
    QStringLiteral( "Noto Sans NKo" ),
    QStringLiteral( "Noto Sans Nabataean" ),
    QStringLiteral( "Noto Sans Nag Mundari" ),
    QStringLiteral( "Noto Sans Nandinagari" ),
    QStringLiteral( "Noto Sans New Tai Lue" ),
    QStringLiteral( "Noto Sans Newa" ),
    QStringLiteral( "Noto Sans Nushu" ),
    QStringLiteral( "Noto Sans Ogham" ),
    QStringLiteral( "Noto Sans Ol Chiki" ),
    QStringLiteral( "Noto Sans Old Hungarian" ),
    QStringLiteral( "Noto Sans Old Italic" ),
    QStringLiteral( "Noto Sans Old North Arabian" ),
    QStringLiteral( "Noto Sans Old Permic" ),
    QStringLiteral( "Noto Sans Old Persian" ),
    QStringLiteral( "Noto Sans Old Sogdian" ),
    QStringLiteral( "Noto Sans Old South Arabian" ),
    QStringLiteral( "Noto Sans Old Turkic" ),
    QStringLiteral( "Noto Sans Oriya" ),
    QStringLiteral( "Noto Sans Osage" ),
    QStringLiteral( "Noto Sans Osmanya" ),
    QStringLiteral( "Noto Sans Pahawh Hmong" ),
    QStringLiteral( "Noto Sans Palmyrene" ),
    QStringLiteral( "Noto Sans Pau Cin Hau" ),
    QStringLiteral( "Noto Sans Phags Pa" ),
    QStringLiteral( "Noto Sans Phoenician" ),
    QStringLiteral( "Noto Sans Psalter Pahlavi" ),
    QStringLiteral( "Noto Sans Rejang" ),
    QStringLiteral( "Noto Sans Runic" ),
    QStringLiteral( "Noto Sans SC" ),
    QStringLiteral( "Noto Sans Samaritan" ),
    QStringLiteral( "Noto Sans Saurashtra" ),
    QStringLiteral( "Noto Sans Sharada" ),
    QStringLiteral( "Noto Sans Shavian" ),
    QStringLiteral( "Noto Sans Siddham" ),
    QStringLiteral( "Noto Sans SignWriting" ),
    QStringLiteral( "Noto Sans Sinhala" ),
    QStringLiteral( "Noto Sans Sogdian" ),
    QStringLiteral( "Noto Sans Sora Sompeng" ),
    QStringLiteral( "Noto Sans Soyombo" ),
    QStringLiteral( "Noto Sans Sundanese" ),
    QStringLiteral( "Noto Sans Syloti Nagri" ),
    QStringLiteral( "Noto Sans Symbols" ),
    QStringLiteral( "Noto Sans Symbols 2" ),
    QStringLiteral( "Noto Sans Syriac" ),
    QStringLiteral( "Noto Sans Syriac Eastern" ),
    QStringLiteral( "Noto Sans TC" ),
    QStringLiteral( "Noto Sans Tagalog" ),
    QStringLiteral( "Noto Sans Tagbanwa" ),
    QStringLiteral( "Noto Sans Tai Le" ),
    QStringLiteral( "Noto Sans Tai Tham" ),
    QStringLiteral( "Noto Sans Tai Viet" ),
    QStringLiteral( "Noto Sans Takri" ),
    QStringLiteral( "Noto Sans Tamil" ),
    QStringLiteral( "Noto Sans Tamil Supplement" ),
    QStringLiteral( "Noto Sans Tangsa" ),
    QStringLiteral( "Noto Sans Telugu" ),
    QStringLiteral( "Noto Sans Thaana" ),
    QStringLiteral( "Noto Sans Thai" ),
    QStringLiteral( "Noto Sans Thai Looped" ),
    QStringLiteral( "Noto Sans Tifinagh" ),
    QStringLiteral( "Noto Sans Tirhuta" ),
    QStringLiteral( "Noto Sans Ugaritic" ),
    QStringLiteral( "Noto Sans Vai" ),
    QStringLiteral( "Noto Sans Vithkuqi" ),
    QStringLiteral( "Noto Sans Wancho" ),
    QStringLiteral( "Noto Sans Warang Citi" ),
    QStringLiteral( "Noto Sans Yi" ),
    QStringLiteral( "Noto Sans Zanabazar Square" ),
    QStringLiteral( "Noto Serif" ),
    QStringLiteral( "Noto Serif Ahom" ),
    QStringLiteral( "Noto Serif Armenian" ),
    QStringLiteral( "Noto Serif Balinese" ),
    QStringLiteral( "Noto Serif Bengali" ),
    QStringLiteral( "Noto Serif Devanagari" ),
    QStringLiteral( "Noto Serif Display" ),
    QStringLiteral( "Noto Serif Dogra" ),
    QStringLiteral( "Noto Serif Ethiopic" ),
    QStringLiteral( "Noto Serif Georgian" ),
    QStringLiteral( "Noto Serif Grantha" ),
    QStringLiteral( "Noto Serif Gujarati" ),
    QStringLiteral( "Noto Serif Gurmukhi" ),
    QStringLiteral( "Noto Serif HK" ),
    QStringLiteral( "Noto Serif Hebrew" ),
    QStringLiteral( "Noto Serif JP" ),
    QStringLiteral( "Noto Serif KR" ),
    QStringLiteral( "Noto Serif Kannada" ),
    QStringLiteral( "Noto Serif Khitan Small Script" ),
    QStringLiteral( "Noto Serif Khmer" ),
    QStringLiteral( "Noto Serif Khojki" ),
    QStringLiteral( "Noto Serif Lao" ),
    QStringLiteral( "Noto Serif Makasar" ),
    QStringLiteral( "Noto Serif Malayalam" ),
    QStringLiteral( "Noto Serif Myanmar" ),
    QStringLiteral( "Noto Serif NP Hmong" ),
    QStringLiteral( "Noto Serif Oriya" ),
    QStringLiteral( "Noto Serif Ottoman Siyaq" ),
    QStringLiteral( "Noto Serif SC" ),
    QStringLiteral( "Noto Serif Sinhala" ),
    QStringLiteral( "Noto Serif TC" ),
    QStringLiteral( "Noto Serif Tamil" ),
    QStringLiteral( "Noto Serif Tangut" ),
    QStringLiteral( "Noto Serif Telugu" ),
    QStringLiteral( "Noto Serif Thai" ),
    QStringLiteral( "Noto Serif Tibetan" ),
    QStringLiteral( "Noto Serif Toto" ),
    QStringLiteral( "Noto Serif Vithkuqi" ),
    QStringLiteral( "Noto Serif Yezidi" ),
    QStringLiteral( "Noto Traditional Nushu" ),
    QStringLiteral( "Nova Cut" ),
    QStringLiteral( "Nova Flat" ),
    QStringLiteral( "Nova Mono" ),
    QStringLiteral( "Nova Oval" ),
    QStringLiteral( "Nova Round" ),
    QStringLiteral( "Nova Script" ),
    QStringLiteral( "Nova Slim" ),
    QStringLiteral( "Nova Square" ),
    QStringLiteral( "Numans" ),
    QStringLiteral( "Nunito" ),
    QStringLiteral( "Nunito Sans" ),
    QStringLiteral( "Nuosu SIL" ),
    QStringLiteral( "Odibee Sans" ),
    QStringLiteral( "Odor Mean Chey" ),
    QStringLiteral( "Offside" ),
    QStringLiteral( "Oi" ),
    QStringLiteral( "Old Standard TT" ),
    QStringLiteral( "Oldenburg" ),
    QStringLiteral( "Ole" ),
    QStringLiteral( "Oleo Script" ),
    QStringLiteral( "Oleo Script Swash Caps" ),
    QStringLiteral( "Oooh Baby" ),
    QStringLiteral( "Open Sans" ),
    QStringLiteral( "Open Sans Condensed" ),
    QStringLiteral( "Oranienbaum" ),
    QStringLiteral( "Orbit" ),
    QStringLiteral( "Orbitron" ),
    QStringLiteral( "Oregano" ),
    QStringLiteral( "Orelega One" ),
    QStringLiteral( "Orienta" ),
    QStringLiteral( "Original Surfer" ),
    QStringLiteral( "Oswald" ),
    QStringLiteral( "Outfit" ),
    QStringLiteral( "Over the Rainbow" ),
    QStringLiteral( "Overlock" ),
    QStringLiteral( "Overlock SC" ),
    QStringLiteral( "Overpass" ),
    QStringLiteral( "Overpass Mono" ),
    QStringLiteral( "Ovo" ),
    QStringLiteral( "Oxanium" ),
    QStringLiteral( "Oxygen" ),
    QStringLiteral( "Oxygen Mono" ),
    QStringLiteral( "PT Mono" ),
    QStringLiteral( "PT Sans" ),
    QStringLiteral( "PT Sans Caption" ),
    QStringLiteral( "PT Sans Narrow" ),
    QStringLiteral( "PT Serif" ),
    QStringLiteral( "PT Serif Caption" ),
    QStringLiteral( "Pacifico" ),
    QStringLiteral( "Padauk" ),
    QStringLiteral( "Padyakke Expanded One" ),
    QStringLiteral( "Palanquin" ),
    QStringLiteral( "Palanquin Dark" ),
    QStringLiteral( "Palette Mosaic" ),
    QStringLiteral( "Pangolin" ),
    QStringLiteral( "Paprika" ),
    QStringLiteral( "Parisienne" ),
    QStringLiteral( "Passero One" ),
    QStringLiteral( "Passion One" ),
    QStringLiteral( "Passions Conflict" ),
    QStringLiteral( "Pathway Extreme" ),
    QStringLiteral( "Pathway Gothic One" ),
    QStringLiteral( "Patrick Hand" ),
    QStringLiteral( "Patrick Hand SC" ),
    QStringLiteral( "Pattaya" ),
    QStringLiteral( "Patua One" ),
    QStringLiteral( "Pavanam" ),
    QStringLiteral( "Paytone One" ),
    QStringLiteral( "Peddana" ),
    QStringLiteral( "Peralta" ),
    QStringLiteral( "Permanent Marker" ),
    QStringLiteral( "Petemoss" ),
    QStringLiteral( "Petit Formal Script" ),
    QStringLiteral( "Petrona" ),
    QStringLiteral( "Philosopher" ),
    QStringLiteral( "Phudu" ),
    QStringLiteral( "Piazzolla" ),
    QStringLiteral( "Piedra" ),
    QStringLiteral( "Pinyon Script" ),
    QStringLiteral( "Pirata One" ),
    QStringLiteral( "Plaster" ),
    QStringLiteral( "Play" ),
    QStringLiteral( "Playball" ),
    QStringLiteral( "Playfair" ),
    QStringLiteral( "Playfair Display" ),
    QStringLiteral( "Playfair Display SC" ),
    QStringLiteral( "Plus Jakarta Sans" ),
    QStringLiteral( "Podkova" ),
    QStringLiteral( "Poiret One" ),
    QStringLiteral( "Poller One" ),
    QStringLiteral( "Poltawski Nowy" ),
    QStringLiteral( "Poly" ),
    QStringLiteral( "Pompiere" ),
    QStringLiteral( "Pontano Sans" ),
    QStringLiteral( "Poor Story" ),
    QStringLiteral( "Poppins" ),
    QStringLiteral( "Port Lligat Sans" ),
    QStringLiteral( "Port Lligat Slab" ),
    QStringLiteral( "Potta One" ),
    QStringLiteral( "Pragati Narrow" ),
    QStringLiteral( "Praise" ),
    QStringLiteral( "Prata" ),
    QStringLiteral( "Preahvihear" ),
    QStringLiteral( "Press Start 2P" ),
    QStringLiteral( "Pridi" ),
    QStringLiteral( "Princess Sofia" ),
    QStringLiteral( "Prociono" ),
    QStringLiteral( "Prompt" ),
    QStringLiteral( "Prosto One" ),
    QStringLiteral( "Proza Libre" ),
    QStringLiteral( "Public Sans" ),
    QStringLiteral( "Puppies Play" ),
    QStringLiteral( "Puritan" ),
    QStringLiteral( "Purple Purse" ),
    QStringLiteral( "Qahiri" ),
    QStringLiteral( "Quando" ),
    QStringLiteral( "Quantico" ),
    QStringLiteral( "Quattrocento" ),
    QStringLiteral( "Quattrocento Sans" ),
    QStringLiteral( "Questrial" ),
    QStringLiteral( "Quicksand" ),
    QStringLiteral( "Quintessential" ),
    QStringLiteral( "Qwigley" ),
    QStringLiteral( "Qwitcher Grypen" ),
    QStringLiteral( "REM" ),
    QStringLiteral( "Racing Sans One" ),
    QStringLiteral( "Radio Canada" ),
    QStringLiteral( "Radley" ),
    QStringLiteral( "Rajdhani" ),
    QStringLiteral( "Rakkas" ),
    QStringLiteral( "Raleway" ),
    QStringLiteral( "Raleway Dots" ),
    QStringLiteral( "Ramabhadra" ),
    QStringLiteral( "Ramaraja" ),
    QStringLiteral( "Rambla" ),
    QStringLiteral( "Rammetto One" ),
    QStringLiteral( "Rampart One" ),
    QStringLiteral( "Ranchers" ),
    QStringLiteral( "Rancho" ),
    QStringLiteral( "Ranga" ),
    QStringLiteral( "Rasa" ),
    QStringLiteral( "Rationale" ),
    QStringLiteral( "Ravi Prakash" ),
    QStringLiteral( "Readex Pro" ),
    QStringLiteral( "Recursive" ),
    QStringLiteral( "Red Hat Display" ),
    QStringLiteral( "Red Hat Mono" ),
    QStringLiteral( "Red Hat Text" ),
    QStringLiteral( "Red Rose" ),
    QStringLiteral( "Redacted" ),
    QStringLiteral( "Redacted Script" ),
    QStringLiteral( "Redressed" ),
    QStringLiteral( "Reem Kufi" ),
    QStringLiteral( "Reem Kufi Fun" ),
    QStringLiteral( "Reem Kufi Ink" ),
    QStringLiteral( "Reenie Beanie" ),
    QStringLiteral( "Reggae One" ),
    QStringLiteral( "Revalia" ),
    QStringLiteral( "Rhodium Libre" ),
    QStringLiteral( "Ribeye" ),
    QStringLiteral( "Ribeye Marrow" ),
    QStringLiteral( "Righteous" ),
    QStringLiteral( "Risque" ),
    QStringLiteral( "Road Rage" ),
    QStringLiteral( "Roboto" ),
    QStringLiteral( "Roboto Condensed" ),
    QStringLiteral( "Roboto Flex" ),
    QStringLiteral( "Roboto Mono" ),
    QStringLiteral( "Roboto Serif" ),
    QStringLiteral( "Roboto Slab" ),
    QStringLiteral( "Rochester" ),
    QStringLiteral( "Rock 3D" ),
    QStringLiteral( "Rock Salt" ),
    QStringLiteral( "RocknRoll One" ),
    QStringLiteral( "Rokkitt" ),
    QStringLiteral( "Romanesco" ),
    QStringLiteral( "Ropa Sans" ),
    QStringLiteral( "Rosario" ),
    QStringLiteral( "Rosarivo" ),
    QStringLiteral( "Rouge Script" ),
    QStringLiteral( "Rowdies" ),
    QStringLiteral( "Rozha One" ),
    QStringLiteral( "Rubik" ),
    QStringLiteral( "Rubik 80s Fade" ),
    QStringLiteral( "Rubik Beastly" ),
    QStringLiteral( "Rubik Bubbles" ),
    QStringLiteral( "Rubik Burned" ),
    QStringLiteral( "Rubik Dirt" ),
    QStringLiteral( "Rubik Distressed" ),
    QStringLiteral( "Rubik Gemstones" ),
    QStringLiteral( "Rubik Glitch" ),
    QStringLiteral( "Rubik Iso" ),
    QStringLiteral( "Rubik Marker Hatch" ),
    QStringLiteral( "Rubik Maze" ),
    QStringLiteral( "Rubik Microbe" ),
    QStringLiteral( "Rubik Mono One" ),
    QStringLiteral( "Rubik Moonrocks" ),
    QStringLiteral( "Rubik One" ),
    QStringLiteral( "Rubik Pixels" ),
    QStringLiteral( "Rubik Puddles" ),
    QStringLiteral( "Rubik Spray Paint" ),
    QStringLiteral( "Rubik Storm" ),
    QStringLiteral( "Rubik Vinyl" ),
    QStringLiteral( "Rubik Wet Paint" ),
    QStringLiteral( "Ruda" ),
    QStringLiteral( "Rufina" ),
    QStringLiteral( "Ruge Boogie" ),
    QStringLiteral( "Ruluko" ),
    QStringLiteral( "Rum Raisin" ),
    QStringLiteral( "Ruslan Display" ),
    QStringLiteral( "Russo One" ),
    QStringLiteral( "Ruthie" ),
    QStringLiteral( "Ruwudu" ),
    QStringLiteral( "Rye" ),
    QStringLiteral( "STIX Two Text" ),
    QStringLiteral( "Sacramento" ),
    QStringLiteral( "Sahitya" ),
    QStringLiteral( "Sail" ),
    QStringLiteral( "Saira" ),
    QStringLiteral( "Saira Condensed" ),
    QStringLiteral( "Saira Extra Condensed" ),
    QStringLiteral( "Saira Semi Condensed" ),
    QStringLiteral( "Saira Stencil One" ),
    QStringLiteral( "Salsa" ),
    QStringLiteral( "Sanchez" ),
    QStringLiteral( "Sancreek" ),
    QStringLiteral( "Sansita" ),
    QStringLiteral( "Sansita One" ),
    QStringLiteral( "Sansita Swashed" ),
    QStringLiteral( "Sarabun" ),
    QStringLiteral( "Sarala" ),
    QStringLiteral( "Sarina" ),
    QStringLiteral( "Sarpanch" ),
    QStringLiteral( "Sassy Frass" ),
    QStringLiteral( "Satisfy" ),
    QStringLiteral( "Sawarabi Gothic" ),
    QStringLiteral( "Sawarabi Mincho" ),
    QStringLiteral( "Scada" ),
    QStringLiteral( "Scheherazade" ),
    QStringLiteral( "Scheherazade New" ),
    QStringLiteral( "Schibsted Grotesk" ),
    QStringLiteral( "Schoolbell" ),
    QStringLiteral( "Scope One" ),
    QStringLiteral( "Seaweed Script" ),
    QStringLiteral( "Secular One" ),
    QStringLiteral( "Sedgwick Ave" ),
    QStringLiteral( "Sedgwick Ave Display" ),
    QStringLiteral( "Sen" ),
    QStringLiteral( "Send Flowers" ),
    QStringLiteral( "Sevillana" ),
    QStringLiteral( "Seymour One" ),
    QStringLiteral( "Shadows Into Light" ),
    QStringLiteral( "Shadows Into Light Two" ),
    QStringLiteral( "Shalimar" ),
    QStringLiteral( "Shantell Sans" ),
    QStringLiteral( "Shanti" ),
    QStringLiteral( "Share" ),
    QStringLiteral( "Share Tech" ),
    QStringLiteral( "Share Tech Mono" ),
    QStringLiteral( "Shippori Antique" ),
    QStringLiteral( "Shippori Antique B1" ),
    QStringLiteral( "Shippori Mincho" ),
    QStringLiteral( "Shippori Mincho B1" ),
    QStringLiteral( "Shizuru" ),
    QStringLiteral( "Shojumaru" ),
    QStringLiteral( "Short Stack" ),
    QStringLiteral( "Shrikhand" ),
    QStringLiteral( "Siemreap" ),
    QStringLiteral( "Sigmar" ),
    QStringLiteral( "Sigmar One" ),
    QStringLiteral( "Signika" ),
    QStringLiteral( "Signika Negative" ),
    QStringLiteral( "Silkscreen" ),
    QStringLiteral( "Simonetta" ),
    QStringLiteral( "Single Day" ),
    QStringLiteral( "Sintony" ),
    QStringLiteral( "Sirin Stencil" ),
    QStringLiteral( "Six Caps" ),
    QStringLiteral( "Skranji" ),
    QStringLiteral( "Slabo 13px" ),
    QStringLiteral( "Slabo 27px" ),
    QStringLiteral( "Slackey" ),
    QStringLiteral( "Slackside One" ),
    QStringLiteral( "Smokum" ),
    QStringLiteral( "Smooch" ),
    QStringLiteral( "Smooch Sans" ),
    QStringLiteral( "Smythe" ),
    QStringLiteral( "Sniglet" ),
    QStringLiteral( "Snippet" ),
    QStringLiteral( "Snowburst One" ),
    QStringLiteral( "Sofadi One" ),
    QStringLiteral( "Sofia" ),
    QStringLiteral( "Sofia Sans" ),
    QStringLiteral( "Sofia Sans Condensed" ),
    QStringLiteral( "Sofia Sans Extra Condensed" ),
    QStringLiteral( "Sofia Sans Semi Condensed" ),
    QStringLiteral( "Solitreo" ),
    QStringLiteral( "Solway" ),
    QStringLiteral( "Song Myung" ),
    QStringLiteral( "Sono" ),
    QStringLiteral( "Sonsie One" ),
    QStringLiteral( "Sora" ),
    QStringLiteral( "Sorts Mill Goudy" ),
    QStringLiteral( "Source Code Pro" ),
    QStringLiteral( "Source Sans 3" ),
    QStringLiteral( "Source Sans Pro" ),
    QStringLiteral( "Source Serif 4" ),
    QStringLiteral( "Source Serif Pro" ),
    QStringLiteral( "Space Grotesk" ),
    QStringLiteral( "Space Mono" ),
    QStringLiteral( "Special Elite" ),
    QStringLiteral( "Spectral" ),
    QStringLiteral( "Spectral SC" ),
    QStringLiteral( "Spicy Rice" ),
    QStringLiteral( "Spinnaker" ),
    QStringLiteral( "Spirax" ),
    QStringLiteral( "Splash" ),
    QStringLiteral( "Spline Sans" ),
    QStringLiteral( "Spline Sans Mono" ),
    QStringLiteral( "Squada One" ),
    QStringLiteral( "Square Peg" ),
    QStringLiteral( "Sree Krushnadevaraya" ),
    QStringLiteral( "Sriracha" ),
    QStringLiteral( "Srisakdi" ),
    QStringLiteral( "Staatliches" ),
    QStringLiteral( "Stalemate" ),
    QStringLiteral( "Stalinist One" ),
    QStringLiteral( "Stardos Stencil" ),
    QStringLiteral( "Stick" ),
    QStringLiteral( "Stick No Bills" ),
    QStringLiteral( "Stint Ultra Condensed" ),
    QStringLiteral( "Stint Ultra Expanded" ),
    QStringLiteral( "Stoke" ),
    QStringLiteral( "Strait" ),
    QStringLiteral( "Style Script" ),
    QStringLiteral( "Stylish" ),
    QStringLiteral( "Sue Ellen Francisco" ),
    QStringLiteral( "Suez One" ),
    QStringLiteral( "Sulphur Point" ),
    QStringLiteral( "Sumana" ),
    QStringLiteral( "Sunflower" ),
    QStringLiteral( "Sunshiney" ),
    QStringLiteral( "Supermercado One" ),
    QStringLiteral( "Sura" ),
    QStringLiteral( "Suranna" ),
    QStringLiteral( "Suravaram" ),
    QStringLiteral( "Suwannaphum" ),
    QStringLiteral( "Swanky and Moo Moo" ),
    QStringLiteral( "Syncopate" ),
    QStringLiteral( "Syne" ),
    QStringLiteral( "Syne Mono" ),
    QStringLiteral( "Syne Tactile" ),
    QStringLiteral( "Tai Heritage Pro" ),
    QStringLiteral( "Tajawal" ),
    QStringLiteral( "Tangerine" ),
    QStringLiteral( "Tapestry" ),
    QStringLiteral( "Taprom" ),
    QStringLiteral( "Tauri" ),
    QStringLiteral( "Taviraj" ),
    QStringLiteral( "Teko" ),
    QStringLiteral( "Tektur" ),
    QStringLiteral( "Telex" ),
    QStringLiteral( "Tenali Ramakrishna" ),
    QStringLiteral( "Tenor Sans" ),
    QStringLiteral( "Text Me One" ),
    QStringLiteral( "Texturina" ),
    QStringLiteral( "Thasadith" ),
    QStringLiteral( "The Girl Next Door" ),
    QStringLiteral( "The Nautigal" ),
    QStringLiteral( "Tienne" ),
    QStringLiteral( "Tillana" ),
    QStringLiteral( "Tilt Neon" ),
    QStringLiteral( "Tilt Prism" ),
    QStringLiteral( "Tilt Warp" ),
    QStringLiteral( "Timmana" ),
    QStringLiteral( "Tinos" ),
    QStringLiteral( "Tiro Bangla" ),
    QStringLiteral( "Tiro Devanagari Hindi" ),
    QStringLiteral( "Tiro Devanagari Marathi" ),
    QStringLiteral( "Tiro Devanagari Sanskrit" ),
    QStringLiteral( "Tiro Gurmukhi" ),
    QStringLiteral( "Tiro Kannada" ),
    QStringLiteral( "Tiro Tamil" ),
    QStringLiteral( "Tiro Telugu" ),
    QStringLiteral( "Titan One" ),
    QStringLiteral( "Titillium Web" ),
    QStringLiteral( "Tomorrow" ),
    QStringLiteral( "Tourney" ),
    QStringLiteral( "Trade Winds" ),
    QStringLiteral( "Train One" ),
    QStringLiteral( "Trirong" ),
    QStringLiteral( "Trispace" ),
    QStringLiteral( "Trocchi" ),
    QStringLiteral( "Trochut" ),
    QStringLiteral( "Truculenta" ),
    QStringLiteral( "Trykker" ),
    QStringLiteral( "Tsukimi Rounded" ),
    QStringLiteral( "Tulpen One" ),
    QStringLiteral( "Turret Road" ),
    QStringLiteral( "Twinkle Star" ),
    QStringLiteral( "Ubuntu" ),
    QStringLiteral( "Ubuntu Condensed" ),
    QStringLiteral( "Ubuntu Mono" ),
    QStringLiteral( "Uchen" ),
    QStringLiteral( "Ultra" ),
    QStringLiteral( "Unbounded" ),
    QStringLiteral( "Uncial Antiqua" ),
    QStringLiteral( "Underdog" ),
    QStringLiteral( "Unica One" ),
    QStringLiteral( "UnifrakturCook" ),
    QStringLiteral( "UnifrakturMaguntia" ),
    QStringLiteral( "Unkempt" ),
    QStringLiteral( "Unlock" ),
    QStringLiteral( "Unna" ),
    QStringLiteral( "Updock" ),
    QStringLiteral( "Urbanist" ),
    QStringLiteral( "VT323" ),
    QStringLiteral( "Vampiro One" ),
    QStringLiteral( "Varela" ),
    QStringLiteral( "Varela Round" ),
    QStringLiteral( "Varta" ),
    QStringLiteral( "Vast Shadow" ),
    QStringLiteral( "Vazirmatn" ),
    QStringLiteral( "Vesper Libre" ),
    QStringLiteral( "Viaoda Libre" ),
    QStringLiteral( "Vibes" ),
    QStringLiteral( "Vibur" ),
    QStringLiteral( "Victor Mono" ),
    QStringLiteral( "Vidaloka" ),
    QStringLiteral( "Viga" ),
    QStringLiteral( "Vina Sans" ),
    QStringLiteral( "Voces" ),
    QStringLiteral( "Volkhov" ),
    QStringLiteral( "Vollkorn" ),
    QStringLiteral( "Vollkorn SC" ),
    QStringLiteral( "Voltaire" ),
    QStringLiteral( "Vujahday Script" ),
    QStringLiteral( "Waiting for the Sunrise" ),
    QStringLiteral( "Wallpoet" ),
    QStringLiteral( "Walter Turncoat" ),
    QStringLiteral( "Warnes" ),
    QStringLiteral( "Water Brush" ),
    QStringLiteral( "Waterfall" ),
    QStringLiteral( "Wavefont" ),
    QStringLiteral( "Wellfleet" ),
    QStringLiteral( "Wendy One" ),
    QStringLiteral( "Whisper" ),
    QStringLiteral( "WindSong" ),
    QStringLiteral( "Wire One" ),
    QStringLiteral( "Wix Madefor Display" ),
    QStringLiteral( "Wix Madefor Text" ),
    QStringLiteral( "Work Sans" ),
    QStringLiteral( "Xanh Mono" ),
    QStringLiteral( "Yaldevi" ),
    QStringLiteral( "Yanone Kaffeesatz" ),
    QStringLiteral( "Yantramanav" ),
    QStringLiteral( "Yatra One" ),
    QStringLiteral( "Yellowtail" ),
    QStringLiteral( "Yeon Sung" ),
    QStringLiteral( "Yeseva One" ),
    QStringLiteral( "Yesteryear" ),
    QStringLiteral( "Yomogi" ),
    QStringLiteral( "Yrsa" ),
    QStringLiteral( "Ysabeau" ),
    QStringLiteral( "Ysabeau Infant" ),
    QStringLiteral( "Ysabeau Office" ),
    QStringLiteral( "Ysabeau SC" ),
    QStringLiteral( "Yuji Boku" ),
    QStringLiteral( "Yuji Hentaigana Akari" ),
    QStringLiteral( "Yuji Hentaigana Akebono" ),
    QStringLiteral( "Yuji Mai" ),
    QStringLiteral( "Yuji Syuku" ),
    QStringLiteral( "Yusei Magic" ),
    QStringLiteral( "ZCOOL KuaiLe" ),
    QStringLiteral( "ZCOOL QingKe HuangYou" ),
    QStringLiteral( "ZCOOL XiaoWei" ),
    QStringLiteral( "Zen Antique" ),
    QStringLiteral( "Zen Antique Soft" ),
    QStringLiteral( "Zen Dots" ),
    QStringLiteral( "Zen Kaku Gothic Antique" ),
    QStringLiteral( "Zen Kaku Gothic New" ),
    QStringLiteral( "Zen Kurenaido" ),
    QStringLiteral( "Zen Loop" ),
    QStringLiteral( "Zen Maru Gothic" ),
    QStringLiteral( "Zen Old Mincho" ),
    QStringLiteral( "Zen Tokyo Zoo" ),
    QStringLiteral( "Zeyada" ),
    QStringLiteral( "Zhi Mang Xing" ),
    QStringLiteral( "Zilla Slab" ),
    QStringLiteral( "Zilla Slab Highlight" ),
  };

  auto cleanFontFamily = []( const QString & family ) -> QString
  {
    const thread_local QRegularExpression charsToRemove( QStringLiteral( "[^a-z]" ) );
    const thread_local QRegularExpression styleNames( QStringLiteral( "(?:normal|regular|light|bold|black|demi|italic|oblique|medium|thin)" ) );

    QString processed = family.toLower();
    processed.replace( styleNames, QString() );
    return processed.replace( charsToRemove, QString() );
  };

  matchedFamily.clear();
  const QString cleanedFamily = cleanFontFamily( family );
  for ( const QString &candidate : sGoogleFonts )
  {
    if ( cleanFontFamily( candidate ) == cleanedFamily )
    {
      QString paramName = candidate;
      paramName.replace( ' ', '+' );
      matchedFamily = candidate;
      return QStringLiteral( "https://fonts.google.com/download?family=%1" ).arg( paramName );
    }
  }
  return QString();
}

void QgsFontManager::downloadAndInstallFont( const QUrl &url, const QString &identifier )
{
  QString description;
  if ( identifier.isEmpty() )
  {
    description = tr( "Installing fonts from %1" ).arg( url.toString() );
  }
  else
  {
    description = tr( "Installing %1" ).arg( identifier );
  }

  QgsNetworkContentFetcherTask *task = new QgsNetworkContentFetcherTask( url, QString(), QgsTask::CanCancel, description );
  connect( task, &QgsNetworkContentFetcherTask::fetched, this, [this, task, url, identifier]
  {
    if ( task->reply()->error() != QNetworkReply::NoError )
    {
      emit fontDownloadErrorOccurred( url, identifier, task->reply()->errorString() );
    }
    else
    {
      QString errorMessage;
      QStringList families;
      QString licenseDetails;
      if ( installFontsFromData( task->reply()->readAll(), errorMessage, families, licenseDetails, task->contentDispositionFilename() ) )
      {
        QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
        mPendingFontDownloads.remove( identifier );
        locker.unlock();

        emit fontDownloaded( families, licenseDetails );
      }
      else
      {
        QgsReadWriteLocker locker( mReplacementLock, QgsReadWriteLocker::Write );
        mPendingFontDownloads.remove( identifier );
        locker.unlock();

        emit fontDownloadErrorOccurred( url, identifier, errorMessage );
      }
    }
  } );
  QgsApplication::taskManager()->addTask( task );
}

bool QgsFontManager::installFontsFromData( const QByteArray &data, QString &errorMessage, QStringList &families, QString &licenseDetails, const QString &filename )
{
  errorMessage.clear();
  families.clear();
  licenseDetails.clear();

  QTemporaryFile tempFile;
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
  int id = QFontDatabase::addApplicationFont( sourcePath );
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
    const QString destPath = fontsDir.filePath( filename.isEmpty() ? family : filename );
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
