#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/Constraint.h"
#include "../../Common/Quaternion.h"
#include "../../Common/Vector3.h"


using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 500.0f;
	useGravity		= false;
	inSelectionMode = false;
	rotateFloor = false;
	SpawnPoint = Checkpoint1;
	Debug::SetRenderer(renderer);
	
	InitialiseAssets();
}

/*
t
Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!
 
*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	SelectObject();
	MoveSelectedObject();
	physics->Update(dt);

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}

	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		Ball->Respawn(SpawnPoint);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
		if (Teleport1) {
			useGravity = false;
			Ball->Respawn(Vector3(100 / 8 * 12.5 + 100 / 12, 100 / 12 + 52, 100 / 8 * 4.75 + 1));
			useGravity = true;
			std::cout << "Checkpoint Reached\n";
			SpawnPoint = Checkpoint2;
			bridgeproject = true;
			Teleport1 = false;
		}
		else if (Teleport2) {
			useGravity = false;
			Ball->Respawn(Vector3(206 - 100 / 8 * 2, 100 / 8 + 30, 100 / 8 * 8));
			useGravity = true;
			Teleport2 = false;
		}
		else 
			return;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::H)) {
		hold = !hold;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::J)) {
		if(bridgeproject)
			Ball->GetPhysicsObject()->AddForceAtPosition(Vector3(-1, 0, 0) * 2500, Vector3(164, 58, 58));
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	/*if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}*/

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	//InitMixedGridWorld(5, 5, 3.5f, 3.5f);
	//InitGameExamples();
	InitSphereGridWorld(SpawnPoint, 5.0f, 2.0f);
	InitDefaultFloor();
	InitAddObstacles();
	Stairs();
	JumpPad1();
	JumpPad2();
	AddRotatingBridges();
	AddRotatingPlatform();
	AddWobblingPlatform();
	AddTravelPlatform();
	AddMazePlatform();
	AddProjectilePlatform();
	AddGravityWell();
	BridgeConstraintTest();
	AddBridge();
	AddBouncePad();
	AddIcePatch();
	AddTeleport();
	AddGoal();
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(1, 1, 1);

	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 6;
	float maxDistance = 4; // constraint distance
	float cubeDistance = 2.5; // distance between links
	
	Vector3 startPos = Vector3(120, 100 / 8 + 60, 100 / 8 * 18);

	GameObject* start = AddCubeToWorldOBB(startPos + Vector3(0, 0, 0)
		, cubeSize, 0);
	GameObject* end = AddCubeToWorldOBB(startPos + Vector3(0, 0, (numLinks + 2)
		* cubeDistance), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorldOBB(startPos + Vector3((i + 1) *
			cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous,
			block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint* constraint = new PositionConstraint(previous,
		end, maxDistance);
	world->AddConstraint(constraint);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize	= Vector3(50, 2, 50);
	AABBVolume* volume	= new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	floor->SetName("floor");
	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddCubeToWorldOBB(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	OBBVolume* volume = new OBBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}


void TutorialGame::InitAddObstacles() {

	auto obs = AddCubeToWorld(Vector3(-100 / 8 * 4, 100 / 8, -100 / 8 + 100/ 16 ), Vector3(1, 100 / 8, 100 / 8 * 3.5), 0);

	auto obs1 = AddCubeToWorldOBB(Vector3(-100 / 8 * 3, 100 / 8, 100 / 8 * 2.5 + 100 / 16), Vector3(1, 100 / 8, 100 / 8), 0);
	Quaternion rotate1 = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), 45);
	obs1->SetOrientation(rotate1);
	
	auto obs2 = AddCubeToWorldOBB(Vector3(-100 / 8, 100 / 8, 100 / 8 * 2.5 + 100 / 16), Vector3(1, 100 / 8, 100 / 8), 0);
	Quaternion rotate2 = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -45);
	obs2->SetOrientation(rotate2);

	auto obs3 = AddCubeToWorld(Vector3(-100 / 8 * 2, 100 / 8, -100 / 8 - 100 / 16), Vector3(1, 100 / 8, 100 / 8 * 2.5), 0);

	auto obs4 = AddCubeToWorld(Vector3(0, 100 / 8, 0), Vector3(1, 100 / 8, 100 / 8 * 2), 0);

	auto obs5 = AddCubeToWorldOBB(Vector3(-100 / 8, 100 / 8, -100 / 8 * 3.5), Vector3(1, 100 / 8, 100 / 8), 0);
	Quaternion rotate5 = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -45);
	obs5->SetOrientation(rotate5);

	auto obs6 = AddCubeToWorld(Vector3(100 / 8, 100 / 8, -100 / 8 * 4), Vector3(100 / 8, 100 / 8, 1), 0);

	auto obs7 = AddCubeToWorldOBB(Vector3(100 / 8 * 1.5, 1, -100 / 8 * 3), Vector3(100 / 8 * 0.9, 1, 100 / 8 * 0.9), 0);
	Quaternion rotate7 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 15);
	obs7->SetOrientation(rotate7);

	auto obs8 = AddCubeToWorld(Vector3(100 / 8 *2, 100 / 8 *1.25, -100 / 8 * 1.5), Vector3(100 / 8 * 1.5, 100 / 8 * 1.25, 1), 0);

	auto obs9 = AddCubeToWorldOBB(Vector3(100 / 8 * 14.5, 100 / 8 * 3, -100 / 8 * 3), Vector3(100 / 8, 2, 100 / 8), 0);
	Quaternion rotate9 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -30);
	obs9->SetOrientation(rotate9);

	auto obs10 = AddCubeToWorldOBB(Vector3(100 / 8 * 18.5, 100 / 8 * 3, -100 / 8 * 3), Vector3(100 / 8, 2, 100 / 8), 0);
	Quaternion rotate10 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 30);
	obs10->SetOrientation(rotate10);

	auto obs11 = AddCubeToWorldOBB(Vector3(100 / 8 * 16.5, 100 / 8 * 2, -100 / 8 * 2.5), Vector3(100 / 8, 2, 100 / 6), 0);
	Quaternion rotate11 = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 15);
	obs11->SetOrientation(rotate11);

	auto obs12 = AddCubeToWorld(Vector3(100 / 8 * 11.5, 100 / 8 * 1.5, -100 / 8 * 5.5), Vector3(1, 100 / 16, 100 / 16), 0);

	auto obs13 = AddCubeToWorld(Vector3(100 / 8 * 11, 100 / 8 * 1.5, -100 / 8 * 6.25), Vector3(100 / 16, 100 / 16, 1), 0);

	auto obs14 = AddCubeToWorld(Vector3(100 / 8 * 12.5, 100 / 8 * 4, -100 / 8 * 5.5), Vector3(100 / 8, 100 / 16, 1), 0);

	auto obs15 = AddCubeToWorld(Vector3(100 / 8 * 12.5, 100 / 8 * 4, -100 / 8 * 1.5), Vector3(100 / 8, 100 / 16, 1), 0);

	auto obs16 = AddCubeToWorld(Vector3(100 / 8 * 13.5, 100 / 8 * 4, -100 / 8 * 4.5), Vector3(1, 100 / 16, 100 / 16), 0);

	auto obs17 = AddCubeToWorldOBB(Vector3(100 / 8 * 15.5, 100 / 8 * 2.5, -100 / 8 * 1.5), Vector3(1, 100 / 32, 100 / 8), 0);
	Quaternion rotate17a = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 45);
	Quaternion rotate17b = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 15);
	Quaternion rotate17c = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), 25);
	obs17->SetOrientation(rotate17a * rotate17b * rotate17c);

	auto obs18 = AddCubeToWorldOBB(Vector3(100 / 8 * 17.5, 100 / 8 * 2.5, -100 / 8 * 1.5), Vector3(1, 100 / 16, 100 / 8), 0);
	Quaternion rotate18a = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 45);
	Quaternion rotate18b = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -15);
	Quaternion rotate18c = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -15);
	obs18->SetOrientation(rotate18a * rotate18b * rotate18c);

	auto obs19 = AddCubeToWorld(Vector3(100 / 8 * 12.5 + 100 / 12, 100 / 8 + 40, 100 / 8 * 4.75 + 1), Vector3(100 / 8, 1, 100 / 8), 0);
	auto obs20 = AddCubeToWorldOBB(Vector3(-3, 100 / 8 + 40, 100 / 8 * 4.75 + 1), Vector3(100 / 8, 1, 100 / 8), 0);
	obs21 = AddCubeToWorldOBB(Vector3(-3, 100 / 8 + 35, 100 / 8 * 10.75 + 1), Vector3(100 / 8, 1, 100 / 8), 0);
	auto obs22 = AddCubeToWorld(Vector3(100 / 8 * 13.5 + 100 / 12, 100 / 12 + 57.5, 100 / 8 * 4.75 + 1), Vector3(1, 100 / 8, 100 / 8), 0);
}

