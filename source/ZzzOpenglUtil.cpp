///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "zzzObject.h"
#include "zzzcharacter.h"
#include "Zzzinfomation.h"
#include "NewUISystem.h"

int OpenglWindowX = 0;
int OpenglWindowY = 0;
int OpenglWindowWidth = 640;
int OpenglWindowHeight = 480;
bool CameraTopViewEnable = false;
float CameraViewNear = 20.f;
float CameraViewFar = 2000.f;
float CameraFOV = 55.f;
vec3_t CameraPosition = { 0.f, 0.f, 0.f };
vec3_t CameraAngle = { 0.f, 0.f, 0.f };
float CameraMatrix[3][4];
vec3_t MousePosition = { 0.f, 0.f, 0.f };
vec3_t MouseTarget = { 0.f, 0.f, 0.f };
float g_fCameraCustomDistance = 0.f;
bool FogEnable = false;
GLfloat FogDensity = 0.0004f;
GLfloat FogColor[4] = { 30 / 256.f, 20 / 256.f, 10 / 256.f, 1.f };

unsigned int WindowWidth = 1024;
unsigned int WindowHeight = 768;
int MouseX = WindowWidth / 2;
int MouseY = WindowHeight / 2;
int BackMouseX = MouseX;
int BackMouseY = MouseY;
bool MouseLButton = false;
bool MouseLButtonPop = false;
bool MouseLButtonPush = false;
bool MouseRButton = false;
bool MouseRButtonPop = false;
bool MouseRButtonPush = false;
bool MouseLButtonDBClick = false;
bool MouseMButton = false;
bool MouseMButtonPop = false;
bool MouseMButtonPush = false;
int MouseWheel = 0;
DWORD MouseRButtonPress = 0;

void OpenExploper(char* Name, char* para)
{
	ShellExecute(NULL, "open", Name, para, "", SW_SHOW);
}

bool CheckID_HistoryDay(char* Name, WORD day)
{
	struct DayHistory
	{
		char ID[MAX_ID_SIZE + 1];
		WORD date;
	};
	DayHistory days[100];
	int count = 0;
	WORD num = 0;
	bool sameName = false;
	bool update = true;
	FILE* fp = fopen("dconfig.ini", "rb");

	if (fp != nullptr)
	{
		fread(&num, sizeof(WORD), 1, fp);

		if (num > 100)
		{
			num = 0;
		}
		else
		{
			for (int i = 0; i < num; ++i)
			{
				fread(days[i].ID, sizeof(char), MAX_ID_SIZE + 1, fp);
				fread(&days[i].date, sizeof(WORD), 1, fp);

				if (!strcmp(days[i].ID, Name))
				{
					sameName = true;
					if (days[i].date == day)
					{
						update = false;
						break;
					}
					days[i].date = day;
				}
				count++;
			}
		}
		fclose(fp);
	}

	if (update)
	{
		if (!sameName)
		{
			memcpy(days[num].ID, Name, (MAX_ID_SIZE + 1) * sizeof(char));
			days[num].date = day;

			num++;
		}

		fp = fopen("dconfig.ini", "wb");

		fwrite(&num, sizeof(WORD), 1, fp);
		for (int i = 0; i < num; ++i)
		{
			fwrite(days[i].ID, sizeof(char), MAX_ID_SIZE + 1, fp);
			fwrite(&days[i].date, sizeof(WORD), 1, fp);
		}

		fclose(fp);
	}

	return update;
}

bool GrabEnable = false;
char GrabFileName[MAX_PATH];
int  GrabScreen = 0;
bool GrabFirst = false;

void SaveScreen()
{
	GrabFirst = true;
	auto Buffer = new unsigned char[WindowWidth * WindowHeight * 4];

	glReadPixels(0, 0, WindowWidth, WindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, Buffer);

	auto BufferNew = new unsigned char[WindowWidth * WindowHeight * 3];

	int counter = 0;
	for (int i = 0; i < WindowWidth * WindowHeight * 4; i += 4)
	{
		BufferNew[counter + 0] = Buffer[i + 0];
		BufferNew[counter + 1] = Buffer[i + 1];
		BufferNew[counter + 2] = Buffer[i + 2];

		counter += 3;
	}

	WriteJpeg(GrabFileName, WindowWidth, WindowHeight, BufferNew, 100);

	delete[] Buffer;
	delete[] BufferNew;

	GrabScreen++;
	GrabScreen %= 10000;
}

float PerspectiveX;
float PerspectiveY;
int   ScreenCenterX;
int   ScreenCenterY;
int   ScreenCenterYFlip;

void GetOpenGLMatrix(float Matrix[3][4])
{
	float OpenGLMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, OpenGLMatrix);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			Matrix[i][j] = OpenGLMatrix[j * 4 + i];
		}
	}
}

void gluPerspective2(float Fov, float Aspect, float ZNear, float ZFar)
{
	gluPerspective(Fov, Aspect, ZNear, ZFar);
	ScreenCenterX = OpenglWindowX + OpenglWindowWidth / 2;
	ScreenCenterY = OpenglWindowY + OpenglWindowHeight / 2;
	ScreenCenterYFlip = WindowHeight - ScreenCenterY;

	float AspectY = (float)(WindowHeight) / (float)(OpenglWindowHeight);
	PerspectiveX = tanf(Fov * 0.5f * 3.141592f / 180.f) / ((float)OpenglWindowWidth / 2) * Aspect;
	PerspectiveY = tanf(Fov * 0.5f * 3.141592f / 180.f) / ((float)OpenglWindowHeight / 2) * AspectY;
}

