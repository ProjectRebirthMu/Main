// Binary Model Data 2.0
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "zzzlodterrain.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "SMD.h"
#include "zzzEffect.h"
#include "MapManager.h"
#include "./Utilities/Log/muConsoleDebug.h"
#include "GMBattleCastle.h"
#include "UIMng.h"
#include "CameraMove.h"

BMD *Models;
BMD *ModelsDump;

vec4_t BoneQuaternion[MAX_BONES];
short  BoundingVertices[MAX_BONES];
vec3_t BoundingMin[MAX_BONES];
vec3_t BoundingMax[MAX_BONES];

float  BoneTransform[MAX_BONES][3][4];

vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
vec3_t NormalTransform[MAX_MESH][MAX_VERTICES];
float  IntensityTransform[MAX_MESH][MAX_VERTICES];
vec3_t LightTransform[MAX_MESH][MAX_VERTICES];

#include <cstdint>
#include <array>

constexpr uint16_t ShadowBufferWidth = 256;
constexpr uint16_t ShadowBufferHeight = 256;

std::array<std::array<uint8_t, ShadowBufferWidth>, ShadowBufferHeight> ShadowBuffer;

extern int MouseX;
extern int MouseY;
extern bool MouseLButton;

bool  StopMotion = false;
float ParentMatrix[3][4];

#include <glm/glm.hpp>

static glm::vec3 LightVector(0.0f, -0.1f, -0.8f);
static glm::vec3 LightVector2(0.0f, -0.5f, -0.8f);

#ifdef PBG_ADD_NEWCHAR_MONK_ANI
void BMD::Animation(float(*BoneMatrix)[3][4], float AnimationFrame, float PriorFrame, unsigned short PriorAction, vec3_t Angle, vec3_t HeadAngle, bool Parent, bool Translate)
#else //PBG_ADD_NEWCHAR_MONK_ANI
void BMD::Animation(float(*BoneMatrix)[3][4], float AnimationFrame, float PriorFrame, unsigned char PriorAction, vec3_t Angle, vec3_t HeadAngle, bool Parent, bool Translate)
#endif //PBG_ADD_NEWCHAR_MONK_ANI
{
	if (NumActions <= 0)
		return;

	if (PriorAction >= NumActions)
		PriorAction = 0;
	if (CurrentAction >= NumActions)
		CurrentAction = 0;
	VectorCopy(Angle, BodyAngle);

	CurrentAnimation = AnimationFrame;
	CurrentAnimationFrame = static_cast<int>(AnimationFrame);

	float s1 = CurrentAnimation - CurrentAnimationFrame;
	float s2 = 1.f - s1;

	int PriorAnimationFrame = static_cast<int>(PriorFrame);
	if (NumActions > 0)
	{
		if (PriorAnimationFrame < 0)
			PriorAnimationFrame = 0;
		if (CurrentAnimationFrame < 0)
			CurrentAnimationFrame = 0;
		if (PriorAnimationFrame >= Actions[PriorAction].NumAnimationKeys)
			PriorAnimationFrame = 0;
		if (CurrentAnimationFrame >= Actions[CurrentAction].NumAnimationKeys)
			CurrentAnimationFrame = 0;
	}

	// Bones
	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		if (b->Dummy)
			continue;

		BoneMatrix_t* bm1 = &b->BoneMatrixes[PriorAction];
		BoneMatrix_t* bm2 = &b->BoneMatrixes[CurrentAction];
		vec4_t q1, q2;

		if (i == BoneHead)
		{
			vec3_t Angle1, Angle2;
			VectorCopy(bm1->Rotation[PriorAnimationFrame], Angle1);
			VectorCopy(bm2->Rotation[CurrentAnimationFrame], Angle2);

			float HeadAngleX = HeadAngle[0] / (180.f / Q_PI);
			float HeadAngleY = HeadAngle[1] / (180.f / Q_PI);
			Angle1[0] -= HeadAngleX;
			Angle2[0] -= HeadAngleX;
			Angle1[2] -= HeadAngleY;
			Angle2[2] -= HeadAngleY;
			AngleQuaternion(Angle1, q1);
			AngleQuaternion(Angle2, q2);
		}
		else
		{
			QuaternionCopy(bm1->Quaternion[PriorAnimationFrame], q1);
			QuaternionCopy(bm2->Quaternion[CurrentAnimationFrame], q2);
		}

		if (!QuaternionCompare(q1, q2))
			QuaternionSlerp(q1, q2, s1, BoneQuaternion[i]);
		else
			QuaternionCopy(q1, BoneQuaternion[i]);

		float Matrix[3][4];
		QuaternionMatrix(BoneQuaternion[i], Matrix);
		float* Position1 = bm1->Position[PriorAnimationFrame];
		float* Position2 = bm2->Position[CurrentAnimationFrame];

		if (i == 0 && (Actions[PriorAction].LockPositions || Actions[CurrentAction].LockPositions))
		{
			Matrix[0][3] = bm2->Position[0][0];
			Matrix[1][3] = bm2->Position[

				0][1];
				Matrix[2][3] = Position1[2] * s2 + Position2[2] * s1 + BodyHeight;
		}
		else
		{
			Matrix[0][3] = Position1[0] * s2 + Position2[0] * s1;
			Matrix[1][3] = Position1[1] * s2 + Position2[1] * s1;
			Matrix[2][3] = Position1[2] * s2 + Position2[2] * s1;
		}

		if (b->Parent == -1)
		{
			if (!Parent)
			{
				AngleMatrix(BodyAngle, ParentMatrix);
				if (Translate)
				{
					for (int y = 0; y < 3; ++y)
					{
						for (int x = 0; x < 3; ++x)
						{
							ParentMatrix[y][x] *= BodyScale;
						}
					}

					ParentMatrix[0][3] = BodyOrigin[0];
					ParentMatrix[1][3] = BodyOrigin[1];
					ParentMatrix[2][3] = BodyOrigin[2];
				}
			}
			R_ConcatTransforms(ParentMatrix, Matrix, BoneMatrix[i]);
		}
		else
		{
			R_ConcatTransforms(BoneMatrix[b->Parent], Matrix, BoneMatrix[i]);
		}
	}
}

extern int SceneFlag;
extern int EditFlag;

bool HighLight = true;
float BoneScale = 1.f;

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
void BMD::Transform(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate, float _Scale)
#else //PBG_ADD_NEWCHAR_MONK_ITEM
void BMD::Transform(float(*BoneMatrix)[3][4], vec3_t BoundingBoxMin, vec3_t BoundingBoxMax, OBB_t* OBB, bool Translate)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
{
	vec3_t LightPosition;

	if (LightEnable)
	{
		vec3_t Position;
		float Matrix[3][4];

		if (HighLight)
		{
			Vector(1.3f, 0.f, 2.f, Position);
		}
		else if (gMapManager.InBattleCastle())
		{
			Vector(0.5f, -1.f, 1.f, Position);
			Vector(0.f, 0.f, -45.f, ShadowAngle);
		}
		else
		{
			Vector(0.f, -1.5f, 0.f, Position);
		}

		AngleMatrix(ShadowAngle, Matrix);
		VectorIRotate(Position, Matrix, LightPosition);
	}

	vec3_t BoundingMin;
	vec3_t BoundingMax;

#ifndef _DEBUG
	if (EditFlag == 2)
#endif
	{
		Vector(999999.f, 999999.f, 999999.f, BoundingMin);
		Vector(-999999.f, -999999.f, -999999.f, BoundingMax);
	}

	for (int i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];

		for (int j = 0; j < m->NumVertices; j++)
		{
			Vertex_t* v = &m->Vertices[j];
			float* vp = VertexTransform[i][j];

			if (BoneScale == 1.f)
			{
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				if (_Scale)
				{
					vec3_t Position;
					VectorCopy(v->Position, Position);
					VectorScale(Position, _Scale, Position);
					VectorTransform(Position, BoneMatrix[v->Node], vp);
				}
				else
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					VectorTransform(v->Position, BoneMatrix[v->Node], vp);

				if (Translate)
					VectorScale(vp, BodyScale, vp);
			}
			else
			{
				VectorRotate(v->Position, BoneMatrix[v->Node], vp);
				vp[0] = vp[0] * BoneScale + BoneMatrix[v->Node][0][3];
				vp[1] = vp[1] * BoneScale + BoneMatrix[v->Node][1][3];
				vp[2] = vp[2] * BoneScale + BoneMatrix[v->Node][2][3];

				if (Translate)
					VectorScale(vp, BodyScale, vp);
			}

#ifndef _DEBUG
			if (EditFlag == 2)
#endif
			{
				for (int k = 0; k < 3; k++)
				{
					BoundingMin[k] = std::min(BoundingMin[k], vp[k]);
					BoundingMax[k] = std::max(BoundingMax[k], vp[k]);
				}
			}

			if (Translate)
				VectorAdd(vp, BodyOrigin, vp);
		}

		for (int j = 0; j < m->NumNormals; j++)
		{
			Normal_t* sn = &m->Normals[j];
			float* tn = NormalTransform[i][j];
			VectorRotate(sn->Normal, BoneMatrix[sn->Node], tn);

			if (LightEnable)
			{
				float Luminosity = DotProduct(tn, LightPosition) * 0.8f + 0.4f;
				Luminosity = std::max(Luminosity, 0.2f);
				IntensityTransform[i][j] = Luminosity;
			}
		}
	}

	if (EditFlag == 2)
	{
		VectorCopy(BoundingMin, OBB->StartPos);
		OBB->XAxis[0] = (BoundingMax[0] - BoundingMin[0]);
		OBB->YAxis[1] = (BoundingMax[1] - BoundingMin[1]);
		OBB->ZAxis[2] = (BoundingMax[2] - BoundingMin[2]);
	}
	else
	{
		VectorCopy(BoundingBoxMin, OBB->StartPos);
		OBB->XAxis[0] = (BoundingBoxMax[0] - BoundingBoxMin[0]);
		OBB->YAxis[1] = (BoundingBoxMax[1] - BoundingBoxMin[1]);
		OBB->ZAxis[2] = (BoundingBoxMax[2] - BoundingBoxMin[2]);
	}

	float fTransformedSize = std::max({ BoundingMax[0] - BoundingMin[0], BoundingMax[1] - BoundingMin[1], BoundingMax[2] - BoundingMin[2] });
	VectorAdd(OBB->StartPos, BodyOrigin, OBB->StartPos);
	OBB->XAxis[1] = 0.f;
	OBB->XAxis[2] = 0.f;
	OBB->YAxis[0] = 0.f;
	OBB->YAxis[2] = 0.f;
	OBB->ZAxis[0] = 0.f;
	OBB->ZAxis[1] = 0.f;
}

void BMD::TransformByObjectBone(vec3_t vResultPosition, OBJECT * pObject, int iBoneNumber, vec3_t vRelativePosition)
{
	if (iBoneNumber < 0 || iBoneNumber >= NumBones)
	{
		assert(!"Bone number error");
		return;
	}
	if (pObject == NULL)
	{
		assert(!"Empty Bone");
		return;
	}

	float (*TransformMatrix)[4];
	if (pObject->BoneTransform != NULL)
	{
		TransformMatrix = pObject->BoneTransform[iBoneNumber];
	}
	else
	{
		TransformMatrix = BoneTransform[iBoneNumber];
	}

	vec3_t vTemp;
	if (vRelativePosition == NULL)
	{
		vTemp[0] = TransformMatrix[0][3];
		vTemp[1] = TransformMatrix[1][3];
		vTemp[2] = TransformMatrix[2][3];
	}
	else
	{
		VectorTransform(vRelativePosition, TransformMatrix, vTemp);
	}
	VectorScale(vTemp, BodyScale, vTemp);
	VectorAdd(vTemp, pObject->Position, vResultPosition);
}

