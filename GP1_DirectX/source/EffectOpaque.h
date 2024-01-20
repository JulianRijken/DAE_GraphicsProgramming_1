#pragma once
#include "EffectBase.h"

class EffectOpaque final : public EffectBase
{
public:
	EffectOpaque(ID3D11Device* devicePtr, const std::wstring& effectFileName);

	void SetUseNormalMap(bool useNormalMap) const { m_UseNormalMapVarPtr->SetRawValue(&useNormalMap, 0, sizeof(bool)); }

private:
	ID3DX11EffectVariable* m_UseNormalMapVarPtr{};
};
