#ifndef _VISUALIZATION_H_
#define _VISUALIZATION_H_

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <cmath>
#include "BBox.h"
#include "DataPlace.h"

void draw_circle(double x, double y, double r);  // отрисовывает окружность по
// заданным координатам центра
// и радиусу
void draw_quads(double x, double y, double a);  // отрисовывает квадрат по
// заданным координатам центра и
// длине стороны
void draw_cross(double x, double y, double k);  // отрисовывает крест по
// заданным координатам центра и
// коэффициенту масштабировани¤
void draw_scene();    // отрисовывает основную поверхность, на которой в
                      // дальнейшем будут располагатьс¤ все элементы
void render_scene();  // функци¤, внутри которой вызываютс¤ все функции OpenGL,
                      // отвечающие за отрисовку
void keyboard_control(unsigned char key, int, int);  // функци¤, осуществл¤юща¤
// управление параметрами
// отображени¤ с помощью
// клавиатуры
void mouse_control(int button, int state, int x,
                   int y);  // функци¤, осуществл¤юща¤ управление параметрами
                            // отображени¤ с помощью мыши
void mouse_move(int x, int y);                // функци¤, осуществл¤юща¤ изменение параметров
                                              // отображени¤ в соответствии с перемещением
                                              // мыши
void timf(int value);                         // функци¤, отвечающа¤ за автоматическое вращение сцены
void opengl_control(int argcp, char **argv);  // функци¤, внутри которой
// осуществл¤етс¤ вызов всех
// св¤занных с OpenGL функций

#endif