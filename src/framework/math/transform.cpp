#include "transform.h"
#include <math.h>

// Transforms can be combined in the same way as matrices and quaternions and the effects of two transforms can be combined into one transform
// To keep things consistent, combining transforms should maintain a right-to-left combination order
Transform combine(const Transform& t1, const Transform& t2)
{
	Transform out;
	out.scale = t1.scale * t2.scale;
	out.rotation = t2.rotation * t1.rotation; // right-to-left multiplication (right is the first rotation applyed)
	// The combined position needs to be affected by the rotation and scale
	out.position = t1.rotation * (t1.scale * t2.position); // M = R*S*T
	out.position = t1.position + out.position;
	return out;
}

Transform inverse(const Transform& t)
{
	Transform inv;
	inv.rotation = inverse(t.rotation);
	inv.scale.x = fabs(t.scale.x) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.x;
	inv.scale.y = fabs(t.scale.y) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.y;
	inv.scale.z = fabs(t.scale.z) < VEC3_EPSILON ? 0.0f : 1.0f / t.scale.z;
	vec3 invTrans = t.position * -1.0f;
	inv.position = inv.rotation * (inv.scale * invTrans);
	return inv;
}

Transform mix(const Transform& a, const Transform& b, float t)
{
	quat bRot = b.rotation;
	if (dot(a.rotation, bRot) < 0.0f) {
		bRot = -bRot;
	}
	return Transform(
		lerp(a.position, b.position, t),
		nlerp(a.rotation, bRot, t),
		lerp(a.scale, b.scale, t));
}

// Extract the rotation and the translation from a matrix is easy. But not for the scale
// M = SRT, ignore the translation: M = SR -> invert R to isolate S
Transform mat4_to_transform(const mat4& m)
{
	Transform t;

	// set the translation
	t.position.x = m.r0c3;
	t.position.y = m.r1c3;
	t.position.z = m.r2c3;

	// set the rotation directly
	t.rotation = mat4_to_quat(m);

	// set the scale
	mat4 inv_rot = quat_to_mat4(t.rotation);
	invert(inv_rot);

	mat4 scale = m * inv_rot;
	
	t.scale.x = scale.r0c0;
	t.scale.y = scale.r1c1;
	t.scale.z = scale.r1c1;

	return Transform();
}

// Converts a transform into a mat4
mat4 transform_to_mat4(const Transform& t)
{
	// set scale manually
	// would be the same if using a quad but it is technically not a quad
	mat4 m;
	m.r0c0 = t.scale.x;
	m.r1c1 = t.scale.y;
	m.r2c2 = t.scale.z;

	m = m * quat_to_mat4(t.rotation);
	
	m.r0c3 = t.position.x;
	m.r1c3 = t.position.y;
	m.r2c3 = t.position.z;
	
	return m;
}

vec3 transform_point(const Transform& a, const vec3& b)
{
	vec3 out;
	out = a.rotation * (a.scale * b);
	out = a.position + out;
	return out;
}

// First, apply the scale, then rotation
vec3 transform_vector(const Transform& t, const vec3& v)
{
	vec3 out;
	out = t.rotation * (t.scale * v);
	return out;
}