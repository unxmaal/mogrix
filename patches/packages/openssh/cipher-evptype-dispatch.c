/*
 * IRIX rld fix: dispatch EVP cipher type by name instead of through
 * function pointers in a static array (rld fails some R_MIPS_REL32
 * relocations for function pointers in static data).
 *
 * This file is #included into cipher.c before cipher_init().
 */

static const EVP_CIPHER *
ssh_cipher_evptype(const char *name)
{
	if (strcmp(name, "3des-cbc") == 0) return EVP_des_ede3_cbc();
	if (strcmp(name, "aes128-cbc") == 0) return EVP_aes_128_cbc();
	if (strcmp(name, "aes192-cbc") == 0) return EVP_aes_192_cbc();
	if (strcmp(name, "aes256-cbc") == 0) return EVP_aes_256_cbc();
	if (strcmp(name, "aes128-ctr") == 0) return EVP_aes_128_ctr();
	if (strcmp(name, "aes192-ctr") == 0) return EVP_aes_192_ctr();
	if (strcmp(name, "aes256-ctr") == 0) return EVP_aes_256_ctr();
	if (strcmp(name, "aes128-gcm@openssh.com") == 0) return EVP_aes_128_gcm();
	if (strcmp(name, "aes256-gcm@openssh.com") == 0) return EVP_aes_256_gcm();
	return NULL;
}
