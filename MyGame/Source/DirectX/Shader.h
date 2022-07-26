#pragma once

#include "DirectXIncludes.h"

#include <glm/glm.hpp>

#include <string>

namespace MyGame
{
	class Shader
	{
	public:

		// Dx Compiler
		static bool CompileVertexShader(Microsoft::WRL::ComPtr<IDxcBlob>&, const std::string&);
		static bool CompilePixelShader(Microsoft::WRL::ComPtr<IDxcBlob>&, const std::string&);

		static bool CompileFromFile(Microsoft::WRL::ComPtr<IDxcBlob>&, const std::string&, const std::string&);


		// D3D Compiler
		static bool D3CompileVertexShader(Microsoft::WRL::ComPtr<ID3DBlob>&, const std::string&);
		static bool D3CompilePixelShader(Microsoft::WRL::ComPtr<ID3DBlob>&, const std::string&);

		static bool D3CompileFromFile(Microsoft::WRL::ComPtr<ID3DBlob>&, const std::string&, const std::string&);

	private:
	};
}
