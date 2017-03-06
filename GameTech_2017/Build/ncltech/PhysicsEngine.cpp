#include "PhysicsEngine.h"
#include "Object.h"
#include "CollisionDetectionSAT.h"
#include "NCLDebug.h"
#include <nclgl\Window.h>
#include <omp.h>


void PhysicsEngine::SetDefaults()
{
	m_DebugDrawFlags = NULL;
	m_IsPaused = false;
	m_UpdateTimestep = 1.0f / 60.f;
	m_UpdateAccum = 0.0f;
	m_Gravity = Vector3(0.0f, -9.81f, 0.0f);
	m_DampingFactor = 0.999f;
}

PhysicsEngine::PhysicsEngine()
{
	SetDefaults();
}

PhysicsEngine::~PhysicsEngine()
{
	RemoveAllPhysicsObjects();
}

void PhysicsEngine::AddPhysicsObject(PhysicsObject* obj)
{
	m_PhysicsObjects.push_back(obj);
}

void PhysicsEngine::RemovePhysicsObject(PhysicsObject* obj)
{
	//Lookup the object in question
	auto found_loc = std::find(m_PhysicsObjects.begin(), m_PhysicsObjects.end(), obj);

	//If found, remove it from the list
	if (found_loc != m_PhysicsObjects.end())
	{
		m_PhysicsObjects.erase(found_loc);
	}
}

void PhysicsEngine::RemoveAllPhysicsObjects()
{
	//Delete and remove all constraints/collision manifolds
	for (Constraint* c : m_vpConstraints)
	{
		delete c;
	}
	m_vpConstraints.clear();

	for (Manifold* m : m_vpManifolds)
	{
		delete m;
	}
	m_vpManifolds.clear();


	//Delete and remove all physics objects
	// - we also need to inform the (possible) associated game-object
	//   that the physics object no longer exists
	for (PhysicsObject* obj : m_PhysicsObjects)
	{
		if (obj->m_pParent != NULL) obj->m_pParent->m_pPhysicsObject = NULL;
		delete obj;
	}
	m_PhysicsObjects.clear();
}


void PhysicsEngine::Update(float deltaTime)
{
	const int max_updates_per_frame = 5;

	if (!m_IsPaused)
	{
		m_UpdateAccum += deltaTime;
		for (int i = 0; (m_UpdateAccum >= m_UpdateTimestep) && i < max_updates_per_frame; ++i)
		{
			m_UpdateAccum -= m_UpdateTimestep;
			if (!m_IsPaused) UpdatePhysics(); //Additional check here incase physics was paused mid-update and the contents of the physics need to be displayed
		}

		if (m_UpdateAccum >= m_UpdateTimestep)
		{
			NCLERROR("Physics too slow to run in real time!");
			//Drop Time in the hope that it can continue to run in real-time
			m_UpdateAccum = 0.0f;
		}
	}
}


void PhysicsEngine::UpdatePhysics()
{
	for (Manifold* m : m_vpManifolds)
	{
		delete m;
	}
	m_vpManifolds.clear();

	//Check for collisions
	BroadPhaseCollisions();
	NarrowPhaseCollisions();

	//Solve collision constraints
	SolveConstraints();

	//check if it is sleeping
	for (PhysicsObject* obj : m_PhysicsObjects)
	{	
		if(!obj->GetSleepMode())
			//Update movement
			UpdatePhysicsObject(obj);
	}
}


void PhysicsEngine::SolveConstraints()
{
	//Optional step to allow constraints to 
	// precompute values based off current velocities 
	// before they are updated in the main loop below.
	for (Manifold* m : m_vpManifolds)		m->PreSolverStep(m_UpdateTimestep);
	for (Constraint* c : m_vpConstraints)	c->PreSolverStep(m_UpdateTimestep);

	// Solve all Constraints and Collision Manifolds
	for (size_t i = 0; i < SOLVER_ITERATIONS; ++i) {
		for (Manifold* m : m_vpManifolds)		m->ApplyImpulse();
		for (Constraint* c : m_vpConstraints)	c->ApplyImpulse();
	}
}


