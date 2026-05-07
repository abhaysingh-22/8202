#include <GL/glut.h>
#include <cmath>
#include <iostream>
using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
float x1_, y1_, x2_, y2_;
int depth;
void koch(float x1, float y1, float x2, float y2, int d)
{
    if (d == 0)
    {
        glVertex2f(x2, y2); // only push
        endpoint return;
    }
    float xA = x1 + (x2 - x1) / 3;
    float yA = y1 + (y2 - y1) / 3;
    float xB = x1 + 2 * (x2 - x1) / 3;
    float yB = y1 + 2 * (y2 - y1) / 3;

    float angle = M_PI / 3;
    float xC = xA + (xB - xA) * cos(angle) -
               (yB - yA) * sin(angle);
    float yC = yA + (xB - xA) * sin(angle) +
               (yB - yA) * cos(angle);
    koch(x1, y1, xA, yA, d - 1);
    koch(xA, yA, xC, yC, d - 1);
    koch(xC, yC, xB, yB, d - 1);
    koch(xB, yB, x2, y2, d - 1);
}
void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1, 1, 1);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x1_, y1_); // start point
    koch(x1_, y1_, x2_, y2_, depth);
    glEnd();

    glFlush();
}
void init()
{
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 500, 0, 500);
}
int main(int argc, char **argv)
{
    cout << "Enter x1 y1 (pixel): ";
    cin >> x1_ >> y1_;
    cout << "Enter x2 y2 (pixel): ";
    cin >> x2_ >> y2_;
    cout << "Enter depth (0,1,2...): ";
    cin >> depth;
    y1_ = 500 - y1_;
    y2_ = 500 - y2_;
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_SINGLE |
                        GLUT_RGB);
    glutInitWindowSize(500, 500);
glutCreateWindow("Koch Curve -
Continuous");
init();
glutDisplayFunc(display);
glutMainLoop();
return 0;
}