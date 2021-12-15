#pragma once
#include "..\CSC8503Common\GameObject.h"
#include "../CSC8503Common/PhysicsSystem.h"
namespace NCL {
	namespace CSC8503 {
		class StateMachine;
		class StateGameObject : public GameObject {
		public:
			StateGameObject();
			~StateGameObject();

			virtual void Update(float dt, Vector3 force1, Vector3 force2);

		protected:
			void MoveLeft(float dt);
			void MoveRight(float dt);
			void Seek(float dt, Vector3 force);
			void Flee(float dt, Vector3 force);
			StateMachine* stateMachine;
			float counter;
			Vector3 state1force;
			Vector3 state2force;
		};
	}
}