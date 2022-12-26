#include "json_builder.h"
#include <utility>

using namespace std;

namespace json {

	//-------------методы ядра билдера-----------------------------------------------------------
	KeyContext Builder::Key(string k) {

		if (nodes_stack_.empty()) { 
			throw logic_error("Incorrect Key() call!");
		}

		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Incorrect Key() call!");
		}

		//key_ = move(key);
		Node* key = new Node{ move(k) }; 
		nodes_stack_.push_back(key); //храним ключ в nodes_stack_
		return { *this };
	}

	Builder& Builder::Value(Node value) {

		if (!nodes_stack_.empty() && nodes_stack_.back()->IsString()) { //в данный момент строится узел Dict, последний элемент - ключ

			string key = nodes_stack_.back()->AsString(); //достаем ключ
			delete nodes_stack_.back(); //устраняем утечки
			nodes_stack_.pop_back();

			if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) { //за ключом должен лежать соответствующий словарь
				Dict& cur_dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
				cur_dict.insert({ key, move(value) });
			}
			else {
				throw logic_error("Logic error!");
			}
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) { //в данный момент строится узел Array
			Array& cur_array = const_cast<Array&>(nodes_stack_.back()->AsArray());
			cur_array.emplace_back(move(value));
		}
		else if (nodes_stack_.empty()) { //value задается сразу после конструктора  
			root_ = move(value);
			nodes_stack_.push_back(&root_);
		}
		return *this;
	}

	void Builder::AddItem(Node node) {

		if (nodes_stack_.empty()) { //вызов сразу после конструктора
			root_ = node;
			nodes_stack_.push_back(&root_);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) { //это новый элемент array
			Array& cur_array = const_cast<Array&>(nodes_stack_.back()->AsArray());
			cur_array.emplace_back(node);
			nodes_stack_.push_back(&cur_array.back());
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsString()) { //это value, а до этого был задан key

			string key = nodes_stack_.back()->AsString(); //достаем ключ
			delete nodes_stack_.back(); //устраняем утечки
			nodes_stack_.pop_back();

			if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) { //за ключом должен лежать соответствующий словарь
				Dict& cur_dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
				cur_dict.insert({ key, node });
				nodes_stack_.push_back(&cur_dict.at(key));
			}
			else {
				throw logic_error("Logic error!");
			}
		}
		else { //все остальные вызовы start недопустимы
			throw logic_error("Incorrect Start<item>() call!");
		}
	}

	ArrayItemContext Builder::StartArray() {

		AddItem(Array{});
		return { *this };
	}

	DictItemContext Builder::StartDict() {

		AddItem(Dict{});
		return { *this };
	}

	Builder& Builder::EndDict() {

		if (nodes_stack_.empty()) { //вызов EndDict() первым
			throw logic_error("Incorrect EndDict() call!");
		}

		if (!nodes_stack_.back()->IsDict()) {
			throw std::logic_error("Incorrect EndDict() call!");
		}

		if (nodes_stack_.size() != 1){ //последний(первый в векторе) узел Node закрываем через Build
		nodes_stack_.pop_back();
		}
		return *this;
	}

	Builder& Builder::EndArray() {

		if (nodes_stack_.empty()) { //закончены построения всех узлов
			throw logic_error("Incorrect EndArray() call!");
		}

		if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("Incorrect EndArray() call!");
		}

		if (nodes_stack_.size() != 1){ //последний(первый в векторе) узел Node закрываем через Build
		nodes_stack_.pop_back();
		}
		return *this;
	}

	Node Builder::Build() {

		if (nodes_stack_.empty() || (nodes_stack_.size() != 1)) { //метод вызван сразу после конструктора или есть незаконченные узлы (root всегда первый узел)
			throw logic_error("Build should be used when a document is ready!");
		}

		return root_;
	}
	//-----------------------------------------------------------------

	
	DictItemContext Context::StartDict() {
		builder.StartDict();
		return { builder };
	}

	ArrayItemContext Context::StartArray() {
		builder.StartArray();
		return { builder };
	}

	KeyContext Context::Key(std::string key) {
		builder.Key(move(key));
		return { builder };
	}

	Builder& Context::EndDict() {
		builder.EndDict();
		return builder;
	}

	ValueInArrayContext Context::ValueInArray(Node value) {
		builder.Value(move(value));
		return { builder };
	}

	Builder& Context::EndArray() {
		builder.EndArray();
		return builder;
	}

	ValueInDictContext Context::ValueInDict(Node value) {
		builder.Value(move(value));
		return { builder };
	}

	ValueInDictContext KeyContext::Value(Node value) {
		return ValueInDict(value);
	}

	ValueInArrayContext ValueInArrayContext::Value(Node value) {
		return ValueInArray(value);
	}

	ValueInArrayContext ArrayItemContext::Value(Node value) {
		return ValueInArray(value);
	}
}
