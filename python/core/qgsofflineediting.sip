/** \class QgsOfflineEditing
 *  \brief Class for accessing functionality of Offline Editing Plugin.
 * This class can be used to access offline editing functionality from plugins.
 */
class QgsOfflineEditing : QObject
{
%TypeHeaderCode
#include <qgsofflineediting.h>
%End

  public:
    enum ProgressMode
    {
      CopyFeatures,
      ProcessFeatures,
      AddFields,
      AddFeatures,
      RemoveFeatures,
      UpdateFeatures,
      UpdateGeometries
    };

    QgsOfflineEditing();
    ~QgsOfflineEditing();

    /** convert current project for offline editing
     * @param offlineDataPath path to offline db file
     * @param offlineDbFile offline db file name
     * @param layerIds list of layer names to convert
     */
    bool convertToOfflineProject( const QString& offlineDataPath, const QString& offlineDbFile, const QStringList& layerIds );

    /** return true if current project is offline */
    bool isOfflineProject();

    /** synchronize to remote layers */
    void synchronize();

  signals:
    /** emit a signal that processing has started */
    void progressStarted();

    /** emit a signal that the next layer of numLayers has started processing
     * @param layer current layer index
     * @param numLayers total number of layers
     */
    void layerProgressUpdated( int layer, int numLayers );

    /** emit a signal that sets the mode for the progress of the current operation
     * @param mode progress mode
     * @param maximum total number of entities to process in the current operation
     */
    void progressModeSet( QgsOfflineEditing::ProgressMode mode, int maximum );

    /** emit a signal with the progress of the current mode
     * @param progress current index of processed entities
     */
    void progressUpdated( int progress );

    /** emit a signal that processing of all layers has finished */
    void progressStopped();
};
