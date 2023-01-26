file(GLOB_RECURSE sources CONFIGURE_DEPENDS
    src/*.c
    src/*.h
    include/*.h
)

add_executable(image-transformer ${sources} src/utilities/source.h src/utilities/source.c src/utilities/transformation.h src/utilities/wedebug.h src/utilities/math_work.h)
target_include_directories(image-transformer PRIVATE src include)