void TutorialGame::Stairs() {
	stair1 = AddCubeToWorldOBB(Vector3(100 / 8 * 3.25, 2.75, -100 / 8 * 3), Vector3(100 / 8, 1, 100 / 8), 0);
	Quaternion rotate1a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate1b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), -5);
	stair1->SetOrientation(rotate1a * rotate1b);

	stair2 = AddCubeToWorldOBB(Vector3(100 / 8 * 4.25, 1.5, -100 / 8 * 5), Vector3(100 / 8 * 2, 1, 100 / 8), 0);
	Quaternion rotate2a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate2b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 5);
	stair2->SetOrientation(rotate2a * rotate2b);

	stair3 = AddCubeToWorldOBB(Vector3(100 / 8 * 5.25, 2.75, -100 / 8 * 3), Vector3(100 / 8, 100 / 16, 100 / 8), 0);
	Quaternion rotate3a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate3b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), -5);
	stair3->SetOrientation(rotate3a * rotate3b);

	stair4 = AddCubeToWorldOBB(Vector3(100 / 8 * 6.25, 1.5, -100 / 8 * 5), Vector3(100 / 8, 100 / 16, 100 / 8), 0);
	Quaternion rotate4a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate4b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 5);
	stair4->SetOrientation(rotate4a * rotate4b);

	stair5 = AddCubeToWorldOBB(Vector3(100 / 8 * 7.25, 7.75, -100 / 8 * 3), Vector3(100 / 8, 100 / 16, 100 / 8), 0);
	Quaternion rotate5a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate5b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), -5);
	stair5->SetOrientation(rotate5a * rotate5b);

	stair6 = AddCubeToWorldOBB(Vector3(100 / 8 * 8.25, 6.5, -100 / 8 * 5), Vector3(100 / 8, 100 / 16, 100 / 8), 0);
	Quaternion rotate6a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate6b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 5);
	stair6->SetOrientation(rotate6a * rotate6b);

	stair7 = AddCubeToWorldOBB(Vector3(100 / 8 * 9.25, 12.75, -100 / 8 * 3), Vector3(100 / 8, 100 / 16, 100 / 8), 0);
	Quaternion rotate7a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate7b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), -5);
	stair7->SetOrientation(rotate7a * rotate7b);

	stair8 = AddCubeToWorldOBB(Vector3(100 / 8 * 10.25, 11.5, -100 / 8 * 5), Vector3(100 / 8, 100 / 16, 100 / 8), 0);
	Quaternion rotate8a = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), -5);
	Quaternion rotate8b = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), 5);
	stair8->SetOrientation(rotate8a * rotate8b);

	stair9 = AddCubeToWorld(Vector3(100 / 8 * 12.25, 100 / 8 * 2, -100 / 8 * 3), Vector3(100 / 8 * 1.5, 100 / 8 * 2, 100 / 8), 0);
	stair9->SetName("Checkpoint1");
	stair10 = AddCubeToWorld(Vector3(100 / 8 * 12.25, 100 / 8 * 2, -100 / 8 * 4.5), Vector3(100 / 8, 100 / 8 * 2, 100 / 16), 0);
}

