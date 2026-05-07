#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

using namespace std;

struct Point { float x, y; };

vector<Point> polygon;
vector<float> colors[] = {{1,0,0}, {0,1,0}, {0,0,1}, {0,1,1}, {1,0,1}, {1,1,0}};
int colorIdx = 0;
bool filled = false;

void scanlineFill() {
    if (polygon.size() < 3) return;
    
    float minY = polygon[0].y, maxY = polygon[0].y;
    for (auto& p : polygon) {
        minY = min(minY, p.y);
        maxY = max(maxY, p.y);
    }
    
    glBegin(GL_POINTS);
    glColor3fv(&colors[colorIdx][0]);
    
    for (int y = (int)minY; y <= (int)maxY; y++) {
        vector<float> x_intersects;
        
        for (size_t i = 0; i < polygon.size(); i++) {
            size_t j = (i + 1) % polygon.size();
            float y1 = polygon[i].y, y2 = polygon[j].y;
            float x1 = polygon[i].x, x2 = polygon[j].x;
            
            if ((y1 <= y && y < y2) || (y2 <= y && y < y1)) {
                float x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
                x_intersects.push_back(x);
            }
        }
        
        sort(x_intersects.begin(), x_intersects.end());
        
        for (size_t i = 0; i + 1 < x_intersects.size(); i += 2) {
            for (int x = (int)x_intersects[i]; x <= (int)x_intersects[i + 1]; x++) {
                glVertex2f((float)x, (float)y);
            }
        }
    }
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    if (filled && polygon.size() >= 3) {
        scanlineFill();
    }
    
    glPointSize(2.0f);
    if (polygon.size() > 1) {
        glBegin(GL_LINE_STRIP);
        glColor3f(0, 0, 0);
        for (auto& p : polygon) glVertex2f(p.x, p.y);
        if (filled) glVertex2f(polygon[0].x, polygon[0].y);
        glEnd();
    }
    
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(1, 0, 0);
    for (auto& p : polygon) glVertex2f(p.x, p.y);
    glEnd();
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'F':
        case 'f':
            if (!filled && polygon.size() >= 3) {
                filled = true;
            }
            break;
        case 'K':
        case 'k':
            if (filled) {
                colorIdx = (colorIdx + 1) % 6;
            }
            break;
        case 'U':
        case 'u':
            if (!filled && polygon.size() > 0) {
                polygon.pop_back();
            }
            break;
        case 'C':
        case 'c':
            polygon.clear();
            filled = false;
            break;
        case 27:
            exit(0);
    }
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!filled) {
            polygon.push_back({(float)x, (float)y});
            
            // Auto-fill when 3rd point is added
            if (polygon.size() == 3) {
                filled = true;
            }
            
            glutPostRedisplay();
        }
    }
}

void idle() { glutPostRedisplay(); }

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(900, 700);
    glutCreateWindow("Scanline Polygon Fill");
    
    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 900, 700, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);
    
    glutMainLoop();
    return 0;
}