void BMD::TransformByBoneMatrix(vec3_t vResultPosition, float (*BoneMatrix)[4], vec3_t vWorldPosition, vec3_t vRelativePosition)
{
	if (BoneMatrix == NULL)
	{
		assert(!"Empty Matrix");
		return;
	}

	vec3_t vTemp;
	if (vRelativePosition == NULL)
	{
		vTemp[0] = BoneMatrix[0][3];
		vTemp[1] = BoneMatrix[1][3];
		vTemp[2] = BoneMatrix[2][3];
	}
	else
	{
		VectorTransform(vRelativePosition, BoneMatrix, vTemp);
	}
	if (vWorldPosition != NULL)
	{
		VectorScale(vTemp, BodyScale, vTemp);
		VectorAdd(vTemp, vWorldPosition, vResultPosition);
	}
	else
	{
		VectorScale(vTemp, BodyScale, vResultPosition);
	}
}

void BMD::TransformPosition(float (*Matrix)[4],vec3_t Position,vec3_t WorldPosition,bool Translate)
{
	if(Translate)
	{
		vec3_t p;
		VectorTransform(Position,Matrix,p);
		VectorScale(p,BodyScale,p);
		VectorAdd(p,BodyOrigin,WorldPosition);
	}
	else
    	VectorTransform(Position,Matrix,WorldPosition);
}

void BMD::RotationPosition(float (*Matrix)[4],vec3_t Position,vec3_t WorldPosition)
{
	vec3_t p;
	VectorRotate(Position,Matrix,p);
	VectorScale(p,BodyScale,WorldPosition);
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<4;j++)
		{
			ParentMatrix[i][j] = Matrix[i][j];
		}
	}
}

#ifdef PBG_ADD_NEWCHAR_MONK_ANI
bool BMD::PlayAnimation(float *AnimationFrame,float *PriorAnimationFrame,unsigned short *PriorAction,float Speed,vec3_t Origin,vec3_t Angle)
#else //PBG_ADD_NEWCHAR_MONK_ANI
bool BMD::PlayAnimation(float *AnimationFrame,float *PriorAnimationFrame,unsigned char *PriorAction,float Speed,vec3_t Origin,vec3_t Angle)
#endif //PBG_ADD_NEWCHAR_MONK_ANI
{
	bool Loop = true;

	if(AnimationFrame == NULL || PriorAnimationFrame == NULL || PriorAction == NULL || (NumActions > 0 && CurrentAction >= NumActions))
	{
		return Loop;
	}

	if(NumActions == 0 || Actions[CurrentAction].NumAnimationKeys <= 1) 
	{
		return Loop;
	}

	int Temp = (int)*AnimationFrame;
   	*AnimationFrame += Speed;
	if(Temp != (int)*AnimationFrame) 
	{
		*PriorAction = CurrentAction;
		*PriorAnimationFrame = (float)Temp;
	}
	if(*AnimationFrame <= 0.f)
	{
		*AnimationFrame += (float)Actions[CurrentAction].NumAnimationKeys-1.f;
	}
	
	if(Actions[CurrentAction].Loop)
	{
    	if(*AnimationFrame >= (float)Actions[CurrentAction].NumAnimationKeys)
		{
      		*AnimationFrame = (float)Actions[CurrentAction].NumAnimationKeys-0.01f;
    		Loop = false;
		}
	}
	else
	{
		int Key;
		if(Actions[CurrentAction].LockPositions)
			Key = Actions[CurrentAction].NumAnimationKeys-1;
		else
			Key = Actions[CurrentAction].NumAnimationKeys;

		float fTemp; 

		if(SceneFlag == 4)
		{
			fTemp = *AnimationFrame+2;
		}
		else if(gMapManager.WorldActive == WD_39KANTURU_3RD && CurrentAction == MONSTER01_APEAR)
		{
			fTemp = *AnimationFrame+1;
		}
		else 
		{
			fTemp = *AnimationFrame;
		}

		if(fTemp >= (int)Key)
		{
     		int Frame = (int)*AnimationFrame;
     		*AnimationFrame = (float)(Frame%(Key))+(*AnimationFrame-(float)Frame);
    		Loop = false;
		}	
	}
	CurrentAnimation = *AnimationFrame;
	CurrentAnimationFrame = (int)CurrentAnimation;

	return Loop;
}
void BMD::AnimationTransformWithAttachHighModel_usingGlobalTM(OBJECT* oHighHierarchyModel,BMD* bmdHighHierarchyModel,int iBoneNumberHighHierarchyModel,vec3_t &vOutPosHighHiearachyModelBone,vec3_t *arrOutSetfAllBonePositions,bool bApplyTMtoVertices)
{
	if( NumBones < 1) return;
	if( NumBones > MAX_BONES ) return;
	
	vec34_t*	arrBonesTMLocal;
	
	vec34_t		tmBoneHierarchicalObject;
	
	vec3_t		Temp, v3Position;
	OBB_t		OBB;
	
	arrBonesTMLocal = new vec34_t[NumBones];
	Vector( 0.0f, 0.0f, 0.0f, Temp );
	
	memset( arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones );
	memset( tmBoneHierarchicalObject, 0, sizeof(vec34_t) );
	
	memcpy( tmBoneHierarchicalObject, oHighHierarchyModel->BoneTransform[iBoneNumberHighHierarchyModel], sizeof(vec34_t) );
	BodyScale = oHighHierarchyModel->Scale;		
	
	tmBoneHierarchicalObject[0][3] = tmBoneHierarchicalObject[0][3] * BodyScale;
	tmBoneHierarchicalObject[1][3] = tmBoneHierarchicalObject[1][3] * BodyScale;
	tmBoneHierarchicalObject[2][3] = tmBoneHierarchicalObject[2][3] * BodyScale;
	
	if( NULL != vOutPosHighHiearachyModelBone )
	{
		Vector( tmBoneHierarchicalObject[0][3], tmBoneHierarchicalObject[1][3], tmBoneHierarchicalObject[2][3],
			vOutPosHighHiearachyModelBone );
	}
	
	VectorCopy( oHighHierarchyModel->Position, v3Position );
	
	Animation( arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false );
	
	for( int i_ = 0; i_ < NumBones; ++i_ )
	{
		R_ConcatTransforms( tmBoneHierarchicalObject, arrBonesTMLocal[i_], BoneTransform[i_] );
		BoneTransform[i_][0][3] = BoneTransform[i_][0][3] + v3Position[0];
		BoneTransform[i_][1][3] = BoneTransform[i_][1][3] + v3Position[1];
		BoneTransform[i_][2][3] = BoneTransform[i_][2][3] + v3Position[2];

		Vector( BoneTransform[i_][0][3], 
			BoneTransform[i_][1][3],
			BoneTransform[i_][2][3],
			arrOutSetfAllBonePositions[i_] );
	}
	
	if(true == bApplyTMtoVertices)
	{
		Transform(BoneTransform, Temp, Temp, &OBB, false);
	}

	delete [] arrBonesTMLocal;
}
						
void BMD::AnimationTransformWithAttachHighModel(OBJECT* oHighHierarchyModel, BMD* bmdHighHierarchyModel, int iBoneNumberHighHierarchyModel, vec3_t& vOutPosHighHierarchyModelBone, vec3_t* arrOutSetfAllBonePositions)
{
	if (NumBones < 1 || NumBones > MAX_BONES)
		return;

	vec34_t* arrBonesTMLocal = new vec34_t[NumBones];
	vec34_t* arrBonesTMLocalResult = new vec34_t[NumBones];
	vec34_t tmBoneHierarchicalObject;
	vec3_t Temp, v3Position;

	Vector(0.0f, 0.0f, 0.0f, Temp);

	memset(arrBonesTMLocalResult, 0, sizeof(vec34_t) * NumBones);
	memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);
	memset(tmBoneHierarchicalObject, 0, sizeof(vec34_t));

	memcpy(tmBoneHierarchicalObject, oHighHierarchyModel->BoneTransform[iBoneNumberHighHierarchyModel], sizeof(vec34_t));

	BodyScale = oHighHierarchyModel->Scale;

	tmBoneHierarchicalObject[0][3] *= BodyScale;
	tmBoneHierarchicalObject[1][3] *= BodyScale;
	tmBoneHierarchicalObject[2][3] *= BodyScale;

	if (vOutPosHighHierarchyModelBone != nullptr)
	{
		Vector(tmBoneHierarchicalObject[0][3], tmBoneHierarchicalObject[1][3], tmBoneHierarchicalObject[2][3], vOutPosHighHierarchyModelBone);
	}

	VectorCopy(oHighHierarchyModel->Position, v3Position);

	Animation(arrBonesTMLocal, 0, 0, 0, Temp, Temp, false, false);

	for (int i_ = 0; i_ < NumBones; ++i_)
	{
		R_ConcatTransforms(tmBoneHierarchicalObject, arrBonesTMLocal[i_], arrBonesTMLocalResult[i_]);
		arrBonesTMLocalResult[i_][0][3] += v3Position[0];
		arrBonesTMLocalResult[i_][1][3] += v3Position[1];
		arrBonesTMLocalResult[i_][2][3] += v3Position[2];

		Vector(arrBonesTMLocalResult[i_][0][3], arrBonesTMLocalResult[i_][1][3], arrBonesTMLocalResult[i_][2][3], arrOutSetfAllBonePositions[i_]);
	}

	delete[] arrBonesTMLocalResult;
	delete[] arrBonesTMLocal;
}

void BMD::AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions, const OBJECT* oSelf)
{
	if (NumBones < 1 || NumBones > MAX_BONES)
		return;

	vec34_t* arrBonesTMLocal = new vec34_t[NumBones];
	vec3_t Temp;

	Vector(0.0f, 0.0f, 0.0f, Temp);

	memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);

	Animation(arrBonesTMLocal, oSelf->AnimationFrame, oSelf->PriorAnimationFrame, oSelf->PriorAction, const_cast<OBJECT*>(oSelf)->Angle, Temp, false, true);

	for (int i_ = 0; i_ < NumBones; ++i_)
	{
		Vector(arrBonesTMLocal[i_][0][3], arrBonesTMLocal[i_][1][3], arrBonesTMLocal[i_][2][3], arrOutSetfAllBonePositions[i_]);
	}

	delete[] arrBonesTMLocal;
}

void BMD::AnimationTransformOnlySelf(vec3_t* arrOutSetfAllBonePositions, const vec3_t& v3Angle, const vec3_t& v3Position, const float& fScale,
	OBJECT* oRefAnimation, const float fFrameArea, const float fWeight)
{
	if (NumBones < 1 || NumBones > MAX_BONES)
		return;

	vec34_t* arrBonesTMLocal = new vec34_t[NumBones];
	vec3_t v3RootAngle, v3RootPosition;
	float fRootScale;
	vec3_t Temp;

	fRootScale = fScale;

	v3RootAngle[0] = v3Angle[0];
	v3RootAngle[1] = v3Angle[1];
	v3RootAngle[2] = v3Angle[2];

	v3RootPosition[0] = v3Position[0];
	v3RootPosition[1] = v3Position[1];
	v3RootPosition[2] = v3Position[2];

	Vector(0.0f, 0.0f, 0.0f, Temp);

	memset(arrBonesTMLocal, 0, sizeof(vec34_t) * NumBones);

	if (oRefAnimation == NULL)
	{
		Animation(arrBonesTMLocal, 0, 0, 0, v3RootAngle, Temp, false, true);
	}
	else
	{
		float fAnimationFrame = oRefAnimation->AnimationFrame;
		float fPriorAnimationFrame = oRefAnimation->PriorAnimationFrame;
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
		unsigned short iPriorAction = oRefAnimation->PriorAction;
#else
		unsigned char iPriorAction = oRefAnimation->PriorAction;
#endif

		if (fWeight >= 0.0f && fFrameArea > 0.0f)
		{
			float fAnimationFrameStart = fAnimationFrame - fFrameArea;
			float fAnimationFrameEnd = fAnimationFrame;
			LInterpolationF(fAnimationFrame, fAnimationFrameStart, fAnimationFrameEnd, fWeight);
		}

		Animation(arrBonesTMLocal, fAnimationFrame, fPriorAnimationFrame, iPriorAction, v3RootAngle, Temp, false, true);
	}

	vec3_t v3RelatePos;
	Vector(1.0f, 1.0f, 1.0f, v3RelatePos);
	for (int i_ = 0; i_ < NumBones; ++i_)
	{
		Vector(arrBonesTMLocal[i_][0][3], arrBonesTMLocal[i_][1][3], arrBonesTMLocal[i_][2][3], arrOutSetfAllBonePositions[i_]);
	}

	delete[] arrBonesTMLocal;
}

