#ifndef MOVING_IMG_H
#define MOVING_IMG_H

#include "basics.h"

#include <stack>
#include <queue>
#include <string>
#define ACTION_LEFT 'l'
#define ACTION_RIGHT 'r'
#define ACTION_UP 'u'
#define ACTION_DOWN 'd'
#define ACTION_ROTATE 'R'
#define ACTION_ROTBAK 'B'

// Clase que representa una imagen como una colección de 3 matrices siguiendo el
// esquema de colores RGB

class moving_image {
private:
  unsigned char **red_layer; // Capa de tonalidades rojas
  unsigned char **green_layer; // Capa de tonalidades verdes
  unsigned char **blue_layer; // Capa de tonalidades azules

  std::stack<std::pair<char,int>> st_undo;
  std::stack<std::pair<char,int>> st_redo;

  std::queue<std::pair<char,int>> history;

  void registerAction(char type, int d){
    st_undo.push({type,d});
    if (!st_redo.empty())
      st_redo = std::stack<std::pair<char,int>>();
  }

  void addHistory(char type, int d){
    history.push({type,d});
  }

public:
  // Constructor de la imagen. Se crea una imagen por defecto
  moving_image() {
    // Reserva de memoria para las 3 matrices RGB
    red_layer = new unsigned char*[H_IMG];
    green_layer = new unsigned char*[H_IMG];
    blue_layer = new unsigned char*[H_IMG];

    for(int i=0; i < H_IMG; i++) {
      red_layer[i] = new unsigned char[W_IMG];
      green_layer[i] = new unsigned char[W_IMG];
      blue_layer[i] = new unsigned char[W_IMG];
    }

    // Llenamos la imagen con su color de fondo
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++) {
	red_layer[i][j] = DEFAULT_R;
	green_layer[i][j] = DEFAULT_G;
	blue_layer[i][j] = DEFAULT_B;
      }

