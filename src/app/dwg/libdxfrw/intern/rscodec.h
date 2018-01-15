/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2014 J.F. Soriano (Rallaz), rallazz@gmail.com         **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

/**
 * Reed-Solomon codec
 * Reed Solomon code lifted from encoder/decoder for Reed-Solomon written by Simon Rockliff
 *
 * Original code:
 * This program may be freely modified and/or given to whoever wants it.
 *  A condition of such distribution is that the author's contribution be
 *  acknowledged by his name being left in the comments heading the program,
 *  however no responsibility is accepted for any financial or other loss which
 *  may result from some unforseen errors or malfunctioning of the program
 *  during use.
 *                                Simon Rockliff, 26th June 1991
 */



#ifndef RSCODEC_H
#define RSCODEC_H

/**
mm: RS code over GF(2^4)
nn: nn= (2^mm) - 1   length of codeword
tt: number of errors that can be corrected
kk: kk = nn-2*tt
pp: irreducible polynomial coeffts, pp [mm] send as int
*/
class RScodec
{
  public:
    RScodec( unsigned int pp, int mm, int tt );

    ~RScodec();
//    bool encode(int *data, int *parity);
//    int decode(int *recd);
    bool encode( unsigned char *data, unsigned char *parity );
    int decode( unsigned char *data );
    bool isOkey() {return isOk;}
    const unsigned int *indexOf() {return index_of;}
    const int *alphaTo() {return alpha_to;}

  private:
    void RSgenerate_gf( unsigned int pp );
    void RSgen_poly();
    int calcDecode( unsigned char *data, int *recd, int **elp, int *d, int *l, int *u_lu, int *s, int *root, int *loc, int *z, int *err, int *reg, int bb );


  private:
    int mm; //RS code over GF(2^4)
    int tt; //number of errors that can be corrected
    int nn; //(2^mm) - 1   length of codeword
    int kk; //nn-2*tt length of original data

    int *gg = nullptr;
    bool isOk;
    unsigned int *index_of;
    int *alpha_to = nullptr;
};

#endif // RSCODEC_H
