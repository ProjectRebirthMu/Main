// SMD2BMD
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzBMD.h"
#include "SMD.h"

struct BoneFixup_t
{
	float m[3][4];
	float im[3][4];
	vec3_t WorldOrg;
};

BoneFixup_t BoneFixup[NODE_MAX];

void FixupSMD()
{
	Skeleton_t* s = &NodeGroup.Skeleton;
	NodeGroup_t* ng = &NodeGroup;

	for (int i = 0; i < ng->NodeNum; i++)
	{
		Node_t* n = &ng->Node[i];

		vec3_t Angle;
		Angle[0] = s->Rotation[i][0] * (180.f / Q_PI);
		Angle[1] = s->Rotation[i][1] * (180.f / Q_PI);
		Angle[2] = s->Rotation[i][2] * (180.f / Q_PI);

		if (n->Parent == -1)
		{
			AngleMatrix(Angle, BoneFixup[i].m);
			AngleIMatrix(Angle, BoneFixup[i].im);
			VectorCopy(s->Position[i], BoneFixup[i].WorldOrg);
		}
		else
		{
			float m[3][4];
			AngleMatrix(Angle, m);
			R_ConcatTransforms(BoneFixup[n->Parent].m, m, BoneFixup[i].m);
			AngleIMatrix(Angle, m);
			R_ConcatTransforms(m, BoneFixup[n->Parent].im, BoneFixup[i].im);

			vec3_t p;
			VectorTransform(s->Position[i], BoneFixup[n->Parent].m, p);
			VectorAdd(p, BoneFixup[n->Parent].WorldOrg, BoneFixup[i].WorldOrg);
		}
	}

	TriangleGroup_t* tg = &TriangleGroup;

	for (int i = 0; i < tg->TriangleNum; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			SMDVertex_t* v = &tg->Vertex[i][j];
			vec3_t p;

			VectorSubtract(v->Position, BoneFixup[v->Node].WorldOrg, p);
			VectorTransform(p, BoneFixup[v->Node].im, v->Position);

			VectorCopy(v->Normal, p);
			VectorTransform(p, BoneFixup[v->Node].im, v->Normal);
			VectorNormalize(v->Normal);
		}
	}

	SMDMeshGroup_t* mg = &MeshGroup;
	mg->MeshNum = 0;

	for (int i = 0; i < MESH_MAX; i++)
	{
		SMDMesh_t* m = &mg->Mesh[i];
		m->VertexNum = 0;
		m->NormalNum = 0;
		m->TexCoordNum = 0;
		m->TriangleNum = 0;
	}

	for (int i = 0; i < tg->TriangleNum; i++)
	{
		int MeshNum = 0;
		for (int k = 0; k < mg->MeshNum; k++)
		{
			if (strcmp(tg->TextureName[i], mg->Texture[k].FileName) == 0)
			{
				MeshNum = k;
				break;
			}
		}

		if (MeshNum == 0)
		{
			MeshNum = mg->MeshNum;
			mg->Mesh[MeshNum].Texture =

				mg->MeshNum;
			strcpy(mg->Texture[MeshNum].FileName, tg->TextureName[i]);
			mg->MeshNum++;
		}

		SMDMesh_t* m = &mg->Mesh[MeshNum];

		for (int j = 0; j < 3; j++)
		{
			SMDVertex_t* v = &tg->Vertex[i][j];
			int k = m->VertexNum - 1;
			bool vertexFound = false;

			while (k >= 0)
			{
				Vertex_t* v2 = &m->Vertex[k];
				if (v->Position[0] == v2->Position[0] &&
					v->Position[1] == v2->Position[1] &&
					v->Position[2] == v2->Position[2])
				{
					m->VertexList[m->TriangleNum][j] = k;
					vertexFound = true;
					break;
				}
				k--;
			}

			if (!vertexFound)
			{
				m->VertexList[m->TriangleNum][j] = m->VertexNum;
				m->Vertex[m->VertexNum].Node = tg->Vertex[i][j].Node;
				m->Vertex[m->VertexNum].Position[0] = tg->Vertex[i][j].Position[0];
				m->Vertex[m->VertexNum].Position[1] = tg->Vertex[i][j].Position[1];
				m->Vertex[m->VertexNum].Position[2] = tg->Vertex[i][j].Position[2];
				m->VertexNum++;
			}

			k = m->NormalNum - 1;
			bool normalFound = false;

			while (k >= 0)
			{
				Normal_t* n = &m->Normal[k];
				if (v->Normal[0] == n->Normal[0] &&
					v->Normal[1] == n->Normal[1] &&
					v->Normal[2] == n->Normal[2])
				{
					m->NormalList[m->TriangleNum][j] = k;
					normalFound = true;
					break;
				}
				k--;
			}

			if (!normalFound)
			{
				m->NormalList[m->TriangleNum][j] = m->NormalNum;
				m->Normal[m->NormalNum].Node = tg->Vertex[i][j].Node;
				m->Normal[m->NormalNum].Normal[0] = tg->Vertex[i][j].Normal[0];
				m->Normal[m->NormalNum].Normal[1] = tg->Vertex[i][j].Normal[1];
				m->Normal[m->NormalNum].Normal[2] = tg->Vertex[i][j].Normal[2];
				m->Normal[m->NormalNum].BindVertex = m->VertexList[m->TriangleNum][j];
				m->NormalNum++;
			}

			k = m->TexCoordNum - 1;
			bool texCoordFound = false;

			while (k >= 0)
			{
				TexCoord_t* t = &m->TexCoord[k];
				if (v->TexCoordU == t->TexCoordU &&
					v->TexCoordV == t->TexCoordV)
				{
					m->TexCoordList[m->TriangleNum][j] = k;
					texCoordFound = true;
					break;
				}
				k--;
			}

			if (!texCoordFound)
			{
				m->TexCoordList[m->TriangleNum][j] = m->TexCoordNum;
				m->TexCoord[m->TexCoordNum].TexCoordU

					= tg->Vertex[i][j].TexCoordU;
				m->TexCoord[m->TexCoordNum].TexCoordV = tg->Vertex[i][j].TexCoordV;
				m->TexCoordNum++;
			}
		}

		m->Polygon[m->TriangleNum] = 3;
		m->TriangleNum++;
	}
}

