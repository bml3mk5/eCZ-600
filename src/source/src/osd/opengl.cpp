/** @file opengl.cpp

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.03.08

	@brief [ OpenGL ]
*/


#include "../common.h"

#if defined(USE_OPENGL)

#include "opengl.h"
#include "../csurface.h"
#include "../logging.h"

// ==============================================================================

COpenGLTexture::COpenGLTexture()
{
	m_opengl = NULL;
	m_id = 0;
	m_seq = 0;
	m_format = GL_RGBA;
}
COpenGLTexture::COpenGLTexture(int seq)
{
	m_opengl = NULL;
	m_id = 0;
	m_seq = seq;
	m_format = GL_RGBA;
}
COpenGLTexture::~COpenGLTexture()
{
	Release();
}
/// @brief Create a new instance
/// @param[in] opengl : Parent
/// @param[in] seq : Unique id
/// @param[in] force : 0 or OpenGL version
/// @return new instance or NULL
COpenGLTexture *COpenGLTexture::New(COpenGL *opengl, int seq, int force)
{
	if (!opengl) return NULL;

	COpenGLTexture *new_obj = NULL;

	const GLubyte *glsl_version  = NULL;

//	const GLubyte *vender   = glGetString(GL_VENDOR);
//	const GLubyte *renderer = glGetString(GL_RENDERER);
//	const GLubyte *version  = glGetString(GL_VERSION);

//	logging->out_logf(LOG_DEBUG, "OPENGL: VENDER:%s RENDERER:%s VERSION:%s", vender, renderer, version);

#ifdef GL_SHADING_LANGUAGE_VERSION
	glsl_version  = glGetString(GL_SHADING_LANGUAGE_VERSION);
#endif
#ifdef USE_OPENGL33
	if (opengl->Version() == 3 || force == 3) {
		logging->out_logf(LOG_DEBUG, "GLSL:%s  Use COpenGL3", (const char *)glsl_version);
		new_obj = new COpenGL3Texture(seq);
		new_obj->SetParent(opengl);
		return new_obj;
	}
#endif
#ifdef USE_OPENGL21
	if (opengl->Version() == 2 || force == 2) {
		logging->out_logf(LOG_DEBUG, "GLSL:%s  Use COpenGL2", (const char *)glsl_version);
		new_obj = new COpenGL2Texture(seq);
		new_obj->SetParent(opengl);
		return new_obj;
	}
#endif
	logging->out_logf(LOG_DEBUG, "GLSL:%s  Use COpenGL1", glsl_version != NULL ? (const char *)glsl_version : "(null)");
	new_obj = new COpenGL1Texture(seq);
	new_obj->SetParent(opengl);
	return new_obj;
}
void COpenGLTexture::SetParent(COpenGL *opengl)
{
	m_opengl = opengl;
}
GLuint COpenGLTexture::Release()
{
	return m_id;
}
void COpenGLTexture::SetFilter(int filter)
{
}
void COpenGLTexture::ReleaseBuffer()
{
}
void COpenGLTexture::PresetRGBA()
{
	m_format = GL_RGBA;
}
void COpenGLTexture::PresetBGRA()
{
#ifdef _WIN32
	m_format = GL_BGRA_EXT;
#else
	m_format = GL_BGRA;
#endif
}

// ------------------------------------------------------------------------------

COpenGL::COpenGL()
{
	m_disp_w = 1;
	m_disp_h = 1;
}
COpenGL::~COpenGL()
{
}
/// @brief Create a new instance
/// @param[in] force : 0 or OpenGL version
COpenGL *COpenGL::New(int force)
{
	const GLubyte *glsl_version  = NULL;

	const GLubyte *vender   = glGetString(GL_VENDOR);
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *version  = glGetString(GL_VERSION);

	logging->out_logf(LOG_DEBUG, "OPENGL: VENDER:%s RENDERER:%s VERSION:%s", vender, renderer, version);

#ifdef GL_SHADING_LANGUAGE_VERSION
	glsl_version  = glGetString(GL_SHADING_LANGUAGE_VERSION);
#endif
#ifdef USE_OPENGL33
	if (glsl_version && ((force == 0 && glsl_version[0] >= '3' && glsl_version[0] <= '9') || force == 3)) {
		logging->out_logf(LOG_DEBUG, "GLSL:%s  Use COpenGL3", (const char *)glsl_version);
		return new COpenGL3();
	}
#endif
#ifdef USE_OPENGL21
	if (glsl_version && ((force == 0 && glsl_version[0] >= '1') || force == 2)) {
		logging->out_logf(LOG_DEBUG, "GLSL:%s  Use COpenGL2", (const char *)glsl_version);
		return new COpenGL2();
	}
#endif
	logging->out_logf(LOG_DEBUG, "GLSL:%s  Use COpenGL1", glsl_version != NULL ? (const char *)glsl_version : "(null)");
	return new COpenGL1();
}

int COpenGL::Version() const
{
	return 0;
}

