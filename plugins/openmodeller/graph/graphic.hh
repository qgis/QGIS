/**
 * Declarations of GImage, GGraph and GFrame classes.
 *
 * This is an interface to held different graphic libraries.
 * 
 * @file
 * @author Mauro E S Muñoz <mauro@cria.org.br>
 * @date 2003-10-25
 * $Id$
 *
 * LICENSE INFORMATION
 * 
 * Copyright(c) 2003 by CRIA -
 * Centro de Referencia em Informacao Ambiental
 *
 * http://www.cria.org.br
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details:
 * 
 * http://www.gnu.org/copyleft/gpl.html
 */

#ifndef _GRAPHICHH_
#define _GRAPHICHH_

#include "graph/color.hh"



/**********************************************************************/
/*************************** Frame Creation ***************************/

class GFrame;

GFrame *createFrame( char *title, int control_height,
                     int dimx, int dimy );


/**********************************************************************/
/******************************** GImage ******************************/

class GGraph;

/** 
 * Base class to integer drawables.
 * 
 */

/**********/
class GImage
{
public:
  GImage( int dimx, int dimy );
  virtual ~GImage();

  virtual int  foreground( GColor c )=0;
  virtual int  background( GColor c )=0;

  virtual void clear() = 0;

  // Draw mode: simply copy or invert.
  virtual void copyMode() = 0;
  virtual void invMode()  = 0;

  virtual void text( int x, int y, char *text, GColor c )=0;

  // Draw geometric forms.
  //
  virtual void pixel  ( int x, int y )=0;
  virtual void line   ( int x0, int y0, int x1, int y1 )=0;
  virtual void lines  ( int *pnt, int npnt )=0;
  virtual void fPolyg ( int *pnt, int npnt )=0;
  virtual void rect   ( int x, int y, int w, int h )=0;
  virtual void fRect  ( int x, int y, int w, int h )=0;
  virtual void circle ( int x, int y, int raio ) = 0;
  virtual void fCircle( int x, int y, int raio ) = 0;
  virtual void elipse ( int x, int y, int rx, int ry )=0;
  virtual void fElipse( int x, int y, int rx, int ry )=0;

  // Draw geometric forms passing temporary color.
  //
  virtual void pixel  ( int x, int y,
                        GColor c )=0;
  virtual void line   ( int x0, int y0, int x1, int y1,
                        GColor c )=0;
  virtual void lines  ( int *pnt, int npnt,
                        GColor c )=0;
  virtual void fPolyg ( int *pnt, int npnt,
                        GColor c )=0;
  virtual void rect   ( int x, int y, int w, int h,
                        GColor c )=0;
  virtual void fRect  ( int x, int y, int w, int h,
                        GColor c )=0;
  virtual void circle ( int x, int y, int raio,
                        GColor c )=0;
  virtual void fCircle( int x, int y, int raio,
                        GColor c )=0;
  virtual void elipse ( int x, int y, int rx, int ry,
                        GColor c )=0;
  virtual void fElipse( int x, int y, int rx, int ry,
                        GColor c )=0;

  // Put other images.
  //
  virtual void put( GGraph *grafico );
  virtual void put( GImage *imagem )=0;
  virtual void put( int x0, int y0, GImage *imagem )=0;
  virtual void put( int x, int y, int x0, int y0,
                    int dimx, int dimy, GImage *imagem )=0;

  int dimX()     { return( f_dimx ); }
  int dimY()     { return( f_dimy ); }


protected:

  int  f_dimx;
  int  f_dimy;
};




/**********************************************************************/
/******************************* GGraph *******************************/

/** 
 * Graphic float drawable.
 *
 */

/**********/
class GGraph
{
public:
  GGraph( GImage *imag );
  virtual ~GGraph();

  int foreground( GColor &c )  { return f_imag->foreground( c ); };
  int background( GColor &c )  { return f_imag->background( c ); };

  void clear()  { f_imag->clear(); }

  void scale( float x0, float y0, float x1, float y1 );

  void text( float x, float y, char *text, GColor &c );

  // Draw geometric forms.
  //
  void pixel( float x, float y );
  void line( float x0, float y0, float x1, float y1 );
  void lines( float *pnt, int npnt );
  void polyg( float *pnt, int npnt ); // Emenda o final.
  void fPolyg( float *pnt, int npnt );
  void rect( float x0, float y0, float w,  float h );
  void fRect( float x0, float y0, float w,  float h );
  void circle( float x0, float y0, float raio );
  void fCircle( float x0, float y0, float raio );
  void elipse( float x0, float y0, float rx, float ry );
  void fElipse( float x0, float y0, float rx, float ry );

