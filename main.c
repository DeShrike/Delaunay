// https://www.youtube.com/watch?v=pPqZPX9DvTg

#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"
#include "da.h"
#include "utils.h"
#include "delaunay.h"

#define WIDTH        800
#define HEIGHT       600

#define BORDER    50

#define POINT_RADIUS  5
#define POINT_COUNT   35

Font text_font;
Font number_font;

typedef struct {
   Point* items;
   int count;
   int capacity;
} LocalPoints;

LocalPoints points = { 0 };

Delaunay delaunay;

int offsetx = 0;
int offsety = 0;

void init()
{
   points.count = 0;
   for (int y = 100; y < HEIGHT - BORDER; y += 50)
   {
      for (int x = 100; x < WIDTH - BORDER; x += 50)
      {
         Point p = {
            .x = x + (rand() % 20) - 10,
            .y = y + (rand() % 20) - 10
         };
         
         da_append(&points, p);
      }
   }

   /*for (int i = 0; i < POINT_COUNT; ++i)
   {
      points[i].x = (rand() % (WIDTH - BORDER * 2)) + BORDER;
      points[i].y = (rand() % (HEIGHT - BORDER * 2)) + BORDER;
   }*/

   /*
   points[0].x = 300;
   points[0].y = 300;

   points[1].x = 600;
   points[1].y = 350;
   
   points[2].x = 300;
   points[2].y = 450;

   points[3].x = 310;
   points[3].y = 550;
*/
   delaunay_free(&delaunay);
   delaunay = delaunay_init(points.items, points.count);
}

void cleanup()
{
   delaunay_free(&delaunay);
}

void draw_number(int x, int y, int value)
{
   char temp[20];
   sprintf(temp, "%d", value);

   int font_size = 16;
   int spacing = 1;

   Vector2 size = MeasureTextEx(number_font, temp, font_size, spacing);
   Vector2 pos = {
      .x = x - (size.x / 2),
      .y = y - (size.y / 2)
   };

   DrawTextEx(number_font, temp, pos, font_size, spacing, WHITE);
}

void draw_center(int x, int y, int value, char* text)
{
   char temp[20];
   sprintf(temp, "%d", value);

   int font_size = 16;
   int spacing = 1;

   Vector2 size = MeasureTextEx(number_font, temp, font_size, spacing);
   Vector2 pos = {
      .x = x - (size.x / 2),
      .y = y - (size.y / 2)
   };

   DrawTextEx(number_font, temp, pos, font_size, spacing, WHITE);

   Vector2 pos2 = {
      .x = x - (size.x / 2),
      .y = y - (size.y / 2) + size.y
   };
   DrawTextEx(number_font, text, pos2, font_size, spacing, WHITE);
}

void draw_point(Point p, Color c)
{
   DrawCircle(p.x + offsetx, p.y + offsety, POINT_RADIUS, c);
   DrawCircleLines(p.x + offsetx, p.y + offsety, POINT_RADIUS, WHITE);
}

void draw_line(int x1, int y1, int x2, int y2)
{
   DrawLine(x1, y1, x2, y2, WHITE);
}

void draw_line_v(Line l, Color c)
{
   for (int x = 0; x < WIDTH; ++x)
   {
      int y = (l.c - l.a * x) / l.b;
      if (y >= 0 && y < HEIGHT)
      {
         DrawPixel(x, y, c);
      }
   }
}

void draw_title()
{
   const char* temp = "Delaunay Triangulation";
   int font_size = 40;
   int spacing = 2;

   Vector2 size = MeasureTextEx(text_font, temp, font_size, spacing);
   Vector2 pos = {
      .x = (WIDTH / 2) - (size.x / 2),
      .y = (size.y / 2)
   };

   DrawTextEx(text_font, temp, pos, font_size, spacing, WHITE);
}

/*
void drawX()
{
   Point a = { .x = WIDTH / 3, .y = 100 };
   Point b = { .x = 150, .y = HEIGHT - 150 };
   Point c = { .x = WIDTH - 100, .y = HEIGHT - 250 };

   draw_point(a, GREEN);
   draw_point(b, GREEN);
   draw_point(c, GREEN);

   Point ab = points_lerp(0.5f, a, b);
   Point bc = points_lerp(0.5f, b, c);
   Point ac = points_lerp(0.5f, a, c);

   draw_point(ab, BLUE);
   draw_point(bc, BLUE);
   draw_point(ac, BLUE);

   Line lab = linear_eq(&a, &b);
   draw_line_v(lab, WHITE);
   Line lbc = linear_eq(&b, &c);
   draw_line_v(lbc, WHITE);
   Line lac = linear_eq(&a, &c);
   draw_line_v(lac, WHITE);

   Line perp_ab = perp_line(&ab, &lab);
   Line perp_bc = perp_line(&bc, &lbc);
   Line perp_ac = perp_line(&ac, &lac);
   draw_line_v(perp_ab, GREEN);
   draw_line_v(perp_bc, GREEN);
   draw_line_v(perp_ac, GREEN);

   Circle circle = circle_from_triangle(&a, &b, &c);

   draw_point(circle.center, YELLOW);

   // printf("Circle; %.3f, %.3f  - %.3f\n", circle.center.x, circle.center.y, circle.radius);

   DrawCircleLines(circle.center.x, circle.center.y, circle.radius, RED);

   for (int i = 0; i < POINT_COUNT; ++i)
   {
      draw_point(i, points[i].x, points[i].y);
   }

   for (int i = 0; i < POINT_COUNT; ++i)
   {
      if (point_in_triangle(&points[i], &a, &b, &c))
      {
         DrawCircle(points[i].x, points[i].y, POINT_RADIUS, GREEN);
      }
      else
      {
         DrawCircle(points[i].x, points[i].y, POINT_RADIUS, RED);
      }
   }
}
*/

