#include "pch.hpp"

bool Camera::Update(const Transform& _transform)
{
	if(dirty || _transform.IsDirty())
	{
		if (dirty) UpdateProjection();
		if (_transform.IsDirty()) UpdateView(_transform);

		cameraUBO.viewProjection = cameraUBO.projection * cameraUBO.view;
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
		cameraUBO.projection = glm::perspectiveRH_ZO(glm::radians(fov), aspectRatio, near, far);
		cameraUBO.projection[1][1] *= -1; // Flip the y-axis
		break;
	case CameraType::Orthographic:
		cameraUBO.projection = glm::orthoRH_ZO(left, right, bottom, top, near, far);
		cameraUBO.projection[1][1] *= -1; // Flip the y-axis
		break;
	}

	cameraUBO.nearFar = glm::vec2(near, far);
}

void Camera::UpdateView(const Transform& _transform)
{
	cameraUBO.view = glm::lookAtRH(_transform.GetPosition(), _transform.GetPosition() + _transform.GetForward(), VEC3_UP);
}
