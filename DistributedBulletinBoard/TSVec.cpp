#include "TSVec.h"

TSVec::TSVec(int size)
: _values(size)
{ }


int& TSVec::operator[](int index)
{
	assert(0 <= index && index < _values.size() && "Illegal index specified.");
	return _values[index];
}

bool TSVec::operator==(const TSVec& rhs) const
{
	for(int i = 0; i < size(); ++i) {
		if(_values[i] != rhs._values[i])
			return false;
	}

	return true;
}

bool TSVec::operator!=(const TSVec& rhs) const
{
	return !(*this == rhs);
}

// u <= v iff u[k] <= v[k] (for all k = 0, ..., n-1)
bool TSVec::operator<=(const TSVec& rhs) const
{
	for(int i = 0; i < size(); ++i) {
		if(_values[i] > rhs._values[i])
			return false;
	}

	return true;
}

// u < v iff u <= k and u != v
bool TSVec::operator<(const TSVec& rhs) const
{
	return (*this <= rhs && *this != rhs);
}

std::string TSVec::toString() const
{
    std::ostringstream oss;
    oss << "<";
	for(int i = 0; i < size(); ++i) {

        oss << _values[i];
        if (i != size()-1)
            oss << ", ";
    }
    oss << ">";
    return oss.str();
}


TSVec TSVec::max(const TSVec& l, const TSVec& r)
{
    if (l.size() != r.size())
        return TSVec(1);
    TSVec result(l.size());
    for (int i = 0; i < l.size(); ++i) {
        result[i] = std::max(l._values[i], r._values[i]);
    }
    return result;
}