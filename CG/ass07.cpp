#include <GL/glut.h>
#include <cmath>

#define W 700
#define H 500
#define M_PI 3.14159265358979323846

class Pendulum {
public:
    float px, py, L, A, w, r, t, bx, by;
    
    Pendulum(float x, float y) : px(x), py(y), L(180), A(0.7), w(0.07), r(18), t(0) {
        bx = px;
        by = py + L;
    }
    
    void move() {
        t += 1;
        float theta = A * sin(w * t);
        bx = px + L * sin(theta);
        by = py + L * cos(theta);
    }
    
    void draw() {
        glColor3f(0,0,0);
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex2f(px, py);
        glVertex2f(bx, by);
        glEnd();
        
        glColor3f(0.12, 0.55, 1);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(bx, by);
        for(int i = 0; i <= 360; i += 10)
            glVertex2f(bx + r * cos(i * M_PI/180), by + r * sin(i * M_PI/180));
        glEnd();
        
        glColor3f(0,0,0);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(px, py);
        for(int i = 0; i <= 360; i += 10)
            glVertex2f(px + 6 * cos(i * M_PI/180), py + 6 * sin(i * M_PI/180));
        glEnd();
    }
};

Pendulum p(W/2, 80);

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    p.move();
    p.draw();
    glutSwapBuffers();
}

void timer(int) {
    glutPostRedisplay();
    glutTimerFunc(20, timer, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(W, H);
    glutCreateWindow("Pendulum");
    glClearColor(1,1,1,1);
    gluOrtho2D(0, W, H, 0);
    glutDisplayFunc(display);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}