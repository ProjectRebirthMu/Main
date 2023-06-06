//////////////////////////////////////////////////////////////////////
// ShadowVolume.cpp: implementation of the CShadowVolume class.
// REWORK
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBmd.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ShadowVolume.h"
#include "ZzzLodTerrain.h"
#include "zzzTexture.h"
#include "BaseCls.h"
#include "ZzzCharacter.h"

CQueue<CShadowVolume*> m_qSV;

void InsertShadowVolume(CShadowVolume* psv)
{
	if (psv != nullptr)
	{
		m_qSV.Insert(psv);
	}
}

void RenderShadowVolumesAsFrame()
{
	glPolygonMode(GL_FRONT, GL_LINE);
	glDepthMask(true);
	DisableAlphaBlend();
	DisableTexture();
	vec3_t vLight = { 0.4f, 0.f, 0.f };
	glColor3fv(vLight);
	while (m_qSV.GetCount() > 0)
	{
		CShadowVolume* psv = m_qSV.Remove();
		if (psv != nullptr)
		{
			psv->RenderAsFrame();
			psv->Destroy();
			delete psv;
		}
	}

	glPolygonMode(GL_FRONT, GL_FILL);
}

void ShadeWithShadowVolumes()
{
	DisableAlphaBlend();
	DisableDepthMask();
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
	while (m_qSV.GetCount() > 0)
	{
		CShadowVolume* psv = m_qSV.Remove();
		if (psv != nullptr)
		{
			psv->Shade();
			psv->Destroy();
			delete psv;
		}
	}

	glFrontFace(GL_CCW);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDisable(GL_STENCIL_TEST);
	EnableDepthMask();
}

void RenderShadowToScreen()
{
	DisableDepthTest();
	DisableDepthMask();
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_LEQUAL, 0x1, 0xFFFFFFFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glDepthFunc(GL_ALWAYS);

	EnableAlphaBlendMinus();
	DisableTexture();
	vec3_t vLight = { .7f, 0.7f, 0.5f };
	float p[4][2];
	float Width = (float)WindowWidth;
	float Height = (float)WindowHeight;
	p[0][0] = 0.f;
	p[0][1] = 0.f;
	p[1][0] = 0.f;
	p[1][1] = Height;
	p[2][0] = 0.f + Width;
	p[2][1] = Height;
	p[3][0] = 0.f + Width;
	p[3][1] = 0.f;
	glBegin(GL_TRIANGLE_FAN);
	glColor3fv(vLight);
	for (int i = 0; i < 4; i++)
	{
		glVertex2f(p[i][0], p[i][1]);
	}
	glEnd();
	glDepthFunc(GL_LESS);
	glDisable(GL_STENCIL_TEST);
	EnableDepthMask();
}

CShadowVolume::CShadowVolume()
{
	Clear();
}

CShadowVolume::~CShadowVolume()
{
	// Empty destructor
}

void CShadowVolume::Clear()
{
	m_nNumVertices = 0;
	delete[] m_pVertices;
	m_pVertices = nullptr;
	m_iNumEdge = 0;
	delete[] m_pEdges;
	m_pEdges = nullptr;
}

BOOL CShadowVolume::GetReadyToCreate(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o, bool SkipTga)
{
	if (o->Alpha < 0.01f)
	{
		return FALSE;
	}
	short nHiddenMesh = o->HiddenMesh;
	short nBlendMesh = o->BlendMesh;
	if (nHiddenMesh == -2 || nBlendMesh == -2)
	{
		return FALSE;
	}
	int iNumTriangles = 0;
	for (int i = 0; i < b->NumMeshs; ++i)
	{
		if (nHiddenMesh == i || nBlendMesh == i)
		{
			continue;
		}

		if (Bitmaps[b->IndexTexture[i]].Components == 4)
		{
			if (SkipTga) continue;
		}
		iNumTriangles += max(0, b->Meshs[i].NumTriangles);
	}
	m_iNumEdge = 0;
	delete[] m_pEdges;
	m_pEdges = new St_Edges[iNumTriangles * 3];

	for (int i = 0; i < b->NumMeshs; ++i)
	{
		if (nHiddenMesh == i || nBlendMesh == i)
		{
			continue;
		}

		bool Tga = false;
		if (Bitmaps[b->IndexTexture[i]].Components == 4)
		{
			Tga = true;
			if (SkipTga) continue;
		}
		DeterminateSilhouette(i, ppVertexTransformed, b->Meshs[i].NumTriangles, b->Meshs[i].Triangles, Tga);
	}

	return TRUE;
}

void CShadowVolume::Create(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], BMD* b, OBJECT* o, bool SkipTga)
{
	m_vLight[0] = -1.f;
	m_vLight[1] = 0.03f;
	m_vLight[2] = -1.f;
	VectorNormalize(m_vLight);
	if (!GetReadyToCreate(ppVertexTransformed, b, o, SkipTga))
	{
		return;
	}

	GenerateSidePolygon(ppVertexTransformed);
	delete[] m_pEdges;
	m_pEdges = nullptr;
}

void CShadowVolume::Destroy()
{
	if (m_pVertices)
	{
		delete[] m_pVertices;
		m_pVertices = nullptr;
	}
}

