#ifndef QGSAIFILECONTEXTPROVIDER_H
#define QGSAIFILECONTEXTPROVIDER_H

#include "qgis_app.h"

#include <QObject>
#include <QString>
#include <QStringList>

struct APP_EXPORT QgsAiFileContext
{
  QString filePath;
  QString selectedText;
  QString fileSnippet;
  qint64 fileSize = 0;
  bool truncated = false;
  bool binary = false;

  bool isValid() const { return !filePath.isEmpty() || !selectedText.isEmpty(); }
};

class APP_EXPORT QgsAiFileContextProvider : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiFileContextProvider( const QString &workspaceRoot, QObject *parent = nullptr );

    QgsAiFileContext buildContext( const QString &filePath, const QString &selectedText = QString(), int maxBytes = 16384, bool allowExternal = false ) const;
    QString resolveWorkspaceFile( const QString &filePath ) const;
    QStringList workspaceFileCandidates( const QString &query, int maxResults = 25 ) const;
    QStringList searchInFile( const QString &filePath, const QString &needle, int maxMatches = 25 ) const;
    QString diffPreview( const QString &beforeText, const QString &afterText ) const;
    QString workspaceRoot() const { return mWorkspaceRoot; }

  private:
    QString normalizePath( const QString &filePath, bool allowExternal = false ) const;
    bool isInWorkspace( const QString &absolutePath ) const;
    QString mWorkspaceRoot;
};

#endif // QGSAIFILECONTEXTPROVIDER_H
