#include "Quine_McCluskey_Simplifier.hpp"
#include <sstream>
#include <tuple>
#include <regex>

/*! \mainpage Домашнее задание по АиСД
*
* \section intro_sec Введение
*
* Работа выполнена в рамках домашнего задания по дисциплине
* "Алгоритмы и структуры данных",
* задание: "Напишите программу для получения минимальной ДНФ булевой
* функции".
*
* \section install_sec Installation
*
* \subsection step1 Step 1: Opening the box
*
* etc...
*/

//////////////////////////////////////////////
//                                          //
//        Quine_McCluskey_simplifier        //
//                                          //
//////////////////////////////////////////////

/**
Конструктор объекта класса. Создает объект по входному файлу (его имени), заполняя
поле input_sets и выравнивая наборы по количеству перменных
\param[in]		file_name	Имя открываемого файла
\throw	logic_error	Исключение, если файл не был открыт
*/
Quine_McCluskey_Simplifier::Quine_McCluskey_Simplifier(const std::string & file_name) {
	std::ifstream input(file_name);
	std::string temp;
	std::smatch m;
	std::regex e("d+");
	if (!input.is_open())
		throw std::logic_error("File not opened.");
	while (input.good()) {
		getline(input, temp);
		while (std::regex_search(temp, m, e)) {
			for (auto x : m)
				if (!in_vect(input_sets_, string_base10_to_base2(x)))
					input_sets_.push_back(string_base10_to_base2(x));
			temp = m.suffix().str();
		}
	}
	const size_t m_l = num_of_vars();
	groups_.resize(m_l + 1);
	for (auto i = 0; i < input_sets_.size(); ++i) {
		align(i, m_l);
	}
	input.close();
}

/**
Конструктор объекта класса. Создает объект по входному потоку, заполняя
поле input_sets и выравнивая наборы по количеству перменных. Предполагается, что 
вектор будет записан в одну строку, так что есть проверка на то, чтобы длина строки была 
степенью двойки
\param[in]		ss	Входной поток
\throw	logic_error	Исключение, если размер вектора не является степенью двойки
*/
Quine_McCluskey_Simplifier::Quine_McCluskey_Simplifier(std::istream & ss) {
	std::string temp;
	while (ss.good()) {
		getline(ss, temp);
		for (size_t i = 0; i < temp.length(); ++i) {
			if (temp[i] == '1')
				if (!in_vect(input_sets_, string_base10_to_base2(std::to_string(i))))
					input_sets_.push_back(string_base10_to_base2(std::to_string(i)));
		}
	}
	if (!((input_sets_.size() == 0) ? 0 : (input_sets_.size() & (input_sets_.size() - 1)) == 0))
		throw std::logic_error("Size of vector is invalid. Check your input.");

	const size_t m_l = num_of_vars();
	groups_.resize(m_l + 1);
	for (auto i = 0; i < input_sets_.size(); ++i) {
		align(i, m_l);
	}
}

/**
Выравнивает набор input_sets[i] по количеству переменных функции
\param[in]		index		Номер набора для выравнивания
\param[in]		size		Количество переменных
*/
auto Quine_McCluskey_Simplifier::align(const size_t index, const size_t size) -> void {
	const auto curr_size = input_sets_[index].size();
	for (auto j = 0; j < size - curr_size; ++j)
		input_sets_[index].insert(input_sets_[index].begin(), '0');
}

/**
Создает разбиение входных наборов (из is) на группы по их весам. Данные записывает в поле groups
\param[in]		is			Наборы для разбиения
*/
auto Quine_McCluskey_Simplifier::create_groups(decltype(input_sets_)& is) -> void {
	for (auto i : is) {
		groups_[get_weight(i)].push_back(make_pair(i, false));
	}
}

/**
Создает таблицу покрытия единиц импликантами, которые были переданы функции
\param[in]		ones			Единицы для покрытия
\param[in]		impls			Импликанты для покрытия
*/
auto Quine_McCluskey_Simplifier::create_table(const decltype(input_sets_)& ones, const decltype(input_sets_)& impls) -> void {
	// Cоздаем таблицу покрытия импликантами единиц функции.
	table_.resize(ones.size());
	for (auto i : table_)
		std::get<0>(i).resize(impls.size());
	for (auto i = 0; i < ones.size(); ++i) {
		for (auto j = 0; j < impls.size(); ++j) {
			if (is_cover(ones[i], impls[j])) {
				std::get<0>(table_[i]).push_back(1);
			}
			else {
				std::get<0>(table_[i]).push_back(0);
			}
		}
	}
}

