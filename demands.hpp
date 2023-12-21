#include <flatbuffers/flatbuffers.h>

class demands
{
public:
    demands(/* args */);
    ~demands();
private:
    std::vector<flatbuffers::Offset<demand>> demands;
    
};

demands::demands(/* args */)
{
}

demands::~demands()
{
}