bool COpenGL::Initialize()
{
	return false;
}
void COpenGL::Terminate()
{
}
bool COpenGL::InitViewport(int w, int h)
{
	m_disp_w = w;
	m_disp_h = h;

	return true;
}
/// @brief Set swap interval
/// @param[in] use : 1: use interval / otherwise: noninterval
/// @param[in] user_data : pointer of Display on X11
/// @return result of setting interval 1:use interval 2:noninterval
unsigned char COpenGL::SetInterval(unsigned char use, void *user_data)
{
	/// Sync VSYNC set
#if defined(_WIN32)
	// OpenGL for Windows (wgl)
	GLenum err = 0;

	const char *ext = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
	if (ext && strstr(ext, "WGL_EXT_swap_control")) {
		void (WINAPI *wglSwapIntervalEXT)(int);
		int (WINAPI *wglGetSwapIntervalEXT)();

		wglSwapIntervalEXT = reinterpret_cast<void (WINAPI *)(int)>(wglGetProcAddress("wglSwapIntervalEXT"));
		if (wglSwapIntervalEXT) {
			wglSwapIntervalEXT(use == 1 ? 1 : 0);
			while ((err = glGetError()) != GL_NO_ERROR) {
				logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::wglSwapIntervalEXT err:0x%x"), err);
			}
		}
		wglGetSwapIntervalEXT = reinterpret_cast<int (WINAPI *)()>(wglGetProcAddress("wglGetSwapIntervalEXT"));
		if (wglGetSwapIntervalEXT) {
			int interval = wglGetSwapIntervalEXT();
			use = (interval == 1 ? 1 : 2);
			while ((err = glGetError()) != GL_NO_ERROR) {
				logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::wglGetSwapIntervalEXT err:0x%x"), err);
			}
			logging->out_debugf(_T("COpenGL::SetInterval::wglGetSwapIntervalEXT: %d"), interval);
		}
	}
#elif defined(USE_OPENGL_GLX)
	// OpenGL on X11 (glx)
	GLenum err = 0;

	Display *display = reinterpret_cast<Display *>(user_data);
	const char *ext = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
	if ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glGetString err:0x%x"), err);
		return use;
	}
	if (!ext) {
		ext = glXGetClientString(display, GLX_EXTENSIONS);
		while ((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXGetClientString err:0x%x"), err);
		}
	}
	if (ext) {
		if (strstr(ext, "GLX_MESA_swap_control")) {
			int (*glXSwapIntervalMESA)(unsigned int);
			int (*glXGetSwapIntervalMESA)();
			glXSwapIntervalMESA = reinterpret_cast<int (*)(unsigned int)>(
				glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalMESA"))
			);
			if (glXSwapIntervalMESA) {
				glXSwapIntervalMESA(use == 1 ? 1 : 0);
				while ((err = glGetError()) != GL_NO_ERROR) {
					logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXSwapIntervalMESA err:0x%x"), err);
				}
			}
			glXGetSwapIntervalMESA = reinterpret_cast<int (*)()>(
				glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXGetSwapIntervalMESA"))
			);
			if (glXGetSwapIntervalMESA) {
				int interval = glXGetSwapIntervalMESA();
				use = (interval == 1 ? 1 : 2);
				while ((err = glGetError()) != GL_NO_ERROR) {
					logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXGetSwapIntervalMESA err:0x%x"), err);
				}
				logging->out_debugf(_T("COpenGL::SetInterval::glXGetSwapIntervalMESA: %d"), interval);
			}
		}
		else if (strstr(ext, "GLX_EXT_swap_control")) {
			void (*glXSwapIntervalEXT)(Display*, GLXDrawable, int);
			int (*glXGetSwapIntervalEXT)();
			glXSwapIntervalEXT = reinterpret_cast<void (*)(Display*, GLXDrawable, int)>(
				glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalEXT"))
			);
			if (glXSwapIntervalEXT) {
				glXSwapIntervalEXT(display, 0, use == 1 ? 1 : 0);
				while ((err = glGetError()) != GL_NO_ERROR) {
					logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXSwapIntervalEXT err:0x%x"), err);
				}
			}
			glXGetSwapIntervalEXT = reinterpret_cast<int (*)()>(
				glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXGetSwapIntervalEXT"))
			);
			if (glXGetSwapIntervalEXT) {
				int interval = glXGetSwapIntervalEXT();
				use = (interval == 1 ? 1 : 2);
				while ((err = glGetError()) != GL_NO_ERROR) {
					logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXGetSwapIntervalEXT err:0x%x"), err);
				}
				logging->out_debugf(_T("COpenGL::SetInterval::glXGetSwapIntervalEXT: %d"), interval);
			}
		}
		else if (strstr(ext, "GLX_SGI_swap_control")) {
			void (*glXSwapIntervalSGI)(int);
			int (*glXGetSwapIntervalSGI)();
			glXSwapIntervalSGI = reinterpret_cast<void (*)(int)>(
				glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXSwapIntervalSGI"))
			);
			if (glXSwapIntervalSGI) {
				glXSwapIntervalSGI(use == 1 ? 1 : 0);
				while ((err = glGetError()) != GL_NO_ERROR) {
					logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXSwapIntervalSGI err:0x%x"), err);
				}
			}
			glXGetSwapIntervalSGI = reinterpret_cast<int (*)()>(
				glXGetProcAddress(reinterpret_cast<const GLubyte *>("glXGetSwapIntervalSGI"))
			);
			if (glXGetSwapIntervalSGI) {
				int interval = glXGetSwapIntervalSGI();
				use = (interval == 1 ? 1 : 2);
				while ((err = glGetError()) != GL_NO_ERROR) {
					logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::glXGetSwapIntervalSGI err:0x%x"), err);
				}
				logging->out_debugf(_T("COpenGL::SetInterval::glXGetSwapIntervalSGI: %d"), interval);
			}
		}
	}
#elif defined(USE_OPENGL_CGL)
	// OpenGL for MacOSX (cgl)
	CGLError err;
	int interval = (use == 1 ? 1 : 0);
	err = CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);
	if (err != kCGLNoError) {
		logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::CGLSetParameter err:%d"), err);
	}
	err = CGLGetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &interval);
	if (err != kCGLNoError) {
		logging->out_logf(LOG_ERROR, _T("COpenGL::SetInterval::CGLGetParameter err:%d"), err);
	}
	use = (interval == 1 ? 1 : 2);
	logging->out_debugf(_T("COpenGL::SetInterval::kCGLCPSwapInterval: %d"), interval);
#endif

	return use;
}
void COpenGL::ClearScreen()
{
}

// ==============================================================================

