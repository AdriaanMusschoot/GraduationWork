#pragma once
#include <SDL_keyboard.h>
#include <SDL_mouse.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <numbers>

namespace sdf
{
	static float TO_RADIANS{ std::numbers::pi_v<float> / 180.f };
	struct Camera
	{
		explicit Camera(const glm::vec3& _origin, float _fovAngle, glm::vec3 const& forwards) :
			origin{ _origin },
			fovAngle{ _fovAngle },
			forward{ forwards }
		{
			CalculateCameraToWorld();
		}

		glm::vec3 origin{};
		float fovAngle{ 45.f };
		float fovValue{ tanf(fovAngle * TO_RADIANS / 2.0f) };
		const float SPEED_ROTATION{ 0.3f };
		const float SPEED_TRANSLATION{ 2.f };

		glm::vec3 forward{ 0.f, 0.f, 1.f };
		glm::vec3 up{ 0.f, 1.f, 0.f };
		glm::vec3 right{ 1.f, 0.f, 0.f };

		float totalPitch{ 0.f };
		float totalYaw{ 0.f };

		glm::mat3 cameraToWorld{};

		int mouseX{}, mouseY{};

		void CalculateCameraToWorld()
		{
			right = glm::cross(glm::vec3{ 0.f, 1.f, 0.f }, forward);
			right = glm::normalize(right);

			up = glm::cross(forward, right);
			up = glm::normalize(up);

			cameraToWorld = 
			{
				{ right.x, right.y, right.z },
				{ up.x, up.y, up.z },
				{ forward.x, forward.y, forward.z }
			};
		}

		void Update(float ElapsedTime)
		{
			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);
			
			if (pKeyboardState[SDL_SCANCODE_W] || pKeyboardState[SDL_SCANCODE_UP])
			{
				origin.x += forward.x * SPEED_TRANSLATION * ElapsedTime;
				origin.y += forward.y * SPEED_TRANSLATION * ElapsedTime;
				origin.z += forward.z * SPEED_TRANSLATION * ElapsedTime;
			}
			if (pKeyboardState[SDL_SCANCODE_S] || pKeyboardState[SDL_SCANCODE_DOWN])
			{
				origin.x -= forward.x * SPEED_TRANSLATION * ElapsedTime;
				origin.y -= forward.y * SPEED_TRANSLATION * ElapsedTime;
				origin.z -= forward.z * SPEED_TRANSLATION * ElapsedTime;
			}
			if (pKeyboardState[SDL_SCANCODE_D] || pKeyboardState[SDL_SCANCODE_RIGHT])
			{
				origin.x += right.x * SPEED_TRANSLATION * ElapsedTime;
				origin.y += right.y * SPEED_TRANSLATION * ElapsedTime;
				origin.z += right.z * SPEED_TRANSLATION * ElapsedTime;
			}
			if (pKeyboardState[SDL_SCANCODE_A] || pKeyboardState[SDL_SCANCODE_LEFT])
			{
				origin.x -= right.x * SPEED_TRANSLATION * ElapsedTime;
				origin.z -= right.z * SPEED_TRANSLATION * ElapsedTime;
				origin.y -= right.y * SPEED_TRANSLATION * ElapsedTime;
			}

			//Mouse Input
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			bool holdingLeftMouseButton{ (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0 };
			bool holdingRightMouseButton{ (mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0 };
			if (holdingRightMouseButton && holdingLeftMouseButton)
			{
				origin.y += up.y * mouseY * SPEED_TRANSLATION * ElapsedTime;
			}
			else if (holdingRightMouseButton)
			{
				totalPitch -= mouseX * SPEED_ROTATION * ElapsedTime;
				totalYaw -= mouseY * SPEED_ROTATION * ElapsedTime;
			}
			else if (holdingLeftMouseButton)
			{
				origin.x += forward.x * mouseY * SPEED_TRANSLATION * ElapsedTime;
				origin.y += forward.y * mouseY * SPEED_TRANSLATION * ElapsedTime;
				origin.z += forward.z * mouseY * SPEED_TRANSLATION * ElapsedTime;

				totalPitch += mouseX * SPEED_ROTATION * ElapsedTime;
			}
			if (holdingLeftMouseButton != holdingRightMouseButton)
			{
				glm::mat4 final{ glm::rotate(glm::mat4{ 1.0f }, totalYaw, glm::vec3{ 1.0f, 0.f, 0.f }) * glm::rotate(glm::mat4{ 1.0f }, totalPitch, glm::vec3{ 0.f, 1.0f, 0.f }) };
				forward = glm::mat3(final) * glm::vec3{ 0.f, 0.f, 1.f };
				forward = glm::normalize(forward);
				CalculateCameraToWorld();
			}
		}
	};
}
