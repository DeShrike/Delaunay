#ifndef _DELAUNAY_H_
#define _DELAUNAY_H_

#include "vector2.h"

#define Point Vec2

typedef struct {
   float a;
   float b;
   float c;
} Line;

typedef struct {
   Point center;
   float radius;
} Circle;

typedef struct {
   int ix1;
   int ix2;
   int ix3;
   Circle circle;
   int neighbours[3];
} Triangle;

typedef struct {
   Triangle* items;
   int count;
   int capacity;
} Triangles;

typedef struct {
   Point* items;
   int count;
   int capacity;
} Points;

typedef struct {
   Points points;
   Triangles triangles;
   int currentpoint;
} Delaunay;


#define POINT(d, ix)  (d->points.items[ix])
#define TRIA(d, ix)  (d->triangles.items[ix])

#define POINTV(d, ix)  (d.points.items[ix])
#define TRIAV(d, ix)  (d.triangles.items[ix])

// public
Delaunay delaunay_init(Point* points, int point_count);
void delaunay_free(Delaunay* delaunay);
void delaunay_step(Delaunay* delaunay);

// private
bool point_in_triangle(Point* p, Point* a, Point* b, Point* c);
Point points_lerp(float value, Point a, Point b);
Circle circle_from_triangle(Point* a, Point* b, Point* c);
Line perp_line(Point* p, Line* l);
Line linear_eq(Point* p1, Point* p2);

#endif
