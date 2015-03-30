#include "stdafx.h"
#include "visualization.h"
#include <tuple>

extern DataPlace rbplace;

float moving_x = 0, moving_y = 0, zoom = 0;
int Rotate = 0, origx = 0, origy = 0, angle = 0;

void draw_circle(double x, double y, double r)
{
	const int t = 50; // на сколько частей делится окружность
	glBegin(GL_TRIANGLE_FAN);
        glVertex2f( x, y ); //центр окружности
        for(int i = 0; i <= t; i++ ) 
		{
            float a = (float)i / (float)t * 3.1415f * 2.0f;
            glVertex2f( x-cos( a ) * r, y-sin( a ) * r );
        }
    glEnd();
}

void draw_quads(double x, double y, double a)
{
    glBegin(GL_QUADS);
    glVertex2d(x-0.5*a,y+0.5*a);
    glVertex2d(x+0.5*a,y+0.5*a);
    glVertex2d(x+0.5*a,y-0.5*a);
    glVertex2d(x-0.5*a,y-0.5*a);
    glEnd();
}

void draw_cross(double x, double y, double a)
{
	glBegin(GL_LINES);
		glVertex2f(x-0.5*a, y-0.5*a);
		glVertex2f(x+0.5*a, y+0.5*a);
	glEnd();

	glBegin(GL_LINES);
		glVertex2f(x-0.5*a, y+0.5*a);
		glVertex2f(x+0.5*a, y-0.5*a);
	glEnd();
}