void draw()
{
   //printf("main: Triangle count: %d\n", delaunay.triangles.count);

   for (int i = 0; i < delaunay.triangles.count; ++i)
   {
      //printf("Triangle %d: %d %d %d\n", i, delaunay.triangles.items[i].ix1, delaunay.triangles.items[i].ix2, delaunay.triangles.items[i].ix3);
      Point p1 = POINTV(delaunay, TRIAV(delaunay, i).ix1);
      Point p2 = POINTV(delaunay, TRIAV(delaunay, i).ix2);
      Point p3 = POINTV(delaunay, TRIAV(delaunay, i).ix3);
      /*
      if (p1.x == 2000 || p2.x == 2000 || p3.x == 2000)
         continue;
      if (p1.y == 2000 || p2.y == 2000 || p3.y == 2000)
         continue;
      if (p1.x == -2000 || p2.x == -2000 || p3.x == -2000)
         continue;
      if (p1.y == -2000 || p2.y == -2000 || p3.y == -2000)
         continue;
      if (p1.x == 1000 || p2.x == 1000 || p3.x == 1000)
         continue;
      if (p1.y == 1000 || p2.y == 1000 || p3.y == 1000)
         continue;
      */
      //printf(" = %.1f, %.1f  --  %.1f, %.1f  --  %.1f, %.1f\n", p1.x, p1.y, p2.x, p2.y, p3.x, p3.x);
      DrawLine(p1.x + offsetx, p1.y + offsety, p2.x + offsetx, p2.y + offsety, WHITE);
      DrawLine(p2.x + offsetx, p2.y + offsety, p3.x + offsetx, p3.y + offsety, WHITE);
      DrawLine(p1.x + offsetx, p1.y + offsety, p3.x + offsetx, p3.y + offsety, WHITE);
      /*
      Circle* circle = &TRIAV(delaunay, i).circle;
      DrawCircleLines(circle->center.x + offsetx, circle->center.y + offsety, circle->radius, RED);

      char temp[100];
      sprintf(temp,"%d %d %d",
               TRIAV(delaunay, i).neighbours[0],
               TRIAV(delaunay, i).neighbours[1],
               TRIAV(delaunay, i).neighbours[2]);
      draw_center(circle->center.x + offsetx, circle->center.y + offsety, i, temp);
      */
   }

   for (int i = 0; i < delaunay.points.count; ++i)
   {
      draw_point(POINTV(delaunay, i), RED);
      draw_number(POINTV(delaunay, i).x + 10 + offsetx, POINTV(delaunay, i).y + 5 + offsety, i);
   }
}

int main(void)
{
   //srand(time(NULL));
   srand(13);
   SetConfigFlags(FLAG_MSAA_4X_HINT);  // Try to enable MSAA 4X
   // SetConfigFlags(FLAG_WINDOW_RESIZABLE);
   InitWindow(WIDTH, HEIGHT, "Delaunay Triangulation");
   SetTargetFPS(30);

   text_font = LoadFontEx("Roboto-Medium.ttf", 32, 0, 250);
   number_font = LoadFontEx("FjallaOne-Regular.ttf", 32, 0, 250);

   init();

   while (!WindowShouldClose())
   {
      if (IsKeyPressed(KEY_Q))
      {
         break;
      }
      else if (IsKeyPressed(KEY_R))
      {
         init();
      }
      else if (IsKeyPressed(KEY_S))
      {
         delaunay_step(&delaunay);
      }
      else if (IsKeyPressed(KEY_LEFT))
      {
         offsetx += 50;
      }
      else if (IsKeyPressed(KEY_RIGHT))
      {
         offsetx -= 50;
      }
      else if (IsKeyPressed(KEY_UP))
      {
         offsety += 50;
      }
      else if (IsKeyPressed(KEY_DOWN))
      {
         offsety -= 50;
      }
      else if (IsKeyPressed(KEY_HOME))
      {
         offsetx = 0;
         offsety = 0;
      }

      BeginDrawing();
      ClearBackground(GetColor(0x181818FF));

      DrawText("Q = Exit   R = Reset   S = Step", 20, HEIGHT - 20, 14, YELLOW);

         delaunay_step(&delaunay);

      draw_title();
      draw();

      EndDrawing();
   }

   cleanup();

   UnloadFont(number_font);
   UnloadFont(text_font);

   da_free(points);

   CloseWindow();

   return 0;
}
