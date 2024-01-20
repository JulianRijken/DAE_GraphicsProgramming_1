#include "pch.h"
#include "EffectPartialCoverage.h"
EffectPartialCoverage::EffectPartialCoverage(ID3D11Device* devicePtr, const std::wstring& effectFileName) :
	EffectBase(devicePtr, effectFileName) {}
