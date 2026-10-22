#include "Renderer/Camera.h"

using namespace DirectX;

XMMATRIX Camera::View() const
{
	XMVECTOR position = XMLoadFloat3(&m_vPosition);
	XMVECTOR target = XMLoadFloat3(&m_vTarget);
	XMVECTOR up = XMLoadFloat3(&m_vUp);

	// 앞쪽 = normalize(바라보는 점 − 위치)
	// 오른쪽 = normalize(cross(위쪽, 앞쪽)) ← 외적
	// 진짜 위쪽 = cross(앞쪽, 오른쪽)
	// 이 세 방향으로 회전하고, −위치만큼 이동하는 행렬을 만들어요.
	return XMMatrixLookAtLH(position, target, up);
}

XMMATRIX Camera::Projection() const
{
	return XMMatrixPerspectiveFovLH(m_fFieldOfView, m_fAspect, m_fNear, m_fFar);
}


void Camera::SetPosition(XMFLOAT3 vNewPosition)
{
	m_vPosition = vNewPosition;
}

void Camera::SetTarget(XMFLOAT3 vNewTarget)
{
	m_vTarget = vNewTarget;
}

void Camera::SetUp(XMFLOAT3 vNewUp)
{
	m_vUp = vNewUp;
}

void Camera::SetFieldOfView(float fNewFieldOfView)
{
	m_fFieldOfView = fNewFieldOfView;
}

void Camera::SetAspect(float fNewAspect)
{
	m_fAspect = fNewAspect;
}

void Camera::SetNear(float fNewNear)
{
	m_fNear = fNewNear;
}

void Camera::SetFar(float fNewFar)
{
	m_fFar = fNewFar;
}

XMFLOAT3 Camera::GetPosition() const
{
	return m_vPosition;
}

XMFLOAT3 Camera::GetTarget() const
{
	return m_vTarget;
}

XMFLOAT3 Camera::GetUp() const
{
	return m_vUp;
}

float Camera::GetFieldOfView() const
{
	return m_fFieldOfView;
}

float Camera::GetAspect() const
{
	return m_fAspect;
}

float Camera::GetNear() const
{
	return m_fNear;
}

float Camera::GetFar() const
{
	return m_fFar;
}