void CreateScreenVector(int sx, int sy, vec3_t Target, bool bFixView) // OK
{
	sx = sx * WindowWidth / 640;
	sy = sy * WindowHeight / 480;
	vec3_t p1, p2;
	float farVal = bFixView ? CameraViewFar : RENDER_ITEMVIEW_FAR;
	p1[0] = (float)(sx - ScreenCenterX) * farVal * PerspectiveX;
	p1[1] = -(float)(sy - ScreenCenterY) * farVal * PerspectiveY;
	p1[2] = -farVal;
	p2[0] = -CameraMatrix[0][3];
	p2[1] = -CameraMatrix[1][3];
	p2[2] = -CameraMatrix[2][3];
	VectorIRotate(p2, CameraMatrix, MousePosition);
	VectorIRotate(p1, CameraMatrix, p2);
	VectorAdd(MousePosition, p2, Target);
}

void Projection(vec3_t Position, int* sx, int* sy)
{
	vec3_t TrasformPosition;
	VectorTransform(Position, CameraMatrix, TrasformPosition);
	*sx = -(int)(TrasformPosition[0] / PerspectiveX / TrasformPosition[2]) + ScreenCenterX;
	*sy = (int)(TrasformPosition[1] / PerspectiveY / TrasformPosition[2]) + ScreenCenterY;
	*sx = *sx * 640 / (int)WindowWidth;
	*sy = *sy * 480 / (int)WindowHeight;
}

void TransformPosition(vec3_t Position, vec3_t WorldPosition, int* x, int* y)
{
	vec3_t Temp;
	VectorSubtract(Position, CameraPosition, Temp);
	VectorRotate(Temp, CameraMatrix, WorldPosition);
	*x = (int)(WorldPosition[0] / PerspectiveX / -WorldPosition[2]) + ScreenCenterX;
	*y = (int)(WorldPosition[1] / PerspectiveY / -WorldPosition[2]) + ScreenCenterYFlip;
}