vec3_t g_vright;

int g_smodels_total = 1;                // cookie
float g_chrome[MAX_VERTICES][2];        // texture coords for surface normals
int g_chromeage[MAX_BONES];             // last time chrome vectors were updated
vec3_t g_chromeup[MAX_BONES];           // chrome vector "up" in bone reference frames
vec3_t g_chromeright[MAX_BONES];        // chrome vector "right" in bone reference frames

void BMD::UpdateChromeVectors(int bone) // New
{
	Vector(0.f, 0.f, 1.f, g_vright);

	vec3_t chromeupvec;       // g_chrome t vector in world reference frame
	vec3_t chromerightvec;    // g_chrome s vector in world reference frame
	vec3_t tmp;               // vector pointing at bone in world reference frame
	VectorScale(BodyOrigin, -1, tmp);
	VectorNormalize(tmp);
	CrossProduct(tmp, g_vright, chromeupvec);
	VectorNormalize(chromeupvec);
	CrossProduct(tmp, chromeupvec, chromerightvec);
	VectorNormalize(chromerightvec);
	VectorIRotate(chromeupvec, nullptr, g_chromeup[bone]);
	VectorIRotate(chromerightvec, nullptr, g_chromeright[bone]);

	g_chromeage[bone] = g_smodels_total;
}

void BMD::Chrome(float* pchrome, int bone, vec3_t normal)
{
	float n;

	if (g_chromeage[bone] != g_smodels_total)
	{
		UpdateChromeVectors(bone);
	}

	// calc s coord
	n = DotProduct(normal, g_chromeright[bone]);
	pchrome[0] = (n + 1.f);

	// calc t coord
	n = DotProduct(normal, g_chromeup[bone]);
	pchrome[1] = (n + 1.f);
}

void BMD::Lighting(float* pLight, const Light_t* lp, const vec3_t Position, const vec3_t Normal)
{
	vec3_t Light;
	VectorSubtract(lp->Position, Position, Light);
	float LengthSquared = DotProduct(Light, Light);
	float Length = sqrtf(LengthSquared);

	float LightCos = (DotProduct(Normal, Light) / Length) * 0.8f + 0.3f;
	if (Length > lp->Range)
		LightCos -= (Length - lp->Range) * 0.01f;
	LightCos = (LightCos < 0.f) ? 0.f : LightCos;

	pLight[0] += LightCos * lp->Color[0];
	pLight[1] += LightCos * lp->Color[1];
	pLight[2] += LightCos * lp->Color[2];
}

///////////////////////////////////////////////////////////////////////////////
// light map
///////////////////////////////////////////////////////////////////////////////

const int AXIS_X = 0;
const int AXIS_Y = 1;
const int AXIS_Z = 2;

const float SubPixel = 16.0f;

void SmoothBitmap(int Width, int Height, unsigned char* Buffer)
{
	int RowStride = Width * 3;
	for (int i = 1; i < Height - 1; i++)
	{
		for (int j = 1; j < Width - 1; j++)
		{
			int Index = (i * Width + j) * 3;
			for (int k = 0; k < 3; k++)
			{
				Buffer[Index] = (Buffer[Index - RowStride - 3] + Buffer[Index - RowStride] + Buffer[Index - RowStride + 3] +
					Buffer[Index - 3] + Buffer[Index + 3] +
					Buffer[Index + RowStride - 3] + Buffer[Index + RowStride] + Buffer[Index + RowStride + 3]) / 8;
				Index++;
			}
		}
	}
}

bool BMD::CollisionDetectLineToMesh(vec3_t Position, vec3_t Target, bool Collision, int Mesh, int Triangle)
{
	int i, j;
	for (i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];

		for (j = 0; j < m->NumTriangles; j++)
		{
			if (i == Mesh && j == Triangle) continue;
			Triangle_t* tp = &m->Triangles[j];
			float* vp1 = VertexTransform[i][tp->VertexIndex[0]];
			float* vp2 = VertexTransform[i][tp->VertexIndex[1]];
			float* vp3 = VertexTransform[i][tp->VertexIndex[2]];
			float* vp4 = VertexTransform[i][tp->VertexIndex[3]];

			vec3_t Normal;
			FaceNormalize(vp1, vp2, vp3, Normal);
			bool success = CollisionDetectLineToFace(Position, Target, tp->Polygon, vp1, vp2, vp3, vp4, Normal, Collision);
			if (success == true) return true;
		}
	}
	return false;
}

void BMD::CreateLightMapSurface(Light_t* lp, Mesh_t* m, int i, int j, int MapWidth, int MapHeight, int MapWidthMax, int MapHeightMax, vec3_t BoundingMin, vec3_t BoundingMax, int Axis)
{
	Triangle_t* tp = &m->Triangles[j];
	float* np = NormalTransform[i][tp->NormalIndex[0]];
	float* vp = VertexTransform[i][tp->VertexIndex[0]];
	float d = -DotProduct(vp, np);

	Bitmap_t* lmp = &LightMaps[NumLightMaps];
	if (lmp->Buffer == nullptr)
	{
		lmp->Width = MapWidthMax;
		lmp->Height = MapHeightMax;
		int BufferBytes = lmp->Width * lmp->Height * 3;
		lmp->Buffer = new unsigned char[BufferBytes]();
	}

	for (int k = 0; k < MapHeight; k++)
	{
		for (int l = 0; l < MapWidth; l++)
		{
			vec3_t p;
			Vector(BoundingMin[0] + l * SubPixel, BoundingMin[1] + k * SubPixel, 0.f, p);

			switch (Axis)
			{
			case AXIS_Z:
				p[2] = (np[0] * p[0] + np[1] * p[1] + d) / -np[2];
				break;
			case AXIS_Y:
				p[1] = (np[0] * p[0] + np[2] * p[2] + d) / -np[1];
				break;
			case AXIS_X:
				p[0] = (np[2] * p[2] + np[1] * p[1] + d) / -np[0];
				break;
			}

			vec3_t Direction;
			VectorSubtract(p, lp->Position, Direction);
			VectorNormalize(Direction);
			VectorSubtract(p, Direction, p);

			bool success = CollisionDetectLineToMesh(lp->Position, p, true, i, j);

			if (!success)
			{
				unsigned char* Bitmap = &lmp->Buffer[(k * MapWidthMax + l) * 3];
				vec3_t Light;
				Vector(0.f, 0.f, 0.f, Light);
				Lighting(Light, lp, p, np);
				for (int c = 0; c < 3; c++)
				{
					int Color = Bitmap[c] + static_cast<unsigned char>(Light[c] * 255.f);
					Bitmap[c] = (Color > 255) ? 255 : Color;
				}
			}
		}
	}
}

void BMD::CreateLightMaps()
{
}

void BMD::BindLightMaps()
{
	if (LightMapEnable)
		return;

	for (int i = 0; i < NumLightMaps; i++)
	{
		Bitmap_t* lmp = &LightMaps[i];
		if (lmp->Buffer != nullptr)
		{
			SmoothBitmap(lmp->Width, lmp->Height, lmp->Buffer);
			SmoothBitmap(lmp->Width, lmp->Height, lmp->Buffer);

			glBindTexture(GL_TEXTURE_2D, i + IndexLightMap);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, lmp->Width, lmp->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, lmp->Buffer);
		}
	}
	LightMapEnable = true;
}

void BMD::ReleaseLightMaps()
{
	if (!LightMapEnable)
		return;

	for (int i = 0; i < NumLightMaps; i++)
	{
		Bitmap_t& lmp = LightMaps[i];
		if (lmp.Buffer)
		{
			delete[] lmp.Buffer;
			lmp.Buffer = nullptr;
		}
	}

	LightMapEnable = false;
}

void BMD::BeginRender(float Alpha)
{
    // Assuming OpenGL context is active
    glPushMatrix();
}

void BMD::EndRender()
{
    // Assuming OpenGL context is active
    glPopMatrix();
}

extern float WorldTime;
extern int WaterTextureNumber;
extern int MoveSceneFrame;

