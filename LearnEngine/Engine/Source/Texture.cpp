#include "Texture.h"

TTexture2D::TTexture2D(const std::string& InName, std::string InFilePath)
	:TTexture(InName, ETextureType::TEXTURE_2D, InFilePath)
{
}

TTexture2D::~TTexture2D()
{

}
