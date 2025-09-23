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

#define CUBE_FACE_BACK1   {0, 4, 6}
#define CUBE_FACE_BACK2   {0, 6, 2}
#define CUBE_FACE_FRONT1  {5, 1, 3}
#define CUBE_FACE_FRONT2  {5, 3, 7}
#define CUBE_FACE_LEFT1   {1, 0, 2}
#define CUBE_FACE_LEFT2   {1, 2, 3}
#define CUBE_FACE_RIGHT1  {4, 5, 7}
#define CUBE_FACE_RIGHT2  {4, 7, 6}
#define CUBE_FACE_TOP1    {2, 6, 7}
#define CUBE_FACE_TOP2    {2, 7, 3}
#define CUBE_FACE_BOTTOM1 {1, 5, 4}
#define CUBE_FACE_BOTTOM2 {4, 0, 1}

#define CUBE_FACE_BACK    CUBE_FACE_BACK1, CUBE_FACE_BACK2
#define CUBE_FACE_FRONT   CUBE_FACE_FRONT1, CUBE_FACE_FRONT2
#define CUBE_FACE_LEFT    CUBE_FACE_LEFT1, CUBE_FACE_LEFT2
#define CUBE_FACE_RIGHT   CUBE_FACE_RIGHT1, CUBE_FACE_RIGHT2
#define CUBE_FACE_TOP     CUBE_FACE_TOP1, CUBE_FACE_TOP2
#define CUBE_FACE_BOTTOM  CUBE_FACE_BOTTOM1, CUBE_FACE_BOTTOM2

#define CUBE_EDGE_01      {0, 1}
#define CUBE_EDGE_23      {2, 3}
#define CUBE_EDGE_45      {4, 5}
#define CUBE_EDGE_67      {6, 7}
#define CUBE_EDGE_02      {0, 2}
#define CUBE_EDGE_13      {1, 3}
#define CUBE_EDGE_46      {4, 6}
#define CUBE_EDGE_57      {5, 7}
#define CUBE_EDGE_04      {0, 4}
#define CUBE_EDGE_15      {1, 5}
#define CUBE_EDGE_26      {2, 6}
#define CUBE_EDGE_37      {3, 7}
#define CUBE_VERTICES     CUBE_VERT0, CUBE_VERT1, CUBE_VERT2, CUBE_VERT3, CUBE_VERT4, CUBE_VERT5, CUBE_VERT6, CUBE_VERT7
#define CUBE_FACES        CUBE_FACE_BACK, CUBE_FACE_FRONT, CUBE_FACE_LEFT, CUBE_FACE_RIGHT, CUBE_FACE_TOP, CUBE_FACE_BOTTOM

#define CUBE_EDGES \
    CUBE_EDGE_01, CUBE_EDGE_23, CUBE_EDGE_45, CUBE_EDGE_67,             \
    CUBE_EDGE_02, CUBE_EDGE_13, CUBE_EDGE_46, CUBE_EDGE_57,             \
    CUBE_EDGE_04, CUBE_EDGE_15, CUBE_EDGE_26, CUBE_EDGE_37

#define QUAD_VERT0        {-1.0f, -1.0f, 0.0f}
#define QUAD_VERT1        {-1.0f,  1.0f, 0.0f}
#define QUAD_VERT2        { 1.0f, -1.0f, 0.0f}
#define QUAD_VERT3        { 1.0f,  1.0f, 0.0f}

#define QUAD_FACE1        {0, 2, 3}
#define QUAD_FACE2        {0, 3, 1}

#define QUAD_VERTICES     QUAD_VERT0, QUAD_VERT1, QUAD_VERT2, QUAD_VERT3
#define QUAD_FACES        QUAD_FACE1, QUAD_FACE2

#define TRIANGLE_VERT0    {-1.0f, -1.0f, 0.0f}
#define TRIANGLE_VERT1    { 1.0f, -1.0f, 0.0f}
#define TRIANGLE_VERT2    { 0.0f,  1.0f, 0.0f}

#define TRIANGLE_FACE1    {0, 1, 2}

#define TRIANGLE_VERTICES TRIANGLE_VERT0, TRIANGLE_VERT1, TRIANGLE_VERT2
#define TRIANGLE_FACES    TRIANGLE_FACE1
