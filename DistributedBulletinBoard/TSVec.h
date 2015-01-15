#pragma once

#include <boost/mpi.hpp>
#include <boost/serialization/set.hpp>

/*
	Timestamp vector class, handling comparators etc.
*/
class TSVec
{
	friend class boost::serialization::access;
public:
	TSVec(int size);

	int& operator[](int index); 

	bool operator==(const TSVec& rhs) const;
	bool operator!=(const TSVec& rhs) const;
	bool operator<=(const TSVec& rhs) const;
	bool operator<(const TSVec& rhs) const;

	std::string toString() const;

    static TSVec max(const TSVec& l, const TSVec& r);

	int size() const { return _values.size(); }

private:
	std::vector<int> _values;
    
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & _values;
	}
};