/**
Посредством сложных (на самом деле, не очень) махинаций, находит индекс того импликанта, который
обеспечивает максимальное покрытие таблицы
\param[in]		vect			Вектор, в котором ищем максимальное покрытие
\params[out]	ind				Индекс импликанта, обеспечивающего максимальное покрытие
*/
auto Quine_McCluskey_Simplifier::find_max_cover_ind(const std::vector<std::size_t>& vect) const -> size_t {
	size_t ind = 0;
	for (auto i = 0; i < vect.size(); ++i)
		if (vect[i] > vect[ind])
			ind = i;
	return ind;
}

/**
Вычисляет ядро функции. Неочевидно, но возвращает те наборы, которые не покрыты ядром
\param[out]		not_covered_ones	Наборы, которые не покрыты ядром
*/
auto Quine_McCluskey_Simplifier::get_func_core() -> std::vector<std::string> {
	std::vector<std::string> to_choice;
	std::vector<std::string> not_covered_ones;
	for (auto i = 0; i < table_.size(); ++i) {
		if (is_prime(std::get<0>(table_[i]))) {
			for (auto j = 0; j < std::get<0>(table_[i]).size(); ++j) {
				if (std::get<0>(table_[i])[j] == 1) {
					if (!in_vect(prime_, implicants_[j])) {
						prime_.push_back(implicants_[j]);
					}
					std::get<1>(table_[i]) = true;
					for (auto k = 0; k < input_sets_.size(); ++k) {
						if (std::get<0>(table_[k])[j] == 1)
							std::get<1>(table_[k]) = true;
					}
					break;
				}
			}
		}
	}
	for (auto i = 0; i < table_.size(); ++i) {
		if (std::get<1>(table_[i]) == false) {
			not_covered_ones.push_back(input_sets_[i]);
		}
	}
	// Возвращает те единицы, которые не покрыты простыми импликантами.
	return not_covered_ones;
}

/**
Находит все простые импликанты функции, записывает их в поле implicants.
В функции используется лямбда:
\code
auto find_index = [this](auto x, auto y) { for (size_t i = 0; i < x.size(); ++i)
if (x[i] != y[i])
return i;
};
\endcode
Эта функция находит индекс, который нужно изменить с '0' или '1' на '-'.
Проще говоря, нужна для склейки
*/
auto Quine_McCluskey_Simplifier::get_implicants() -> void {
	auto find_index = [this](auto x, auto y) { for (size_t i = 0; i < x.size(); ++i)
		if (x[i] != y[i])
			return i;
	};
	create_groups(input_sets_);
	std::vector<std::string> tmp;
	auto find = true;
	// Пока находятся скейки
	while (find) {
		find = false;
		// Цикл по всей таблице
		for (auto i = 0; i < groups_.size() - 1; ++i) {
			// Цикл по строке таблице
			for (auto j = 0; j < groups_[i].size(); ++j) {
				// Цикл сравнения со всеми соседями
				for (auto k = 0; k < groups_[i + 1].size(); ++k) {
					if (is_neighbors(std::get<0>(groups_[i][j]), std::get<0>(groups_[i + 1][k]))) {
						auto ind = find_index(std::get<0>(groups_[i][j]), std::get<0>(groups_[i + 1][k]));
						std::get<1>(groups_[i][j]) = true;
						std::get<1>(groups_[i + 1][k]) = true;
						auto str = std::get<0>(groups_[i][j]);
						str[ind] = '-';
						tmp.push_back(str);
						find = true;
					}
				}
				if (std::get<1>(groups_[i][j]) == false) {
					if (!in_vect(implicants_, std::get<0>(groups_[i][j]))) {
						implicants_.push_back(std::get<0>(groups_[i][j]));
					}
				}
			}
		}
		groups_.clear();
		groups_.resize(num_of_vars() + 1);
		create_groups(tmp);
		tmp.clear();
	}
	// Здесь сформирован список всех минтермов. Ура!
}

