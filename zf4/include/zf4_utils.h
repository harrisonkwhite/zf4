#include <assert.h>

#define GL_CALL(X) X; assert(glGetError() == GL_NO_ERROR)
