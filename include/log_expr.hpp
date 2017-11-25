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
\brief	Парсер логических формул.

\detail Данный класс позволяет получить вектор функции, заданной формулой. Нужен
для дальнейшей работы класса Quine_McCluskey_Simlifier

\data	Октябрь-ноябрь 2017 года.
*/
class log_expr {
	/**
	Для типов узлов
	*/
	enum exp_type {
		/**
		Корень дерева
		*/
		e_root,
		/**
		Обычный узел
		*/
		e_node,
		/**
		Узел, представляющий собой выржание в скобке
		*/
		e_subnode,
		/**
		Лист, содержит не фиктивное поле data
		*/
		e_leaf,
	};

	/**
	Представляет собой операцию между двумя операндами
	*/
	enum op_type {
		/**
		Фикитивная операция. Например, если все выжение - это отрицание одной переменной, т.е. !a1
		*/
		op_null,
		/**
		Операция И
		*/
		op_and,
		/**
		Операция ИЛИ
		*/
		op_or,
		/**
		Операция НЕ. Создается отдельный узел, где first = nullptr, а second - аргумент операции НЕ
		*/
		op_not
	};

	/**
	\brief Узел дерева

	\detail Структура, представляющая собой узлы (вершины) дерева
	*/
	struct exp_node {
		/**
		Родительский узел
		*/
		exp_node* prev;
		/**
		Тип узла, представлен структтурой exp_type
		*/
		exp_type type;
		/**
		Только для листьев дерева - хранит индекс идентификатора
		*/
		size_t data;
		/**
		Первый операнд
		*/
		exp_node* first;
		/**
		Сама операция между двумя операндами
		*/
		op_type operation;
		/**
		Второй операнд
		*/
		exp_node* second;

		exp_node(exp_node* prev, exp_type type, size_t data = 0);
		exp_node(const exp_node& o);
		~exp_node();

		auto process(std::vector<bool>&) const -> bool;
		auto state() const->size_t;
	};

	std::vector<std::string> ids_;
	exp_node* root_;
public:
	log_expr(const std::string&);
	auto print(std::ostream& os = std::cout) const -> void;

	~log_expr();
};