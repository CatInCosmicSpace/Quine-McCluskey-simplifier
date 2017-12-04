#include "Quine_McCluskey_Simplifier.hpp"


/*! \mainpage Домашнее задание по АиСД
*
* \section intro_sec Короткие сведения
*
* Работа выполнена в рамках домашнего задания по дисциплине
* "Алгоритмы и структуры данных",
* задание: "Напишите программу для получения минимальной ДНФ булевой
* функции".
*
* \section complexity Сложности
* \subsection time Сложность по времени
* \f$O(log(k) \cdot (k \cdot n^3) + k \cdot n \cdot m^2 + k \cdot n^3 + k^2)\f$, где \f$k\f$ - количество единиц функции, \f$n\f$ - количество переменных, \f$m\f$ - количество импликант 
* \subsection memory Сложность по памяти
* \f$ O(n \cdot m) \f$, где \f$ n \f$ - количество единиц функции, \f$ m \f$ - количество импликант функции
*
* \section install_sec Установка
*
* \subsection step0 Шаг 0: подготовка
Убедитесь в наличии компилятора C++, установите себе cmake для упрощения сборки
* \subsection step1 Шаг 1: cmake
Соберите все с помощью команды cmake .  в консоли. По умолчанию он сам выберет компилятор, но
вы можете поставить и свой, добавив к команде флаг -G. Список доступных на вышей платформе компиляторов будет выведен при
выполнении команды cmake -h
* \subsection step2 Шаг 2: g++?
Если у вас есть компилятор g++, а также утилита make или mingw32-make, то в качестве параметров установите "-G MinGW Makefile" (мной проверялось
на Ubuntu и Windows 10)
* \subsection step3 Шаг 3: Осталось только make
Командой make или mingw32-make соберите проект
* \subsection step4 Пояснение: по умолчанию проект настроен на тесты.
Т.е. при сборке получится исполняемый файл QMS_test. Чтобы получился исполняемый проект, надо удалить /tests/init.cpp и /test/main.cpp и поместить
Source.cpp в папку /tests. Source.cpp находится в папке /tools

* \subsection else Я никогда не просил об этом
Весь нужный код находится в файлах /include/Quine_McCluskey_Simplifier.hpp,
/include/Quine_McCluskey_Simplifier.cpp и /tools/Source.cpp, которые соотвенно содержат объявление класса,
определение класса и исполняемый код с функцией main(). Собрать это все вы можете и сами, если захотите.
*
*/

//////////////////////////////////////////////
//                                          //
//        Quine_McCluskey_simplifier        //
//                                          //
//////////////////////////////////////////////

/**
Конструктор объекта класса. Создает объект по входному файлу (его имени), заполняя
поле input_sets и выравнивая наборы по количеству перменных \n
Сложность \f$O(2^m + n)\f$, где \f$m\f$ - длина регулярного выражения, 
\f$n\f$ - длина входной строки (из файла)

\param[in]		file_name	Имя открываемого файла
\throw			logic_error	Исключение, если файл не был открыт
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
степенью двойки \n
Сложность \f$O(n + k)\f$, где \f$n\f$ - длина входной строки (из потока), 
\f$k\f$ - количество значимых единиц функции
\param[in]		ss	Входной поток
\throw	logic_error	Исключение, если размер вектора не является степенью двойки
\throw	logic_error	Исключение, если встретились символы, отличные от "0" и "1"
*/
Quine_McCluskey_Simplifier::Quine_McCluskey_Simplifier(std::istream & ss) {
	std::string temp;
	while (ss.good()) {
		getline(ss, temp); 
		if (!((temp.size() == 0) ? 0 : (temp.size() & (temp.size() - 1)) == 0))
			throw std::logic_error("Size of vector is invalid. Check your input.");

		for (size_t i = 0; i < temp.length(); ++i) {
			if (!(temp[i] == '1' || temp[i] == '0'))
				throw std::logic_error("Incorrect input.");
			if (temp[i] == '1')
				if (!in_vect(input_sets_, string_base10_to_base2(std::to_string(i))))
					input_sets_.push_back(string_base10_to_base2(std::to_string(i)));
		}
	}
	
	const size_t m_l = num_of_vars();
	groups_.resize(m_l + 1);
	for (auto i = 0; i < input_sets_.size(); ++i) {
		align(i, m_l);
	}
}

