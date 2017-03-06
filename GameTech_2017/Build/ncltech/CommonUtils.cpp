#include "CommonUtils.h"

#include "ObjectMesh.h"
#include "ObjectMeshDragable.h"
#include "SphereCollisionShape.h"
#include "CuboidCollisionShape.h"
#include "CommonMeshes.h"

Vector4 CommonUtils::GenColour(float scalar, float alpha)
{
	Vector4 c;
	c.w = alpha;

	float t;
	c.x = abs(modf(scalar + 1.0f, &t) * 6.0f - 3.0f) - 1.0f;
	c.y = abs(modf(scalar + 2.0f / 3.0f, &t) * 6.0f - 3.0f) - 1.0f;
	c.z = abs(modf(scalar + 1.0f / 3.0f, &t) * 6.0f - 3.0f) - 1.0f;

	c.x = min(max(c.x, 0.0f), 1.0f);
	c.y = min(max(c.y, 0.0f), 1.0f);
	c.z = min(max(c.z, 0.0f), 1.0f);

	return c;
}

Object* CommonUtils::BuildSphereObject(
	const std::string& name,
	const Vector3& pos,
	float radius,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pSphere = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pSphere->SetMesh(CommonMeshes::Sphere(), false);
	pSphere->SetTexture(CommonMeshes::CheckerboardTex(), false);
	pSphere->SetLocalTransform(Matrix4::Scale(Vector3(radius, radius, radius)));
	pSphere->SetColour(color);

	pSphere->SetBoundingRadius(radius);

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pSphere->SetLocalTransform(Matrix4::Translation(pos) * pSphere->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pSphere->CreatePhysicsNode();

		pSphere->Physics()->SetPosition(pos);
		pSphere->Physics()->SetInverseMass(inverse_mass);
		
		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pSphere->Physics()->SetInverseInertia(SphereCollisionShape(radius).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new SphereCollisionShape(radius);
			pSphere->Physics()->SetCollisionShape(pColshape);
			pSphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}			
	}

	return pSphere;
}

