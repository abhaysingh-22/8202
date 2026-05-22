#include <GLUT/glut.h>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
using namespace std;

float x1_, y1_, x2_, y2_;
int depth;
int step = 0;  // 0 = waiting for input, 1 = showing point, 2 = showing line, 3 = showing fractal

void koch(float x1, float y1, float x2, float y2, int d) 
{ 
    if (d == 0) { 
        glVertex2f(x1, y1);
        glVertex2f(x2, y2);
        return; 
    } 
 
    float dx = (x2 - x1) / 3.0f; 
    float dy = (y2 - y1) / 3.0f; 
 
    float xA = x1 + dx; 
    float yA = y1 + dy; 
 
    float xB = x1 + 2 * dx; 
    float yB = y1 + 2 * dy; 
 
    float angle = M_PI / 3; 
    float xPeak = xA + (dx * cos(angle) - dy * sin(angle)); 
    float yPeak = yA + (dx * sin(angle) + dy * cos(angle)); 
 
    koch(x1, y1, xA, yA, d - 1); 
    koch(xA, yA, xPeak, yPeak, d - 1); 
    koch(xPeak, yPeak, xB, yB, d - 1); 
    koch(xB, yB, x2, y2, d - 1); 
} 
 
void drawSnowflake() 
{ 
    glColor3f(0.1, 0.1, 0.6);  // Blue lines 
    
    if (step == 1) {
        glPointSize(20.0f);
        glBegin(GL_POINTS);
        glVertex2f(x1_, y1_);
        glEnd();
    }
    else if (step == 2) {
        glBegin(GL_LINES);
        glVertex2f(x1_, y1_);
        glVertex2f(x2_, y2_);
        glEnd();
    }
    else if (step == 3) {
        glBegin(GL_LINES);
        koch(x1_, y1_, x2_, y2_, depth); 
        glEnd();
    }
} 
 
void display() 
{ 
    glClear(GL_COLOR_BUFFER_BIT); 
    drawSnowflake(); 
    glFlush(); 
} 

void keyboard(unsigned char key, int x, int y) {
    if (key == 'q' || key == 'Q') {
        cout << "\nExiting program..." << endl;
        exit(0);
    }
    else if (key == 'n' || key == 'N' || key == ' ') {
        // Next button - advance to next step
        step++;
        if (step > 3) {
            cout << "Process complete! Press 'Q' to quit." << endl;
            step = 3;
            return;
        }
        
        if (step == 1) {
            cout << "\n>>> STEP 1: Showing Point 1 (" << x1_ << ", " << y1_ << ")" << endl;
        }
        else if (step == 2) {
            cout << "\n>>> STEP 2: Showing Line from (" << x1_ << ", " << y1_ << ") to (" << x2_ << ", " << y2_ << ")" << endl;
        }
        else if (step == 3) {
            cout << "\n>>> STEP 3: Showing Koch Fractal (Depth " << depth << ")" << endl;
        }
        
        glutPostRedisplay();
    }
}

void init() 
{ 
    glClearColor(1, 1, 1, 1);
    glColor3f(0, 0, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.5, 1.5, -1.5, 1.5);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
} 
 
int main(int argc, char** argv) 
{ 
    string line;
    
    cout << "\n========== KOCH CURVE FRACTAL - STEP BY STEP ==========" << endl;
    cout << "\nEnter values separated by SPACES or COMMAS:" << endl;
    
    // Read x1, y1
    cout << "Enter x1 y1 (e.g., -0.5 0.5 or -0.5,0.5): ";
    getline(cin, line);
    // Replace commas with spaces
    for (char &c : line) if (c == ',') c = ' ';
    stringstream ss1(line);
    ss1 >> x1_ >> y1_;
    
    // Read x2, y2
    cout << "Enter x2 y2 (e.g., 0.5 0.5 or 0.5,0.5): ";
    getline(cin, line);
    for (char &c : line) if (c == ',') c = ' ';
    stringstream ss2(line);
    ss2 >> x2_ >> y2_;
    
    // Read depth
    cout << "Enter depth (0, 1, 2, ...): ";
    getline(cin, line);
    stringstream ss3(line);
    ss3 >> depth;
    
    glutInit(&argc, argv); 
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB); 
    glutInitWindowSize(900, 900); 
    glutCreateWindow("Koch Curve - Step by Step Visualization"); 
 
    init(); 
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    
    cout << "\n========== WINDOW OPENED ==========" << endl;
    cout << "\nPress SPACE or 'N' to show next step" << endl;
    cout << "Press 'Q' to quit" << endl;
    cout << "\nPoint 1: (" << x1_ << ", " << y1_ << ")" << endl;
    cout << "Point 2: (" << x2_ << ", " << y2_ << ")" << endl;
    cout << "Fractal Depth: " << depth << endl;
    
    glutMainLoop(); 
    
    return 0; 
}