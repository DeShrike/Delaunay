// https://www.youtube.com/watch?v=pPqZPX9DvTg

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "da.h"
#include "utils.h"
#include "delaunay.h"

// private /////////////////////////////////////////////////////////
int block_index(const Point* p);
int sort_by_region(const void* aa, const void* bb);

bool point_in_triangle(Point* p, Point* a, Point* b, Point* c);
bool point_in_circle(Point p, Circle c);

Line linear_eq(Point* p1, Point* p2);
Line perp_line(Point* p, Line* l);
Point crossing_point(Line* l1, Line* l2);
Circle circle_from_triangle(Point* a, Point* b, Point* c);
////////////////////////////////////////////////////////////////////

#define MAX_STACK 100
int stack[MAX_STACK];
int stack_pointer = 0;

void stack_push(int value)
{
   if (value == -1)
   {
      return;
   }
   
   for (int i = 0; i < stack_pointer; ++i)
   {
      if (stack[i] == value)
      {
         return;
      }
   }
   
   if (stack_pointer >= MAX_STACK)
   {
      printf("ERROR: stack too small\n");
      return;
   }
   
   stack[stack_pointer++] = value;
}

int stack_pop()
{
   return stack[--stack_pointer];
}

void stack_print()
{
   printf("Stack: ");
   for (int i = 0; i < stack_pointer; ++i)
   {
      printf("%d ", stack[i]);
   }
   
   printf("\n");
}

////////////////////////////////////////////////////////////////////


Delaunay delaunay_init(Point* points, int point_count)
{
   #define BIG 2000
   Delaunay d = { 0 };
   d.points = (Points) { 0 };
   d.triangles = (Triangles) { 0 };

   /*float minx = 1000000;
   float maxx = -1000000;
   float miny = minx;
   float maxy = maxx;

   for (int i = 0; i < point_count; ++i)
   {
      minx = min(points[i].x, minx);
      maxx = max(points[i].x, maxx);
      miny = min(points[i].y, miny);
      maxy = max(points[i].y, maxy);
   }*/

   qsort(points, point_count, sizeof(Point), sort_by_region);

   // big triangle:
   Point b0 = { .x = BIG / 2, .y = -BIG };
   Point b1 = { .x = BIG, .y = BIG };
   Point b2 = { .x = -BIG, .y = BIG };
   //Point b0 = { .x = 400, .y = 10 };
   //Point b1 = { .x = 780, .y = 570 };
   //Point b2 = { .x = 10, .y = 580 };
   Triangle big = { 
      .ix1 = 0,
      .ix2 = 1,
      .ix3 = 2,
      .circle = circle_from_triangle(&b0, &b1, &b2),
      .neighbours = {-1, -1, -1}
   };
   da_append(&d.points, b0);
   da_append(&d.points, b1);
   da_append(&d.points, b2);
   da_append(&d.triangles, big);
   d.currentpoint = 3;

   for (int i = 0; i < point_count; ++i)
   {
      da_append(&d.points, points[i]);
   }

   return d;
}

void delaunay_free(Delaunay* delaunay)
{
   da_free(delaunay->triangles);
   da_free(delaunay->points);
}

void add_neightbour(Triangle* tr, int n)
{
   if (tr->neighbours[0] == -1)
   {
      tr->neighbours[0] = n;
      return;
   }

   if (tr->neighbours[1] == -1)
   {
      tr->neighbours[1] = n;
      return;
   }

   if (tr->neighbours[2] == -1)
   {
      tr->neighbours[2] = n;
      return;
   }
   
   printf("Error: no room for new neighbour\n");
}

bool has_2_points_in_common(Triangle* a, Triangle* b)
{
   int in_common = 0;
   if (a->ix1 == b->ix1 || a->ix1 == b->ix2 || a->ix1 == b->ix3)
   {
      in_common++;
   }
   
   if (a->ix2 == b->ix1 || a->ix2 == b->ix2 || a->ix2 == b->ix3)
   {
      in_common++;
   }

   if (a->ix3 == b->ix1 || a->ix3 == b->ix2 || a->ix3 == b->ix3)
   {
      in_common++;
   }

   //printf("%d %d %d", a->ix1, a->ix2, a->ix3);
   //printf("   >>  ");
   //printf("%d %d %d  ", b->ix1, b->ix2, b->ix3);

   //printf("In common: %d\n", in_common);
   return (in_common == 2);
}

