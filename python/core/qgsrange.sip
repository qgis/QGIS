class QgsDoubleRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsDoubleRange( double lower, double upper, bool includeLower = true, bool includeUpper = true );
    double lower() const;
    double upper() const;
    bool includeLower() const;
    bool includeUpper() const;
    bool isEmpty() const;
    bool isSingleton() const;
    bool contains( const QgsDoubleRange &other ) const;
    bool contains( double element ) const;
    bool overlaps( const QgsDoubleRange &other ) const;
};

class QgsIntRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsIntRange( int lower, int upper, bool includeLower = true, bool includeUpper = true );
    int lower() const;
    int upper() const;
    bool includeLower() const;
    bool includeUpper() const;
    bool isEmpty() const;
    bool isSingleton() const;
    bool contains( const QgsIntRange &other ) const;
    bool contains( int element ) const;
    bool overlaps( const QgsIntRange &other ) const;
};

class QgsDateRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsDateRange( const QDate &lower = QDate(), const QDate &upper = QDate(), bool includeLower = true, bool includeUpper = true );
    QDate begin() const;
    QDate end() const;
    bool includeBeginning() const;
    bool includeEnd() const;
    bool isEmpty() const;
    bool isInstant() const;
    bool isInfinite() const;
    bool contains( const QgsDateRange &other ) const;
    bool contains( const QDate &element ) const;
    bool overlaps( const QgsDateRange &other ) const;
};

class QgsDateTimeRange
{
%TypeHeaderCode
#include <qgsrange.h>
%End

  public:

    QgsDateTimeRange( const QDateTime &lower = QDateTime(), const QDateTime &upper = QDateTime(), bool includeLower = true, bool includeUpper = true );
    QDateTime begin() const;
    QDateTime end() const;
    bool includeBeginning() const;
    bool includeEnd() const;
    bool isEmpty() const;
    bool isInstant() const;
    bool isInfinite() const;
    bool contains( const QgsDateTimeRange &other ) const;
    bool contains( const QDateTime &element ) const;
    bool overlaps( const QgsDateTimeRange &other ) const;
};