bool TestDepthBuffer(vec3_t Position)
{
	vec3_t WorldPosition;
	int x, y;
	TransformPosition(Position, WorldPosition, &x, &y);
	if (x < OpenglWindowX ||
		y < OpenglWindowY ||
		x >= OpenglWindowX + OpenglWindowWidth ||
		y >= OpenglWindowY + OpenglWindowHeight)
	{
		return false;
	}
	GLfloat depth;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
	float z = 1.f - CameraViewNear / -WorldPosition[2] + CameraViewNear / CameraViewFar;
	if (depth >= z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////
// OpenGL - Render Util
///////////////////////////////////////////////////////////////////////////////

int  CachTexture = -1;
bool TextureEnable;
bool DepthTestEnable;
bool CullFaceEnable;
bool DepthMaskEnable;
bool AlphaTestEnable;
int  AlphaBlendType;

void BindTexture(int tex)
{
	if (CachTexture != tex)
	{
		CachTexture = tex;
		if (tex >= 0)
		{
			BITMAP_t* b = &Bitmaps[tex];
			glBindTexture(GL_TEXTURE_2D, b->TextureNumber);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, -1 * tex);
		}
	}
}

bool TextureStream = false;

extern  int test;
void BindTextureStream(int tex)
{
	if (CachTexture != tex)
	{
		CachTexture = tex;
		if (TextureStream)
			glEnd();
		BITMAP_t* b = &Bitmaps[tex];
		glBindTexture(GL_TEXTURE_2D, b->TextureNumber);

		glBegin(GL_TRIANGLES);
		TextureStream = true;
	}
}

void EndTextureStream()
{
	if (TextureStream)
		glEnd();
	TextureStream = false;
}

void EnableDepthTest()
{
	if (!DepthTestEnable)
	{
		DepthTestEnable = true;
		glEnable(GL_DEPTH_TEST);
	}
}

void DisableDepthTest()
{
	if (DepthTestEnable)
	{
		DepthTestEnable = false;
		glDisable(GL_DEPTH_TEST);
	}
}

void EnableDepthMask()
{
	if (!DepthMaskEnable)
	{
		DepthMaskEnable = true;
		glDepthMask(true);
	}
}

void DisableDepthMask()
{
	if (DepthMaskEnable)
	{
		DepthMaskEnable = false;
		glDepthMask(false);
	}
}

void EnableCullFace()
{
	if (!CullFaceEnable)
	{
		CullFaceEnable = true;
		glEnable(GL_CULL_FACE);
	}
}

void DisableCullFace()
{
	if (CullFaceEnable)
	{
		CullFaceEnable = false;
		glDisable(GL_CULL_FACE);
	}
}

void DisableTexture(bool AlphaTest)
{
	EnableDepthMask();
	if (AlphaTest == true)
	{
		if (!AlphaTestEnable)
		{
			AlphaTestEnable = true;
			glEnable(GL_ALPHA_TEST);
		}
	}
	else
	{
		if (AlphaTestEnable)
		{
			AlphaTestEnable = false;
			glDisable(GL_ALPHA_TEST);
		}
	}
	if (TextureEnable)
	{
		TextureEnable = false;
		glDisable(GL_TEXTURE_2D);
	}
}

void DisableAlphaBlend()
{
	if (AlphaBlendType != 0)
	{
		AlphaBlendType = 0;
		glDisable(GL_BLEND);
	}
	EnableCullFace();
	EnableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaTest(bool DepthMask)
{
	if (AlphaBlendType != 2)
	{
		AlphaBlendType = 2;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	DisableCullFace();
	if (DepthMask)
		EnableDepthMask();
	if (!AlphaTestEnable)
	{
		AlphaTestEnable = true;
		glEnable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend()
{
	if (AlphaBlendType != 3)
	{
		AlphaBlendType = 3;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
	}
	DisableCullFace();
	DisableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glDisable(GL_FOG);
}

void EnableAlphaBlendMinus()
{
	if (AlphaBlendType != 4)
	{
		AlphaBlendType = 4;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	}
	DisableCullFace();
	DisableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend2()
{
	if (AlphaBlendType != 5)
	{
		AlphaBlendType = 5;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ONE);
	}
	DisableCullFace();
	DisableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend3()
{
	if (AlphaBlendType != 6)
	{
		AlphaBlendType = 6;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	DisableCullFace();
	DisableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend4()
{
	if (AlphaBlendType != 7)
	{
		AlphaBlendType = 7;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}
	DisableCullFace();
	DisableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void EnableLightMap()
{
	if (AlphaBlendType != 1)
	{
		AlphaBlendType = 1;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	}
	EnableCullFace();
	EnableDepthMask();
	if (AlphaTestEnable)
	{
		AlphaTestEnable = false;
		glDisable(GL_ALPHA_TEST);
	}
	if (!TextureEnable)
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if (FogEnable)
		glEnable(GL_FOG);
}

void glViewport2(int x, int y, int Width, int Height)
{
	OpenglWindowX = x;
	OpenglWindowY = y;
	OpenglWindowWidth = Width;
	OpenglWindowHeight = Height;
	glViewport(x, WindowHeight - (y + Height), Width, Height);
}

float ConvertX(float x)
{
	return x * (float)WindowWidth / 640.f;
}

float ConvertY(float y)
{
	return y * (float)WindowHeight / 480.f;
}

void BeginOpengl(int x, int y, int Width, int Height)
{
	x = x * WindowWidth / 640;
	y = y * WindowHeight / 480;
	Width = Width * WindowWidth / 640;
	Height = Height * WindowHeight / 480;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glViewport2(x, y, Width, Height);

	gluPerspective2(CameraFOV, (float)Width / (float)Height, CameraViewNear, CameraViewFar * 1.4f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(CameraAngle[1], 0.f, 1.f, 0.f);
	if (CameraTopViewEnable == false)
		glRotatef(CameraAngle[0], 1.f, 0.f, 0.f);
	glRotatef(CameraAngle[2], 0.f, 0.f, 1.f);
	glTranslatef(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);

	glDisable(GL_ALPHA_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(true);
	AlphaTestEnable = false;
	TextureEnable = true;
	DepthTestEnable = true;
	CullFaceEnable = true;
	DepthMaskEnable = true;
	glDepthFunc(GL_LEQUAL);
	glAlphaFunc(GL_GREATER, 0.25f);
	if (FogEnable)
	{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_DENSITY, FogDensity);
		glFogfv(GL_FOG_COLOR, FogColor);
	}
	else
	{
		glDisable(GL_FOG);
	}

	GetOpenGLMatrix(CameraMatrix);
}

void EndOpengl()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void UpdateMousePosition()
{
	vec3_t vPos;

	glLoadIdentity();
	glTranslatef(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);
	GetOpenGLMatrix(CameraMatrix);

	Vector(-CameraMatrix[0][3], -CameraMatrix[1][3], -CameraMatrix[2][3], vPos);
	VectorIRotate(vPos, CameraMatrix, MousePosition);
}

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
BOOL IsGLExtensionSupported(const char* extension)
{
	const size_t extlen = strlen(extension);
	const char* supported = NULL;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char* (__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL)
		supported = (char*)glGetString(GL_EXTENSIONS);

	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
		return FALSE;

	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL)
			return FALSE;															// No Match

		if ((p == supported || p[-1] == ' ') && (p[extlen] == '\0' || p[extlen] == ' '))
			return TRUE;															// Match
	}
}

BOOL InitGLMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd, int iRequestMSAAValue, int& OutiPixelFormat)
{
	BOOL bIsGLMultisampleSupported = FALSE;

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// See If The String Exists In WGL!
	if (!IsGLExtensionSupported("WGL_ARB_multisample"))
	{
		bIsGLMultisampleSupported = FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
	// Get Our Pixel Format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB)
	{
		bIsGLMultisampleSupported = FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Get Our Current Device Context
	HDC hDC = GetDC(hWnd);

	int		valid;
	UINT	numFormats;
	float	fAttributes[] = { 0,0 };

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
			WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB,24,
			WGL_ALPHA_BITS_ARB,8,
			WGL_DEPTH_BITS_ARB,16,
			WGL_STENCIL_BITS_ARB,0,
			WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
			WGL_SAMPLES_ARB, iRequestMSAAValue,					// xN MultiSampling (N=4,2,1)
			0,0
	};

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &OutiPixelFormat, &numFormats);

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &OutiPixelFormat, &numFormats);
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Return The Valid Format
	return  bIsGLMultisampleSupported;
}

void SetEnableMultisample()
{
	if (TRUE == g_bSupportedMSAA)
	{
		glEnable(GL_MULTISAMPLE_ARB);							// Enable Multisampling
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
}

void SetDisableMultisample()
{
	if (TRUE == g_bSupportedMSAA)
	{
		glDisable(GL_MULTISAMPLE_ARB);							// Enable Multisampling
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
}

#endif // LDS_ADD_MULTISAMPLEANTIALIASING

///////////////////////////////////////////////////////////////////////////////
// render util
///////////////////////////////////////////////////////////////////////////////

inline void TEXCOORD(float* c, float u, float v)
{
	c[0] = u;
	c[1] = v;
}

void RenderBox(float Matrix[3][4])
{
	vec3_t BoundingBoxMin;
	vec3_t BoundingBoxMax;
	Vector(-10.f, -30.f, -10.f, BoundingBoxMin);
	Vector(10.f, 0.f, 10.f, BoundingBoxMax);

	vec3_t BoundingVertices[8];
	Vector(BoundingBoxMax[0], BoundingBoxMax[1], BoundingBoxMax[2], BoundingVertices[0]);
	Vector(BoundingBoxMax[0], BoundingBoxMax[1], BoundingBoxMin[2], BoundingVertices[1]);
	Vector(BoundingBoxMax[0], BoundingBoxMin[1], BoundingBoxMax[2], BoundingVertices[2]);
	Vector(BoundingBoxMax[0], BoundingBoxMin[1], BoundingBoxMin[2], BoundingVertices[3]);
	Vector(BoundingBoxMin[0], BoundingBoxMax[1], BoundingBoxMax[2], BoundingVertices[4]);
	Vector(BoundingBoxMin[0], BoundingBoxMax[1], BoundingBoxMin[2], BoundingVertices[5]);
	Vector(BoundingBoxMin[0], BoundingBoxMin[1], BoundingBoxMax[2], BoundingVertices[6]);
	Vector(BoundingBoxMin[0], BoundingBoxMin[1], BoundingBoxMin[2], BoundingVertices[7]);

	vec3_t TransformVertices[8];
	for (int j = 0; j < 8; j++)
	{
		VectorTransform(BoundingVertices[j], Matrix, TransformVertices[j]);
	}

	glBegin(GL_QUADS);
	//glBegin(GL_LINES);
	glColor3f(0.2f, 0.2f, 0.2f);
	glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
	glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[6]);
	glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[4]);
	glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[5]);

	glColor3f(0.2f, 0.2f, 0.2f);
	glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
	glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[2]);
	glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[3]);
	glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[1]);

	glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
	glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[3]);
	glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[2]);
	glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[6]);

	glColor3f(0.6f, 0.6f, 0.6f);
	glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
	glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[1]);
	glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[5]);
	glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[4]);

	glColor3f(0.4f, 0.4f, 0.4f);
	glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
	glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[5]);
	glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[1]);
	glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[3]);

	glColor3f(0.4f, 0.4f, 0.4f);
	glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
	glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[4]);
	glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[6]);
	glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[2]);
	glEnd();
}