void TutorialGame::JumpPad1(){
	auto Walla1 = AddCubeToWorld(Vector3(100 / 8 * 16 - 2, 100 / 8 + 100 / 24, 0), Vector3(1, 100 / 16, 100 / 12), 0);
	auto Walla2 = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 6, 100 / 8 + 100 / 24, 0), Vector3(1, 100 / 16, 100 / 12), 0);
	//auto Walla3 = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 12, 100 / 8, -100 / 12 - 1), Vector3(100 / 12, 100 / 16, 1), 0);
	//auto Walla4 = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 12, 100 / 8 + 100 / 24, 100 / 12 + 1), Vector3(100 / 12, 100 / 16, 1), 0);
	floora = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 12 - 1, 100 / 8 + 1, 1), Vector3(100 / 12, 1, 100 / 12), 0);
	floora->SetName("JumpPadA");
}

void TutorialGame::JumpPad2() {
	//auto Wallb1 = AddCubeToWorld(Vector3(100 / 8 * 16 - 2, 100 / 8 + 100 / 32, 100 / 8 * 5), Vector3(1, 100 / 16, 100 / 12), 0);
	//auto Wallb2 = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 6, 100 / 8 + 100 / 24 - 4, 100 / 8 * 5), Vector3(1, 100 / 16, 100 / 12), 0);
	//auto Wallb3 = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 12, 100 / 8, -100 / 12 - 1 + 100/8*5), Vector3(100 / 12, 100 / 16, 1), 0);
	Wallb4 = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 12, 100 / 8 + 100 / 32, 100 / 12 + 1 + 100 / 8 * 5), Vector3(100 / 8, 100 / 8, 1), 0);
	Wallb4->SetName("Wallb4");
	floorb = AddCubeToWorld(Vector3(100 / 8 * 16 + 100 / 12 - 1, 100 / 8 - 4, 100 / 8 * 4.75 + 1), Vector3(100 / 8, 1, 100 / 12), 0);
	floorb->SetName("JumpPadB");
}

