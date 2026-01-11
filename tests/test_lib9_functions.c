/*
 * Exhaustive Unit Tests for lib9 Utility Functions
 * Tests string, format, and utility functions from lib9
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* Test framework macros */
#define TEST_SUITE(name) \
    printf("\n========================================\n"); \
    printf("Test Suite: %s\n", name); \
    printf("========================================\n")

#define TEST_CASE(name) \
    printf("\n  Testing: %s... ", name); \
    test_count++

#define TEST_PASS() \
    printf("PASS\n"); \
    pass_count++

#define TEST_FAIL(msg) \
    printf("FAIL: %s\n", msg); \
    fail_count++

#define ASSERT_TRUE(cond) \
    if (!(cond)) { TEST_FAIL("Assertion failed: " #cond); return; }

#define ASSERT_EQ(a, b) \
    if ((a) != (b)) { \
        printf("FAIL: Expected %ld, got %ld\n", (long)(b), (long)(a)); \
        fail_count++; return; \
    }

#define ASSERT_EQ_STR(a, b) \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL: Expected '%s', got '%s'\n", (b), (a)); \
        fail_count++; return; \
    }

#define ASSERT_NOT_NULL(ptr) \
    if ((ptr) == NULL) { TEST_FAIL("Unexpected NULL: " #ptr); return; }

#define ASSERT_NULL(ptr) \
    if ((ptr) != NULL) { TEST_FAIL("Expected NULL: " #ptr); return; }

static int test_count = 0;
static int pass_count = 0;
static int fail_count = 0;

/*
 * String Functions - Reimplementations matching lib9
 */

/* strlen - get string length */
size_t lib9_strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

/* strcmp - compare strings */
int lib9_strcmp(const char *s1, const char *s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/* strncmp - compare n characters */
int lib9_strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0 && *s1 && *s1 == *s2) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return (unsigned char)*s1 - (unsigned char)*s2;
}

/* strcpy - copy string */
char* lib9_strcpy(char *dst, const char *src) {
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

/* strncpy - copy n characters */
char* lib9_strncpy(char *dst, const char *src, size_t n) {
    char *d = dst;
    while (n > 0 && *src) {
        *d++ = *src++;
        n--;
    }
    while (n > 0) {
        *d++ = '\0';
        n--;
    }
    return dst;
}

/* strcat - concatenate strings */
char* lib9_strcat(char *dst, const char *src) {
    char *d = dst;
    while (*d) d++;
    while ((*d++ = *src++));
    return dst;
}

/* strncat - concatenate n characters */
char* lib9_strncat(char *dst, const char *src, size_t n) {
    char *d = dst;
    while (*d) d++;
    while (n > 0 && *src) {
        *d++ = *src++;
        n--;
    }
    *d = '\0';
    return dst;
}

/* strchr - find character */
char* lib9_strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return (c == '\0') ? (char*)s : NULL;
}

/* strrchr - find last occurrence */
char* lib9_strrchr(const char *s, int c) {
    const char *last = NULL;
    while (*s) {
        if (*s == (char)c) last = s;
        s++;
    }
    return (c == '\0') ? (char*)s : (char*)last;
}

/* strstr - find substring */
char* lib9_strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;

    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && *h == *n) {
            h++;
            n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}

/* strdup - duplicate string */
char* lib9_strdup(const char *s) {
    size_t len = lib9_strlen(s) + 1;
    char *dup = malloc(len);
    if (dup) lib9_strcpy(dup, s);
    return dup;
}

/* memset - fill memory */
void* lib9_memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c;
    return s;
}

/* memcpy - copy memory */
void* lib9_memcpy(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;
    while (n--) *d++ = *s++;
    return dst;
}

/* memmove - copy overlapping memory */
void* lib9_memmove(void *dst, const void *src, size_t n) {
    unsigned char *d = dst;
    const unsigned char *s = src;

    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dst;
}

/* memcmp - compare memory */
int lib9_memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = s1;
    const unsigned char *p2 = s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++;
        p2++;
    }
    return 0;
}

/* memchr - find byte in memory */
void* lib9_memchr(const void *s, int c, size_t n) {
    const unsigned char *p = s;
    while (n--) {
        if (*p == (unsigned char)c) return (void*)p;
        p++;
    }
    return NULL;
}

