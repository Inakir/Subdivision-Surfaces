#ifndef CLASSES_H
#define CLASSES_H
#define ImageW 400
#define ImageH 400

#include <vector>

using namespace std;

class Pt
{
public:
	float x, y, z;
	int index;
	int past_index;
	vector<int> faces_included;
	bool moved = false;

	Pt(void)
	{
		x = y = z = 0;
		past_index = -1;
	}

	Pt(float nX, float nY, float nZ, int i)
	{
		x = nX;
		y = nY;
		z = nZ;
		index = i;
		past_index = -1;
	}

	Pt(float nX, float nY, float nZ, int i, int pi)
	{
		x = nX;
		y = nY;
		z = nZ;
		index = i;
		past_index = pi;
	}

	void move(float nX, float nY, float nZ)
	{
		x = nX;
		y = nY;
		z = nZ;
		moved = true;
	}

	void mult(float m)
	{
		x *= m;
		y *= m;
		z *= m;
	}

	bool equals(float nx, float ny, float nz)
	{
		return (nx == x && ny == y && nz == z);
	}

	void add(int count)
	{
		faces_included.push_back(count);
	}

	void remove(int f)
	{
		for (int i = 0; i < faces_included.size(); i++)
		{
			if (faces_included[i] == f)
			{
				faces_included.erase(faces_included.begin() + i);
			}
		}
	}
};

class normalized_vec
{
public:
	float x, y, z, angle;

	normalized_vec() {}

	normalized_vec(Pt point1, Pt point2, Pt point3, Pt point4)
	{
		Pt v(point3.x - point1.x, point3.y - point1.y, point3.z - point1.z, -1);
		Pt w(point4.x - point2.x, point4.y - point2.y, point4.z - point2.z, -1);
		x = -v.y*w.z + v.z*w.y;
		y = -v.z*w.x + v.x*w.z;
		z = -v.x*w.y + v.y*w.x;
		float magnitude = sqrt(x*x + y*y + z*z);
		x = x / magnitude;
		y = y / magnitude;
		z = z / magnitude;
	}

	void invert()
	{
		x = -x;
		y = -y;
		z = -z;
	}
};

class triangle //used to find centroid
{
public:
	Pt centroid;
	triangle(Pt point1, Pt point2, Pt point3)
	{
		float nx = (point1.x + point2.x + point3.x) / 3;
		float ny = (point1.y + point2.y + point3.y) / 3;
		float nz = (point1.z + point2.z + point3.z) / 3;
		
		centroid = Pt(nx, ny, nz, -1);
	}
	
};

class Face
{
public:
	bool deleted = false;

	Pt point1;
	Pt point2;
	Pt point3;
	Pt point4;
	Pt center;

	normalized_vec normal;

	Face(Pt one, Pt two, Pt three)
	{
		point1 = one;
		point2 = two;
		point3 = three;

		normal = normalized_vec(point1, point2, point3, point4);
	}

	Face(Pt one, Pt two, Pt three, Pt four)
	{
		point1 = one;
		point2 = two;
		point3 = three;
		point4 = four;

		normal = normalized_vec(point1, point2, point3, point4);

		triangle t1(point1, point2, point3);
		triangle t2(point3, point4, point1);

		triangle t3(point1, point2, point4);
		triangle t4(point4, point3, point2);

		float nx = (t1.centroid.x + t2.centroid.x + t3.centroid.x + t4.centroid.x) / 4;
		float ny = (t1.centroid.y + t2.centroid.y + t3.centroid.y + t4.centroid.y) / 4;
		float nz = (t1.centroid.z + t2.centroid.z + t3.centroid.z + t4.centroid.z) / 4;
		center = Pt(nx, ny, nz, -1);
	}

	void normalize()
	{
		normal = normalized_vec(point1, point2, point3, point4);

		triangle t1(point1, point2, point3);
		triangle t2(point3, point4, point1);

		triangle t3(point1, point2, point4);
		triangle t4(point4, point3, point2);

		float nx = (t1.centroid.x + t2.centroid.x + t3.centroid.x + t4.centroid.x) / 4;
		float ny = (t1.centroid.y + t2.centroid.y + t3.centroid.y + t4.centroid.y) / 4;
		float nz = (t1.centroid.z + t2.centroid.z + t3.centroid.z + t4.centroid.z) / 4;
		center = Pt(nx, ny, nz, -1);
	}

	void refresh(Pt one, Pt two, Pt three, Pt four)
	{
		point1 = one;
		point2 = two;
		point3 = three;
		point4 = four;

		normal = normalized_vec(point1, point2, point3, point4);

		triangle t1(point1, point2, point3);
		triangle t2(point3, point4, point1);

		triangle t3(point1, point2, point4);
		triangle t4(point4, point3, point2);

		float nx = (t1.centroid.x + t2.centroid.x + t3.centroid.x + t4.centroid.x) / 4;
		float ny = (t1.centroid.y + t2.centroid.y + t3.centroid.y + t4.centroid.y) / 4;
		float nz = (t1.centroid.z + t2.centroid.z + t3.centroid.z + t4.centroid.z) / 4;
		center = Pt(nx, ny, nz, -1);
	}

	void warp(int index, float nx, float ny, float nz)
	{
		if (point1.index == index)
		{
			point1.move(nx, ny, nz);
		}
		else if (point2.index == index)
		{
			point2.move(nx, ny, nz);
		}
		else if (point3.index == index)
		{
			point3.move(nx, ny, nz);
		}
		else if (point4.index == index)
		{
			point4.move(nx, ny, nz);
		}
	}

};

#endif