void BMD::RenderMesh(int i, int RenderFlag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int MeshTexture)
{
	if (i >= NumMeshs || i < 0)
		return;

	Mesh_t* m = &Meshs[i];
	if (m->NumTriangles == 0)
		return;

	float Wave = (int)WorldTime % 10000 * 0.0001f;

	int Texture = IndexTexture[m->Texture];
	if (Texture == BITMAP_HIDE)
		return;
	else if (Texture == BITMAP_SKIN)
	{
		if (HideSkin) return;
		Texture = BITMAP_SKIN + Skin;
	}
	else if (Texture == BITMAP_WATER)
	{
		Texture = BITMAP_WATER + WaterTextureNumber;
	}
	else  if (Texture == BITMAP_HAIR)
	{
		if (HideSkin) return;
		Texture = BITMAP_HAIR + (Skin - 8);
	}

	if (MeshTexture != -1)
		Texture = MeshTexture;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

	bool EnableWave = false;
	int streamMesh = StreamMesh;
	if (m->m_csTScript != NULL)
	{
		if (m->m_csTScript->getStreamMesh())
		{
			streamMesh = i;
		}
	}
	if ((i == BlendMesh || i == streamMesh) && (BlendMeshTexCoordU != 0.f || BlendMeshTexCoordV != 0.f))
		EnableWave = true;

	bool EnableLight = LightEnable;
	if (i == StreamMesh)
	{
		glColor3fv(BodyLight);
		EnableLight = false;
	}
	else if (EnableLight)
	{
		for (int j = 0; j < m->NumNormals; j++)
		{
			VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
		}
	}

	int Render = RenderFlag;
	if ((RenderFlag & RENDER_COLOR) == RENDER_COLOR)
	{
		Render = RENDER_COLOR;
		if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();

		if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
		{
			DisableDepthTest();
		}

		DisableTexture();
		if (Alpha >= 0.99f)
		{
			glColor3fv(BodyLight);
		}
		else
		{
			EnableAlphaTest();
			glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
		}
	}
	else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME ||
		(RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 ||
		(RenderFlag & RENDER_CHROME3) == RENDER_CHROME3 ||
		(RenderFlag & RENDER_CHROME4) == RENDER_CHROME4 ||
		(RenderFlag & RENDER_CHROME5) == RENDER_CHROME5 ||
		(RenderFlag & RENDER_CHROME6) == RENDER_CHROME6 ||
		(RenderFlag & RENDER_CHROME7) == RENDER_CHROME7 ||
		(RenderFlag & RENDER_METAL) == RENDER_METAL ||
		(RenderFlag & RENDER_OIL) == RENDER_OIL
		)
	{
		if (m->m_csTScript != NULL)
		{
			if (m->m_csTScript->getNoneBlendMesh()) return;
		}

		if (m->NoneBlendMesh)
			return;

		Render = RENDER_CHROME;
		if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
		{
			Render = RENDER_CHROME4;
		}
		if ((RenderFlag & RENDER_OIL) == RENDER_OIL)
		{
			Render = RENDER_OIL;
		}

		float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

		vec3_t L = { (float)(cos(WorldTime * 0.001f)), (float)(sin(WorldTime * 0.002f)), 1.f };
		for (int j = 0; j < m->NumNormals; j++)
		{
			if (j > MAX_VERTICES) break;
			float* Normal = NormalTransform[i][j];

			if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
				g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
			}
			else if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3)
			{
				g_chrome[j][0] = DotProduct(Normal, LightVector);
				g_chrome[j][1] = 1.f - DotProduct(Normal, LightVector);
			}
			else if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
			{
				g_chrome[j][0] = DotProduct(Normal, L);
				g_chrome[j][1] = 1.f - DotProduct(Normal, L);
				g_chrome[j][1] -= Normal[2] * 0.5f + Wave * 3.f;
				g_chrome[j][0] += Normal[1] * 0.5f + L[1] * 3.f;
			}
			else if ((RenderFlag & RENDER_CHROME5) == RENDER_CHROME5)
			{
				g_chrome[j][0] = DotProduct(Normal, L);
				g_chrome[j][1] = 1.f - DotProduct(Normal, L);
				g_chrome[j][1] -= Normal[2] * 2.5f + Wave * 1.f;
				g_chrome[j][0] += Normal[1] * 3.f + L[1] * 5.f;
			}
			else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
				g_chrome[j][1] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
			}
			else if ((RenderFlag & RENDER_CHROME7) == RENDER_CHROME7)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + WorldTime * 0.00006f;
				g_chrome[j][1] = (Normal[2] + Normal[0]) * 0.8f + WorldTime * 0.00006f;
			}
			else if ((RenderFlag & RENDER_OIL) == RENDER_OIL)
			{
				g_chrome[j][0] = Normal[0];
				g_chrome[j][1] = Normal[1];
			}
			else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME)
			{
				g_chrome[j][0] = Normal[2] * 0.5f + Wave;
				g_chrome[j][1] = Normal[1] * 0.5f + Wave * 2.f;
			}
			else
			{
				g_chrome[j][0] = Normal[2] * 0.5f + 0.2f;
				g_chrome[j][1] = Normal[1] * 0.5f + 0.5f;
			}
		}
		if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3
			|| (RenderFlag & RENDER_CHROME4) == RENDER_CHROME4
			|| (RenderFlag & RENDER_CHROME5) == RENDER_CHROME5
			|| (RenderFlag & RENDER_CHROME7) == RENDER_CHROME7
			)
		{
			if (Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
			EnableAlphaBlend();
		}
		else if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			if (Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
			EnableAlphaBlend();
		}
		else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else if ((RenderFlag & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
			EnableLightMap();
		else if (Alpha >= 0.99f)
		{
			DisableAlphaBlend();
		}
		else
		{
			EnableAlphaTest();
		}

		if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
		{
			DisableDepthTest();
		}

		if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME2);
		}
		else if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3 && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME2);
		}
		else if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4 && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME2);
		}
		else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6 && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME6);
		}
		else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME);
		}
		else if ((RenderFlag & RENDER_METAL) == RENDER_METAL && MeshTexture == -1)
		{
			BindTexture(BITMAP_SHINY);
		}
		else
		{
			BindTexture(Texture);
		}
	}
	else if (BlendMesh <= -2 || m->Texture == BlendMesh)
	{
		Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else
			EnableAlphaBlend();

		if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
		{
			DisableDepthTest();
		}

		glColor3f(BodyLight[0] * BlendMeshLight, BodyLight[1] * BlendMeshLight, BodyLight[2] * BlendMeshLight);
		EnableLight = false;
	}
	else if ((RenderFlag & RENDER_TEXTURE) == RENDER_TEXTURE)
	{
		Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			EnableAlphaBlend();
		}
		else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
		{
			EnableAlphaBlendMinus();
		}
		else if (Alpha < 0.99f || pBitmap->Components == 4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}

		if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
		{
			DisableDepthTest();
		}
	}
	else if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if (pBitmap->Components == 4 || m->Texture == BlendMesh)
		{
			return;
		}

		Render = RENDER_BRIGHT;
		EnableAlphaBlend();
		DisableTexture();
		DisableDepthMask();

		if ((RenderFlag & RENDER_NODEPTH) == RENDER_NODEPTH)
		{
			DisableDepthTest();
		}
	}
	else
	{
		Render = RENDER_TEXTURE;
	}
	if (RenderFlag & RENDER_DOPPELGANGER)
	{
		if (pBitmap->Components != 4)
		{
			EnableCullFace();
			EnableDepthMask();
		}
	}

	glBegin(GL_TRIANGLES);
	for (int j = 0; j < m->NumTriangles; j++)
	{
		Triangle_t* tp = &m->Triangles[j];
		for (int k = 0; k < tp->Polygon; k++)
		{
			int vi = tp->VertexIndex[k];
			switch (Render)
			{

			case RENDER_TEXTURE:
			{
				TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
				if (EnableWave)
				{
					glTexCoord2f(texp->TexCoordU + BlendMeshTexCoordU, texp->TexCoordV + BlendMeshTexCoordV);
				}
				else
				{
					glTexCoord2f(texp->TexCoordU, texp->TexCoordV);
				}

				if (EnableLight)
				{
					int ni = tp->NormalIndex[k];

					if (Alpha >= 0.99f)
					{
						glColor3fv(LightTransform[i][ni]);
					}
					else
					{
						float* Light = LightTransform[i][ni];
						glColor4f(Light[0], Light[1], Light[2], Alpha);
					}
				}
				break;
			}
			case RENDER_CHROME:
			{
				if (Alpha >= 0.99f)
					glColor3fv(BodyLight);
				else
					glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
				int ni = tp->NormalIndex[k];
				glTexCoord2f(g_chrome[ni][0], g_chrome[ni][1]);
				break;
			}
			case RENDER_CHROME4:
			{
				if (Alpha >= 0.99f)
					glColor3fv(BodyLight);
				else
					glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
				int ni = tp->NormalIndex[k];
				glTexCoord2f(g_chrome[ni][0] + BlendMeshTexCoordU, g_chrome[ni][1] + BlendMeshTexCoordV);
			}
			break;

			case RENDER_OIL:
			{
				if (Alpha >= 0.99f)
					glColor3fv(BodyLight);
				else
					glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
				TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
				int ni = tp->VertexIndex[k];
				glTexCoord2f(g_chrome[ni][0] * texp->TexCoordU + BlendMeshTexCoordU, g_chrome[ni][1] * texp->TexCoordV + BlendMeshTexCoordV);
				break;
			}
			}

			if ((RenderFlag & RENDER_SHADOWMAP) == RENDER_SHADOWMAP)
			{
				int vi = tp->VertexIndex[k];
				vec3_t Position;
				VectorSubtract(VertexTransform[i][vi], BodyOrigin, Position);

				Position[0] += Position[2] * (Position[0] + 2000.f) / (Position[2] - 4000.f);
				Position[2] = 5.f;

				VectorAdd(Position, BodyOrigin, Position);
				glVertex3fv(Position);
			}
			else if ((RenderFlag & RENDER_WAVE) == RENDER_WAVE)
			{
				float vPos[3];
				float fParam = (float)((int)WorldTime + vi * 931) * 0.007f;
				float fSin = sinf(fParam);
				float fCos = cosf(fParam);

				int ni = tp->NormalIndex[k];
				Normal_t* np = &m->Normals[ni];
				float* Normal = NormalTransform[i][ni];
				for (int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord] * fSin * 28.0f;
				}
				glVertex3fv(vPos);
			}
			else
			{
				glVertex3fv(VertexTransform[i][vi]);
			}
		}
	}
	glEnd();
}

