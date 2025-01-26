cmake_minimum_required(VERSION 3.25.1)
project(SPO_Lab1 C)
set(CMAKE_C_STANDARD 99)


add_executable(SPO_Lab1 main.c
        node.c
        node.h
        error.c
        error.h
        main.h
        parser.tab.c
        parser.tab.h
        lex.yy.c

        parser.c parser.h
        execution.c
        execution.h
        listing.c
        listing.h)