/**
Выравнивает набор input_sets[i] по количеству переменных функции \n
Сложность \f$O(n)\f$, где \f$n\f$ - длина строки, которой представлен набор
\param[in]		index		Номер набора для выравнивания
\param[in]		size		Количество переменных
*/
auto Quine_McCluskey_Simplifier::align(const size_t index, const size_t size) -> void {
	const auto curr_size = input_sets_[index].size();
	// O(size - curr_size + n) + O(n - curr_size)
	input_sets_[index].insert(0, std::string(size - curr_size, '0'));
	//for (auto j = 0; j < size - curr_size; ++j)
	//	input_sets_[index].insert(input_sets_[index].begin(), '0');
}

/**
Создает разбиение входных наборов (из is) на группы по их весам. Данные записывает в поле groups \n
Сложность \f$O(k + n)\f$, где \f$k\f$ - количество единиц функции, \f$n\f$ - количество переменных функции
\param[in]		is			Наборы для разбиения
*/
auto Quine_McCluskey_Simplifier::create_groups(decltype(input_sets_)& is) -> void {
	// количество единиц = k
	for (auto i : is) {
		// get_weight = O(n)
		groups_[get_weight(i)].push_back(make_pair(i, false));
	}
}

/**
Создает таблицу покрытия единиц импликантами, которые были переданы функции \n
Сложность \f$O(k \cdot n^3)\f$, где \f$k\f$ - количество еще не покрытых единиц функции, 
\f$n\f$ - количество импликант, которые рассматриваются
\param[in]		ones			Единицы для покрытия
\param[in]		impls			Импликанты для покрытия
*/
auto Quine_McCluskey_Simplifier::create_table(const decltype(input_sets_)& ones, const decltype(input_sets_)& impls) -> void {
	// Cоздаем таблицу покрытия импликантами единиц функции.
	table_.resize(ones.size()); // O(k)
	for (auto i : table_)
		std::get<0>(i).resize(impls.size()); // O(k * n)
	for (auto i = 0; i < ones.size(); ++i) { // k *
		for (auto j = 0; j < impls.size(); ++j) { // n *
			if (is_cover(ones[i], impls[j])) { // n
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
обеспечивает максимальное покрытие таблицы \n
Сложность \f$O(n)\f$, где \f$n\f$ - размер вектора
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
Вычисляет ядро функции. Неочевидно, но возвращает те наборы, которые не покрыты ядром \n
Сложность \f$O(k \cdot n \cdot m^2)\f$, где \f$k\f$ - количество единиц функции,
\f$n\f$ - количество переменных функции, 
\f$m\f$ - количество импликант
\param[out]		not_covered_ones	Наборы, которые не покрыты ядром
*/
auto Quine_McCluskey_Simplifier::get_func_core() -> std::vector<std::string> {
	std::vector<std::string> to_choice;
	std::vector<std::string> not_covered_ones;
	for (auto i = 0; i < table_.size(); ++i) { // O(k) - количество единиц
		if (is_prime(std::get<0>(table_[i]))) { // O(n) - количество переменных
			for (auto j = 0; j < std::get<0>(table_[i]).size(); ++j) { // O(m) - колиество импликант
				if (std::get<0>(table_[i])[j] == 1) {
					if (!in_vect(prime_, implicants_[j])) { // O(m)
						prime_.push_back(implicants_[j]);
					}
					std::get<1>(table_[i]) = true;
					for (auto k = 0; k < input_sets_.size(); ++k) { // O(k)
						if (std::get<0>(table_[k])[j] == 1)
							std::get<1>(table_[k]) = true;
					}
					break;
				}
			}
		}
	}
	for (auto i = 0; i < table_.size(); ++i) { // O(k)
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
Проще говоря, нужна для склейки \n
Сложность (это не точно, это оценка) \f$O(log(k) \cdot (k \cdot n^3))\f$, где 
\f$k\f$ - количество значимых единиц функции, \f$n\f$ - количество переменных функции
*/
auto Quine_McCluskey_Simplifier::get_implicants() -> void {
	auto find_index = [this](auto x, auto y) { for (size_t i = 0; i < x.size(); ++i)
		if (x[i] != y[i])
			return i;
	}; // O(n)
	create_groups(input_sets_);
	std::vector<std::string> tmp;
	auto find = true;
	// Пока находятся скейки
	// Вероятно, O(log(k))
	while (find) {
		find = false;
		// Цикл по всей таблице
		for (auto i = 0; i < groups_.size() - 1; ++i) { // Количество значимых единиц
			// Цикл по строке таблице
			for (auto j = 0; j < groups_[i].size(); ++j) { // Количество переменных
				// Цикл сравнения со всеми соседями
				for (auto k = 0; k < groups_[i + 1].size(); ++k) { // Еще раз количество перменнных
					if (is_neighbors(std::get<0>(groups_[i][j]), std::get<0>(groups_[i + 1][k]))) {
						auto ind = find_index(std::get<0>(groups_[i][j]), std::get<0>(groups_[i + 1][k])); // Еще раз количество перменных
						std::get<1>(groups_[i][j]) = true;
						std::get<1>(groups_[i + 1][k]) = true;
						auto str = std::get<0>(groups_[i][j]);
						str[ind] = '-';
						tmp.push_back(str);
						find = true;
					}
				}
			}
		} // O(k \cdot n^3)
		for (auto i = 0; i < groups_.size(); ++i) {
			for (auto j = 0; j < groups_[i].size(); ++j) {
				if (std::get<1>(groups_[i][j]) == false) {
					if (!in_vect(implicants_, std::get<0>(groups_[i][j]))) {
						implicants_.push_back(std::get<0>(groups_[i][j]));
					}
				}
			}
		}
		groups_.clear(); // O(n)
		groups_.resize(num_of_vars() + 1); // O(n + 1)
		create_groups(tmp); // O(n + k)
		tmp.clear(); // O(n)
	}
}

/**
Возвращает вес набора - количество единиц в нем \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество переменных функции
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
Возвращает строку, которая содержит представление полученной МДНФ в формульном виде \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество переменных функции
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
Проверяет наличие элемента в векторе \n
Сложность \f$O(n)\f$, где \f$n\f$ - длина вектора
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
Аналогичная функция, но проверяет для вектора пар (std::vector<std::pair<>>) \n
Сложность \f$O(n)\f$, где \f$n\f$ - длина вектора
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
Проверяет, покрывает ли второй набор первый \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество переменных функции
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
Соседние ли наборы? Нужна для попарного сравнения в группах \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество переменных функции
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
может покрыть только этот импликант и следует внести его в ядро \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество переменных функции
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
Возвращает количество переменных рассматриваемой функции \n
Сложность \f$O(k)\f$, где \f$k\f$ - количество значимых единиц функции
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
Выводит полученную МДНФ в стандарный поток вывода - std::cout \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество полученных импликантов в МДНФ функции
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
Выводит полученную МДНФ в файл file_name\n
Сложность \f$O(n)\f$, где \f$n\f$ - количество полученных импликантов в МДНФ функции
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
По своей сути аналогичек конструктору по потоку\n
Сложность \f$O(2^m + n)\f$, где \f$m\f$ - длина регулярного выражения, 
\f$n\f$ - длина входной строки
\param[in] file_name	Имя выходного файла
\param[in] sets			По номерам наборов или нет?
\throw	logic_error	Исключение, если размер вектора не является степенью двойки
*/
auto Quine_McCluskey_Simplifier::init(std::istream& is, bool sets) -> void {
	if (sets == false) {
		*this = Quine_McCluskey_Simplifier(is);
		return;
	}
	std::string temp; // O(1)
	std::smatch m; // O(1)
	std::regex e("[[:digit:]]+"); // O(1)?
	while (is.good()) {
		getline(is, temp);
		while (std::regex_search(temp, m, e)) { // O(n)?
			for (auto x : m)
				if (!in_vect(input_sets_, string_base10_to_base2(x)))
					input_sets_.push_back(string_base10_to_base2(x)); // O(1)
			temp = m.suffix().str();
		}
	}
	
	const size_t m_l = num_of_vars();
	groups_.resize(m_l + 1);
	for (auto i = 0; i < input_sets_.size(); ++i) {
		align(i, m_l);
	}
}

/**
Главная функция доступа извне - создает внутри класса МДНФ \n
Сложность \f$O(log(k) \cdot (k \cdot n^3) + k \cdot n \cdot m^2 + k \cdot n^3 + k^2)\f$, где 
\f$k\f$ - количество единиц функции, \f$n\f$ - количество переменных, \f$m\f$ - количество импликант
*/
auto Quine_McCluskey_Simplifier::simplify() -> void {
	get_implicants(); // O(log(k) * (k * n^3))
	create_table(input_sets_, implicants_); // O(k * n^3)
	auto not_covered_ones = get_func_core(); // O(k * n * m^2)
	table_.clear(); // O(k * m)
	std::vector<std::string> not_prime_implicants;
	std::vector<size_t> every_impl_covers;
	for (const auto i : implicants_) { // O(m)
		if (!in_vect(prime_, i)) {
			not_prime_implicants.push_back(i);
			every_impl_covers.push_back(0);
		}
	}

	// Новая таблица - таблица непокрытых единиц и всех импликант, 
	// не вошедших в ядро.
	create_table(not_covered_ones, not_prime_implicants); // O(k * n^3)

	std::vector<std::vector<bool>> covering;
	std::vector<std::string> simple_cover;

	//is_cover(not_prime_implicants[j], not_covered_ones[j])
	std::vector<std::string> final_cover;
	while (not_covered_ones.size() != 0) {
		// Проходим по наборам, на которых функция равна 1
		for (auto i = 0; i < table_.size(); ++i) { // O(k)
			// Теперь - по импликантам
			for (auto j = 0; j < std::get<0>(table_[i]).size(); ++j) { // O(m)
				if (std::get<0>(table_[i])[j] == 1) {
					++every_impl_covers[j];
				}
			}
		}

		const auto ind = find_max_cover_ind(every_impl_covers); // O(m)
		if (ind < not_prime_implicants.size()) {
			final_cover.push_back(not_prime_implicants[ind]);

			for (auto i = 0; i < table_.size(); ++i) { // O(k)
				for (auto j = 0; j < table_.size(); ++j) { // O(k)
					if (std::get<0>(table_[i])[ind] == 1) {
						std::get<1>(table_[i]) = true;
					}
				}
			}
			every_impl_covers.clear(); // O(m)
			decltype(not_prime_implicants) tmp;
			for (const auto i : not_prime_implicants) { // O(m)
				if (!in_vect(final_cover, i)) {
					tmp.push_back(i);
					every_impl_covers.push_back(0);
				}
			}
			not_prime_implicants = tmp;
			tmp.clear();
			for (auto i = 0; i < not_covered_ones.size(); ++i) { // O(k)
				if (std::get<1>(table_[i]) == false)
					tmp.push_back(not_covered_ones[i]);
			}
			not_covered_ones = tmp;
			tmp.clear();
			table_.clear();

			create_table(not_covered_ones, not_prime_implicants);
		}
		else
			throw std::logic_error("What?");
	}

	for (const auto i : prime_) {
		mdnf_.insert(i);
	}
	for (const auto i : final_cover) {
		mdnf_.insert(i);
	}
}

/**
Печатает в поток полученную МДНФ в формульном виде \n
Сложность \f$O(n)\f$, где \f$n\f$ - количество импликантов МДНФ
\param[in]		os			Поток для печати
*/
auto Quine_McCluskey_Simplifier::print_formula(std::ostream& os) const -> void {
	if (mdnf_.size() == 0)
		throw std::logic_error("Function not simplified!");
	for (const auto i : mdnf_) // O(n)
		os << impl_to_formula(i) << " ";
}

/**
Конвертирует входную строку, представляющую десятичное число, в выходную строку,
представлляющую двоичное число\n
Сложность \f$O(n^2)\f$, где \f$n\f$ - длина строки inp
\param[in]		inp		Десятичная строка
\param[out]		res		Двоичная строка
*/
auto Quine_McCluskey_Simplifier::string_base10_to_base2(std::string inp) const -> std::string {
	std::string res; // O(1)
	std::string right; // O(1)
	do {
		right.clear(); // O(n), n - длина right
		int left = 0;
		for (size_t i = 0; i < inp.length(); ++i) {
			left = left * 10 + (inp[i] - '0');
			const auto div = left / 2;
			if ((i != 0) || (div != 0)) {
				right.push_back(div + '0'); // O(1)
				left %= 2;
			}
		}
		res.push_back(left + '0'); // O(1)
		inp = right;
	} while (inp != std::string()); // O(n), n - длина inp

	for (size_t i = 0; i < res.length() / 2; ++i) // O(n/2)
		std::swap(res[i], res[res.length() - 1 - i]); // O(1)
	return res;
}
