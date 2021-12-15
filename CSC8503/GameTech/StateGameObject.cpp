
#include "StateGameObject.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/State.h"

 using namespace NCL;
 using namespace CSC8503;

 StateGameObject::StateGameObject() {
	 counter = 0.0f;
	 stateMachine = new StateMachine();

	 State* stateA = new State([&](float dt)-> void
		 {
			 this->Seek(dt, state1force);
			 std::cout << "I am seeking" << std::endl;
		 }
	 );
	 State* stateB = new State([&](float dt)-> void
		 {
			 this->Flee(dt, state2force);
			 std::cout << "I am fleeing" << std::endl;
		 }
	 );

	 stateMachine->AddState(stateA);
	 stateMachine->AddState(stateB);

	 stateMachine->AddTransition(new StateTransition(stateA, stateB,
		 [&]()-> bool
		 {
			 return this->counter > 3.0f;
		 }
	 ));

	 stateMachine->AddTransition(new StateTransition(stateB, stateA,
		 [&]()-> bool
		 {
			 return this->counter < 0.0f;
		 }
	 ));
 }

 StateGameObject ::~StateGameObject() {
	 delete stateMachine;
 }
 
 void StateGameObject::Update(float dt, Vector3 force1, Vector3 force2) {
	 stateMachine->Update(dt);
	 state1force = force1;
	 state2force = force2;
 }

 void StateGameObject::MoveLeft(float dt) {
	 GetPhysicsObject()->AddForce({ -1, 0, 0 });
	 counter += dt;
 }
 
 void StateGameObject::MoveRight(float dt) {
	 GetPhysicsObject()->AddForce({ 1, 0, 0 });
	 counter -= dt;
 }

 void StateGameObject::Seek(float dt, Vector3 force) {
	 GetPhysicsObject()->AddForce(force);
	 counter += dt;
 }

 void StateGameObject::Flee(float dt, Vector3 force) {
	 GetPhysicsObject()->AddForce(force / 10);
	 counter -= dt;
 }
 