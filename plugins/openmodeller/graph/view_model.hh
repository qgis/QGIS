/**
 * Declaration of ViewModel class.
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

#ifndef _VIEW_MODELHH_
#define _VIEW_MODELHH_


class GFrame;
class GGraph;
class GImage;
class Algorithm;


/****************************************************************/
/*************************** View Model *************************/

/**
 * Shows two dimensions of the model applied in its own domain
 * (eg: abiotic space).
 */

/*************/
class ViewModel
{
public:

  ViewModel( Algorithm *alg, char *name, int dim );
  virtual ~ViewModel();

  // Bottom window function scale.
  void funcScale( float xmin, float ymin, float xmax, float ymax );

  int exec();


private:

  // 
  // Faz todo o desenho para cada passo. Sincroniza as chamadas
  // das funções virtuais.
  void draw();

  // Desenha o mapa.
  void drawMap( GGraph *gr );

  // Estáticas.
  static void funcDraw();
  static void zoomIn( int x, int y );
  static void zoomOut( int x, int y );

  // Para saber quem está executando. Isto limita este objeto a
  // ter apenas uma instância por processo.
  static ViewModel *f_this;

  Algorithm *f_alg;

  GFrame *f_frame;
  GGraph *f_gmap;  // Mapa.
  GImage *f_imap;

  GGraph *f_gfunc; // Função a ser mostrada em baixo do mapa.
  GImage *f_ifunc;
  int     f_cicle;

  double f_zoom;
};


#endif
