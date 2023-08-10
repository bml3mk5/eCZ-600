/** @file opengl.h

	Skelton for retropc emulator

	@author Sasaji
	@date   2020.03.08

	@brief [ OpenGL ]
*/

#ifndef OSD_OPENGL_H
#define OSD_OPENGL_H

#if defined(USE_OPENGL)

#if defined(_WIN32)
#include <windows.h>
//# define USE_OPENGL_GLEW
//# define USE_OPENGL21
//# define USE_OPENGL33
# if defined(_MSC_VER) && defined(USE_OPENGL_GLEW)
#  ifdef _DEBUG
#   pragma  comment(lib,"glew32sd.lib")
#  else
#   pragma  comment(lib,"glew32s.lib")
#  endif
# endif
#elif defined(__APPLE__) && defined(__MACH__)
# define USE_OPENGL_CGL
# define USE_OPENGL_GLEXT
# define USE_OPENGL21
# define USE_OPENGL33
#elif defined(linux)
# define USE_OPENGL_GLX
# define USE_OPENGL_GLEXT
# define USE_OPENGL21
# define USE_OPENGL33
#endif

#ifdef USE_OPENGL_GLEXT
# ifndef GL_GLEXT_PROTOTYPES
#  define GL_GLEXT_PROTOTYPES
# endif
#endif

#ifdef USE_OPENGL_GLEW

#define GLEW_STATIC 1
#include <gl/glew.h>

#else

#if defined(__APPLE__) && defined(__MACH__)
# include <OpenGL/gl.h>
#else
# include <GL/gl.h>
#endif

#ifdef USE_OPENGL_GLX
# if defined(__APPLE__) && defined(__MACH__)
#  include <OpenGL/glx.h>
# else
#  include <GL/glx.h>
# endif
#endif

#ifdef USE_OPENGL_CGL
# if defined(__APPLE__) && defined(__MACH__)
#  include <OpenGL/OpenGL.h>
#  include <OpenGL/CGLContext.h>
#  include <OpenGL/CGLTypes.h>
# endif
#endif

#ifdef USE_OPENGL_GLEXT
# if defined(__APPLE__) && defined(__MACH__)
#  include <OpenGL/glext.h>
# else
#  include <GL/glext.h>
# endif
#endif

#endif /* USE_OPENGL_GLEW */


class CSurface;
class COpenGL;

/// @brief Wrapper class to access an OpenGL texture.
///
/// @note This is an abstruct class.
class COpenGLTexture
{
protected:
	COpenGL *m_opengl;

	// texture ID
	GLuint m_id;

	// Sequence id
	int m_seq;

	// Pixel format
	GLenum m_format;

	COpenGLTexture();

public:
	COpenGLTexture(int seq);
	virtual ~COpenGLTexture();

	static COpenGLTexture *New(COpenGL *opengl, int seq, int force = 0);

	virtual void SetParent(COpenGL *opengl);

	virtual GLuint Create(int filter) = 0;
	virtual GLuint Release();
	virtual	void SetFilter(int filter);

	virtual bool CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom) = 0;
	virtual void ReleaseBuffer();

	virtual bool SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom) = 0;

	virtual bool BuildProgram() { return false; }
	virtual void ReleaseProgram() {}

	virtual void Draw(int width, int height, void *buffer) = 0;
	virtual void Render(int width, int height, void *buffer) = 0;

	GLuint GetID() const { return m_id; }

	void PresetRGBA();
	void PresetBGRA();
};

/// @brief Wrapper class to access the OpenGL function.
///
/// @note This is an abstruct class.
class COpenGL
{
protected:
	int m_disp_w, m_disp_h;

public:
	COpenGL();
	virtual ~COpenGL();

	static COpenGL *New(int force = 0);
	virtual int Version() const;

	virtual bool Initialize();
	virtual void Terminate();

	virtual bool InitViewport(int w, int h);
	int Width() const { return m_disp_w; }
	int Height() const { return m_disp_h; }

	virtual unsigned char SetInterval(unsigned char use, void *user_data = 0);

	virtual void ClearScreen();
};

