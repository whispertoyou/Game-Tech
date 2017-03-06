#pragma once

#include <nclgl\Mesh.h>
#include <ncltech\Scene.h>
#include <ncltech\CommonUtils.h>
//networking
#include <ncltech\NetworkBase.h>
#include <enet/enet.h>

class EmptyScene : public Scene
{
public:
	EmptyScene(const std::string& friendly_name);
	virtual ~EmptyScene();
	virtual void OnInitializeScene()	 override;
	/*{
		Scene::OnInitializeScene();
		this->AddGameObject(CommonUtils::BuildCuboidObject("Ground", Vector3(0.0f, -1.0f, 0.0f), Vector3(20.0f, 1.0f, 20.0f), true, 0.0f, true, false, Vector4(0.2f, 0.5f, 1.0f, 1.0f)));
	}*/
	virtual void OnCleanupScene()		 override;
	virtual void OnUpdateScene(float dt) override;
	//{
	//	Scene::OnUpdateScene(dt);

	//	if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_J))
	//	{
	//		Camera* cam = SceneManager::Instance()->GetCamera();
	//		Matrix4 viewMtx = cam->BuildViewMatrix();
	//		Vector3 forward = -Vector3(viewMtx[2], viewMtx[6], viewMtx[10]);

	//		Object* sphere = CommonUtils::BuildSphereObject("",
	//			cam->GetPosition(),					//Position leading to 0.1 meter overlap on faces, and more on diagonals
	//			0.5f,									//Radius
	//			true,									//Has Physics Object
	//			2.0f,									//Infinite Mass
	//			true,									//Has Collision Shape
	//			false,									//Dragable by the user
	//			CommonUtils::GenColour(0.3f, 0.5f));	//Color

	//		sphere->Physics()->SetLinearVelocity(forward * 10.f);
	//		this->AddGameObject(sphere);
	//	}
	//}
	//get the powered value
	inline float GetPoweredValue() { return power; };
	inline void SetPoweredValue(float pow) { power=pow; };

	//track of the sun
	Vector3 eaTra = Vector3(0.0f, 1.5f, 0.0f);

	//networking
	void ProcessNetworkEvent(const ENetEvent& evnt);
protected:
	float m_AccumTime;
	static float power;
	bool powered = false;
	bool debug;
	bool* atmos;
	int score;
	int score2;
	int debugCount = 1;
	vector<Vector4> col;

	Object*     player2;
	//Camera*		cam2;
	NetworkBase m_Network;
	NetworkBase m_Network2;
	ENetPeer*	m_pServerConnection;
	ENetPeer*	m_pServerConnection2;
};