COpenGL1Texture::COpenGL1Texture() : COpenGLTexture()
{
	m_tex_l = m_tex_t = 0.0f;
	m_tex_r = m_tex_b = 1.0f;
	m_pyl_l = m_pyl_t = -1.0f;
	m_pyl_r = m_pyl_b =  1.0f;
}
COpenGL1Texture::COpenGL1Texture(int seq) : COpenGLTexture(seq)
{
	m_tex_l = m_tex_t = 0.0f;
	m_tex_r = m_tex_b = 1.0f;
	m_pyl_l = m_pyl_t = -1.0f;
	m_pyl_r = m_pyl_b =  1.0f;
}
COpenGL1Texture::~COpenGL1Texture()
{
}
/// @brief Create a texture
/// @param[in] filter  GL_NEAREST/GL_LINEAR
/// @return texture id / 0 if failed to create.
GLuint COpenGL1Texture::Create(int filter)
{
	GLenum err;

#if defined(_RGB555) || defined(_RGB565)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
#elif defined(_RGB888)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#else
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

	glGenTextures(1, &m_id);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL1Texture::Create::glGenTextures: 0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL1Texture::Create::textureId: %d"), m_id);

	SetFilter(filter);

	// fill black
	glBegin(GL_QUADS);
	glColor3ub(0, 0, 0);
#ifdef USE_OPENGL_WH_ORTHO
	GLint w = m_opengl->Width();
	GLint h = m_opengl->Height();
	glVertex3i(0, 0, 0);
	glVertex3i(0, h, 0);
	glVertex3i(w, h, 0);
	glVertex3i(w, 0, 0);
#else
	glVertex3i(-1, -1, 0);
	glVertex3i(-1,  1, 0);
	glVertex3i( 1,  1, 0);
	glVertex3i( 1, -1, 0);
#endif
	glEnd();

	return m_id;
}
/// @brief Release a texture
/// @return 0
GLuint COpenGL1Texture::Release()
{
	if (m_id) {
		// release texture
		glDeleteTextures(1, &m_id);
		logging->out_debug(_T("COpenGL1Texture::Release"));
		m_id = 0;
	}
	return m_id;
}
/// @brief Set a filter type
/// @param[in] filter  GL_NEAREST/GL_LINEAR
void COpenGL1Texture::SetFilter(int filter)
{
	GLenum err;

	if (!m_id) return;

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST + filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST + filter);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL1Texture::SetFilter::glTexParameteri FILTER 0x%x"), err);
	}
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_CLAMP);	// no wrap texture
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_CLAMP);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL1Texture::SetFilter::glTexParameteri WRAP 0x%x"), err);
	}
	glBindTexture(GL_TEXTURE_2D, 0);	// unbound
}
/// @brief Set texture position
/// @param[in] pyl_left    left side of polygon
/// @param[in] pyl_top     top side of polygon
/// @param[in] pyl_right   right side of polygon
/// @param[in] pyl_bottom  bottom side of polygon
/// @param[in] tex_left    left side of texture
/// @param[in] tex_top     top side of texture
/// @param[in] tex_right   right side of texture
/// @param[in] tex_bottom  bottom side of texture
bool COpenGL1Texture::CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom)
{
	m_pyl_l = pyl_left;
	m_pyl_r = pyl_right;
	m_pyl_t = pyl_top;
	m_pyl_b = pyl_bottom;
	m_tex_l = tex_left;
	m_tex_t = tex_top;
	m_tex_r = tex_right;
	m_tex_b = tex_bottom;
	return true;
}
/// @brief Set texture position
/// @param[in] pyl_left    left side of polygon
/// @param[in] pyl_top     top side of polygon
/// @param[in] pyl_right   right side of polygon
/// @param[in] pyl_bottom  bottom side of polygon
/// @param[in] tex_left    left side of texture
/// @param[in] tex_top     top side of texture
/// @param[in] tex_right   right side of texture
/// @param[in] tex_bottom  bottom side of texture
bool COpenGL1Texture::SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom)
{
	m_pyl_l = pyl_left;
	m_pyl_r = pyl_right;
	m_pyl_t = pyl_top;
	m_pyl_b = pyl_bottom;
	m_tex_l = tex_left;
	m_tex_t = tex_top;
	m_tex_r = tex_right;
	m_tex_b = tex_bottom;
	return true;
}
/// @brief Draw the texture on the window
/// @param[in] width
/// @param[in] height
/// @param[in] buffer
void COpenGL1Texture::Draw(int width, int height, void *buffer)
{
	GLenum err;

	glClear(GL_COLOR_BUFFER_BIT);

	// draw texture using screen pixel buffer
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, m_format,
			GL_UNSIGNED_BYTE, static_cast<GLvoid *>(buffer));
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL1Texture::Draw::glTexImage2D: 0x%x"), err);
	}

	// fill black
	glBegin(GL_QUADS);
		glColor3ub(0, 0, 0);
#ifdef USE_OPENGL_WH_ORTHO
		GLint w = m_opengl->Width();
		GLint h = m_opengl->Height();
		glVertex3i(0, 0, 0);
		glVertex3i(0, h, 0);
		glVertex3i(w, h, 0);
		glVertex3i(w, 0, 0);
#else
		glVertex3i(-1, -1, 0);
		glVertex3i(-1,  1, 0);
		glVertex3i( 1,  1, 0);
		glVertex3i( 1, -1, 0);
#endif
	glEnd();

	glEnable(GL_TEXTURE_2D);
	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);
		// anticlockwise
		glTexCoord2f(m_tex_l, m_tex_t);
		glVertex3f(m_pyl_l, m_pyl_t, 0.0f);
		glTexCoord2f(m_tex_l, m_tex_b);
		glVertex3f(m_pyl_l, m_pyl_b, 0.0f);
		glTexCoord2f(m_tex_r, m_tex_b);
		glVertex3f(m_pyl_r, m_pyl_b, 0.0f);
		glTexCoord2f(m_tex_r, m_tex_t);
		glVertex3f(m_pyl_r, m_pyl_t, 0.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}