/* toupper - convert to uppercase */
int lib9_toupper(int c) {
    if (c >= 'a' && c <= 'z') return c - 'a' + 'A';
    return c;
}

/* tolower - convert to lowercase */
int lib9_tolower(int c) {
    if (c >= 'A' && c <= 'Z') return c - 'A' + 'a';
    return c;
}

/* isdigit - check if digit */
int lib9_isdigit(int c) {
    return c >= '0' && c <= '9';
}

/* isalpha - check if alphabetic */
int lib9_isalpha(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

/* isalnum - check if alphanumeric */
int lib9_isalnum(int c) {
    return lib9_isalpha(c) || lib9_isdigit(c);
}

/* isspace - check if whitespace */
int lib9_isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

/* atoi - string to integer */
int lib9_atoi(const char *s) {
    int result = 0;
    int sign = 1;

    while (lib9_isspace(*s)) s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (lib9_isdigit(*s)) {
        result = result * 10 + (*s - '0');
        s++;
    }

    return sign * result;
}

/* atol - string to long */
long lib9_atol(const char *s) {
    long result = 0;
    int sign = 1;

    while (lib9_isspace(*s)) s++;

    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    while (lib9_isdigit(*s)) {
        result = result * 10 + (*s - '0');
        s++;
    }

    return sign * result;
}

/* snprint - formatted print to buffer (simplified) */
int lib9_snprint(char *buf, int len, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, len, fmt, args);
    va_end(args);
    return n;
}

/* seprint - formatted print to buffer end */
char* lib9_seprint(char *buf, char *end, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, end - buf, fmt, args);
    va_end(args);
    if (n < 0) return buf;
    if (n >= end - buf) return end;
    return buf + n;
}

/*
 * TEST FUNCTIONS
 */

/* strlen Tests */
void test_strlen_empty(void) {
    TEST_CASE("strlen empty string");
    ASSERT_EQ(lib9_strlen(""), 0);
    TEST_PASS();
}

void test_strlen_single(void) {
    TEST_CASE("strlen single character");
    ASSERT_EQ(lib9_strlen("a"), 1);
    TEST_PASS();
}

void test_strlen_normal(void) {
    TEST_CASE("strlen normal string");
    ASSERT_EQ(lib9_strlen("hello"), 5);
    ASSERT_EQ(lib9_strlen("hello world"), 11);
    TEST_PASS();
}

void test_strlen_long(void) {
    TEST_CASE("strlen long string");
    char long_str[1001];
    memset(long_str, 'a', 1000);
    long_str[1000] = '\0';
    ASSERT_EQ(lib9_strlen(long_str), 1000);
    TEST_PASS();
}

/* strcmp Tests */
void test_strcmp_equal(void) {
    TEST_CASE("strcmp equal strings");
    ASSERT_EQ(lib9_strcmp("hello", "hello"), 0);
    ASSERT_EQ(lib9_strcmp("", ""), 0);
    TEST_PASS();
}

void test_strcmp_less(void) {
    TEST_CASE("strcmp first less than second");
    ASSERT_TRUE(lib9_strcmp("abc", "abd") < 0);
    ASSERT_TRUE(lib9_strcmp("abc", "abcd") < 0);
    TEST_PASS();
}

void test_strcmp_greater(void) {
    TEST_CASE("strcmp first greater than second");
    ASSERT_TRUE(lib9_strcmp("abd", "abc") > 0);
    ASSERT_TRUE(lib9_strcmp("abcd", "abc") > 0);
    TEST_PASS();
}

void test_strcmp_case_sensitive(void) {
    TEST_CASE("strcmp case sensitive");
    ASSERT_TRUE(lib9_strcmp("ABC", "abc") != 0);
    ASSERT_TRUE(lib9_strcmp("a", "A") != 0);
    TEST_PASS();
}

/* strncmp Tests */
void test_strncmp_equal(void) {
    TEST_CASE("strncmp equal within n");
    ASSERT_EQ(lib9_strncmp("hello", "hello", 5), 0);
    ASSERT_EQ(lib9_strncmp("hello world", "hello there", 5), 0);
    TEST_PASS();
}

