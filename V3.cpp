#define _USE_MATH_DEFINES
#include <cmath>

#include "v3.h"
#include "m33.h"

V3::V3() {}

using namespace std;

V3::V3(float x, float y, float z) {
	xyz[0] = x;
	xyz[1] = y;
	xyz[2] = z;
}

float& V3::operator[](int i) {
	return xyz[i];
}

V3 V3::operator+(V3 v2) {
	V3 ret;
	V3 &v1 = *this;
	for (int i = 0; i < 3; i++) {
		ret[i] = v1[i] + v2[i];
	}
	return ret;
}

V3& V3::operator+=(V3 v2) {
	V3& v1 = *this;
	for (int i = 0; i < 3; i++) {
		v1[i] += v2[i];
	}
	return v1;
}

V3 V3::operator-(V3 v2) {
	V3 ret;
	V3 &v1 = *this;
	for (int i = 0; i < 3; i++) {
		ret[i] = v1[i] - v2[i];
	}
	return ret;
}

V3& V3::operator-=(V3 v2) {
	V3& v1 = *this;
	for (int i = 0; i < 3; i++) {
		v1[i] -= v2[i];
	}
	return v1;
}

float V3::operator*(V3 v2) {
	float ret = 0.0f;
	V3 &v1 = *this;
	for (int i = 0; i < 3; i++) {
		ret += v1[i] * v2[i];
	}
	return ret;
}

V3 V3::operator^(V3 v2) {
	V3 ret;
	V3 &v1 = *this;
	ret[0] = v1[1] * v2[2] - v1[2] * v2[1];
	ret[1] = v1[2] * v2[0] - v1[0] * v2[2];
	ret[2] = v1[0] * v2[1] - v1[1] * v2[0];
	return ret;
}

V3 V3::operator*(float scalar) {
	V3 ret;
	V3 &v = *this;
	for (int i = 0; i < 3; i++) {
		ret[i] = v[i] * scalar;
	}
	return ret;
}

V3 operator*(float scalar, V3 v) {
	return v * scalar;
}
 
V3& V3::operator*=(float scalar) {
	V3& v = *this;
	for (int i = 0; i < 3; i++) {
		v[i] *= scalar;
	}
	return v;
}

V3 V3::operator/(float scalar) {

	if (scalar == 0.0f) {
		throw "Division by zero in vector division";
	}

	V3 ret;
	V3 &v = *this;
	for (int i = 0; i < 3; i++) {
		ret[i] = v[i] / scalar;
	}
	return ret;
}

V3& V3::operator/=(float scalar) {
	if (scalar == 0.0f) {
		throw "Division by zero in vector division";
	}

	V3& v = *this;
	for (int i = 0; i < 3; i++) {
		v[i] /= scalar;
	}
	return v;
}

bool V3::operator==(V3 v2) {
	V3 &v1 = *this;
	for (int i = 0; i < 3; i++) {
		if (v1[i] != v2[i]) return false;
	}
	return true;
}

bool V3::operator!=(V3 v2) {
	return !(*this == v2);
}

ostream& operator<<(ostream& ostr, V3 v) {
	return ostr << v[0] << " " << v[1] << " " << v[2];
}

istream& operator>>(istream& istr, V3& v) {
	return istr >> v[0] >> v[1] >> v[2];
}

float V3::length() {
	V3 &v = *this;
	return sqrt(v * v);
}

V3 V3::normalized() {
	V3 &v = *this;
	float len = v.length();
	if (len == 0.0f) {
		throw "Division by zero in vector normalization";
	}
	return v / len;
}

V3 V3::rotate_point(V3 Oa, V3 a, float rotation_degrees) {
	V3& p = *this;

	V3 axis = a.normalized();

	//Sets to x or y basis vector depending on which is more perpendicular to axis
	//No need for dot product since axis is normalized and basis vectors are unit length
	V3 temp = (fabsf(axis[0]) < fabsf(axis[1])) ? V3(1, 0, 0) : V3(0, 1, 0); 
	V3 u = (temp ^ axis).normalized();
	V3 v = (axis ^ u).normalized();

	M33 basis(u, v, axis);

	// Transform and translate to new coordinate system
	V3 p_local = basis * (p - Oa);  // transpose is inverse for orthonormal matrices

	// Rotate around Z-axis in local coordinates
	M33 rotation_matrix;
	rotation_matrix.set_as_z_rotation(rotation_degrees);
	V3 p_rotated = rotation_matrix * p_local;

	// Transform back and translate
	return basis.transposed() * p_rotated + Oa;
}

V3 V3::rotate_direction(V3 a, float rotation_degrees) {
	return rotate_point(V3(), a, rotation_degrees);
}

unsigned int V3::convert_to_color_int() {
	unsigned int r = (unsigned int)(fmaxf(0.0f, fminf(1.0f, xyz[0])) * 255.0f);
	unsigned int g = (unsigned int)(fmaxf(0.0f, fminf(1.0f, xyz[1])) * 255.0f);
	unsigned int b = (unsigned int)(fmaxf(0.0f, fminf(1.0f, xyz[2])) * 255.0f);
	return 0xFF000000 + (b << 16) + (g << 8) + r;
}

void V3::set_as_color(unsigned int color) {
	xyz[0] = (float)(color & 0x000000FF) / 255.0f;
	xyz[1] = (float)((color & 0x0000FF00) >> 8) / 255.0f;
	xyz[2] = (float)((color & 0x00FF0000) >> 16) / 255.0f;
}