/// @brief Render the texture on the window
/// @param[in] width
/// @param[in] height
/// @param[in] buffer
void COpenGL1Texture::Render(int width, int height, void *buffer)
{
	GLenum err;

	// draw texture using screen pixel buffer
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, m_format,
			GL_UNSIGNED_BYTE, static_cast<GLvoid *>(buffer));
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL1Texture::Render::glTexImage2D: 0x%x"), err);
	}

	glEnable(GL_TEXTURE_2D);
	glColor3ub(255, 255, 255);
	glBegin(GL_QUADS);
		// anticlockwise
		glTexCoord2f(m_tex_l, m_tex_t);
		glVertex3f(m_pyl_l, m_pyl_t, 0.0f);
		glTexCoord2f(m_tex_l, m_tex_b);
		glVertex3f(m_pyl_l, m_pyl_b, 0.0f);
		glTexCoord2f(m_tex_r, m_tex_b);
		glVertex3f(m_pyl_r, m_pyl_b, 0.0f);
		glTexCoord2f(m_tex_r, m_tex_t);
		glVertex3f(m_pyl_r, m_pyl_t, 0.0f);
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
}

// ------------------------------------------------------------------------------

COpenGL1::COpenGL1()
{
}

COpenGL1::~COpenGL1()
{
}

int COpenGL1::Version() const
{
	return 1;
}
/// @brief Initialize the instance
bool COpenGL1::Initialize()
{
	return COpenGL::Initialize();
}

void COpenGL1::Terminate()
{
}

bool COpenGL1::InitViewport(int w, int h)
{
	COpenGL::InitViewport(w, h);

	// shading
	glShadeModel( GL_SMOOTH );
	// culling
	glFrontFace( GL_CCW );
	glEnable( GL_CULL_FACE );
	glCullFace( GL_BACK );
	// bg color
	glClearColor( 0.0, 0.0, 0.0, 1.0 );

	// Setup our viewport : same as window size
	glViewport( 0, 0, w, h );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	// change axis unit
#ifdef USE_OPENGL_WH_ORTHO
	glOrtho(0.0, (GLdouble)w, (GLdouble)h, 0.0, -1.0, 1.0);
#else
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
#endif

	return true;
}
void COpenGL1::ClearScreen()
{
	// fill black
	glBegin(GL_QUADS);
	glColor3ub(0, 0, 0);
#ifdef USE_OPENGL_WH_ORTHO
	glVertex3i(0, 0, 0);
	glVertex3i(0, m_disp_h, 0);
	glVertex3i(m_disp_w, m_disp_h, 0);
	glVertex3i(m_disp_w, 0, 0);
#else
	glVertex3i(-1, -1, 0);
	glVertex3i(-1,  1, 0);
	glVertex3i( 1,  1, 0);
	glVertex3i( 1, -1, 0);
#endif
	glEnd();
}

// ==============================================================================

#ifdef USE_OPENGL21