void RenderPlane3D(float Width, float Height, float Matrix[3][4])
{
	vec3_t BoundingVertices[4];
	Vector(-Width, -Width, Height, BoundingVertices[3]);
	Vector(Width, Width, Height, BoundingVertices[2]);
	Vector(Width, Width, -Height, BoundingVertices[1]);
	Vector(-Width, -Width, -Height, BoundingVertices[0]);

	vec3_t TransformVertices[4];
	for (int j = 0; j < 4; j++)
	{
		VectorTransform(BoundingVertices[j], Matrix, TransformVertices[j]);
	}

	glBegin(GL_QUADS);
	glTexCoord2f(0.f, 1.f); glVertex3fv(TransformVertices[0]);
	glTexCoord2f(1.f, 1.f); glVertex3fv(TransformVertices[1]);
	glTexCoord2f(1.f, 0.f); glVertex3fv(TransformVertices[2]);
	glTexCoord2f(0.f, 0.f); glVertex3fv(TransformVertices[3]);
	glEnd();
}

void BeginSprite()
{
	glPushMatrix();
	glLoadIdentity();
}

void EndSprite()
{
	glPopMatrix();
}

void RenderSprite(int texture, vec3_t position, float width, float height, vec3_t light, float rotation, float u, float v, float uWidth, float vHeight) {
	BindTexture(texture);

	vec3_t p2;
	VectorTransform(position, CameraMatrix, p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	width *= 0.5f;
	height *= 0.5f;

	vec3_t p[4];
	if (rotation == 0) {
		Vector(x - width, y - height, z, p[0]);
		Vector(x + width, y - height, z, p[1]);
		Vector(x + width, y + height, z, p[2]);
		Vector(x - width, y + height, z, p[3]);
	}
	else {
		vec3_t p2[4];
		Vector(-width, -height, z, p2[0]);
		Vector(width, -height, z, p2[1]);
		Vector(width, height, z, p2[2]);
		Vector(-width, height, z, p2[3]);
		vec3_t angle;
		Vector(0.0f, 0.0f, rotation, angle);
		float matrix[3][4];
		AngleMatrix(angle, matrix);
		for (int i = 0; i < 4; i++) {
			VectorRotate(p2[i], matrix, p[i]);
			p[i][0] += x;
			p[i][1] += y;
		}
	}

	float c[4][2];
	TEXCOORD(c[3], u, v);
	TEXCOORD(c[2], u + uWidth, v);
	TEXCOORD(c[1], u + uWidth, v + vHeight);
	TEXCOORD(c[0], u, v + vHeight);

	glBegin(GL_QUADS);
	if (Bitmaps[texture].Components == 3) {
		glColor3fv(light);
	}
	else {
		if (texture == BITMAP_BLOOD + 1 || texture == BITMAP_FONT_HIT) {
			glColor4f(light[0], light[1], light[2], 1.0f);
		}
		else {
			glColor4f(light[0], light[1], light[2], light[0]);
		}
	}
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(c[i][0], c[i][1]);
		glVertex3fv(p[i]);
	}
	glEnd();
}