void calculate_circle(Delaunay* delaunay, int triangle_ix)
{
   Triangle* tr = &TRIA(delaunay, triangle_ix);
   
   Point* a = &POINT(delaunay, tr->ix1);
   Point* b = &POINT(delaunay, tr->ix2);
   Point* c = &POINT(delaunay, tr->ix3);
   tr->circle = circle_from_triangle(a, b, c);
}

void calculate_neighbours(Delaunay* delaunay, int triangle_ix)
{
   //printf("Calc neighbours for triangle #%d\n", triangle_ix);

   TRIA(delaunay, triangle_ix).neighbours[0] = -1;
   TRIA(delaunay, triangle_ix).neighbours[1] = -1;
   TRIA(delaunay, triangle_ix).neighbours[2] = -1;

   for (int t = 0; t < delaunay->triangles.count; ++t)
   {
      if (t == triangle_ix)
      {
         continue;
      }

      if (has_2_points_in_common(&TRIA(delaunay, triangle_ix), &TRIA(delaunay, t)))
      {
         add_neightbour(&TRIA(delaunay, triangle_ix), t);
      }
   }

   //printf("  Result: %d %d %d\n", TRIA(delaunay, triangle_ix).neighbours[0],
   //                               TRIA(delaunay, triangle_ix).neighbours[1],
   //                               TRIA(delaunay, triangle_ix).neighbours[2]);
}

void common_points(Triangle* t1, Triangle* t2, int* common1, int* common2)
{
   *common1 = -1;
   *common2 = -1;
   if (t1->ix1 == t2->ix1 || t1->ix1 == t2->ix2 || t1->ix1 == t2->ix3)
   {
      *common1 = t1->ix1;
   }

   if (t1->ix2 == t2->ix1 || t1->ix2 == t2->ix2 || t1->ix2 == t2->ix3)
   {
      if (*common1 == -1)
      {
         *common1 = t1->ix2;
      }
      else
      {
         *common2 = t1->ix2;
      }
   }

   if (t1->ix3 == t2->ix1 || t1->ix3 == t2->ix2 || t1->ix3 == t2->ix3)
   {
      *common2 = t1->ix3;
   }
}

int not_common_point(Triangle* t1, Triangle* t2)
{
   if (t2->ix1 != t1->ix1 && t2->ix1 != t1->ix2 && t2->ix1 != t1->ix3)
   {
      return t2->ix1;
   }
   
   if (t2->ix2 != t1->ix1 && t2->ix2 != t1->ix2 && t2->ix2 != t1->ix3)
   {
      return t2->ix2;
   }
   
   if (t2->ix3 != t1->ix1 && t2->ix3 != t1->ix2 && t2->ix3 != t1->ix3)
   {
      return t2->ix3;
   }
   
   return -1;
}

void replace_point(Triangle *t, int from, int to)
{
   if (t->ix1 == from)
   {
      t->ix1 = to;
      return;
   }

   if (t->ix2 == from)
   {
      t->ix2 = to;
      return;
   }

   if (t->ix3 == from)
   {
      t->ix3 = to;
      return;
   }
}

void swap_triangles(Triangle* t1, Triangle* t2)
{
   //printf("Swapping T1: %d %d %d  with T2: %d %d %d\n", t1->ix1, t1->ix2, t1->ix3, t2->ix1, t2->ix2, t2->ix3);

   int nc1 = not_common_point(t1, t2);
   int nc2 = not_common_point(t2, t1);

   int c1, c2;
   common_points(t1, t2, &c1, &c2);

   //printf("Common: %d and %d    Not Common: %d and %d\n", c1, c2, nc1, nc2);

   replace_point(t1, c1, nc1);
   replace_point(t2, c2, nc2);
   //printf(" Result: T1: %d %d %d  with T2: %d %d %d\n", t1->ix1, t1->ix2, t1->ix3, t2->ix1, t2->ix2, t2->ix3);
}


