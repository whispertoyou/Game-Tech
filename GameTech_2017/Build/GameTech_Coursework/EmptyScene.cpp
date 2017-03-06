#include "EmptyScene.h"

#include <nclgl\Vector4.h>
#include <ncltech\PhysicsEngine.h>
#include <ncltech\DistanceConstraint.h>
#include <ncltech\SceneManager.h>
#include <ncltech\CommonUtils.h>

#include <ncltech\ObjectMesh.h>
#include <ncltech\CommonMeshes.h>

using namespace CommonUtils;

float EmptyScene::power = 0.0f;

EmptyScene::EmptyScene(const std::string& friendly_name)
	: Scene(friendly_name)
	, m_AccumTime(0.0f)
	, atmos(NULL)
	, m_pServerConnection(NULL)
{
}

EmptyScene::~EmptyScene()
{
	delete atmos;
}

void EmptyScene::OnInitializeScene()
{
	//Initialize Client Network
	if (m_Network.Initialize(0) && m_Network2.Initialize(0))
	{
		NCLDebug::Log("Network: Initialized!");

		//Attempt to connect to the server on localhost:1234
		m_pServerConnection = m_Network.ConnectPeer(127, 0, 0, 1, 1234);
		m_pServerConnection2 = m_Network2.ConnectPeer(127, 0, 0, 1, 1234);
		NCLDebug::Log("Network: Attempting to connect to server.");
	}

	Scene::OnInitializeScene();
	m_AccumTime = 0.0f;
	atmos = new bool(true);
	//*atmos = true;#
	debug = false;
	score = 0;

	//sphere-Planet
	auto create_ball_cubeTest = [&](const Vector3& offset, const Vector3& scale, float ballsize) {
		const int dims = 10;
		const Vector4 col = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		Vector3 pos = offset;

		Object* sphereTest = BuildPlanetObject(
			"Sun",
			TEXTUREDIR"Solar2.jpg",
			false,
			pos,
			10.0f,
			true,
			0.000001f,
			true,
			false,
			col);
		this->AddGameObject(sphereTest);
		sphereTest->Physics()->SetElasticity(0.01f);
		
	};

	

	//this->AddGameObject(CommonUtils::BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));

	//earth
	Object* earth = CommonUtils::BuildPlanetObject(
		"earth",
		TEXTUREDIR"Planet2.jpg",
		false,
		eaTra,	//Position leading to 0.1 meter overlap on faces, and more on diagonals
		2.0f,									//Radius
		true,									//Has Physics Object
		0.0001f,									//Infinite Mass
		true,									//Has Collision Shape
		false,									//Dragable by the user
		Vector4(1.0f, 1.0f, 1.0f, 1.0f));	//Color
	this->AddGameObject(earth);
	earth->Physics()->SetElasticity(0.01f);

	//moon of earth
	Object* moon = CommonUtils::BuildPlanetObject(
		"moon",
		TEXTUREDIR"Planet.png",
		false,									//sleep 
		eaTra,									//Position leading to 0.1 meter overlap on faces, and more on diagonals
		0.5f,									//Radius
		true,									//Has Physics Object
		0.01f,									//Infinite Mass
		true,									//Has Collision Shape
		false,									//Dragable by the user
		Vector4(1.0f, 1.0f, 1.0f, 1.0f));	//Color
	this->AddGameObject(moon);
	moon->Physics()->SetElasticity(0.01f);

	//Target on the sun
	Object* tarSun = CommonUtils::BuildTargetObject(
		"TarSun",
		TEXTUREDIR"target.tga",
		false,
		Vector3(0.0f, 8.f, 0.0f), 
		Vector3(1.2f, 0.2f, 1.2f), 
		true, 
		0.0f, 
		true, 
		false, 
		Vector4(0.2f, 0.5f, 1.0f, 1.0f));
	this->AddGameObject(tarSun);
	tarSun->Physics()->SetOnCollisionCallback([&](PhysicsObject* self, PhysicsObject* collidingObj) {
		Object* at1 = FindGameObject("atmosphere");
		if (collidingObj == at1->Physics()) {
			self->SetIsColliding(false);
			return false;
		}
		else {
			
		}
		return true;
	});

	//Target for player2
	Object* tar2 = CommonUtils::BuildTargetObject(
		"Tar2",
		TEXTUREDIR"target.tga",
		false,
		Vector3(0.0f, 15.f, 0.0f),
		Vector3(0.2f, 1.2f, 1.2f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 0.5f, 1.0f, 1.0f));
	this->AddGameObject(tar2);

	//floating obj
	Object* fObj = CommonUtils::BuildTargetObject(
		"fObj",
		"",
		true,
		Vector3(20.f, 20.f, -20.f),
		Vector3(1.2f, 1.2f, 1.2f),
		true,
		0.01f,
		true,
		false,
		Vector4(0.9f, 0.5f, 1.0f, 1.0f)
		);
	this->AddGameObject(fObj);

	//draw Sun
	create_ball_cubeTest(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), 0.1f);

	// Atmosphere
	Vector4 atCol(0.4f, 0.8f, 0.3f, 0.3);
	Object* atmosphere = BuildPlanetObject(
		"atmosphere",
		TEXTUREDIR"atmos.jpg",
		false,
		Vector3(0, 0, 0),
		12.0f,
		true,
		0.0f,
		true,
		false,
		atCol);
	this->AddGameObject(atmosphere);
	atmosphere->Physics()->SetOnCollisionCallback([&](PhysicsObject* self, PhysicsObject* collidingObj)
	{	
		if (*atmos) {
			Object* sun = FindGameObject("Sun");
			Object* target_sun = FindGameObject("TarSun");
			if (collidingObj == sun->Physics() || collidingObj == target_sun->Physics()) {
				return false;
			}
			else {
				collidingObj->SetLinearVelocity(collidingObj->GetLinearVelocity() * 0.5f);
				collidingObj->SetAngularVelocity(collidingObj->GetAngularVelocity() * 0.5f);
				return false;
			}
		}
		else {
			return false;
		}
	}
	);

	//vertical & horizontal plane
	Object* vPlane = CommonUtils::BuildPlaneObject(
		"vPlane",
		"",
		true,
		Vector3(-20.f, 18.7f, -20.f),
		Vector3(1.0f, 1.f, 0.f),
		true,
		0.0f,
		true,
		false,
		Vector4(0.9f, 0.5f, 1.0f, 1.0f)
		);
	this->AddGameObject(vPlane);
	vPlane->Physics()->SetOrientation(Quaternion::FromMatrix(Matrix4::Rotation(90.f, Vector3(0, 1, 0))));
	
	Object* hPlane = CommonUtils::BuildPlaneObject(
		"hPlane",
		"",
		true,
		Vector3(-19.f, 17.7f, -20.f),
		Vector3(1.f, 1.f, 0.f)/*Vector3(2.5f, 0.0f, 1.2f)*/,
		true,
		0.0f,
		true,
		false,
		Vector4(0.9f, 0.5f, 1.0f, 1.0f)
		);
	this->AddGameObject(hPlane);
	hPlane->Physics()->SetOrientation(Quaternion::FromMatrix(Matrix4::Rotation(-90.f, Vector3(1, 0, 0))));
	//Raptor===================================================
	Object* Raptor = CommonUtils::BuildRaptorObject(
		"Raptor",
		"",
		true,
		Vector3(10.f, -10.f, 10.f),
		4.0f,
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rBody = CommonUtils::BuildTargetObject(
		"rBody",
		"",
		true,
		Vector3(9.95f, -9.6f, 9.6f),
		Vector3(0.55f,0.65f,1.0f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rTail = CommonUtils::BuildTargetObject(
		"rTail",
		"",
		true,
		Vector3(9.95f, -9.45f, 11.f),
		Vector3(0.43f, 0.43f, 0.4f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rTail2 = CommonUtils::BuildTargetObject(
		"rTail2",
		"",
		true,
		Vector3(9.95f, -9.f, 13.82f),
		Vector3(0.23f, 0.23f, 2.4f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rFrontLeg = CommonUtils::BuildTargetObject(
		"rFrontLeg",
		"",
		true,
		Vector3(9.95f, -10.52f, 8.28f),
		Vector3(0.59f, 0.65f, 0.35f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rNeck = CommonUtils::BuildTargetObject(
		"rNeck",
		"",
		true,
		Vector3(9.95f, -9.12f, 8.26f),
		Vector3(0.5f, 0.45f, 0.35f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rHead = CommonUtils::BuildTargetObject(
		"rHead",
		"",
		true,
		Vector3(9.95f, -8.22f, 7.46f),
		Vector3(0.5f, 0.45f, 0.76f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);
	Object* rBackLeg = CommonUtils::BuildTargetObject(
		"rBackLeg",
		"",
		true,
		Vector3(9.95f, -11.15f, 10.17f),
		Vector3(0.55f, 0.85f, 0.5f),
		true,
		0.0f,
		true,
		false,
		Vector4(1.f, 1.f, 1.f, 1.0f)
		);

	this->AddGameObject(Raptor);
	//Comment out these to make the cubes invisible
	/*this->AddGameObject(rBody);
	this->AddGameObject(rHead);
	this->AddGameObject(rNeck);
	this->AddGameObject(rFrontLeg);
	this->AddGameObject(rBackLeg);
	this->AddGameObject(rTail);
	this->AddGameObject(rTail2);*/

	//set the main sphere(Raptor) to ignore the collision
	Raptor->Physics()->SetOnCollisionCallback([&](PhysicsObject* self, PhysicsObject* collidingObj)
	{
		//Object* sun = FindGameObject("Sun");
		//Object* target_sun = FindGameObject("TarSun");
		//if (collidingObj == sun->Physics() || collidingObj == target_sun->Physics()) {
		//	return false;
		//}
		//else {
			//collidingObj->SetLinearVelocity(collidingObj->GetLinearVelocity() * 0.9f);
			//collidingObj->SetAngularVelocity(collidingObj->GetAngularVelocity() * 0.9f);
			return false;
		//}
	}
	);

	//add constrain
	DistanceConstraint* conMain = new DistanceConstraint(
		Raptor->Physics(),					//Physics Object A
		rBody->Physics(),					//Physics Object B
		Raptor->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rBody->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conMain);

	DistanceConstraint* conNHead = new DistanceConstraint(
		rNeck->Physics(),					//Physics Object A
		rHead->Physics(),					//Physics Object B
		rNeck->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rHead->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conNHead);

	DistanceConstraint* conBNeck = new DistanceConstraint(
		rBody->Physics(),					//Physics Object A
		rNeck->Physics(),					//Physics Object B
		rBody->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rNeck->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conBNeck);

	DistanceConstraint* conBFL = new DistanceConstraint(
		rBody->Physics(),					//Physics Object A
		rFrontLeg->Physics(),					//Physics Object B
		rBody->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rFrontLeg->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conBFL);
	
	DistanceConstraint* conBBL = new DistanceConstraint(
		rBody->Physics(),					//Physics Object A
		rBackLeg->Physics(),					//Physics Object B
		rBody->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rBackLeg->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conBBL);

	DistanceConstraint* conT1T2 = new DistanceConstraint(
		rTail->Physics(),					//Physics Object A
		rTail2->Physics(),					//Physics Object B
		rTail->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rTail2->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conT1T2);

	DistanceConstraint* conBT = new DistanceConstraint(
		rBody->Physics(),					//Physics Object A
		rTail->Physics(),					//Physics Object B
		rBody->Physics()->GetPosition(),	//Attachment Position on Object A	-> Currently the centre
		rTail->Physics()->GetPosition());	//Attachment Position on Object B	-> Currently the centre  
	PhysicsEngine::Instance()->AddConstraint(conBT);

	//Raptor  end================================================
	//player 2
	player2 = CommonUtils::BuildCuboidObject(
		"Server",
		Vector3(10.f, -12.0f, -10.0f),
		Vector3(1.5f, 1.5f, 1.5f),
		true,									//Physics Enabled here Purely to make setting position easier via Physics()->SetPosition()
		0.0f,
		true,
		false,
		Vector4(1.0f, 0.f, 0.f, 1.0f));
	this->AddGameObject(player2);




	if(!col.size())
		col.clear();

	vector<Object*> objs = this->FindALLGameObj();
	//gather the color of all objs in order
	for (auto obj : objs) {
		col.push_back(obj->GetColour());
	}

}

void EmptyScene::OnCleanupScene()
{
	//Just delete all created game objects 
	//  - this is the default command on any Scene instance so we don't really need to override this function here.
	Scene::OnCleanupScene();
	player2 = NULL;
	//Send one final packet telling the server we are disconnecting
	// - We are not waiting to resend this, so if it fails to arrive
	//   the server will have to wait until we time out naturally

	if (m_pServerConnection != NULL || m_pServerConnection2 != NULL) {
		enet_peer_disconnect_now(m_pServerConnection, 0);
		enet_peer_disconnect_now(m_pServerConnection2, 0);
		//Release network and all associated data/peer connections
		m_Network.Release();
		//test
		m_Network2.Release();
	}
	
	m_pServerConnection = NULL;
	m_pServerConnection2 = NULL;
}

void EmptyScene::OnUpdateScene(float dt)
{
	m_AccumTime += dt;

	Scene::OnUpdateScene(dt);

	//Update Network1
	auto callback = std::bind(
		&EmptyScene::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	m_Network.ServiceNetwork(dt, callback);

	//Network2
	auto callback2 = std::bind(
		&EmptyScene::ProcessNetworkEvent,	// Function to call
		this,								// Associated class instance
		std::placeholders::_1);				// Where to place the first parameter
	m_Network2.ServiceNetwork(dt, callback2);


	//Add Debug Information to screen
	uint8_t ip1 = m_pServerConnection->address.host & 0xFF;
	uint8_t ip2 = (m_pServerConnection->address.host >> 8) & 0xFF;
	uint8_t ip3 = (m_pServerConnection->address.host >> 16) & 0xFF;
	uint8_t ip4 = (m_pServerConnection->address.host >> 24) & 0xFF;

	/*NCLDebug::DrawTextWs(m_pObj->Physics()->GetPosition() + Vector3(0.f, 0.5f, 0.f), 14.f, TEXTALIGN_CENTRE, Vector4(),
		"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, m_pServerConnection->address.port);*/

	Vector4 status_color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	NCLDebug::AddStatusEntry(status_color, "Network Traffic");
	NCLDebug::AddStatusEntry(status_color, "    Incoming: %5.2fKbps", m_Network.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color, "    Outgoing: %5.2fKbps", m_Network.m_OutgoingKb);

	//Add Debug Information to screen 2
	uint8_t ip5 = m_pServerConnection2->address.host & 0xFF;
	uint8_t ip6 = (m_pServerConnection2->address.host >> 8) & 0xFF;
	uint8_t ip7 = (m_pServerConnection2->address.host >> 16) & 0xFF;
	uint8_t ip8 = (m_pServerConnection2->address.host >> 24) & 0xFF;

	/*NCLDebug::DrawTextWs(m_pObj->Physics()->GetPosition() + Vector3(0.f, 0.5f, 0.f), 14.f, TEXTALIGN_CENTRE, Vector4(),
	"Peer: %u.%u.%u.%u:%u", ip1, ip2, ip3, ip4, m_pServerConnection->address.port);*/

	Vector4 status_color2 = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	NCLDebug::AddStatusEntry(status_color2, "Network Traffic 2");
	NCLDebug::AddStatusEntry(status_color2, "    Incoming: %5.2fKbps", m_Network2.m_IncomingKb);
	NCLDebug::AddStatusEntry(status_color2, "    Outgoing: %5.2fKbps", m_Network2.m_OutgoingKb);
	
	//detections
	uint drawFlags = PhysicsEngine::Instance()->GetDebugDrawFlags();

	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "Physics:");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Volumes : %s (Press C to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONVOLUMES) ? "Enabled" : "Disabled");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Collision Normals : %s (Press N to toggle)", (drawFlags & DEBUGDRAW_FLAGS_COLLISIONNORMALS) ? "Enabled" : "Disabled");
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.9f, 0.8f, 1.0f), "     Draw Manifolds : %s (Press M to toggle)", (drawFlags & DEBUGDRAW_FLAGS_MANIFOLD) ? "Enabled" : "Disabled");


	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_C))
		drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_N))
		drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONNORMALS;

	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_O))
		drawFlags ^= DEBUGDRAW_FLAGS_MANIFOLD;



	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_M)) {
		if (debugCount == 4)
			debugCount = 1;
		int i = 0;
		//adjust the transparent of each obj
		switch (debugCount)
		{
		case 1:
			drawFlags ^= DEBUGDRAW_FLAGS_COLLISIONVOLUMES;

			//debug = !debug;
			//int i = 0;
			for (auto obj : FindALLGameObj()) {
				Vector4 colour = obj->GetColour();
				//if (!debug) {
				//	//cout << " Col2 : " << col2.x << "," << col2.y << "," << col2.z << "," << col2.w << endl;//visible
				//	colour.w = col.at(i).w;
				//	obj->SetColour(colour);
				//}
				//else
				//{
					//cout <<" Col : "<< col[i].x << "," << col[i].y << "," << col[i].z << "," << col[i].w << endl;
					colour.w = 0.f;
					obj->SetColour(colour);							//invisible
				//}
				i++;
			}
			debugCount++;
			break;
		case 2:
			drawFlags ^= DEBUGDRAW_FLAGS_MANIFOLD | DEBUGDRAW_FLAGS_COLLISIONNORMALS;
			debugCount++;
			break;
		case 3:
			drawFlags ^= DEBUGDRAW_FLAGS_MANIFOLD | DEBUGDRAW_FLAGS_COLLISIONNORMALS|DEBUGDRAW_FLAGS_COLLISIONVOLUMES;
			i = 0;
			for (auto obj : FindALLGameObj()) {
				Vector4 colour = obj->GetColour();
				//if (!debug) {
					//cout << " Col2 : " << col2.x << "," << col2.y << "," << col2.z << "," << col2.w << endl;//visible
					colour.w = col.at(i).w;
					obj->SetColour(colour);
				//}
				//else
				//{
				//	//cout <<" Col : "<< col[i].x << "," << col[i].y << "," << col[i].z << "," << col[i].w << endl;
				//	colour.w = 0.f;
				//	obj->SetColour(colour);							//invisible
				//}
				i++;
			}
			debugCount++;
			break;
		}
		
	}

	PhysicsEngine::Instance()->SetDebugDrawFlags(drawFlags);

	//print the power of the bullet
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "POWER RANGE: %0.f", GetPoweredValue());
	//print the score
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "Score: %i", score);
	NCLDebug::AddStatusEntry(Vector4(1.0f, 0.4f, 0.4f, 1.0f), "Score2: %i", score2);

	Object* Sun = this->FindGameObject("Sun");
	if (Sun != NULL) {
		Sun->Physics()->SetPosition(Vector3(0, 0, 0));
		//Rotate
		Sun->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.5f, 1.0f, 0.8f), m_AccumTime * 5.0f));
	}

	//Get the orientation and set a specific point for it
	//Quaternion Or = Sun->Physics()->GetOrientation();
	Matrix3 Or = Sun->Physics()->GetOrientation().ToMatrix3();
	Vector3 P = Vector3(0, 11, 0);

	//Or = Or*P*Or.Conjugate();
	P = Or*P;

	//adjust power of bullet
	if (Window::GetKeyboard()->KeyHeld(KEYBOARD_K)){
		power++;
		powered = true;
	}
	//texture on/off
	/*ObjectMesh* Suntx = (ObjectMesh*)this->FindGameObject("Sun");
	if(Window::GetKeyboard()->KeyTriggered(KEYBOARD_O)){
		*textOn = !(*textOn);
		if (*textOn) {
			Suntx->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"Solar2.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT),true);
		}	
		else {
			Suntx->SetTexture(CommonMeshes::CheckerboardTex(), true);
		}
	}*/

	//atmosphere on/off
	Vector4 atCol(0.4f, 0.8f, 0.3f, 0.3);
	Vector4 atCol2(1.f, 1.f, 1.f, 0.0);
	Object* at = this->FindGameObject("atmosphere");
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_I)) {
		*atmos = !(*atmos);
		if (*atmos) {
			at->SetColour(atCol);
		}
		else {
			at->SetColour(atCol2);
		}
	}


		/*for (int i = 0;i < col.size();i++) {
			cout << col[i].x <<","<< col[i].y << "," <<col[i].z << ","<< col[i].w << endl;
		}*/
	//}

	//Emitter
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
	{	
		Camera* cam = SceneManager::Instance()->GetCamera();
		Matrix4 viewMtx = cam->BuildViewMatrix();
		Vector3 forward = -Vector3(viewMtx[2], viewMtx[6], viewMtx[10]);

		Object* sphere = CommonUtils::BuildSphereObject("",
			cam->GetPosition(),					//Position leading to 0.1 meter overlap on faces, and more on diagonals
			0.1f,									//Radius
			true,									//Has Physics Object
			0.01f,									//Infinite Mass
			true,									//Has Collision Shape
			false,									//Dragable by the user
			Vector4(1.f, 0.f, 0.f, 1.f)/*CommonUtils::GenColour(0.3f, 0.5f)*/);	//Color

		sphere->Physics()->SetLinearVelocity(forward * (powered ? power : 10.f));
		this->AddGameObject(sphere);
		powered = false;
		power = 0.f;
		col.push_back(Vector4(1.f, 0.f, 0.f, 1.f));
	}
	//Emitter2
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_T))
	{
		/*cam2 = new Camera(20.f, 120.f,player2->Physics()->GetPosition());
		Matrix4 viewMtx = cam2->BuildViewMatrix();*/
		Vector3 forward = -(player2->Physics()->GetPosition());

		Object* sphere = CommonUtils::BuildSphereObject("",
			player2->Physics()->GetPosition()+Vector3(-0.15, 0,0),					//Position leading to 0.1 meter overlap on faces, and more on diagonals
			0.1f,									//Radius
			true,									//Has Physics Object
			0.01f,									//Infinite Mass
			true,									//Has Collision Shape
			false,									//Dragable by the user
			Vector4(0.f, 1.f, 1.f, 1.f)/*CommonUtils::GenColour(0.3f, 0.5f)*/);	//Color

		sphere->Physics()->SetLinearVelocity(forward * (powered ? power : 1.f));
		this->AddGameObject(sphere);
		powered = false;
		power = 0.f;
		col.push_back(Vector4(1.f, 0.f, 0.f, 1.f));
	}
	//take control of player2
	const float mv_speed = 10.f * dt;			//Motion: Meters per second
	const float rot_speed = 90.f * dt;			//Rotation: Degrees per second

	PhysicsObject* pobj = player2->Physics();
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP))
	{
		pobj->SetPosition(pobj->GetPosition() +
			pobj->GetOrientation().ToMatrix3() * Vector3(0.0f, 0.0f, -mv_speed));
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
	{
		pobj->SetPosition(pobj->GetPosition() +
			pobj->GetOrientation().ToMatrix3()* Vector3(0.0f, 0.0f, mv_speed));
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
	{
		pobj->SetPosition(pobj->GetPosition() +
			pobj->GetOrientation().ToMatrix3()* Vector3(-mv_speed, 0.0f,  0.0f));
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
	{
		pobj->SetPosition(pobj->GetPosition() +
			pobj->GetOrientation().ToMatrix3()* Vector3(mv_speed, 0.0f, 0.0f));
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD0))
	{
		pobj->SetPosition(pobj->GetPosition() +
			pobj->GetOrientation().ToMatrix3()* Vector3( 0.0f, -mv_speed, 0.0f));
	}
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_NUMPAD1))
	{
		pobj->SetPosition(pobj->GetPosition() +
			pobj->GetOrientation().ToMatrix3()* Vector3(0.0f, mv_speed, 0.0f));
	}



	//Earth
	Object* Earth = this->FindGameObject("earth");
	//Earth->Physics()->SetPosition(sunTra);
	if (Earth != NULL)
	{
		//Move orbiting sphere1 around centre object at 45 degrees per second with an orbiting radius of 75cm
		Earth->Physics()->SetPosition(Vector3(
			eaTra.x + cos(DegToRad(m_AccumTime * 5.0f) * 2.f) * 25.f,
			eaTra.y,
			eaTra.z + sin(DegToRad(m_AccumTime * 5.0f) * 2.f) * 22.f));
		//Rotate
		Earth->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.0f, 1.0f, 0.0f), m_AccumTime * 45.0f));
	}
	//Moon
	Object* Moon = this->FindGameObject("moon");
	if (Moon != NULL)
	{
		Vector3 ePos = Earth->Physics()->GetPosition();
		//Move orbiting sphere1 around centre object at 45 degrees per second with an orbiting radius of 75cm
		Moon->Physics()->SetPosition(Vector3(
			ePos.x + cos(DegToRad(m_AccumTime * 65.0f) * 2.f) * 4.75f,
			ePos.y,
			ePos.z + sin(DegToRad(m_AccumTime * 65.0f) * 2.f) * 4.75f));
	}
	//Target for 100 points
	Object* TarSun = this->FindGameObject("TarSun");
	if (TarSun != NULL) {
		TarSun->Physics()->SetPosition(P);
		//Rotate
		TarSun->Physics()->SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0.5f, 1.0f, 0.8f), m_AccumTime * 5.0f));
		//calculate
		if (TarSun->Physics()->IsColliding()) {
			score += 100;
			/*string s = std::to_string(score);
			char const* score_text = s.c_str();

			char buffer[256];
			strncpy_s(buffer, score_text, sizeof(buffer));

			ENetPacket* packet = enet_packet_create(buffer, sizeof(buffer), 0);
			enet_peer_send(m_pServerConnection, 0, packet);*/
		}
	}
	Object* Tar2 = this->FindGameObject("Tar2");
	if (Tar2 != NULL) {
		Tar2->Physics()->SetPosition(Vector3(0.0f, 15.f, 0.0f));
		if (Tar2->Physics()->IsColliding())
			score2 += 100;
	}


	//show the scores
	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_U)) {
		string s = std::to_string(score);
		//char* text_data = "Player 1's Score: ";
		char const* score_text = s.c_str();

		char buffer[256];
		strncpy_s(buffer, score_text, sizeof(buffer));
		//strncpy_s(buffer, text_data, sizeof(buffer));
		//strncat_s(buffer, score_text, sizeof(buffer));


		ENetPacket* packet = enet_packet_create(buffer, sizeof(buffer), 0);
		enet_peer_send(m_pServerConnection, 0, packet);
		//player 2
		/*string s2 = std::to_string(score2);
		char* text_data2 = "Player 2's Score: ";
		char const* score_text2 = s.c_str();

		char buffer2[256];
		strncpy_s(buffer2, text_data2, sizeof(buffer2));
		strncat_s(buffer2, score_text2, sizeof(buffer2));


		ENetPacket* packet2 = enet_packet_create(buffer2, sizeof(buffer2), 0);
		enet_peer_send(m_pServerConnection2, 0, packet2);*/
	}

	
	//press X to offline
	//if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_X)) {
	//	if (m_pServerConnection != NULL) {
	//		enet_peer_disconnect_now(m_pServerConnection, 0);

	//		//Release network and all associated data/peer connections
	//		m_Network.Release();
	//	}

	//	m_pServerConnection = NULL;
	//}

	//Object* fObj = this->FindGameObject("fObj");
	//Vector4 fObjCol(0.9f, 0.5f, 1.0f, 1.0f);
	//if (fObj != NULL) {
	//	if (fObj->Physics()->IsColliding()) {
	//		fObj->Physics()->SetSleepMode(false);
	//		//fObj->OnUpdateObject(m_AccumTime);
	//		//cout << "Sleeping MOD: " << fObj->Physics()->GetSleepMode() << endl;
	//	}
	//	fObj->SetColour(fObj->Physics()->IsColliding() ? fObjCol : Vector4(1.f, 1.f, 1.f, 1.f));
	//	
	//}
}

