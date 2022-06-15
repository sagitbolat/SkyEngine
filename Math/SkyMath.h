#pragma once
/********************************
 *
 * Intrinsic functions declarations.
 *
 ********************************/

inline float SquareRoot(float a);
inline float CosDeg(float deg);
inline float SinDeg(float deg);

/********************************
 *
 * Vector Math
 *
 ********************************/

#define SQUARE(a) ((a)*(a))
#define CUBE(a) ((a)*(a)*(a))

union Vector3D {
	struct {
		float x, y, z;
	};
	struct {
		float r, g, b;
	};
	float E[3];
};
union Vector4D {
	struct {
		Vector3D xyz;
		float w;
	};
	struct {
		float x, y, z, w;
	};
	struct {
		//NOTE: Quaternion representation
		Vector3D v;
		float s;
	};
	struct {
		Vector3D rgb;
		float a;
	};
	struct {
		float r, g, b, a;
	};
	float E[4];
};

inline Vector3D I_Vector3D() {
	return { 1, 0, 0 };
}
inline Vector3D J_Vector3D() {
	return { 0, 1, 0 };
}
inline Vector3D K_Vector3D() {
	return { 0, 0, 1 };
}

inline Vector3D Vec3(float x, float y, float z) {
	return { x, y, z };
}
inline Vector3D Vec3(Vector4D v) {
	return { v.z, v.y, v.z};
}
inline Vector3D Vec3(Vector3D v) {
	return { v.x, v.y, v.z };
}

inline Vector4D Vec4(float x, float y, float z, float w) {
	return { x, y, z, w };
}
inline Vector4D Vec4(Vector3D v, float w) {
	return { v.x, v.y, v.z, w };
}
inline Vector4D Vec4(Vector4D v) {
	return { v.x, v.y, v.z, v.w };
}

inline Vector3D operator+(const Vector3D a, const Vector3D b) {
	return Vector3D{ a.x + b.x, a.y + b.y, a.z + b.z };
}
inline Vector3D operator-(const Vector3D a, const Vector3D b) {
	return Vector3D{ a.x - b.x, a.y - b.y, a.z - b.z };
}
inline Vector3D operator*(const Vector3D a, const float b) {
	return Vector3D{ a.x * b, a.y * b, a.z * b };	
}
inline Vector3D operator*(const float b, const Vector3D a) {
	return a * b;	
}
inline Vector3D operator/(const Vector3D a, const float b) {
	return Vector3D{ a.x / b, a.y / b, a.z / b }; 
}

inline Vector4D operator+(const Vector4D a, const Vector4D b) {
	return Vector4D{ a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}
inline Vector4D operator-(const Vector4D a, const Vector4D b) {
	return Vector4D{ a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}
inline Vector4D operator*(const Vector4D a, const float b) {
	return Vector4D{ a.x * b, a.y * b, a.z * b, a.w * b };
}
inline Vector4D operator*(const float b, const Vector4D a) {
	return a * b;
}
inline Vector4D operator/(const Vector4D a, const float b) {
	return Vector4D{ a.x / b, a.y / b, a.z / b, a.w / b }; 
}

static float VectorDot(const Vector3D a, const Vector3D b) {
	return a.x * b.x + a.y + b.y + a.z + b.z;
}

static Vector3D VectorCross(const Vector3D a, const Vector3D b){
	return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x); 
}

float Magnitude(const Vector3D vec) {	
	return SquareRoot(SQUARE(vec.x) + SQUARE(vec.y) + SQUARE(vec.z)); 
}

float Magnitude(const Vector4D vec) {
	return SquareRoot(SQUARE(vec.x) + SQUARE(vec.y) + SQUARE(vec.z) + SQUARE(vec.w));
}

Vector3D Normalize(const Vector3D vec) {	
	return (vec / Magnitude(vec)); 
}
	

/********************************
 *
 * Matrix Math
 *
 ********************************/

union Matrix4X4 {
	//NOTE: Stored ROW MAJOR. (mat.E[ROW][COLUMN] OR mat.D[ROW * 4 + COLUMN).
	float E[4][4];
	float D[16];
};

inline Matrix4X4 Identity() {
	return Matrix4X4{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
}

inline Matrix4X4 operator*(const Matrix4X4 A, const Matrix4X4 B) {
	Matrix4X4 Result = {};
	for (int r = 0; r < 4; ++r) {
		for (int i = 0; i < 4; ++i) {
			for (int c = 0; c < 4; ++c) {
				Result.E[r][c] += A.E[r][i] * B.E[i][c];
			}
		}	
	}
	return Result;
}

static Vector4D Transform(const Vector4D v, const Matrix4X4 T) {
	//NOTE: This is column wise multiplication, so the Vector [v] is represented as a column Vector
	Vector4D result = {};

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			result.E[r] += T.E[r][c] * v.E[c];
		}
	}
	return result;
}

