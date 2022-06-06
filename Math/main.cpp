/*
		THIS FILE IS ONLY FOR DEBUGGING
		TODO: DELETE DURING RELEASE
*/

#include <iostream>
#include <cmath>
#include <time.h>

#include "SkyIntrinsics.h"
#include "SkyMath.h"

using std::cout;
using std::endl;

void PrintVector3(Vector3D v);
void PrintVector4(Vector4D v);

void Mat_RotationTests();
void Quat_RotationTests();

int main() {

	/*Vector4D q = AxisAngleToQuaternion({ 1, 0, 0 }, 90);
	Vector4D p = AxisAngleToQuaternion({ 0, 1, 0 }, 180);
	PrintVector4(q);
	PrintVector4(p);
	PrintVector4(QuaternionProduct(q, p));*/

	//Mat_RotationTests();
	Quat_RotationTests();

	return 0;
}

void PrintVector3(Vector3D v) {
	cout << "{ " << v.x << ", " << v.y << ", " << v.z << "}" << endl;
}

void PrintVector4(Vector4D v) {
	cout << "{ " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "}" << endl;
}

inline void PassTest(int n) {
	cout << "PASSED: Test case " << n << endl;
}

inline void FailTest(int n) {
	cout << "FAILED: Test case " << n << endl;
}

void Mat_RotationTests() {
	//NOTE: Used to following to generate tests: https://www.vcalc.com/wiki/vCalc/V3+-+Vector+Rotation
	struct local {
		static void Test(Vector3D v, Vector3D axisAngle, Vector3D answer, int testN) {
			Matrix4X4 T = Identity();

			T = Rotate(T, axisAngle);

			Vector3D vR = Transform({ v, 1 }, T).xyz;
			if (std::llround(vR.x) == answer.x
				&& std::llround(vR.y) == answer.y
				&& std::llround(vR.z) == answer.z) {
				PassTest(testN);
			} else {
				FailTest(testN);
				cout << "    >> Expected: ";
				PrintVector3(answer);
				cout <<  "    >> Got: ";
				PrintVector3(vR);
			}
		}
	};

	cout << "========Matrix Rotation Tests========" << endl;

	local::Test({ 1, 0, 0 }, { 0, 0, 90 }, { 0, 1, 0 }, 1);
	local::Test({ 1, 1 ,1 }, { 0, 90, 0 }, { 1, 1, -1 }, 2);
	local::Test({ 0, 1 ,1 }, { 180, 0, 0 }, { 0, -1, -1 }, 3);
}

void Quat_RotationTests() {
	//NOTE: Used to following to generate tests: https://www.vcalc.com/wiki/vCalc/V3+-+Vector+Rotation
	struct local {
		static void Test(Vector3D v, Vector3D axis, float angle, Vector3D answer, int testN) {
			Vector4D q = AxisAngleToQuaternion(axis, angle);
			Vector3D vR = QuaternionRotate(v, q);
			if (std::llround(vR.x) == answer.x
				&& std::llround(vR.y) == answer.y
				&& std::llround(vR.z) == answer.z) {
				PassTest(testN);
			}
			else {
				FailTest(testN);
				cout << "    >> Expected: ";
				PrintVector3(answer);
				cout << "    >> Got: ";
				PrintVector3(vR);
			}
		}
	};

	cout << "======Quaternion Rotation Tests======" << endl;

	local::Test({ 1, 0, 0 }, { 0, 0, 1 }, 90, { 0, 1, 0 }, 1);
	local::Test({ 1, 1 ,1 }, { 0, 1, 0 }, 90, { 1, 1, -1 }, 2);
	local::Test({ 0, 1 ,1 }, { 1, 0, 0 }, 180, { 0, -1, -1 }, 3);
}