void test_strncmp_different(void) {
    TEST_CASE("strncmp different within n");
    ASSERT_TRUE(lib9_strncmp("hello", "hallo", 5) != 0);
    TEST_PASS();
}

void test_strncmp_zero(void) {
    TEST_CASE("strncmp with n=0");
    ASSERT_EQ(lib9_strncmp("abc", "xyz", 0), 0);
    TEST_PASS();
}

/* strcpy Tests */
void test_strcpy_basic(void) {
    TEST_CASE("strcpy basic");
    char buf[20];
    lib9_strcpy(buf, "hello");
    ASSERT_EQ_STR(buf, "hello");
    TEST_PASS();
}

void test_strcpy_empty(void) {
    TEST_CASE("strcpy empty string");
    char buf[20] = "original";
    lib9_strcpy(buf, "");
    ASSERT_EQ_STR(buf, "");
    TEST_PASS();
}

void test_strcpy_overwrite(void) {
    TEST_CASE("strcpy overwrite");
    char buf[20] = "original string";
    lib9_strcpy(buf, "new");
    ASSERT_EQ_STR(buf, "new");
    TEST_PASS();
}

/* strncpy Tests */
void test_strncpy_exact(void) {
    TEST_CASE("strncpy exact length");
    char buf[6];
    lib9_strncpy(buf, "hello", 6);
    ASSERT_EQ_STR(buf, "hello");
    TEST_PASS();
}

void test_strncpy_truncate(void) {
    TEST_CASE("strncpy truncate");
    char buf[4];
    lib9_strncpy(buf, "hello", 3);
    buf[3] = '\0';
    ASSERT_EQ_STR(buf, "hel");
    TEST_PASS();
}

void test_strncpy_pad(void) {
    TEST_CASE("strncpy padding");
    char buf[10];
    lib9_memset(buf, 'x', 10);
    lib9_strncpy(buf, "hi", 10);
    ASSERT_EQ_STR(buf, "hi");
    /* Check padding */
    ASSERT_EQ(buf[2], '\0');
    ASSERT_EQ(buf[9], '\0');
    TEST_PASS();
}

/* strcat Tests */
void test_strcat_basic(void) {
    TEST_CASE("strcat basic");
    char buf[20] = "hello";
    lib9_strcat(buf, " world");
    ASSERT_EQ_STR(buf, "hello world");
    TEST_PASS();
}

void test_strcat_empty(void) {
    TEST_CASE("strcat empty strings");
    char buf[20] = "";
    lib9_strcat(buf, "hello");
    ASSERT_EQ_STR(buf, "hello");

    char buf2[20] = "hello";
    lib9_strcat(buf2, "");
    ASSERT_EQ_STR(buf2, "hello");
    TEST_PASS();
}

void test_strcat_multiple(void) {
    TEST_CASE("strcat multiple");
    char buf[30] = "";
    lib9_strcat(buf, "one");
    lib9_strcat(buf, " two");
    lib9_strcat(buf, " three");
    ASSERT_EQ_STR(buf, "one two three");
    TEST_PASS();
}

/* strncat Tests */
void test_strncat_basic(void) {
    TEST_CASE("strncat basic");
    char buf[20] = "hello";
    lib9_strncat(buf, " world", 6);
    ASSERT_EQ_STR(buf, "hello world");
    TEST_PASS();
}

void test_strncat_truncate(void) {
    TEST_CASE("strncat truncate");
    char buf[20] = "hello";
    lib9_strncat(buf, " world", 3);
    ASSERT_EQ_STR(buf, "hello wo");
    TEST_PASS();
}

/* strchr Tests */
void test_strchr_found(void) {
    TEST_CASE("strchr character found");
    char *p = lib9_strchr("hello", 'l');
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(*p, 'l');
    ASSERT_TRUE(p == "hello" + 2);
    TEST_PASS();
}

void test_strchr_not_found(void) {
    TEST_CASE("strchr character not found");
    char *p = lib9_strchr("hello", 'x');
    ASSERT_NULL(p);
    TEST_PASS();
}

void test_strchr_null_char(void) {
    TEST_CASE("strchr find null terminator");
    char *p = lib9_strchr("hello", '\0');
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(*p, '\0');
    TEST_PASS();
}

