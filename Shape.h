#ifndef SHAPE_H
#define SHAPE_H

#include <GL/glui.h>
#include "Algebra.h"
#include "TextureMap.h"

#define MIN_ISECT_DISTANCE 30
#define RAD 0.0174532925199432954743716805978692718782
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923
#define DEG_TO_RAD(a)	(a*RAD)

class Shape {
public:
	Shape() {};
	~Shape() {};

	void setSegments(int x, int y) {
		m_segmentsX = x;
		m_segmentsY = y;
	}

	virtual void draw()=0;
	virtual void drawNormal()=0;

	virtual double Intersect(Point eyePointP, Vector rayV, Matrix transformMatrix) = 0;
	virtual Vector findIsectNormal(Point eyePoint, Vector ray, double dist) = 0;

    virtual Point textureAt(Point intersectWorld, Matrix TransformMatrix, TextureMap* textureMap) = 0;

protected:
	void normalizeNormal (float x, float y, float z) {
        Vector v = Vector(x, y, z);
		normalizeNormal (v);
	};

	void normalizeNormal (Vector& v) {
		v.normalize();
        const double* d = v.unpack();
		glNormal3dv(d);
	};


	int m_segmentsX, m_segmentsY;
};

double getAngleOnCircle(double x, double y) {
    if (x >= 0 && y >= 0) {
        return atan(y / x) * 180 / PI; 
    } else if (x < 0 && y >= 0) {
        return 90.0 + atan(-x / y) * 180 / PI; 
    } else if (x < 0 && y < 0) {
        return 180 + atan(y / x) * 180 / PI; 
    } else if (x >= 0 && y < 0) {
        return 270 + atan(x / -y) * 180 / PI; 
    } 
}

#endif
