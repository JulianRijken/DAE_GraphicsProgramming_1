
#include <iostream>

#include "ColorRGB.h"

void main()
{
	ColorRGB color1{ 0.3f,0.3f,0.3f };
	ColorRGB color2{ 0.7f,0.7f,0.7f };
	ColorRGB color3{ color1 * color2 };

	float float1{ 0.3f };
	float float2{ 0.7f };
	float float3{ float1 * float2 };
}