///
/// @brief Classic OpenGL texture.
///
class COpenGL1Texture : public COpenGLTexture
{
private:
	GLfloat m_tex_l, m_tex_r, m_tex_t, m_tex_b;
	GLfloat m_pyl_l, m_pyl_r, m_pyl_t, m_pyl_b;

	COpenGL1Texture();

public:
	COpenGL1Texture(int seq);
	~COpenGL1Texture();

	GLuint Create(int filter);
	GLuint Release();
	void SetFilter(int filter);

	bool CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom);

	bool SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom);

	void Draw(int width, int height, void *buffer);
	void Render(int width, int height, void *buffer);
};

///
/// @brief Classic OpenGL 
///
class COpenGL1 : public COpenGL
{
private:

public:
	COpenGL1();
	~COpenGL1();

	int Version() const;

	bool Initialize();
	void Terminate();

	bool InitViewport(int w, int h);

	void ClearScreen();
};

#ifdef USE_OPENGL21
///
/// @brief OpenGL 2.1 texture.
///
class COpenGL2Texture : public COpenGLTexture
{
private:
	// Location
	GLint m_loc;
	// vertex buffer
	GLint mPositionLoc;
	GLuint mPosBufferId;
	// UV buffer
	GLint mTexPositionLoc;
	GLuint mTexPosBufferId;
	// for shader program
	GLuint mVertexId;
	GLuint mFragmentId;
	GLuint mProgramId;
	bool CompileShader(GLenum type, GLuint &id, const GLchar *prog);
	bool CompileVertex();
	bool CompileFragment();
	bool LinkProgram();
	bool BuildProgram();
	void ReleaseProgram();
	void UseProgram();

	COpenGL2Texture();

public:
	COpenGL2Texture(int seq);
	~COpenGL2Texture();

	GLuint Create(int filter);
	GLuint Release();
	void SetFilter(int filter);

	bool CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom);
	void ReleaseBuffer();

	bool SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom);

	void Draw(int width, int height, void *buffer);
	void Render(int width, int height, void *buffer);
};

///
/// @brief OpenGL 2.1
///
class COpenGL2 : public COpenGL
{
private:

public:
	COpenGL2();
	~COpenGL2();

	int Version() const;

	bool Initialize();
	void Terminate();

	bool InitViewport(int w, int h);

	void ClearScreen();
};
#endif /* USE_OPENGL21 */

#ifdef USE_OPENGL33
///
/// @brief OpenGL 3.3 texture.
///
class COpenGL3Texture : public COpenGLTexture
{
private:
	// Location
	GLint m_loc;
	// vertex buffer
	GLuint mPosBufferId;
	// UV buffer
	GLuint mTexPosBufferId;

	// for shader program
	GLuint mVertexId;
	GLuint mFragmentId;
	GLuint mProgramId;
	bool CompileShader(GLenum type, GLuint &id, const GLchar *prog);
	bool CompileVertex();
	bool CompileFragment();
	bool LinkProgram();
	bool BuildProgram();
	void ReleaseProgram();
	void UseProgram();

	COpenGL3Texture();

public:
	COpenGL3Texture(int seq);
	~COpenGL3Texture();

	GLuint Create(int filter);
	GLuint Release();
	void SetFilter(int filter);

	bool CreateBuffer(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom);
	void ReleaseBuffer();

	bool SetPos(GLfloat pyl_left, GLfloat pyl_top, GLfloat pyl_right, GLfloat pyl_bottom, GLfloat tex_left, GLfloat tex_top, GLfloat tex_right, GLfloat tex_bottom);

	void Draw(int width, int height, void *buffer);
	void Render(int width, int height, void *buffer);
};
///
/// @brief OpenGL 3.3 or later
///
class COpenGL3 : public COpenGL
{
private:
	// vertex array
	GLuint vaoId;
	bool CreateVao();
	void ReleaseVao();

public:
	COpenGL3();
	~COpenGL3();

	int Version() const;

	bool Initialize();
	void Terminate();

	bool InitViewport(int w, int h);

	void ClearScreen();
};
#endif /* USE_OPENGL33 */

#endif /* USE_OPENGL */

#endif /* OSD_OPENGL_H */