//networking
void EmptyScene::ProcessNetworkEvent(const ENetEvent& evnt)
{
	switch (evnt.type)
	{
		//New connection request or an existing peer accepted our connection request
	case ENET_EVENT_TYPE_CONNECT:
	{
		if (evnt.peer == m_pServerConnection)
		{
			NCLDebug::Log("Network: Successfully connected to server!");

			//Send a 'hello' packet
			char* text_data = "My Score Now is : ";
			std::string scr = std::to_string(score);
			char const* pchar = scr.c_str();
			
			//combine two chars
			char buffer[256];
			strncpy_s(buffer, text_data, sizeof(buffer));
			strncat_s(buffer, pchar, sizeof(buffer));
			
			
			ENetPacket* packet = enet_packet_create(buffer, sizeof(buffer) + 1, 0);
			enet_peer_send(m_pServerConnection, 0, packet);
		}
		else if (evnt.peer == m_pServerConnection2) {
			NCLDebug::Log("Network 2: Successfully connected to server!");
			//Send a 'hello' packet
			char* text_data = "Hellooo! I'm player 2!";
			ENetPacket* packet = enet_packet_create(text_data, strlen(text_data) + 1, 0);
			enet_peer_send(m_pServerConnection2, 0, packet);
		}
	}
	break;


	//Server has sent us a new packet
	case ENET_EVENT_TYPE_RECEIVE:
	{
		if (evnt.packet->dataLength == sizeof(Vector3))
		{
			//Vector3 pos;
			//memcpy(&score, evnt.packet->data, sizeof(float));
			//m_pObj->Physics()->SetPosition(pos);
		}
		else
		{
			NCLERROR("Recieved Invalid Network Packet!");
		}

	}
	break;


	//Server has disconnected
	case ENET_EVENT_TYPE_DISCONNECT:
	{
		NCLDebug::Log("Network: Server has disconnected!");
	}
	break;
	}
}