void SMD2BMDModel(int ID, int Actions)
{
	int i, j;
	BMD* bmd = &Models[ID];

	bmd->NumBones = NodeGroup.NodeNum;
	bmd->NumMeshs = MeshGroup.MeshNum;
	bmd->Bones = new Bone_t[bmd->NumBones];
	bmd->Meshs = new Mesh_t[bmd->NumMeshs];
	bmd->Textures = new Texture_t[bmd->NumMeshs];
	bmd->IndexTexture = new GLuint[bmd->NumMeshs];
	bmd->NumLightMaps = 0;

	for (i = 0; i < bmd->NumMeshs; i++)
	{
		SMDMesh_t* sm = &MeshGroup.Mesh[i];
		Mesh_t* m = &bmd->Meshs[i];

		m->NoneBlendMesh = false;
		m->Texture = sm->Texture;

		m->NumVertices = sm->VertexNum;
		m->Vertices = new Vertex_t[m->NumVertices];
		memcpy(m->Vertices, sm->Vertex, sizeof(Vertex_t) * m->NumVertices);

		m->NumNormals = sm->NormalNum;
		m->Normals = new Normal_t[m->NumNormals];
		memcpy(m->Normals, sm->Normal, sizeof(Normal_t) * m->NumNormals);

		m->NumTexCoords = sm->TexCoordNum;
		m->TexCoords = new TexCoord_t[m->NumTexCoords];
		memcpy(m->TexCoords, sm->TexCoord, sizeof(TexCoord_t) * m->NumTexCoords);

		memcpy(bmd->Textures[i].FileName, MeshGroup.Texture[i].FileName, 32);

		TextureScriptParsing TSParsing;

		if (TSParsing.parsingTScript(bmd->Textures[i].FileName))
		{
			m->m_csTScript = new TextureScript;
			m->m_csTScript->setScript((TextureScript&)TSParsing);
		}
		else
		{
			m->m_csTScript = NULL;
		}

		m->NumTriangles = sm->TriangleNum;
		m->Triangles = new Triangle_t[m->NumTriangles];
		for (j = 0; j < m->NumTriangles; j++)
		{
			Triangle_t* tp = &m->Triangles[j];
			tp->Polygon = sm->Polygon[j];
			for (int k = 0; k < tp->Polygon; k++)
			{
				tp->VertexIndex[k] = sm->VertexList[j][k];
				tp->NormalIndex[k] = sm->NormalList[j][k];
				tp->TexCoordIndex[k] = sm->TexCoordList[j][k];
			}
		}
	}

	bmd->Actions = new Action_t[Actions];
	for (i = 0; i < bmd->NumBones; i++)
	{
		Node_t* n = &NodeGroup.Node[i];
		Bone_t* b = &bmd->Bones[i];

		strcpy(b->Name, n->Name);
		b->Parent = n->Parent;

		bmd->Bones[i].BoneMatrixes = new BoneMatrix_t[Actions];
	}

	bmd->Init(true);
	bmd->NumActions = 0;
}

void SMD2BMDAnimation(int ID, bool LockPosition)
{
	int i, j;
	BMD* bmd = &Models[ID];

	for (i = 0; i < bmd->NumBones; i++)
	{
		Bone_t* b = &bmd->Bones[i];

		{
			BoneMatrix_t* bm = &b->BoneMatrixes[bmd->NumActions];
			bm->Position = new vec3_t[SkeletonGroup.TimeNum]();
			bm->Rotation = new vec3_t[SkeletonGroup.TimeNum]();
			bm->Quaternion = new vec4_t[SkeletonGroup.TimeNum]();
			for (j = 0; j < SkeletonGroup.TimeNum; j++)
			{
				VectorCopy(SkeletonGroup.Skeleton[j].Position[i], bm->Position[j]);
				VectorCopy(SkeletonGroup.Skeleton[j].Rotation[i], bm->Rotation[j]);
				AngleQuaternion(bm->Rotation[j], bm->Quaternion[j]);
			}
		}
	}

	Action_t* a = &bmd->Actions[bmd->NumActions];
	a->Loop = false;
	a->LockPositions = LockPosition;
	a->NumAnimationKeys = SkeletonGroup.TimeNum;
	a->PlaySpeed = 0.3f;

	a->Positions = new vec3_t[a->NumAnimationKeys]();
	Bone_t* b = &bmd->Bones[0];
	for (i = 0; i < a->NumAnimationKeys; i++)
	{
		BoneMatrix_t* bm = &b->BoneMatrixes[bmd->NumActions];
		j = i + 1;
		if (j > a->NumAnimationKeys - 1) j = a->NumAnimationKeys - 1;
		VectorSubtract(bm->Position[j], bm->Position[i], a->Positions[i]);
	}

	bmd->NumActions++;
}