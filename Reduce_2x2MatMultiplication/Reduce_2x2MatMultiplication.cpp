#include <boost/mpi.hpp>

#include <iostream>

#pragma warning(disable:4996)

namespace mpi = boost::mpi;

struct Mat2
{
	friend class boost::serialization::access;
	float m[2][2];

	Mat2(float m00 = 1.0f, float m01 = 1.0f,
		 float m10 = 1.0f, float m11 = 1.0f)
	{
		// vs 2013 doesn't allow for explicit initialization of arrays...
		m[0][0] = m00;
		m[0][1] = m01;
		m[1][0] = m10;
		m[1][1] = m11;
	}
	
	Mat2 operator*(const Mat2& right) const
	{
		Mat2 result;
		result.m[0][0] = m[0][0] * right.m[0][0] + m[0][1] * right.m[1][0];
		result.m[0][1] = m[0][0] * right.m[0][1] + m[0][1] * right.m[1][1];
		result.m[1][0] = m[1][0] * right.m[0][0] + m[1][1] * right.m[1][0];
		result.m[1][1] = m[1][0] * right.m[0][1] + m[1][1] * right.m[1][1];

		return result;
	}

	friend std::ostream& operator<<(std::ostream& os, const Mat2& mat)
	{
		os << "{ {" << mat.m[0][0] << ", " << mat.m[0][1] << "}, {" << mat.m[1][0] << ", " << mat.m[1][1] << "} }";
		return os;
	}

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & m;
	}
};

int main()
{
	// mpi stuff
	mpi::environment	env;
	mpi::communicator	world;

	// data for this process
	const int maxVal = 5;
	Mat2 in;

	// initialize in with random values
	std::srand(world.rank());
	in = Mat2(std::rand() % maxVal, std::rand() % maxVal, std::rand() % maxVal, std::rand() % maxVal);
	std::cout << "p" << world.rank() << " matrix = " << in << std::endl;


	if(world.rank() == 0) {
		Mat2 out;
		reduce(world, in, out, std::multiplies<Mat2>(), 0);
		std::cout << "\n-----------------\n";
		std::cout << "The result is " << out << std::endl;
	}
	else {
		reduce(world, in, std::multiplies<Mat2>(), 0);
	}



	return 0;
}