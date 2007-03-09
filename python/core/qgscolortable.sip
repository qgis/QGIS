
class QgsColorTable
{
%TypeHeaderCode
#include <qgscolortable.h>
%End

public:
    /** 
     *  \brief Constructor.
     *  \param interp color table interpretation
     */
    QgsColorTable ( int interp = Rgb ); 

    /** \brief The destuctor.  */
    ~QgsColorTable();

    /** \brief Color table interpretation. */
    // This is taken from GDAL, GPI_RGB is mapped to Rgba 
    enum Interp {
	Gray = 0, // Use c1 as grayscale value.
	Rgb,      // Use c1 as red, c2 as green, c3 as blue
	Rgba,     // Use c1 as red, c2 as green, c3 as blue and c4 as alpha.
	Cmyk,     // Use c1 as cyan, c2 as magenta, c3 as yellow and c4 as black.
	Hls       // c1 hue, c2 lightness, c3 saturation; should be His ? 
    };

    /** 
     *  \brief Color table is defined 
     *  \return true if at least one rule is defined 
     *  \false no rule defined
     */
    bool defined ( );

    /** \brief Get color table interpretation */
    int interpretation ( );

    /** \brief Add a discrete color */
    void add ( unsigned int index, unsigned char c1, unsigned char c2, unsigned char c3, unsigned char c4 = 0 );

    /** \brief Add a ramp rule */ 
    void add ( double min,  double max,
	       unsigned char min_c1, unsigned char min_c2, unsigned char min_c3, unsigned char min_c4,
               unsigned char max_c1, unsigned char max_c2, unsigned char max_c3, unsigned char max_c4);
    
    /** \brief Get color 
     *  \return true color was found
     *  \return false color was no found
     */
    bool color ( double value, int *c1, int *c2, int *c3 );
    
    /** \brief Sort ramp rules */
    void sort ( ); 

    /** \brief Print to stderr - for debuging */
    void print ( );

    /** \brief Minimum value */
    double rmin();
   
    /** \brief Maximum value */
    double rmax();

    /** \brief Clear the color table */
    void clear();

};

