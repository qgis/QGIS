class QgsRuntimeProfiler
{
%TypeHeaderCode
#include <qgsruntimeprofiler.h>
%End
  public:
    /**
     * @brief Instance of the run time profiler. To use the main profiler
     * use this instance.
     * @return The instance of the run time profiler
     */
    QgsRuntimeProfiler();

    /**
     * @brief Begin the group for the profiler. Groups will append {GroupName}/ to the
     * front of the profile tag set using start.
     * @param name The name of the group.
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
     */
     const QList<QPair<QString, double>> profileTimes() const;

    /**
     * @brief clear Clear all profile data.
     */
    void clear();

    /**
     * @brief The current total time collected in the profiler.
     * @return The current total time collected in the profiler.
     */
    double totalTime();

};
