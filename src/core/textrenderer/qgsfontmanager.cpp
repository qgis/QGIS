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
#include "qgsfontutils.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QDir>
#include <QFontDatabase>
#include <QTemporaryFile>
#include <QTemporaryDir>

QgsFontManager::QgsFontManager( QObject *parent )
  : QObject( parent )
{
  const QStringList replacements = settingsFontFamilyReplacements.value();
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
  settingsFontFamilyReplacements.setValue( replacements );
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
      QgsDebugMsg( QStringLiteral( "Cannot create local fonts dir: %1" ).arg( dir ) );
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
      QgsDebugMsg( QStringLiteral( "The user font %1 could not be installed" ).arg( infoIt->filePath() ) );
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
  if ( !settingsDownloadMissingFonts.value() )
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
    QStringLiteral( "Abel" ),
    QStringLiteral( "Abril Fatface" ),
    QStringLiteral( "Aclonica" ),
    QStringLiteral( "Acme" ),
    QStringLiteral( "Actor" ),
    QStringLiteral( "Adamina" ),
    QStringLiteral( "Advent Pro" ),
    QStringLiteral( "Aguafina Script" ),
    QStringLiteral( "Akronim" ),
    QStringLiteral( "Aladin" ),
    QStringLiteral( "Aldrich" ),
    QStringLiteral( "Alef" ),
    QStringLiteral( "Alegreya" ),
    QStringLiteral( "Alegreya SC" ),
    QStringLiteral( "Alegreya Sans" ),
    QStringLiteral( "Alegreya Sans SC" ),
    QStringLiteral( "Alex Brush" ),
    QStringLiteral( "Alfa Slab One" ),
    QStringLiteral( "Alice" ),
    QStringLiteral( "Alike" ),
    QStringLiteral( "Alike Angular" ),
    QStringLiteral( "Allan" ),
    QStringLiteral( "Allerta" ),
    QStringLiteral( "Allerta Stencil" ),
    QStringLiteral( "Allura" ),
    QStringLiteral( "Almendra" ),
    QStringLiteral( "Almendra Display" ),
    QStringLiteral( "Almendra SC" ),
    QStringLiteral( "Amarante" ),
    QStringLiteral( "Amaranth" ),
    QStringLiteral( "Amatic SC" ),
    QStringLiteral( "Amethysta" ),
    QStringLiteral( "Amiri" ),
    QStringLiteral( "Anaheim" ),
    QStringLiteral( "Andada" ),
    QStringLiteral( "Andika" ),
    QStringLiteral( "Angkor" ),
    QStringLiteral( "Annie Use Your Telescope" ),
    QStringLiteral( "Anonymous Pro" ),
    QStringLiteral( "Antic" ),
    QStringLiteral( "Antic Didone" ),
    QStringLiteral( "Antic Slab" ),
    QStringLiteral( "Anton" ),
    QStringLiteral( "Arapey" ),
    QStringLiteral( "Arbutus" ),
    QStringLiteral( "Arbutus Slab" ),
    QStringLiteral( "Architects Daughter" ),
    QStringLiteral( "Archivo Black" ),
    QStringLiteral( "Archivo Narrow" ),
    QStringLiteral( "Arimo" ),
    QStringLiteral( "Arizonia" ),
    QStringLiteral( "Armata" ),
    QStringLiteral( "Artifika" ),
    QStringLiteral( "Arvo" ),
    QStringLiteral( "Asap" ),
    QStringLiteral( "Asset" ),
    QStringLiteral( "Astloch" ),
    QStringLiteral( "Asul" ),
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
    QStringLiteral( "Bad Script" ),
    QStringLiteral( "Balthazar" ),
    QStringLiteral( "Bangers" ),
    QStringLiteral( "Barlow" ),
    QStringLiteral( "Basic" ),
    QStringLiteral( "Battambang" ),
    QStringLiteral( "Baumans" ),
    QStringLiteral( "Bayon" ),
    QStringLiteral( "Belgrano" ),
    QStringLiteral( "Belleza" ),
    QStringLiteral( "BenchNine" ),
    QStringLiteral( "Bentham" ),
    QStringLiteral( "Berkshire Swash" ),
    QStringLiteral( "Bevan" ),
    QStringLiteral( "Bigelow Rules" ),
    QStringLiteral( "Bigshot One" ),
    QStringLiteral( "Bilbo" ),
    QStringLiteral( "Bilbo Swash Caps" ),
    QStringLiteral( "Biryani" ),
    QStringLiteral( "Bitter" ),
    QStringLiteral( "Black Ops One" ),
    QStringLiteral( "Bokor" ),
    QStringLiteral( "Bonbon" ),
    QStringLiteral( "Boogaloo" ),
    QStringLiteral( "Bowlby One" ),
    QStringLiteral( "Bowlby One SC" ),
    QStringLiteral( "Brawler" ),
    QStringLiteral( "Bree Serif" ),
    QStringLiteral( "Bubblegum Sans" ),
    QStringLiteral( "Bubbler One" ),
    QStringLiteral( "Buda" ),
    QStringLiteral( "Buenard" ),
    QStringLiteral( "Butcherman" ),
    QStringLiteral( "Butterfly Kids" ),
    QStringLiteral( "Cabin" ),
    QStringLiteral( "Cabin Condensed" ),
    QStringLiteral( "Cabin Sketch" ),
    QStringLiteral( "Caesar Dressing" ),
    QStringLiteral( "Cagliostro" ),
    QStringLiteral( "Calligraffitti" ),
    QStringLiteral( "Cambay" ),
    QStringLiteral( "Cambo" ),
    QStringLiteral( "Candal" ),
    QStringLiteral( "Cantarell" ),
    QStringLiteral( "Cantata One" ),
    QStringLiteral( "Cantora One" ),
    QStringLiteral( "Capriola" ),
    QStringLiteral( "Cardo" ),
    QStringLiteral( "Carme" ),
    QStringLiteral( "Carrois Gothic" ),
    QStringLiteral( "Carrois Gothic SC" ),
    QStringLiteral( "Carter One" ),
    QStringLiteral( "Caudex" ),
    QStringLiteral( "Cedarville Cursive" ),
    QStringLiteral( "Ceviche One" ),
    QStringLiteral( "Changa One" ),
    QStringLiteral( "Chango" ),
    QStringLiteral( "Chau Philomene One" ),
    QStringLiteral( "Chela One" ),
    QStringLiteral( "Chelsea Market" ),
    QStringLiteral( "Chenla" ),
    QStringLiteral( "Cherry Cream Soda" ),
    QStringLiteral( "Cherry Swash" ),
    QStringLiteral( "Chewy" ),
    QStringLiteral( "Chicle" ),
    QStringLiteral( "Chivo" ),
    QStringLiteral( "Cinzel" ),
    QStringLiteral( "Cinzel Decorative" ),
    QStringLiteral( "Clicker Script" ),
    QStringLiteral( "Coda" ),
    QStringLiteral( "Coda Caption" ),
    QStringLiteral( "Codystar" ),
    QStringLiteral( "Combo" ),
    QStringLiteral( "Comfortaa" ),
    QStringLiteral( "Coming Soon" ),
    QStringLiteral( "Concert One" ),
    QStringLiteral( "Condiment" ),
    QStringLiteral( "Content" ),
    QStringLiteral( "Contrail One" ),
    QStringLiteral( "Convergence" ),
    QStringLiteral( "Cookie" ),
    QStringLiteral( "Copse" ),
    QStringLiteral( "Corben" ),
    QStringLiteral( "Courgette" ),
    QStringLiteral( "Cousine" ),
    QStringLiteral( "Coustard" ),
    QStringLiteral( "Covered By Your Grace" ),
    QStringLiteral( "Crafty Girls" ),
    QStringLiteral( "Creepster" ),
    QStringLiteral( "Crete Round" ),
    QStringLiteral( "Crimson Text" ),
    QStringLiteral( "Croissant One" ),
    QStringLiteral( "Crushed" ),
    QStringLiteral( "Cuprum" ),
    QStringLiteral( "Cutive" ),
    QStringLiteral( "Cutive Mono" ),
    QStringLiteral( "Damion" ),
    QStringLiteral( "Dancing Script" ),
    QStringLiteral( "Dangrek" ),
    QStringLiteral( "Dawning of a New Day" ),
    QStringLiteral( "Days One" ),
    QStringLiteral( "Dekko" ),
    QStringLiteral( "Delius" ),
    QStringLiteral( "Delius Swash Caps" ),
    QStringLiteral( "Delius Unicase" ),
    QStringLiteral( "Della Respira" ),
    QStringLiteral( "Denk One" ),
    QStringLiteral( "Devonshire" ),
    QStringLiteral( "Dhurjati" ),
    QStringLiteral( "Didact Gothic" ),
    QStringLiteral( "Diplomata" ),
    QStringLiteral( "Diplomata SC" ),
    QStringLiteral( "Domine" ),
    QStringLiteral( "Donegal One" ),
    QStringLiteral( "Doppio One" ),
    QStringLiteral( "Dorsa" ),
    QStringLiteral( "Dosis" ),
    QStringLiteral( "Dr Sugiyama" ),
    QStringLiteral( "Droid Sans" ),
    QStringLiteral( "Droid Sans Mono" ),
    QStringLiteral( "Droid Serif" ),
    QStringLiteral( "Duru Sans" ),
    QStringLiteral( "Dynalight" ),
    QStringLiteral( "EB Garamond" ),
    QStringLiteral( "Eagle Lake" ),
    QStringLiteral( "Eater" ),
    QStringLiteral( "Economica" ),
    QStringLiteral( "Ek Mukta" ),
    QStringLiteral( "Electrolize" ),
    QStringLiteral( "Elsie" ),
    QStringLiteral( "Elsie Swash Caps" ),
    QStringLiteral( "Emblema One" ),
    QStringLiteral( "Emilys Candy" ),
    QStringLiteral( "Engagement" ),
    QStringLiteral( "Englebert" ),
    QStringLiteral( "Enriqueta" ),
    QStringLiteral( "Erica One" ),
    QStringLiteral( "Esteban" ),
    QStringLiteral( "Euphoria Script" ),
    QStringLiteral( "Ewert" ),
    QStringLiteral( "Exo" ),
    QStringLiteral( "Exo 2" ),
    QStringLiteral( "Expletus Sans" ),
    QStringLiteral( "Fanwood Text" ),
    QStringLiteral( "Fascinate" ),
    QStringLiteral( "Fascinate Inline" ),
    QStringLiteral( "Faster One" ),
    QStringLiteral( "Fasthand" ),
    QStringLiteral( "Fauna One" ),
    QStringLiteral( "Federant" ),
    QStringLiteral( "Federo" ),
    QStringLiteral( "Felipa" ),
    QStringLiteral( "Fenix" ),
    QStringLiteral( "Finger Paint" ),
    QStringLiteral( "Fira Mono" ),
    QStringLiteral( "Fira Sans" ),
    QStringLiteral( "Fjalla One" ),
    QStringLiteral( "Fjord One" ),
    QStringLiteral( "Flamenco" ),
    QStringLiteral( "Flavors" ),
    QStringLiteral( "Fondamento" ),
    QStringLiteral( "Fontdiner Swanky" ),
    QStringLiteral( "Forum" ),
    QStringLiteral( "Francois One" ),
    QStringLiteral( "Freckle Face" ),
    QStringLiteral( "Fredericka the Great" ),
    QStringLiteral( "Fredoka One" ),
    QStringLiteral( "Freehand" ),
    QStringLiteral( "Fresca" ),
    QStringLiteral( "Frijole" ),
    QStringLiteral( "Fruktur" ),
    QStringLiteral( "Fugaz One" ),
    QStringLiteral( "GFS Didot" ),
    QStringLiteral( "GFS Neohellenic" ),
    QStringLiteral( "Gabriela" ),
    QStringLiteral( "Gafata" ),
    QStringLiteral( "Galdeano" ),
    QStringLiteral( "Galindo" ),
    QStringLiteral( "Gentium Basic" ),
    QStringLiteral( "Gentium Book Basic" ),
    QStringLiteral( "Geo" ),
    QStringLiteral( "Geostar" ),
    QStringLiteral( "Geostar Fill" ),
    QStringLiteral( "Germania One" ),
    QStringLiteral( "Gidugu" ),
    QStringLiteral( "Gilda Display" ),
    QStringLiteral( "Give You Glory" ),
    QStringLiteral( "Glass Antiqua" ),
    QStringLiteral( "Glegoo" ),
    QStringLiteral( "Gloria Hallelujah" ),
    QStringLiteral( "Goblin One" ),
    QStringLiteral( "Gochi Hand" ),
    QStringLiteral( "Gorditas" ),
    QStringLiteral( "Goudy Bookletter 1911" ),
    QStringLiteral( "Graduate" ),
    QStringLiteral( "Grand Hotel" ),
    QStringLiteral( "Gravitas One" ),
    QStringLiteral( "Great Vibes" ),
    QStringLiteral( "Griffy" ),
    QStringLiteral( "Gruppo" ),
    QStringLiteral( "Gudea" ),
    QStringLiteral( "Gurajada" ),
    QStringLiteral( "Habibi" ),
    QStringLiteral( "Halant" ),
    QStringLiteral( "Hammersmith One" ),
    QStringLiteral( "Hanalei" ),
    QStringLiteral( "Hanalei Fill" ),
    QStringLiteral( "Handlee" ),
    QStringLiteral( "Hanuman" ),
    QStringLiteral( "Happy Monkey" ),
    QStringLiteral( "Headland One" ),
    QStringLiteral( "Henny Penny" ),
    QStringLiteral( "Herr Von Muellerhoff" ),
    QStringLiteral( "Hind" ),
    QStringLiteral( "Holtwood One SC" ),
    QStringLiteral( "Homemade Apple" ),
    QStringLiteral( "Homenaje" ),
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
    QStringLiteral( "Iceberg" ),
    QStringLiteral( "Iceland" ),
    QStringLiteral( "Imprima" ),
    QStringLiteral( "Inconsolata" ),
    QStringLiteral( "Inder" ),
    QStringLiteral( "Indie Flower" ),
    QStringLiteral( "Inika" ),
    QStringLiteral( "Irish Grover" ),
    QStringLiteral( "Istok Web" ),
    QStringLiteral( "Italiana" ),
    QStringLiteral( "Italianno" ),
    QStringLiteral( "Jacques Francois" ),
    QStringLiteral( "Jacques Francois Shadow" ),
    QStringLiteral( "Jaldi" ),
    QStringLiteral( "Jim Nightshade" ),
    QStringLiteral( "Jockey One" ),
    QStringLiteral( "Jolly Lodger" ),
    QStringLiteral( "Josefin Sans" ),
    QStringLiteral( "Josefin Slab" ),
    QStringLiteral( "Joti One" ),
    QStringLiteral( "Judson" ),
    QStringLiteral( "Julee" ),
    QStringLiteral( "Julius Sans One" ),
    QStringLiteral( "Junge" ),
    QStringLiteral( "Jura" ),
    QStringLiteral( "Just Another Hand" ),
    QStringLiteral( "Just Me Again Down Here" ),
    QStringLiteral( "Kalam" ),
    QStringLiteral( "Kameron" ),
    QStringLiteral( "Kantumruy" ),
    QStringLiteral( "Karla" ),
    QStringLiteral( "Karma" ),
    QStringLiteral( "Kaushan Script" ),
    QStringLiteral( "Kavoon" ),
    QStringLiteral( "Kdam Thmor" ),
    QStringLiteral( "Keania One" ),
    QStringLiteral( "Kelly Slab" ),
    QStringLiteral( "Kenia" ),
    QStringLiteral( "Khand" ),
    QStringLiteral( "Khmer" ),
    QStringLiteral( "Khula" ),
    QStringLiteral( "Kite One" ),
    QStringLiteral( "Knewave" ),
    QStringLiteral( "Kotta One" ),
    QStringLiteral( "Koulen" ),
    QStringLiteral( "Kranky" ),
    QStringLiteral( "Kreon" ),
    QStringLiteral( "Kristi" ),
    QStringLiteral( "Krona One" ),
    QStringLiteral( "La Belle Aurore" ),
    QStringLiteral( "Laila" ),
    QStringLiteral( "Lakki Reddy" ),
    QStringLiteral( "Lancelot" ),
    QStringLiteral( "Lateef" ),
    QStringLiteral( "Lato" ),
    QStringLiteral( "League Script" ),
    QStringLiteral( "Leckerli One" ),
    QStringLiteral( "Ledger" ),
    QStringLiteral( "Lekton" ),
    QStringLiteral( "Lemon" ),
    QStringLiteral( "Libre Baskerville" ),
    QStringLiteral( "Life Savers" ),
    QStringLiteral( "Lilita One" ),
    QStringLiteral( "Lily Script One" ),
    QStringLiteral( "Limelight" ),
    QStringLiteral( "Linden Hill" ),
    QStringLiteral( "Lobster" ),
    QStringLiteral( "Lobster Two" ),
    QStringLiteral( "Londrina Outline" ),
    QStringLiteral( "Londrina Shadow" ),
    QStringLiteral( "Londrina Sketch" ),
    QStringLiteral( "Londrina Solid" ),
    QStringLiteral( "Lora" ),
    QStringLiteral( "Love Ya Like A Sister" ),
    QStringLiteral( "Loved by the King" ),
    QStringLiteral( "Lovers Quarrel" ),
    QStringLiteral( "Luckiest Guy" ),
    QStringLiteral( "Lusitana" ),
    QStringLiteral( "Lustria" ),
    QStringLiteral( "Macondo" ),
    QStringLiteral( "Macondo Swash Caps" ),
    QStringLiteral( "Magra" ),
    QStringLiteral( "Maiden Orange" ),
    QStringLiteral( "Mako" ),
    QStringLiteral( "Mallanna" ),
    QStringLiteral( "Mandali" ),
    QStringLiteral( "Marcellus" ),
    QStringLiteral( "Marcellus SC" ),
    QStringLiteral( "Marck Script" ),
    QStringLiteral( "Margarine" ),
    QStringLiteral( "Marko One" ),
    QStringLiteral( "Marmelad" ),
    QStringLiteral( "Martel" ),
    QStringLiteral( "Martel Sans" ),
    QStringLiteral( "Marvel" ),
    QStringLiteral( "Mate" ),
    QStringLiteral( "Mate SC" ),
    QStringLiteral( "Maven Pro" ),
    QStringLiteral( "McLaren" ),
    QStringLiteral( "Meddon" ),
    QStringLiteral( "MedievalSharp" ),
    QStringLiteral( "Medula One" ),
    QStringLiteral( "Megrim" ),
    QStringLiteral( "Meie Script" ),
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
    QStringLiteral( "Miniver" ),
    QStringLiteral( "Miss Fajardose" ),
    QStringLiteral( "Modak" ),
    QStringLiteral( "Modern Antiqua" ),
    QStringLiteral( "Molengo" ),
    QStringLiteral( "Molle" ),
    QStringLiteral( "Monda" ),
    QStringLiteral( "Monofett" ),
    QStringLiteral( "Monoton" ),
    QStringLiteral( "Monsieur La Doulaise" ),
    QStringLiteral( "Montaga" ),
    QStringLiteral( "Montez" ),
    QStringLiteral( "Montserrat" ),
    QStringLiteral( "Montserrat Alternates" ),
    QStringLiteral( "Montserrat Subrayada" ),
    QStringLiteral( "Moul" ),
    QStringLiteral( "Moulpali" ),
    QStringLiteral( "Mountains of Christmas" ),
    QStringLiteral( "Mouse Memoirs" ),
    QStringLiteral( "Mr Bedfort" ),
    QStringLiteral( "Mr Dafoe" ),
    QStringLiteral( "Mr De Haviland" ),
    QStringLiteral( "Mrs Saint Delafield" ),
    QStringLiteral( "Mrs Sheppards" ),
    QStringLiteral( "Muli" ),
    QStringLiteral( "Mystery Quest" ),
    QStringLiteral( "NTR" ),
    QStringLiteral( "Neucha" ),
    QStringLiteral( "Neuton" ),
    QStringLiteral( "New Rocker" ),
    QStringLiteral( "News Cycle" ),
    QStringLiteral( "Niconne" ),
    QStringLiteral( "Nixie One" ),
    QStringLiteral( "Nobile" ),
    QStringLiteral( "Nokora" ),
    QStringLiteral( "Norican" ),
    QStringLiteral( "Nosifer" ),
    QStringLiteral( "Nothing You Could Do" ),
    QStringLiteral( "Noticia Text" ),
    QStringLiteral( "Noto Sans" ),
    QStringLiteral( "Noto Serif" ),
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
    QStringLiteral( "Odor Mean Chey" ),
    QStringLiteral( "Offside" ),
    QStringLiteral( "Old Standard TT" ),
    QStringLiteral( "Oldenburg" ),
    QStringLiteral( "Oleo Script" ),
    QStringLiteral( "Oleo Script Swash Caps" ),
    QStringLiteral( "Open Sans" ),
    QStringLiteral( "Open Sans Condensed" ),
    QStringLiteral( "Oranienbaum" ),
    QStringLiteral( "Orbitron" ),
    QStringLiteral( "Oregano" ),
    QStringLiteral( "Orienta" ),
    QStringLiteral( "Original Surfer" ),
    QStringLiteral( "Oswald" ),
    QStringLiteral( "Over the Rainbow" ),
    QStringLiteral( "Overlock" ),
    QStringLiteral( "Overlock SC" ),
    QStringLiteral( "Ovo" ),
    QStringLiteral( "Oxygen" ),
    QStringLiteral( "Oxygen Mono" ),
    QStringLiteral( "PT Mono" ),
    QStringLiteral( "PT Sans" ),
    QStringLiteral( "PT Sans Caption" ),
    QStringLiteral( "PT Sans Narrow" ),
    QStringLiteral( "PT Serif" ),
    QStringLiteral( "PT Serif Caption" ),
    QStringLiteral( "Pacifico" ),
    QStringLiteral( "Palanquin" ),
    QStringLiteral( "Palanquin Dark" ),
    QStringLiteral( "Paprika" ),
    QStringLiteral( "Parisienne" ),
    QStringLiteral( "Passero One" ),
    QStringLiteral( "Passion One" ),
    QStringLiteral( "Pathway Gothic One" ),
    QStringLiteral( "Patrick Hand" ),
    QStringLiteral( "Patrick Hand SC" ),
    QStringLiteral( "Patua One" ),
    QStringLiteral( "Paytone One" ),
    QStringLiteral( "Peddana" ),
    QStringLiteral( "Peralta" ),
    QStringLiteral( "Permanent Marker" ),
    QStringLiteral( "Petit Formal Script" ),
    QStringLiteral( "Petrona" ),
    QStringLiteral( "Philosopher" ),
    QStringLiteral( "Piedra" ),
    QStringLiteral( "Pinyon Script" ),
    QStringLiteral( "Pirata One" ),
    QStringLiteral( "Plaster" ),
    QStringLiteral( "Play" ),
    QStringLiteral( "Playball" ),
    QStringLiteral( "Playfair Display" ),
    QStringLiteral( "Playfair Display SC" ),
    QStringLiteral( "Podkova" ),
    QStringLiteral( "Poiret One" ),
    QStringLiteral( "Poller One" ),
    QStringLiteral( "Poly" ),
    QStringLiteral( "Pompiere" ),
    QStringLiteral( "Pontano Sans" ),
    QStringLiteral( "Poppins" ),
    QStringLiteral( "Port Lligat Sans" ),
    QStringLiteral( "Port Lligat Slab" ),
    QStringLiteral( "Pragati Narrow" ),
    QStringLiteral( "Prata" ),
    QStringLiteral( "Preahvihear" ),
    QStringLiteral( "Press Start 2P" ),
    QStringLiteral( "Princess Sofia" ),
    QStringLiteral( "Prociono" ),
    QStringLiteral( "Prosto One" ),
    QStringLiteral( "Puritan" ),
    QStringLiteral( "Purple Purse" ),
    QStringLiteral( "Quando" ),
    QStringLiteral( "Quantico" ),
    QStringLiteral( "Quattrocento" ),
    QStringLiteral( "Quattrocento Sans" ),
    QStringLiteral( "Questrial" ),
    QStringLiteral( "Quicksand" ),
    QStringLiteral( "Quintessential" ),
    QStringLiteral( "Qwigley" ),
    QStringLiteral( "Racing Sans One" ),
    QStringLiteral( "Radley" ),
    QStringLiteral( "Rajdhani" ),
    QStringLiteral( "Raleway" ),
    QStringLiteral( "Raleway Dots" ),
    QStringLiteral( "Ramabhadra" ),
    QStringLiteral( "Ramaraja" ),
    QStringLiteral( "Rambla" ),
    QStringLiteral( "Rammetto One" ),
    QStringLiteral( "Ranchers" ),
    QStringLiteral( "Rancho" ),
    QStringLiteral( "Ranga" ),
    QStringLiteral( "Rationale" ),
    QStringLiteral( "Ravi Prakash" ),
    QStringLiteral( "Redressed" ),
    QStringLiteral( "Reenie Beanie" ),
    QStringLiteral( "Revalia" ),
    QStringLiteral( "Ribeye" ),
    QStringLiteral( "Ribeye Marrow" ),
    QStringLiteral( "Righteous" ),
    QStringLiteral( "Risque" ),
    QStringLiteral( "Roboto" ),
    QStringLiteral( "Roboto Condensed" ),
    QStringLiteral( "Roboto Slab" ),
    QStringLiteral( "Rochester" ),
    QStringLiteral( "Rock Salt" ),
    QStringLiteral( "Rokkitt" ),
    QStringLiteral( "Romanesco" ),
    QStringLiteral( "Ropa Sans" ),
    QStringLiteral( "Rosario" ),
    QStringLiteral( "Rosarivo" ),
    QStringLiteral( "Rouge Script" ),
    QStringLiteral( "Rozha One" ),
    QStringLiteral( "Rubik Mono One" ),
    QStringLiteral( "Rubik One" ),
    QStringLiteral( "Ruda" ),
    QStringLiteral( "Rufina" ),
    QStringLiteral( "Ruge Boogie" ),
    QStringLiteral( "Ruluko" ),
    QStringLiteral( "Rum Raisin" ),
    QStringLiteral( "Ruslan Display" ),
    QStringLiteral( "Russo One" ),
    QStringLiteral( "Ruthie" ),
    QStringLiteral( "Rye" ),
    QStringLiteral( "Sacramento" ),
    QStringLiteral( "Sail" ),
    QStringLiteral( "Salsa" ),
    QStringLiteral( "Sanchez" ),
    QStringLiteral( "Sancreek" ),
    QStringLiteral( "Sansita One" ),
    QStringLiteral( "Sarabun" ),
    QStringLiteral( "Sarina" ),
    QStringLiteral( "Sarpanch" ),
    QStringLiteral( "Satisfy" ),
    QStringLiteral( "Scada" ),
    QStringLiteral( "Scheherazade" ),
    QStringLiteral( "Schoolbell" ),
    QStringLiteral( "Seaweed Script" ),
    QStringLiteral( "Sevillana" ),
    QStringLiteral( "Seymour One" ),
    QStringLiteral( "Shadows Into Light" ),
    QStringLiteral( "Shadows Into Light Two" ),
    QStringLiteral( "Shanti" ),
    QStringLiteral( "Share" ),
    QStringLiteral( "Share Tech" ),
    QStringLiteral( "Share Tech Mono" ),
    QStringLiteral( "Shojumaru" ),
    QStringLiteral( "Short Stack" ),
    QStringLiteral( "Siemreap" ),
    QStringLiteral( "Sigmar One" ),
    QStringLiteral( "Signika" ),
    QStringLiteral( "Signika Negative" ),
    QStringLiteral( "Simonetta" ),
    QStringLiteral( "Sintony" ),
    QStringLiteral( "Sirin Stencil" ),
    QStringLiteral( "Six Caps" ),
    QStringLiteral( "Skranji" ),
    QStringLiteral( "Slabo 13px" ),
    QStringLiteral( "Slabo 27px" ),
    QStringLiteral( "Slackey" ),
    QStringLiteral( "Smokum" ),
    QStringLiteral( "Smythe" ),
    QStringLiteral( "Sniglet" ),
    QStringLiteral( "Snippet" ),
    QStringLiteral( "Snowburst One" ),
    QStringLiteral( "Sofadi One" ),
    QStringLiteral( "Sofia" ),
    QStringLiteral( "Sonsie One" ),
    QStringLiteral( "Sorts Mill Goudy" ),
    QStringLiteral( "Source Code Pro" ),
    QStringLiteral( "Source Sans Pro" ),
    QStringLiteral( "Source Serif Pro" ),
    QStringLiteral( "Special Elite" ),
    QStringLiteral( "Spicy Rice" ),
    QStringLiteral( "Spinnaker" ),
    QStringLiteral( "Spirax" ),
    QStringLiteral( "Squada One" ),
    QStringLiteral( "Sree Krushnadevaraya" ),
    QStringLiteral( "Stalemate" ),
    QStringLiteral( "Stalinist One" ),
    QStringLiteral( "Stardos Stencil" ),
    QStringLiteral( "Stint Ultra Condensed" ),
    QStringLiteral( "Stint Ultra Expanded" ),
    QStringLiteral( "Stoke" ),
    QStringLiteral( "Strait" ),
    QStringLiteral( "Sue Ellen Francisco" ),
    QStringLiteral( "Sunshiney" ),
    QStringLiteral( "Supermercado One" ),
    QStringLiteral( "Suranna" ),
    QStringLiteral( "Suravaram" ),
    QStringLiteral( "Suwannaphum" ),
    QStringLiteral( "Swanky and Moo Moo" ),
    QStringLiteral( "Syncopate" ),
    QStringLiteral( "Tangerine" ),
    QStringLiteral( "Taprom" ),
    QStringLiteral( "Tauri" ),
    QStringLiteral( "Teko" ),
    QStringLiteral( "Telex" ),
    QStringLiteral( "Tenali Ramakrishna" ),
    QStringLiteral( "Tenor Sans" ),
    QStringLiteral( "Text Me One" ),
    QStringLiteral( "The Girl Next Door" ),
    QStringLiteral( "Tienne" ),
    QStringLiteral( "Timmana" ),
    QStringLiteral( "Tinos" ),
    QStringLiteral( "Titan One" ),
    QStringLiteral( "Titillium Web" ),
    QStringLiteral( "Trade Winds" ),
    QStringLiteral( "Trocchi" ),
    QStringLiteral( "Trochut" ),
    QStringLiteral( "Trykker" ),
    QStringLiteral( "Tulpen One" ),
    QStringLiteral( "Ubuntu" ),
    QStringLiteral( "Ubuntu Condensed" ),
    QStringLiteral( "Ubuntu Mono" ),
    QStringLiteral( "Ultra" ),
    QStringLiteral( "Uncial Antiqua" ),
    QStringLiteral( "Underdog" ),
    QStringLiteral( "Unica One" ),
    QStringLiteral( "UnifrakturCook" ),
    QStringLiteral( "UnifrakturMaguntia" ),
    QStringLiteral( "Unkempt" ),
    QStringLiteral( "Unlock" ),
    QStringLiteral( "Unna" ),
    QStringLiteral( "VT323" ),
    QStringLiteral( "Vampiro One" ),
    QStringLiteral( "Varela" ),
    QStringLiteral( "Varela Round" ),
    QStringLiteral( "Vast Shadow" ),
    QStringLiteral( "Vesper Libre" ),
    QStringLiteral( "Vibur" ),
    QStringLiteral( "Vidaloka" ),
    QStringLiteral( "Viga" ),
    QStringLiteral( "Voces" ),
    QStringLiteral( "Volkhov" ),
    QStringLiteral( "Vollkorn" ),
    QStringLiteral( "Voltaire" ),
    QStringLiteral( "Waiting for the Sunrise" ),
    QStringLiteral( "Wallpoet" ),
    QStringLiteral( "Walter Turncoat" ),
    QStringLiteral( "Warnes" ),
    QStringLiteral( "Wellfleet" ),
    QStringLiteral( "Wendy One" ),
    QStringLiteral( "Wire One" ),
    QStringLiteral( "Yanone Kaffeesatz" ),
    QStringLiteral( "Yellowtail" ),
    QStringLiteral( "Yeseva One" ),
    QStringLiteral( "Yesteryear" ),
    QStringLiteral( "Zeyada" ),
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
  connect( task, &QgsNetworkContentFetcherTask::fetched, this, [ = ]
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
    QgsDebugMsg( QStringLiteral( "Cannot create local fonts dir: %1" ).arg( directory ) );
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