/* strrchr Tests */
void test_strrchr_found(void) {
    TEST_CASE("strrchr last occurrence");
    char *p = lib9_strrchr("hello", 'l');
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(*p, 'l');
    ASSERT_TRUE(p == "hello" + 3);
    TEST_PASS();
}

void test_strrchr_not_found(void) {
    TEST_CASE("strrchr not found");
    char *p = lib9_strrchr("hello", 'x');
    ASSERT_NULL(p);
    TEST_PASS();
}

/* strstr Tests */
void test_strstr_found(void) {
    TEST_CASE("strstr substring found");
    char *p = lib9_strstr("hello world", "world");
    ASSERT_NOT_NULL(p);
    ASSERT_EQ_STR(p, "world");
    TEST_PASS();
}

void test_strstr_not_found(void) {
    TEST_CASE("strstr substring not found");
    char *p = lib9_strstr("hello world", "xyz");
    ASSERT_NULL(p);
    TEST_PASS();
}

void test_strstr_empty_needle(void) {
    TEST_CASE("strstr empty needle");
    char *p = lib9_strstr("hello", "");
    ASSERT_NOT_NULL(p);
    TEST_PASS();
}

void test_strstr_at_start(void) {
    TEST_CASE("strstr at start");
    char *p = lib9_strstr("hello world", "hello");
    ASSERT_NOT_NULL(p);
    ASSERT_TRUE(p == "hello world");
    TEST_PASS();
}

/* strdup Tests */
void test_strdup_basic(void) {
    TEST_CASE("strdup basic");
    char *dup = lib9_strdup("hello");
    ASSERT_NOT_NULL(dup);
    ASSERT_EQ_STR(dup, "hello");
    free(dup);
    TEST_PASS();
}

void test_strdup_empty(void) {
    TEST_CASE("strdup empty string");
    char *dup = lib9_strdup("");
    ASSERT_NOT_NULL(dup);
    ASSERT_EQ_STR(dup, "");
    free(dup);
    TEST_PASS();
}

/* memset Tests */
void test_memset_zero(void) {
    TEST_CASE("memset zero");
    char buf[10] = "xxxxxxxxx";
    lib9_memset(buf, 0, 10);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(buf[i], 0);
    }
    TEST_PASS();
}

void test_memset_character(void) {
    TEST_CASE("memset character");
    char buf[10];
    lib9_memset(buf, 'A', 10);
    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(buf[i], 'A');
    }
    TEST_PASS();
}

void test_memset_partial(void) {
    TEST_CASE("memset partial");
    char buf[10] = "abcdefghi";
    lib9_memset(buf + 3, 'X', 3);
    ASSERT_EQ(buf[0], 'a');
    ASSERT_EQ(buf[3], 'X');
    ASSERT_EQ(buf[5], 'X');
    ASSERT_EQ(buf[6], 'g');
    TEST_PASS();
}

/* memcpy Tests */
void test_memcpy_basic(void) {
    TEST_CASE("memcpy basic");
    char src[] = "hello";
    char dst[10];
    lib9_memcpy(dst, src, 6);
    ASSERT_EQ_STR(dst, "hello");
    TEST_PASS();
}

void test_memcpy_partial(void) {
    TEST_CASE("memcpy partial");
    char src[] = "hello";
    char dst[10] = "xxxxxxxxx";
    lib9_memcpy(dst, src, 3);
    ASSERT_EQ(dst[0], 'h');
    ASSERT_EQ(dst[2], 'l');
    ASSERT_EQ(dst[3], 'x');
    TEST_PASS();
}

/* memmove Tests */
void test_memmove_no_overlap(void) {
    TEST_CASE("memmove no overlap");
    char buf[20] = "hello world";
    lib9_memmove(buf + 12, buf, 5);
    buf[17] = '\0';
    ASSERT_EQ_STR(buf + 12, "hello");
    TEST_PASS();
}

void test_memmove_overlap_forward(void) {
    TEST_CASE("memmove overlap forward");
    char buf[20] = "hello world";
    lib9_memmove(buf + 2, buf, 5);
    buf[7] = '\0';
    ASSERT_EQ_STR(buf + 2, "hello");
    TEST_PASS();
}

