#pragma once

#define COLOR_RED         1.0f, 0.0f, 0.0f, 1.0f
#define COLOR_GREEN       0.0f, 1.0f, 0.0f, 1.0f
#define COLOR_BLUE        0.0f, 0.0f, 1.0f, 1.0f
#define COLOR_YELLOW      1.0f, 1.0f, 0.0f, 1.0f
#define COLOR_MAGENTA     1.0f, 0.0f, 1.0f, 1.0f
#define COLOR_CYAN        0.0f, 1.0f, 1.0f, 1.0f
#define COLOR_WHITE       1.0f, 1.0f, 1.0f, 1.0f
#define COLOR_BLACK       0.0f, 0.0f, 0.0f, 1.0f

#define PALETTE_RED       0.7f, 0.25f, 0.25f, 1.0f
#define PALETTE_GREEN     0.25f, 0.6f, 0.35f, 1.0f
#define PALETTE_BLUE      0.25f, 0.45f, 0.7f, 1.0f
#define PALETTE_ORANGE    0.8f, 0.5f, 0.25f, 1.0f
#define PALETTE_TEAL      0.25f, 0.6f, 0.6f, 1.0f
#define PALETTE_PURPLE    0.55f, 0.35f, 0.6f, 1.0f
#define PALETTE_PINK      0.85f, 0.5f, 0.65f, 1.0f
#define PALETTE_YELLOW    0.8f, 0.75f, 0.25f, 1.0f
#define PALETTE_BROWN     0.55f, 0.4f, 0.25f, 1.0f
#define PALETTE_DARKGRAY  0.15f, 0.15f, 0.15f, 1.0f
#define PALETTE_MIDGRAY   0.4f, 0.4f, 0.4f, 1.0f
#define PALETTE_LIGHTGRAY 0.6f, 0.6f, 0.6f, 1.0f
#define PALETTE_NAVY      0.2f, 0.25f, 0.5f, 1.0f
#define PALETTE_OLIVE     0.4f, 0.45f, 0.2f, 1.0f
#define PALETTE_MAROON    0.5f, 0.2f, 0.2f, 1.0f
#define PALETTE_CYAN      0.2f, 0.55f, 0.55f, 1.0f
#define PALETTE_LIME      0.4f, 0.7f, 0.2f, 1.0f
#define PALETTE_INDIGO    0.35f, 0.3f, 0.6f, 1.0f
#define PALETTE_CORAL     0.85f, 0.45f, 0.4f, 1.0f
#define PALETTE_SALMON    0.9f, 0.55f, 0.5f, 1.0f

#define AXIS_X            {1.0f, 0.0f, 0.0f}
#define AXIS_NEG_X        {-1.0f, 0.0f, 0.0f}
#define AXIS_Y            {0.0f, 1.0f, 0.0f}
#define AXIS_NEG_Y        {0.0f, -1.0f, 0.0f}
#define AXIS_Z            {0.0f, 0.0f, 1.0f}
#define AXIS_NEG_Z        {0.0f, 0.0f, -1.0f}

#define CUBE_VERT0        {-1.0f, -1.0f, -1.0f}
#define CUBE_VERT1        {-1.0f, -1.0f,  1.0f}
#define CUBE_VERT2        {-1.0f,  1.0f, -1.0f}
#define CUBE_VERT3        {-1.0f,  1.0f,  1.0f}
#define CUBE_VERT4        { 1.0f, -1.0f, -1.0f}
#define CUBE_VERT5        { 1.0f, -1.0f,  1.0f}
#define CUBE_VERT6        { 1.0f,  1.0f, -1.0f}
#define CUBE_VERT7        { 1.0f,  1.0f,  1.0f}

#define CUBE_FACE_BACK    {0, 4, 6}, {0, 6, 2}
#define CUBE_FACE_FRONT   {5, 1, 3}, {5, 3, 7}
#define CUBE_FACE_LEFT    {1, 0, 2}, {1, 2, 3}
#define CUBE_FACE_RIGHT   {4, 5, 7}, {4, 7, 6}
#define CUBE_FACE_TOP     {2, 6, 7}, {2, 7, 3}
#define CUBE_FACE_BOTTOM  {1, 5, 4}, {4, 0, 1}

#define CUBE_VERTICES     CUBE_VERT0, CUBE_VERT1, CUBE_VERT2, CUBE_VERT3, CUBE_VERT4, CUBE_VERT5, CUBE_VERT6, CUBE_VERT7
#define CUBE_FACES        CUBE_FACE_BACK, CUBE_FACE_FRONT, CUBE_FACE_LEFT, CUBE_FACE_RIGHT, CUBE_FACE_TOP, CUBE_FACE_BOTTOM

#define CUBE_APPLY_TO_VERTICES(V) V(CUBE_VERT0) V(CUBE_VERT1) V(CUBE_VERT2) V(CUBE_VERT3) V(CUBE_VERT4) V(CUBE_VERT5) V(CUBE_VERT6) V(CUBE_VERT7)
#define CUBE_APPLY_TO_FACES(HEAD, TAIL) \
    HEAD(0, 4, 6) TAIL(0, 6, 2) \
    TAIL(5, 1, 3) TAIL(5, 3, 7) \
    TAIL(1, 0, 2) TAIL(1, 2, 3) \
    TAIL(4, 5, 7) TAIL(4, 7, 6) \
    TAIL(2, 6, 7) TAIL(2, 7, 3) \
    TAIL(1, 5, 4) TAIL(4, 0, 1)

#define QUAD_VERT0        {-1.0f, -1.0f, 0.0f}
#define QUAD_VERT1        {-1.0f,  1.0f, 0.0f}
#define QUAD_VERT2        { 1.0f, -1.0f, 0.0f}
#define QUAD_VERT3        { 1.0f,  1.0f, 0.0f}

#define QUAD_FACE1        {0, 2, 3}
#define QUAD_FACE2        {0, 3, 1}

#define QUAD_VERTICES     QUAD_VERT0, QUAD_VERT1, QUAD_VERT2, QUAD_VERT3
#define QUAD_FACES        QUAD_FACE1, QUAD_FACE2

#define QUAD_APPLY_TO_VERTICES(V) V(QUAD_VERT0) V(QUAD_VERT1) V(QUAD_VERT2) V(QUAD_VERT3)
#define QUAD_APPLY_TO_FACES(F) F(QUAD_FACE1) F(QUAD_FACE2)

#define TRIANGLE_VERT0    {-1.0f, -1.0f, 0.0f}
#define TRIANGLE_VERT1    { 1.0f, -1.0f, 0.0f}
#define TRIANGLE_VERT2    { 0.0f,  1.0f, 0.0f}

#define TRIANGLE_FACE1    {0, 1, 2}

#define TRIANGLE_VERTICES TRIANGLE_VERT0, TRIANGLE_VERT1, TRIANGLE_VERT2
#define TRIANGLE_FACES    TRIANGLE_FACE1

#define TRIANGLE_APPLY_TO_VERTICES(V) V(TRIANGLE_VERT0) V(TRIANGLE_VERT1) V(TRIANGLE_VERT2)
#define TRIANGLE_APPLY_TO_FACES(F) F(TRIANGLE_FACE1)

#define LIST_HEAD(...) {__VA_ARGS__}
#define LIST_TAIL(...) ,{__VA_ARGS__}
#define LIST_ITEM(V) {V},
