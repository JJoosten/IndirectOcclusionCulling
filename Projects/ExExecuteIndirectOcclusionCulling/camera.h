#pragma once

#include <cfc/math/math.h>

struct view_state
{
	cfc::math::matrix4f ProjectionMatrix = cfc::math::matrix4f();
	cfc::math::matrix4f ViewMatrix = cfc::math::matrix4f();
	float ScreenWidth;
	float ScreenHeight;
};

class camera
{
public:
	camera(float focalLengthInMM = 50.0f, float sensorWidthInMM = 36.0f, float sensorHeightInMM = 24.0f, cfc::math::vector3f pos = cfc::math::vector3f());

	void LookAt(cfc::math::vector3f target = cfc::math::vector3f(), cfc::math::vector3f up = cfc::math::vector3f(0.0f, 1.0f, 0.0f) );
	
	void SetFocalLength(float focalLengthInMM);

	void SetPosition(const cfc::math::vector3f& position);

	cfc::math::vector3f GetPosition() const { return m_position; }

	cfc::math::vector3f GetLookAt() const { return m_lookAt; }

	view_state GetViewState() const { return m_viewState; }

private:
	float calculateFOVInDegrees() const;
	float calculateAspectRatio() const;

	void recalculateCameraMatrix();

private:
	view_state m_viewState;
	cfc::math::vector3f m_position;
	cfc::math::vector3f m_lookAt;
	cfc::math::vector3f m_lookAtUp;
	float m_focalLengthMM;
	float m_sensorWidthInMM;
	float m_sensorHeightInMM;
};