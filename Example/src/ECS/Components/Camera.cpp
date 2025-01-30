#include "pch.hpp"

bool Camera::Update(const Transform& _transform)
{
	if(dirty || _transform.IsDirty())
	{
		if (dirty) UpdateProjection();
		if (_transform.IsDirty()) UpdateView(_transform);

		viewProjection = projection * view;
		dirty = false;
		return true;
	}

	return false;
}

void Camera::UpdateProjection()
{
	switch (type)
	{
	case CameraType::Perspective:
		projection = glm::perspectiveRH_ZO(glm::radians(fov), aspectRatio, near, far);
		projection[1][1] *= -1; // Flip the y-axis
		break;
	case CameraType::Orthographic:
		projection = glm::orthoRH_ZO(left, right, bottom, top, near, far);
		projection[1][1] *= -1; // Flip the y-axis
		break;
	}
}

void Camera::UpdateView(const Transform& _transform)
{
	view = glm::lookAtRH(_transform.GetPosition(), _transform.GetPosition() + _transform.GetForward(), VEC3_UP);
}