void TutorialGame::AddRotatingBridges() {
	plat1 = AddCubeToWorldOBB(Vector3(100 / 8 * 9.5 + 100 / 12, 100 / 8 + 38, 100 / 8 * 4.75 + 1), Vector3(20, 1, 10), 0);
	plat2 = AddCubeToWorldOBB(Vector3(100 / 8 * 9.5 + 100 / 12 - 45, 100 / 8 + 38, 100 / 8 * 4.75 + 1), Vector3(20, 1, 10), 0);
	plat3 = AddCubeToWorldOBB(Vector3(100 / 8 * 9.5 + 100 / 12 - 90, 100 / 8 + 38, 100 / 8 * 4.75 + 1), Vector3(20, 1, 10), 0);
}

void TutorialGame::RotatingBridges(float angle) {
	Quaternion rotateplat1 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), angle);
	Quaternion rotateplat3 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), angle * 1.5);
	plat1->SetOrientation(rotateplat1);
	plat3->SetOrientation(rotateplat3);
}

void TutorialGame::AddRotatingPlatform() {
	rotatingplat = AddCubeToWorldOBB(Vector3(-25, 100 / 8 + 65, 100 / 8 * 7.75), Vector3(100 / 16, 4, 3.5 * 100 / 8), 0);
	rotatingplat->SetName("Spinner"); //Vector3(0, -14, -100/8 * 3.5); // Vector3(0, -25, -100/8 * 3);
	rotatingplat2 = AddCubeToWorldOBB((rotatingplat->Position() + Vector3(0, -14, -100 / 8 * 3.5)), Vector3(100 / 16, 10, 2), 0);
	rotatingplat2->SetName("Support");
	rotatingplat3 = AddCubeToWorldOBB((rotatingplat->Position() + Vector3(0, -25, -100 / 8 * 3)), Vector3(100 / 16, 1, 100 / 12), 0);
	rotatingplat3->SetName("RotatingPlatform");
}

void TutorialGame::RotatingPlatform(float angle) {	
	Quaternion rotatePLAT = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angle);
	rotatingplat->SetOrientation(rotatePLAT);
	rotatingplat2->SetOrientation(rotatePLAT);
	rotatingplat3->SetOrientation(rotatePLAT);
	Matrix3 transform = Matrix3(rotatingplat->GetTransform().GetOrientation());
	auto adjust2 = transform * Vector3(0, -14, -100 / 8 * 3.5);
	auto adjust3 = transform * Vector3(0, -25, -100 / 8 * 3);
	rotatingplat2->Respawn((rotatingplat->Position() + adjust2));
	rotatingplat3->Respawn((rotatingplat->Position() + adjust3));
}

void TutorialGame::AddWobblingPlatform() {
	Wplat = AddCubeToWorldOBB(Vector3(40, 100 / 8 + 25, 100 / 8 * 10.75 + 1), Vector3(100 / 8 * 3, 1, 100 / 8 * 3), 0);
	Wplat->SetName("WobblingPlatform");
	
}

void TutorialGame::WobblingPlatform(float angle, std::string dir) {
	Quaternion rotateWPLATx = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), angle);
	Quaternion rotateWPLATz = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), angle);
	if (dir == "x")
		Wplat->SetOrientation(rotateWPLATx);
	else if (dir == "z")
		Wplat->SetOrientation(rotateWPLATz);
	else
		return;
}

