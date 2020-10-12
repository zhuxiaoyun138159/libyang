/**
 * @file test_tree_schema.c
 * @author: Radek Krejci <rkrejci@cesnet.cz>
 * @brief unit tests for functions from tress_data.c
 *
 * Copyright (c) 2018-2019 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#include "../macros.h"

#include "libyang.h"
#include "common.h"
#include "path.h"
#include "xpath.h"

#define BUFSIZE 1024
char logbuf[BUFSIZE] = {0};
int store = -1; /* negative for infinite logging, positive for limited logging */

/* set to 0 to printing error messages to stderr instead of checking them in code */
#define ENABLE_LOGGER_CHECKING 1

#if ENABLE_LOGGER_CHECKING
static void
logger(LY_LOG_LEVEL level, const char *msg, const char *path)
{
    (void) level; /* unused */
    if (store) {
        if (path && path[0]) {
            snprintf(logbuf, BUFSIZE - 1, "%s %s", msg, path);
        } else {
            strncpy(logbuf, msg, BUFSIZE - 1);
        }
        if (store > 0) {
            --store;
        }
    }
}
#endif



const char *schema_a = "module a {namespace urn:tests:a;prefix a;yang-version 1.1;"
            "list l1 { key \"a b\"; leaf a {type string;} leaf b {type string;} leaf c {type string;}}"
            "leaf foo { type string;}"
            "leaf-list ll { type string;}"
            "container c {leaf-list x {type string;}}"
            "anydata any {config false;}"
            "list l2 {config false;"
                "container c{leaf x {type string;} leaf-list d {type string;}}"
            "}}";


#define CONTEXT_CREATE \
                CONTEXT_CREATE_PATH(NULL);\
                assert_int_equal(LY_SUCCESS, lys_parse_mem(CONTEXT_GET, schema_a, LYS_IN_YANG, NULL));\
                ly_set_log_clb(logger, 1)


#define MODEL_CREATE(INPUT, MODEL) \
                MODEL_CREATE_PARAM(INPUT, LYD_XML, 0, LYD_VALIDATE_PRESENT, LY_SUCCESS, "", MODEL)


#define MODEL_CHECK_CHAR(IN_MODEL, TEXT) \
                MODEL_CHECK_CHAR_PARAM(IN_MODEL, TEXT, LYD_XML, LYD_PRINT_SHRINK | LYD_PRINT_WITHSIBLINGS | LYD_PRINT_SHRINK)



void
logbuf_clean(void)
{
    logbuf[0] = '\0';
}

#if ENABLE_LOGGER_CHECKING
#   define logbuf_assert(str) assert_string_equal(logbuf, str)
#else
#   define logbuf_assert(str)
#endif

static void
test_compare(void **state)
{
    (void) state;

    struct lyd_node *tree1, *tree2;

    const char *data1 = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    const char *data2 = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>y</c></l1>";

    CONTEXT_CREATE;

    assert_int_equal(LY_SUCCESS, lyd_compare_single(NULL, NULL, 0));

    MODEL_CREATE(data1, tree1);
    MODEL_CREATE(data2, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    assert_int_equal(LY_ENOT, lyd_compare_single(((struct lyd_node_inner*)tree1)->child, tree2, 0));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data1 = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    data2 = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    MODEL_CREATE(data1, tree1);
    MODEL_CREATE(data2, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, tree2, 0));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data1 = "<ll xmlns=\"urn:tests:a\">a</ll><ll xmlns=\"urn:tests:a\">b</ll>";
    data2 = "<ll xmlns=\"urn:tests:a\">b</ll>";
    MODEL_CREATE(data1, tree1);
    MODEL_CREATE(data2, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(NULL, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, NULL, 0));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1->next, tree2, 0));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>x</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>y</x></c>";
    MODEL_CREATE(data1, tree1);
    MODEL_CREATE(data2, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data1 = "<c xmlns=\"urn:tests:a\"><x>x</x></c>";
    data2 = "<c xmlns=\"urn:tests:a\"><x>x</x><x>y</x></c>";
    MODEL_CREATE(data1, tree1);
    MODEL_CREATE(data2, tree2);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data1 = "<any xmlns=\"urn:tests:a\"><x>x</x></any>";
    data2 = "<any xmlns=\"urn:tests:a\"><x>x</x><x>y</x></any>";
    MODEL_CREATE(data1, tree1);
    MODEL_CREATE(data2, tree2);
    assert_int_equal(LY_ENOT, lyd_compare_single(tree1, tree2, 0));
    MODEL_DESTROY(tree1);
    data1 = "<any xmlns=\"urn:tests:a\"><x>x</x><x>y</x></any>";
    MODEL_CREATE(data1, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, 0));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    CONTEXT_DESTROY;
}