void BMD::RenderMeshAlternative( int iRndExtFlag, int iParam, int i,int RenderFlag,float Alpha,int BlendMesh,float BlendMeshLight,float BlendMeshTexCoordU,float BlendMeshTexCoordV,int MeshTexture)
{
	if (i >= NumMeshs || i < 0)
		return;

	Mesh_t* m = &Meshs[i];
	if (m->NumTriangles == 0)
		return;
	float Wave = (int)WorldTime % 10000 * 0.0001f;

	int Texture = IndexTexture[m->Texture];
	if (Texture == BITMAP_HIDE)
		return;
	if (MeshTexture != -1)
		Texture = MeshTexture;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

	bool EnableWave = false;
	int streamMesh = StreamMesh;
	if (m->m_csTScript != NULL)
	{
		if (m->m_csTScript->getStreamMesh())
		{
			streamMesh = i;
		}
	}
	if ((i == BlendMesh || i == streamMesh) && (BlendMeshTexCoordU != 0.f || BlendMeshTexCoordV != 0.f))
		EnableWave = true;

	bool EnableLight = LightEnable;
	if (i == StreamMesh)
	{
		glColor3fv(BodyLight);
		EnableLight = false;
	}
	else if (EnableLight)
	{
		for (int j = 0; j < m->NumNormals; j++)
		{
			VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
		}
	}

	int Render = RenderFlag;
	if((RenderFlag&RENDER_COLOR) == RENDER_COLOR)
	{
    	Render = RENDER_COLOR;
       	if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        DisableTexture();
		if(Alpha >= 0.99f)
        {
            glColor3fv(BodyLight);
        }
		else
        {
            EnableAlphaTest();
            glColor4f(BodyLight[0],BodyLight[1],BodyLight[2],Alpha);
        }
 	}
	else if ((RenderFlag & (RENDER_CHROME | RENDER_CHROME2 | RENDER_CHROME3 | RENDER_CHROME4 | RENDER_CHROME5 | RENDER_CHROME7 | RENDER_METAL | RENDER_OIL)) != 0)
	{
		if ( m->m_csTScript!=NULL )
        {
            if ( m->m_csTScript->getNoneBlendMesh() ) return;
        }
		if(m->NoneBlendMesh )
			return;
   		Render = RENDER_CHROME;
        if ( (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 )
        {
            Render = RENDER_CHROME4;
        }
		float Wave2 = (int)WorldTime % 5000 * 0.00024f - 0.4f;

		vec3_t L = { (float)(cos(WorldTime * 0.001f)), (float)(sin(WorldTime * 0.002f)), 1.f };
		for (int j = 0; j < m->NumNormals; j++)
		{
			if (j > MAX_VERTICES)
				break;
			float* Normal = NormalTransform[i][j];

			if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
				g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
			}
			else if ((RenderFlag & RENDER_CHROME3) == RENDER_CHROME3)
			{
				g_chrome[j][0] = DotProduct(Normal, LightVector);
				g_chrome[j][1] = 1.f - DotProduct(Normal, LightVector);
			}
			else if ((RenderFlag & RENDER_CHROME4) == RENDER_CHROME4)
			{
				g_chrome[j][0] = DotProduct(Normal, L);
				g_chrome[j][1] = 1.f - DotProduct(Normal, L);
				g_chrome[j][1] -= Normal[2] * 0.5f + Wave * 3.f;
				g_chrome[j][0] += Normal[1] * 0.5f + L[1] * 3.f;
			}
			else if ((RenderFlag & RENDER_CHROME5) == RENDER_CHROME5)
			{
				Vector(0.1f, -0.23f, 0.22f, LightVector2);

				g_chrome[j][0] = (DotProduct(Normal, LightVector2) + Normal[1] + LightVector2[1] * 3.f) / 1.08f;
				g_chrome[j][1] = (1.f - DotProduct(Normal, LightVector2) - Normal[2] * 0.5f + 3.f) / 1.08f;
			}
			else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
				g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
			}
			else if ((RenderFlag & RENDER_CHROME7) == RENDER_CHROME7)
			{
				Vector(0.1f, -0.23f, 0.22f, LightVector2);

				g_chrome[j][0] = (DotProduct(Normal, LightVector2)) / 1.08f;
				g_chrome[j][1] = (1.f - DotProduct(Normal, LightVector2)) / 1.08f;
			}
			else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME)
			{
				g_chrome[j][0] = Normal[2] * 0.5f + Wave;
				g_chrome[j][1] = Normal[1] * 0.5f + Wave * 2.f;
			}
			else
			{
				g_chrome[j][0] = Normal[2] * 0.5f + 0.2f;
				g_chrome[j][1] = Normal[1] * 0.5f + 0.5f;
			}
		}

        if ( (RenderFlag&RENDER_CHROME3)==RENDER_CHROME3
			|| (RenderFlag&RENDER_CHROME4)==RENDER_CHROME4
			|| (RenderFlag&RENDER_CHROME5)==RENDER_CHROME5
			|| (RenderFlag&RENDER_CHROME7)==RENDER_CHROME7 
		   )
        {
			if ( Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
     		EnableAlphaBlend();
        }
        else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			if ( Alpha < 0.99f)
			{
				BodyLight[0] *= Alpha; BodyLight[1] *= Alpha; BodyLight[2] *= Alpha;
			}
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
     	else if((RenderFlag&RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
            EnableLightMap();
		else if ( Alpha >= 0.99f)
		{
			DisableAlphaBlend();
		}
		else
		{
			EnableAlphaTest();
		}

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

        if((RenderFlag&RENDER_CHROME2)==RENDER_CHROME2 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME3)==RENDER_CHROME3 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
        else if((RenderFlag&RENDER_CHROME4)==RENDER_CHROME4 && MeshTexture==-1)
        {
			BindTexture(BITMAP_CHROME2);
        }
		if (MeshTexture == -1)
		{
			if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME)
				BindTexture(BITMAP_CHROME);
			else if ((RenderFlag & RENDER_METAL) == RENDER_METAL)
				BindTexture(BITMAP_SHINY);
			else
				BindTexture(Texture);
		}
	}	
	else if(BlendMesh<=-2 || m->Texture == BlendMesh)
	{
    	Render = RENDER_TEXTURE;
   		BindTexture(Texture);
		if((RenderFlag&RENDER_DARK) == RENDER_DARK)
     		EnableAlphaBlendMinus();
		else
     		EnableAlphaBlend();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }

		glColor3f(BodyLight[0]*BlendMeshLight,BodyLight[1]*BlendMeshLight,BodyLight[2]*BlendMeshLight);
		EnableLight = false;
	}
	else if((RenderFlag&RENDER_TEXTURE) == RENDER_TEXTURE)
	{
    	Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
		{
     		EnableAlphaBlend();
		}
		else if((RenderFlag&RENDER_DARK) == RENDER_DARK)
		{
     		EnableAlphaBlendMinus();
		}
		else if(Alpha<0.99f || pBitmap->Components==4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else if((RenderFlag&RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if(pBitmap->Components==4 || m->Texture == BlendMesh)
		{
			return;
		}
    	Render = RENDER_BRIGHT;
        EnableAlphaBlend();
        DisableTexture();
        DisableDepthMask();

        if ((RenderFlag&RENDER_NODEPTH)==RENDER_NODEPTH )
        {
            DisableDepthTest ();				
        }
	}
	else
	{
    	Render = RENDER_TEXTURE;
	}

	glBegin(GL_TRIANGLES);
	for (int j = 0; j < m->NumTriangles; j++)
	{
		Triangle_t* tp = &m->Triangles[j];
		for (int k = 0; k < tp->Polygon; k++)
		{
			int vi = tp->VertexIndex[k];
			switch (Render)
			{
			case RENDER_TEXTURE:
			{
				TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
				if (EnableWave)
					glTexCoord2f(texp->TexCoordU + BlendMeshTexCoordU, texp->TexCoordV + BlendMeshTexCoordV);
				else
					glTexCoord2f(texp->TexCoordU, texp->TexCoordV);
				if (EnableLight)
				{
					int ni = tp->NormalIndex[k];
					glColor4fv(Alpha >= 0.99f ? LightTransform[i][ni] : LightTransform[i][ni]);
				}
				break;
			}
			case RENDER_CHROME:
			{
				glColor4fv(Alpha >= 0.99f ? BodyLight : BodyLight);
				int ni = tp->NormalIndex[k];
				glTexCoord2f(g_chrome[ni][0], g_chrome[ni][1]);
				break;
			}
			}
			if ((iRndExtFlag & RNDEXT_WAVE))
			{
				float vPos[3];
				float fParam = (float)(((int)WorldTime + vi * 931) * 0.007f);
				float fSin = sinf(fParam);
				int ni = tp->NormalIndex[k];
				float* Normal = NormalTransform[i][ni];
				for (int iCoord = 0; iCoord < 3; ++iCoord)
				{
					vPos[iCoord] = VertexTransform[i][vi][iCoord] + Normal[iCoord] * fSin * 28.0f;
				}
				glVertex3fv(vPos);
			}
			else
			{
				glVertex3fv(VertexTransform[i][vi]);
			}
		}
	}
	glEnd();
}

void BMD::RenderMeshEffect(int i, int iType, int iSubType, vec3_t Angle, VOID* obj)
{
	if (i >= NumMeshs || i < 0)
		return;

	Mesh_t* m = &Meshs[i];
	if (m->NumTriangles <= 0)
		return;

	vec3_t angle, Light;
	int iEffectCount = 0;

	Vector(0.f, 0.f, 0.f, angle);
	Vector(1.f, 1.f, 1.f, Light);

	for (int j = 0; j < m->NumTriangles; j++)
	{
		Triangle_t* tp = &m->Triangles[j];

		for (int k = 0; k < tp->Polygon; k++)
		{
			int vi = tp->VertexIndex[k];

			switch (iType)
			{
			case MODEL_STONE_COFFIN:
				if (iSubType == 0)
				{
					if (rand() % 2 == 0)
						CreateEffect(MODEL_STONE_COFFIN + 1, VertexTransform[i][vi], angle, Light);

					if (rand() % 10 == 0)
						CreateEffect(MODEL_STONE_COFFIN, VertexTransform[i][vi], angle, Light);
				}
				else if (iSubType == 1)
				{
					CreateEffect(MODEL_STONE_COFFIN + 1, VertexTransform[i][vi], angle, Light, 2);
				}
				else if (iSubType == 2)
				{
					CreateEffect(MODEL_STONE_COFFIN + 1, VertexTransform[i][vi], angle, Light, 3);
				}
				else if (iSubType == 3)
				{
					CreateEffect(MODEL_STONE_COFFIN + rand() % 2, VertexTransform[i][vi], angle, Light, 4);
				}
				break;

			case MODEL_GATE:
				if (iSubType == 1)
				{
					Vector(0.2f, 0.2f, 0.2f, Light);
					if (rand() % 5 == 0)
						CreateEffect(MODEL_GATE + 1, VertexTransform[i][vi], angle, Light, 2);

					if (rand() % 10 == 0)
						CreateEffect(MODEL_GATE, VertexTransform[i][vi], angle, Light, 2);
				}
				else if (iSubType == 0)
				{
					Vector(0.2f, 0.2f, 0.2f, Light);
					if (rand() % 12 == 0)
						CreateEffect(MODEL_GATE + 1, VertexTransform[i][vi], angle, Light);

					if (rand() % 50 == 0)
						CreateEffect(MODEL_GATE, VertexTransform[i][vi], angle, Light);
				}
				break;

			case MODEL_BIG_STONE_PART1:
				if (rand() % 3 == 0)
					CreateEffect(MODEL_BIG_STONE_PART1 + rand() % 2, VertexTransform[i][vi], angle, Light, 1);
				break;

			case MODEL_BIG_STONE_PART2:
				if (rand() % 3 == 0)
					CreateEffect(MODEL_BIG_STONE_PART1 + rand() % 2, VertexTransform[i][vi], angle, Light);
				break;

			case MODEL_WALL_PART1:
				if (rand() % 3 == 0)
					CreateEffect

					(MODEL_WALL_PART1 + rand() % 2, VertexTransform[i][vi], angle, Light);
				break;

			case MODEL_GATE_PART1:
				Vector(0.2f, 0.2f, 0.2f, Light);
				if (rand() % 12 == 0)
					CreateEffect(MODEL_GATE_PART1 + 1, VertexTransform[i][vi], angle, Light);

				if (rand() % 40 == 0)
					CreateEffect(MODEL_GATE_PART1, VertexTransform[i][vi], angle, Light);

				if (rand() % 40 == 0)
					CreateEffect(MODEL_GATE_PART1 + 2, VertexTransform[i][vi], angle, Light);
				break;

			case MODEL_GOLEM_STONE:
				if (rand() % 45 == 0 && iEffectCount < 20)
				{
					if (iSubType == 0) 
					{
						CreateEffect(MODEL_GOLEM_STONE, VertexTransform[i][vi], angle, Light);
					}
					else if (iSubType == 1) 
					{
						CreateEffect(MODEL_BIG_STONE_PART1, VertexTransform[i][vi], angle, Light, 2);
						CreateEffect(MODEL_BIG_STONE_PART2, VertexTransform[i][vi], angle, Light, 2);
					}
					iEffectCount++;
				}
				break;

			case MODEL_SKIN_SHELL:
				if (rand() % 8 == 0)
					CreateEffect(MODEL_SKIN_SHELL, VertexTransform[i][vi], angle, Light, iSubType);
				break;

			case BITMAP_LIGHT:
				Vector(0.08f, 0.08f, 0.08f, Light);
				if (iSubType == 0)
				{
					CreateSprite(BITMAP_LIGHT, VertexTransform[i][vi], BodyScale, Light, nullptr);
				}
				else if (iSubType == 1)
				{
					Vector(1.f, 0.8f, 0.2f, Light);
					if (j % 22 == 0)
					{
						OBJECT* o = static_cast<OBJECT*>(obj);
						angle[0] = -static_cast<float>(rand() % 90);
						angle[1] = 0.f;
						angle[2] = Angle[2] + static_cast<float>(rand() % 120 - 60);
						CreateJoint(BITMAP_JOINT_SPIRIT, VertexTransform[i][vi], o->Position, angle, 13, o, 20.f, 0, 0);
					}
				}
				break;

			case BITMAP_BUBBLE:
				Vector(1.f, 1.f, 1.f, Light);
				if (rand() % 30 == 0)
					CreateParticle(BITMAP_BUBBLE, VertexTransform[i][vi], angle, Light, 2);
				break;
			}
		}
	}
}

void BMD::RenderBody(int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
	if (NumMeshs == 0)
		return;

	int iBlendMesh = BlendMesh;
	BeginRender(Alpha);

	if (!LightEnable)
	{
		if (Alpha >= 0.99f)
			glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
	}

	for (int i = 0; i < NumMeshs; i++)
	{
		iBlendMesh = BlendMesh;
		Mesh_t* m = &Meshs[i];

		if (m->m_csTScript != NULL)
		{
			if (!m->m_csTScript->getHiddenMesh() && i != HiddenMesh)
			{
				if (m->m_csTScript->getBright())
				{
					iBlendMesh = i;
				}

				RenderMesh(i, Flag, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);

				BYTE shadowType = m->m_csTScript->getShadowMesh();
				if (shadowType == SHADOW_RENDER_COLOR)
				{
					DisableAlphaBlend();
					if (Alpha >= 0.99f)
						glColor3f(0.f, 0.f, 0.f);
					else
						glColor4f(0.f, 0.f, 0.f, Alpha);

					RenderMesh(i, RENDER_COLOR | RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV);

					glColor3f(1.f, 1.f, 1.f);
				}
				else if (shadowType == SHADOW_RENDER_TEXTURE)
				{
					DisableAlphaBlend();
					if (Alpha >= 0.99f)
						glColor3f(0.f, 0.f, 0.f);
					else
						glColor4f(0.f, 0.f, 0.f, Alpha);

					RenderMesh(i, RENDER_TEXTURE | RENDER_SHADOWMAP, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV);

					glColor3f(1.f, 1.f, 1.f);
				}
			}
		}
		else
		{
			if (i != HiddenMesh)
			{
				RenderMesh(i, Flag, Alpha, iBlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);
			}
		}
	}

	EndRender();
}

void BMD::RenderBodyAlternative(int iRndExtFlag, int iParam, int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
	if (NumMeshs == 0)
		return;

	BeginRender(Alpha);

	if (!LightEnable)
	{
		if (Alpha >= 0.99f)
			glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
	}

	for (int i = 0; i < NumMeshs; i++)
	{
		if (i != HiddenMesh)
		{
			RenderMeshAlternative(iRndExtFlag, iParam, i, Flag, Alpha, BlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);
		}
	}

	EndRender();
}

void BMD::RenderMeshTranslate(int i, int RenderFlag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int MeshTexture)
{
	if (i >= NumMeshs || i < 0)
		return;

	Mesh_t* m = &Meshs[i];
	if (m->NumTriangles == 0)
		return;

	float Wave = static_cast<int>(WorldTime) % 10000 * 0.0001f;

	int Texture = IndexTexture[m->Texture];
	if (Texture == BITMAP_HIDE)
		return;
	else if (Texture == BITMAP_SKIN)
	{
		if (HideSkin)
			return;
		Texture = BITMAP_SKIN + Skin;
	}
	else if (Texture == BITMAP_WATER)
	{
		Texture = BITMAP_WATER + WaterTextureNumber;
	}
	if (MeshTexture != -1)
		Texture = MeshTexture;

	BITMAP_t* pBitmap = Bitmaps.GetTexture(Texture);

	bool EnableWave = false;
	int streamMesh = StreamMesh;
	if (m->m_csTScript != NULL)
	{
		if (m->m_csTScript->getStreamMesh())
		{
			streamMesh = i;
		}
	}
	if ((i == BlendMesh || i == streamMesh) && (BlendMeshTexCoordU != 0.f || BlendMeshTexCoordV != 0.f))
		EnableWave = true;

	bool EnableLight = LightEnable;
	if (i == StreamMesh)
	{
		glColor3fv(BodyLight);
		EnableLight = false;
	}
	else if (EnableLight)
	{
		for (int j = 0; j < m->NumNormals; j++)
		{
			VectorScale(BodyLight, IntensityTransform[i][j], LightTransform[i][j]);
		}
	}

	int Render = RenderFlag;
	if ((RenderFlag & RENDER_COLOR) == RENDER_COLOR)
	{
		Render = RENDER_COLOR;
		if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else
			DisableAlphaBlend();
		DisableTexture();
		glColor3fv(BodyLight);
	}
	else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME || (RenderFlag & RENDER_METAL) == RENDER_METAL || (RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 || (RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
	{
		if (m->m_csTScript != NULL)
		{
			if (m->m_csTScript->getNoneBlendMesh())
				return;
		}
		if (m->NoneBlendMesh)
			return;
		Render = RENDER_CHROME;

		float Wave2 = static_cast<int>(WorldTime) % 5000 * 0.00024f - 0.4f;

		for (int j = 0; j < m->NumNormals; j++)
		{
			if (j > MAX_VERTICES)
				break;
			float* Normal = NormalTransform[i][j];

			if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
				g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
			}
			else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME)
			{
				g_chrome[j][0] = Normal[2] * 0.5f + Wave;
				g_chrome[j][1] = Normal[1] * 0.5f + Wave * 2.f;
			}
			else if ((RenderFlag & RENDER_CHROME6) == RENDER_CHROME6)
			{
				g_chrome[j][0] = (Normal[2] + Normal[0]) * 0.8f + Wave2 * 2.f;
				g_chrome[j][1] = (Normal[1] + Normal[0]) * 1.0f + Wave2 * 3.f;
			}
			else
			{
				g_chrome[j][0] = Normal[2] * 0.5f + 0.2f;
				g_chrome[j][1] = Normal[1] * 0.5f + 0.5f;
			}
		}

		if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
			EnableAlphaBlend();
		else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else if ((RenderFlag & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
			EnableLightMap();
		else
			DisableAlphaBlend();

		if ((RenderFlag & RENDER_CHROME2) == RENDER_CHROME2 && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME2);
		}
		else if ((RenderFlag & RENDER_CHROME) == RENDER_CHROME && MeshTexture == -1)
		{
			BindTexture(BITMAP_CHROME);
		}
		else if ((RenderFlag & RENDER_METAL) == RENDER_METAL && MeshTexture == -1)
		{
			BindTexture(BITMAP_SHINY);
		}
		else
		{
			BindTexture(Texture);
		}
	}
	else if (BlendMesh <= -2 || m->Texture == BlendMesh)
	{
		Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
			EnableAlphaBlendMinus();
		else
			EnableAlphaBlend();
		glColor3f(BodyLight[0] * BlendMeshLight, BodyLight[1] * BlendMeshLight, BodyLight[2] * BlendMeshLight);
		EnableLight = false;
	}
	else if ((RenderFlag & RENDER_TEXTURE) == RENDER_TEXTURE)
	{
		Render = RENDER_TEXTURE;
		BindTexture(Texture);
		if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
		{
			EnableAlphaBlend();
		}
		else if ((RenderFlag & RENDER_DARK) == RENDER_DARK)
		{
			EnableAlphaBlendMinus();
		}
		else if (Alpha < 0.99f || pBitmap->Components == 4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}
	}
	else if ((RenderFlag & RENDER_BRIGHT) == RENDER_BRIGHT)
	{
		if (pBitmap->Components == 4 || m->Texture == BlendMesh)
		{
			return;
		}
		Render = RENDER_BRIGHT;
		EnableAlphaBlend();
		DisableTexture();
		DisableDepthMask();
	}
	else
	{
		Render = RENDER_TEXTURE;
	}

	glBegin(GL_TRIANGLES);
	for (int j = 0; j < m->NumTriangles; j++)


	{
		vec3_t pos;
		Triangle_t* tp = &m->Triangles[j];
		for (int k = 0; k < tp->Polygon; k++)
		{
			int vi = tp->VertexIndex[k];
			switch (Render)
			{
			case RENDER_TEXTURE:
			{
				TexCoord_t* texp = &m->TexCoords[tp->TexCoordIndex[k]];
				if (EnableWave)
					glTexCoord2f(texp->TexCoordU + BlendMeshTexCoordU, texp->TexCoordV + BlendMeshTexCoordV);
				else
					glTexCoord2f(texp->TexCoordU, texp->TexCoordV);
				if (EnableLight)
				{
					int ni = tp->NormalIndex[k];
					if (Alpha >= 0.99f)
					{
						glColor3fv(LightTransform[i][ni]);
					}
					else
					{
						float* Light = LightTransform[i][ni];
						glColor4f(Light[0], Light[1], Light[2], Alpha);
					}
				}
				break;
			}
			case RENDER_CHROME:
			{
				if (Alpha >= 0.99f)
					glColor3fv(BodyLight);
				else
					glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
				int ni = tp->NormalIndex[k];
				glTexCoord2f(g_chrome[ni][0], g_chrome[ni][1]);
				break;
			}
			}
			{
				VectorAdd(VertexTransform[i][vi], BodyOrigin, pos);
				glVertex3fv(pos);
			}
		}
	}
	glEnd();
}

void BMD::RenderBodyTranslate(int Flag, float Alpha, int BlendMesh, float BlendMeshLight, float BlendMeshTexCoordU, float BlendMeshTexCoordV, int HiddenMesh, int Texture)
{
	if (NumMeshs == 0)
		return;

	BeginRender(Alpha);
	if (!LightEnable)
	{
		if (Alpha >= 0.99f)
			glColor3fv(BodyLight);
		else
			glColor4f(BodyLight[0], BodyLight[1], BodyLight[2], Alpha);
	}
	for (int i = 0; i < NumMeshs; i++)
	{
		if (i != HiddenMesh)
		{
			RenderMeshTranslate(i, Flag, Alpha, BlendMesh, BlendMeshLight, BlendMeshTexCoordU, BlendMeshTexCoordV, Texture);
		}
	}
	EndRender();
}

void BMD::RenderBodyShadow(int BlendMesh, int HiddenMesh, int StartMeshNumber, int EndMeshNumber)
{
	if (NumMeshs == 0)
		return;

	DisableTexture();
	DisableDepthMask();
	BeginRender(1.f);

	int startMesh = 0;
	int endMesh = NumMeshs;

	if (StartMeshNumber != -1)
		startMesh = StartMeshNumber;
	if (EndMeshNumber != -1)
		endMesh = EndMeshNumber;

	float sx = 2000.f;
	float sy = 4000.f;

	if (gMapManager.InBattleCastle())
	{
		sx = 2500.f;
		sy = 4000.f;
	}

	for (int i = startMesh; i < endMesh; i++)
	{
		if (i != HiddenMesh)
		{
			Mesh_t* m = &Meshs[i];
			if (m->NumTriangles > 0 && m->Texture != BlendMesh)
			{
				glBegin(GL_TRIANGLES);
				for (int j = 0; j < m->NumTriangles; j++)
				{
					Triangle_t* tp = &m->Triangles[j];
					for (int k = 0; k < tp->Polygon; k++)
					{
						int vi = tp->VertexIndex[k];
						vec3_t Position;
						VectorSubtract(VertexTransform[i][vi], BodyOrigin, Position);
						Position[0] += Position[2] * (Position[0] + sx) / (Position[2] - sy);
						Position[2] = 5.f;
						VectorAdd(Position, BodyOrigin, Position);
						glVertex3fv(Position);
					}
				}
				glEnd();
			}
		}
	}
	EndRender();
	EnableDepthMask();
}

void BMD::RenderObjectBoundingBox()
{
	DisableTexture();
	glPushMatrix();
	glTranslatef(BodyOrigin[0], BodyOrigin[1], BodyOrigin[2]);
	glScalef(BodyScale, BodyScale, BodyScale);
	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		if (b->BoundingBox)
		{
			vec3_t BoundingVertices[8];
			for (int j = 0; j < 8; j++)
			{
				VectorTransform(b->BoundingVertices[j], BoneTransform[i], BoundingVertices[j]);
			}

			glBegin(GL_QUADS);
			glColor3f(0.2f, 0.2f, 0.2f);
			glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
			glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
			glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);
			glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[5]);

			glColor3f(0.2f, 0.2f, 0.2f);
			glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
			glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[2]);
			glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
			glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);

			glColor3f(0.6f, 0.6f, 0.6f);
			glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
			glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[3]);
			glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
			glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[6]);

			glColor3f(0.6f, 0.6f, 0.6f);
			glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
			glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[1]);
			glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
			glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[4]);

			glColor3f(0.4f, 0.4f, 0.4f);
			glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[7]);
			glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[5]);
			glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[1]);
			glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[3]);

			glColor3f(0.4f, 0.4f, 0.4f);
			glTexCoord2f(0.0F, 1.0F); glVertex3fv(BoundingVertices[0]);
			glTexCoord2f(1.0F, 1.0F); glVertex3fv(BoundingVertices[4]);
			glTexCoord2f(1.0F, 0.0F); glVertex3fv(BoundingVertices[6]);
			glTexCoord2f(0.0F, 0.0F); glVertex3fv(BoundingVertices[2]);
			glEnd();
		}
	}
	glPopMatrix();
	DisableAlphaBlend();
}

