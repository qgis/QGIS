#ifndef QGSPOSTGRESFEATUREITERATOR_H
#define QGSPOSTGRESFEATUREITERATOR_H

#include "qgsfeatureiterator.h"

#include <QQueue>


class QgsPostgresProvider;
class QgsPostgresResult;

class QgsPostgresFeatureIterator : public QgsAbstractFeatureIterator
{
  public:
    QgsPostgresFeatureIterator( QgsPostgresProvider* p, const QgsFeatureRequest& request );

    ~QgsPostgresFeatureIterator();

    //! fetch next feature, return true on success
    virtual bool nextFeature( QgsFeature& feature );

    //! reset the iterator to the starting position
    virtual bool rewind();

    //! end of iterating: free the resources / lock
    virtual bool close();

  protected:
    QgsPostgresProvider* P;

    QString whereClauseRect();
    bool getFeature( QgsPostgresResult &queryResult, int row, QgsFeature &feature );
    bool declareCursor( const QString& whereClause );

    QString mCursorName;

    /**
     * Feature queue that GetNextFeature will retrieve from
     * before the next fetch from PostgreSQL
     */
    QQueue<QgsFeature> mFeatureQueue;

    //! Maximal size of the feature queue
    int mFeatureQueueSize;

    //!< Number of retrieved features
    int mFetched;

    static const int sFeatureQueueSize;

    //! Whether to use mFeatureQueue
    bool mUseQueue;
};

#endif // QGSPOSTGRESFEATUREITERATOR_H
