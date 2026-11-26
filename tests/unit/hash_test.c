#include <test.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string_lib.h>
#include <hash.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bn.h>

static int stdout_saved = -1;
static char stdout_path[64];

static void StartCapturingLog(void)
{
    fflush(stdout);
    strlcpy(stdout_path, "/tmp/hash_test_log_XXXXXX", sizeof(stdout_path));
    int fd = mkstemp(stdout_path);
    assert_true(fd >= 0);
    stdout_saved = dup(STDOUT_FILENO);
    assert_true(stdout_saved >= 0);
    assert_true(dup2(fd, STDOUT_FILENO) >= 0);
    close(fd);
}

static void StopCapturingLog(char *const buffer, const size_t size)
{
    fflush(stdout);
    dup2(stdout_saved, STDOUT_FILENO);
    close(stdout_saved);
    stdout_saved = -1;

    size_t got = 0;
    FILE *file_in = fopen(stdout_path, "r");
    if (file_in != NULL)
    {
        got = fread(buffer, 1, size - 1, file_in);
        fclose(file_in);
    }
    buffer[got] = '\0';
    unlink(stdout_path);
}


/*
 * To test the hash implementation we need three things:
 * - A string
 * - A file
 * - A RSA key
 * We run one test for each, using two algorithms, MD5 and SHA256.
 */
static int initialized = 0;
static char message[] = "This is a message";
static int message_length = 0;
static char file[] = "/tmp/hashXXXXXX";
static int fd = -1;
static RSA *rsa = NULL;
void tests_setup()
{
    int result = 0;
    fd = mkstemp(file);
    if (fd < 0)
    {
        initialized = 0;
        return;
    }
    message_length = strlen(message);
    result = write(fd, message, message_length);
    if (result < 0)
    {
        close (fd);
        unlink (file);
        initialized = 0;
        return;
    }

    OPENSSL_init_crypto(0, NULL);
    initialized = 1;

    rsa = RSA_new();
    if (rsa)
    {
        BIGNUM *bn = NULL;
        bn = BN_new();
        if (!bn)
        {
            close (fd);
            unlink (file);
            RSA_free(rsa);
            initialized = 0;
            return;
        }
        BN_set_word(bn, RSA_F4);
        RSA_generate_key_ex(rsa, 1024, bn, NULL);
        BN_free(bn);
    }
}

void tests_teardown()
{
    if (fd >= 0)
    {
        close (fd);
        unlink (file);
    }
    if (rsa)
    {
        RSA_free(rsa);
    }
    initialized = 0;
}
#define ASSERT_IF_NOT_INITIALIZED \
    assert_int_equal(1, initialized)
/*
 * Tests
 * Each test does the same but from different sources, this is:
 * 1. Create a new hash structure using MD5
 * 2. Check the length of the generated hash.
 * 3. Check the printable version (check that is not NULL).
 * 4. Destroy the hash structure.
 * 5. Create a new hash structure using SHA256.
 * 6. Check the length of the generated hash.
 * 7. Check the printable version (check that is not NULL).
 * 8. Destroy the hash structure.
 */
static void test_HashString(void)
{
    ASSERT_IF_NOT_INITIALIZED;
    Hash *hash = NULL;
    unsigned int length = 0;
    assert_true(hash == NULL);
    hash = HashNew(message, message_length, HASH_METHOD_MD5);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_MD5, HashType(hash));
    assert_int_equal(CF_MD5_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_MD5_LEN);
    assert_true(HashPrintable(hash) != NULL);
    const char *md5_hash = HashPrintable(hash);
    assert_true((md5_hash[0] == 'M') && (md5_hash[1] == 'D') && (md5_hash[2] == '5') && (md5_hash[3] == '='));
    HashDestroy(&hash);
    assert_true(hash == NULL);
    hash = HashNew(message, message_length, HASH_METHOD_SHA256);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_SHA256, HashType(hash));
    assert_int_equal(CF_SHA256_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_SHA256_LEN);
    assert_true(HashPrintable(hash) != NULL);
    const char *sha256_hash = HashPrintable(hash);
    assert_true((sha256_hash[0] == 'S') && (sha256_hash[1] == 'H') && (sha256_hash[2] == 'A') && (sha256_hash[3] == '='));
    HashDestroy(&hash);
    /* Negative cases */
    assert_true(HashNew(NULL, message_length, HASH_METHOD_MD5) == NULL);
    assert_true(HashNew(message, 0, HASH_METHOD_MD5) == NULL);
    assert_true(HashNew(message, message_length, HASH_METHOD_NONE) == NULL);
    assert_true(HashNew(message, 0, HASH_METHOD_NONE) == NULL);
    assert_true(HashNew(NULL, message_length, HASH_METHOD_NONE) == NULL);
}

