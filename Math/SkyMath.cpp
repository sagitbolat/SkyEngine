#include <cmath>

#include "SkyMath.h"
	
float Magnitude(const Vector3D vec) {	
	return (SQUARE(vec.x) + SQUARE(vec.y) + SQUARE(vec.z)); 
}
	
Vector3D Normalize(const Vector3D vec) {
	return (vec / Magnitude(vec)); 
}