static void
test_dup(void **state)
{
    (void) state;

    struct lyd_node *tree1, *tree2;
    const char *result;
    const char *data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";

    CONTEXT_CREATE;
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, LYD_DUP_RECURSIVE, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>x</c></l1>";
    result = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b></l1>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, 0, &tree2));
    MODEL_DESTROY(tree1);
    MODEL_CREATE(result, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    result = "<l2 xmlns=\"urn:tests:a\"><c><x>a</x></c></l2>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_siblings(tree1, NULL, LYD_DUP_RECURSIVE, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree2);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, LYD_DUP_RECURSIVE, &tree2));
    MODEL_DESTROY(tree1);
    MODEL_CREATE(result, tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree2);

    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, 0, &tree2));
    MODEL_DESTROY(tree1);
    result = "<l2 xmlns=\"urn:tests:a\"/>";
    MODEL_CREATE_PARAM(result, LYD_XML, LYD_PARSE_ONLY, 0, LY_SUCCESS, "", tree1);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data = "<any xmlns=\"urn:tests:a\"><c><a>a</a></c></any>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, 0, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(((struct lyd_node_inner*)((struct lyd_node_inner*)tree1)->child)->child, NULL,
                                         LYD_DUP_WITH_PARENTS, &tree2));
    LYSC_NODE_CHECK(tree2->schema, LYS_LEAF, "x");
    assert_non_null(tree2->parent);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, (struct lyd_node*)tree2->parent->parent, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>c</c></l1>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(((struct lyd_node_inner*)tree1)->child->prev, NULL,
                                                LYD_DUP_WITH_PARENTS, &tree2));
    LYSC_NODE_CHECK(tree2->schema, LYS_LEAF, "c");
    assert_non_null(tree2->parent);
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, (struct lyd_node*)tree2->parent, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    data = "<l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_SUCCESS, lyd_dup_single(tree1, NULL, 0, &tree2));
    assert_int_equal(LY_SUCCESS, lyd_dup_single(((struct lyd_node_inner*)((struct lyd_node_inner*)tree1)->child)->child,
                                 (struct lyd_node_inner*)tree2, LYD_DUP_WITH_PARENTS, NULL));
    assert_int_equal(LY_SUCCESS, lyd_compare_single(tree1, tree2, LYD_COMPARE_FULL_RECURSION));
    MODEL_DESTROY(tree1);
    MODEL_DESTROY(tree2);

    /* invalid */
    data = "<l1 xmlns=\"urn:tests:a\"><a>a</a><b>b</b><c>c</c></l1><l2 xmlns=\"urn:tests:a\"><c><x>b</x></c></l2>";
    MODEL_CREATE(data, tree1);
    assert_int_equal(LY_EINVAL, lyd_dup_single(((struct lyd_node_inner*)tree1)->child->prev,
                                               (struct lyd_node_inner*)tree1->next, LYD_DUP_WITH_PARENTS, NULL));
    MODEL_DESTROY(tree1);

    CONTEXT_DESTROY;
}

static void
test_target(void **state)
{
    (void) state;
    *state = test_target;

    const struct lyd_node_term *term;
    struct lyd_node *tree;
    struct lyxp_expr *exp;
    struct ly_path *path;
    const char *path_str = "/a:l2[2]/c/d[3]";
    const char *data =
        "<l2 xmlns=\"urn:tests:a\"><c>"
            "<d>a</d>"
        "</c></l2>"
        "<l2 xmlns=\"urn:tests:a\"><c>"
            "<d>a</d>"
            "<d>b</d>"
            "<d>b</d>"
            "<d>c</d>"
        "</c></l2>"
        "<l2 xmlns=\"urn:tests:a\"><c>"
        "</c></l2>";

    CONTEXT_CREATE;

    MODEL_CREATE(data, tree);
    assert_int_equal(LY_SUCCESS, ly_path_parse(CONTEXT_GET, NULL, path_str, strlen(path_str), LY_PATH_BEGIN_EITHER, LY_PATH_LREF_FALSE,
                                               LY_PATH_PREFIX_OPTIONAL, LY_PATH_PRED_SIMPLE, &exp));
    assert_int_equal(LY_SUCCESS, ly_path_compile(CONTEXT_GET, NULL, NULL, exp, LY_PATH_LREF_FALSE, LY_PATH_OPER_INPUT,
                                                 LY_PATH_TARGET_SINGLE, LY_PREF_JSON, NULL, &path));
    term = lyd_target(path, tree);

    LYSC_NODE_CHECK(term->schema, LYS_LEAFLIST, "d");
    assert_string_equal(LYD_CANON_VALUE(term), "b");
    assert_string_equal(LYD_CANON_VALUE(term->prev), "b");

    MODEL_DESTROY(tree);
    ly_path_free(CONTEXT_GET, path);
    lyxp_expr_free(CONTEXT_GET, exp);

    CONTEXT_DESTROY;
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_compare),
        cmocka_unit_test(test_dup),
        cmocka_unit_test(test_target),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
