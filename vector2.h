#ifndef _VECTOR2_H_
#define _VECTOR2_H_

#include <math.h>

typedef struct {
   float x;
   float y;
} Vec2;

inline float distance(Vec2 a, Vec2 b)
{
   return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}


#endif

