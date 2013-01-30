#ifndef QGSSQLANYWHEREFEATUREITERATOR_H
#define QGSSQLANYWHEREFEATUREITERATOR_H

#include "qgsfeatureiterator.h"
#include "sqlanystatement.h"

class QgsSqlAnywhereProvider;
class QgsSqlAnywhereResult;

class QgsSqlAnywhereFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsSqlAnywhereFeatureIterator( QgsSqlAnywhereProvider* p, const QgsFeatureRequest & request );

    ~QgsSqlAnywhereFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );
    bool nextFeature( QgsFeature& feature, SqlAnyStatement *stmt );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsSqlAnywhereProvider* P;

  private:
    bool prepareStatement( QString whereClause );

    QString whereClauseRect() const;

    QString quotedPrimaryKey() const;
    QString whereClauseFid() const;

    /**
     * Statement handle for fetching of features by bounding rectangle
     */
    SqlAnyStatement *mStmt;

    QgsRectangle mStmtRect;

};

#endif // QGSSQLANYWHEREFEATUREITERATOR_H