/**
Возвращает вес набора - количество единиц в нем
\param[in]		set			Набор для вычисления веса
\param[out]		weight		Количество единиц в наборе
*/
auto Quine_McCluskey_Simplifier::get_weight(const std::string& set) const -> size_t {
	auto weight = 0;
	for (auto i : set)
		if (i == '1')
			++weight;
	return weight;
}

/**
Возвращает строку, которая содержит представление полученной МДНФ в формульном виде
\param[in]		impl			Набор, который нужно перевести в формулу
\param[out]		res				Строка-формула
*/
auto Quine_McCluskey_Simplifier::impl_to_formula(const std::string & impl) const -> std::string {
	std::string res;
	for (auto i = 0; i < impl.size(); ++i) {
		if (impl[i] == '1')
			res += "x" + std::to_string(i);
		else if (impl[i] == '0')
			res += "!x" + std::to_string(i);
		else if (impl[i] == '-')
			continue;
	}
	return res;
}

/**
Проверяет наличие элемента в векторе
\param[in]		vect		Вектор для проверки
\param[in]		s			Элемент, наличие которого нужно проверить
\param[out]		true/false	Если элемент найден, то true; иначе false
*/
auto Quine_McCluskey_Simplifier::in_vect(const std::vector<std::string>& vec, const std::string & s) const -> bool {
	for (const auto i : vec)
		if (i == s)
			return true;
	return false;
}

/**
Аналогичная функция, но проверяет для вектора пар (std::vector<std::pair<>>)
\param[in]		vect		Вектор для проверки
\param[in]		s			Элемент, наличие которого нужно проверить
\param[out]		true/false	Если элемент найден, то true; иначе false
*/
auto Quine_McCluskey_Simplifier::in_vect(const std::vector<std::pair<std::string, size_t>>& vec, const std::string & s) const -> bool {
	for (auto i : vec)
		if (std::get<0>(i) == s)
			return true;
	return false;
}

/**
Проверяет, покрывает ли второй набор первый
\param[in]		first		Первый набор
\param[in]		second		Второй набор
\param[out]		true/false	Если набор second покрывает набор first возвращает true;
иначе false
*/
auto Quine_McCluskey_Simplifier::is_cover(const std::string & first, const std::string & second) const -> bool {
	for (auto i = 0; i < first.size(); ++i)
		if (first[i] != second[i] && second[i] != '-')
			return false;
	return true;
}

/**
Соседние ли наборы? Нужна для попарного сравнения в группах
\param[in]		first		Первый набор
\param[in]		second		Второй набор
\param[out]		true/false	Если наборы соседние (различаются в одной координате),
то возвращает true; иначе false
*/
auto Quine_McCluskey_Simplifier::is_neighbors(const std::string& first, const std::string& second) const -> bool {
	size_t k = 0;
	for (auto i = 0; i < first.size(); ++i) {
		if (first[i] != second[i])
			++k;
		if (k > 1)
			return false;
	}
	return true;
}

/**
Проверяет, входит ли импликант в ядро функции. Если единица нашлась только одна, то вывод: эту единицу
может покрыть только этот импликант и следует внести его в ядро
\param[in]		x			Вектор для проверки
\param[out]		true/false	Если единица в векторе только одна, возвращает true;
иначе false
*/
auto Quine_McCluskey_Simplifier::is_prime(const std::vector<size_t>& x) const -> bool {
	auto k = 0;
	for (auto i : x) {
		if (i == 1)
			++k;
		if (k > 1)
			return false;
	}
	return true;
}

/**
Возвращает количество переменных рассматриваемой функции
\param[out]		k			Рассматривает максимальную длину строки (строки хранят
наборы), она и будет количеством переменных функции
*/
auto Quine_McCluskey_Simplifier::num_of_vars() const -> size_t {
	auto k = 0;
	for (auto i = 0; i < input_sets_.size(); ++i)
		if (input_sets_[i].length() > k)
			k = input_sets_[i].size();
	return k;
}

