#include "log_expr.hpp"
//////////////////////////////////////////////
//                                          //
//                 log_expr                 //
//                                          //
//////////////////////////////////////////////

/**
Параметрический конструктор. Инициализирует все поля структуры \n
Сложность \f$O(1)\f$
\param[in] prev		Предыдущий узел
\param[in] type		Тип узла
\param[in] data		Данные узла
*/
log_expr::exp_node::exp_node(exp_node* prev, exp_type type, size_t data)
	: prev(prev),
	type(type),
	data(data),
	first(nullptr),
	operation(op_null),
	second(nullptr) {}

/**
Конструктор копирования, что тут еще сказать? \n
Сложность \f$O(1)\f$
\param[in] o		Узел для копирования
*/
log_expr::exp_node::exp_node(const exp_node& o)
	: prev(o.prev),
	type(o.type),
	data(o.data),
	first(o.first),
	operation(o.operation),
	second(o.second) {}

/**
Нужен для определения стадии парсинга. В соответствии с стадией выкидываются исключения, означающие ошибки в процессе парсинга \n
Сложность \f$O(1)\f$
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
Принимает булев вектор того же размера, что и вектор идентификаторов и рекурсивно получает значение выражения (формулы) \n
Сложность \f$O(V + E)\f$, где \f$V\f$ - количество вершин, 
\f$E\f$ - количество ребер
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
Деструктор. Ну тут ничего необычного, простое освобождение памяти, выделенной в exp_node \n
Сложность \f$O(1)\f$
*/
log_expr::exp_node::~exp_node() {
	delete first;
	delete second;
}

/**
Конструктор, строит дерево по переданной строке \n
Сложность \f$O(n)\f$, где \f$n\f$ - длина входной строки
\param[in]	str	Строка для разбора
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
Вывод полученного из логического выражения в поток os \n
Сложность \f$O(1)\f$
\param[in]	os	Поток вывода
*/
auto log_expr::print(std::ostream& os) const -> void {
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
Деструктор. Здесь тоже ничего необычного, простое освобождение памяти, выделенной в под root_ \n
Сложность \f$O(1)\f$
*/
log_expr::~log_expr() {
	delete root_;
}