    // Dibujamos el objeto en su posición inicial
    for(int i=0; i < 322; i++)
      for(int j=0; j < 256; j++) {
	if(!s_R[i][j] && !s_G[i][j] && !s_B[i][j]) {
	  red_layer[INIT_Y+i][INIT_X+j] = DEFAULT_R;
	  green_layer[INIT_Y+i][INIT_X+j] = DEFAULT_G;
	  blue_layer[INIT_Y+i][INIT_X+j] = DEFAULT_B;
	} else {
	  red_layer[INIT_Y+i][INIT_X+j] = s_R[i][j];
	  green_layer[INIT_Y+i][INIT_X+j] = s_G[i][j];
	  blue_layer[INIT_Y+i][INIT_X+j] = s_B[i][j];
	}
      }
  }

  // Destructor de la clase
  ~moving_image() {
    for(int i=0; i < H_IMG; i++) {
      delete red_layer[i];
      delete green_layer[i];
      delete blue_layer[i];
    }

    delete red_layer;
    delete green_layer;
    delete blue_layer;
  }

  // Función utilizada para guardar la imagen en formato .png
  void draw(const char* nb) {
    _draw(nb);
  }


  void undo(){
    if (st_undo.empty()) return;
    std::pair<char,int> act = st_undo.top(); st_undo.pop();

    char revact = ActionCounter(act.first);

    execAction(revact,act.second);
    addHistory(revact,act.second);

    st_redo.push(act);
  }

  void redo(){
    if (st_redo.empty()) return;
    std::pair<char,int> act = st_redo.top(); st_redo.pop();

    execAction(act.first,act.second);
    addHistory(act.first,act.second);

    st_undo.push(act);
  }

  void repeat(){
    if (st_undo.empty()) return;
    std::pair<char,int> act = st_undo.top();

    registerAction(act.first,act.second);
    addHistory(act.first,act.second);
    execAction(act.first,act.second);

    st_undo.push(act);
  }

  void repeat_all(){
    moving_image clon;

    size_t n = history.size();

    for (int i=0; i<n; i++)
    {
      std::pair<char,int> act = history.front(); history.pop();

      clon.execAction(act.first, act.second);
      clon.draw((std::string("history")+std::to_string(i)+".png").c_str());

      history.push(act);
    }
    
    //clon.~moving_image();
  }

  void move_left(int d){
    registerAction(ACTION_LEFT,d);
    addHistory(ACTION_LEFT,d);
    do_move_left(d);
  }
  void move_right(int d){
    registerAction(ACTION_RIGHT,d);
    addHistory(ACTION_RIGHT,d);
    do_move_right(d);
  }
  void move_up(int d){
    registerAction(ACTION_UP,d);
    addHistory(ACTION_UP,d);
    do_move_up(d);
  }
  void move_down(int d){
    registerAction(ACTION_DOWN,d);
    addHistory(ACTION_DOWN,d);
    do_move_down(d);
  }

  void rotate(){
    registerAction(ACTION_ROTATE,0);
    addHistory(ACTION_ROTATE,0);
    do_rotate();
  }


  private:

  void execAction(char type, int d){
    switch (type)
    {
    case ACTION_LEFT:
      do_move_left(d);
      break;
    case ACTION_RIGHT:
      do_move_right(d);
      break;
    case ACTION_UP:
      do_move_up(d);
      break;
    case ACTION_DOWN:
      do_move_down(d);
      break;

    case ACTION_ROTATE:
      do_rotate();
      break;
    case ACTION_ROTBAK:
      do_rotate_back();
      break;

    default:
      break;
    }
  }

  char ActionCounter(char type){
    switch (type)
    {
    case ACTION_LEFT:
      return ACTION_RIGHT;
    case ACTION_RIGHT:
      return ACTION_LEFT;
    case ACTION_UP:
      return ACTION_DOWN;
    case ACTION_DOWN:
      return ACTION_UP;

    case ACTION_ROTATE:
      return ACTION_ROTBAK;
    case ACTION_ROTBAK:
      return ACTION_ROTATE;

    default:
      return 0;
    }
  }

  // Función que similar desplazar la imagen, de manera circular, d pixeles a la izquierda
  void do_move_left(int d) {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++)
      tmp_layer[i] = new unsigned char[W_IMG];

    // Mover la capa roja
        //mueve los pixeles [d,W_IMG] hasta [0,W_IMG-d]
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
	tmp_layer[i][j] = red_layer[i][j+d];
        //mueve los pixeles [0,d] hasta [W_IMG-d,W_IMG] (por el wrapping)
    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][j] = red_layer[i][k];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	tmp_layer[i][j] = green_layer[i][j+d];

    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][j] = green_layer[i][k];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	tmp_layer[i][j] = blue_layer[i][j+d];

    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][j] = blue_layer[i][k];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	blue_layer[i][j] = tmp_layer[i][j];
  }

  void do_move_down(int d) {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++)
      tmp_layer[i] = new unsigned char[W_IMG];

    // Mover la capa roja
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
	tmp_layer[i+d][j] = red_layer[i][j];

    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[k][j] = red_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i+d][j] = green_layer[i][j];

    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[k][j] = green_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i+d][j] = blue_layer[i][j];

    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[k][j] = blue_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	blue_layer[i][j] = tmp_layer[i][j];
  }

  void do_move_up(int d) {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++)
      tmp_layer[i] = new unsigned char[W_IMG];

    // Mover la capa roja

    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
	tmp_layer[i][j] = red_layer[i+d][j];

    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i][j] = red_layer[k][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i][j] = green_layer[i+d][j];

    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i][j] = green_layer[k][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG-d; i++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i][j] = blue_layer[i+d][j];

    for(int i=H_IMG-d, k=0; i < H_IMG; i++, k++)
      for(int j=0; j < W_IMG; j++)
    	tmp_layer[i][j] = blue_layer[k][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	blue_layer[i][j] = tmp_layer[i][j];
  }

  void do_move_right(int d) {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++)
      tmp_layer[i] = new unsigned char[W_IMG];

    // Mover la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
	tmp_layer[i][j+d] = red_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][k] = red_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	tmp_layer[i][j+d] = green_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][k] = green_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG-d; j++)
    	tmp_layer[i][j+d] = blue_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=W_IMG-d, k=0; j < W_IMG; j++, k++)
    	tmp_layer[i][k] = blue_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	blue_layer[i][j] = tmp_layer[i][j];
  }
  void do_rotate() {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++)
      tmp_layer[i] = new unsigned char[W_IMG];

    // Mover la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[H_IMG-j-1][i] = red_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[H_IMG-j-1][i] = green_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[H_IMG-j-1][i] = blue_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];
  }

  void do_rotate_back() {
    unsigned char **tmp_layer = new unsigned char*[H_IMG];
    for(int i=0; i < H_IMG; i++)
      tmp_layer[i] = new unsigned char[W_IMG];

    // Mover la capa roja
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      tmp_layer[j][W_IMG-i-1] = red_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
	      red_layer[i][j] = tmp_layer[i][j];

    // Mover la capa verde
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[j][W_IMG-i-1] = green_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  green_layer[i][j] = tmp_layer[i][j];

    // Mover la capa azul
    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  tmp_layer[j][W_IMG-i-1] = blue_layer[i][j];

    for(int i=0; i < H_IMG; i++)
      for(int j=0; j < W_IMG; j++)
    	  blue_layer[i][j] = tmp_layer[i][j];
  }

private:

  // Función privada que guarda la imagen en formato .png
  void _draw(const char* nb) {
    //    unsigned char rgb[H_IMG * W_IMG * 3], *p = rgb;
    unsigned char *rgb = new unsigned char[H_IMG * W_IMG * 3];
    unsigned char *p = rgb;
    unsigned x, y;

    // La imagen resultante tendrá el nombre dado por la variable 'nb'
    FILE *fp = fopen(nb, "wb");

    // Transformamos las 3 matrices en una arreglo unidimensional
    for (y = 0; y < H_IMG; y++)
        for (x = 0; x < W_IMG; x++) {
            *p++ = red_layer[y][x];    /* R */
            *p++ = green_layer[y][x];    /* G */
            *p++ = blue_layer[y][x];    /* B */
        }
    // La función svpng() transforma las 3 matrices RGB en una imagen PNG
    svpng(fp, W_IMG, H_IMG, rgb, 0);
    fclose(fp);
}


};

#endif
