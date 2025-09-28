#pragma once

#include <ostream>
#include <istream>
using namespace std;

class V3 {
public:
	float xyz[3] = { 0.0f, 0.0f, 0.0f };
	V3();
	V3(float x, float y, float z);

	float& operator[](int i);

	V3 operator+(V3 v2);
	V3& operator+=(V3 v2);
	V3 operator-(V3 v2);
	V3& operator-=(V3 v2);

	float operator*(V3 v2);
	V3 operator^(V3 v2);
	V3 operator*(float scalar);
	V3& operator*=(float scalar);
	friend V3 operator*(float scalar, V3 v); // Scalar multiplication from left
	V3 operator/(float scalar);
	V3& operator/=(float scalar);

	bool operator==(V3 v2);
	bool operator!=(V3 v2);

	friend ostream& operator<<(ostream& ostr, V3 v);
	friend istream& operator>>(istream& ostr, V3& v);

	float length();
	V3 normalized();

	V3 rotate_point(V3 Oa, V3 a, float rotation_degrees);
	V3 rotate_direction(V3 a, float rotation_degrees);

	unsigned int convert_to_color_int();
	void set_as_color(unsigned int color);

	V3 lighted(V3 n, V3 ld, float ka);
	void light(V3 n, V3 ld, float ka);

	V3 reflected(V3 l);
};