static Matrix4X4 Translate(const Matrix4X4 A, const Vector3D transform) {
	//NOTE: Translates Matrix [A] by the 3-dimensional vector [transform].
	Matrix4X4 Result = A;
	
	Result.E[0][3] += transform.x;
	Result.E[1][3] += transform.y;
	Result.E[2][3] += transform.z;
	
	return Result;
}

static Matrix4X4 Rotate(const Matrix4X4 A, const Vector3D transform) {
	//NOTE: Rotates Matrix [A] by the 3-dimensional vector [transform].
	//Ex: Rotate(A, {0, 0, 90}) will rotate A by 90 degrees about the z axis
	//Ex: Rotate(A, {90, 90, 90}) will rotate A by 90 degrees about the vector (1, 1, 1)

	Matrix4X4 Result = Identity();
	
	//NOTE: Rotate about the X axis.
	Result.E[1][1] *= CosDeg(transform.x);
	Result.E[1][2] -= SinDeg(transform.x);
	Result.E[2][1] += SinDeg(transform.x);	
	Result.E[2][2] *= CosDeg(transform.x);
	
	//NOTE: Rotate about the Y axis.
	Result.E[0][0] *= CosDeg(transform.y);
	Result.E[0][2] += SinDeg(transform.y);
	Result.E[2][0] -= SinDeg(transform.y);
	Result.E[2][2] *= CosDeg(transform.y);

	//NOTE: Rotate about the Z axis.
	Result.E[0][0] *= CosDeg(transform.z);
	Result.E[0][1] -= SinDeg(transform.z);
	Result.E[1][0] += SinDeg(transform.z);
	Result.E[1][1] *= CosDeg(transform.z);
	
	Result = Result * A;

	return Result;
}	

static Matrix4X4 Scale(const Matrix4X4 A, const Vector3D transform) {	
	//NOTE: Scales Matrix [A] by the 3-dimensional vector [transform].
	Matrix4X4 Result = Identity();
	
	Result.E[0][0] *= transform.x;
	Result.E[1][1] *= transform.y;
	Result.E[2][2] *= transform.z;
	Result = Result * A;
	
	return Result;
}

inline Vector4D operator*(const Vector4D a, const Matrix4X4 B) {
	return Transform(a, B);
}
inline Vector3D operator*(const Vector3D a, const Matrix4X4 B) {
	return Transform(Vec4(a, 1.0f), B).xyz; 
}	

//TODO: Implement gaussian inverse alg
static Matrix4X4 InverseGaussian(const Matrix4X4 A) {
	Matrix4X4 Result = {};
	
	return Result;
}

static Matrix4X4 Transpose(const Matrix4X4 A) {
	Matrix4X4 Result = {};
	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			Result.E[r][c] = A.E[c][r];
		}
	}
	return Result;	
}

inline Matrix4X4 BasisMatrix(const Vector3D I, const Vector3D J, const Vector3D K, const Vector3D O) {
	return {
		I.x, J.x, K.x, O.x ,
		I.y, J.y, K.y, O.y,
		I.z, J.z, K.z, O.z,
		0,   0,   0,   1  
	};
}

inline Vector3D ModelToWorldCoords(const Vector3D v, const Matrix4X4 ModelBasis) {
	return Transform(Vec4(v.x, v.y, v.z, 1), ModelBasis).xyz;
}


/********************************
 *
 * Quaternion Math.
 *
 ********************************/

inline Vector4D Quaternion(float i, float j, float k, float w) {
	return Vec4(i, j, k, w);
}
inline Vector4D Quaternion(Vector3D q, float s) {
	return Vec4(q, s);
}

static Vector4D AxisAngleToQuaternion(const Vector3D axis, const float angle) {
	Vector3D axisN = Normalize(axis);
	float sin = SinDeg(angle / 2);
	return Quaternion(axisN.x * sin, axisN.y * sin, axisN.z * sin, CosDeg(angle / 2));
}

static Vector4D QuaternionProduct(const Vector4D p, const Vector4D q) {
	//NOTE: Returns the multiplication [pq] of the Quaternions [p] and [q].
	Vector4D Result = {};

	Result.w = p.w * q.w - p.x * q.x - p.y * q.y - p.z * q.z;
	Result.x = p.w * q.x + p.x * q.w + p.y * q.z - p.z * q.y;
	Result.y = p.w * q.y - p.x * q.z + p.y * q.w + p.z * q.x;
	Result.z = p.w * q.z + p.x * q.y - p.y * q.x + p.z * q.w;

	return Result;
}

static Vector4D QuaternionInverse(const Vector4D q) {
	Vector4D Result = {};
	
	Result.v = q.v * -1;
	Result.s = q.s;

	return Result / Magnitude(q);
}

static Vector3D QuaternionRotate(const Vector3D v, const Vector4D q) {
	Vector4D v4 = { v.x, v.y, v.z, 0 };
	Vector4D Result = QuaternionProduct(q, v4);
	Result = QuaternionProduct(Result, QuaternionInverse(q));

	return Result.xyz;
}
