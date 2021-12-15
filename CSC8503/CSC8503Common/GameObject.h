#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "Ray.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			bool GetCollision() const {
				return collision;
			}

			void setCollsion(bool active) {
				collision = active;
			}

			Ray CreateRay() {
				GameObject* obj = this;
				return Ray(Position(), Forward());
			}

			Vector3 Forward() {
				return transform.GetForward();
			}

			Vector3 Position() {
				return transform.GetPosition();
			}

			void Respawn(Vector3 spawnPoint) {
				transform.SetPosition(spawnPoint);
			}

			void Move(Vector3 movement) {
				transform.SetPosition(transform.GetPosition() + movement);
			}

			Transform& GetTransform() {
				return transform;
			}

			void SetTransform(Transform T) {
				transform = T;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}

			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			void SetOrientation(Quaternion rotate) {
				Quaternion current = transform.GetOrientation();
				current = current * rotate;
				transform.SetOrientation(current);
			}

			const string& GetName() const {
				return name;
			}

			void SetName(std::string n) {
				name = n;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int		GetWorldID() const {
				return worldID;
			}

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;
			bool	collision;
			bool	isActive;
			int		worldID;
			string	name;
			Vector3 broadphaseAABB;
		};
	}
}

