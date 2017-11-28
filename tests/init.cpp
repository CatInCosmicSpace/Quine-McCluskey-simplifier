#include "Quine_McCluskey_Simplifier.hpp"
#include "log_expr.hpp"
#include "catch.hpp"
#include <fstream>

SCENARIO("QMS: ctor", "[ctor]") {
	Quine_McCluskey_Simplifier test;

	REQUIRE(sizeof(test) != 0);
}

SCENARIO("QMS: ctor(str)", "[ctor(str)]") {
	std::string file_name("in_s.txt");
	Quine_McCluskey_Simplifier test(file_name);

	REQUIRE(sizeof(file_name) != 0);
}

SCENARIO("QMS: ctor(str) throw", "[ctor(str)] throw") {
	std::string file_name("not_ext_file.txt");

	REQUIRE_THROWS_AS(Quine_McCluskey_Simplifier(file_name), std::logic_error);
}

SCENARIO("QMS: ctor(str) nothrow", "[ctor(str)] nothrow") {
	std::string file_name("in_s.txt");

	REQUIRE_NOTHROW(Quine_McCluskey_Simplifier(file_name));
}

SCENARIO("QMS: init with formula, out sets", "[init(formula) -> sets]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_f("(x1   + x3) &	(x2&x4)");
	std::stringstream sin, sout, out;
	sin.str(in_f);

	REQUIRE_NOTHROW(log_expr(in_f));

	log_expr le(in_f);
	le.print(sout);

	REQUIRE(sout.str() == (std::string)"0000000000000111");
	REQUIRE_NOTHROW(QMS.init(sout, false));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_mdnf(out);

	REQUIRE(out.str() == (std::string)"11-1 111- ");
}

SCENARIO("QMS: init with formula, out formula", "[init(formula) -> formula]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_f("(x1   + x3) &	(x2&x4)");
	std::stringstream sin, sout, out;
	sin.str(in_f);

	REQUIRE_NOTHROW(log_expr(in_f));

	log_expr le(in_f);
	le.print(sout);

	REQUIRE(sout.str() == (std::string)"0000000000000111");
	REQUIRE_NOTHROW(QMS.init(sout, false));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_formula(out);

 	REQUIRE(out.str() == (std::string)"x0x1x3 x0x1x2 ");
}

SCENARIO("QMS: init with sets, out formula", "[init(sets) -> formula]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_s("1 4 10 5 15");
	std::stringstream in_ss(in_s), out;
	REQUIRE_NOTHROW(QMS.init(in_ss, true));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_formula(out);
	
	REQUIRE(out.str() == (std::string)"!x0!x2x3 !x0x1!x2 x0!x1x2!x3 x0x1x2x3 ");
}

SCENARIO("QMS: init with sets, out sets", "[init(sets) -> sets]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_s("1 4 10 5 15");
	std::stringstream in_ss(in_s), out;
	REQUIRE_NOTHROW(QMS.init(in_ss, true));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_mdnf(out);

	REQUIRE(out.str() == (std::string)"0-01 010- 1010 1111 ");
}

SCENARIO("QMS: init with incorrect vector", "[init(incorrect vector) -> logic_error]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_v("010101010111111111111111111111111111111");
	std::stringstream in_vs(in_v), out;
	REQUIRE_THROWS_AS(QMS.init(in_vs, false), std::logic_error);
}

SCENARIO("QMS: init with vector, out formula", "[init(vector) -> formula]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_v("0110011110000101");
	std::stringstream in_vs(in_v), out;
	REQUIRE_NOTHROW(QMS.init(in_vs, false));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_formula(out);

	REQUIRE(out.str() == (std::string)"x1x3 !x0!x2x3 !x0x2!x3 x0!x1!x2!x3 ");
}

SCENARIO("QMS: init with vector, out sets", "[init(vector) -> sets]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_v("0110011110000101");
	std::stringstream in_vs(in_v), out;
	REQUIRE_NOTHROW(QMS.init(in_vs, false));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_mdnf(out);

	REQUIRE(out.str() == (std::string)"-1-1 0-01 0-10 1000 ");
}

SCENARIO("QMS: test sets 1", "[ts1]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_s("2");
	std::stringstream in_vs(in_s), out;
	REQUIRE_NOTHROW(QMS.init(in_vs, true));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_mdnf(out);

	REQUIRE(out.str() == (std::string)"10 ");
}

SCENARIO("QMS: test sets 2", "[ts2]") {
	Quine_McCluskey_Simplifier QMS;
	std::string in_s("0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15");
	std::stringstream in_vs(in_s), out;
	REQUIRE_NOTHROW(QMS.init(in_vs, true));
	REQUIRE_NOTHROW(QMS.simplify());

	QMS.print_mdnf(out);

	REQUIRE(out.str() == (std::string)"---- ");
}