void TutorialGame::AddTravelPlatform() {
	liftPlat = AddCubeToWorldOBB(Vector3(50, 100 / 8 + 50, 100 / 8 * 15), Vector3(100 / 16, 2, 3.5 * 100 / 8), 0);
	liftPlat->SetName("Spinner"); //Vector3(0, -14, -100/8 * 3.5); // Vector3(0, -25, -100/8 * 3);
	liftPlat2 = AddCubeToWorldOBB(Vector3(0, 0, 0), Vector3(100 / 16, 8, 1), 0);
	liftPlat2->SetName("LiftSupport");
	liftPlat3 = AddCubeToWorldOBB(Vector3(0, 0, 0), Vector3(100 / 16, 1, 100 / 12), 0);
	liftPlat3->SetName("Lift");
	Quaternion rotatelifta = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -90);
	Quaternion rotateliftb = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), -30);
	liftPlat->SetOrientation(rotatelifta * rotateliftb);
	liftPlat2->SetOrientation(rotatelifta);
	liftPlat3->SetOrientation(rotatelifta);
	Matrix3 transform = Matrix3(liftPlat->GetTransform().GetOrientation());
	auto adjust2 = transform * Vector3(0, -8, -100 / 8 * 3.5);
	auto adjust3 = transform * Vector3(0, -18, -100 / 8 * 3.25);
	liftPlat2->Respawn((liftPlat->Position() + adjust2));
	liftPlat3->Respawn((liftPlat->Position() + adjust3));
}

void TutorialGame::TravelPlatform(Vector3 lift) {
	liftcount = (lift.y > 0) ? liftcount + 1 : liftcount - 1;
	if (liftcount >= 0 && liftcount <= 800) {
		liftPlat2->Move(lift);
		liftPlat3->Move(lift);
	}
}

void TutorialGame::AddMazePlatform() {
	mazeplat = AddCubeToWorldOBB(Vector3(36, 100 / 8 + 50, 100 / 8 * 19), Vector3(36, 1, 36), 0);
	mazeplat->SetName("maze");
	Matrix3 transform = Matrix3(mazeplat->GetTransform().GetOrientation());
	maze1 = AddCubeToWorldOBB(Vector3(0, 0, 0), Vector3(1, 6, 36), 0);
	auto adjust = transform * Vector3(-35, 7, 0);
	maze1->Respawn(mazeplat->Position() + adjust);
	maze2 = AddCubeToWorldOBB(Vector3(0, 0, 0), Vector3(1, 6, 27), 0);
	auto adjust2 = transform * Vector3(35, 7, -9);
	maze2->Respawn(mazeplat->Position() + adjust2);
	maze3 = AddCubeToWorldOBB(Vector3(0, 0, 0), Vector3(35, 6, 1), 0);
	auto adjust3 = transform * Vector3(1, 7, 35);
	maze3->Respawn(mazeplat->Position() + adjust3);
	maze4 = AddCubeToWorld(Vector3(0, 0, 0), Vector3(26, 6, 1), 0);
	auto adjust4 = transform * Vector3(8, 7, -35);
	maze4->Respawn(mazeplat->Position() + adjust4);
	maze5 = AddCubeToWorld(Vector3(0, 0, 0), Vector3(22.5, 6, 1), 0);
	auto adjust5 = transform * Vector3(-10.5, 7, -9);
	maze5->Respawn(mazeplat->Position() + adjust5);
	maze6 = AddCubeToWorld(Vector3(0, 0, 0), Vector3(22.5, 6, 1), 0);
	auto adjust6 = transform * Vector3(11.5, 7, 9);
	maze6->Respawn(mazeplat->Position() + adjust6);
	mazeExit = AddCubeToWorldOBB((mazeplat->Position() + Vector3(45, -5, 27)), Vector3(9, 1, 9), 0);
	auto mazeExitWall1 = AddCubeToWorldOBB((mazeplat->Position() + Vector3(45, 5, 36)), Vector3(9, 9, 1), 0);
	auto mazeExitWall2 = AddCubeToWorldOBB((mazeplat->Position() + Vector3(45, 5, 18)), Vector3(9, 9, 1), 0);
}