bool process_stack(Delaunay* delaunay)
{
   if (stack_pointer == 0)
   {
      return false;
   }

   //printf("Process Stack --------------\n");

   stack_pointer--;
   Triangle* tr = &TRIA(delaunay, stack[stack_pointer]);
   
   // for all neighbours of this triangle, check if the third point is inside this triangles circle
   // if so, swap the coeection from the common points to the not-common points and add all neightbours to the stack 

   for (int n = 0; n < 3; ++n)
   {
      if (tr->neighbours[n] == -1)
      {
         continue;
      }
      
      //printf("Neighbour: %d\n", tr->neighbours[n]);

      Triangle* ntr = &TRIA(delaunay, tr->neighbours[n]);

      int pix = not_common_point(tr, ntr);
      if (pix == -1)
      {
         printf("Error: no non-common point found between T #%d and #%d \n", stack[stack_pointer], tr->neighbours[n]);
         return false;
      }

      if (!point_in_circle(POINT(delaunay, pix), tr->circle))
      {
         continue;
      }

      // we need to swap
      /*
      for (int tria = 0; tria < delaunay->triangles.count; ++tria)
      {
         Triangle* tt = &TRIA(delaunay, tria);
         printf("T%d: %d %d %d\n", tria, tt->ix1, tt->ix2, tt->ix3);
      }*/
      //printf("Swapping %d and %d\n", stack[stack_pointer], tr->neighbours[n]);
      swap_triangles(ntr, tr);
      /*printf("Swapped\n");
      for (int tria = 0; tria < delaunay->triangles.count; ++tria)
      {
         Triangle* tt = &TRIA(delaunay, tria);
         printf("T%d: %d %d %d\n", tria, tt->ix1, tt->ix2, tt->ix3);
      }*/
      for (int i = 0; i < delaunay->triangles.count; ++i)
      {
         calculate_neighbours(delaunay, i);
         calculate_circle(delaunay, i);
      }
      stack_push(tr->neighbours[0]);
      stack_push(tr->neighbours[1]);
      stack_push(tr->neighbours[2]);
      stack_push(ntr->neighbours[0]);
      stack_push(ntr->neighbours[1]);
      stack_push(ntr->neighbours[2]);
      break;
   }

   for (int i = 0; i < delaunay->triangles.count; ++i)
   {
      calculate_neighbours(delaunay, i);
      calculate_circle(delaunay, i);
   }

   return true;
}

void delaunay_step(Delaunay* delaunay)
{
   if (delaunay->currentpoint >= delaunay->points.count)
   {
      printf("No steps left\n");
      return;
   }

   bool found = false;
   for (int t = 0; t < delaunay->triangles.count; ++t)
   {
      Triangle* tr = &TRIA(delaunay, t);
      Point* p = &POINT(delaunay, delaunay->currentpoint);

      Point a = POINT(delaunay, tr->ix1);
      Point b = POINT(delaunay, tr->ix2);
      Point c = POINT(delaunay, tr->ix3);
      if (point_in_triangle(p, &a, &b, &c))
      {
         printf("Point %d is in triangle %d\n", delaunay->currentpoint, t);
         found = true;

         //printf("Point %.2f,%.2f is in triangle %d\n", p->x, p->y, t);
         Triangle n1 = {
            .ix1 = tr->ix1,
            .ix2 = tr->ix2,
            .ix3 = delaunay->currentpoint,
            .circle = circle_from_triangle(&a, &b, p),
            .neighbours = { -1, -1, -1 }
         };
         Triangle n2 = {
            .ix1 = tr->ix2,
            .ix2 = tr->ix3,
            .ix3 = delaunay->currentpoint,
            .circle = circle_from_triangle(&b, &c, p),
            .neighbours = { -1, -1, -1 }
         };
         tr->ix2 = delaunay->currentpoint;
         tr->circle = circle_from_triangle(&a, p, &c);

         da_append(&delaunay->triangles, n1);
         da_append(&delaunay->triangles, n2);

         stack_push(t);
         stack_push(delaunay->triangles.count - 1);
         stack_push(delaunay->triangles.count - 2);
         stack_push(tr->neighbours[0]);
         stack_push(tr->neighbours[1]);
         stack_push(tr->neighbours[2]);

         for (int s = 0; s < stack_pointer; ++s)
         {
            calculate_neighbours(delaunay, stack[s]);
         }

         while (process_stack(delaunay));
         break;
      }
   }

   if (!found)
   {
      printf("ERROR: Point not in any triangle\n");
   }
   
   //printf("Triangle count: %d\n", delaunay->triangles.count);
   //stack_print();
   delaunay->currentpoint++;
}
















