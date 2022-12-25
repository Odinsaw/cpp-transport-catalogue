#include "json_builder.h"
#include <utility>

using namespace std;

namespace json {

	//-------------методы ядра билдера-----------------------------------------------------------
	KeyContext Builder::Key(string key) {

		if (nodes_stack_.empty() && root_initialized_) { //закончены построения всех узлов
			throw logic_error("Trying to add dict key to finished document!");
		}

		if (!nodes_stack_.back()->IsDict() || previous_is_key_) {
			throw std::logic_error("Wrong Key() call!");
		}
		previous_is_key_ = true;
		key_ = move(key);
		return { *this };
	}

	Builder& Builder::Value(Node value) {

		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) { //в данный момент строится узел Dict
			if (previous_is_key_) {
				Dict& cur_dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
				cur_dict.insert({ key_, move(value) });
				previous_is_key_ = false;
			}
			else {
				throw logic_error("Value must follow a key!");
			}
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) { //в данный момент строится узел Array

			Array& cur_array = const_cast<Array&>(nodes_stack_.back()->AsArray());
			cur_array.emplace_back(move(value));
		}
		else if (nodes_stack_.empty() && !root_initialized_) { //value задается сразу после конструктора  

			root_initialized_ = true;
			root_ = move(value);
		}
		else if (nodes_stack_.empty() && root_initialized_) { //закончены построения всех узлов

			throw logic_error("Trying to add value to finished document!");
		}
		return *this;
	}

	DictItemContext Builder::StartDict() {

		if (nodes_stack_.empty() && root_initialized_) { //закончены построения всех узлов
			throw logic_error("Trying to add dict to finished document!");
		}
		else if (nodes_stack_.empty() && !root_initialized_) { //вызов startdict сразу после конструктора
			root_initialized_ = true;
			root_ = Dict{};
			nodes_stack_.push_back(&root_);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) { //dict это новый элемент array
			Array& cur_array = const_cast<Array&>(nodes_stack_.back()->AsArray());
			cur_array.emplace_back(Node(Dict{}));
			nodes_stack_.push_back(&cur_array.back());
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && previous_is_key_) { //dict это value, а до этого был задан key
			Dict& cur_dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
			cur_dict.insert({ key_, Dict{} });
			previous_is_key_ = false;
			nodes_stack_.push_back(&cur_dict.at(key_));
		}
		else { //все остальные вызовы start dict недопустимы
			throw logic_error("Wrong StartDict() call!");
		}
		return { *this };
	}

	Builder& Builder::EndDict() {

		if (nodes_stack_.empty() && root_initialized_) { //закончены построения всех узлов
			throw logic_error("Trying to add dict to finished document!");
		}

		if (!nodes_stack_.back()->IsDict() || previous_is_key_) {
			throw std::logic_error("Incorrect EndDict() call!");
		}

		nodes_stack_.pop_back();
		return *this;
	}

	ArrayItemContext Builder::StartArray() {

		if (nodes_stack_.empty() && root_initialized_) { //закончены построения всех узлов
			throw logic_error("Trying to add array to finished document!");
		}
		else if (nodes_stack_.empty() && !root_initialized_) { //вызов startarray сразу после конструктора
			root_initialized_ = true;
			root_ = Array{};
			nodes_stack_.push_back(&root_);
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) { //array это новый элемент array
			Array& cur_array = const_cast<Array&>(nodes_stack_.back()->AsArray());
			cur_array.emplace_back(Node(Array{}));
			nodes_stack_.push_back(&cur_array.back());
		}
		else if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && previous_is_key_) { //array это value, а до этого был задан key
			Dict& cur_dict = const_cast<Dict&>(nodes_stack_.back()->AsDict());
			cur_dict.insert({ key_, Array{} });
			previous_is_key_ = false;
			nodes_stack_.push_back(&cur_dict.at(key_));
		}
		else { //все остальные вызовы start array недопустимы
			throw logic_error("Wrong StartArray() call!");
		}
		return { *this };

	}

	Builder& Builder::EndArray() {

		if (nodes_stack_.empty() && root_initialized_) { //закончены построения всех узлов
			throw logic_error("Trying to add array to finished document!");
		}

		if (!nodes_stack_.back()->IsArray()) {
			throw std::logic_error("Incorrect array synthax!");
		}

		nodes_stack_.pop_back();
		return *this;
	}

	Node Builder::Build() {

		if (!nodes_stack_.empty() || !root_initialized_) { //есть незаконченные узлы или метод вызван сразу после конструктора
			throw logic_error("Build should be used when a document is ready!");
		}

		return root_;
	}
	//-----------------------------------------------------------------

	
	DictItemContext DictAndArrayMethods::StartDict() {
		builder.StartDict();
		return { builder };
	}

	ArrayItemContext DictAndArrayMethods::StartArray() {
		builder.StartArray();
		return { builder };
	}

	KeyContext KeyAndEndDictMethods::Key(std::string key) {
		builder.Key(move(key));
		return { builder };
	}

	Builder& KeyAndEndDictMethods::EndDict() {
		builder.EndDict();
		return builder;
	}

	ValueInArrayContext ValueInArrayAndEndArrayMethods::Value(Node value) {
		builder.Value(move(value));
		return { builder };
	}

	Builder& ValueInArrayAndEndArrayMethods::EndArray() {
		builder.EndArray();
		return builder;
	}

	ValueInDictContext ValueInDictMethods::Value(Node value) {
		builder.Value(move(value));
		return { builder };
	}
}