void TutorialGame::mazePlatform(float angle, std::string dir) {
	Quaternion rotateWPLATx = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), angle);
	Quaternion rotateWPLATz = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), angle);
	if (dir == "x") {
		mazeplat->SetOrientation(rotateWPLATx);
		Matrix3 transform = Matrix3(mazeplat->GetTransform().GetOrientation());
		auto adjust = transform * Vector3(-35, 7, 0);
		maze1->Respawn(mazeplat->Position() + adjust);
		maze1->SetOrientation(rotateWPLATx);
		
		auto adjust2 = transform * Vector3(35, 7, -9);
		maze2->Respawn(mazeplat->Position() + adjust2);
		maze2->SetOrientation(rotateWPLATx);
		
		auto adjust3 = transform * Vector3(1, 7, 35);
		maze3->Respawn(mazeplat->Position() + adjust3);
		maze3->SetOrientation(rotateWPLATx);
		
		auto adjust4 = transform * Vector3(8, 7, -35);
		maze4->Respawn(mazeplat->Position() + adjust4);
		maze4->SetOrientation(rotateWPLATx);
		
		auto adjust5 = transform * Vector3(-10.5, 7, -9);
		maze5->Respawn(mazeplat->Position() + adjust5);
		maze5->SetOrientation(rotateWPLATx);
	
		auto adjust6 = transform * Vector3(11.5, 7, 9);
		maze6->Respawn(mazeplat->Position() + adjust6);
		maze6->SetOrientation(rotateWPLATx);
	}
	else if (dir == "z") {
		mazeplat->SetOrientation(rotateWPLATz);
		Matrix3 transform = Matrix3(mazeplat->GetTransform().GetOrientation());
	
		auto adjust = transform * Vector3(-35, 7, 0);
		maze1->Respawn(mazeplat->Position() + adjust);
		maze1->SetOrientation(rotateWPLATz);

		auto adjust2 = transform * Vector3(35, 7, -9);
		maze2->Respawn(mazeplat->Position() + adjust2);
		maze2->SetOrientation(rotateWPLATz);

		auto adjust3 = transform * Vector3(1, 7, 35);
		maze3->Respawn(mazeplat->Position() + adjust3);
		maze3->SetOrientation(rotateWPLATz);
	
		auto adjust4 = transform * Vector3(8, 7, -35);
		maze4->Respawn(mazeplat->Position() + adjust4);
		maze4->SetOrientation(rotateWPLATz);
	
		auto adjust5 = transform * Vector3(-10.5, 7, -9);
		maze5->Respawn(mazeplat->Position() + adjust5);
		maze5->SetOrientation(rotateWPLATz);

		auto adjust6 = transform * Vector3(11.5, 7, 9);
		maze6->Respawn(mazeplat->Position() + adjust6);
		maze6->SetOrientation(rotateWPLATz);
	}
	else
		return;
}

void TutorialGame::AddProjectilePlatform() {
	auto Projplat = AddCubeToWorldOBB(Vector3(124, 100 / 8 + 60, 100 / 8 * 19), Vector3(36, 1, 36), 0);
	Quaternion rotateproj = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 25);
	Projplat->SetOrientation(rotateproj);
	auto projplatbase = AddCubeToWorldOBB(Vector3(100, 100 / 8 + 60, 100 / 8 * 18.25), Vector3(1, 9, 27), 0);
	Quaternion rotateprojbasea = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 25);
	Quaternion rotateprojbaseb = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -15);
	projplatbase->SetOrientation(rotateprojbasea * rotateprojbaseb);
	auto projplattopa = AddCubeToWorldOBB(Vector3(140, 100 / 8 + 70, 100 / 8 * 17.2), Vector3(1, 9, 18), 0);
	Quaternion rotateprojtopa1 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 25);
	Quaternion rotateprojtopa2 = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), 45);
	projplattopa->SetOrientation(rotateprojtopa1 * rotateprojtopa2);

	auto projplattopb = AddCubeToWorldOBB(Vector3(140, 100 / 8 + 70, 100 / 8 * 21), Vector3(1, 9, 18), 0);
	Quaternion rotateprojtopb1 = Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 1), 25);
	Quaternion rotateprojtopb2 = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), -45);
	projplattopb->SetOrientation(rotateprojtopb1 * rotateprojtopb2);
}

void TutorialGame::AddGravityWell() {
	gravwell = AddCubeToWorldOBB(Vector3(206, 100 / 8 + 74.5, 100 / 8 * 16), Vector3(50, 1, 50), 0);
	gravwell->GetPhysicsObject()->gravitywell = true;
}

void TutorialGame::AddBridge() {
	auto bridge = AddCubeToWorldOBB(Vector3(206, 100 / 8 + 35, 100 / 8 * 14.5), Vector3(100 / 8, 1, 100 / 8 * 2), 0);
	Quaternion rotatebridge = Quaternion::AxisAngleToQuaterion(Vector3(1, 0, 0), -25);
	bridge->SetOrientation(rotatebridge);
}

void TutorialGame::AddBouncePad() {
	bounce = AddCubeToWorldOBB(Vector3(206, 100 / 8 + 10, 100 / 8 * 11), Vector3(100 / 8, 1, 100 / 8), 0);
	bounce->GetPhysicsObject()->spring = true;
}