void draw_scene()
{
	vector<node>& nd = rbplace.nodes;
	vector<terminal>& tr = rbplace.terminals;
	vector<row>& rw = rbplace.rows;

	//vector<terminal>::iterator termMinY, termMaxY;
	//vector<terminal>::iterator termMinX, termMaxX;
	//if(!_tr.empty())
	//{
	//	tie(termMinY, termMaxY) = minmax_element(rb.terminals.begin(), rb.terminals.end(),
	//		[] (terminal const& s1, terminal const& s2)
	//		{
	//			return s1.pos_y < s2.pos_y;
	//		});

	//	tie(termMinX, termMaxX) = minmax_element(rb.terminals.begin(), rb.terminals.end(),
	//		[] (terminal const& s1, terminal const& s2)
	//		{
	//			return s1.pos_x < s2.pos_x;
	//		});
	//}

	//double CoreXmin = _rw.begin()->coord_x;
	//double CoreYmin = _rw.begin()->coord_y;
	//double CoreXmax = CoreXmin + (_rw.begin()->num_sites) * (_rw.begin()->site_sp);
	//double CoreYmax = CoreYmin + rb.NumRows * _rw.begin()->h;

	BBox layoutBBox(rbplace, true);
	double Xmin = layoutBBox.xMin;
	double Xmax = layoutBBox.xMax;
	double Ymin = layoutBBox.yMin;
	double Ymax = layoutBBox.yMax;


	//if(!_tr.empty())
	//{
	//	Ymin = (termMinY->pos_y > CoreYmin) ? CoreYmin - _rw.begin()->site_sp : termMinY->pos_y;
	//	Ymax = (termMaxY->pos_y > CoreYmax) ? termMaxY->pos_y + termMaxX->h : CoreYmax + _rw.begin()->site_sp;
	//	Xmin = (termMinX->pos_x > CoreXmin) ? CoreXmin - _rw.begin()->site_sp : termMinX->pos_x;
	//	Xmax = (termMaxX->pos_x > CoreXmax) ? termMaxX->pos_x + termMaxX->w : CoreXmax + _rw.begin()->site_sp;
	//}
	//else
	//{
	//	Ymin = CoreYmin - _rw.begin()->site_sp;
	//	Xmin = CoreXmin - _rw.begin()->site_sp;
	//	Ymax = CoreYmax + _rw.begin()->site_sp;
	//	Xmax = CoreXmax + _rw.begin()->site_sp;
	//}

	BBox Core(rbplace);

	float sc = 0.1503 * fabs(Core.getHeight());

	//double midX = 0.5*abs(Xmin - Xmax);
	//double midY = 0.5*abs(Ymin - Ymax);
	//glTranslated(midX, midY, 0);

	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_POLYGON);
		glVertex2f(Xmin/sc, Ymin/sc);
		glVertex2f(Xmax/sc, Ymin/sc);
		glVertex2f(Xmax/sc, Ymax/sc);
		glVertex2f(Xmin/sc, Ymax/sc);
    glEnd();

	glLineWidth(3);
	glColor3f(0.5, 0.5, 0.5);
	glBegin(GL_LINE_LOOP);
		glVertex2f(Xmin/sc, Ymin/sc);
		glVertex2f(Xmax/sc, Ymin/sc);
		glVertex2f(Xmax/sc, Ymax/sc);
		glVertex2f(Xmin/sc, Ymax/sc);
    glEnd();

	glLineWidth(1);
	glColor3f(0, 1.0, 0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(Core.xMin/sc, Core.yMin/sc);
		glVertex2f(Core.xMax/sc, Core.yMin/sc);
		glVertex2f(Core.xMax/sc, Core.yMax/sc);
		glVertex2f(Core.xMin/sc, Core.yMax/sc);
    glEnd();

	for(vector<row>::iterator itR = rw.begin(); itR != rw.end(); itR++)
	{
		glBegin(GL_LINE_LOOP);
			glVertex2f( itR->coord_x/sc, itR->coord_y/sc );
			glVertex2f( (itR->coord_x + itR->num_sites*itR->site_sp)/sc, itR->coord_y/sc );
			glVertex2f( (itR->coord_x + itR->num_sites*itR->site_sp)/sc, (itR->coord_y + itR->h)/sc );
			glVertex2f( itR->coord_x/sc, (itR->coord_y + itR->h)/sc );
		glEnd();
	}

	if(!tr.empty())
		for(vector<terminal>::iterator it = tr.begin(); it != tr.end(); it++)
		{
			glColor3f(1.0, 0, 0);
			glBegin(GL_POLYGON);
				glVertex2f( it->pos_x/sc, it->pos_y/sc);
				glVertex2f( (it->pos_x + it->w)/sc, it->pos_y/sc);
				glVertex2f( (it->pos_x + it->w)/sc, (it->pos_y + it->h)/sc );
				glVertex2f( it->pos_x/sc, (it->pos_y + it->h)/sc );
			glEnd();
		}

	for(vector<node>::iterator it = nd.begin(); it != nd.end(); it++)
	{
		glColor3f(1.0, 0, 0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(it->pos_x/sc, it->pos_y/sc);
			glVertex2f( (it->pos_x + it->w)/sc, it->pos_y/sc);
			glVertex2f( (it->pos_x + it->w)/sc, (it->pos_y + it->h)/sc );
			glVertex2f(it->pos_x/sc, (it->pos_y + it->h)/sc );
		glEnd();
	}

	/*double midX = 0.5*abs(DB.area[1][0] - DB.area[0][0])/1e6;
	double midY = 0.5*abs(DB.area[1][1] - DB.area[0][1])/1e6;
	glTranslated(-midX, -midY, 0);

	// отрисовка поверхности //
	glColor3f(0.8, 0.8, 0.8);
	glBegin(GL_POLYGON);
		glVertex2f(DB.area[0][0]/1e6, DB.area[0][1]/1e6);
		glVertex2f(DB.area[0][0]/1e6, DB.area[1][1]/1e6);
		glVertex2f(DB.area[1][0]/1e6, DB.area[1][1]/1e6);
		glVertex2f(DB.area[1][0]/1e6, DB.area[0][1]/1e6);
    glEnd();
	glLineWidth(2);
	glColor3f(0, 0, 0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(DB.area[0][0]/1e6, DB.area[0][1]/1e6);
		glVertex2f(DB.area[0][0]/1e6, DB.area[1][1]/1e6);
		glVertex2f(DB.area[1][0]/1e6, DB.area[1][1]/1e6);
		glVertex2f(DB.area[1][0]/1e6, DB.area[0][1]/1e6);
    glEnd();

	// отрисовка препятствий //
	glColor3f(0.2, 0.2, 0.2);
	for(int i=0; i<DB.blockage.size(); i++)
	{
		glBegin(GL_POLYGON);
			glVertex2f(DB.blockage[i][0]/1e6, DB.blockage[i][1]/1e6);
			glVertex2f(DB.blockage[i][0]/1e6, DB.blockage[i][3]/1e6);
			glVertex2f(DB.blockage[i][2]/1e6, DB.blockage[i][3]/1e6);
			glVertex2f(DB.blockage[i][2]/1e6, DB.blockage[i][1]/1e6);
		glEnd();
	}
	glColor3f(0, 0, 0);
	for(int i=0; i<DB.blockage.size(); i++)
	{
		glBegin(GL_LINE_LOOP);
			glVertex2f(DB.blockage[i][0]/1e6, DB.blockage[i][1]/1e6);
			glVertex2f(DB.blockage[i][0]/1e6, DB.blockage[i][3]/1e6);
			glVertex2f(DB.blockage[i][2]/1e6, DB.blockage[i][3]/1e6);
			glVertex2f(DB.blockage[i][2]/1e6, DB.blockage[i][1]/1e6);
		glEnd();
	}

	// отрисовка цепей //
	glLineWidth(1.5);
	glColor3f(0, 0, 0);
	if(wire_on)
	{
		if(Struct_Tree.get_root() != NULL)
		{
			draw_wire(Struct_Tree.get_root());
			glBegin(GL_LINES);
				glVertex2f(DB.source_base.x/1e6, DB.source_base.y/1e6);
				glVertex2f(DB[Struct_Tree.get_root()->info]->x/1e6, DB[Struct_Tree.get_root()->info]->y/1e6);
			glEnd();
		}
	}

	// отрисовка источника //
	glColor3f(0, 0.5, 1);
	draw_quads(DB.source_base.x/1e6, DB.source_base.y/1e6, 0.07);

	// отрисовка приемников //
	glColor3f(1, 0, 0);
	for(int i=0; i<DB.Ns; i++)
		draw_cross(DB[i]->x/1e6, DB[i]->y/1e6, 0.05);

	// отрисовка промежуточных точек //
	if(tap_point_on)
	{
		glColor3f(1, 1, 0.1);
		for(int i=DB.Ns; i<DB.points.size(); i++)
			draw_quads(DB[i]->x/1e6, DB[i]->y/1e6, 0.03);
	}

	glTranslated(midX, midY, 0);*/
}

void render_scene()
{
	glClearColor(255, 255, 255, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, glutGet(GLUT_WINDOW_WIDTH)/glutGet(GLUT_WINDOW_HEIGHT), 0.1, 100);
	glTranslated(moving_x, moving_y, -10+zoom);
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

void keyboard_control(unsigned char key, int, int)
{
	// поворот по часовой стрелке //
	if(key=='a')
	{
		angle-=10;
		glutPostRedisplay();
	}
	// поворот против часовой стрелки //
    if(key=='s')
	{
		angle+=10;
		glutPostRedisplay();
	}
	// вернуться к исходному виду //
	if(key=='r')
	{
		angle=0;
		zoom=0;
		moving_x = moving_y = 0;
		glutPostRedisplay();
	}
	// приблизить //
    if(key=='+')
	{
		zoom+=1;
		glutPostRedisplay();
	}
	// отдалить //
    if(key=='-')
	{
		zoom-=1;
		glutPostRedisplay();
	}
}

void mouse_control(int button, int state, int x, int y)
{
    switch(button){
        case GLUT_LEFT_BUTTON:
            if (state == GLUT_DOWN)
			{
                origx = x;
                origy = y;
	            Rotate = 1;
			}
			else Rotate = 0;
			break;
	}
}

void mouse_move(int x, int y)
{
    if(Rotate)
    {
		int k = zoom<=0 ? 60 : 40*zoom;
        moving_x += (float)(x - origx)/k;
        moving_y -= (float)(y - origy)/k;
        origx = x;
        origy = y;
        glutPostRedisplay();
    }
}

void timf(int value)
{
	glutPostRedisplay();  
	glutTimerFunc(40, timf, 0); 
}

void opengl_control(int argcp, char **argv)
{
	glutInit(&argcp, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(800,800);
	glutCreateWindow("Placement 2D Visualization");
	glutDisplayFunc(render_scene);
	glutKeyboardFunc(keyboard_control);
	glutMouseFunc(mouse_control);
	glutMotionFunc(mouse_move);
	glutMainLoop();
}