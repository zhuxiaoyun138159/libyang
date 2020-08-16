/*
 * @file macros.h
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief macros for schema tests
 *
 * Copyright (c) 2018-2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */
#ifndef TESTS_UTESTS_SCHEMA_TEST_SCHEMA_H_
#define TESTS_UTESTS_SCHEMA_TEST_SCHEMA_H_

#include "log.h"
#include "set.h"
#include "parser_schema.h"
#include "tests/config.h"

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

void logbuf_clean(void);
extern struct ly_set logbuf;

#if ENABLE_LOGGER_CHECKING
#define logbuf_assert(STR) \
    assert_true(logbuf.count >= 1); \
    assert_string_equal(logbuf.objs[logbuf.count - 1], STR); \
    logbuf_clean()

#define logbuf_assert2(STR1, STR2) \
    assert_true(logbuf.count >= 2); \
    assert_string_equal(logbuf.objs[logbuf.count - 1], STR1); \
    assert_string_equal(logbuf.objs[logbuf.count - 2], STR2); \
    logbuf_clean()
#else
#define logbuf_assert(str)
#define logbuf_assert2(str1, str2)
#endif

LY_ERR test_imp_clb(const char *UNUSED(mod_name), const char *UNUSED(mod_rev), const char *UNUSED(submod_name),
                    const char *UNUSED(sub_rev), void *user_data, LYS_INFORMAT *format,
                    const char **module_data, void (**free_module_data)(void *model_data, void *user_data));


#define TEST_YANG_MODULE_10(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "module "MOD_NAME" { namespace "MOD_NS"; prefix "MOD_PREFIX"; "CONTENT"}"

#define TEST_YANG_MODULE_11(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "module "MOD_NAME" {yang-version 1.1; namespace "MOD_NS"; prefix "MOD_PREFIX"; "CONTENT"}"

#define TEST_YIN_MODULE_10(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\""MOD_NAME"\">" \
    "<namespace uri=\""MOD_NS"\"/><prefix value=\""MOD_PREFIX"\"/>"CONTENT"</module>"

#define TEST_YIN_MODULE_11(MOD_NAME, MOD_PREFIX, MOD_NS, CONTENT) \
    "<module xmlns=\"urn:ietf:params:xml:ns:yang:yin:1\" name=\""MOD_NAME"\"><yang-version value=\"1.1\"/>" \
    "<namespace uri=\""MOD_NS"\"/><prefix value=\""MOD_PREFIX"\"/>"CONTENT"</module>"

#define TEST_SCHEMA_STR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, STR) \
    if (YIN) { \
        if (RFC7950) { \
            STR = TEST_YIN_MODULE_11(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } else { \
            STR = TEST_YIN_MODULE_10(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } \
    } else { /* YANG */ \
        if (RFC7950) { \
            STR = TEST_YANG_MODULE_11(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } else { \
            STR = TEST_YANG_MODULE_10(MOD_NAME, MOD_NAME, "urn:libyang:test:"MOD_NAME, CONTENT); \
        } \
    }

#define TEST_SCHEMA_OK(CTX, RFC7950, YIN, MOD_NAME, CONTENT, RESULT) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_int_equal(LY_SUCCESS, lys_parse_mem(CTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG, &(RESULT))); \
    }

#define TEST_SCHEMA_ERR_(CTX, RFC7950, YIN, MOD_NAME, CONTENT) \
    { \
    const char *test_str__; \
    TEST_SCHEMA_STR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, test_str__) \
    assert_int_not_equal(lys_parse_mem(CTX, test_str__, YIN ? LYS_IN_YIN : LYS_IN_YANG, NULL), LY_SUCCESS); \
    }

#define TEST_SCHEMA_ERR(CTX, RFC7950, YIN, MOD_NAME, CONTENT, ERRMSG) \
    TEST_SCHEMA_ERR_(CTX, RFC7950, YIN, MOD_NAME, CONTENT); \
    logbuf_assert(ERRMSG);

#define TEST_SCHEMA_ERR2(CTX, RFC7950, YIN, MOD_NAME, CONTENT, ERRMSG1, ERRMSG2) \
    TEST_SCHEMA_ERR_(CTX, RFC7950, YIN, MOD_NAME, CONTENT); \
    logbuf_assert2(ERRMSG1, ERRMSG2);

#define TEST_STMT_DUP(CTX, RFC7950, YIN, STMT, MEMBER, VALUE1, VALUE2, LINE) \
    if (YIN) { \
        TEST_SCHEMA_ERR2(CTX, RFC7950, YIN, "dup", "", \
                        "Parsing module \"dup\" failed.", "Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    } else { \
        TEST_SCHEMA_ERR2(CTX, RFC7950, YIN, "dup", STMT"{"MEMBER" "VALUE1";"MEMBER" "VALUE2";}", \
                        "Parsing module \"dup\" failed.", \
                        "Duplicate keyword \""MEMBER"\". Line number "LINE"."); \
    }

#define TEST_STMT_SUBSTM_ERR(CTX, RFC7950, STMT, SUBSTMT, VALUE); \
        TEST_SCHEMA_ERR2(CTX, RFC7950, 0, "inv", STMT" test {"SUBSTMT" "VALUE";}", \
                        "Parsing module \"inv\" failed.", "Invalid keyword \""SUBSTMT"\" as a child of \""STMT"\". Line number 1.");

#endif /* TESTS_UTESTS_SCHEMA_TEST_SCHEMA_H_ */