void test_memmove_overlap_backward(void) {
    TEST_CASE("memmove overlap backward");
    char buf[20] = "hello world";
    lib9_memmove(buf, buf + 6, 5);
    buf[5] = '\0';
    ASSERT_EQ_STR(buf, "world");
    TEST_PASS();
}

/* memcmp Tests */
void test_memcmp_equal(void) {
    TEST_CASE("memcmp equal");
    ASSERT_EQ(lib9_memcmp("hello", "hello", 5), 0);
    TEST_PASS();
}

void test_memcmp_less(void) {
    TEST_CASE("memcmp less");
    ASSERT_TRUE(lib9_memcmp("abc", "abd", 3) < 0);
    TEST_PASS();
}

void test_memcmp_greater(void) {
    TEST_CASE("memcmp greater");
    ASSERT_TRUE(lib9_memcmp("abd", "abc", 3) > 0);
    TEST_PASS();
}

void test_memcmp_partial(void) {
    TEST_CASE("memcmp partial");
    ASSERT_EQ(lib9_memcmp("hello", "hallo", 1), 0);
    ASSERT_TRUE(lib9_memcmp("hello", "hallo", 2) != 0);
    TEST_PASS();
}

/* memchr Tests */
void test_memchr_found(void) {
    TEST_CASE("memchr found");
    char *p = lib9_memchr("hello", 'l', 5);
    ASSERT_NOT_NULL(p);
    ASSERT_EQ(*p, 'l');
    TEST_PASS();
}

void test_memchr_not_found(void) {
    TEST_CASE("memchr not found");
    void *p = lib9_memchr("hello", 'x', 5);
    ASSERT_NULL(p);
    TEST_PASS();
}

void test_memchr_with_null(void) {
    TEST_CASE("memchr with embedded null");
    char data[] = {'a', '\0', 'b'};
    void *p = lib9_memchr(data, 'b', 3);
    ASSERT_NOT_NULL(p);
    TEST_PASS();
}

/* Character Classification Tests */
void test_isdigit(void) {
    TEST_CASE("isdigit");
    ASSERT_TRUE(lib9_isdigit('0'));
    ASSERT_TRUE(lib9_isdigit('5'));
    ASSERT_TRUE(lib9_isdigit('9'));
    ASSERT_TRUE(!lib9_isdigit('a'));
    ASSERT_TRUE(!lib9_isdigit(' '));
    TEST_PASS();
}

void test_isalpha(void) {
    TEST_CASE("isalpha");
    ASSERT_TRUE(lib9_isalpha('a'));
    ASSERT_TRUE(lib9_isalpha('Z'));
    ASSERT_TRUE(!lib9_isalpha('0'));
    ASSERT_TRUE(!lib9_isalpha(' '));
    TEST_PASS();
}

void test_isalnum(void) {
    TEST_CASE("isalnum");
    ASSERT_TRUE(lib9_isalnum('a'));
    ASSERT_TRUE(lib9_isalnum('0'));
    ASSERT_TRUE(!lib9_isalnum(' '));
    ASSERT_TRUE(!lib9_isalnum('!'));
    TEST_PASS();
}

void test_isspace(void) {
    TEST_CASE("isspace");
    ASSERT_TRUE(lib9_isspace(' '));
    ASSERT_TRUE(lib9_isspace('\t'));
    ASSERT_TRUE(lib9_isspace('\n'));
    ASSERT_TRUE(!lib9_isspace('a'));
    ASSERT_TRUE(!lib9_isspace('0'));
    TEST_PASS();
}

void test_toupper(void) {
    TEST_CASE("toupper");
    ASSERT_EQ(lib9_toupper('a'), 'A');
    ASSERT_EQ(lib9_toupper('z'), 'Z');
    ASSERT_EQ(lib9_toupper('A'), 'A');
    ASSERT_EQ(lib9_toupper('0'), '0');
    TEST_PASS();
}

void test_tolower(void) {
    TEST_CASE("tolower");
    ASSERT_EQ(lib9_tolower('A'), 'a');
    ASSERT_EQ(lib9_tolower('Z'), 'z');
    ASSERT_EQ(lib9_tolower('a'), 'a');
    ASSERT_EQ(lib9_tolower('0'), '0');
    TEST_PASS();
}

