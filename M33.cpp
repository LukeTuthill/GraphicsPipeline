#define _USE_MATH_DEFINES
#include <cmath>

#include "m33.h"

M33::M33() {
	rows[0] = V3();
	rows[1] = V3();
	rows[2] = V3();
}

M33::M33(float scalar) {
	M33 &m = *this;
	m[0][0] = scalar;
	m[1][1] = scalar;
	m[2][2] = scalar;
}

M33::M33(V3 v1, V3 v2, V3 v3) {
	rows[0] = v1;
	rows[1] = v2;
	rows[2] = v3;
}

V3& M33::operator[](int i) {
	return rows[i];
}

V3 M33::get_column(int i) {
	V3 ret;
	M33 &m = *this;
	ret[0] = m[0][i];
	ret[1] = m[1][i];
	ret[2] = m[2][i];

	return ret;
}

void M33::set_column(int i, V3 v) {
	rows[0][i] = v[0];
	rows[1][i] = v[1];
	rows[2][i] = v[2];
}

V3 M33::operator*(V3 v) {
	V3 ret;
	ret[0] = rows[0] * v;
	ret[1] = rows[1] * v;
	ret[2] = rows[2] * v;
	return ret;
}

M33 M33::operator*(M33 m) {
	M33 ret;

	for (int j = 0; j < 3; j++) {
		V3 col = m.get_column(j);
		for (int i = 0; i < 3; i++) {
			ret[i][j] = rows[i] * col;
		}
	}

	return ret;
}

bool M33::operator==(M33 m2) {
	M33 &m1 = *this;
	for (int i = 0; i < 3; i++) {
		if (m1[i] != m2[i]) return false;
	}
	return true;
}

bool M33::operator!=(M33 m2) {
	return !(*this == m2);
}

ostream& operator<<(ostream &ostr, M33 m) {
	return ostr << m[0] << endl << m[1] << endl << m[2];
}

istream& operator>>(istream& ostr, M33& m) {
	return ostr >> m[0] >> m[1] >> m[2];
}

M33 M33::transposed() {
	M33 ret = *this;
	for (int i = 0; i < 3; i++) {
		for (int j = i + 1; j < 3; j++) {
			swap(ret[i][j], ret[j][i]);
		}
	}
	return ret;
}

M33 M33::inverted() {
	M33 ret;

	V3 a = get_column(0);
	V3 b = get_column(1);
	V3 c = get_column(2);

	V3 _a = b ^ c;
	_a /= (a * _a);
	
	V3 _b = c ^ a;
	_b /= (b * _b);

	V3 _c = a ^ b;
	_c /= (c * _c);

	ret[0] = _a;
	ret[1] = _b;
	ret[2] = _c;

	return ret;
}

void M33::set_as_x_rotation(float degrees) {
	float radians = degrees * ((float)M_PI / 180.0f);
	float c = cosf(radians);
	float s = sinf(radians);
	rows[0] = V3(1, 0, 0);
	rows[1] = V3(0, c, -s);
	rows[2] = V3(0, s, c);
}

void M33::set_as_y_rotation(float degrees) {
	float radians = degrees * ((float)M_PI / 180.0f);
	float c = cosf(radians);
	float s = sinf(radians);
	rows[0] = V3(c, 0, s);
	rows[1] = V3(0, 1, 0);
	rows[2] = V3(-s, 0, c);
}

void M33::set_as_z_rotation(float degrees) {
	float radians = degrees * ((float)M_PI / 180.0f);
	float c = cosf(radians);
	float s = sinf(radians);
	rows[0] = V3(c, -s, 0);
	rows[1] = V3(s, c, 0);
	rows[2] = V3(0, 0, 1);
}
