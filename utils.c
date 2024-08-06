#include "utils.h"

float map(float value, float minin, float maxin, float minout, float maxout)
{
	return minout + (value - minin) / (maxin - minin) * (maxout - minout);
}
