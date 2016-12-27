#include "transition_table.h"

namespace evaluator_internal
{

void transition_table_record::set_values(const std::string & T, int J, bool A, bool S, bool R, bool E)
{
    Jump = J;
    Accept = A;
    Stack = S;
    Return = R;
    Error = E;

    std::size_t beg = 0, end = T.find_first_of(" \t\r\n");
    while(end != std::string::npos)
    {
        Terminals.push_back(T.substr(beg, end - beg));
        beg = end + 1;
        end = T.find_first_of(" \t\r\n", beg);
    }
    Terminals.push_back(T.substr(beg));
}

}

