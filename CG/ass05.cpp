#include <iostream>
#include <vector>
#include <cmath>
#include <GLUT/glut.h>
#include <OpenGL/gl.h>

using namespace std;

const int WIDTH = 800, HEIGHT = 600;
const float PI = 3.14159f;

// 2D Point class with operator overloading
class Point {
public:
    float x, y;
    
    Point(float x = 0, float y = 0) : x(x), y(y) {}
    
    // Operator overloading for transformations
    Point operator+(const Point& p) const {
        return Point(x + p.x, y + p.y);
    }
    
    Point operator-(const Point& p) const {
        return Point(x - p.x, y - p.y);
    }
    
    Point operator*(float scalar) const {
        return Point(x * scalar, y * scalar);
    }
    
    Point operator/(float scalar) const {
        return Point(x / scalar, y / scalar);
    }
    
    // Rotate around origin
    Point rotate(float angle) {
        float rad = angle * PI / 180.0f;
        float cosA = cos(rad);
        float sinA = sin(rad);
        return Point(x * cosA - y * sinA, x * sinA + y * cosA);
    }
    
    // Rotate around a pivot point
    Point rotateAround(float angle, const Point& pivot) {
        Point translated = *this - pivot;
        Point rotated = translated.rotate(angle);
        return rotated + pivot;
    }
};

// Shape class
class Shape {
public:
    vector<Point> vertices;
    Point position;
    float rotation;
    float scale;
    
    Shape() : position(400, 300), rotation(0), scale(1.0f) {}
    
    virtual void draw() = 0;
    
    // Operator overloading for transformations
    Shape& operator+=(const Point& translate) {
        position = position + translate;
        return *this;
    }
    
    Shape& operator*=(float s) {
        scale *= s;
        return *this;
    }
};

// Pentagon class
class Pentagon : public Shape {
public:
    Pentagon() {
        for (int i = 0; i < 5; i++) {
            float angle = (i * 72.0f - 90.0f) * PI / 180.0f;
            float x = 70 * cos(angle);
            float y = 70 * sin(angle);
            vertices.push_back(Point(x, y));
        }
    }
    
    void draw() override {
        glColor3f(0.0f, 0.0f, 1.0f);  // Blue fill
        glBegin(GL_POLYGON);
        
        for (const auto& v : vertices) {
            Point p = v * scale;
            p = p.rotateAround(rotation, Point(0, 0));
            p = p + position;
            glVertex2f(p.x, p.y);
        }
        glEnd();
        
        // Draw outline
        glColor3f(0, 0, 0);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        for (const auto& v : vertices) {
            Point p = v * scale;
            p = p.rotateAround(rotation, Point(0, 0));
            p = p + position;
            glVertex2f(p.x, p.y);
        }
        glEnd();
        glLineWidth(1.0f);
    }
};

// Global shape
Pentagon pentagon;
Shape* currentShape = &pentagon;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw grey X and Y axes
    glColor3f(0.7f, 0.7f, 0.7f);  // Grey color
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    // X-axis (horizontal)
    glVertex2f(0, 300);
    glVertex2f(WIDTH, 300);
    // Y-axis (vertical)
    glVertex2f(400, 0);
    glVertex2f(400, HEIGHT);
    glEnd();
    glLineWidth(1.0f);
    
    currentShape->draw();
    glFlush();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    const float ROTATE_STEP = 5.0f;
    const float SCALE_STEP = 0.1f;
    
    switch (key) {
        // Rotation
        case 'q':
        case 'Q':
            currentShape->rotation += ROTATE_STEP;
            break;
        case 'e':
        case 'E':
            currentShape->rotation -= ROTATE_STEP;
            break;
        
        // Scaling
        case 'z':
        case 'Z':
            *currentShape *= (1.0f + SCALE_STEP);
            break;
        case 'x':
        case 'X':
            *currentShape *= (1.0f / (1.0f + SCALE_STEP));
            break;
        
        // Reset
        case 'c':
        case 'C':
            currentShape->position = Point(400, 300);
            currentShape->rotation = 0;
            currentShape->scale = 1.0f;
            break;
        
        case 27: // ESC
            exit(0);
        default:
            break;
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    const float MOVE_STEP = 20.0f;
    
    switch (key) {
        case GLUT_KEY_UP:
            *currentShape += Point(0, -MOVE_STEP);
            break;
        case GLUT_KEY_DOWN:
            *currentShape += Point(0, MOVE_STEP);
            break;
        case GLUT_KEY_LEFT:
            *currentShape += Point(-MOVE_STEP, 0);
            break;
        case GLUT_KEY_RIGHT:
            *currentShape += Point(MOVE_STEP, 0);
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("2D Transformations - Pentagon");

    glClearColor(1, 1, 1, 1);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);

    glutMainLoop();
    return 0;
}