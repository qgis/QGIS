/**
* \ingroup analysis
* \class QgsLineVectorLayerDirector
* \brief Determine making the graph from vector line layer
*/
class QgsLineVectorLayerDirector : QgsGraphDirector
{
%TypeHeaderCode
#include <qgslinevectorlayerdirector.h>
%End

  public:
    /** Road direction
     * Road can be one-way with direct flow (one can move only from the start
     * point to the end point), one-way with reversed flow (one can move only
     * from the end point to the start point) and bidirectional or two-way
     * (one can move in any direction)
     */
    enum Direction
    {
      DirectionForward,     //!< One-way direct
      DirectionBackward,    //!< One-way reversed
      DirectionBoth,        //!< Two-way
    };

    /**
     * @param myLayer source vector layer
     * @param directionFieldId feield contain road direction value
     * @param directDirectionValue value for one-way road
     * @param reverseDirectionValue value for reverse one-way road
     * @param bothDirectionValue value for road
     * @param defaultDirection default road direction. Will be used if corresponding
     * attribute value is not set or does not equal to the given values
     */
    QgsLineVectorLayerDirector( QgsVectorLayer* myLayer,
                                int directionFieldId,
                                const QString& directDirectionValue,
                                const QString& reverseDirectionValue,
                                const QString& bothDirectionValue,
                                const Direction defaultDirection
                              );

    //! Destructor
    virtual ~QgsLineVectorLayerDirector();

    /*
     * MANDATORY DIRECTOR PROPERTY DECLARATION
     */
    void makeGraph( QgsGraphBuilderInterface *builder,
                    const QVector< QgsPoint >& additionalPoints,
                    QVector< QgsPoint>& snappedPoints /Out/ ) const;

    QString name() const;
};