void TutorialGame::AddIcePatch() {
	auto ice1 = AddCubeToWorld(Vector3(206, 100 / 8 + 20, 100 / 8 * 7), Vector3(100 / 8, 1, 100 / 8 * 3), 0);
	ice1->GetPhysicsObject()->SetFriction(0.1);
	auto ice2 = AddCubeToWorld(Vector3(206 - 100 / 8 * 2, 100 / 8 + 20, 100 / 8 *8), Vector3(100 / 8, 1, 100 / 8), 0);
	ice2->GetPhysicsObject()->SetFriction(0.1);
	auto ice3 = AddCubeToWorld(Vector3(206 + 100 / 8 * 2, 100 / 8 + 20, 100 / 8 * 8), Vector3(100 / 8, 1, 100 / 8), 0);
	ice3->GetPhysicsObject()->SetFriction(0.1);
}

void TutorialGame::AddTeleport() {
	teleport = AddCubeToWorld(Vector3(206, 100 / 8 + 20, 100 / 8 * 3), Vector3(100 / 8, 1, 100 / 8), 0);
	teleport->GetPhysicsObject()->SetFriction(100);
}

void TutorialGame::AddGoal() {
	goal = AddCubeToWorld(Vector3(206 + 100 / 8 * 4, 100 / 8 + 20, 100 / 8 * 8), Vector3(100 / 8, 1, 100 / 8), 0);
}

