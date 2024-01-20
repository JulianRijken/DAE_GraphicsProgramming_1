#include "pch.h"
#include "EffectOpaque.h"

EffectOpaque::EffectOpaque(ID3D11Device* devicePtr, const std::wstring& effectFileName) :
EffectBase(devicePtr, effectFileName)
{
	BIND(m_UseNormalMapVarPtr,"g_UseNormalMap",AsScalar);
}
