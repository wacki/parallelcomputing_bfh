
#include "Msg.h"


std::string Message::toString() const
{
    
    std::ostringstream oss;
    oss << _ts.toString() << "    ";
    oss << "msg: " << _title << " from FE " << _userId;
    oss << " (prev " << _prev.toString() << ")";
    oss << "   cid: " << _cid;
    return oss.str();
}