/**
Выводит полученную МДНФ в стандарный поток вывода - std::cout
\throw	logic_error	Кидает исключение, если минимизация не была произведена, а функция была вызвана
*/
auto Quine_McCluskey_Simplifier::print_mdnf(std::ostream& os) const -> void {
	if (mdnf_.size() == 0)
		throw std::logic_error("Minimization was not carried out");
	for (const auto i : mdnf_) {
		os << i << " ";
	}
}

/**
Выводит полученную МДНФ в файл file_name
\param[in] file_name	Имя выходного файла
\throw	logic_error	Кидает исключение, если минимизация не была произведена, а функция была вызвана
*/
auto Quine_McCluskey_Simplifier::print_mdnf(const std::string & file_name) const -> void {
	if (mdnf_.size() == 0)
		throw std::logic_error("Minimization was not carried out");
	std::ofstream output(file_name);
	for (const auto i : mdnf_) {
		output << i << " ";
	}
	output.close();
}

/**
Функция-инициализатор объекта. Инциализирует объект по потоку. Если sets установлен в true, 
то инциализирует по номерам наборов, в которых функция равна единице (в десятичном виде)
По своей сути аналогичек конструктору по потоку
\param[in] file_name	Имя выходного файла
\param[in] sets			По номерам наборов или нет?
\throw	logic_error	Исключение, если размер вектора не является степенью двойки
*/
auto Quine_McCluskey_Simplifier::init(std::istream& is, bool sets) -> void {
	if (sets == false) {
		*this = Quine_McCluskey_Simplifier(is);
		return;
	}
	std::string temp;
	std::smatch m;
	std::regex e("[[:digit:]]+");
	while (is.good()) {
		getline(is, temp);
		while (std::regex_search(temp, m, e)) {
			for (auto x : m)
				if (!in_vect(input_sets_, string_base10_to_base2(x)))
					input_sets_.push_back(string_base10_to_base2(x));
			temp = m.suffix().str();
		}
	}
	if (!((input_sets_.size() == 0) ? 0 : (input_sets_.size() & (input_sets_.size() - 1)) == 0))
		throw std::logic_error("Size of vector is invalid. Check your input.");

	const size_t m_l = num_of_vars();
	groups_.resize(m_l + 1);
	for (auto i = 0; i < input_sets_.size(); ++i) {
		align(i, m_l);
	}
}

/**
Главная функция доступа извне - создает внутри класса МДНФ
*/
auto Quine_McCluskey_Simplifier::simplify() -> void {
	get_implicants();
	create_table(input_sets_, implicants_);
	auto not_covered_ones = get_func_core();
	table_.clear();
	std::vector<std::string> not_prime_implicants;
	std::vector<size_t> every_impl_covers;
	for (const auto i : implicants_) {
		if (!in_vect(prime_, i)) {
			not_prime_implicants.push_back(i);
			every_impl_covers.push_back(0);
		}
	}

	// Новая таблица - таблица непокрытых единиц и всех импликант, 
	// не вошедших в ядро.
	create_table(not_covered_ones, not_prime_implicants);

	std::vector<std::vector<bool>> covering;
	std::vector<std::string> simple_cover;

	//is_cover(not_prime_implicants[j], not_covered_ones[j])
	std::vector<std::string> final_cover;
	while (not_covered_ones.size() != 0) {
		// Проходим по наборам, на которых функция равна 1
		for (auto i = 0; i < table_.size(); ++i) {
			// Теперь - по импликантам
			for (auto j = 0; j < std::get<0>(table_[i]).size(); ++j) {
				if (std::get<0>(table_[i])[j] == 1) {
					++every_impl_covers[j];
				}
			}
		}

		const auto ind = find_max_cover_ind(every_impl_covers);
		final_cover.push_back(not_prime_implicants[ind]);

		for (auto i = 0; i < table_.size(); ++i) {
			for (auto j = 0; j < table_.size(); ++j) {
				if (std::get<0>(table_[i])[ind] == 1) {
					std::get<1>(table_[i]) = true;
				}
			}
		}
		every_impl_covers.clear();
		decltype(not_prime_implicants) tmp;
		for (const auto i : not_prime_implicants) {
			if (!in_vect(final_cover, i)) {
				tmp.push_back(i);
				every_impl_covers.push_back(0);
			}
		}
		not_prime_implicants = tmp;
		tmp.clear();
		for (auto i = 0; i < not_covered_ones.size(); ++i) {
			if (std::get<1>(table_[i]) == false)
				tmp.push_back(not_covered_ones[i]);
		}
		not_covered_ones = tmp;
		tmp.clear();
		table_.clear();

		create_table(not_covered_ones, not_prime_implicants);
	}

	for (const auto i : prime_) {
		mdnf_.insert(i);
	}
	for (const auto i : final_cover) {
		mdnf_.insert(i);
	}
}

