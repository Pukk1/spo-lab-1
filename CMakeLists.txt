cmake_minimum_required(VERSION 3.25.1)
project(SPO_Lab1 C)
set(CMAKE_C_STANDARD 99)


add_executable(SPO_Lab1 main.c
        node/node.c
        node/node.h
        error.c
        error.h
        main.h
        parser.tab.c
        parser.tab.h
        lex.yy.c

        parse/parser.c parse/parser.h
        execution/execution_graph.c
        execution/execution_graph.h
        listing/listing.c
        listing/listing.h
        common/list/list_util.c
        common/list/list_util.h
        execution/print/execution_print.c
        execution/print/execution_print.h
        execution/execution_types.h
        node/node_types.h
        parse/parse_types.h
        listing/listing_types.h
        common/node/tree_node_util.c
        common/node/tree_node_util.h
        common/string/string_util.c
        common/string/string_util.h
        common/exception/exception_util.c
        common/exception/exception_util.h
        common/execution/execution_util.c
        common/execution/execution_util.h
        listing/place_lables.c
        listing/place_lables.h
        common/file/file_work.c
        common/file/file_work.h
)
