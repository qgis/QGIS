#ifndef QGSAITOOLREGISTRY_H
#define QGSAITOOLREGISTRY_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QJsonArray>
#include <QObject>
#include <QStringList>
#include <map>
#include <memory>

class APP_EXPORT QgsAiToolRegistry : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiToolRegistry( QObject *parent = nullptr );
    ~QgsAiToolRegistry() override;

    //! Registers \a tool. Takes ownership. Returns false if a tool with same name already exists.
    bool registerTool( std::unique_ptr<QgsAiTool> tool );

    //! Returns pointer to registered tool, or nullptr.
    QgsAiTool *find( const QString &name ) const;

    //! Returns all registered tool names.
    QStringList toolNames() const;

    int count() const { return mTools.size(); }

    /**
     * Returns a JSON array describing registered tools in Anthropic Claude tool-use format:
     * `[{ "name": "...", "description": "...", "input_schema": {...} }, ...]`.
     * If \a allowedTools is non-empty, only tools whose name is in the set are included.
     * The router converts this format to provider-specific payloads.
     */
    QJsonArray schemasJson( const QStringList &allowedTools = QStringList() ) const;

    //! Removes all registered tools.
    void clear();

  signals:
    void toolRegistered( const QString &name );

  private:
    std::map<QString, std::unique_ptr<QgsAiTool>> mTools;
};

#endif // QGSAITOOLREGISTRY_H