/**
Печатает в поток полученную МДНФ в формульном виде
\param[in]		os			Поток для печати
*/
auto Quine_McCluskey_Simplifier::print_formula(std::ostream& os) const -> void {
	if (mdnf_.size() == 0)
		throw std::logic_error("Function not simplified!");
	for (const auto i : mdnf_)
		os << impl_to_formula(i) << " ";
	os << std::endl;
}

/**
Конвертирует входную строку, представляющую десятичное число, в выходную строку,
представлляющую двоичную строку
\param[in]		inp		Десятичная строка
\param[out]		res		Двоичная строка
*/
auto Quine_McCluskey_Simplifier::string_base10_to_base2(std::string inp) const -> std::string {
	std::string res;
	std::string right;
	do {
		right.clear();
		int left = 0;
		for (size_t i = 0; i < inp.length(); ++i) {
			left = left * 10 + (inp[i] - '0');
			const auto div = left / 2;
			if ((i != 0) || (div != 0)) {
				right.push_back(div + '0');
				left %= 2;
			}
		}
		res.push_back(left + '0');
		inp = right;
	} while (inp != std::string());

	for (size_t i = 0; i < res.length() / 2; ++i)
		std::swap(res[i], res[res.length() - 1 - i]);
	return res;
}

//////////////////////////////////////////////
//                                          //
//                 log_expr                 //
//                                          //
//////////////////////////////////////////////

/**
Параметрический конструктор. Инициализирует все поля структуры
*/
log_expr::exp_node::exp_node(exp_node* prev, exp_type type, size_t data)
	: prev(prev),
	type(type),
	data(data),
	first(nullptr),
	operation(op_null),
	second(nullptr) {}

/**
Конструктор копирования, что тут еще сказать?
*/
log_expr::exp_node::exp_node(const exp_node& o)
	: prev(o.prev),
	type(o.type),
	data(o.data),
	first(o.first),
	operation(o.operation),
	second(o.second) {}

/**
Нужен для определения стадии парсинга. В соответствии с стадией выкидываются исключения, означающие ошибки в процессе парсинга
\param[out]		size_t		<table>
	<caption id="multi_row">Возвращаемые значения</caption>
	<tr><td align="center">0<td align="center">если поле не имеет на первого операнда, ни знака операции
	<tr><td align="center">1<td align="center">если есть первый операнд, но нет знака операции
	<tr><td align="center">2<td align="center">если есть первый операнд и знак операции, но нет второго операнда
	<tr><td align="center">3<td align="center">если все поля заполнены
	</table>\n
*/
auto log_expr::exp_node::state() const -> size_t {
	if (!operation) {
		if (!first)
			return 0;
		return 1;
	}
	if (!second)
		return 2;
	return 3;
}

/**
Принимает булев вектор того же размера, что и вектор идентификаторов и рекурсивно получает значение выражения (формулы)
\param[out]		bool	значение функции на наборе переменных
*/
auto log_expr::exp_node::process(std::vector<bool>& inps) const -> bool {
	switch (type) {
	case e_root:
	case e_node:
	case e_subnode: {

		switch (operation) {
		case op_and:
			return first->process(inps) & second->process(inps);
		case op_or:
			return first->process(inps) | second->process(inps);
		case op_not:
			return !second->process(inps);
		case op_null:
			return first->process(inps);
		}
		break;
	}

	case e_leaf:
		return inps[data];

	}
	throw std::logic_error("unreachable section");
}

/**
 Деструктор. Ну тут ничего необычного, простое освобождение памяти, выделенной в exp_node
 */
log_expr::exp_node::~exp_node() {
	delete first;
	delete second;
}

