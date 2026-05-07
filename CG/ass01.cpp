#include <iostream>
#include <cmath>
#include <vector>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

using namespace std;

// Structure to store points
struct Point {
    int x, y;
};

// Global variables
vector<Point> points;
int lineAlgorithm = 0; // 0 = DDA, 1 = Bresenham

// DDA Line Drawing Algorithm
void drawLineDDA(int x0, int y0, int x1, int y1) {
    int dx = x1 - x0;
    int dy = y1 - y0;
    
    int steps = max(abs(dx), abs(dy));
    
    if (steps == 0) {
        glBegin(GL_POINTS);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex2f((float)x0, (float)y0);
        glEnd();
        return;
    }
    
    float xIncrement = (float)dx / steps;
    float yIncrement = (float)dy / steps;
    
    float x = x0;
    float y = y0;
    
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color for DDA (stays red on white)
    
    for (int i = 0; i <= steps; i++) {
        glVertex2f(x, y);
        x += xIncrement;
        y += yIncrement;
    }
    
    glEnd();
}

// Bresenham Line Drawing Algorithm
void drawLineBresenham(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    
    int err = dx - dy;
    int x = x0;
    int y = y0;
    
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.5f, 1.0f); // Blue color for Bresenham (better visibility on white)
    
    while (true) {
        glVertex2f((float)x, (float)y);
        
        if (x == x1 && y == y1) break;
        
        int e2 = 2 * err;
        
        if (e2 > -dy) {
            err = err - dy;
            x = x + sx;
        }
        
        if (e2 < dx) {
            err = err + dx;
            y = y + sy;
        }
    }
    
    glEnd();
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background (light mode)
    
    glPointSize(2.0f);
    
    // Draw all complete line pairs
    for (size_t i = 0; i + 1 < points.size(); i += 2) {
        if (lineAlgorithm == 0) {
            drawLineDDA(points[i].x, points[i].y, points[i+1].x, points[i+1].y);
        } else {
            drawLineBresenham(points[i].x, points[i].y, points[i+1].x, points[i+1].y);
        }
    }
    
    // Draw all endpoints as dark dots
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(0.2f, 0.2f, 0.2f); // Dark gray/black for endpoints
    for (const auto& p : points) {
        glVertex2f((float)p.x, (float)p.y);
    }
    glEnd();
    
    glPointSize(1.0f);
    glutSwapBuffers();
}

// Reshape function
void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Keyboard function
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'D':
        case 'd':
            lineAlgorithm = 0;
            break;
        case 'B':
        case 'b':
            lineAlgorithm = 1;
            break;
        case 'C':
        case 'c':
            points.clear();
            break;
        case 27: // ESC key
            exit(0);
            break;
    }
    
    glutPostRedisplay();
}

// Mouse function for click events
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        points.push_back({x, y});
        glutPostRedisplay();
    }
}

// Idle function
void idle() {
    glutPostRedisplay();
}



// Main function
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    
    glutCreateWindow("Line Drawing Algorithms - DDA vs Bresenham");
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 600, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Register callback functions
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);
    
    glutMainLoop();
    
    return 0;
}