void BMD::RenderBone(float(*BoneMatrix)[3][4])
{
	DisableTexture();
	glDepthFunc(GL_ALWAYS);
	glColor3f(0.8f, 0.8f, 0.2f);
	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		if (!b->Dummy)
		{
			BoneMatrix_t* bm = &b->BoneMatrixes[CurrentAction];
			int Parent = b->Parent;
			if (Parent > 0)
			{
				float Scale = 1.f;
				float dx = bm->Position[CurrentAnimationFrame][0];
				float dy = bm->Position[CurrentAnimationFrame][1];
				float dz = bm->Position[CurrentAnimationFrame][2];
				Scale = sqrtf(dx * dx + dy * dy + dz * dz) * 0.05f;
				vec3_t Position[3];
				Vector(0.f, 0.f, -Scale, Position[0]);
				Vector(0.f, 0.f, Scale, Position[1]);
				Vector(0.f, 0.f, 0.f, Position[2]);
				vec3_t BoneVertices[3];
				VectorTransform(Position[0], BoneMatrix[Parent], BoneVertices[0]);
				VectorTransform(Position[1], BoneMatrix[Parent], BoneVertices[1]);
				VectorTransform(Position[2], BoneMatrix[i], BoneVertices[2]);
				for (int j = 0; j < 3; j++)
				{
					VectorMA(BodyOrigin, BodyScale, BoneVertices[j], BoneVertices[j]);
				}
				glBegin(GL_LINES);
				glVertex3fv(BoneVertices[0]);
				glVertex3fv(BoneVertices[1]);
				glVertex3fv(BoneVertices[1]);
				glVertex3fv(BoneVertices[2]);
				glVertex3fv(BoneVertices[2]);
				glVertex3fv(BoneVertices[0]);
				glEnd();
			}
		}
	}
	glDepthFunc(GL_LEQUAL);
}

void BlurShadow()
{
	for (int i = 1; i < ShadowBufferHeight - 1; i++)
	{
		auto& row = ShadowBuffer[i];
		for (int j = 1; j < ShadowBufferWidth - 1; j++)
		{
			row[j] = (row[j - 1] + row[j + 1] + ShadowBuffer[i - 1][j] + ShadowBuffer[i + 1][j]) / 4;
		}
	}
}

