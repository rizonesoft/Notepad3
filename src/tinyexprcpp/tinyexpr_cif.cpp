/* encoding: UTF-8
 * TinyExpr CPP : C-Interface implementation
 */
#include "tinyexpr_cif.h"
#include "tinyexpr.h"


extern "C" double te_interp(const char *expression, te_int_t* error)
{
    te_parser tep;
    bool const ok = tep.compile(expression);
    if (error) {
        *error = ok ? 0 : tep.get_last_error_position();
    }
    return tep.evaluate();
}


extern "C" double te_evaluate(const char* expression, const te_variable_c variables[], int var_count, te_int_t* error)
{
    std::vector<te_variable> vars;
    for (int i = 0; i < var_count; ++i) {
        vars.emplace_back(variables[i].name, variables[i].address, TE_DEFAULT);
    }
    te_parser tep;
    tep.set_vars(vars);
    bool const ok = tep.compile(expression);
    if (error) {
        *error = ok ? 0 : tep.get_last_error_position();
    }
    return tep.evaluate();
}

