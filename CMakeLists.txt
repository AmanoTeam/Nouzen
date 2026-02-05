cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(
	Nouzen
	VERSION 0.1
	DESCRIPTION "A command-line utility to download and install packages from APT repositories."
	HOMEPAGE_URL "https://github.com/AmanoTeam/Nouzen"
	LANGUAGES C
)

option(NOUZEN_ENABLE_LTO "Turn on compiler link-time optimizations" OFF)
option(NOUZEN_ENABLE_UNICODE "Enable Unicode support for Windows builds" ON)
option(NOUZEN_DBG "Build the main project with debugging symbols, even in release builds" OFF)
option(NOUZEN_BUILD_SHARED "Build the project as a shared library" OFF)

set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0069 NEW)
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(CMAKE_PLATFORM_NO_VERSIONED_SONAME ON)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

set(EXEC_RPATH "")
set(LIB_RPATH "")

if (APPLE)
	set(EXEC_RPATH "@loader_path/../lib")
	set(LIB_RPATH "@loader_path")
elseif (UNIX)
	set(EXEC_RPATH "$ORIGIN/../lib:$ORIGIN")
	set(LIB_RPATH "$ORIGIN")
endif()

set(
	WARNING_OPTIONS
	"-Wall"
	"-Wextra"
)

set(CURL_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/curl")
set(BEARSSL_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/bearssl")
set(NGHTTP2_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/nghttp2")
set(LIBARCHIVE_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/libarchive")
set(XZ_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/xz-utils")
set(ZLIB_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/zlib")
set(ZSTD_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/zstd")
set(BZIP2_SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/submodules/bzip2")

set(
	INCLUDE_DIRECTORIES
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${BEARSSL_SOURCE_DIRECTORY}/src"
	"${BEARSSL_SOURCE_DIRECTORY}/inc"
	"${CMAKE_CURRENT_BINARY_DIR}/submodules/nghttp2/lib/includes"
)

set(
	NOUZEN_SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/argparse.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ask.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/base_uri.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/biggestint.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/buffer.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/downloader.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/format.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/absrel.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/exists.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/getexec.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/mkdir.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/normpath.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/parentpath.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/realpath.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/rm.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/stripsep.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/splitext.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/chdir.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/symlink.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/chmod.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/walkdir.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/fs/fstream.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/term/screen.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/term/keyboard.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/wildcard_match.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/guess_file_format.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/guess_uri.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/hex.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/errors.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/logging.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/options.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/os/cpuinfo.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/os/envdir.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/os/find_exe.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/os/shell.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/os/osdetect.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/package.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/pprint.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/progress_callback.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/query.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/repository.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/sslcerts.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/strsplit.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/uncompress.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/urldecode.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/urlencode.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/wcurl.c"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/write_callback.c"
)

set(
	BEARSSL_SOURCE_FILES
	"${BEARSSL_SOURCE_DIRECTORY}/src/settings.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/aead/ccm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/aead/eax.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/aead/gcm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/ccopy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec16be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec16le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec32be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec32le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec64be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/dec64le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc16be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc16le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc32be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc32le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc64be.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/enc64le.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/pemdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/codec/pemenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_all_m15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_all_m31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_i15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_i31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m62.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_c25519_m64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_curve25519.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_default.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m62.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_p256_m64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_prime_i15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_prime_i31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_pubkey.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_secp256r1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_secp384r1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ec_secp521r1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_atr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_sign_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_sign_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_vrfy_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_default_vrfy_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_bits.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_sign_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_sign_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_vrfy_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i15_vrfy_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_bits.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_sign_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_sign_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_vrfy_asn1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_i31_vrfy_raw.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ec/ecdsa_rta.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/dig_oid.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/dig_size.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_ctmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_ctmul32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_ctmul64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_pclmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/ghash_pwr8.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/md5.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/md5sha1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/mgf1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/multihash.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/sha1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/sha2big.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/hash/sha2small.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_add.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_bitlen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_decmod.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_decode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_decred.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_encode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_fmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_iszero.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_moddiv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_modpow.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_modpow2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_montmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_mulacc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_muladd.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_ninv15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_reduce.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_rshift.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_sub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i15_tmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_add.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_bitlen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_decmod.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_decode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_decred.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_encode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_fmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_iszero.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_moddiv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_modpow.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_modpow2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_montmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_mulacc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_muladd.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_ninv31.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_reduce.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_rshift.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_sub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i31_tmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_add.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_bitlen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_decmod.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_decode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_decred.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_div32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_encode.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_fmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_iszero.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_modpow.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_montmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_mulacc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_muladd.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_ninv32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_reduce.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_sub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i32_tmont.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/int/i62_modpow2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/kdf/hkdf.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/kdf/shake.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/mac/hmac.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/mac/hmac_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rand/aesctr_drbg.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rand/hmac_drbg.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rand/sysrng.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_modulus.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_privexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_default_pubexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_modulus.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_privexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i15_pubexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_keygen_inner.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_modulus.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_privexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i31_pubexp.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i32_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_keygen.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_oaep_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_oaep_encrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pkcs1_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pkcs1_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_priv.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pss_sign.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pss_vrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_i62_pub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_oaep_pad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_oaep_unpad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pkcs1_sig_pad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pkcs1_sig_unpad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pss_sig_pad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_pss_sig_unpad.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/rsa/rsa_ssl_decrypt.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf_md5sha1.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf_sha256.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/prf_sha384.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_ccert_single_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_ccert_single_rsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_client.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_client_default_rsapub.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_client_full.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_aescbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_aesccm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_aesgcm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_chapol.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_descbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_ecdsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_engine_default_rsavrfy.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_hashes.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_hs_client.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_hs_server.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_io.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_keyexport.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_lru.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_cbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_ccm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_chapol.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_rec_gcm.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_scert_single_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_scert_single_rsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_full_ec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_full_rsa.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_mine2c.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_mine2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minf2c.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minf2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minr2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minu2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/ssl/ssl_server_minv2g.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_big_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_common.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct64_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_ct_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_pwr8_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_dec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_small_enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_ctr.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/aes_x86ni_ctrcbc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/chacha20_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/chacha20_sse2.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_ct.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_ct_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_ct_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_support.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_tab.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_tab_cbcdec.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/des_tab_cbcenc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_ctmul.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_ctmul32.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_ctmulq.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/symcipher/poly1305_i15.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/asn1enc.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_ec_pk8der.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_ec_rawder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_rsa_pk8der.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/encode_rsa_rawder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/skey_decoder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_decoder.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_knownkey.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_minimal.c"
	"${BEARSSL_SOURCE_DIRECTORY}/src/x509/x509_minimal_full.c"
)

set(
	TARGETS
	nz
	bearssl
	libcurl_shared
	libzstd_shared
	liblzma
	zlib
	archive
	nghttp2
	bz2
)


set(
	CURL_LIBRARY_DEPENDENCIES
	bearssl
	nghttp2
)

set(
	ARCHIVE_LIBRARY_DEPENDENCIES
	libzstd_shared
	liblzma
	zlib
	bz2
)

# curl
set(PICKY_COMPILER OFF)
set(BUILD_CURL_EXE OFF)
set(HTTP_ONLY ON)
set(ENABLE_THREADED_RESOLVER ON)
set(CURL_USE_BEARSSL ON)
set(CURL_USE_OPENSSL OFF)
set(CURL_USE_LIBPSL OFF)
set(CURL_DISABLE_ALTSVC ON)
set(CURL_DISABLE_CRYPTO_AUTH ON)
set(CURL_DISABLE_COOKIES OFF)
set(CURL_DISABLE_HSTS ON)
set(CURL_DISABLE_HTTP_AUTH ON)
set(CURL_DISABLE_NETRC ON)
set(CURL_DISABLE_WEBSOCKETS ON)
set(USE_LIBIDN2 OFF)
set(ENABLE_UNIX_SOCKETS OFF)
set(CURL_USE_LIBSSH2 OFF)
set(CURL_CA_BUNDLE "none")
set(CURL_CA_PATH "none")
set(CURL_WERROR OFF)
set(CURL_DISABLE_DOH OFF)
set(USE_NGHTTP2 ON)
set(BUILD_LIBCURL_DOCS OFF)
set(BUILD_MISC_DOCS OFF)
set(ENABLE_CURL_MANUAL OFF)
set(BUILD_TESTING OFF)
set(BUILD_EXAMPLES OFF)
set(CURL_ENABLE_EXPORT_TARGET OFF)

if (WIN32 AND NOUZEN_ENABLE_UNICODE)
	set(ENABLE_UNICODE ON)
endif()

# nghttp2
set(ENABLE_LIB_ONLY ON)
set(WITH_LIBXML2 OFF)
set(WITH_JEMALLOC OFF)
set(ENABLE_FAILMALLOC OFF)
set(ENABLE_DOC OFF)

# xz-utils
set(XZ_NLS OFF)

if (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	set(XZ_ASM_I386 OFF)
endif()

# zstd
set(BUILD_SHARED_LIBS ON)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_TESTS OFF)
set(ZSTD_BUILD_STATIC OFF)

# libarchive
set(HAVE_LIBZSTD ON)
set(HAVE_LZMA_STREAM_ENCODER_MT OFF)
set(LIBLZMA_HAS_AUTO_DECODER ON)
set(LIBLZMA_HAS_EASY_ENCODER ON)
set(LIBLZMA_HAS_LZMA_PRESET ON)
set(USE_BZIP2_DLL ON)
set(ENABLE_OPENSSL OFF)

# bzip2
set(ENABLE_LIB_ONLY ON)
set(ENABLE_SHARED_LIB ON)
set(ENABLE_STATIC_LIB OFF)

file(READ "${CURL_SOURCE_DIRECTORY}/CMake/FindBearSSL.cmake" FILE_CONTENTS) 
string(REPLACE "args(BEARSSL" "args(BearSSL" FILE_CONTENTS "${FILE_CONTENTS}") 
file(WRITE "${CURL_SOURCE_DIRECTORY}/CMake/FindBearSSL.cmake" "${FILE_CONTENTS}") 

file(READ "${CURL_SOURCE_DIRECTORY}/CMakeLists.txt" FILE_CONTENTS) 
string(REPLACE "DISABLE_FILE ON" "DISABLE_FILE OFF" FILE_CONTENTS "${FILE_CONTENTS}") 
file(WRITE "${CURL_SOURCE_DIRECTORY}/CMakeLists.txt" "${FILE_CONTENTS}") 

file(READ "${BZIP2_SOURCE_DIRECTORY}/CMakeLists.txt" FILE_CONTENTS) 
string(REPLACE "Python3_FOUND" "0" FILE_CONTENTS "${FILE_CONTENTS}") 
file(WRITE "${BZIP2_SOURCE_DIRECTORY}/CMakeLists.txt" "${FILE_CONTENTS}") 

file(READ "${ZLIB_SOURCE_DIRECTORY}/CMakeLists.txt" FILE_CONTENTS)
string(REPLACE "(UNIX)" "(1)" FILE_CONTENTS "${FILE_CONTENTS}")
string(REPLACE "(NOT APPLE)" "(0)" FILE_CONTENTS "${FILE_CONTENTS}")
file(WRITE "${ZLIB_SOURCE_DIRECTORY}/CMakeLists.txt" "${FILE_CONTENTS}")

file(READ "${LIBARCHIVE_SOURCE_DIRECTORY}/CMakeLists.txt" FILE_CONTENTS)
string(REPLACE "Haiku" "none" FILE_CONTENTS "${FILE_CONTENTS}")
file(WRITE "${LIBARCHIVE_SOURCE_DIRECTORY}/CMakeLists.txt" "${FILE_CONTENTS}")

include_directories(${INCLUDE_DIRECTORIES})

set(
	NGHTTP2_INCLUDE_DIR
	"${NGHTTP2_SOURCE_DIRECTORY}/lib/includes"
	"${CMAKE_CURRENT_BINARY_DIR}/submodules/nghttp2/lib/includes"
	CACHE PATH ""
)

set(NGHTTP2_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}nghttp2${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE FILEPATH "")

set(BEARSSL_INCLUDE_DIR "${BEARSSL_SOURCE_DIRECTORY}/inc" CACHE PATH "")
set(BEARSSL_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}bearssl${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE FILEPATH "")

set(BZIP2_INCLUDE_DIR "${BZIP2_SOURCE_DIRECTORY}" CACHE PATH "")
set(BZIP2_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}bz2${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE FILEPATH "")

set(ZLIB_INCLUDE_DIR "${ZLIB_SOURCE_DIRECTORY}" CACHE PATH "")
set(ZLIB_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}z${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE FILEPATH "")

set(ZSTD_INCLUDE_DIR "${ZSTD_SOURCE_DIRECTORY}/lib" CACHE PATH "")
set(ZSTD_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}zstd${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE FILEPATH "")

set(LIBLZMA_INCLUDE_DIR "${XZ_SOURCE_DIRECTORY}/src/liblzma/api" CACHE PATH "")
set(LIBLZMA_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}lzma${CMAKE_SHARED_LIBRARY_SUFFIX}" CACHE FILEPATH "")

add_library(
	bearssl
	SHARED
	${BEARSSL_SOURCE_FILES}
)

if (HAIKU)
	target_compile_definitions(
		bearssl
		PRIVATE
		BR_USE_URANDOM BR_USE_UNIX_TIME
	)
endif()

if (ANDROID)
	set(HAVE_FSEEKO 0)
endif()

add_subdirectory(
	"${ZSTD_SOURCE_DIRECTORY}/build/cmake"
	EXCLUDE_FROM_ALL
)

add_subdirectory(
	"${BZIP2_SOURCE_DIRECTORY}"
	EXCLUDE_FROM_ALL
)

add_subdirectory(
	"${ZLIB_SOURCE_DIRECTORY}"
	EXCLUDE_FROM_ALL
)

add_subdirectory(
	"${CURL_SOURCE_DIRECTORY}"
	EXCLUDE_FROM_ALL
)

add_subdirectory(
	"${XZ_SOURCE_DIRECTORY}"
	EXCLUDE_FROM_ALL
)

add_subdirectory(
	"${LIBARCHIVE_SOURCE_DIRECTORY}"
	EXCLUDE_FROM_ALL
)

add_subdirectory(
	"${NGHTTP2_SOURCE_DIRECTORY}"
	EXCLUDE_FROM_ALL
)

target_compile_definitions(
	libzstd_shared
	PRIVATE
	ZDICT_QSORT=ZDICT_QSORT_MIN
)

if (CMAKE_SYSTEM_NAME STREQUAL "OpenBSD")
	target_compile_options(
		liblzma
		PRIVATE
		-include sys/types.h
	)
endif()

if (NOT WIN32)
	target_compile_definitions(
		nghttp2
		PRIVATE
		HAVE_ARPA_INET_H HAVE_NETINET_IN_H
	)
endif()

foreach(target ${CURL_LIBRARY_DEPENDENCIES})
	add_custom_command(
		OUTPUT ${target}
		COMMAND ${CMAKE_COMMAND} --build ./ --target ${target}
	)
	
	add_custom_target(
		"ensure_${target}" ALL DEPENDS ${target}
	)
	
	add_dependencies(
		libcurl_shared
		"ensure_${target}"
	)
endforeach()

foreach(target ${ARCHIVE_LIBRARY_DEPENDENCIES})
	add_custom_command(
		OUTPUT ${target}
		COMMAND ${CMAKE_COMMAND} --build ./ --target ${target}
	)
	
	add_custom_target(
		"ensure_${target}" ALL DEPENDS ${target}
	)
	
	add_dependencies(
		archive
		"ensure_${target}"
	)
endforeach()

set_target_properties(
	libcurl_shared
	PROPERTIES
	IMPORTED_LOCATION "${BEARSSL_LIBRARY}"
)

if (NOUZEN_BUILD_SHARED)
	add_library(
		nz
		SHARED
		${NOUZEN_SOURCE_FILES}
	)
else()
	add_executable(
		nz
		${NOUZEN_SOURCE_FILES}
	)
	
	target_sources(
		nz
		PRIVATE
		"${CMAKE_CURRENT_SOURCE_DIR}/src/main.c"
	)
endif()

target_compile_options(
	nz
	PRIVATE
	${WARNING_OPTIONS}
)

if (NOUZEN_DBG)
	target_compile_options(
		nz
		PRIVATE
		-g
	)
endif()

target_compile_definitions(
	nz
	PRIVATE
	CURL_SKIP_INCLUDE_MPRINTF
)

set_target_properties(
	nz
	PROPERTIES
	BUILD_RPATH "${EXEC_RPATH}"
	INSTALL_RPATH "${EXEC_RPATH}"
)

set_target_properties(
	libcurl_shared archive
	PROPERTIES
	BUILD_RPATH "${LIB_RPATH}"
	INSTALL_RPATH "${LIB_RPATH}"
)

# Enable Unicode on Windows builds
if (WIN32 AND NOUZEN_ENABLE_UNICODE)
	target_sources(
		nz
		PRIVATE
		"src/wio.c"
	)
	
	target_compile_definitions(
		nz
		PRIVATE
		UNICODE _UNICODE
	)
	
	target_link_options(
		nz
		BEFORE
		PRIVATE
		-municode
	)
endif()

if (NOUZEN_ENABLE_LTO)
	set(NOUZEN_HAS_LTO OFF)
	
	include(CheckIPOSupported)
	check_ipo_supported(RESULT NOUZEN_HAS_LTO LANGUAGES C)
	
	if (NOUZEN_HAS_LTO)
		set_target_properties(
			${TARGETS}
			PROPERTIES
			INTERPROCEDURAL_OPTIMIZATION TRUE
		)
	endif()
endif()

target_link_libraries(
	nz
	libcurl_shared
	archive
)

install(
	TARGETS ${TARGETS}
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib NAMELINK_SKIP
)

install(
	FILES "${CMAKE_CURRENT_SOURCE_DIR}/submodules/cabundle/pem/cert.pem"
	DESTINATION etc/tls
	RENAME cert.pem
)

install(
	FILES "${CMAKE_CURRENT_SOURCE_DIR}/options.conf"
	DESTINATION etc/nouzen
)

install(
	PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/tools/apt.sh"
	TYPE BIN
	RENAME apt
)

install(
	PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/tools/apt.sh"
	TYPE BIN
	RENAME apt-get
)