void PhysicsEngine::UpdatePhysicsObject(PhysicsObject* obj)
{	
	/* TUTORIAL 2 */
	//Midpoint Method=======================================================
	Vector3 gForce = Vector3(0, 0, 0);
	//coursework gravity
	for (PhysicsObject* pObj : m_PhysicsObjects) {
		if (obj != pObj && pObj->GetInverseMass() > 0.f) {
			float dis = (pObj->GetPosition() - obj->GetPosition()).LengthSquared();
			Vector3 dir = pObj->GetPosition() - obj->GetPosition();
			dir.Normalise();
			gForce += dir*(GRVITY_PLANET / ( pObj->m_InvMass * dis));
		}
	}

	Vector3 acceleration = Vector3(0.f, 0.f, 0.f);
	//tutorials 
	/*if (obj->m_InvMass > 0.0f)
		acceleration = obj->m_Force * obj->m_InvMass + m_Gravity;*/

	//coursework's acceleration  
	if(obj->GetInverseMass() > 0.f)
		acceleration = obj->m_Force * obj->m_InvMass + gForce;
	
	Vector3 tempV = obj->m_LinearVelocity;
	Vector3 tempVhalf = tempV + (acceleration * m_UpdateTimestep) / 2.0f;



	/*if (obj->m_InvMass > 0.0f)
		tempVhalf += m_Gravity * m_UpdateTimestep;*/

	

	tempVhalf = tempVhalf * m_DampingFactor;

	obj->m_Position += tempVhalf * m_UpdateTimestep;
	//Midpoint Method end===================================================

	// Apply Gravity
	// Technically gravity here is calculated by formula :
	// ( m_Gravity / invMass * invMass * dt )
	// So even though the divide and multiply cancel out , we still need
	// to handle the possibility of divide by zero .
	/*if (obj -> m_InvMass > 0.0f)
	obj -> m_LinearVelocity += m_Gravity * m_UpdateTimestep;*/

	
	// - See " Update Position " below
	obj -> m_LinearVelocity += acceleration 
		* m_UpdateTimestep;
	
	// Apply Velocity Damping
	// - This removes a tiny bit of energy from the simulation each
	// update to stop slight calculation errors accumulating and adding
	// force from nowhere .
	// - In it ’s present form this can be seen as a rough approximation
	// of air resistance , albeit wrongly making the assumption that all
	// objects have the same surface area .
	obj -> m_LinearVelocity = obj -> m_LinearVelocity * m_DampingFactor;
	
	// Update Position
	// - Euler integration , works on the assumption that linearvelocity
	// does not change over time (or changes so slightly it doesnt make
	// a difference ).
	// - In this scenario , gravity / will / be increasing velocity over
	// time . The inaccuracy of not taking into account of these changes
	// over time can be visibly seen in the Phy2_Integration Test
	// Scene .. and thus how better integration schemes lead to better
	// approximations by taking into account of curvature .
	//obj -> m_Position += obj -> m_LinearVelocity * m_UpdateTimestep;
	

	
	
	
	// Angular Rotation
	// - These are the exact same calculations as the three lines above
	// except for rotations rather than positions .
	// - Mass -> Torque
	// - Velocity -> Rotational Velocity
	// - Position -> Orientation
	obj -> m_AngularVelocity += obj -> m_InvInertia
		* obj -> m_Torque * m_UpdateTimestep;
	
	
	// Apply Velocity Damping
	obj -> m_AngularVelocity = obj -> m_AngularVelocity * m_DampingFactor;
	
	
	// Update Orientation
	// - This is slightly different calculation due to the weirdness of
	// quaternions .
	// - If you are interested in it ’s derivation , there is lots of
	// stuff online about it. Though in essence , i t s the fastest
	// approximation for updating orientation , and the smaller
	// timestep / velocity is the more accurate it is - so fits nicely
	// with Euler above . ;)
	obj -> m_Orientation = obj -> m_Orientation + obj -> m_Orientation
	* (obj -> m_AngularVelocity * m_UpdateTimestep * 0.5f);
	obj -> m_Orientation.Normalise();
	
	
	// Finally invalidate the world - transform matrix .
	// - The next time it is requested ; it will be rebuilt from scratch
	// with the new position / orientation we set above .
	obj -> m_wsTransformInvalidated = true;



}


void PhysicsEngine::BroadPhaseCollisions()
{
	m_BroadphaseCollisionPairs.clear();

	PhysicsObject *m_pObj1, *m_pObj2;
	//	The broadphase needs to build a list of all potentially colliding objects in the world,
	//	which then get accurately assesed in narrowphase. If this is too coarse then the system slows down with
	//	the complexity of narrowphase collision checking, if this is too fine then collisions may be missed.


	//	Brute force approach.
	//  - For every object A, assume it could collide with every other object.. 
	//    even if they are on the opposite sides of the world.
	if (m_PhysicsObjects.size() > 0)
	{
		for (size_t i = 0; i < m_PhysicsObjects.size() - 1; ++i)
		{
			for (size_t j = i + 1; j < m_PhysicsObjects.size(); ++j)
			{
				m_pObj1 = m_PhysicsObjects[i];
				m_pObj2 = m_PhysicsObjects[j];

				////Check they both atleast have collision shapes
				if (m_pObj1->GetCollisionShape() != NULL
					&& m_pObj2->GetCollisionShape() != NULL)
				{
					//	CollisionPair cp;
					//	cp.pObjectA = m_pObj1;
					//	cp.pObjectB = m_pObj2;
					//	m_BroadphaseCollisionPairs.push_back(cp);

					//Check they both atleast have collision shapes(sphere-sphere interface)
					
					bool sp_sp = SphereSphereInterface(m_pObj1, m_pObj2, m_pObj1->GetCollisionShape(), m_pObj2->GetCollisionShape());

					if (sp_sp)
					{
						//m_pObj1->GetAssociatedObject()->SetColour(Vector4(1.0f,0.0f,0.0f,1));
						CollisionPair cp;
						cp.pObjectA = m_pObj1;
						cp.pObjectB = m_pObj2;
						m_BroadphaseCollisionPairs.push_back(cp);
					}
				}
			}
		}
	}
}