static void test_HashDescriptor(void)
{
    ASSERT_IF_NOT_INITIALIZED;
    Hash *hash = NULL;
    unsigned int length = 0;
    assert_true(hash == NULL);
    hash = HashNewFromDescriptor(fd, HASH_METHOD_MD5);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_MD5, HashType(hash));
    assert_int_equal(CF_MD5_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_MD5_LEN);
    assert_true(HashPrintable(hash) != NULL);
    HashDestroy(&hash);
    assert_true(hash == NULL);
    hash = HashNewFromDescriptor(fd, HASH_METHOD_SHA256);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_SHA256, HashType(hash));
    assert_int_equal(CF_SHA256_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_SHA256_LEN);
    assert_true(HashPrintable(hash) != NULL);
    HashDestroy(&hash);
    /* Negative cases */
    assert_true(HashNewFromDescriptor(-1, HASH_METHOD_MD5) == NULL);
    assert_true(HashNewFromDescriptor(fd, HASH_METHOD_NONE) == NULL);
    assert_true(HashNewFromDescriptor(-1, HASH_METHOD_NONE) == NULL);
}

static void test_HashKey(void)
{
    ASSERT_IF_NOT_INITIALIZED;
    Hash *hash = NULL;
    unsigned int length = 0;
    assert_true(hash == NULL);
    hash = HashNewFromKey(rsa, HASH_METHOD_MD5);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_MD5, HashType(hash));
    assert_int_equal(CF_MD5_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_MD5_LEN);
    assert_true(HashPrintable(hash) != NULL);
    HashDestroy(&hash);
    assert_true(hash == NULL);
    hash = HashNewFromKey(rsa, HASH_METHOD_SHA256);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_SHA256, HashType(hash));
    assert_int_equal(CF_SHA256_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_SHA256_LEN);
    assert_true(HashPrintable(hash) != NULL);
    HashDestroy(&hash);
    /* Negative cases */
    assert_true(HashNewFromKey(NULL, HASH_METHOD_MD5) == NULL);
    assert_true(HashNewFromKey(rsa, HASH_METHOD_NONE) == NULL);
    assert_true(HashNewFromKey(NULL, HASH_METHOD_NONE) == NULL);
}

static void test_HashCopy(void)
{
    ASSERT_IF_NOT_INITIALIZED;
    Hash *hash = NULL;
    Hash *copy = NULL;
    unsigned int length = 0;
    assert_true(hash == NULL);
    hash = HashNew(message, message_length, HASH_METHOD_MD5);
    assert_true(hash != NULL);
    assert_int_equal(HASH_METHOD_MD5, HashType(hash));
    assert_int_equal(CF_MD5_LEN, HashLength(hash));
    assert_true(HashData(hash, &length) != NULL);
    assert_int_equal(length, CF_MD5_LEN);
    assert_true(HashPrintable(hash) != NULL);
    assert_int_equal(0, HashCopy(hash, &copy));
    assert_int_equal(HASH_METHOD_MD5, HashType(copy));
    assert_int_equal(CF_MD5_LEN, HashLength(copy));
    assert_true(HashData(copy, &length) != NULL);
    assert_int_equal(length, CF_MD5_LEN);
    assert_true(HashPrintable(copy) != NULL);
    assert_string_equal(HashPrintable(hash), HashPrintable(copy));
    HashDestroy(&copy);
    assert_true(copy == NULL);
    /* Negative cases */
    assert_int_equal(-1, HashCopy(NULL, &copy));
    assert_int_equal(-1, HashCopy(hash, NULL));
    assert_int_equal(-1, HashCopy(NULL, NULL));
    /* Finish */
    HashDestroy(&hash);
    assert_true(hash == NULL);
}

static void test_HashesMatch(void)
{
    unsigned char digest_a[EVP_MAX_MD_SIZE + 1] = { 0 };
    unsigned char digest_b[EVP_MAX_MD_SIZE + 1] = { 0 };
    unsigned char digest_c[EVP_MAX_MD_SIZE + 1] = { 0 };
    unsigned char digest_d[EVP_MAX_MD_SIZE + 1] = { 0 };

    HashString("abc", 4, digest_a, HASH_METHOD_MD5);
    HashString("abc", 4, digest_b, HASH_METHOD_MD5);
    HashString("abc", 3, digest_c, HASH_METHOD_MD5);
    HashString("abd", 4, digest_d, HASH_METHOD_MD5);

    assert_true(HashesMatch(digest_a, digest_b, HASH_METHOD_MD5));
    assert_false(HashesMatch(digest_b, digest_c, HASH_METHOD_MD5));
    assert_false(HashesMatch(digest_c, digest_d, HASH_METHOD_MD5));
    assert_false(HashesMatch(digest_a, digest_d, HASH_METHOD_MD5));
}

