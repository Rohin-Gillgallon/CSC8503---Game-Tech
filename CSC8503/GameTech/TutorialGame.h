#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "StateGameObject.h"
#include "../CSC8503Common/NavigationGrid.h"
#include "..//CSC8503Common/BehaviourNode.h"
#include "..//CSC8503Common/BehaviourAction.h"
#include "..//CSC8503Common/BehaviourSequence.h"
#include "..//CSC8503Common/BehaviourSelector.h"

namespace NCL {

	enum class GameState {
		Title = 0,
		Menu = 1,
		Level1 = 2,
		Level1Score = 3,
		Level2 = 4,
		Level2Score = 5
	};

	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);
			int count;
			int start;
			int current;

		protected:

			GameState state;

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

			void TestBehaviourTree();

			vector <Vector3 > seekNodes;
			std::vector<Vector3> walls;
			std::vector<Vector3> route;

			void TestPathfinding(Vector3 target, vector<Vector3> &testNodes) {
				NavigationGrid grid("TestGrid1.txt");
				walls = grid.GetWalls();
				route = grid.GetRoute();
				NavigationPath outPath;


				int dist1 = NearestPoint(testStateObject->Position());
				int dist2 = NearestPoint(testStateObject2->Position());
				Vector3 startPos = route[dist1]; // Vector3(10, 0, 10);
				Vector3 endPos = route[dist2];//target; // (290, 0, 290);

				bool found = grid.FindPath(startPos, endPos, outPath);

				Vector3 pos;
				while (outPath.PopWaypoint(pos)) {
					testNodes.push_back(pos);
				}
				start = testNodes.size();
			}

			void DisplayPathfinding() {
				for (int i = 1; i < seekNodes.size(); ++i) {
					Vector3 a = seekNodes[i - 1];
					Vector3 b = seekNodes[i];

					Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
				}
			}

			Vector3 Seek(std::vector<Vector3> target, Vector3 seeker, Vector3 velocity, int start);
			Vector3 Flee(std::vector<Vector3> target, Vector3 seeker, Vector3 velocity, int start);
			Vector3 velocity;
			Vector3 seekforce;
			Vector3 fleeforce;
			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddMazeFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, bool active = true);
			GameObject* AddCubeToWorldOBB(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			
			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position, float radius, float inverseMass);
			StateGameObject* testStateObject;
			StateGameObject* testStateObject2;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;

			void Stairs();
			void JumpPad1();
			void JumpPad2();
			void AddRotatingBridges();
			void RotatingBridges(float angle);
			void AddRotatingPlatform();
			void RotatingPlatform(float angle);
			void AddWobblingPlatform();
			void WobblingPlatform(float angle, std::string dir);
			void AddTravelPlatform();
			void TravelPlatform(Vector3 lift);
			void AddMazePlatform();
			void mazePlatform(float angle, std::string dir);
			void AddProjectilePlatform();
			void AddGravityWell();
			void AddBridge();
			void AddBouncePad();
			void AddIcePatch();
			void AddTeleport();
			void AddGoal();
			void AddBonuses();

			void Addmazefloor();
			void AddWalls();

			int NearestPoint(Vector3 position);

			Vector3 SpawnPoint = Vector3(-100 / 8 * 3, 10, -100 / 8 * 3);
			Vector3 Checkpoint1 = Vector3(100 / 8 * 12.25, 100 / 8 * 4 + 10, -100 / 8 * 3);
			Vector3 Checkpoint2 = Vector3(100 / 8 * 12.5 + 100 / 12, 100 / 12 + 50, 100 / 8 * 4.75 + 1);
			Vector3 Checkpoint3 = Vector3(-3, 100 / 8 + 50, 100 / 8 * 10.75 + 1);
			Vector3 Checkpoint4 = Vector3(81, 100/8+55, 100/8*19+27);
			Vector3 Checkpoint5 = Vector3(201 - 100 / 8 * 3, 100 / 8 + 80, 100 / 8 * 19.5);
			Vector4 colour;
			Vector4 colour2;

			bool useGravity;
			bool inSelectionMode;
			bool rotateFloor;
			bool jumppad1 = false;
			bool jumppad2 = false;
			bool jumppadcollision1 = true;
			bool jumppadcollision2 = true;
			bool bridgeproject = false;
			bool hold = false;
			bool Teleport1 = false;
			bool Teleport2 = false;

			Vector3 stairlift = Vector3(0, 0.1, 0);
			Vector3 platLift = Vector3(-0.1 * cos(30 * 3.14 / 180), 0.1 * sin(30 *  3.14 / 180), 0);
			int liftcount = 0;
			float forceMagnitude;
			long starttime;
			long currenttime;
			int score = 0;
			Vector3 direction;

			GameObject* ActiveObject = nullptr;
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
			GameObject* plat1;
			GameObject* plat2;
			GameObject* plat3;
			GameObject* rotatingplat;
			GameObject* rotatingplat2;
			GameObject* rotatingplat3;
			GameObject* obs21;
			GameObject* Wplat;
			GameObject* liftPlat;
			GameObject* liftPlat2;
			GameObject* liftPlat3;
			GameObject* maze1;
			GameObject* maze2;
			GameObject* maze3;
			GameObject* maze4;
			GameObject* maze5;
			GameObject* maze6;
			GameObject* mazeplat;
			GameObject* mazeExit;
			GameObject* gravwell;
			GameObject* bounce;
			GameObject* teleport;
			GameObject* goal;
			
			vector<GameObject*> bonuses;

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

