#pragma once

#include "cfcConfig.h"
#include "cfcMath.h"
#include "cfcStructures.h"

#include <stdlib.h>

namespace cfc {
namespace math {
namespace complexscalar {
	typedef float factor;
	typedef int factor_fp16;

	enum class repeatmode
	{
		Clamp,
		Repeat,
	};

	template <typename T>
	class curve
	{
	public:
		curve() : ModeLeft(repeatmode::Clamp), ModeRight(repeatmode::Clamp) { }
		curve(const T* values, int numValues) : ModeLeft(repeatmode::Clamp), ModeRight(repeatmode::Clamp) { SetValues(values, numValues); }
		curve(T v0, T v1) : ModeLeft(repeatmode::Clamp), ModeRight(repeatmode::Clamp) { T values[2] = { v0, v1 };  SetValues(values, 2); }
		curve(T v0, T v1, T v2) : ModeLeft(repeatmode::Clamp), ModeRight(repeatmode::Clamp) { T values[3] = { v0, v1, v2 };  SetValues(values, 3); }

		repeatmode ModeLeft, ModeRight;

		T Evaluate(factor X)
		{
			return EvaluateFP16(X * 65536.0f);
		}

		T EvaluateFP16(factor_fp16 X)
		{
			if (X > 0x10000)
			{
				switch (ModeRight)
				{
				case repeatmode::Clamp:
					X = 0x10000;
					break;
				case repeatmode::Repeat:
					{

						factor_fp16 nx = X & 0xFFFF;
						if(X<0)
							nx = -nx;
						X = nx;
					}

					break;
				}
			}
			else if (X < 0)
			{
				switch (ModeLeft)
				{
				case repeatmode::Clamp:
					X = 0;
					break;
				case repeatmode::Repeat:
					factor_fp16 nx = X & 0xFFFF;
					if(X<0)
						nx = -nx;
					X = nx;
					break;
				}
			}

			return EvaluateClampedFP16(X);
		}

		// Evaluate curve. Expects X to be in range of 0 and 0xFFFF.
		T EvaluateClampedFP16(factor_fp16 X)
		{
			factor_fp16 v = X * static_cast<factor_fp16>(Values.size() - 1);
			factor_fp16 idx = v >> 16;
			if ((idx << 16) == v)
				return Values[idx];

			factor fraction = (v & 0xFFFF) / 65536.0f;
			return cfc::math::lerp::Lerp(Values[idx], Values[idx + 1], fraction);
		}

		void SetValues(const T* values, int numValues)
		{
			Values.resize(numValues);
			if (numValues > 0)
			{
				for (int i = 0; i < numValues; i++)
					Values[i] = values[i];
			}
		}

		int GetNumValues() const																							{ return Values.size(); }
		const T& GetValueAsReference(int idx)																				{ return Values[idx]; }
		T GetValue(int idx)																									{ return Values[idx]; }
	protected:
		cfc::structures::array<T> Values;
	};

	template <typename T>
	class randomness
	{
	public:
		randomness(T randomMax=T()) { RandomMin = T(); RandomMax = randomMax; }
		randomness(T randomMin, T randomMax) { RandomMin = randomMin; RandomMax = randomMax; }

		T RandomMin, RandomMax;
		T Evaluate()
		{
			T rndValue;
			Randomized(rndValue);
			return rndValue;
		}

		void Randomized(cfc::math::vector3f& v)
		{
			factor c[] = { (factor)((double)rand() / (double)RAND_MAX), (factor)((double)rand() / (double)RAND_MAX), (factor)((double)rand() / (double)RAND_MAX), (factor)((double)rand() / (double)RAND_MAX) };
			for (int i = 0; i < 3; i++)
				v.V[i] = RandomMin.V[i] + (RandomMax.V[i] - RandomMin.V[i]) * c[i];
		}
		void Randomized(cfc::math::colorf& v)
		{
			factor c[] = { (factor)((double)rand() / (double)RAND_MAX), (factor)((double)rand() / (double)RAND_MAX), (factor)((double)rand() / (double)RAND_MAX), (factor)((double)rand() / (double)RAND_MAX) };
			for (int i = 0; i < 4; i++)
				v.rgba[i] = RandomMin.rgba[i] + (RandomMax.rgba[i] - RandomMin.rgba[i]) * c[i];
		}
		template <typename T2> void Randomized(T2& v)
		{
			v = cfc::math::lerp::Lerp(RandomMin, RandomMax, (factor)((double)rand() / (double)RAND_MAX));
		}
	};
	
	template <typename T>
	class descriptor
	{
	public:
		descriptor(T baseValue = T())
		{
			BaseValue = baseValue;
		}

		T BaseValue;
		cfc::structures::unqptr<curve<T> > Curves[3];
		cfc::structures::unqptr<randomness<T> > InitialRandomness;
	};

	template <typename T>
	class instance
	{
	public:
		instance() : m_description(nullptr) {}
		void SetDescriptor(descriptor<T>* desc)
		{
			if (desc == m_description)
				return;

			m_description = desc;
			m_constant = m_description->BaseValue;
			if (m_description->InitialRandomness)
				m_constant += m_description->InitialRandomness->Evaluate();
		}

		void Evaluate(factor X)
		{
			EvaluateFP16(static_cast<factor_fp16>(X * 65536.0f));
		}

		void EvaluateFP16(factor_fp16 X)
		{
			if (m_description == nullptr)
				CFC_BREAKPOINT;

			m_current = m_constant;
			if (m_description->Curves[0])
				m_current += m_description->Curves[0]->EvaluateFP16(X);
		}

		T GetCurrent() { return m_current; }
		const T& GetCurrentRef() { return m_current; }

		operator const T&() const {
			return m_current;
		}
	protected:
		descriptor<T>* m_description;
		T m_constant, m_current;
	};

}; // end namespace complexscalar
}; // end namespace math
}; // end namespace cfc	