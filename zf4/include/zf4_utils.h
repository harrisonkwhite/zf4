#pragma once

#include <cassert>

#define ZF4_GL_CALL(X) X; assert(glGetError() == GL_NO_ERROR)