  // Draw geometric forms passing temporary color.
  //
  void pixel( float x, float y, GColor &c );
  void line( float x0, float y0, float x1, float y1, GColor &c );
  void lines( float *pnt, int npnt, GColor &c );
  void polyg( float *pnt, int npnt, GColor &c );   // Emenda o final.
  void fPolyg( float *pnt, int npnt, GColor &c );
  void rect( float x0, float y0, float w,  float h, GColor &c );
  void fRect( float x0, float y0, float w,  float h, GColor &c );
  void circle( float x0, float y0, float raio, GColor &c );
  void fCircle( float x0, float y0, float raio, GColor &c );
  void elipse( float x0, float y0, float rx, float ry, GColor &c );
  void fElipse( float x0, float y0, float rx, float ry, GColor &c );

  // Draw marks.
  void markSquare ( float x, float y, int size, GColor &c );
  void markFSquare( float x, float y, int size, GColor &c );
  void markRound  ( float x, float y, int size, GColor &c );
  void markFRound ( float x, float y, int size, GColor &c );
  void markAxe    ( float x, float y, int size, GColor &c );

  float minX()  { return( f_x0 ); }
  float minY()  { return( f_y0 ); }
  float maxX()  { return( f_x0 + f_dx ); }
  float maxY()  { return( f_y0 + f_dy ); }

  float dimX()  { return( f_dx ); }
  float dimY()  { return( f_dy ); }

  // Transform image coordinates in graphic coordinates.
  float transfX( int x_imagem );
  float transfY( int y_imagem );

  // Distancia entre dois pontos.
  double stepX()  { return( f_dx / f_imag->dimX() ); }
  double stepY()  { return( f_dy / f_imag->dimY() ); }

  GImage *getImage()  { return( f_imag ); }


private:

  int convX( float x );
  int convY( float y );


  GImage *f_imag;

  float f_x0;
  float f_y0;
  float f_dx;
  float f_dy;
};




/**********************************************************************/
/******************************* GFrame *******************************/

/** 
 * Base class to frame windows.
 *
 */

/**********/
class GFrame
{
public:

  typedef void (*FuncExec)  (void);
  typedef void (*FuncNotif) (int);
  typedef void (*FuncButton)(int, int);


public:
  GFrame()           {}
  virtual ~GFrame()  {}

  /** Function to be called in each window loop cicle. */
  virtual void funcAlways( FuncExec func )=0;

  /** Function to be called before the window appears. */
  virtual void funcInit( FuncExec func )=0;

  /** Function to be called at the "Expose" window event. */
  virtual void funcShow( FuncExec func )=0;

  // 'bt'(button) = 1, 2 ou 3.

  /**
   * Function to be called at the "Press" mouse event of button 'bt'.
   * 'bt' = 1, 2 or 3.
   */
  virtual void funcBtPress( GImage *cnv, FuncButton f, int bt )=0;

  /**
   * Function to be called at the "Release" mouse event of button 'bt'.
   * 'bt' = 1, 2 or 3.
   */
  virtual void funcBtRelease( GImage *cnv, FuncButton f, int bt )=0;

  /**
   * Function to be called at the "Drag" mouse event of button 'bt'.
   * 'bt' = 1, 2 or 3.
   */
  virtual void funcBtDrag( GImage *cnv, FuncButton f, int bt )=0;

  /** Initiate the window main loop. */
  virtual void exec()=0;
  virtual void flush()=0;

  /**
   * Create a new Canvas.
   * All created canvas are deallocated with GFrame!
   */
  virtual GImage *newCanvas( int x, int y, int dimx, int dimy )=0;

  /**
   * Create a new Pixmap associated with the canvas 'cnv'.
   * All created pixmap are deallocated with GFrame!
   */
  virtual GImage *newPixmap( GImage *cnv, int dimx, int dimy  )=0;

  /**
   * Insert a press button labeled 'lab' at the position (x,y) of the
   * frame. The function 'f' will be called at the "press" mouse event.
   */
  virtual void button  ( int x, int y, char *lab, FuncExec f,
                         GColor c )=0;

  /**
   * Insert a two states press/release button labeled 'lab' at the
   * position (x,y) of the frame. The function 'f' will be called at
   * the "press" mouse event no matter what is the button state.
   */
  virtual void buttonA ( int x, int y, char *lab, FuncExec f,
                         GColor c )=0;

  /**
   * Insert a multiple two states press/release buttons at the
   * position (x,y) of the frame. Each button is labeled by one 'lab's
   * letter. The function 'f' will be called at the "press" mouse event
   * no matter what is the button state. The number of the button is
   * passed to 'f' as a parameter.
   */
  virtual void buttonMA( int x, int y, char *lab, FuncNotif f,
                         GColor *c=0 )=0;

  /**
   * Insert a choose widget.
   * @see buttonMA()
   */
  virtual void buttonME( int x, int y, char *lab, FuncNotif f,
                         GColor *c=0 )=0;
};




#endif