/**
Конструктор, строит дерево по переданной строке
*/
log_expr::log_expr(const std::string& str) {
	std::stringstream ss(str);
	root_ = new exp_node(nullptr, e_root);
	exp_node* cur = root_;
	size_t i;
	for (i = 0; ss; ++i) {
		char c = ss.get();
		if (!ss)
			break;

		switch (c) {
		case '&':
		case '+': {
			op_type t;
			if (c == '&')
				t = op_and;
			else
				t = op_or;
			if ((cur->state() == 0) || (cur->state() == 2))
				throw std::runtime_error("Variable missing at position " + std::to_string(i));
			if (cur->state() == 1)
				cur->operation = t;
			else if (cur->state() == 3) {
				if (t == op_or) {
					exp_node* temp = new exp_node(*cur);
					cur->first = temp;
					temp->prev = cur;
					cur->operation = t;
					cur->second = nullptr;
				}
				else {
					exp_node* temp = new exp_node(cur, e_leaf, cur->second->data);
					cur = cur->second;
					cur->first = temp;
					cur->operation = t;
					cur->type = e_node;
				}
			}
			break;
		}
		case '!': {
			if ((cur->state() == 1) || (cur->state() == 3))
				throw std::runtime_error("Operator missing at position " + std::to_string(i));
			if (cur->operation == op_not)
				throw std::runtime_error("Double negage at position " + std::to_string(i));
			exp_node* temp = new exp_node(cur, e_node);
			temp->operation = op_not;
			if (cur->state() == 0) {
				cur->first = temp;
				cur = cur->first;
			}
			else if (cur->state() == 2) {
				cur->second = temp;
				cur = cur->second;
			}
			break;
		}
		case '(': {
			if ((cur->state() == 1) || (cur->state() == 3))
				throw std::runtime_error("Operator missing at position " + std::to_string(i));
			exp_node* temp = new exp_node(cur, e_subnode);
			if (cur->state() == 0) {
				cur->first = temp;
				cur = cur->first;
			}
			else if (cur->state() == 2) {
				cur->second = temp;
				cur = cur->second;
			}
			break;
		}
		case ')': {
			while ((cur->type != e_subnode) && (cur->type != e_root))
				cur = cur->prev;
			if (cur->type == e_root)
				throw std::runtime_error("Left bracket not found at position " + std::to_string(i));
			cur = cur->prev;
			if (cur->operation == op_not)
				cur = cur->prev;
			break;
		}
		case ' ':
		case '\t':
			break;
		default: {
			std::string id(1, c);
			while (ss) {
				c = ss.get();
				if (!ss)
					break;
				if ((c < '0') || (c > '9')) {
					ss.unget();
					break;
				}
				id += c;
				++i;
			}
			if (id.length() == 1)
				throw std::runtime_error("Index of variable missing at position " + std::to_string(i));
			int idind = -1;
			for (size_t x = 0; x < ids_.size(); ++x)
				if (ids_[x] == id) {
					idind = x;
					break;
				}
			if (idind == -1) {
				idind = ids_.size();
				ids_.push_back(id);
			}
			if ((cur->state() == 3) || (cur->state() == 1))
				throw std::runtime_error("Operator missing at position " + std::to_string(i));
			exp_node* temp = new exp_node(cur, e_leaf, idind);
			if (cur->state() == 2)
				cur->second = temp;
			else if (cur->state() == 0)
				cur->first = temp;
			if (cur->operation == op_not)
				cur = cur->prev;
		}
		}
	}
	while ((cur->type != e_subnode) && (cur->type != e_root))
		cur = cur->prev;
	if (cur->type == e_subnode)
		throw std::runtime_error("Right bracket not found at position " + std::to_string(i));
}

/**
Вывод полученного из логического выражения в поток os
 */
auto log_expr::print(std::ostream& os) const -> void{
	std::vector<bool> vals(ids_.size(), false);
	while (true) {
		os << root_->process(vals);
		size_t i;
		for (i = 0; (i < vals.size()) && vals[i]; ++i)
			vals[i] = false;
		if (i == vals.size())
			break;
		vals[i] = true;
	}
}

/**
Деструктор. Здесь тоже ничего необычного, простое освобождение памяти, выделенной в под root_
*/
log_expr::~log_expr() {
	delete root_;
}