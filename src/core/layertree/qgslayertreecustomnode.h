#ifndef QGSLAYERTREECUSTOMNODE_H
#define QGSLAYERTREECUSTOMNODE_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslayertreenode.h"
#include <QObject>

class CORE_EXPORT QgsLayerTreeCustomNode : public QgsLayerTreeNode
{
    Q_OBJECT
  public:
    explicit QgsLayerTreeCustomNode( const QString &nodeId, const QString &nodeName = QString(), bool checked = true );

#ifndef SIP_RUN
    QgsLayerTreeCustomNode( const QgsLayerTreeCustomNode &other );
#endif

#ifdef SIP_RUN
    SIP_PYOBJECT __repr__();
    % MethodCode
    QString str = QStringLiteral( "<QgsLayerTreeCustomNode: %1>" ).arg( sipCpp->name() );
    sipRes = PyUnicode_FromString( str.toUtf8().constData() );
    % End
#endif

    QString nodeId() const { return mId; };

    /**
     * Returns the group's name.
     */
    QString name() const override;

    /**
     * Sets the group's name.
     */
    void setName( const QString &name ) override;

    /**
     * Read group (tree) from XML element <layer-tree-group> and return the newly created group (or NULLPTR on error).
     * Does not resolve textual references to layers. Call resolveReferences() afterwards to do it.
     */
    static QgsLayerTreeCustomNode *readXml( const QDomElement &element, const QgsReadWriteContext &context ) SIP_FACTORY;  // cppcheck-suppress duplInheritedMember

    /**
     * Write group (tree) as XML element <layer-tree-group> and add it to the given parent element
     */
    void writeXml( QDomElement &parentElement, const QgsReadWriteContext &context ) override;

    QString dump() const override;

    QgsLayerTreeCustomNode *clone() const override SIP_FACTORY;

    void resolveReferences( const QgsProject *project, bool looseMatching = false ) override;

  private:
#ifdef SIP_RUN

    /**
     * Copies are not allowed
     */
    QgsLayerTreeCustomNode( const QgsLayerTreeCustomNode &other );
#endif

    QgsLayerTreeCustomNode &operator= ( const QgsLayerTreeCustomNode & ) = delete;

    QString mId;
    QString mName;
};

#endif // QGSLAYERTREECUSTOMNODE_H
