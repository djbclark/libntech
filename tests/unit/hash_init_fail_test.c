#include <test.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <hash.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
#include <openssl/provider.h>
#endif

/*
 * Verifies the handling of EVP_DigestInit() / EVP_DigestInit_ex() failure:
 * HashNew() must return NULL, HashFile() must return false, and HashPubKey()
 * must report the failure rather than silently returning something that looks
 * like success.
 *
 * This is a separate program from hash_test on purpose.  The way to force
 * EVP_DigestInit*() to fail from a unit test, without mocking, is to unload
 * the OpenSSL 3 providers before anything in the process performs a digest
 * operation.  Once any digest has run, OpenSSL activates the default
 * provider as a fallback, and an explicit load+unload pair no longer
 * deactivates it -- appended to the end of hash_test.c, this test would
 * find EVP_DigestInit() still succeeding and fail spuriously.
 *
 * The drain below loads each provider exactly once and unloads it exactly
 * once.  Never unload more times than loaded: that crashes inside OpenSSL.
 * EVP_cleanup() and ERR_free_strings() are no-op macros since OpenSSL 1.1.0
 * and are deliberately not called.
 */

static bool init_failure_forced = false;

/*
 * HashPubKey() returns void and left the digest zeroed on failure before this
 * change too, so the digest alone does not distinguish fixed from unfixed --
 * the error message is the whole of what was added.  Asserting on it means
 * capturing the log, and there is no public reader for the buffer
 * StartLoggingIntoBuffer() fills, so redirect the stream Log() writes to
 * instead.  HashFile() needs none of this now that it returns bool.
 */
static int stdout_saved = -1;
static char stdout_path[64];

static void StartCapturingLog(void)
{
    fflush(stdout);
    strlcpy(stdout_path, "/tmp/hash_init_fail_log_XXXXXX", sizeof(stdout_path));
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
    FILE *file = fopen(stdout_path, "r");
    if (file != NULL)
    {
        got = fread(buffer, 1, size - 1, file);
        fclose(file);
    }
    buffer[got] = '\0';
    unlink(stdout_path);
}

static void drain_openssl_providers(void)
{
#if OPENSSL_VERSION_NUMBER >= 0x30000000L
    OSSL_PROVIDER *legacy = OSSL_PROVIDER_load(NULL, "legacy");
    OSSL_PROVIDER *dflt = OSSL_PROVIDER_load(NULL, "default");
    if (legacy != NULL)
    {
        OSSL_PROVIDER_unload(legacy);
    }
    if (dflt != NULL)
    {
        OSSL_PROVIDER_unload(dflt);
    }
#endif

    /* Precondition for the tests below: the digest must still be *known*, so
     * that the functions under test reach EVP_DigestInit*() rather than
     * their earlier md == NULL guard, while initialization itself fails.  In
     * environments where the drain cannot take effect -- OpenSSL before
     * 3.0, a provider activated by openssl.cnf, FIPS -- init_failure_forced
     * stays false and the tests skip rather than fail. */
    const EVP_MD *md = EVP_get_digestbyname("sha256");
    if (md == NULL)
    {
        return;
    }
    EVP_MD_CTX *context = EVP_MD_CTX_new();
    if (context == NULL)
    {
        return;
    }
    init_failure_forced = (EVP_DigestInit_ex(context, md, NULL) != 1);
    EVP_MD_CTX_free(context);
}

static void test_HashNew_returns_NULL_on_init_failure(void)
{
    if (!init_failure_forced)
    {
        return;
    }
    static const char message[] = "This is a message";
    Hash *hash = HashNew(message, strlen(message), HASH_METHOD_SHA256);
    assert_true(hash == NULL);
}

static void test_HashFile_reports_init_failure(void)
{
    if (!init_failure_forced)
    {
        return;
    }
    static const char message[] = "This is a message";
    char file[] = "/tmp/hash_init_fail_XXXXXX";
    int fd = mkstemp(file);
    assert_true(fd >= 0);
    ssize_t written = write(fd, message, strlen(message));
    assert_true(written == (ssize_t) strlen(message));

    unsigned char digest[EVP_MAX_MD_SIZE + 1];
    memset(digest, 0xAA, sizeof(digest));

    assert_false(HashFile(file, digest, HASH_METHOD_SHA256, false));
    for (size_t i = 0; i < sizeof(digest); i++)
    {
        assert_int_equal(digest[i], 0);
    }

    close(fd);
    unlink(file);
}

static void test_HashPubKey_leaves_zero_digest_on_init_failure(void)
{
    if (!init_failure_forced)
    {
        return;
    }
    /* Generating a key needs a live provider, which the drain removed, so
     * assemble one directly; HashPubKey() only reads n and e. */
    RSA *rsa = RSA_new();
    assert_true(rsa != NULL);
    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();
    assert_true(n != NULL);
    assert_true(e != NULL);
    BN_set_word(n, 0xF00DFACE);
    BN_set_word(e, RSA_F4);
    assert_int_equal(RSA_set0_key(rsa, n, e, NULL), 1);

    unsigned char digest[EVP_MAX_MD_SIZE + 1];
    memset(digest, 0xAA, sizeof(digest));
    char log[4096];
    StartCapturingLog();
    HashPubKey(rsa, digest, HASH_METHOD_SHA256);
    StopCapturingLog(log, sizeof(log));

    for (size_t i = 0; i < sizeof(digest); i++)
    {
        assert_int_equal(digest[i], 0);
    }
    /* As above: the digest was already zeroed before this change. */
    assert_true(
        strstr(log, "Failed to initialize digest for hashing public key")
        != NULL);

    RSA_free(rsa);
}

int main()
{
    PRINT_TEST_BANNER();
    drain_openssl_providers();
    if (!init_failure_forced)
    {
        puts("hash_init_fail_test: could not force digest initialization"
             " failure in this environment; tests will pass vacuously");
    }
    const UnitTest tests[] =
    {
        unit_test(test_HashNew_returns_NULL_on_init_failure),
        unit_test(test_HashFile_reports_init_failure),
        unit_test(test_HashPubKey_leaves_zero_digest_on_init_failure),
    };
    return run_tests(tests);
}
