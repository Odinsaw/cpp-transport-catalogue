#pragma once
#include "json.h"

namespace json {

	class Builder;

	class Context;

	//-------возвращаемые классы--------------
	class KeyContext;

	class DictItemContext;

	class ArrayItemContext;

	class ValueInDictContext;

	class ValueInArrayContext;
	//-----------------------------------------

	class Builder {

	public:

		KeyContext Key(std::string key);

		Builder& Value(Node value); //конструктор базового класса Value(variant<string, double,int...>) неявно приведет string, double, int... к Node

		DictItemContext StartDict();

		Builder& EndDict();

		ArrayItemContext StartArray();

		Builder& EndArray();

		Node Build();

	private:
		Node root_;
		std::vector<Node*> nodes_stack_;
		bool previous_is_key_ = false;
		std::string key_ = "";
		bool root_initialized_ = false;
	};

	//Базовый класс хранящий ссылку на билдер
	class Context {
	public:
		Context(Builder& b)
			:builder(b)
		{
		}

		Builder& builder;

		Builder& GetBuilder() {
			return builder;
		}
	};

	//------Базовые классы реализующие методы-----------------
	class DictAndArrayMethods : private Context {
	public:
		DictAndArrayMethods(Builder& b)
			:Context(b)
		{
		}

		DictItemContext StartDict();

		ArrayItemContext StartArray();
	};

	class KeyAndEndDictMethods : private Context {
	public:
		KeyAndEndDictMethods(Builder& b)
			:Context(b)
		{
		}

		KeyContext Key(std::string key);

		Builder& EndDict();
	};

	class ValueInArrayAndEndArrayMethods : private Context {
	public:
		ValueInArrayAndEndArrayMethods(Builder& b)
			:Context(b)
		{
		}

		ValueInArrayContext Value(Node value);

		Builder& EndArray();
	};

	class ValueInDictMethods : private Context {
	public:
		ValueInDictMethods(Builder& b)
			:Context(b)
		{
		}

		ValueInDictContext Value(Node value);
	};
	//----------------------------------------------------------

	class KeyContext : public DictAndArrayMethods, public ValueInDictMethods {
	public:
		KeyContext(Builder& b)
			:DictAndArrayMethods(b), ValueInDictMethods(b)
		{
		}
	};

	class ValueInArrayContext : public DictAndArrayMethods, public ValueInArrayAndEndArrayMethods {
	public:
		ValueInArrayContext(Builder& b)
			:DictAndArrayMethods(b), ValueInArrayAndEndArrayMethods(b)
		{
		}
	};

	class ArrayItemContext : public DictAndArrayMethods, public ValueInArrayAndEndArrayMethods {
	public:
		ArrayItemContext(Builder& b)
			:DictAndArrayMethods(b), ValueInArrayAndEndArrayMethods(b)
		{
		}
	};

	class DictItemContext : public KeyAndEndDictMethods {
	public:
		DictItemContext(Builder& b)
			:KeyAndEndDictMethods(b)
		{
		}
	};

	class ValueInDictContext : public KeyAndEndDictMethods {
	public:
		ValueInDictContext(Builder& b)
			:KeyAndEndDictMethods(b)
		{
		}
	};
}

