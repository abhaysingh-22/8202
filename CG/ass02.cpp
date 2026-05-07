#include <iostream>
#include <cmath>
#include <vector>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

using namespace std;

// Structure to store circle data
struct Circle {
    int centerX, centerY;
    int radius;
};

// Global variables
vector<Circle> circles;
int centerX = -1, centerY = -1;  // For tracking first click (center)
bool selectingCenter = true;

// Midpoint Circle Drawing Algorithm
void drawCircleMidpoint(int centerX, int centerY, int radius) {
    int x = 0;
    int y = radius;
    int d = 3 - 2 * radius;  // Decision parameter
    
    glBegin(GL_POINTS);
    glColor3f(0.0f, 0.0f, 1.0f); // Blue color for circles (better on light background)
    
    while (x <= y) {
        // Plot 8 symmetric points
        glVertex2f((float)(centerX + x), (float)(centerY + y));
        glVertex2f((float)(centerX - x), (float)(centerY + y));
        glVertex2f((float)(centerX + x), (float)(centerY - y));
        glVertex2f((float)(centerX - x), (float)(centerY - y));
        glVertex2f((float)(centerX + y), (float)(centerY + x));
        glVertex2f((float)(centerX - y), (float)(centerY + x));
        glVertex2f((float)(centerX + y), (float)(centerY - x));
        glVertex2f((float)(centerX - y), (float)(centerY - x));
        
        // Update decision parameter and coordinates
        if (d < 0) {
            d = d + 4 * x + 6;
        } else {
            d = d + 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
    
    glEnd();
}

// Display function
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);  // White background (light mode)
    
    glPointSize(2.0f);
    
    // Draw all circles
    for (const auto& circle : circles) {
        drawCircleMidpoint(circle.centerX, circle.centerY, circle.radius);
    }
    
    // Draw circle centers as red dots
    glPointSize(6.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 0.0f); // Red for centers (stays red on white)
    for (const auto& circle : circles) {
        glVertex2f((float)circle.centerX, (float)circle.centerY);
    }
    glEnd();
    
    // Draw temporary center if waiting for radius selection
    if (centerX != -1 && centerY != -1 && !selectingCenter) {
        glPointSize(6.0f);
        glBegin(GL_POINTS);
        glColor3f(0.0f, 0.7f, 0.0f); // Green for temporary center (darker green on white)
        glVertex2f((float)centerX, (float)centerY);
        glEnd();
    }
    
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
        case 'C':
        case 'c':
            circles.clear();
            centerX = -1;
            centerY = -1;
            selectingCenter = true;
            break;
        case 'R':
        case 'r':
            if (centerX != -1 && centerY != -1) {
                centerX = -1;
                centerY = -1;
                selectingCenter = true;
            }
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
        if (selectingCenter) {
            // First click - set center
            centerX = x;
            centerY = y;
            selectingCenter = false;
        } else {
            // Second click - set radius and draw circle
            int radius = (int)sqrt(pow(x - centerX, 2) + pow(y - centerY, 2));
            circles.push_back({centerX, centerY, radius});
            
            centerX = -1;
            centerY = -1;
            selectingCenter = true;
        }
        
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
    
    glutCreateWindow("Midpoint Circle Drawing Algorithm");
    
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
