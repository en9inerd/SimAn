#include "visualization.h"
#include <tuple>

extern DataPlace rbplace;

float moving_x = 0, moving_y = 0, zoom = 0;
int Rotate = 0, origx = 0, origy = 0, angle = 0;

void draw_circle(double x, double y, double r) {
    const int t = 50;  // на сколько частей делится окружность
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(x, y);  //центр окружности
    for (int i = 0; i <= t; i++) {
        float a = (float)i / (float)t * 3.1415f * 2.0f;
        glVertex2f(x - cos(a) * r, y - sin(a) * r);
    }
    glEnd();
}

void draw_quads(double x, double y, double a) {
    glBegin(GL_QUADS);
    glVertex2d(x - 0.5 * a, y + 0.5 * a);
    glVertex2d(x + 0.5 * a, y + 0.5 * a);
    glVertex2d(x + 0.5 * a, y - 0.5 * a);
    glVertex2d(x - 0.5 * a, y - 0.5 * a);
    glEnd();
}

void draw_cross(double x, double y, double a) {
    glBegin(GL_LINES);
    glVertex2f(x - 0.5 * a, y - 0.5 * a);
    glVertex2f(x + 0.5 * a, y + 0.5 * a);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(x - 0.5 * a, y + 0.5 * a);
    glVertex2f(x + 0.5 * a, y - 0.5 * a);
    glEnd();
}

void draw_scene() {
    vector<node>& nd = rbplace.nodes;
    vector<terminal>& tr = rbplace.terminals;
    vector<row>& rw = rbplace.rows;

    BBox layoutBBox(rbplace, true);
    double Xmin = layoutBBox.xMin;
    double Xmax = layoutBBox.xMax;
    double Ymin = layoutBBox.yMin;
    double Ymax = layoutBBox.yMax;

    BBox Core(rbplace);

    float sc = 0.1503 * fabs(Core.getHeight());

    // double midX = 0.5*abs(Xmin - Xmax);
    // double midY = 0.5*abs(Ymin - Ymax);
    // glTranslated(midX, midY, 0);

    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex2f(Xmin / sc, Ymin / sc);
    glVertex2f(Xmax / sc, Ymin / sc);
    glVertex2f(Xmax / sc, Ymax / sc);
    glVertex2f(Xmin / sc, Ymax / sc);
    glEnd();

    glLineWidth(3);
    glColor3f(0.5, 0.5, 0.5);
    glBegin(GL_LINE_LOOP);
    glVertex2f(Xmin / sc, Ymin / sc);
    glVertex2f(Xmax / sc, Ymin / sc);
    glVertex2f(Xmax / sc, Ymax / sc);
    glVertex2f(Xmin / sc, Ymax / sc);
    glEnd();

    glLineWidth(1);
    glColor3f(0, 1.0, 0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(Core.xMin / sc, Core.yMin / sc);
    glVertex2f(Core.xMax / sc, Core.yMin / sc);
    glVertex2f(Core.xMax / sc, Core.yMax / sc);
    glVertex2f(Core.xMin / sc, Core.yMax / sc);
    glEnd();

    for (vector<row>::iterator itR = rw.begin(); itR != rw.end(); itR++) {
        glBegin(GL_LINE_LOOP);
        glVertex2f(itR->coord_x / sc, itR->coord_y / sc);
        glVertex2f((itR->coord_x + itR->num_sites * itR->site_sp) / sc,
                   itR->coord_y / sc);
        glVertex2f((itR->coord_x + itR->num_sites * itR->site_sp) / sc,
                   (itR->coord_y + itR->h) / sc);
        glVertex2f(itR->coord_x / sc, (itR->coord_y + itR->h) / sc);
        glEnd();
    }

    if (!tr.empty())
        for (vector<terminal>::iterator it = tr.begin(); it != tr.end(); it++) {
            glColor3f(1.0, 0, 0);
            glBegin(GL_POLYGON);
            glVertex2f(it->pos_x / sc, it->pos_y / sc);
            glVertex2f((it->pos_x + it->w) / sc, it->pos_y / sc);
            glVertex2f((it->pos_x + it->w) / sc, (it->pos_y + it->h) / sc);
            glVertex2f(it->pos_x / sc, (it->pos_y + it->h) / sc);
            glEnd();
        }

    for (vector<node>::iterator it = nd.begin(); it != nd.end(); it++) {
        glColor3f(1.0, 0, 0);
        glBegin(GL_LINE_LOOP);
        glVertex2f(it->pos_x / sc, it->pos_y / sc);
        glVertex2f((it->pos_x + it->w) / sc, it->pos_y / sc);
        glVertex2f((it->pos_x + it->w) / sc, (it->pos_y + it->h) / sc);
        glVertex2f(it->pos_x / sc, (it->pos_y + it->h) / sc);
        glEnd();
    }
}

void render_scene() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double aspect = (double)glutGet(GLUT_WINDOW_WIDTH) / (double)glutGet(GLUT_WINDOW_HEIGHT);
    double viewSize = 200.0 * (1.0 + zoom * 0.1);
    if (aspect >= 1.0) {
        glOrtho(-viewSize * aspect, viewSize * aspect, -viewSize, viewSize, -100, 100);
    } else {
        glOrtho(-viewSize, viewSize, -viewSize / aspect, viewSize / aspect, -100, 100);
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(moving_x, moving_y, 0);
    glRotatef(angle, 0, 0, 1);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    draw_scene();

    glutSwapBuffers();
}

void keyboard_control(unsigned char key, int, int) {
    // поворот по часовой стрелке //
    if (key == 'a') {
        angle -= 10;
        glutPostRedisplay();
    }
    // поворот против часовой стрелки //
    if (key == 's') {
        angle += 10;
        glutPostRedisplay();
    }
    // вернуться к исходному виду //
    if (key == 'r') {
        angle = 0;
        zoom = 0;
        moving_x = moving_y = 0;
        glutPostRedisplay();
    }
    // приблизить //
    if (key == '+') {
        zoom += 1;
        glutPostRedisplay();
    }
    // отдалить //
    if (key == '-') {
        zoom -= 1;
        glutPostRedisplay();
    }
}

void mouse_control(int button, int state, int x, int y) {
    switch (button) {
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN) {
                origx = x;
                origy = y;
                Rotate = 1;
            } else
                Rotate = 0;
            break;
    }
}

void mouse_move(int x, int y) {
    if (Rotate) {
        int k = zoom <= 0 ? 60 : 40 * zoom;
        moving_x += (float)(x - origx) / k;
        moving_y -= (float)(y - origy) / k;
        origx = x;
        origy = y;
        glutPostRedisplay();
    }
}

void timf(int value) {
    glutPostRedisplay();
    glutTimerFunc(40, timf, 0);
}

void opengl_control(int argcp, char** argv) {
    glutInit(&argcp, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Placement 2D Visualization");
    glutDisplayFunc(render_scene);
    glutKeyboardFunc(keyboard_control);
    glutMouseFunc(mouse_control);
    glutMotionFunc(mouse_move);
    glutMainLoop();
}