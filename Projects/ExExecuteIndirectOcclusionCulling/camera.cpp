#include "camera.h"

#define RAD_TO_DEGREES(x) (x * 57.2958f)
#define POW2(x) (x * x)


camera::camera(float focalLengthInMM /* = 50.0f */, float sensorWidthInMM /*= 36.0f */, float sensorHeightInMM /*= 24.0f*/, cfc::math::vector3f pos /* = cfc::math::vector3f() */)
	:
m_viewState()
,m_focalLengthMM(focalLengthInMM)
,m_position(pos)
,m_lookAt()
,m_sensorWidthInMM(sensorWidthInMM)
,m_sensorHeightInMM(sensorHeightInMM)
{
	recalculateCameraMatrix();
}


void camera::LookAt(cfc::math::vector3f target /*= cfc::math::vector3f()*/, cfc::math::vector3f up /*= cfc::math::vector3f(0.0f, 1.0f, 0.0f) */)
{
	m_lookAt = target;
	m_lookAtUp = up;

	m_viewState.ViewMatrix = cfc::math::matrix4f::View(m_position, m_lookAt, m_lookAtUp);
}

void camera::SetFocalLength(float focalLengthInMM)
{
	if (m_focalLengthMM == focalLengthInMM)
		return;

	m_focalLengthMM = focalLengthInMM;
	recalculateCameraMatrix();
}

void camera::SetPosition(const cfc::math::vector3f& position)
{
	m_position = position;
	m_viewState.ViewMatrix = cfc::math::matrix4f::View(m_position, m_lookAt, m_lookAtUp);
}

float camera::calculateFOVInDegrees() const
{
	float sensorDiameterInMillimeters = sqrt(POW2((float)m_sensorWidthInMM) + POW2((float)m_sensorHeightInMM));

	return RAD_TO_DEGREES(2.0f * atan(sensorDiameterInMillimeters / (2.0f * m_focalLengthMM)));
}

float camera::calculateAspectRatio() const
{
	return m_sensorWidthInMM / m_sensorHeightInMM;
}

void camera::recalculateCameraMatrix()
{
	m_viewState.ProjectionMatrix.SetProjection(calculateFOVInDegrees(), calculateAspectRatio(), .1f, 1000.0f);
}