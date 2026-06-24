/***************************************************************************
    qgsaivisualcontextutils.cpp
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaivisualcontextutils.h"

#include "qgsapplication.h"
#include "qgssettings.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QImageReader>
#include <QMessageBox>
#include <QMimeDatabase>
#include <QString>
#include <QUuid>

using namespace Qt::StringLiterals;

QString QgsAiVisualContextUtils::visualConsentSettingKey()
{
  return u"strata/visual_context/image_send_consent"_s;
}

bool QgsAiVisualContextUtils::hasVisualConsent()
{
  QgsSettings settings;
  if ( settings.contains( visualConsentSettingKey() ) )
    return settings.value( visualConsentSettingKey(), false ).toBool();
  return settings.value( u"geoai/visual_context/image_send_consent"_s, false ).toBool();
}

bool QgsAiVisualContextUtils::ensureVisualContextConsent( QWidget *parent )
{
  if ( hasVisualConsent() )
    return true;

  if ( !parent )
    return false;

  const QMessageBox::StandardButton answer = QMessageBox::question(
    parent,
    QObject::tr( "Share images with AI" ),
    QObject::tr(
      "Strata can send images to vision-capable AI providers when you attach chat files or capture the map canvas. "
      "Images may include visible map data, labels, styles, or screenshot content. Do you want to allow this?"
    ),
    QMessageBox::Yes | QMessageBox::No,
    QMessageBox::No
  );

  if ( answer != QMessageBox::Yes )
    return false;

  QgsSettings settings;
  settings.setValue( visualConsentSettingKey(), true );
  settings.remove( u"geoai/visual_context/image_send_consent"_s );
  return true;
}

QString QgsAiVisualContextUtils::visualContextDirectory()
{
  const QString dir = QgsApplication::qgisSettingsDirPath() + u"ai_visual_context"_s;
  QDir().mkpath( dir );
  return dir;
}

void QgsAiVisualContextUtils::cleanupOldVisualContextFiles( const QString &dirPath )
{
  QDir dir( dirPath );
  const QFileInfoList files = dir.entryInfoList( QStringList() << u"*.png"_s, QDir::Files, QDir::Time );
  const QDateTime now = QDateTime::currentDateTime();
  for ( const QFileInfo &info : files )
  {
    if ( info.lastModified().secsTo( now ) > 24 * 60 * 60 )
      QFile::remove( info.absoluteFilePath() );
  }
}

bool QgsAiVisualContextUtils::isSupportedImagePath( const QString &path )
{
  const QFileInfo info( path );
  if ( !info.isFile() )
    return false;

  const QList<QByteArray> formats = QImageReader::supportedImageFormats();
  for ( const QByteArray &format : formats )
  {
    if ( info.suffix().compare( format, Qt::CaseInsensitive ) == 0 )
      return true;
  }
  return false;
}

QString QgsAiVisualContextUtils::mimeTypeForImagePath( const QString &path )
{
  const QMimeDatabase mimeDb;
  const QMimeType mime = mimeDb.mimeTypeForFile( path, QMimeDatabase::MatchExtension );
  if ( mime.isValid() && mime.name().startsWith( "image/"_L1 ) )
    return mime.name();

  const QFileInfo info( path );
  const QString suffix = info.suffix().toLower();
  if ( suffix == "jpg"_L1 || suffix == "jpeg"_L1 )
    return u"image/jpeg"_s;
  if ( suffix == "gif"_L1 )
    return u"image/gif"_s;
  if ( suffix == "webp"_L1 )
    return u"image/webp"_s;
  if ( suffix == "bmp"_L1 )
    return u"image/bmp"_s;
  if ( suffix == "tif"_L1 || suffix == "tiff"_L1 )
    return u"image/tiff"_s;
  return u"image/png"_s;
}

QString QgsAiVisualContextUtils::saveDroppedImage( const QImage &image, const QString &format )
{
  if ( image.isNull() )
    return QString();

  const QString dirPath = visualContextDirectory();
  cleanupOldVisualContextFiles( dirPath );

  const QString normalizedFormat = format.trimmed().isEmpty() ? u"PNG"_s : format.toUpper();
  const QString filePath = QDir( dirPath ).filePath( u"chat-drop-%1.%2"_s.arg( QUuid::createUuid().toString( QUuid::WithoutBraces ), normalizedFormat.toLower() ) );
  if ( !image.save( filePath, normalizedFormat.toUtf8().constData() ) )
    return QString();

  return filePath;
}
