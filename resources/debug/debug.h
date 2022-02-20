#ifndef DEBUG
#define DEBUG

#include "../engineCode/includes.h"

// gl debug dump
/* void GLDEBUGPROC MessageCallback( GLenum source, */
void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
                     GLsizei length, const GLchar *message, void *userParam) {
  bool show_high_severity = true;
  if (severity == GL_DEBUG_SEVERITY_HIGH && show_high_severity)
    fprintf(stderr,
            "        GL CALLBACK: %s type = 0x%x, severity = "
            "GL_DEBUG_SEVERITY_HIGH, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            message);

  bool show_medium_severity = true;
  if (severity == GL_DEBUG_SEVERITY_MEDIUM && show_medium_severity)
    fprintf(stderr,
            "        GL CALLBACK: %s type = 0x%x, severity = "
            "GL_DEBUG_SEVERITY_MEDIUM, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            message);

  bool show_low_severity = true;
  if (severity == GL_DEBUG_SEVERITY_LOW && show_low_severity)
    fprintf(stderr,
            "        GL CALLBACK: %s type = 0x%x, severity = "
            "GL_DEBUG_SEVERITY_LOW, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            message);

  bool show_notification_severity = false;
  if (severity == GL_DEBUG_SEVERITY_NOTIFICATION && show_notification_severity)
    fprintf(stderr,
            "        GL CALLBACK: %s type = 0x%x, severity = "
            "GL_DEBUG_SEVERITY_NOTIFICATION, message = %s\n",
            (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type,
            message);
}

void gl_debug_enable() {
  // DEBUG ENABLE
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(MessageCallback,
                         0); // getting a seg fault here, I think
  cout << "OpenGL debug output enabled." << endl << endl;

  // report all gl extensions - maybe useful on different platforms

  // GLint n;
  //
  // glGetIntegerv(GL_NUM_EXTENSIONS, &n);
  // cout << "starting dump of " << n << " extensions" << endl;
  //
  // for(int i=0; i<n; i++)
  //   cout << i << ": " << glGetStringi(GL_EXTENSIONS, i) << endl;
  //
  // cout << endl;

  // gl info re:texture size, texture units
  // querying the things that might cause issues with the drivers - so far this
  // has not given me any real idea of what's going on

  /* GLint val; */
  /* glGetIntegerv(GL_MAX_TEXTURE_SIZE, &val); */
  /* cout << "max texture size reports: " << val << endl << endl; */

  // glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &val);
  // cout << "max 3dtexture size reports: " << val << " on all 3 edges" << endl
  // << endl;
  //

  // if (glewIsSupported("GL_EXT_shader_image_load_store"))
  //     cout << "GL_EXT_shader_image_load_store is supported" << endl;
  // else
  //     cout << "GL_EXT_shader_image_load_store is not supported" << endl;

  // GLint textureCount;
  // glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureCount);
  // cout << endl << "GL_MAX_TEXTURE_IMAGE_UNITS returned:" << textureCount <<
  // endl << endl;

  // glGetIntegerv(GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS, &textureCount);
  // cout << endl << "GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS returned:" <<
  // textureCount << endl << endl;

  // glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textureCount);
  // cout << endl << "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS returned:" <<
  // textureCount << endl << endl;

  // glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &textureCount);
  // cout << endl << "GL_MAX_3D_TEXTURE_SIZE returned:" << textureCount << endl
  // << endl;

  /* GLint data[3]; */

  /* glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT,0, &data[0]); */
  /* glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT,1, &data[1]); */
  /* glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT,2, &data[2]); */
  /* cout << endl << "GL_MAX_COMPUTE_WORK_GROUP_COUNT returned x:" << data[0] <<
   * " y:" << data[1] << " z:" << data[2] << endl; */

  /* glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE,0, &data[0]); */
  /* glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE,1, &data[1]); */
  /* glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE,2, &data[2]); */
  /* cout << endl << "GL_MAX_COMPUTE_WORK_GROUP_SIZE returned x:" << data[0] <<
   * " y:" << data[1] << " z:" << data[2] << endl; */

  /* GLint max; */
  /* glGetIntegerv(  GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max); */
  /* cout << endl << "GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS returned: " << max
   * << endl << endl; */

  // glGetIntegerv(  GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &max);
  // cout << endl << "GL_MAX_COMPUTE_SHARED_MEMORY_SIZE returned: " << max << "
  // bytes" << endl << endl;
}

#endif