void BMD::Release()
{
	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];

		if (!b->Dummy)
		{
			for (int j = 0; j < NumActions; j++)
			{
				BoneMatrix_t* bm = &b->BoneMatrixes[j];
				delete[] bm->Position;
				delete[] bm->Rotation;
				delete[] bm->Quaternion;
			}
			delete[] b->BoneMatrixes;
		}
	}

	for (int i = 0; i < NumActions; i++)
	{
		Action_t* a = &Actions[i];
		if (a->LockPositions)
		{
			delete[] a->Positions;
		}
	}

	if (Meshs)
	{
		for (int i = 0; i < NumMeshs; i++)
		{
			Mesh_t* m = &Meshs[i];

			delete[] m->Vertices;
			delete[] m->Normals;
			delete[] m->TexCoords;
			delete[] m->Triangles;

			if (m->m_csTScript)
			{
				delete m->m_csTScript;
				m->m_csTScript = nullptr;
			}
			switch (IndexTexture[m->Texture])
			{
			case BITMAP_SKIN:
				break;
			default:
				DeleteBitmap(IndexTexture[m->Texture]);
				break;
			}
		}
	}

	SAFE_DELETE_ARRAY(Meshs);
	SAFE_DELETE_ARRAY(Bones);
	SAFE_DELETE_ARRAY(Actions);
	SAFE_DELETE_ARRAY(Textures);
	SAFE_DELETE_ARRAY(IndexTexture);

	NumBones   = 0;
	NumActions = 0;
	NumMeshs   = 0;
	
#ifdef LDS_FIX_SETNULLALLOCVALUE_WHEN_BMDRELEASE
	m_bCompletedAlloc = false;
#endif // LDS_FIX_SETNULLALLOCVALUE_WHEN_BMDRELEASE

}

void BMD::FindNearTriangle()
{
	for (int iMesh = 0; iMesh < NumMeshs; iMesh++)
	{
		Mesh_t* m = &Meshs[iMesh];

		Triangle_t* pTriangle = m->Triangles;
		int iNumTriangles = m->NumTriangles;
		for (int iTri = 0; iTri < iNumTriangles; ++iTri)
		{
			for (int i = 0; i < 3; ++i)
			{
				pTriangle[iTri].EdgeTriangleIndex[i] = -1;
			}
		}
		for (int iTri = 0; iTri < iNumTriangles; ++iTri)
		{
			FindTriangleForEdge(iMesh, iTri, 0);
			FindTriangleForEdge(iMesh, iTri, 1);
			FindTriangleForEdge(iMesh, iTri, 2);
		}
	}
}

void BMD::FindTriangleForEdge(int iMesh, int iTri1, int iIndex11)
{
	if (iMesh >= NumMeshs || iMesh < 0)
		return;

	Mesh_t* m = &Meshs[iMesh];
	Triangle_t* pTriangle = m->Triangles;

	Triangle_t* pTri1 = &pTriangle[iTri1];
	if (pTri1->EdgeTriangleIndex[iIndex11] != -1)
	{
		return;
	}

	int iNumTriangles = m->NumTriangles;
	for (int iTri2 = 0; iTri2 < iNumTriangles; ++iTri2)
	{
		if (iTri1 == iTri2)
		{
			continue;
		}

		Triangle_t* pTri2 = &pTriangle[iTri2];
		int iIndex12 = (iIndex11 + 1) % 3;
		for (int iIndex21 = 0; iIndex21 < 3; ++iIndex21)
		{
			int iIndex22 = (iIndex21 + 1) % 3;
			if (pTri2->EdgeTriangleIndex[iIndex21] == -1 &&
				pTri1->VertexIndex[iIndex11] == pTri2->VertexIndex[iIndex22] &&
				pTri1->VertexIndex[iIndex12] == pTri2->VertexIndex[iIndex21])
			{
				pTri1->EdgeTriangleIndex[iIndex11] = iTri2;
				pTri2->EdgeTriangleIndex[iIndex21] = iTri1;
				return;
			}
		}
	}
}

bool BMD::Open(char* DirName, char* ModelFileName)
{
	char ModelName[64];
	strcpy(ModelName, DirName);
	strcat(ModelName, ModelFileName);
	FILE* fp = fopen(ModelName, "rb");
	if (fp == nullptr)
	{
		return false;
	}
	fseek(fp, 0, SEEK_END);
	int DataBytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* Data = new unsigned char[DataBytes];
	fread(Data, 1, DataBytes, fp);
	fclose(fp);

	int Size;
	int DataPtr = 3;
	Version = Data[DataPtr];
	DataPtr += 1;
	memcpy(Name, Data + DataPtr, 32);
	DataPtr += 32;
	NumMeshs = *((short*)(Data + DataPtr));
	DataPtr += 2;
	NumBones = *((short*)(Data + DataPtr));
	DataPtr += 2;
	NumActions = *((short*)(Data + DataPtr));
	DataPtr += 2;

	Meshs = new Mesh_t[(NumMeshs > 0) ? NumMeshs : 1];
	Bones = new Bone_t[(NumBones > 0) ? NumBones : 1];
	Actions = new Action_t[(NumActions > 0) ? NumActions : 1];
	Textures = new Texture_t[(NumMeshs > 0) ? NumMeshs : 1];
	IndexTexture = new GLuint[(NumMeshs > 0) ? NumMeshs : 1];

	int i;
	for (i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];
		m->NumVertices = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NumNormals = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NumTexCoords = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NumTriangles = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->Texture = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NoneBlendMesh = false;
		m->Vertices = new Vertex_t[m->NumVertices];
		m->Normals = new Normal_t[m->NumNormals];
		m->TexCoords = new TexCoord_t[m->NumTexCoords];
		m->Triangles = new Triangle_t[m->NumTriangles];
		Size = m->NumVertices * sizeof(Vertex_t);
		memcpy(m->Vertices, Data + DataPtr, Size);
		DataPtr += Size;
		Size = m->NumNormals * sizeof(Normal_t);
		memcpy(m->Normals, Data + DataPtr, Size);
		DataPtr += Size;
		Size = m->NumTexCoords * sizeof(TexCoord_t);
		memcpy(m->TexCoords, Data + DataPtr, Size);
		DataPtr += Size;
		Size = sizeof(Triangle_t);
		int Size2 = sizeof(Triangle_t2);
		for (int j = 0; j < m->NumTriangles; j++)
		{
			memcpy(&m->Triangles[j], Data + DataPtr, Size);
			DataPtr += Size2;
		}
		memcpy(Textures[i].FileName, Data + DataPtr, 32);
		DataPtr += 32;

		TextureScriptParsing TSParsing;

		if (TSParsing.parsingTScript(Textures[i].FileName))
		{
			m->m_csTScript = new TextureScript;
			m->m_csTScript->setScript((TextureScript&)TSParsing);


			delete (TextureScript*)&TSParsing;
		}
		else
		{
			m->m_csTScript = nullptr;
		}
	}

	FindNearTriangle();

	for (i = 0; i < NumActions; i++)
	{
		Action_t* a = &Actions[i];
		a->Loop = false;
		a->NumAnimationKeys = *((short*)(Data + DataPtr));
		DataPtr += 2;
		a->LockPositions = *((bool*)(Data + DataPtr));
		DataPtr += 1;
		if (a->LockPositions)
		{
			a->Positions = new vec3_t[a->NumAnimationKeys];
			Size = a->NumAnimationKeys * sizeof(vec3_t);
			memcpy(a->Positions, Data + DataPtr, Size);
			DataPtr += Size;
		}
	}
	for (i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		b->Dummy = Data[DataPtr];
		DataPtr += 1;
		if (!b->Dummy)
		{
			memcpy(b->Name, Data + DataPtr, 32);
			DataPtr += 32;
			b->Parent = *((short*)(Data + DataPtr));
			DataPtr += 2;
			b->BoneMatrixes = new BoneMatrix_t[NumActions];
			for (int j = 0; j < NumActions; j++)
			{
				BoneMatrix_t* bm = &b->BoneMatrixes[j];
				Size = Actions[j].NumAnimationKeys * sizeof(vec3_t);
				int NumAnimationKeys = Actions[j].NumAnimationKeys;
				bm->Position = new vec3_t[NumAnimationKeys];
				bm->Rotation = new vec3_t[NumAnimationKeys];
				bm->Quaternion = new vec4_t[NumAnimationKeys];
				memcpy(bm->Position, Data + DataPtr, Size);
				DataPtr += Size;
				memcpy(bm->Rotation, Data + DataPtr, Size);
				DataPtr += Size;
				for (int k = 0; k < NumAnimationKeys; k++)
				{
					AngleQuaternion(bm->Rotation[k], bm->Quaternion[k]);
				}
			}
		}
	}
	delete[] Data;
	Init(false);
	return true;
}

bool BMD::Save(char* DirName, char* ModelFileName)
{
	char ModelName[64];
	strcpy(ModelName, DirName);
	strcat(ModelName, ModelFileName);
	FILE* fp = fopen(ModelName, "wb");
	if (fp == nullptr)
		return false;

	fputs("BMD", fp);
	fwrite(&Version, 1, 1, fp);
	fwrite(Name, sizeof(char), 32, fp);
	fwrite(&NumMeshs, sizeof(short), 1, fp);
	fwrite(&NumBones, sizeof(short), 1, fp);
	fwrite(&NumActions, sizeof(short), 1, fp);

	for (int i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];
		fwrite(&m->NumVertices, sizeof(short), 1, fp);
		fwrite(&m->NumNormals, sizeof(short), 1, fp);
		fwrite(&m->NumTexCoords, sizeof(short), 1, fp);
		fwrite(&m->NumTriangles, sizeof(short), 1, fp);
		fwrite(&m->Texture, sizeof(short), 1, fp);
		fwrite(m->Vertices, sizeof(Vertex_t), m->NumVertices, fp);
		fwrite(m->Normals, sizeof(Normal_t), m->NumNormals, fp);
		fwrite(m->TexCoords, sizeof(TexCoord_t), m->NumTexCoords, fp);
		for (int j = 0; j < m->NumTriangles; j++)
		{
			Triangle_t2 t2;
			t2.VertexIndex[0] = m->Triangles[j].VertexIndex[0];
			t2.VertexIndex[1] = m->Triangles[j].VertexIndex[1];
			t2.VertexIndex[2] = m->Triangles[j].VertexIndex[2];
			fwrite(&t2, sizeof(Triangle_t2), 1, fp);
		}
		fwrite(Textures[i].FileName, sizeof(char), 32, fp);
	}

	for (int i = 0; i < NumActions; i++)
	{
		Action_t* a = &Actions[i];
		fwrite(&a->NumAnimationKeys, sizeof(short), 1, fp);
		fwrite(&a->LockPositions, sizeof(bool), 1, fp);
		if (a->LockPositions)
			fwrite(a->Positions, sizeof(vec3_t), a->NumAnimationKeys, fp);
	}

	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		fwrite(&b->Dummy, sizeof(char), 1, fp);
		if (!b->Dummy)
		{
			fwrite(b->Name, sizeof(char), 32, fp);
			fwrite(&b->Parent, sizeof(short), 1, fp);
			for (int j = 0; j < NumActions; j++)
			{
				BoneMatrix_t* bm = &b->BoneMatrixes[j];
				fwrite(bm->Position, sizeof(vec3_t), Actions[j].NumAnimationKeys, fp);
				fwrite(bm->Rotation, sizeof(vec3_t), Actions[j].NumAnimationKeys, fp);
			}
		}
	}

	fclose(fp);
	return true;
}