Object* CommonUtils::BuildCuboidObject(
	const std::string& name,
	const Vector3& pos,
	const Vector3& halfdims,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pCuboid = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pCuboid->SetMesh(CommonMeshes::Cube(), false);
	pCuboid->SetTexture(CommonMeshes::CheckerboardTex(), false);
	pCuboid->SetLocalTransform(Matrix4::Scale(halfdims));
	pCuboid->SetColour(color);
	pCuboid->SetBoundingRadius(halfdims.Length());

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pCuboid->SetLocalTransform(Matrix4::Translation(pos) * pCuboid->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pCuboid->CreatePhysicsNode();
		pCuboid->Physics()->SetPosition(pos);
		pCuboid->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pCuboid->Physics()->SetInverseInertia(CuboidCollisionShape(halfdims).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new CuboidCollisionShape(halfdims);
			//CollisionShape* pColshape = new SphereCollisionShape(0.2f);
			pCuboid->Physics()->SetCollisionShape(pColshape);
			pCuboid->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pCuboid;
}

Object* CommonUtils::BuildPlanetObject(
	const std::string & name, 
	const std::string & texName, 
	const bool sleep, 
	const Vector3 & pos, 
	float radius, 
	bool physics_enabled, 
	float inverse_mass, 
	bool collidable, 
	bool dragable, 
	const Vector4 & color)
{
	ObjectMesh* pSphere = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pSphere->SetMesh(CommonMeshes::Sphere(), false);
	GLuint planettex;
	//texture
	if (texName != "")
		planettex = SOIL_load_OGL_texture(texName.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	else
		planettex = CommonMeshes::CheckerboardTex();
	

	pSphere->SetTexture(planettex, true);
	pSphere->SetLocalTransform(Matrix4::Scale(Vector3(radius, radius, radius)));
	pSphere->SetColour(color);

	pSphere->SetBoundingRadius(radius);

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pSphere->SetLocalTransform(Matrix4::Translation(pos) * pSphere->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pSphere->CreatePhysicsNode();
		//set sleep
		pSphere->Physics()->SetSleepMode(sleep);

		pSphere->Physics()->SetPosition(pos);
		pSphere->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pSphere->Physics()->SetInverseInertia(SphereCollisionShape(radius).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new SphereCollisionShape(radius);
			pSphere->Physics()->SetCollisionShape(pColshape);
			pSphere->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pSphere;
}

Object* CommonUtils::BuildTargetObject(
	const std::string& name,
	const std::string& texName,
	const bool sleep,
	const Vector3& pos,
	const Vector3& halfdims,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pCuboid = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pCuboid->SetMesh(CommonMeshes::Cube(), false);
	GLuint tartex;
	
	//texture
	if (texName != "")
		tartex = SOIL_load_OGL_texture(texName.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	else
		tartex = CommonMeshes::CheckerboardTex();

	pCuboid->SetTexture(tartex, true);


	pCuboid->SetLocalTransform(Matrix4::Scale(halfdims));
	
	pCuboid->SetColour(color);
	pCuboid->SetBoundingRadius(halfdims.Length());

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pCuboid->SetLocalTransform(Matrix4::Translation(pos) * pCuboid->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pCuboid->CreatePhysicsNode();

		//set sleep
		pCuboid->Physics()->SetSleepMode(sleep);

		pCuboid->Physics()->SetPosition(pos);
		pCuboid->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pCuboid->Physics()->SetInverseInertia(CuboidCollisionShape(halfdims).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new CuboidCollisionShape(halfdims);
			//CollisionShape* pColshape = new SphereCollisionShape(0.2f);
			pCuboid->Physics()->SetCollisionShape(pColshape);
			pCuboid->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pCuboid;
}

//Raptor Object
Object* CommonUtils::BuildRaptorObject(
	const std::string & name,
	const std::string & texName,
	const bool sleep,
	const Vector3 & pos,
	float radius,
	bool physics_enabled,
	float inverse_mass,
	bool collidable,
	bool dragable,
	const Vector4 & color)
{
	ObjectMesh* pRaptor = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pRaptor->SetMesh(CommonMeshes::Raptor(), false);
	GLuint planettex;
	//texture
	if (texName != "")
		planettex = SOIL_load_OGL_texture(texName.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	else
		planettex = CommonMeshes::RaptorTex();


	pRaptor->SetTexture(planettex, true);
	pRaptor->SetLocalTransform(Matrix4::Scale(Vector3(radius, radius, radius)));
	pRaptor->SetColour(color);

	pRaptor->SetBoundingRadius(radius);

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pRaptor->SetLocalTransform(Matrix4::Translation(pos) * pRaptor->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pRaptor->CreatePhysicsNode();
		//set sleep
		pRaptor->Physics()->SetSleepMode(sleep);

		pRaptor->Physics()->SetPosition(pos);
		pRaptor->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pRaptor->Physics()->SetInverseInertia(SphereCollisionShape(radius).BuildInverseInertia(inverse_mass));
		}
		else
		{
			CollisionShape* pColshape = new SphereCollisionShape(radius);
			pRaptor->Physics()->SetCollisionShape(pColshape);
			pRaptor->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pRaptor;
}

Object* CommonUtils::BuildPlaneObject(
	const std::string& name,
	const std::string& texName,
	const bool sleep,
	const Vector3& pos,
	const Vector3& halfdims,
	bool physics_enabled,
	float inverse_mass,			//requires physics_enabled = true
	bool collidable,				//requires physics_enabled = true
	bool dragable,
	const Vector4& color)
{
	ObjectMesh* pCuboid = dragable
		? new ObjectMeshDragable(name)
		: new ObjectMesh(name);

	pCuboid->SetMesh(CommonMeshes::Plane(), false);
	GLuint tartex;

	//texture
	if (texName != "")
		tartex = SOIL_load_OGL_texture(texName.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	else
		tartex = CommonMeshes::CheckerboardTex();

	pCuboid->SetTexture(tartex, false);


	pCuboid->SetLocalTransform(Matrix4::Translation(Vector3(-1, -1, 0)) * Matrix4::Scale(halfdims*2));

	pCuboid->SetColour(color);
	pCuboid->SetBoundingRadius(halfdims.Length());

	if (!physics_enabled)
	{
		//If no physics object is present, just set the local transform (modelMatrix) directly
		pCuboid->SetLocalTransform(Matrix4::Translation(pos) * pCuboid->GetLocalTransform());
	}
	else
	{
		//Otherwise create a physics object, and set it's position etc
		pCuboid->CreatePhysicsNode();

		//set sleep
		pCuboid->Physics()->SetSleepMode(sleep);

		pCuboid->Physics()->SetPosition(pos);
		pCuboid->Physics()->SetInverseMass(inverse_mass);

		if (!collidable)
		{
			//Even without a collision shape, the inertia matrix for rotation has to be derived from the objects shape
			pCuboid->Physics()->SetInverseInertia(CuboidCollisionShape(halfdims).BuildInverseInertia(inverse_mass));
		}
		else
		{
			Vector3 n_halfdims = halfdims;
			n_halfdims.z = 0.05f;
			CollisionShape* pColshape = new CuboidCollisionShape(n_halfdims);
			//CollisionShape* pColshape = new SphereCollisionShape(0.2f);
			pCuboid->Physics()->SetCollisionShape(pColshape);
			pCuboid->Physics()->SetInverseInertia(pColshape->BuildInverseInertia(inverse_mass));
		}
	}

	return pCuboid;
}