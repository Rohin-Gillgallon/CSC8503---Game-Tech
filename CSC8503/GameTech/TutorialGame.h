#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "StateGameObject.h"



namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void InitGameExamples();
			void InitAddObstacles();
			void InitSphereGridWorld(Vector3 position, float radius, float inversemass);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddCubeToWorldOBB(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			void Stairs();
			void JumpPad1();
			void JumpPad2();
			void AddRotatingBridges();
			void RotatingBridges();
			void AddRotatingPlatform();
			void RotatingPlatform(float angle);
			void WobblingPlatform();
			void AddTravelPlatform();
			void TravelPlatform(Vector3 lift);
			void AddMazePlatform();
			void AddProjectilePlatform();
			void AddGravityWell();

			Vector3 SpawnPoint = Vector3(-40, 10, -40);
			Vector3 Checkpoint1 = Vector3(100 / 8 * 12.25, 100 / 8 * 4 + 10, -100 / 8 * 3);

			bool useGravity;
			bool inSelectionMode;
			bool rotateFloor;
			bool jumppad1 = false;
			bool jumppad2 = false;
			bool jumppadcollision1 = true;
			bool jumppadcollision2 = true;
			bool Teleport1 = false;;

			Vector3 stairlift = Vector3(0, 0.1, 0);
			Vector3 platLift = Vector3(-0.1 * cos(30 * 3.14 / 180), 0.1 * sin(30 *  3.14 / 180), 0);
			int liftcount = 0;
			float forceMagnitude;
			Vector3 direction;

			GameObject* selectionObject = nullptr;
			GameObject* selectionObject2 = nullptr;
			GameObject* Ball = nullptr;
			GameObject* stair1;
			GameObject* stair2;
			GameObject* stair3;
			GameObject* stair4;
			GameObject* stair5;
			GameObject* stair6;
			GameObject* stair7;
			GameObject* stair8;
			GameObject* stair9;
			GameObject* stair10;
			GameObject* floora;
			GameObject* floorb;
			GameObject* Wallb4;
			GameObject* rotatingplat;
			GameObject* rotatingplat2;
			GameObject* rotatingplat3;
			GameObject* Wplat;
			GameObject* liftPlat;
			GameObject* liftPlat2;
			GameObject* liftPlat3;
			GameObject* mazeplat;

			

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

		};
	}
}

