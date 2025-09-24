#pragma once

#include <ostream>
#include "v3.h"

class M33 {
public:
	V3 rows[3];
	
	M33();
	M33(float scalar); //For diagonal matrices, use 1 for identity
	M33(V3 v1, V3 v2, V3 v3);

	V3& operator[](int i); //Can use V3 read/write access for each vector
	V3 get_column(int i);
	void set_column(int i, V3 v);

	V3 operator*(V3 v);
	M33 operator*(M33 m);
	bool operator==(M33 m2);
	bool operator!=(M33 m2);

	friend ostream& operator<<(ostream& ostr, M33 m);
	friend istream& operator>>(istream& ostr, M33& m);

	M33 transposed();
	M33 inverted();

	void set_as_x_rotation(float degrees);
	void set_as_y_rotation(float degrees);
	void set_as_z_rotation(float degrees);
};