void RenderSpriteUV(int texture, vec3_t position, float width, float height, float(*uv)[2], vec3_t light[4], float alpha) {
	BindTexture(texture);

	vec3_t p2;
	VectorTransform(position, CameraMatrix, p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	width *= 0.5f;
	height *= 0.5f;
	vec3_t p[4];
	Vector(x - width, y - height, z, p[0]);
	Vector(x + width, y - height, z, p[1]);
	Vector(x + width, y + height, z, p[2]);
	Vector(x - width, y + height, z, p[3]);

	glBegin(GL_QUADS);
	for (int i = 0; i < 4; i++) {
		glColor4f(light[i][0], light[i][1], light[i][2], alpha);
		glTexCoord2f(uv[i][0], uv[i][1]);
		glVertex3fv(p[i]);
	}
	glEnd();
}

void RenderNumber(vec3_t position, int num, vec3_t color, float alpha, float scale) {
	vec3_t p;
	vec3_t light[4];
	VectorCopy(color, light[0]);
	VectorCopy(color, light[1]);
	VectorCopy(color, light[2]);
	VectorCopy(color, light[3]);

	if (num == -1) {
		float uv[4][2];
		TEXCOORD(uv[0], 0.0f, 32.0f / 32.0f);
		TEXCOORD(uv[1], 32.0f / 256.0f, 32.0f / 32.0f);
		TEXCOORD(uv[2], 32.0f / 256.0f, 17.0f / 32.0f);
		TEXCOORD(uv[3], 0.0f, 17.0f / 32.0f);
		RenderSpriteUV(BITMAP_FONT + 1, p, 45, 20, uv, light, alpha);
	}
	else if (num == -2) {
		RenderSprite(BITMAP_FONT_HIT, p, 32 * scale, 20 * scale, light[0], 0.0f, 0.0f, 0.0f, 27.0f / 32.0f, 15.0f / 16.0f);
	}
	else {
		char text[32];
		itoa(num, text, 10);
		p[0] -= strlen(text) * 5.0f;
		unsigned int length = strlen(text);
		p[0] -= length * scale * 0.125f;
		p[1] -= length * scale * 0.125f;
		for (unsigned int i = 0; i < length; i++) {
			float uv[4][2];
			float u = (float)(text[i] - 48) * 16.0f / 256.0f;
			TEXCOORD(uv[0], u, 16.0f / 32.0f);
			TEXCOORD(uv[1], u + 16.0f / 256.0f, 16.0f / 32.0f);
			TEXCOORD(uv[2], u + 16.0f / 256.0f, 0.0f);
			TEXCOORD(uv[3], u, 0.0f);
			RenderSpriteUV(BITMAP_FONT + 1, p, scale, scale, uv, light, alpha);
			p[0] += scale * 0.5f;
			p[1] += scale * 0.5f;
		}
	}
}

float RenderNumber2D(float x, float y, int num, float width, float height) {
	char text[32];
	itoa(num, text, 10);
	int length = strlen(text);
	x -= width * length / 2;
	for (int i = 0; i < length; i++) {
		float u = (float)(text[i] - 48) * 16.f / 256.f;
		RenderBitmap(BITMAP_FONT + 1, x, y, width, height, u, 0.f, 16.f / 256.f, 16.f / 32.f);
		x += width * 0.7f;
	}
	return x;
}

void BeginBitmap() {
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glViewport(0, 0, WindowWidth, WindowHeight);
	gluPerspective(CameraFOV, (WindowWidth) / ((float)WindowHeight), CameraViewNear, CameraViewFar);

	glLoadIdentity();
	gluOrtho2D(0, WindowWidth, 0, WindowHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	DisableDepthTest();
}

void EndBitmap()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void RenderColor(float x, float y, float width, float height, float alpha, int flag) {
	DisableTexture();

	x = ConvertX(x);
	y = ConvertY(y);
	width = ConvertX(width);
	height = ConvertY(height);

	float p[4][2];
	y = WindowHeight - y;

	p[0][0] = x;
	p[0][1] = y;
	p[1][0] = x;
	p[1][1] = y - height;
	p[2][0] = x + width;
	p[2][1] = y - height;
	p[3][0] = x + width;
	p[3][1] = y;

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; i++) {
		if (alpha > 0.0f) {
			if (flag == 0) {
				glColor4f(1.0f, 1.0f, 1.0f, alpha);
			}
			else if (flag == 1) {
				glColor4f(0.0f, 0.0f, 0.0f, alpha);
			}
		}
		glVertex2f(p[i][0], p[i][1]);
		if (alpha > 0.0f) {
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	glEnd();
}

void EndRenderColor() {
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
}

void RenderColorBitmap(int texture, float x, float y, float width, float height, float u, float v, float uWidth, float vHeight, unsigned int color) {
	x = ConvertX(x);
	y = ConvertY(y);

	width = ConvertX(width);
	height = ConvertY(height);

	BindTexture(texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x;
	p[0][1] = y;
	p[1][0] = x;
	p[1][1] = y - height;
	p[2][0] = x + width;
	p[2][1] = y - height;
	p[3][0] = x + width;
	p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	glBegin(GL_TRIANGLE_FAN);

	for (int i = 0; i < 4; i++) {
		glColor4f(
			static_cast<float>((color & 0xff)) / 255.0f,  // Red
			static_cast<float>((color >> 8) & 0xff) / 255.0f,  // Green
			static_cast<float>((color >> 16) & 0xff) / 255.0f,  // Blue
			static_cast<float>((color >> 24) & 0xff) / 255.0f  // Alpha
		);

		glTexCoord2f(c[i][0], c[i][1]);
		glVertex2f(p[i][0], p[i][1]);

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	}
	glEnd();
}

void RenderBitmap(int texture, float x, float y, float width, float height, float u, float v, float uWidth, float vHeight, bool scale, bool startScale, float alpha) {
	if (startScale) {
		x = ConvertX(x);
		y = ConvertY(y);
	}
	if (scale) {
		width = ConvertX(width);
		height = ConvertY(height);
	}

	BindTexture(texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x;
	p[0][1] = y;
	p[1][0] = x;
	p[1][1] = y - height;
	p[2][0] = x + width;
	p[2][1] = y - height;
	p[3][0] = x + width;
	p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; i++) {
		if (alpha > 0.0f) {
			glColor4f(1.0f, 1.0f, 1.0f, alpha);
		}
		glTexCoord2f(c[i][0], c[i][1]);
		glVertex2f(p[i][0], p[i][1]);
		if (alpha > 0.0f) {
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}
	glEnd();
}

void RenderBitmapRotate(int texture, float x, float y, float width, float height, float rotate, float u, float v, float uWidth, float vHeight) {
	x = ConvertX(x);
	y = ConvertY(y);
	width = ConvertX(width);
	height = ConvertY(height);
	BindTexture(texture);

	vec3_t p[4], p2[4];

	y = WindowHeight - y;

	Vector(-width * 0.5f, height * 0.5f, 0.0f, p[0]);
	Vector(-width * 0.5f, -height * 0.5f, 0.0f, p[1]);
	Vector(width * 0.5f, -height * 0.5f, 0.0f, p[2]);
	Vector(width * 0.5f, height * 0.5f, 0.0f, p[3]);

	vec3_t angle;
	Vector(0.0f, 0.0f, rotate, angle);
	float matrix[3][4];
	AngleMatrix(angle, matrix);

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(c[i][0], c[i][1]);
		VectorRotate(p[i], matrix, p2[i]);
		glVertex2f(p2[i][0] + x, p2[i][1] + y);
	}
	glEnd();
}

void RenderBitRotate(int texture, float x, float y, float width, float height, float rotate) {
	x = ConvertX(x);
	y = ConvertY(y);
	width = ConvertX(width);
	height = ConvertY(height);

	BindTexture(texture);

	vec3_t p[4], p2[4];

	y = WindowHeight - y;

	float cx = (width / 2.0f) - (width - x);
	float cy = (height / 2.0f) - (height - y);

	float ax = -width * 0.5f + cx;
	float bx = width * 0.5f + cx;
	float ay = -height * 0.5f + cy;
	float by = height * 0.5f + cy;

	Vector(ax, by, 0.0f, p[0]);
	Vector(ax, ay, 0.0f, p[1]);
	Vector(bx, ay, 0.0f, p[2]);
	Vector(bx, by, 0.0f, p[3]);

	vec3_t angle;
	Vector(0.0f, 0.0f, rotate, angle);
	float matrix[3][4];
	AngleMatrix(angle, matrix);

	float c[4][2];
	TEXCOORD(c[0], 0.0f, 0.0f);
	TEXCOORD(c[3], 1.0f, 0.0f);
	TEXCOORD(c[2], 1.0f, 1.0f);
	TEXCOORD(c[1], 0.0f, 1.0f);

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(c[i][0], c[i][1]);
		VectorRotate(p[i], matrix, p2[i]);
		glVertex2f(p2[i][0] + WindowWidth / 2.0f, p2[i][1] + WindowHeight / 2.0f);
	}
	glEnd();
}

void RenderPointRotate(int texture, float ix, float iy, float iWidth, float iHeight, float x, float y, float Width, float Height, float rotate, float rotateLoc, float uWidth, float vHeight, int num) {
	int i = 0;
	vec3_t p, p2[4], p3, p4[4], angle;
	float c[4][2], matrix[3][4];

	ix = ConvertX(ix);
	iy = ConvertY(iy);
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	BindTexture(texture);

	y = Height - y;
	iy = Height - iy;

	Vector((ix - (Width * 0.5f)) + ((Width / 2.f) - (Width - x)), (iy - (Height * 0.5f)) + ((Height / 2.f) - (Height - y)), 0.f, p);

	Vector(0.f, 0.f, rotate, angle);
	AngleMatrix(angle, matrix);

	VectorRotate(p, matrix, p3);

	Vector(-(iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[0]);
	Vector(-(iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[1]);
	Vector((iWidth * 0.5f), -(iHeight * 0.5f), 0.f, p2[2]);
	Vector((iWidth * 0.5f), (iHeight * 0.5f), 0.f, p2[3]);

	Vector(0.f, 0.f, rotateLoc, angle);
	AngleMatrix(angle, matrix);

	TEXCOORD(c[0], 0.0f, 0.0f);
	TEXCOORD(c[3], uWidth, 0.0f);
	TEXCOORD(c[2], uWidth, vHeight);
	TEXCOORD(c[1], 0.0f, vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for (i = 0; i < 4; i++) {
		glTexCoord2f(c[i][0], c[i][1]);

		matrix[0][3] = p3[0] + 25;
		matrix[1][3] = p3[1];
		VectorTransform(p2[i], matrix, p4[i]);

		glVertex2f(p4[i][0] + WindowWidth / 2.0f, p4[i][1] + WindowHeight / 2.0f);
	}
	glEnd();

	if (num > -1) {
		float dx, dy;
		dx = p4[0][0] + WindowWidth / 2.0f;
		dy = p4[0][1] + WindowHeight / 2.0f;
		dx = dx * (float)(640.0f / WindowWidth);
		dy = dy * (float)(480.0f / WindowHeight);
		if (num >= 100) {
			g_pNewUIMiniMap->SetBtnPos(num - 100, dx - (iWidth / 2), (480 - dy) - (iHeight / 2), iWidth, iHeight);
		}
		else {
			g_pNewUIMiniMap->SetBtnPos(num, dx, 480 - dy, iWidth / 2, iHeight / 2);
		}
	}
}

void RenderBitmapLocalRotate(int texture, float x, float y, float width, float height, float rotate, float u, float v, float uWidth, float vHeight) {
	BindTexture(texture);

	vec3_t p[4];
	x = ConvertX(x);
	y = ConvertY(y);
	y = WindowHeight - y;
	width = ConvertX(width);
	height = ConvertY(height);

	vec3_t vCenter, vDir;
	Vector(x, y, 0.0f, vCenter);
	Vector(width * 0.5f, -height * 0.5f, 0.0f, vDir);
	p[0][0] = vCenter[0] + (vDir[0]) * cosf(rotate);
	p[0][1] = vCenter[1] + (vDir[1]) * sinf(rotate);
	p[1][0] = vCenter[0] + (vDir[0]) * sinf(rotate);
	p[1][1] = vCenter[1] - (vDir[1]) * cosf(rotate);
	p[2][0] = vCenter[0] - (vDir[0]) * cosf(rotate);
	p[2][1] = vCenter[1] - (vDir[1]) * sinf(rotate);
	p[3][0] = vCenter[0] - (vDir[0]) * sinf(rotate);
	p[3][1] = vCenter[1] + (vDir[1]) * cosf(rotate);

	float c[4][2];
	TEXCOORD(c[0], u, v);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(c[i][0], c[i][1]);
		glVertex2f(p[i][0] + WindowWidth / 2.0f, p[i][1] + WindowHeight / 2.0f);
	}
	glEnd();
}

void RenderBitmapAlpha(int texture, float sx, float sy, float width, float height) {
	EnableAlphaTest();
	BindTexture(texture);

	sy = WindowHeight - sy;
	for (int y = 0; y < 4; y++) {
		for (int x = 0; x < 4; x++) {
			float p[4][2];
			p[0][0] = sx + (x * width) * 0.25f;
			p[0][1] = sy - (y * height) * 0.25f;
			p[1][0] = sx + (x * width) * 0.25f;
			p[1][1] = sy - ((y + 1) * height) * 0.25f;
			p[2][0] = sx + ((x + 1) * width) * 0.25f;
			p[2][1] = sy - ((y + 1) * height) * 0.25f;
			p[3][0] = sx + ((x + 1) * width) * 0.25f;
			p[3][1] = sy - (y * height) * 0.25f;

			float c[4][2];
			TEXCOORD(c[0], x * 0.25f, y * 0.25f);
			TEXCOORD(c[1], x * 0.25f, (y + 1) * 0.25f);
			TEXCOORD(c[2], (x + 1) * 0.25f, (y + 1) * 0.25f);
			TEXCOORD(c[3], (x + 1) * 0.25f, y * 0.25f);

			float alpha[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			if (x == 0) {
				alpha[0] = 0.0f;
				alpha[1] = 0.0f;
			}
			if (x == 3) {
				alpha[2] = 0.0f;
				alpha[3] = 0.0f;
			}
			if (y == 0) {
				alpha[0] = 0.0f;
				alpha[3] = 0.0f;
			}
			if (y == 3) {
				alpha[1] = 0.0f;
				alpha[2] = 0.0f;
			}

			glBegin(GL_TRIANGLE_FAN);
			for (int i = 0; i < 4; i++) {
				glColor4f(1.0f, 1.0f, 1.0f, alpha[i]);
				glTexCoord2f(c[i][0], c[i][1]);
				glVertex2f(p[i][0], p[i][1]);
			}
			glEnd();
		}
	}
}

void RenderBitmapUV(int texture, float x, float y, float width, float height, float u, float v, float uWidth, float vHeight) {
	x = ConvertX(x);
	y = ConvertY(y);
	width = ConvertX(width);
	height = ConvertY(height);
	BindTexture(texture);

	float p[4][2];
	y = WindowHeight - y;
	p[0][0] = x;
	p[0][1] = y;
	p[1][0] = x;
	p[1][1] = y - height;
	p[2][0] = x + width;
	p[2][1] = y - height;
	p[3][0] = x + width;
	p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0], u, v + vHeight);
	TEXCOORD(c[3], u + uWidth, v);
	TEXCOORD(c[2], u + uWidth, v + vHeight);
	TEXCOORD(c[1], u, v + (vHeight - vHeight * 0.25f));

	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < 4; i++) {
		glTexCoord2f(c[i][0], c[i][1]);
		glVertex2f(p[i][0] + WindowWidth / 2.0f, p[i][1] + WindowHeight / 2.0f);
	}
	glEnd();
}

///////////////////////////////////////////////////////////////////////////////
// collision detect util
///////////////////////////////////////////////////////////////////////////////

float absf(float a) {
	return a < 0.0f ? -a : a;
}

float minf(float a, float b) {
	return a < b ? a : b;
}

float maxf(float a, float b) {
	return a > b ? a : b;
}

int InsideTest(float x, float y, float z, int n, float* v1, float* v2, float* v3, float* v4, int flag, float type)
{
	if (type > 0.0f) {
		flag = flag << 3;
	}

	int i;
	vec3_t* vtx[4];
	vtx[0] = (vec3_t*)v1;
	vtx[1] = (vec3_t*)v2;
	vtx[2] = (vec3_t*)v3;
	vtx[3] = (vec3_t*)v4;

	int j = n - 1;
	switch (flag)
	{
	case 1:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[1] - y) * ((*vtx[j])[2] - z) - ((*vtx[j])[1] - y) * ((*vtx[i])[2] - z);
			if (d <= 0.f)
				return false;
		}
		break;
	case 2:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[2] - z) * ((*vtx[j])[0] - x) - ((*vtx[j])[2] - z) * ((*vtx[i])[0] - x);
			if (d <= 0.f)
				return false;
		}
		break;
	case 4:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[0] - x) * ((*vtx[j])[1] - y) - ((*vtx[j])[0] - x) * ((*vtx[i])[1] - y);
			if (d <= 0.f)
				return false;
		}
		break;
	case 8:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[1] - y) * ((*vtx[j])[2] - z) - ((*vtx[j])[1] - y) * ((*vtx[i])[2] - z);
			if (d >= 0.f)
				return false;
		}
		break;
	case 16:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[2] - z) * ((*vtx[j])[0] - x) - ((*vtx[j])[2] - z) * ((*vtx[i])[0] - x);
			if (d >= 0.f)
				return false;
		}
		break;
	case 32:
		for (i = 0; i < n; j = i, i++)
		{
			float d = ((*vtx[i])[0] - x) * ((*vtx[j])[1] - y) - ((*vtx[j])[0] - x) * ((*vtx[i])[1] - y);
			if (d >= 0.f)
				return false;
		}
		break;
	}

	return true;
}

