
#ifdef __cplusplus
extern "C" {
#endif

// mipmapping
STBIDEF int   stbi_mipmap_image(const unsigned char* orig, int width, int height, int channels, unsigned char* resampled, int block_size_x, int block_size_y);
STBIDEF void  stbi_mipmap_info_dimensions(int width, int height, int mipLevel, int* outWidth, int* outHeight);
STBIDEF int   stbi_mipmap_info_quantity(int width, int height);
STBIDEF int   stbi_mipmap_info_bytes(int width, int height, int channels, int mipLevelStart, int mipCount);

#ifdef STB_IMAGE_IMPLEMENTATION

STBIDEF int   stbi_mipmap_image(const unsigned char* orig, int width, int height, int channels, unsigned char* resampled, int block_size_x, int block_size_y)
{
	int mip_width, mip_height;
	int i, j, c;

	/*	error check	*/
	if ((width < 1) || (height < 1) ||
		(channels < 1) || (orig == NULL) ||
		(resampled == NULL) ||
		(block_size_x < 1) || (block_size_y < 1))
	{
		/*	nothing to do	*/
		return 0;
	}
	mip_width = width / block_size_x;
	mip_height = height / block_size_y;
	if (mip_width < 1)
	{
		mip_width = 1;
	}
	if (mip_height < 1)
	{
		mip_height = 1;
	}
	for (j = 0; j < mip_height; ++j)
	{
		for (i = 0; i < mip_width; ++i)
		{
			for (c = 0; c < channels; ++c)
			{
				const int index = (j*block_size_y)*width*channels + (i*block_size_x)*channels + c;
				int sum_value;
				int u, v;
				int u_block = block_size_x;
				int v_block = block_size_y;
				int block_area;
				/*	do a bit of checking so we don't over-run the boundaries
				(necessary for non-square textures!)	*/
				if (block_size_x * (i + 1) > width)
				{
					u_block = width - i*block_size_y;
				}
				if (block_size_y * (j + 1) > height)
				{
					v_block = height - j*block_size_y;
				}
				block_area = u_block*v_block;
				/*	for this pixel, see what the average
				of all the values in the block are.
				note: start the sum at the rounding value, not at 0	*/
				sum_value = block_area >> 1;
				for (v = 0; v < v_block; ++v)
					for (u = 0; u < u_block; ++u)
					{
						sum_value += orig[index + v*width*channels + u*channels];
					}
				resampled[j*mip_width*channels + i*channels + c] = sum_value / block_area;
			}
		}
	}
	return 1;
}

STBIDEF void  stbi_mipmap_info_dimensions(int width, int height, int mipLevel, int* outWidth, int* outHeight)
{
	if (outWidth)
	{
		*outWidth = (width >> mipLevel); 
		*outWidth = (*outWidth == 0) ? 1 : *outWidth;
	}
	if (outHeight)
	{
		*outHeight = (height >> mipLevel);
		*outHeight = (*outHeight == 0) ? 1 : *outHeight;
	}

}
STBIDEF int   stbi_mipmap_info_quantity(int width, int height)
{
	int greatest = width > height ? width : height;
	int msb = 0;
	while (greatest > 0) { greatest >>= 1; msb++; }
	return msb;
}
STBIDEF int   stbi_mipmap_info_bytes(int width, int height, int channels, int mipLevelStart, int mipCount)
{
	int numBytes = 0;
	for (int i = mipLevelStart; i < mipLevelStart + mipCount; i++)
	{
		int w, h;
		stbi_mipmap_info_dimensions(width, height, i, &w, &h);
		numBytes += w*h*channels;
	}
	return numBytes;
}
#endif

#ifdef __cplusplus
}
#endif