void PhysicsEngine::NarrowPhaseCollisions()
{
	for (auto* obj : m_PhysicsObjects)
	{
		obj->m_IsColliding = false;
	}

	if (m_BroadphaseCollisionPairs.size() > 0)
	{
		//Collision data to pass between detection and manifold generation stages.
		CollisionData colData;				

		//Collision Detection Algorithm to use
		CollisionDetectionSAT colDetect;	

		// Iterate over all possible collision pairs and perform accurate collision detection
		for (size_t i = 0; i < m_BroadphaseCollisionPairs.size(); ++i)
		{
			CollisionPair& cp = m_BroadphaseCollisionPairs[i];

			CollisionShape *shapeA = cp.pObjectA->GetCollisionShape();
			CollisionShape *shapeB = cp.pObjectB->GetCollisionShape();

			colDetect.BeginNewPair(
				cp.pObjectA,
				cp.pObjectB,
				cp.pObjectA->GetCollisionShape(),
				cp.pObjectB->GetCollisionShape());

			//--TUTORIAL 4 CODE--
			// Detects if the objects are colliding - Seperating Axis Theorem
			if (colDetect.AreColliding(&colData))
			{
				cp.pObjectA->m_IsColliding = true;
				cp.pObjectB->m_IsColliding = true;
				//set sleep
				cp.pObjectA->SetSleepMode(false);
				cp.pObjectB->SetSleepMode(false);

				//Draw collision data to the window if requested
				// - Have to do this here as colData is only temporary. 
				if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS)
				{
					NCLDebug::DrawPointNDT(colData._pointOnPlane, 0.1f, Vector4(0.5f, 0.5f, 1.0f, 1.0f));
					NCLDebug::DrawThickLineNDT(colData._pointOnPlane, colData._pointOnPlane - colData._normal * colData._penetration, 0.05f, Vector4(0.0f, 0.0f, 1.0f, 1.0f));
				}

				//Check to see if any of the objects have collision callbacks that dont want the objects to physically collide
				bool okA = cp.pObjectA->FireOnCollisionEvent(cp.pObjectA, cp.pObjectB);
				bool okB = cp.pObjectB->FireOnCollisionEvent(cp.pObjectB, cp.pObjectA);

				if (okA && okB)
				{
					//-- TUTORIAL 5 CODE --
					// Build full collision manifold that will also handle the collision response between the two objects in the solver stage
					Manifold* manifold = new Manifold();
					manifold->Initiate(cp.pObjectA, cp.pObjectB);

					// Construct contact points that form the perimeter of the collision manifold
					colDetect.GenContactPoints(manifold);

					// Add to list of manifolds that need solving
					m_vpManifolds.push_back(manifold);
				}
			}
		}

	}
}

bool PhysicsEngine::SphereSphereInterface(PhysicsObject* obj1, PhysicsObject* obj2, CollisionShape* shape1, CollisionShape* shape2) {
	// Check that both shapes are Spheres
	
	//SphereCollisionShape * sphere1 =
	//	dynamic_cast <SphereCollisionShape * >(shape1);
	//SphereCollisionShape * sphere2 =
	//	dynamic_cast <SphereCollisionShape * >(shape2);
	//
	//if (sphere1 == NULL || sphere2 == NULL)
	//{
	//	return false; // At least one object isn ’t a sphere !
	//}
	
	// Sphere - Sphere Check
	float sum_radius = shape1 -> GetRadius() + shape2 -> GetRadius();
	float sum_radius_squared = sum_radius * sum_radius;
	
	Vector3 ab = obj2->GetPosition() - obj1->GetPosition();
	float distance_squared = Vector3::Dot(ab, ab);
	
	return (distance_squared <= sum_radius_squared);
	// True if distance between centre points is less or equal
	// to the sum of the two radii
}


void PhysicsEngine::DebugRender()
{
	// Draw all collision manifolds
	if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_MANIFOLD)
	{
		for (Manifold* m : m_vpManifolds)
		{
			m->DebugDraw();
		}
	}

	// Draw all constraints
	if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_CONSTRAINT)
	{
		for (Constraint* c : m_vpConstraints)
		{
			c->DebugDraw();
		}
	}

	// Draw all associated collision shapes
	if (m_DebugDrawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES)
	{
		for (PhysicsObject* obj : m_PhysicsObjects)
		{
			if (obj->GetCollisionShape() != NULL)
			{
				obj->GetCollisionShape()->DebugDraw(obj);
			}
		}
	}
}