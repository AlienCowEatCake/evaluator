#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <set>
#include <string>
#include <vector>

class parser
{
private:
    std::vector<std::string> expression;
    std::set<std::string> functions;
    std::map<std::string, double> constants;
    std::map<char, unsigned short int> operators;
    bool status;
    mutable std::string error_string;
    void init();
    void init_const();
    double calc_operator(const std::string & oper, const double larg, const double rarg) const;
    double calc_function(const std::string & func, const double arg) const;
public:
    parser();
    parser(const std::string & str);
    std::string get_error() const;
    void set_const(const std::string & name, const double value);
    void reset_const();
    bool is_parsed() const;
    bool parse(const std::string & str);
    bool simplify();
    bool calculate(double & result) const;
    void debug_print() const;
};

#endif // PARSER_H
