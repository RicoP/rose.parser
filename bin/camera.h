#include <cstdio>

struct Camera {
	int x;
	int y;
	int z;
};

struct Transform {
	Camera camera;
	vector3 position;
};