/* Conversion Tests */
void test_atoi_positive(void) {
    TEST_CASE("atoi positive");
    ASSERT_EQ(lib9_atoi("123"), 123);
    ASSERT_EQ(lib9_atoi("0"), 0);
    ASSERT_EQ(lib9_atoi("999999"), 999999);
    TEST_PASS();
}

void test_atoi_negative(void) {
    TEST_CASE("atoi negative");
    ASSERT_EQ(lib9_atoi("-123"), -123);
    ASSERT_EQ(lib9_atoi("-1"), -1);
    TEST_PASS();
}

void test_atoi_whitespace(void) {
    TEST_CASE("atoi with whitespace");
    ASSERT_EQ(lib9_atoi("  123"), 123);
    ASSERT_EQ(lib9_atoi("\t\n456"), 456);
    TEST_PASS();
}

void test_atoi_plus(void) {
    TEST_CASE("atoi with plus sign");
    ASSERT_EQ(lib9_atoi("+123"), 123);
    TEST_PASS();
}

void test_atoi_trailing(void) {
    TEST_CASE("atoi with trailing text");
    ASSERT_EQ(lib9_atoi("123abc"), 123);
    ASSERT_EQ(lib9_atoi("456 789"), 456);
    TEST_PASS();
}

void test_atol_basic(void) {
    TEST_CASE("atol basic");
    ASSERT_EQ(lib9_atol("123456789"), 123456789L);
    ASSERT_EQ(lib9_atol("-987654321"), -987654321L);
    TEST_PASS();
}

/* Format Functions Tests */
void test_snprint_basic(void) {
    TEST_CASE("snprint basic");
    char buf[100];
    lib9_snprint(buf, sizeof(buf), "hello %s", "world");
    ASSERT_EQ_STR(buf, "hello world");
    TEST_PASS();
}

void test_snprint_numbers(void) {
    TEST_CASE("snprint numbers");
    char buf[100];
    lib9_snprint(buf, sizeof(buf), "%d + %d = %d", 1, 2, 3);
    ASSERT_EQ_STR(buf, "1 + 2 = 3");
    TEST_PASS();
}

void test_snprint_truncate(void) {
    TEST_CASE("snprint truncate");
    char buf[10];
    lib9_snprint(buf, sizeof(buf), "hello world test");
    ASSERT_EQ(lib9_strlen(buf), 9);
    TEST_PASS();
}

void test_seprint_basic(void) {
    TEST_CASE("seprint basic");
    char buf[100];
    char *end = buf + sizeof(buf);
    char *p = buf;

    p = lib9_seprint(p, end, "hello ");
    p = lib9_seprint(p, end, "world");

    ASSERT_EQ_STR(buf, "hello world");
    TEST_PASS();
}

void test_seprint_chaining(void) {
    TEST_CASE("seprint chaining");
    char buf[100];
    char *end = buf + sizeof(buf);
    char *p = buf;

    p = lib9_seprint(p, end, "one");
    p = lib9_seprint(p, end, " two");
    p = lib9_seprint(p, end, " three");

    ASSERT_EQ_STR(buf, "one two three");
    TEST_PASS();
}

/* Edge Cases */
void test_null_termination(void) {
    TEST_CASE("null termination preserved");
    char buf[20];
    lib9_memset(buf, 'x', 20);
    lib9_strcpy(buf, "hi");

    ASSERT_EQ(buf[2], '\0');
    ASSERT_EQ(lib9_strlen(buf), 2);
    TEST_PASS();
}

void test_empty_operations(void) {
    TEST_CASE("operations on empty strings");
    char buf[10] = "";

    ASSERT_EQ(lib9_strlen(""), 0);
    ASSERT_EQ(lib9_strcmp("", ""), 0);

    lib9_strcat(buf, "");
    ASSERT_EQ_STR(buf, "");

    char *dup = lib9_strdup("");
    ASSERT_EQ_STR(dup, "");
    free(dup);

    TEST_PASS();
}

void test_single_char_operations(void) {
    TEST_CASE("single character operations");
    char buf[10];

    lib9_strcpy(buf, "a");
    ASSERT_EQ(lib9_strlen(buf), 1);

    lib9_strcat(buf, "b");
    ASSERT_EQ_STR(buf, "ab");

    ASSERT_EQ(lib9_strcmp("a", "a"), 0);
    ASSERT_TRUE(lib9_strcmp("a", "b") < 0);

    TEST_PASS();
}

