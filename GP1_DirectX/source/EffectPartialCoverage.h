#pragma once
#include "EffectBase.h"

class EffectPartialCoverage final : public EffectBase
{
public:
	EffectPartialCoverage(ID3D11Device* devicePtr, const std::wstring& effectFileName);
};
