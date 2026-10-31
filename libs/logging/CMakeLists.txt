add_library(logging STATIC
    src/logging.c
)

target_include_directories(logging PUBLIC
    include
)
