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

		void AddItem(Node node);

		Node root_;
		std::vector<Node*> nodes_stack_;
	};

	//Базовый класс хранящий ссылку на билдер
	class Context {

	public:
		Context(Builder& b)
			:builder(b)
		{
		}

		Builder& builder;

		DictItemContext StartDict();

		ArrayItemContext StartArray();

		KeyContext Key(std::string key);

		Builder& EndDict();

		ValueInArrayContext ValueInArray(Node value);

		Builder& EndArray();

		ValueInDictContext ValueInDict(Node value);
	};

	class KeyContext : private Context {
	public:
		KeyContext(Builder& b)
			:Context(b)
		{
		}

		using Context::StartDict;
		using Context::StartArray;
		ValueInDictContext Value(Node value);
	};

	class ValueInArrayContext : private Context {
	public:
		ValueInArrayContext(Builder& b)
			:Context(b)
		{
		}

		using Context::StartDict;
		using Context::StartArray;
		ValueInArrayContext Value(Node value);
		using Context::EndArray;
	};

	class ArrayItemContext : private Context {
	public:
		ArrayItemContext(Builder& b)
			:Context(b)
		{
		}

		using Context::StartDict;
		using Context::StartArray;
		ValueInArrayContext Value(Node value);
		using Context::EndArray;
	};

	class DictItemContext : private Context {
	public:
		DictItemContext(Builder& b)
			:Context(b)
		{
		}

		using Context::Key;
		using Context::EndDict;
	};

	class ValueInDictContext : private Context {
	public:
		ValueInDictContext(Builder& b)
			:Context(b)
		{
		}

		using Context::Key;
		using Context::EndDict;
	};
}

