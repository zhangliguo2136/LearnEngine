#include "JPGLoader.h"
#include "stb_image.h"

void TJPGLoader::Load(const std::string& filename, TTextureInfo& TextureInfo)
{
	stbi_set_flip_vertically_on_load(true);
	int width, height, component;
	uint8_t* data = stbi_load(filename.c_str(), &width, &height, &component, STBI_rgb_alpha);
	if (data)
	{
		uint32_t CompSize = sizeof(uint8_t);
		uint32_t RowSize = width * STBI_rgb_alpha * CompSize;
		uint32_t TotalSize = width * height * STBI_rgb_alpha * CompSize;
		TextureInfo.Datas.resize(TotalSize);

		TextureInfo.Width = width;
		TextureInfo.Height = height;
		TextureInfo.Component = STBI_rgb_alpha;
		TextureInfo.ComponentSize = CompSize;
		TextureInfo.RowBytes = RowSize;
		TextureInfo.TotalBytes = TotalSize;

		printf("TJPGLoader: file %s, width %d, height %d, bpp %d\n", filename.c_str(), width, height, CompSize);

		memcpy_s(TextureInfo.Datas.data(), TotalSize, data, TotalSize);
	}

	stbi_image_free(data);
}
