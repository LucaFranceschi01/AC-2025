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

// Extract the rotation and the translition from a matrix is easy. But not for the scale
// M = SRT, ignore the translation: M = SR -> invert R to isolate S
Transform mat4_to_transform(const mat4& m)
{
	// TODO
	// ..

	/* Tengo mis dudas sobre esta implementacion dado que la matriz de rotacion R tambien se le ha aplicado la transformacion de S, por lo que no termino de entender por que invertimos esto exactamente
	// 1. Extract the translation
	vec3 translation = vec3(m.tx, m.ty, m.tz);
	// 2. Extract the rotation
	mat4 rotationMatrix = mat4(
		m.xx, m.xy, m.xz, 0,
		m.yx, m.yy, m.yz, 0,
		m.zx, m.zy, m.zz, 0,
		0, 0, 0, 1
	);
	quat rotation = mat4_to_quat(rotationMatrix);
	
	mat4 inverseRotation = inverse(rotationMatrix);
	mat4 scaleMatrix = m * inverseRotation;
	// 3. Extract the scale
	vec3 scale = vec3(scaleMatrix.xx, scaleMatrix.yy, scaleMatrix.zz);

	return Transform(translation, rotation, scale);
	*/

	//source: https://math.stackexchange.com/questions/237369/given-this-transformation-matrix-how-do-i-decompose-it-into-translation-rotati
	//observacion: la escala al estar computando los modulos de los vectores, nunca dara un valor negativo, por lo que solo se puede escalar a mas grande
	vec3 translation = vec3(m.tx, m.ty, m.tz);
	vec3 scale = vec3(
		len(vec3(m.xx, m.xy, m.xz)),
		len(vec3(m.yx, m.yy, m.yz)),
		len(vec3(m.zx, m.zy, m.zz))
	);

	mat4 rotationMatrix = mat4(
		m.xx / scale.x, m.xy / scale.x, m.xz / scale.x, 0,
		m.yx / scale.y, m.yy / scale.y, m.yz / scale.y, 0,
		m.zx / scale.z, m.zy / scale.z, m.zz / scale.z, 0,
		0, 0, 0, 1
	);
	quat rotation = mat4_to_quat(rotationMatrix);

	return Transform(translation, rotation, scale);
	// return Transform();
}

// Converts a transform into a mat4
mat4 transform_to_mat4(const Transform& t)
{
	// TODO
	// ..
	mat4 m = mat4();
	// Scale
	m = scale(m, t.scale);

	// Rotation
	mat4 rotationMatrix = quat_to_mat4(t.rotation);
	m = m * rotationMatrix;

	// Translation
	m = translate(m, t.position);
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