void CShadowVolume::AddEdge(short nV1, short nV2, short nMesh)
{
	for (int i = 0; i < m_iNumEdge; ++i)
	{
		if (m_pEdges[i].m_nMesh == nMesh)
		{
			if (m_pEdges[i].m_nVertexIndex[0] == nV2 && m_pEdges[i].m_nVertexIndex[1] == nV1)
			{
				if (m_iNumEdge > 1)
				{
					m_pEdges[i].m_nVertexIndex[0] = m_pEdges[m_iNumEdge - 1].m_nVertexIndex[0];
					m_pEdges[i].m_nVertexIndex[1] = m_pEdges[m_iNumEdge - 1].m_nVertexIndex[1];
					m_pEdges[i].m_nMesh = m_pEdges[m_iNumEdge - 1].m_nMesh;
					m_iNumEdge--;
				}
				return;
			}
		}
	}
	m_pEdges[m_iNumEdge].m_nVertexIndex[0] = nV1;
	m_pEdges[m_iNumEdge].m_nVertexIndex[1] = nV2;
	m_pEdges[m_iNumEdge].m_nMesh = nMesh;
	m_iNumEdge++;
}

void CShadowVolume::AddEdgeFast(short nV1, short nV2, short nMesh, int iTriangle, int Edge, Triangle_t* pTriangles)
{
	Triangle_t* pTriangle = &pTriangles[iTriangle];
	short EdgeTriangleIndex = pTriangle->EdgeTriangleIndex[Edge];
	if (EdgeTriangleIndex == -1 || pTriangles[EdgeTriangleIndex].Front == false)
	{
		m_pEdges[m_iNumEdge].m_nVertexIndex[0] = nV1;
		m_pEdges[m_iNumEdge].m_nVertexIndex[1] = nV2;
		m_pEdges[m_iNumEdge].m_nMesh = nMesh;
		m_pEdges[m_iNumEdge].m_nNormalIndex[0] = pTriangle->NormalIndex[Edge];
		m_pEdges[m_iNumEdge].m_nNormalIndex[1] = pTriangle->NormalIndex[(Edge + 1) % 3];
		m_iNumEdge++;
	}
}

void CShadowVolume::DeterminateSilhouette(short nMesh, vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES], short nNumTriangles, Triangle_t* pTriangles, bool Tga)
{
	for (int iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
	{
		Triangle_t* pTriangle = &pTriangles[iTriangle];
		short* pnVertexIndex = pTriangle->VertexIndex;
		vec3_t* pVertex[3];
		pVertex[0] = &ppVertexTransformed[nMesh][pnVertexIndex[0]];
		pVertex[1] = &ppVertexTransformed[nMesh][pnVertexIndex[1]];
		pVertex[2] = &ppVertexTransformed[nMesh][pnVertexIndex[2]];
		vec3_t Normal;
		FaceNormalize(*pVertex[0], *pVertex[1], *pVertex[2], Normal);
		pTriangle->Front = (DotProduct(Normal, m_vLight) <= 0.f);
	}

	for (int iTriangle = 0; iTriangle < nNumTriangles; ++iTriangle)
	{
		Triangle_t* pTriangle = &pTriangles[iTriangle];

		if (pTriangle->Front)
		{
			short* pnVertexIndex = pTriangle->VertexIndex;
			AddEdgeFast(pnVertexIndex[0], pnVertexIndex[1], nMesh, iTriangle, 0, pTriangles);
			AddEdgeFast(pnVertexIndex[1], pnVertexIndex[2], nMesh, iTriangle, 1, pTriangles);
			AddEdgeFast(pnVertexIndex[2], pnVertexIndex[0], nMesh, iTriangle, 2, pTriangles);
		}
	}
}

#define GROUND_HEIGHT 22.5f

void CShadowVolume::GenerateSidePolygon(vec3_t ppVertexTransformed[MAX_MESH][MAX_VERTICES])
{
	m_nNumVertices = 0;
	m_pVertices = new vec3_t[m_iNumEdge * 6];
	vec3_t Vertex[4];
	for (int i = 0; i < m_iNumEdge; ++i)
	{
		VectorCopy(ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[0]], Vertex[0]);
		VectorCopy(ppVertexTransformed[m_pEdges[i].m_nMesh][m_pEdges[i].m_nVertexIndex[1]], Vertex[1]);

		float fLength = (max(GROUND_HEIGHT, Vertex[0][2]) / -m_vLight[2]);
		VectorMA(Vertex[0], fLength, m_vLight, Vertex[2]);

		fLength = (max(GROUND_HEIGHT, Vertex[1][2]) / -m_vLight[2]);
		VectorMA(Vertex[1], fLength, m_vLight, Vertex[3]);

		VectorCopy(Vertex[0], m_pVertices[m_nNumVertices]);
		m_nNumVertices++;
		VectorCopy(Vertex[2], m_pVertices[m_nNumVertices]);
		m_nNumVertices++;
		VectorCopy(Vertex[1], m_pVertices[m_nNumVertices]);
		m_nNumVertices++;
		VectorCopy(Vertex[1], m_pVertices[m_nNumVertices]);
		m_nNumVertices++;
		VectorCopy(Vertex[2], m_pVertices[m_nNumVertices]);
		m_nNumVertices++;
		VectorCopy(Vertex[3], m_pVertices[m_nNumVertices]);
		m_nNumVertices++;
	}
}

void CShadowVolume::RenderAsFrame()
{
	RenderShadowVolume();
}

void CShadowVolume::RenderShadowVolume()
{
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < m_nNumVertices; ++i)
	{
		glVertex3fv(m_pVertices[i]);
	}

	glEnd();
}

void CShadowVolume::Shade()
{
	glFrontFace(GL_CCW);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	RenderShadowVolume();
	glFrontFace(GL_CW);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	RenderShadowVolume();
}
