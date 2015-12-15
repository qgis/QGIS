#ifndef QGSSLOT_TO_FUNCTION_H
#define QGSSLOT_TO_FUNCTION_H

#include <QObject>

/**
 * This is an helper Qt object used by the SQLite virtual layer module
 * in order to connect to the deletion signal of a vector layer,
 * since internal classes of the SQLite module cannot derive from QObject
 */
class QgsSlotToFunction : public QObject
{
    Q_OBJECT
  public:
    QgsSlotToFunction() : mCallback( nullptr ), mArg( nullptr ) {}
    QgsSlotToFunction( void ( *callback )( void* ), void* arg ) : mCallback( callback ), mArg( arg ) {}
  private slots:
    void onSignal() { if ( mCallback ) mCallback( mArg ); }
  private:
    void ( *mCallback )( void* );
    void* mArg;
};

#endif