int block_index(const Point* p)
{
   #define REGION_SIZE 50

   int x = p->x / REGION_SIZE;
   int y = p->y / REGION_SIZE;
   if (y % 2 == 1)
   {
      x = (2000 / REGION_SIZE) - x;
   }

   return x + y * (2000 / REGION_SIZE);
}

int sort_by_region(const void* aa, const void* bb)
{
   Point* a = (Point*)aa;
   Point* b = (Point*)bb;

   return block_index(a) - block_index(b);
}

bool point_in_circle(Point p, Circle c)
{
   return distance(p, c.center) <= c.radius;
}

bool point_in_triangle(Point* p, Point* a, Point* b, Point* c)
{
   // P = A + w1 * (B - A) + w2 * (C - A);
   float cyay = c->y - a->y;
   float cxax = c->x - a->x;
   float bxax = b->x - a->x;
   float byay = b->y - a->y;
   float pyay = p->y - a->y;
   // float pxax = p->x - a->x;

   float w1 = (a->x * cyay + pyay * cxax - p->x * cyay) / (byay * cxax - bxax * cyay);
   float w2 = (p->y - a->y - w1 * byay) / cyay;

   return w1 >= 0.0f && w2 >= 0.0f && (w1 + w2) <= 1.0f;
}

Line linear_eq(Point* p1, Point* p2)
{
   // Ax + By = C
   // By = C - Ax
   // y = (c - Ax) / B
   
   float deltaX = p2->x - p1->x;
   float deltaY = p2->y - p1->y;
   Line l = {
      .a = deltaY,
      .b = -deltaX,
      .c = deltaY * p1->x + -deltaX * p1->y,
   };

   return l;
}

Line perp_line(Point* p, Line* l)
{
   Line perp = {
      .a = -l->b,
      .b = l->a,
      .c = -l->b * p->x + l->a * p->y
   };

   return perp;
}

Point crossing_point(Line* l1, Line* l2)
{
   float d = l1->a * l2->b - l2->a * l1->b;
   float dx = l1->c * l2->b - l2->c * l1->b;
   float dy = l1->a * l2->c - l2->a * l1->c;

   Point p = {
      .x = dx / d,
      .y = dy / d
   };

   return p;
}

Point points_lerp(float value, Point a, Point b)
{
   return (Point) {
      .x = a.x * value + b.x * (1 - value),
      .y = a.y * value + b.y * (1 - value)
   };
}

Circle circle_from_triangle(Point* a, Point* b, Point* c)
{
   // https://www.youtube.com/watch?v=uIBGSztyB04
   Line ab = linear_eq(a, b);
   Line bc = linear_eq(b, c);

   // printf("Line ab: %.2f %.2f %.2f\n", ab.a, ab.b, ab.c);
   // printf("Line bc: %.2f %.2f %.2f\n", bc.a, bc.b, bc.c);

   Point mid_ab = points_lerp(0.5f, *a, *b);
   Point mid_bc = points_lerp(0.5f, *b, *c);

   Line perp_ab = perp_line(&mid_ab, &ab);
   Line perp_bc = perp_line(&mid_bc, &bc);

   Point crossing = crossing_point(&perp_ab, &perp_bc);
   
   Circle ci = {
      .center = crossing,
      .radius = distance(*a, crossing)
   };
   
   return ci;
}
