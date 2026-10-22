#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	DirectX::XMMATRIX View() const;
	DirectX::XMMATRIX Projection() const;

	void SetPosition(DirectX::XMFLOAT3 vNewPosition);
	void SetTarget(DirectX::XMFLOAT3 vNewTarget);
	void SetUp(DirectX::XMFLOAT3 vNewUp);

	void SetFieldOfView(float fNewFieldOfView);
	void SetAspect(float fNewAspect);
	void SetNear(float fNewNear);
	void SetFar(float fNewFar);

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetTarget() const;
	DirectX::XMFLOAT3 GetUp() const;

	float GetFieldOfView() const;
	float GetAspect() const;
	float GetNear() const;
	float GetFar() const;

private:
	DirectX::XMFLOAT3 m_vPosition = { 0.0f, 0.0f, -5.0f };
	DirectX::XMFLOAT3 m_vTarget = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_vUp = { 0.0f, 1.0f, 0.0f };

	float m_fFieldOfView = DirectX::XM_PIDIV4;	// radians
	float m_fAspect = 1.0f;
	float m_fNear = 0.01f;
	float m_fFar = 100.0f;
};
