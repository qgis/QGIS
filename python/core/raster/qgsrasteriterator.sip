class QgsRasterIterator
{
%TypeHeaderCode
#include <qgsrasteriterator.h>
%End
  public:

    QgsRasterIterator( QgsRasterInterface* input );
    ~QgsRasterIterator();

    /**Start reading of raster band. Raster data can then be retrieved by calling readNextRasterPart until it returns false.
      @param bandNumber number of raster band to read
      @param nCols number of columns
      @param nRows number of rows
      @param extent area to read
     */
    void startRasterRead( int bandNumber, int nCols, int nRows, const QgsRectangle& extent );

    /**Fetches next part of raster data
       @param bandNumber band to read
       @param nCols number of columns on output device
       @param nRows number of rows on output device
       @param block to return the pointer to raster block
       @param topLeftCol top left column
       @param topLeftRow top left row
       @return false if the last part was already returned*/
    bool readNextRasterPart( int bandNumber,
                             int& nCols, int& nRows,
                             QgsRasterBlock** block,
                             int& topLeftCol, int& topLeftRow );

    void stopRasterRead( int bandNumber );

    const QgsRasterInterface* input() const;

    void setMaximumTileWidth( int w );
    int maximumTileWidth() const;

    void setMaximumTileHeight( int h );
    int maximumTileHeight() const;
};
