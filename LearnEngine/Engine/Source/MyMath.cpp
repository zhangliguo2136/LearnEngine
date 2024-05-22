#include "MyMath.h"
#include <float.h>

const float TMath::Infinity = FLT_MAX;
const float TMath::PI = 3.1415926535f;

float TMath::DegreesToRadians(float Degrees)
{
	return Degrees * (TMath::PI / 180.f);
}