void TutorialGame::InitSphereGridWorld(Vector3 position, float radius, float inversemass) {
	Ball = AddSphereToWorld(position, radius, inversemass);
	Ball->SetName("Ball");
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
	AddFloorToWorld(Vector3(100, -2, 0));
	AddFloorToWorld(Vector3(0, -2, 100));
	AddFloorToWorld(Vector3(100, -2, 100));
	AddFloorToWorld(Vector3(200, -2, 0));
	AddFloorToWorld(Vector3(0, -2, 200));
	AddFloorToWorld(Vector3(100, -2, 200));
	AddFloorToWorld(Vector3(200, -2, 100));
	AddFloorToWorld(Vector3(200, -2, 200));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}
			
			if (selectionObject2) {	//set colour to deselected;
				selectionObject2->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject2 = nullptr;
				lockedObject = nullptr;
			}
		
			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true, selectionObject)) {
				selectionObject = (GameObject*)closestCollision.node;
				ActiveObject = selectionObject;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				
				Ray objray = selectionObject->CreateRay();
				{
					if (world->Raycast(objray, closestCollision, true, selectionObject)) {
						selectionObject2 = (GameObject*)closestCollision.node;
						Debug::DrawLine(selectionObject->Position(), selectionObject2->Position(), Vector4(1, 0, 0, 1), 60);
						selectionObject2->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
						return true;
					}
					else {
						return false;
					}
				}
				return true;
			}
			else {
				return false;
			}

		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude),
		 Vector2(10, 20));//Draw debug text at 10,20
	
	 forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	
	 if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP))
	 {
		 if (ActiveObject == mazeplat) {
			 mazePlatform(-0.1, "z");
		 }
		 else if (ActiveObject == Wplat)
			 WobblingPlatform(-0.1, "z");
		 else {
			 stair2->Move(stairlift);
			 stair4->Move(stairlift);
			 stair6->Move(stairlift);
			 stair8->Move(stairlift);
		 }
	 }

	 if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN))
	 {
		 if (ActiveObject == mazeplat)
			 mazePlatform(0.1, "z");
		 else if (ActiveObject == Wplat)
			 WobblingPlatform(0.1, "z");
		 else {
			 stair2->Move(-stairlift);
			 stair4->Move(-stairlift);
			 stair6->Move(-stairlift);
			 stair8->Move(-stairlift);
		 }
	 }

	 if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		 if(ActiveObject == rotatingplat)
			RotatingPlatform(1);
		 else if (ActiveObject == liftPlat)
			TravelPlatform(platLift);
		 else if (ActiveObject == Wplat) 
			 WobblingPlatform(0.1, "x");
		 else if (ActiveObject == mazeplat)
			 mazePlatform(0.1, "x");
		 else
			RotatingBridges(0.1);
	 }

	 if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		 if (ActiveObject == rotatingplat)
			RotatingPlatform(-1);
		 else if (ActiveObject == liftPlat)
			TravelPlatform(-platLift);
		 else if (ActiveObject == Wplat)
			 WobblingPlatform(-0.1, "x");
		 else if (ActiveObject == mazeplat)
			 mazePlatform(-0.1, "x");
		 else
			RotatingBridges(-0.1);
	 }

	 RotatingBridges(-0.1);

	 CollisionDetection::CollisionInfo info1;
	 if (CollisionDetection::ObjectIntersection(Ball, stair9, info1))
	 {
		 
		 std::cout << "Checkpoint Reached\n";
		 SpawnPoint = Checkpoint1;
	 }
	 CollisionDetection::CollisionInfo info2;
	 if (CollisionDetection::ObjectIntersection(Ball, floora, info2)) {
		 
		 useGravity = false;
		 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		 Ball->Respawn(Vector3(100 / 8 * 16 + 100 / 12, 100 / 8 + 4, 1));
		 useGravity = true;
		 Ball->GetPhysicsObject()->AddForceAtPosition(Vector3(-0.0930896, -0.164389, 0.981993) * 4250, Vector3(200.149, 12.2566, -2.58092));
	 }

	 CollisionDetection::CollisionInfo info3;
	 if (CollisionDetection::ObjectIntersection(Ball, Wallb4, info3)) {
		 useGravity = false;
		 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		 Ball->Respawn(Vector3(206, 100 / 8 + 20, 100 / 8 * 4));
		 useGravity = true;
	 }

	 CollisionDetection::CollisionInfo info4;
	 if (CollisionDetection::ObjectIntersection(Ball, floorb, info4)) {
		 
		 useGravity = false;
		 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		 Ball->Respawn(Vector3(100 / 8 * 16 + 100 / 12 - 1, 100 / 8 + 4, 100 / 8 * 4.75 + 1));
		 Teleport1 = true;
	 }

	 CollisionDetection::CollisionInfo info5;
	 if (CollisionDetection::ObjectIntersection(Ball, rotatingplat3, info5)) {
		 if (hold) {
			 useGravity = false;
			 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
			 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
			 Ball->GetPhysicsObject()->ClearForces();
			 Matrix3 transform = Matrix3(rotatingplat3->GetTransform().GetOrientation());
			 auto adjust = transform * Vector3(0, 6, 2);

			 Ball->Respawn((rotatingplat3->Position() + adjust));
		 }
	 }

	 CollisionDetection::CollisionInfo info6;
	 if (CollisionDetection::ObjectIntersection(Ball, obs21, info6)) {
		 std::cout << "Checkpoint Reached\n";
		 SpawnPoint = Checkpoint3;
	 }

	 CollisionDetection::CollisionInfo info7;
	 if (CollisionDetection::ObjectIntersection(Ball, liftPlat3, info7)) {
		 if (hold) {
			 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
			 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
			 Ball->GetPhysicsObject()->ClearForces();
			 Matrix3 transform = Matrix3(liftPlat3->GetTransform().GetOrientation());
			 auto adjust = transform * Vector3(0, -1.25, 10);
			 if (hold)
				 Ball->Respawn((liftPlat2->Position() + adjust));
		 }
	 }
	
	 CollisionDetection::CollisionInfo info8;
	 if (CollisionDetection::ObjectIntersection(Ball, mazeExit, info8)) {
		 std::cout << "Checkpoint Reached\n";
		 SpawnPoint = Checkpoint4;
	 }

	 CollisionDetection::CollisionInfo info9;
	 if (CollisionDetection::ObjectIntersection(Ball, teleport, info9)) {

		 useGravity = false;
		 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		 Teleport2 = true;
	 }

	 auto sum = (Ball->Position() - gravwell->Position() - Vector3(0, 6, 0)).Length();
	 if ((Ball->Position() - gravwell->Position() - Vector3(0, 6, 0)).Length() < 1.0f) {
		 Ball->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
		 Ball->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
		 Ball->Respawn(gravwell->Position() - Vector3(0, 6, 0));
		 SpawnPoint = Checkpoint5;
	 }

	 if (!selectionObject) {
		 return;//we haven’t selected anything!
	 }		
		 //Push the selected object!
	 if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		 Ray ray = CollisionDetection::BuildRayFromMouse(
			 *world->GetMainCamera());
		 RayCollision closestCollision;
		 if (world->Raycast(ray, closestCollision, true)) {
			 /*if (closestCollision.node == selectionObject) {
				 selectionObject->GetPhysicsObject()->
					 AddForce(ray.GetDirection() * forceMagnitude);
			 }*/

			 if (closestCollision.node == selectionObject) {
				 selectionObject->GetPhysicsObject()->AddForceAtPosition(
					 ray.GetDirection() * forceMagnitude,
					 closestCollision.collidedAt);
				 std::cout << "Direction: " << ray.GetDirection() << "Position: " << closestCollision.collidedAt << std::endl;
			 }
		 }
	 }
}