COpenGL2Texture::COpenGL2Texture() : COpenGLTexture()
{
	m_loc = -1;
	mPosBufferId = 0;
	mPositionLoc = -1;
	mTexPosBufferId = 0;
	mTexPositionLoc = -1;
	mVertexId = 0;
	mFragmentId = 0;
	mProgramId = 0;
}
/// @brief Provide a interface for a texture on OpenGL 2.1
COpenGL2Texture::COpenGL2Texture(int seq) : COpenGLTexture(seq)
{
	m_loc = -1;
	mPosBufferId = 0;
	mPositionLoc = -1;
	mTexPosBufferId = 0;
	mTexPositionLoc = -1;
	mVertexId = 0;
	mFragmentId = 0;
	mProgramId = 0;
}
COpenGL2Texture::~COpenGL2Texture()
{
	ReleaseBuffer();
	ReleaseProgram();
}
/// @brief Create a texture
/// @param[in] filter  GL_NEAREST/GL_LINEAR
/// @return texture id / 0 if failed to create.
GLuint COpenGL2Texture::Create(int filter)
{
	GLenum err;

	// build program
	if (!BuildProgram()) {
		return 0;
	}

	// create
#if defined(_RGB555) || defined(_RGB565)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
#elif defined(_RGB888)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#else
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

	glGenTextures(1, &m_id);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Create::glGenTextures: 0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL2Texture::Create::textureId: %d"), m_id);

	SetFilter(filter);

	m_loc = glGetUniformLocation(mProgramId, "myTextureSample");
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Create::glGetUniformLocation: err:0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL2Texture::Create programId:%d textureLoc:%d"), mProgramId, m_loc);

	return m_id;
}
/// @brief Release a texture
/// @return 0
GLuint COpenGL2Texture::Release()
{
	if (m_id) {
		// release texture
		glDeleteTextures(1, &m_id);
		logging->out_debugf(_T("COpenGL2Texture::Release::glDeleteTextures: %d"), m_id);
		m_id = 0;

		ReleaseBuffer();
		ReleaseProgram();
		m_loc = -1;
	}
	return m_id;
}
/// @brief Set a filter type
/// @param[in] filter  GL_NEAREST/GL_LINEAR
void COpenGL2Texture::SetFilter(int filter)
{
	GLenum err;

	if (!m_id) return;

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST + filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST + filter);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::SetFilter::glTexParameteri FILTER 0x%x"), err);
	}
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// no wrap texture
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::SetFilter::glTexParameteri WRAP 0x%x"), err);
	}
}
/// @brief Create vertex buffers and store values in it
/// @param[in] pyl_left    left side of polygon
/// @param[in] pyl_top     top side of polygon
/// @param[in] pyl_right   right side of polygon
/// @param[in] pyl_bottom  bottom side of polygon
/// @param[in] tex_left    left side of texture
/// @param[in] tex_top     top side of texture
/// @param[in] tex_right   right side of texture
/// @param[in] tex_bottom  bottom side of texture
bool COpenGL2Texture::CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom)
{
	GLenum err;

	if (mPosBufferId == 0) {
		glGenBuffers(1, &mPosBufferId);
		while((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer::glGenBuffers: V err:0x%x"), err);
		}
		logging->out_debugf(_T("COpenGL2Texture::CreateBuffer::glGenBuffers: vId:%d"), mPosBufferId);

		glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
		mPositionLoc = glGetAttribLocation(mProgramId, "vPosition");
		while((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer::glGetAttribLocation: vPosition err:0x%x"), err);
		}
		logging->out_debugf(_T("COpenGL2Texture::CreateBuffer::glGetAttribLocation: programId:%d positionLoc:%d"), mProgramId, mPositionLoc);
	}

	// vartex array for polygon
	GLfloat varr[12];
	varr[ 0] = pyl_left; varr[ 1] = pyl_top; varr[ 2] = 0.0f;
	varr[ 3] = pyl_right; varr[ 4] = pyl_top; varr[ 5] = 0.0f;
	varr[ 6] = pyl_left; varr[ 7] = pyl_bottom; varr[ 8] = 0.0f;
	varr[ 9] = pyl_right; varr[10] = pyl_bottom; varr[11] = 0.0f;

	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(varr), varr, GL_STATIC_DRAW);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer::glBufferData: V err:0x%x"), err);
	}

	if (mTexPosBufferId == 0) {
		glGenBuffers(1, &mTexPosBufferId);
		while((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer::glGenBuffers: UV err:0x%x"), err);
		}
		logging->out_debugf(_T("COpenGL2Texture::CreateBuffer::glGenBuffers: uvId:%d"), mTexPosBufferId);

		glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
		mTexPositionLoc = glGetAttribLocation(mProgramId, "vUV");
		while((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer::glGetAttribLocation: vUV err:0x%x"), err);
		}
		logging->out_debugf(_T("COpenGL2Texture::CreateBuffer::glGetAttribLocation: programId:%d texPositionLoc:%d"), mProgramId, mTexPositionLoc);
	}

	// vertex array for texture
	GLfloat uvarr[8];
	uvarr[ 0] = tex_left;  uvarr[ 1] = tex_top;
	uvarr[ 2] = tex_right; uvarr[ 3] = tex_top;
	uvarr[ 4] = tex_left;  uvarr[ 5] = tex_bottom;
	uvarr[ 6] = tex_right; uvarr[ 7] = tex_bottom;

	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvarr), uvarr, GL_STATIC_DRAW);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer::glBufferData: UV err:0x%x"), err);
	}

	// vertex
	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glEnableVertexAttribArray(mPositionLoc);
	glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDisableVertexAttribArray(mPositionLoc);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer:glVertexAttribPointer positionLoc err:0x%x"), err);
	}

	// texture
	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glEnableVertexAttribArray(mTexPositionLoc);
	glVertexAttribPointer(mTexPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDisableVertexAttribArray(mTexPositionLoc);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CreateBuffer:glVertexAttribPointer texPositionLoc err:0x%x"), err);
	}

	return true;
}
/// @brief Release vertex buffers
void COpenGL2Texture::ReleaseBuffer()
{
	if (mTexPosBufferId) {
		glDeleteBuffers(1, &mTexPosBufferId);
		mTexPosBufferId = 0;
	}
	if (mPosBufferId) {
		glDeleteBuffers(1, &mPosBufferId);
		mPosBufferId = 0;
	}
}
bool COpenGL2Texture::SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom)
{
	return CreateBuffer(pyl_left, pyl_top, pyl_right, pyl_bottom, tex_left, tex_top, tex_right, tex_bottom);
}
/// @brief Build (compile and link) shader programs
bool COpenGL2Texture::BuildProgram()
{
	if (!CompileVertex()) {
		return false;
	}
	if (!CompileFragment()) {
		return false;
	}
	if (!LinkProgram()) {
		return false;
	}
	return true;
}
/// @brief Release program
void COpenGL2Texture::ReleaseProgram()
{
	if (mProgramId) {
		glDeleteProgram(mProgramId);
		mProgramId = 0;
	}
	if (mVertexId) {
		glDeleteShader(mVertexId);
		mVertexId = 0;
	}
	if (mFragmentId) {
		glDeleteShader(mFragmentId);
		mFragmentId = 0;
	}
}
/// @brief Compile a shader program
/// @param[in]  type  GL_VERTEX_SHADER/GL_FRAGMENT_SHADER/GL_GEOMETRY_SHADER
/// @param[out] id    shader id
/// @param[in]  prog  program list
/// @return true/false
bool COpenGL2Texture::CompileShader(GLenum type, GLuint &id, const GLchar *prog)
{
	GLenum err;

	id = glCreateShader(type);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Compile::glCreateShader: type:0x%x err:0x%x"), type, err);
	}
	logging->out_debugf(_T("COpenGL2Texture::Compile::glCreateShader: type:0x%x id:%d"), type, id);

	glShaderSource(id, 1, &prog, NULL);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Compile::glShaderSource: type:0x%x err:0x%x"), type, err);
	}
	glCompileShader(id);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Compile::glCompileShader: type:0x%x err:0x%x"), type, err);
	}
	// check compile error
	GLint result = GL_FALSE;
	int len = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		GLchar *msg = new GLchar[len + 1];
		if (msg) {
			glGetShaderInfoLog(id, len, NULL, msg);
			logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::CompileError: type:0x%x %s"), type, msg);
			delete [] msg;
		}
	}

	return (result == GL_TRUE);
}
/// @brief Compile the vertex shader
bool COpenGL2Texture::CompileVertex()
{
	const GLchar *vertexProg =
"#version 120\n"
"attribute vec3 vPosition;\n"
"attribute vec2 vUV;\n"
"varying vec2 UV;\n"
"void main() {\n"
"  gl_Position = vec4(vPosition, 1.0);\n"
"  UV = vUV;\n"
"}\n";
	return CompileShader(GL_VERTEX_SHADER, mVertexId, vertexProg);
}
/// @brief Compile the fragment shader
bool COpenGL2Texture::CompileFragment()
{
	const GLchar *fragmentProg =
"#version 120\n"
"varying vec2 UV;\n"
"uniform sampler2D myTextureSample;\n"
"void main() {\n"
"  gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0) * texture2D(myTextureSample, UV);\n"
"}\n";
	return CompileShader(GL_FRAGMENT_SHADER, mFragmentId, fragmentProg);
}
/// link vertex and fragment shader program
bool COpenGL2Texture::LinkProgram()
{
	GLenum err;

	mProgramId = glCreateProgram();
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::BuildProgram::glCreateProgram: err:0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL2Texture::BuildProgram::glCreateProgram: id:%d"), mProgramId);

	glAttachShader(mProgramId, mVertexId);
	glAttachShader(mProgramId, mFragmentId);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Link::glAttachShader: err:0x%x"), err);
	}

	glLinkProgram(mProgramId);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Link::glLinkProgram: err:0x%x"), err);
	}
	// check compile error
	GLint result = GL_FALSE;
	int len = 0;
	glGetProgramiv(mProgramId, GL_LINK_STATUS, &result);
	glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		GLchar *msg = new GLchar[len + 1];
		if (msg) {
			glGetProgramInfoLog(mProgramId, len, NULL, msg);
			logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::LinkError: %s"), msg);
			delete [] msg;
		}
	}
	glDetachShader(mProgramId, mVertexId);
	glDetachShader(mProgramId, mFragmentId);

	glDeleteShader(mVertexId);
	mVertexId = 0;
	glDeleteShader(mFragmentId);
	mFragmentId = 0;

	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::LinkProgram: err:0x%x"), err);
	}

	return (result == GL_TRUE);
}
/// @brief Select a program to render the texture
void COpenGL2Texture::UseProgram()
{
	glUseProgram(mProgramId);
}
/// @brief Draw the texture on the window
/// @param[in] width
/// @param[in] height
/// @param[in] buffer
void COpenGL2Texture::Draw(int width, int height, void *buffer)
{
	GLenum err;

	UseProgram();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	// draw texture using screen pixel buffer
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, m_format,
			GL_UNSIGNED_BYTE, (GLvoid *)buffer);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Draw::glTexImage2D: 0x%x"), err);
	}

	// bind texture0 to texture in program
	glUniform1i(m_loc, 0);

	// vertex
	glEnableVertexAttribArray(mPositionLoc);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// texture
	glEnableVertexAttribArray(mTexPositionLoc);
	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glVertexAttribPointer(mTexPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// draw vertex
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(mTexPositionLoc);
	glDisableVertexAttribArray(mPositionLoc);

	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Draw: 0x%x"), err);
	}

}
/// @brief Render the texture on the window
/// @param[in] width
/// @param[in] height
/// @param[in] buffer
void COpenGL2Texture::Render(int width, int height, void *buffer)
{
	GLenum err;

	UseProgram();

	glActiveTexture(GL_TEXTURE0);

	// draw texture using screen pixel buffer
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, m_format,
			GL_UNSIGNED_BYTE, (GLvoid *)buffer);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Render::glTexImage2D: 0x%x"), err);
	}

	// bind texture0 to texture in program
	glUniform1i(m_loc, 0);

	// vertex
	glEnableVertexAttribArray(mPositionLoc);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// texture
	glEnableVertexAttribArray(mTexPositionLoc);
	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glVertexAttribPointer(mTexPositionLoc, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// draw vertex
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(mTexPositionLoc);
	glDisableVertexAttribArray(mPositionLoc);

	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL2Texture::Render: 0x%x"), err);
	}

}

