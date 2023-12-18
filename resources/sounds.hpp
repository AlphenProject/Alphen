#include "custom_sounds.hpp"

__forceinline void setup_sounds()
{
	CreateDirectory("csgo\\sound", nullptr);
	FILE* file = nullptr;

	file = fopen(crypt_str("csgo\\sound\\flick.wav"), crypt_str("wb"));
	fwrite(flick, sizeof(unsigned char), 152168, file);
	fclose(file);
}
