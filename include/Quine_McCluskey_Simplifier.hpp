#pragma once
#include <bitset>
#include <iostream>
#include <fstream>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <tuple>
#include <regex>


/**
\file
\brief	Заголовочный файл с описанием класса Quine_McCluskey_Simplifier

Данный файл содержит в себе определения основных
классов, используемых в демонстрационной программе
*/

/**
\brief	Метод Квайна-МакКласки.

\detail Данный класс позволяет минимизировать булеву функцию,
заданную в виде наборов, на которых функция принимает значение 1,
по методу Квайна-МакКласки.

Данные считываются из заданного входного файла,
записываются в заданный выходной.
Имена файлов следует подавать из командной строки.
\data	Октябрь-ноябрь 2017 года.
*/
class Quine_McCluskey_Simplifier {
	/**
	Хранит разбиение наборов на группы по весу
	*/
	std::vector<std::vector<std::pair<std::string, bool>>> groups_;
	/**
	Все простые импликанты функции
	*/
	std::vector<std::string> implicants_;
	/**
	Входные данные - наборы, на которых функция принимает значение 1
	*/
	std::vector<std::string> input_sets_;
	/**
	Ядро функции
	*/
	std::vector<std::string> prime_;
	/**
	Полученная минимальная дизъюнктивная форма (МДНФ)
	*/
	std::set<std::string> mdnf_;
	/**
	Таблица покрытия простыми импликантами.
	table представляет собой таблицу, которая хранит наборы и импликанты в виде:\n
	<center><table>
	<caption id="multi_row">Таблица покрытия</caption>
	<tr><th><th>Набор 1<th>Набор 2<th>...<th>Набор n
	<tr><td align="center">Импликант 1<td align="center">1<td align="center">0<td align="center">...<td align="center">1
	<tr><td align="center">Импликант 2<td align="center">1<td align="center">0<td align="center">...<td align="center">0
	<tr><td align="center">...<td align="center">...<td align="center">...<td align="center">...<td align="center">...
	<tr><td align="center">Импликант m<td align="center">0<td align="center">1<td align="center">...<td align="center">1
	</table>\n</center>
	где 1 в ячейке ставится,
	если импликант j покрывает единицу функции на наборе i,
	0 - иначе.
	*/
	std::vector<std::pair<std::vector<size_t>, bool>> table_;

	auto align(const size_t, const size_t) -> void;
	auto create_groups(decltype(input_sets_)&) -> void;
	auto create_table(const decltype(input_sets_)&, const decltype(input_sets_)&) -> void;
	auto find_max_cover_ind(const std::vector<size_t>&) const->size_t;
	auto get_func_core()->std::vector<std::string>;
	auto get_implicants() -> void;
	auto get_weight(const std::string&) const->size_t;
	auto impl_to_formula(const std::string&) const->std::string;
	auto in_vect(const std::vector<std::string>&, const std::string&) const -> bool;
	auto in_vect(const std::vector<std::pair<std::string, size_t>>&, const std::string&) const -> bool;
	auto is_cover(const std::string&, const std::string&) const -> bool;
	auto is_neighbors(const std::string&, const std::string&) const -> bool;
	auto is_prime(const std::vector<size_t>&) const -> bool;
	auto num_of_vars() const->size_t;
	auto string_base10_to_base2(std::string) const->std::string;
public:
	Quine_McCluskey_Simplifier() {};
	Quine_McCluskey_Simplifier(const std::string & file_name);
	Quine_McCluskey_Simplifier(std::istream & ss);

	auto init(std::istream&, bool) -> void;
	auto simplify() -> void;
	auto print_formula(std::ostream&) const -> void;
	auto print_mdnf(std::ostream& os = std::cout) const -> void;
	auto print_mdnf(const std::string&) const -> void;
};