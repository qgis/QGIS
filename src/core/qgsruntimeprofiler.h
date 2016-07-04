#ifndef QGSRUNTIMEPROFILER_H
#define QGSRUNTIMEPROFILER_H

#include <QTime>
#include <QPair>
#include <QStack>

/** \ingroup core
 * \class QgsRuntimeProfiler
 */
class CORE_EXPORT QgsRuntimeProfiler
{
  public:
    /**
     * Constructor to create a new runtime profiler.
     */
    QgsRuntimeProfiler();

    /**
     * @brief Instance of the run time profiler. To use the main profiler
     * use this instance.
     * @return The instance of the run time profiler
     */
    static QgsRuntimeProfiler * instance();

    /**
     * @brief Begin the group for the profiler. Groups will append {GroupName}/ to the
     * front of the profile tag set using start.
     * @param name The name of the group.
     */
    void beginGroup( const QString& name );

    /**
     * @brief End the current active group.
     */
    void endGroup();

    /**
     * @brief Start a profile event with the given name.
     * @param name The name of the profile event. Will have the name of
     * the active group appened after ending.
     */
    void start( const QString& name );

    /**
     * @brief End the current profile event.
     */
    void end();

    /**
     * @brief Return all the current profile times.
     * @return A list of profile event names and times.
     * @note not available in python bindings
     */
    const QList<QPair<QString, double > > profileTimes() const { return mProfileTimes; }

    /**
     * @brief clear Clear all profile data.
     */
    void clear();

    /**
     * @brief The current total time collected in the profiler.
     * @return The current total time collected in the profiler.
     */
    double totalTime();

  private:
    static QgsRuntimeProfiler* mInstance;

    QString mGroupPrefix;
    QStack<QString> mGroupStack;
    QTime mProfileTime;
    QString mCurrentName;
    QList<QPair<QString, double > > mProfileTimes;
};

#endif // QGSRUNTIMEPROFILER_H