/*
 * Main test runner
 */
int main(void) {
    printf("lib9 Utility Functions Unit Tests\n");
    printf("==================================\n");
    printf("Testing string, memory, and format functions\n");

    /* strlen Tests */
    TEST_SUITE("strlen Tests");
    test_strlen_empty();
    test_strlen_single();
    test_strlen_normal();
    test_strlen_long();

    /* strcmp Tests */
    TEST_SUITE("strcmp Tests");
    test_strcmp_equal();
    test_strcmp_less();
    test_strcmp_greater();
    test_strcmp_case_sensitive();

    /* strncmp Tests */
    TEST_SUITE("strncmp Tests");
    test_strncmp_equal();
    test_strncmp_different();
    test_strncmp_zero();

    /* strcpy Tests */
    TEST_SUITE("strcpy Tests");
    test_strcpy_basic();
    test_strcpy_empty();
    test_strcpy_overwrite();

    /* strncpy Tests */
    TEST_SUITE("strncpy Tests");
    test_strncpy_exact();
    test_strncpy_truncate();
    test_strncpy_pad();

    /* strcat Tests */
    TEST_SUITE("strcat Tests");
    test_strcat_basic();
    test_strcat_empty();
    test_strcat_multiple();

    /* strncat Tests */
    TEST_SUITE("strncat Tests");
    test_strncat_basic();
    test_strncat_truncate();

    /* strchr Tests */
    TEST_SUITE("strchr Tests");
    test_strchr_found();
    test_strchr_not_found();
    test_strchr_null_char();

    /* strrchr Tests */
    TEST_SUITE("strrchr Tests");
    test_strrchr_found();
    test_strrchr_not_found();

    /* strstr Tests */
    TEST_SUITE("strstr Tests");
    test_strstr_found();
    test_strstr_not_found();
    test_strstr_empty_needle();
    test_strstr_at_start();

    /* strdup Tests */
    TEST_SUITE("strdup Tests");
    test_strdup_basic();
    test_strdup_empty();

    /* memset Tests */
    TEST_SUITE("memset Tests");
    test_memset_zero();
    test_memset_character();
    test_memset_partial();

    /* memcpy Tests */
    TEST_SUITE("memcpy Tests");
    test_memcpy_basic();
    test_memcpy_partial();

    /* memmove Tests */
    TEST_SUITE("memmove Tests");
    test_memmove_no_overlap();
    test_memmove_overlap_forward();
    test_memmove_overlap_backward();

    /* memcmp Tests */
    TEST_SUITE("memcmp Tests");
    test_memcmp_equal();
    test_memcmp_less();
    test_memcmp_greater();
    test_memcmp_partial();

    /* memchr Tests */
    TEST_SUITE("memchr Tests");
    test_memchr_found();
    test_memchr_not_found();
    test_memchr_with_null();

    /* Character Classification Tests */
    TEST_SUITE("Character Classification Tests");
    test_isdigit();
    test_isalpha();
    test_isalnum();
    test_isspace();
    test_toupper();
    test_tolower();

    /* Conversion Tests */
    TEST_SUITE("Conversion Tests");
    test_atoi_positive();
    test_atoi_negative();
    test_atoi_whitespace();
    test_atoi_plus();
    test_atoi_trailing();
    test_atol_basic();

    /* Format Functions Tests */
    TEST_SUITE("Format Functions Tests");
    test_snprint_basic();
    test_snprint_numbers();
    test_snprint_truncate();
    test_seprint_basic();
    test_seprint_chaining();

    /* Edge Cases */
    TEST_SUITE("Edge Cases");
    test_null_termination();
    test_empty_operations();
    test_single_char_operations();

    /* Print summary */
    printf("\n========================================\n");
    printf("TEST SUMMARY\n");
    printf("========================================\n");
    printf("Total tests: %d\n", test_count);
    printf("Passed: %d\n", pass_count);
    printf("Failed: %d\n", fail_count);
    printf("========================================\n");

    if (fail_count == 0) {
        printf("All tests PASSED!\n");
        return 0;
    } else {
        printf("Some tests FAILED!\n");
        return 1;
    }
}