static void test_StringCopyTruncateAndHashIfNecessary(void)
{
    char buf[40];
    int ret;

    const char *thirty_nine = "123456789_123456789_123456789_123456789";
    ret = StringCopyTruncateAndHashIfNecessary(thirty_nine, buf, 40);
    assert_string_equal(thirty_nine, buf);
    assert_int_equal(ret, 39);

    StringCopyTruncateAndHashIfNecessary("abc", buf, 40);
    assert_string_equal("abc", buf);

    const char *too_long = "The quick brown fox jumps over the lazy dog";
    ret = StringCopyTruncateAndHashIfNecessary(too_long, buf, 40);
    assert_int_equal(ret, 40);
    assert_false(StringEqual(too_long, buf));
    assert_true(strlen(too_long) != strlen(buf));
    assert_int_equal(strlen(buf), 39);
    assert_true(StringEqual(buf, "Th#MD5=9e107d9d372bb6826bd81d3542a419d6"));

    ret = StringCopyTruncateAndHashIfNecessary(too_long, buf, 39);
    assert_int_equal(ret, 39);
    assert_true(StringEqual(buf, "T#MD5=9e107d9d372bb6826bd81d3542a419d6"));

    ret = StringCopyTruncateAndHashIfNecessary(too_long, buf, 38);
    assert_int_equal(ret, 38);
    assert_true(StringEqual(buf, "#MD5=9e107d9d372bb6826bd81d3542a419d6"));
}

static void test_HashSha256KnownAnswer(void)
{
    ASSERT_IF_NOT_INITIALIZED;

    /* Independent known-answer SHA-256("This is a message"). */
    static const unsigned char expected_msg[CF_SHA256_LEN] = {
        0xa8, 0x26, 0xc7, 0xe3, 0x89, 0xec, 0x9f, 0x37,
        0x9c, 0xaf, 0xdc, 0x54, 0x4d, 0x7e, 0x9a, 0x43,
        0x95, 0xff, 0x7b, 0xfb, 0x58, 0x91, 0x7b, 0xbe,
        0xbe, 0xe5, 0x1b, 0x3d, 0x0b, 0x1c, 0x99, 0x6a
    };
    static const char expected_msg_printable[] =
        "SHA=a826c7e389ec9f379cafdc544d7e9a43"
        "95ff7bfb58917bbebee51b3d0b1c996a";

    /* Paired healthy control for HashNew(). */
    Hash *hash = HashNew(message, message_length, HASH_METHOD_SHA256);
    assert_true(hash != NULL);
    unsigned int length = 0;
    const unsigned char *data = HashData(hash, &length);
    assert_true(data != NULL);
    assert_int_equal(CF_SHA256_LEN, length);
    assert_int_equal(0, memcmp(data, expected_msg, CF_SHA256_LEN));
    assert_string_equal(expected_msg_printable, HashPrintable(hash));
    HashDestroy(&hash);

    /* Paired healthy control for HashFile(). */
    unsigned char file_digest[EVP_MAX_MD_SIZE + 1];
    memset(file_digest, 0xAA, sizeof(file_digest));
    assert_true(HashFile(file, file_digest, HASH_METHOD_SHA256, false));
    assert_int_equal(0, memcmp(file_digest, expected_msg, CF_SHA256_LEN));

    /* Paired healthy control for HashPubKey() using the same fixed RSA key
     * as hash_init_fail_test (n = 0xF00DFACE, e = RSA_F4 = 0x010001).
     * Expected digest is SHA-256(f0 0d fa ce 01 00 01). */
    static const unsigned char expected_pubkey[CF_SHA256_LEN] = {
        0x92, 0xb2, 0x48, 0x43, 0x58, 0x05, 0x91, 0xe4,
        0x5b, 0x4b, 0xf8, 0x9f, 0x88, 0x2f, 0xdb, 0x38,
        0xfd, 0x93, 0xf6, 0xdf, 0xfb, 0xc5, 0xe0, 0x41,
        0xc9, 0x7e, 0x43, 0xef, 0x62, 0x76, 0xa6, 0x43
    };
    RSA *fixed_rsa = RSA_new();
    assert_true(fixed_rsa != NULL);
    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();
    assert_true(n != NULL);
    assert_true(e != NULL);
    BN_set_word(n, 0xF00DFACE);
    BN_set_word(e, RSA_F4);
    assert_int_equal(1, RSA_set0_key(fixed_rsa, n, e, NULL));

    unsigned char pub_digest[EVP_MAX_MD_SIZE + 1];
    memset(pub_digest, 0xAA, sizeof(pub_digest));
    char log[4096];
    StartCapturingLog();
    HashPubKey(fixed_rsa, pub_digest, HASH_METHOD_SHA256);
    StopCapturingLog(log, sizeof(log));

    assert_int_equal(0, memcmp(pub_digest, expected_pubkey, CF_SHA256_LEN));
    assert_true(
        strstr(log, "Failed to initialize digest for hashing public key")
        == NULL);

    RSA_free(fixed_rsa);
}

/*
 * Main routine
 * Notice the calls to both setup and teardown.
 */
int main()
{
    PRINT_TEST_BANNER();
    tests_setup();
    const UnitTest tests[] =
    {
        unit_test(test_HashString),
        unit_test(test_HashDescriptor),
        unit_test(test_HashKey),
        unit_test(test_HashCopy),
        unit_test(test_HashesMatch),
        unit_test(test_StringCopyTruncateAndHashIfNecessary),
        unit_test(test_HashSha256KnownAnswer),
    };
    int result = run_tests(tests);
    tests_teardown();
    return result;
}