bool BMD::Open2(char* DirName, char* ModelFileName, bool bReAlloc)
{
	if (true == m_bCompletedAlloc)
	{
		if (true == bReAlloc)
		{
			Release();
		}
		else
		{
			return true;
		}
	}

	char ModelName[64];
	strcpy(ModelName, DirName);
	strcat(ModelName, ModelFileName);
	FILE* fp = fopen(ModelName, "rb");
	if (fp == nullptr)
	{
		m_bCompletedAlloc = false;
		return false;
	}

	fseek(fp, 0, SEEK_END);
	int DataBytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	unsigned char* Data = new unsigned char[DataBytes];
	fread(Data, 1, DataBytes, fp);
	fclose(fp);

	int Size;
	int DataPtr = 3;
	Version = *((char*)(Data + DataPtr));
	DataPtr += 1;
	if (Version == 12)
	{
		long lSize = *((long*)(Data + DataPtr));
		DataPtr += sizeof(long);
		long lDecSize = MapFileDecrypt(nullptr, Data + DataPtr, lSize);
		BYTE* pbyDec = new BYTE[lDecSize];
		MapFileDecrypt(pbyDec, Data + DataPtr, lSize);
		delete[] Data;
		Data = pbyDec;
		DataPtr = 0;
	}

	memcpy(Name, Data + DataPtr, 32);
	DataPtr += 32;

	NumMeshs = *((short*)(Data + DataPtr));
	DataPtr += 2;
	NumBones = *((short*)(Data + DataPtr));
	DataPtr += 2;
	assert(NumBones <= MAX_BONES && "Bones 200");
	NumActions = *((short*)(Data + DataPtr));
	DataPtr += 2;

	Meshs = new Mesh_t[(NumMeshs > 0) ? NumMeshs : 1];
	Bones = new Bone_t[(NumBones > 0) ? NumBones : 1];
	Actions = new Action_t[(NumActions > 0) ? NumActions : 1];
	Textures = new Texture_t[(NumMeshs > 0) ? NumMeshs : 1];
	IndexTexture = new GLuint[(NumMeshs > 0) ? NumMeshs : 1];

	int i;

	for (i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];
		m->NumVertices = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NumNormals = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NumTexCoords = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NumTriangles = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->Texture = *((short*)(Data + DataPtr));
		DataPtr += 2;
		m->NoneBlendMesh = false;
		m->Vertices = new Vertex_t[m->NumVertices];
		m->Normals = new Normal_t[m->NumNormals];
		m->TexCoords = new TexCoord_t[m->NumTexCoords];
		m->Triangles = new Triangle_t[m->NumTriangles];
		Size = m->NumVertices * sizeof(Vertex_t);
		memcpy(m->Vertices, Data + DataPtr, Size);
		DataPtr += Size;
		Size = m->NumNormals * sizeof(Normal_t);
		memcpy(m->Normals, Data + DataPtr, Size);
		DataPtr += Size;
		Size = m->NumTexCoords * sizeof(TexCoord_t);
		memcpy(m->TexCoords, Data + DataPtr, Size);
		DataPtr += Size;
		Size = sizeof(Triangle_t);
		int Size2 = sizeof(Triangle_t2);

		for (int j = 0; j < m->NumTriangles; j++)
		{
			memcpy(&m->Triangles[j], Data + DataPtr, Size);
			DataPtr += Size2;
		}
		memcpy(Textures[i].FileName, Data + DataPtr, 32);
		DataPtr += 32;

		TextureScriptParsing TSParsing;

		if (TSParsing.parsingTScript(Textures[i].FileName))
		{
			m->m_csTScript = new TextureScript;
			m->m_csTScript->setScript((TextureScript&)TSParsing);
		}
		else
		{
			m->m_csTScript = nullptr;
		}
	}

	for (i = 0; i < NumActions; i++)
	{
		Action_t* a = &Actions[i];
		a->Loop = false;
		a->NumAnimationKeys = *((short*)(Data + DataPtr));
		DataPtr += 2;
		a->LockPositions = *((bool*)(Data + DataPtr));
		DataPtr += 1;
		if (a->LockPositions)
		{
			a->Positions = new vec3_t[a->NumAnimationKeys];
			Size = a->NumAnimationKeys * sizeof(vec3_t);
			memcpy(a->Positions, Data + DataPtr, Size);
			DataPtr += Size;
		}
		else
		{
			a->Positions = nullptr;
		}
	}

	for (i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		b->Dummy = *((char*)(Data + DataPtr));
		DataPtr += 1;
		if (!b->Dummy)
		{
			memcpy(b->Name, Data + DataPtr, 32);
			DataPtr += 32;
			b->Parent = *((short*)(Data + DataPtr));
			DataPtr += 2;
			b->BoneMatrixes = new BoneMatrix_t[NumActions];
			for (int j = 0; j < NumActions; j++)
			{
				BoneMatrix_t* bm = &b->BoneMatrixes[j];
				Size = Actions[j].NumAnimationKeys * sizeof(vec3_t);
				int NumAnimationKeys = Actions[j].NumAnimationKeys;
				bm->Position = new vec3_t[NumAnimationKeys];
				bm->Rotation = new vec3_t[NumAnimationKeys];
				bm->Quaternion = new vec4_t[NumAnimationKeys];
				memcpy(bm->Position, Data + DataPtr, Size);
				DataPtr += Size;
				memcpy(bm->Rotation, Data + DataPtr, Size);
				DataPtr += Size;
				for (int k = 0; k < NumAnimationKeys; k++)
				{
					AngleQuaternion(bm->Rotation[k], bm->Quaternion[k]);
				}
			}
		}
	}

	delete[] Data;
	Init(false);

	m_bCompletedAlloc = true;
	return true;
}

bool BMD::Save2(char* DirName, char* ModelFileName)
{
	std::string ModelName = std::string(DirName) + ModelFileName;
	FILE* fp = fopen(ModelName.c_str(), "wb");
	if (fp == nullptr)
		return false;

	putc('B', fp);
	putc('M', fp);
	putc('D', fp);
	Version = 12;
	fwrite(&Version, 1, 1, fp);

	std::vector<unsigned char> Buffer;
	Buffer.reserve(1024 * 1024);

	Buffer.insert(Buffer.end(), Name, Name + 32);
	Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&NumMeshs), reinterpret_cast<unsigned char*>(&NumMeshs) + 2);
	Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&NumBones), reinterpret_cast<unsigned char*>(&NumBones) + 2);
	Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&NumActions), reinterpret_cast<unsigned char*>(&NumActions) + 2);

	for (int i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&m->NumVertices), reinterpret_cast<unsigned char*>(&m->NumVertices) + 2);
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&m->NumNormals), reinterpret_cast<unsigned char*>(&m->NumNormals) + 2);
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&m->NumTexCoords), reinterpret_cast<unsigned char*>(&m->NumTexCoords) + 2);
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&m->NumTriangles), reinterpret_cast<unsigned char*>(&m->NumTriangles) + 2);
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&m->Texture), reinterpret_cast<unsigned char*>(&m->Texture) + 2);
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(m->Vertices), reinterpret_cast<unsigned char*>(m->Vertices) + m->NumVertices * sizeof(Vertex_t));
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(m->Normals), reinterpret_cast<unsigned char*>(m->Normals) + m->NumNormals * sizeof(Normal_t));
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(m->TexCoords), reinterpret_cast<unsigned char*>(m->TexCoords) + m->NumTexCoords * sizeof(TexCoord_t));
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(m->Triangles), reinterpret_cast<unsigned char*>(m->Triangles) + m->NumTriangles * sizeof(Triangle_t2));
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(Textures[i].FileName), reinterpret_cast<unsigned char*>(Textures[i].FileName) + 32);
	}

	for (int i = 0; i < NumActions; i++)
	{
		Action_t* a = &Actions[i];
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&a->NumAnimationKeys), reinterpret_cast<unsigned char*>(&a->NumAnimationKeys) + 2);
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&a->LockPositions), reinterpret_cast<unsigned char*>(&a->LockPositions) + 1);
		if (a->LockPositions)
		{
			Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(a->Positions), reinterpret_cast<unsigned char*>(a->Positions) + a->NumAnimationKeys * sizeof(vec3_t));
		}
	}

	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&b->Dummy),

			reinterpret_cast<unsigned char*>(&b->Dummy) + 1);
		if (!b->Dummy)
		{
			Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(b->Name), reinterpret_cast<unsigned char*>(b->Name) + 32);
			Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(&b->Parent), reinterpret_cast<unsigned char*>(&b->Parent) + 2);
			for (int j = 0; j < NumActions; j++)
			{
				BoneMatrix_t* bm = &b->BoneMatrixes[j];
				Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(bm->Position), reinterpret_cast<unsigned char*>(bm->Position) + Actions[j].NumAnimationKeys * sizeof(vec3_t));
				Buffer.insert(Buffer.end(), reinterpret_cast<unsigned char*>(bm->Rotation), reinterpret_cast<unsigned char*>(bm->Rotation) + Actions[j].NumAnimationKeys * sizeof(vec3_t));
			}
		}
	}

	long lSize = static_cast<long>(Buffer.size());
	long lEncSize = MapFileEncrypt(nullptr, Buffer.data(), lSize);
	std::vector<unsigned char> EncBuffer(lEncSize);
	MapFileEncrypt(EncBuffer.data(), Buffer.data(), lSize);

	fwrite(&lEncSize, sizeof(long), 1, fp);
	fwrite(EncBuffer.data(), lEncSize, 1, fp);
	fclose(fp);

	return true;
}

void BMD::Init(bool Dummy)
{
	if (Dummy)
	{
		for (int i = 0; i < NumBones; i++)
		{
			Bone_t* b = &Bones[i];
			b->Dummy = (b->Name[0] == 'D' && b->Name[1] == 'u');
		}
	}

	renderCount = 0;
	BoneHead = -1;
	StreamMesh = -1;
	CreateBoundingBox();
}

void BMD::CreateBoundingBox()
{
	for (int i = 0; i < NumBones; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			BoundingMin[i][j] = 9999.0;
			BoundingMax[i][j] = -9999.0;
		}
		BoundingVertices[i] = 0;
	}

	for (int i = 0; i < NumMeshs; i++)
	{
		Mesh_t* m = &Meshs[i];
		for (int j = 0; j < m->NumVertices; j++)
		{
			Vertex_t* v = &m->Vertices[j];
			for (int k = 0; k < 3; k++)
			{
				BoundingMin[v->Node][k] = std::min(BoundingMin[v->Node][k], v->Position[k]);
				BoundingMax[v->Node][k] = std::max(BoundingMax[v->Node][k], v->Position[k]);
			}
			BoundingVertices[v->Node]++;
		}
	}

	for (int i = 0; i < NumBones; i++)
	{
		Bone_t* b = &Bones[i];
		if (BoundingVertices[i] > 0)
			b->BoundingBox = true;
		else
			b->BoundingBox = false;

		Vector(BoundingMax[i][0], BoundingMax[i][1], BoundingMax[i][2], b->BoundingVertices[0]);
		Vector(BoundingMax[i][0], BoundingMax[i][1], BoundingMin[i][2], b->BoundingVertices[1]);
		Vector(BoundingMax[i][0], BoundingMin[i][1], BoundingMax[i][2], b->BoundingVertices[2]);
		Vector(BoundingMax[i][0], BoundingMin[i][1], BoundingMin[i][2], b->BoundingVertices[3]);
		Vector(BoundingMin[i][0], BoundingMax[i][1], BoundingMax[i][2], b->BoundingVertices[4]);
		Vector(BoundingMin[i][0], BoundingMax[i][1], BoundingMin[i][2], b->BoundingVertices[5]);
		Vector(BoundingMin[i][0], BoundingMin[i][1], BoundingMax[i][2], b->BoundingVertices[6]);
		Vector(BoundingMin[i][0], BoundingMin[i][1], BoundingMin[i][2], b->BoundingVertices[7]);
	}
}

BMD::~BMD()
{	
	Release();
}

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
void BMD::InterpolationTrans(float (*Mat1)[4], float (*TransMat2)[4], float _Scale)
{
	TransMat2[0][3] = TransMat2[0][3] - (TransMat2[0][3] - Mat1[0][3]) * (1-_Scale);
	TransMat2[1][3] = TransMat2[1][3] - (TransMat2[1][3] - Mat1[1][3]) * (1-_Scale);
	TransMat2[2][3] = TransMat2[2][3] - (TransMat2[2][3] - Mat1[2][3]) * (1-_Scale);
}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM