
#pragma once

#include "fade.h"

namespace e
{
	class SimpleFade : public Fade
	{
	public:
		enum Type
		{
			typeAlpha,
			typeShutter,
		};
		const Type type;
		SimpleFade(Type _type, bool _useCurrentScreen = false);
		~SimpleFade();
		void Render() override;
	};
}