// ------------------------------------------------------------------------------

COpenGL2::COpenGL2()
	: COpenGL()
{
}

COpenGL2::~COpenGL2()
{
}

int COpenGL2::Version() const
{
	return 2;
}

bool COpenGL2::Initialize()
{
#ifdef USE_OPENGL_GLEW
	glewInit();
#endif

	COpenGL::Initialize();

	return true;
}

void COpenGL2::Terminate()
{
}

bool COpenGL2::InitViewport(int w, int h)
{
	COpenGL::InitViewport(w, h);

	// bg color
	glClearColor( 0.0, 0.0, 0.0, 1.0 );

	// Setup our viewport : same as window size
	glViewport( 0, 0, w, h );

	return true;
}

void COpenGL2::ClearScreen()
{
	// fill black
	glBegin(GL_QUADS);
	glColor3ub(0, 0, 0);
	glVertex3i(-1, -1, 0);
	glVertex3i(-1,  1, 0);
	glVertex3i( 1,  1, 0);
	glVertex3i( 1, -1, 0);
	glEnd();
}
#endif /* USE_OPENGL21 */

// ==============================================================================

#ifdef USE_OPENGL33

COpenGL3Texture::COpenGL3Texture() : COpenGLTexture()
{
	m_loc = -1;

	mPosBufferId = 0;
	mTexPosBufferId = 0;

	mVertexId = 0;
	mFragmentId = 0;
	mProgramId = 0;
}
/// @brief Provide a interface for a texture on OpenGL 3.3
COpenGL3Texture::COpenGL3Texture(int seq) : COpenGLTexture(seq)
{
	m_loc = -1;

	mPosBufferId = 0;
	mTexPosBufferId = 0;

	mVertexId = 0;
	mFragmentId = 0;
	mProgramId = 0;
}
COpenGL3Texture::~COpenGL3Texture()
{
	ReleaseBuffer();
	ReleaseProgram();
}
/// @brief Create a texture
/// @param[in] filter  GL_NEAREST/GL_LINEAR
/// @return texture id / 0 if failed to create.
GLuint COpenGL3Texture::Create(int filter)
{
	GLenum err;

	// build program
	if (!BuildProgram()) {
		return 0;
	}

	// create
#if defined(_RGB555) || defined(_RGB565)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
#elif defined(_RGB888)
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
#else
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
#endif

	glGenTextures(1, &m_id);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::Create::glGenTextures: 0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL3Texture::Create::textureId: %d"), m_id);

	SetFilter(filter);

	m_loc = glGetUniformLocation(mProgramId, "myTextureSample");
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::Create::glGetUniformLocation: err:0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL3Texture::Create::textureLoc: %d"), m_loc);

	return m_id;
}
/// @brief Release a texture
/// @return 0
GLuint COpenGL3Texture::Release()
{
	if (m_id) {
		// release texture
		glDeleteTextures(1, &m_id);
		m_id = 0;

		ReleaseBuffer();
		ReleaseProgram();
		m_loc = -1;
	}
	return m_id;
}
/// @brief Set a filter type
/// @param[in] filter  GL_NEAREST/GL_LINEAR
void COpenGL3Texture::SetFilter(int filter)
{
	GLenum err;

	if (!m_id) return;

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, m_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST + filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST + filter);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::SetFilter::glTexParameteri FILTER 0x%x"), err);
	}
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// no wrap texture
	glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::SetFilter::glTexParameteri WRAP 0x%x"), err);
	}
}
/// @brief Create vertex buffers and store values in it
/// @param[in] pyl_left    left side of polygon
/// @param[in] pyl_top     top side of polygon
/// @param[in] pyl_right   right side of polygon
/// @param[in] pyl_bottom  bottom side of polygon
/// @param[in] tex_left    left side of texture
/// @param[in] tex_top     top side of texture
/// @param[in] tex_right   right side of texture
/// @param[in] tex_bottom  bottom side of texture
bool COpenGL3Texture::CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom)
{
	GLenum err;

	if (mPosBufferId == 0) {
		glGenBuffers(1, &mPosBufferId);
		while((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CreateBuffer::glGenBuffers: V err:0x%x"), err);
		}
		logging->out_debugf(_T("COpenGL3Texture::CreateBuffer::glGenBuffers: vId:%d"), mPosBufferId);
	}

	// vartex array for polygon
	GLfloat varr[12];
	varr[ 0] = pyl_left; varr[ 1] = pyl_top; varr[ 2] = 0.0f;
	varr[ 3] = pyl_right; varr[ 4] = pyl_top; varr[ 5] = 0.0f;
	varr[ 6] = pyl_left; varr[ 7] = pyl_bottom; varr[ 8] = 0.0f;
	varr[ 9] = pyl_right; varr[10] = pyl_bottom; varr[11] = 0.0f;

	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(varr), varr, GL_STATIC_DRAW);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CreateBuffer::glBufferData: V err:0x%x"), err);
	}


	if (mTexPosBufferId == 0) {
		glGenBuffers(1, &mTexPosBufferId);
		while((err = glGetError()) != GL_NO_ERROR) {
			logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CreateBuffer::glGenBuffers: UV err:0x%x"), err);
		}
		logging->out_debugf(_T("COpenGL3Texture::CreateBuffer::glGenBuffers: uvId:%d"), mTexPosBufferId);
	}

	// vartex array for texture
	GLfloat uvarr[8];
	uvarr[ 0] = tex_left;  uvarr[ 1] = tex_top;
	uvarr[ 2] = tex_right; uvarr[ 3] = tex_top;
	uvarr[ 4] = tex_left;  uvarr[ 5] = tex_bottom;
	uvarr[ 6] = tex_right; uvarr[ 7] = tex_bottom;

	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvarr), uvarr, GL_STATIC_DRAW);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CreateBuffer::glBufferData: UV err:0x%x"), err);
	}

	return true;
}
/// @brief Release vertex buffers
void COpenGL3Texture::ReleaseBuffer()
{
	if (mTexPosBufferId) {
		glDeleteBuffers(1, &mTexPosBufferId);
		mTexPosBufferId = 0;
	}
	if (mPosBufferId) {
		glDeleteBuffers(1, &mPosBufferId);
		mPosBufferId = 0;
	}
}
bool COpenGL3Texture::SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom)
{
	return CreateBuffer(pyl_left, pyl_top, pyl_right, pyl_bottom, tex_left, tex_top, tex_right, tex_bottom);
}
/// @brief Build (compile and link) shader programs
bool COpenGL3Texture::BuildProgram()
{
	if (!CompileVertex()) {
		return false;
	}
	if (!CompileFragment()) {
		return false;
	}
	if (!LinkProgram()) {
		return false;
	}
	return true;
}
/// @brief Release program
void COpenGL3Texture::ReleaseProgram()
{
	if (mProgramId) {
		glDeleteProgram(mProgramId);
		mProgramId = 0;
	}
	if (mVertexId) {
		glDeleteShader(mVertexId);
		mVertexId = 0;
	}
	if (mFragmentId) {
		glDeleteShader(mFragmentId);
		mFragmentId = 0;
	}
}
/// @brief Compile a shader program
/// @param[in]  type  GL_VERTEX_SHADER/GL_FRAGMENT_SHADER/GL_GEOMETRY_SHADER
/// @param[out] id    shader id
/// @param[in]  prog  program list
/// @return true/false
bool COpenGL3Texture::CompileShader(GLenum type, GLuint &id, const GLchar *prog)
{
	GLenum err;

	id = glCreateShader(type);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CompileShader::glCreateShader: type:0x%x err:0x%x"), type, err);
	}
	logging->out_debugf(_T("COpenGL3Texture::CompileShader::glCreateShader: type:0x%x id:%d"), type, id);

	glShaderSource(id, 1, &prog, NULL);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CompileShader::glShaderSource: type:0x%x err:0x%x"), type, err);
	}
	glCompileShader(id);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CompileShader::glCompileShader: type:0x%x err:0x%x"), type, err);
	}
	// check compile error
	GLint result = GL_FALSE;
	int len = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		GLchar *msg = new GLchar[len + 1];
		if (msg) {
			glGetShaderInfoLog(id, len, NULL, msg);
			logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::CompileError: type:0x%x %s"), type, msg);
			delete [] msg;
		}
	}

	return (result == GL_TRUE);
}
/// @brief Compile the vertex shader
bool COpenGL3Texture::CompileVertex()
{
	const GLchar *vertexProg =
"#version 330 core\n"
"layout(location = 0) in vec3 vPos;\n"
"layout(location = 1) in vec2 vUV;\n"
"out vec2 UV;\n"
"void main() {\n"
"  gl_Position = vec4(vPos, 1);\n"
"  UV = vUV;\n"
"}\n";
	return CompileShader(GL_VERTEX_SHADER, mVertexId, vertexProg);
}
/// @brief Compile the fragment shader
bool COpenGL3Texture::CompileFragment()
{
	const GLchar *fragmentProg =
"#version 330 core\n"
"in vec2 UV;\n"
"out vec4 color;\n"
"uniform sampler2D myTextureSample;\n"
"void main() {\n"
"  color = texture(myTextureSample, UV);\n"
"}\n";
	return CompileShader(GL_FRAGMENT_SHADER, mFragmentId, fragmentProg);
}
/// @brief Link vertex and fragment shader program
bool COpenGL3Texture::LinkProgram()
{
	GLenum err;

	mProgramId = glCreateProgram();
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::LinkProgram::glCreateProgram: err:0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL3Texture::LinkProgram::glCreateProgram: id:%d"), mProgramId);

	glAttachShader(mProgramId, mVertexId);
	glAttachShader(mProgramId, mFragmentId);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::LinkProgram::glAttachShader: err:0x%x"), err);
	}

	glLinkProgram(mProgramId);
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::LinkProgram::glLinkProgram: err:0x%x"), err);
	}
	// check compile error
	GLint result = GL_FALSE;
	int len = 0;
	glGetProgramiv(mProgramId, GL_LINK_STATUS, &result);
	glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		GLchar *msg = new GLchar[len + 1];
		if (msg) {
			glGetProgramInfoLog(mProgramId, len, NULL, msg);
			logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::LinkError: %s"), msg);
			delete [] msg;
		}
	}
	glDetachShader(mProgramId, mVertexId);
	glDetachShader(mProgramId, mFragmentId);

	glDeleteShader(mVertexId);
	mVertexId = 0;
	glDeleteShader(mFragmentId);
	mFragmentId = 0;

	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::LinkProgram: err:0x%x"), err);
	}

	return (result == GL_TRUE);
}
/// @brief Select a program to render the texture
void COpenGL3Texture::UseProgram()
{
	glUseProgram(mProgramId);
}
/// @brief Draw the texture on the window
/// @param[in] width
/// @param[in] height
/// @param[in] buffer
void COpenGL3Texture::Draw(int width, int height, void *buffer)
{
	GLenum err;

	UseProgram();

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	// draw texture using screen pixel buffer
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, m_format,
			GL_UNSIGNED_BYTE, (GLvoid *)buffer);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::Draw::glTexImage2D: 0x%x"), err);
	}

	// bind texture0 to texture in program
	glUniform1i(m_loc, 0);

	// vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// texture
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// draw vertex
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::Draw: err:0x%x"), err);
	}
}
/// @brief Render the texture on the window
/// @param[in] width
/// @param[in] height
/// @param[in] buffer
void COpenGL3Texture::Render(int width, int height, void *buffer)
{
	GLenum err;

	UseProgram();

	glActiveTexture(GL_TEXTURE0);

	// draw texture using screen pixel buffer
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, m_format,
			GL_UNSIGNED_BYTE, (GLvoid *)buffer);
	while ((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::Render::glTexImage2D: 0x%x"), err);
	}

	// bind texture0 to texture in program
	glUniform1i(m_loc, 0);

	// vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, mPosBufferId);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// texture
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, mTexPosBufferId);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// draw vertex
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3Texture::Renderer: err:0x%x"), err);
	}
}

