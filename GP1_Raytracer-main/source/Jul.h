#pragma once
#include <SDL_stdinc.h>

#include "utils.h"


class Jul
{

public:

	static bool IsEven(const int number)
	{
		return number % 2 == 0;
	}

	template <typename DataType>
	static DataType Clamp(const DataType value, DataType min, DataType max)
	{
		if (min > max)
		{
			//std::swap(min, max);
			const DataType tempMin{ min };
			min = max;
			max = tempMin;
		}


		return std::max(std::min(max, value), min);
	}


	template <typename DataType>
	static DataType Clamp01(const DataType value)
	{
		return 	std::max(std::min(1.0f, value), 0.0f);
	}

	static void ClampLoop01(float& value)
	{
		if (value > 1 || value < 0)
			value -= floorf(value);
	}

	static float ClampLoop01(const float value)
	{
		if (value > 1 || value < 0)
			return value - floorf(value);

		return value;
	}

	static int ClampLoop(const int value, int min, int max)
	{
		if (min > max)
		{
			const int tempMax = max;
			max = min;
			min = tempMax;
		}

		int result{ value };


		int range{ max - min };

		while (result > max)
			result -= range + 1;

		while (result < min)
			result += range + 1;


		return result;
	}


	static float OneMinus(const float& value)
	{
		return 1.0f - value;
	}

	static float MapWave(const float time, const float min, const float max, const float repeatTime, const float startTime)
	{
		const float amplitude{ (max - min) / 2.0f };
		const float intercept{ min + amplitude };
		const float pulsation{ 2 * dae::PI / repeatTime };
		const float phase{ 2 * dae::PI * startTime };

		return amplitude * sinf(pulsation * time + phase) + intercept;
	}


	static float Lerp(const float a, const float b, float t)
	{
		t = Clamp01(t);
		return  a + (b - a) * t;
	}

	static dae::Vector3 Lerp(const dae::Vector3 a, const dae::Vector3 b, float t)
	{
		t = Clamp01(t);
		return dae::Vector3
		{
			Lerp(a.x,b.x,t),
			Lerp(a.y,b.y,t),
			Lerp(a.z,b.z,t),
		};
	}

	static dae::ColorRGB Lerp(const dae::ColorRGB a, const dae::ColorRGB b, float t)
	{
		t = Clamp01(t);
		return dae::ColorRGB
		{
			Lerp(a.r,b.r,t),
			Lerp(a.g,b.g,t),
			Lerp(a.b,b.b,t),
		};
	}


	static float SmoothLerp(const float a, const float b, float t)
	{
		t = Clamp01(t);
		return Lerp(a, b, MapWave(t, 0, 1, 2, -0.25f));
	}

	static dae::Vector3 SmoothLerp(const dae::Vector3 a, const dae::Vector3 b, float t)
	{
		return dae::Vector3
		{
			SmoothLerp(a.x,b.x,t),
			SmoothLerp(a.y,b.y,t),
			SmoothLerp(a.z,b.z,t),
		};
	}



	static float SmoothEndLerp(const float a, const float b, float t)
	{
		t = Clamp01(t);
		return Lerp(a, b, MapWave(t, -1, 1, 4, 0));
	}

	static dae::Vector3 SmoothEndLerp(const dae::Vector3 a, const dae::Vector3 b, float t)
	{
		return dae::Vector3
		{
			SmoothEndLerp(a.x,b.x,t),
			SmoothEndLerp(a.y,b.y,t),
			SmoothEndLerp(a.z,b.z,t),
		};
	}

	static float SmoothStartLerp(const float a, const float b, float t)
	{
		t = Clamp01(t);
		return Lerp(a, b, MapWave(t, 0, 2, 4, -0.25f));
	}

	static dae::Vector3 SmoothStartLerp(const dae::Vector3 a, const dae::Vector3 b, float t)
	{
		return dae::Vector3
		{
			SmoothStartLerp(a.x,b.x,t),
			SmoothStartLerp(a.y,b.y,t),
			SmoothStartLerp(a.z,b.z,t),
		};
	}


	static float MoveTowards(const float& current, const float& target, const float& maxDelta)
	{

		float direction{ target - current };
		if (direction > maxDelta)
			direction = maxDelta;

		return current + direction;
	}

	static dae::Vector3 MoveTowards(const dae::Vector3& current, const dae::Vector3& target, const float& maxDelta)
	{
		dae::Vector3 direction{ target - current };
		if (direction.Magnitude() > maxDelta)
			direction = direction.Normalized() * maxDelta;

		return current + direction;
	}



	static float MapValueInRange(float value, float inRangeMin, float inRangeMax, float outRangeMin = 0.0f, float outRangeMax = 1.0f)
	{
		return (value - inRangeMin) * (outRangeMax - outRangeMin) / (inRangeMax - inRangeMin) + outRangeMin;
	}

	static float MapValueInRangeClamped(float value, float inRangeMin, float inRangeMax, float outRangeMin = 0.0f, float outRangeMax = 1.0f)
	{
		return Clamp((value - inRangeMin) * (outRangeMax - outRangeMin) / (inRangeMax - inRangeMin) + outRangeMin, outRangeMin, outRangeMax);
	}


	static	float RandomRange(float min, float max)
	{
		return float(RandomRange(double(min), double(max)));
	}

	static double RandomRange(double min, double max)
	{
		// Swap
		if (min > max)
		{
			const double tempMax = max;
			max = min;
			min = tempMax;
		}

		const double randomAlpha{ double(rand()) / RAND_MAX };
		const double range{ max - min };

		return randomAlpha * range + min;
	}

	static int RandomRange(int min, int max)
	{
		// Swap
		if (min > max)
		{
			const int tempMax = max;
			max = min;
			min = tempMax;
		}

		return rand() % (max - min + 1) + min;
	}

	static float RandomValue()
	{
		return rand() % RAND_MAX / float(RAND_MAX);
	}

	static int GetRandomItemFromList(const std::initializer_list<bool>& boolList)
	{
		// Get list of filtered items
		std::vector<int> filteredItems;
		for (int index = 0; index < boolList.size(); ++index)
		{
			if (*(boolList.begin() + index))
				filteredItems.push_back(index);
		}

		// If no true values found, return -1
		if (filteredItems.empty())
			return -1;

		// Return the random item
		return filteredItems[std::rand() % filteredItems.size()];
	}


	static float RadToDeg(float radians)
	{
		return radians / dae::PI * 180.0f;
	}

	static float DegToRad(float degrees)
	{
		return degrees / 180.0f * float(M_PI);
	}
};