float Distance;

void InitCollisionDetectLineToFace()
{
	Distance = 9999999.f;
}

vec3_t CollisionPosition;

bool CollisionDetectLineToFace(vec3_t Position, vec3_t Target, int Polygon, float* v1, float* v2, float* v3, float* v4, vec3_t Normal, bool Collision)
{
	vec3_t Direction;
	VectorSubtract(Target, Position, Direction);
	float a = DotProduct(Direction, Normal);
	if (a >= 0.f) return false;
	float b = DotProduct(Position, Normal) - DotProduct(v1, Normal);
	float t = -b / a;
	if (t >= 0.f && t <= Distance)
	{
		float X = Direction[0] * t + Position[0];
		float Y = Direction[1] * t + Position[1];
		float Z = Direction[2] * t + Position[2];
		int Count = 0;
		float MIN = minf(minf(absf(Direction[0]), absf(Direction[1])), absf(Direction[2]));
		if (MIN == absf(Direction[0]))
		{
			if ((Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1])) &&
				(Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2]))) Count++;
		}
		else if (MIN == absf(Direction[1]))
		{
			if ((Z >= minf(Position[2], Target[2]) && Z <= maxf(Position[2], Target[2])) &&
				(X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0]))) Count++;
		}
		else
		{
			if ((X >= minf(Position[0], Target[0]) && X <= maxf(Position[0], Target[0])) &&
				(Y >= minf(Position[1], Target[1]) && Y <= maxf(Position[1], Target[1]))) Count++;
		}
		if (Count == 0) return false;
		Count = 0;
		if (Normal[0] <= -0.5f || Normal[0] >= 0.5f)
		{
			Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 1, Normal[0]);
		}
		else if (Normal[1] <= -0.5f || Normal[1] >= 0.5f)
		{
			Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 2, Normal[1]);
		}
		else
		{
			Count += InsideTest(X, Y, Z, Polygon, v1, v2, v3, v4, 4, Normal[2]);
		}
		if (Count == 0) return false;
		if (Collision)
		{
			Distance = t;
			Vector(X, Y, Z, CollisionPosition);
		}
		return true;
	}
	return false;
}