// ------------------------------------------------------------------------------

COpenGL3::COpenGL3()
	: COpenGL()
{
	vaoId = 0;
}

COpenGL3::~COpenGL3()
{
	ReleaseVao();
}

int COpenGL3::Version() const
{
	return 3;
}
/// @brief Create vertex array objects
/// @return true if success
bool COpenGL3::CreateVao()
{
	GLenum err;

#if defined(__APPLE__) && defined(__MACH__)
	glGenVertexArraysAPPLE(1, &vaoId);
#else
	glGenVertexArrays(1, &vaoId);
#endif
	while((err = glGetError()) != GL_NO_ERROR) {
		logging->out_logf(LOG_ERROR, _T("COpenGL3::CreateVao::glGenVertexArrays: err:0x%x"), err);
	}
	logging->out_debugf(_T("COpenGL3::CreateVao::glGenVertexArrays: id:%d"), vaoId);

#if defined(__APPLE__) && defined(__MACH__)
	glBindVertexArrayAPPLE(vaoId);
#else
	glBindVertexArray(vaoId);
#endif

	return true;
}
/// @brief Release vertex array objects
void COpenGL3::ReleaseVao()
{
	if (vaoId) {
#if defined(__APPLE__) && defined(__MACH__)
		glDeleteVertexArraysAPPLE(1, &vaoId);
#else
		glDeleteVertexArrays(1, &vaoId);
#endif
		vaoId = 0;
	}
}
/// @brief Initialize the instance
bool COpenGL3::Initialize()
{
#ifdef USE_OPENGL_GLEW
	glewInit();
#endif

	COpenGL::Initialize();

	if (!CreateVao()) {
		return false;
	}
	return true;
}
/// @brief Terminate the instance
void COpenGL3::Terminate()
{
	ReleaseVao();
}

bool COpenGL3::InitViewport(int w, int h)
{
	COpenGL::InitViewport(w, h);

	// bg color
	glClearColor( 0.0, 0.0, 0.0, 1.0 );

	// Setup our viewport : same as window size
	glViewport( 0, 0, w, h );

	return true;
}

void COpenGL3::ClearScreen()
{
	// fill black
	glBegin(GL_QUADS);
	glColor3ub(0, 0, 0);
	glVertex3i(-1, -1, 0);
	glVertex3i(-1,  1, 0);
	glVertex3i( 1,  1, 0);
	glVertex3i( 1, -1, 0);
	glEnd();
}
#endif /* USE_OPENGL33 */

#endif /* USE_OPENGL */
