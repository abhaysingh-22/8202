#include <iostream>
#include <vector>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

using namespace std;

// Window dimensions
const int WIDTH = 800, HEIGHT = 600;

// Clipping window bounds
float clipLeft = 150, clipRight = 650, clipTop = 150, clipBottom = 450;

// Region codes
const int INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8;

// Points for line drawing
vector<pair<float, float>> linePoints;
vector<pair<pair<float, float>, pair<float, float>>> randomLines;
bool windowDefined = false;
int colorMode = 0;
bool definingWindow = false;

// Generate random lines
void generateRandomLines() {
    randomLines.clear();
    for (int i = 0; i < 3; i++) {
        float x1 = rand() % WIDTH;
        float y1 = rand() % HEIGHT;
        float x2 = rand() % WIDTH;
        float y2 = rand() % HEIGHT;
        randomLines.push_back({{x1, y1}, {x2, y2}});
    }
}

// Get region code for a point
int getRegionCode(float x, float y) {
    int code = INSIDE;
    if (x < clipLeft) code |= LEFT;
    if (x > clipRight) code |= RIGHT;
    if (y < clipBottom) code |= BOTTOM;
    if (y > clipTop) code |= TOP;
    return code;
}

// Cohen-Sutherland line clipping
bool cohenSutherlandClip(float& x1, float& y1, float& x2, float& y2) {
    int code1 = getRegionCode(x1, y1);
    int code2 = getRegionCode(x2, y2);
    bool accept = false;

    while (true) {
        if ((code1 | code2) == 0) {
            accept = true;
            break;
        }
        if (code1 & code2) break;

        float x, y;
        int codeOut = (code1 != 0) ? code1 : code2;

        if (codeOut & TOP) {
            x = x1 + (x2 - x1) * (clipTop - y1) / (y2 - y1);
            y = clipTop;
        } else if (codeOut & BOTTOM) {
            x = x1 + (x2 - x1) * (clipBottom - y1) / (y2 - y1);
            y = clipBottom;
        } else if (codeOut & RIGHT) {
            y = y1 + (y2 - y1) * (clipRight - x1) / (x2 - x1);
            x = clipRight;
        } else {
            y = y1 + (y2 - y1) * (clipLeft - x1) / (x2 - x1);
            x = clipLeft;
        }

        if (codeOut == code1) {
            x1 = x;
            y1 = y;
            code1 = getRegionCode(x1, y1);
        } else {
            x2 = x;
            y2 = y;
            code2 = getRegionCode(x2, y2);
        }
    }
    return accept;
}

void drawLine(float x1, float y1, float x2, float y2, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void drawPoint(float x, float y, float r, float g, float b, float size = 6.0f) {
    glPointSize(size);
    glColor3f(r, g, b);
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw clipping window
    if (windowDefined) {
        glColor3f(0, 0, 0);  // Black outline
        glLineWidth(2.5f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(clipLeft, clipBottom);
        glVertex2f(clipRight, clipBottom);
        glVertex2f(clipRight, clipTop);
        glVertex2f(clipLeft, clipTop);
        glEnd();
        glLineWidth(1.0f);

        // Draw ONLY clipped lines (remove outer parts)
        for (auto& line : randomLines) {
            float x1 = line.first.first, y1 = line.first.second;
            float x2 = line.second.first, y2 = line.second.second;
            float cx1 = x1, cy1 = y1, cx2 = x2, cy2 = y2;
            
            if (cohenSutherlandClip(cx1, cy1, cx2, cy2)) {
                glLineWidth(2.5f);
                switch (colorMode) {
                    case 0: drawLine(cx1, cy1, cx2, cy2, 1, 0, 0); break;
                    case 1: drawLine(cx1, cy1, cx2, cy2, 0, 1, 0); break;
                    case 2: drawLine(cx1, cy1, cx2, cy2, 1, 1, 0); break;
                    case 3: drawLine(cx1, cy1, cx2, cy2, 1, 0, 1); break;
                }
                glLineWidth(1.0f);
            }
        }
    } else {
        // Before window is defined, show random lines in blue
        for (auto& line : randomLines) {
            float x1 = line.first.first, y1 = line.first.second;
            float x2 = line.second.first, y2 = line.second.second;
            drawLine(x1, y1, x2, y2, 0.3f, 0.3f, 1.0f);
            drawPoint(x1, y1, 0.3f, 0.3f, 1.0f, 4);
            drawPoint(x2, y2, 0.3f, 0.3f, 1.0f, 4);
        }
    }

    // Draw window definition points with distinct colors
    if (definingWindow) {
        if (linePoints.size() >= 1) {
            drawPoint(linePoints[0].first, linePoints[0].second, 0, 1, 0, 12);  // Green for first point
        }
        if (linePoints.size() >= 2) {
            drawPoint(linePoints[1].first, linePoints[1].second, 1, 0, 0, 12);  // Red for second point
        }
    }

    glFlush();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!windowDefined) {
            if (!definingWindow) {
                definingWindow = true;
            }
            
            linePoints.push_back({(float)x, (float)y});
            
            if (linePoints.size() == 2) {
                clipLeft = min(linePoints[0].first, linePoints[1].first);
                clipRight = max(linePoints[0].first, linePoints[1].first);
                clipBottom = min(linePoints[0].second, linePoints[1].second);
                clipTop = max(linePoints[0].second, linePoints[1].second);
                windowDefined = true;
                definingWindow = false;
                linePoints.clear();
            }
        }
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'c':
        case 'C':
            linePoints.clear();
            windowDefined = false;
            definingWindow = false;
            generateRandomLines();
            break;
        case 'r':
        case 'R':
            generateRandomLines();
            break;
        case 'k':
        case 'K':
            if (windowDefined) {
                colorMode = (colorMode + 1) % 4;
            }
            break;
        case 27:
            exit(0);
        default:
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Cohen-Sutherland Line Clipping");

    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);

    generateRandomLines();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}