bool ProjectLineBox(vec3_t ax, vec3_t p1, vec3_t p2, OBB_t obb)
{
	float P1 = DotProduct(ax, p1);
	float P2 = DotProduct(ax, p2);

	float mx1 = maxf(P1, P2);
	float mn1 = minf(P1, P2);

	float ST = DotProduct(ax, obb.StartPos);
	float Q1 = DotProduct(ax, obb.XAxis);
	float Q2 = DotProduct(ax, obb.YAxis);
	float Q3 = DotProduct(ax, obb.ZAxis);

	float mx2 = ST;
	float mn2 = ST;

	if (Q1 > 0)	mx2 += Q1; else mn2 += Q1;
	if (Q2 > 0)	mx2 += Q2; else mn2 += Q2;
	if (Q3 > 0) mx2 += Q3; else mn2 += Q3;

	if (mn1 > mx2) return false;
	if (mn2 > mx1) return false;

	return true;
}

bool CollisionDetectLineToOBB(vec3_t p1, vec3_t p2, OBB_t obb)
{
	vec3_t e1;
	vec3_t eq11, eq12, eq13;

	VectorSubtract(p2, p1, e1);

	CrossProduct(e1, obb.XAxis, eq11);
	CrossProduct(e1, obb.YAxis, eq12);
	CrossProduct(e1, obb.ZAxis, eq13);

	if (!ProjectLineBox(eq11, p1, p2, obb)) return false;
	if (!ProjectLineBox(eq12, p1, p2, obb)) return false;
	if (!ProjectLineBox(eq13, p1, p2, obb)) return false;

	if (!ProjectLineBox(obb.XAxis, p1, p2, obb)) return false;
	if (!ProjectLineBox(obb.YAxis, p1, p2, obb)) return false;
	if (!ProjectLineBox(obb.ZAxis, p1, p2, obb)) return false;

	return true;
}