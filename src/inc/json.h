#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <iomanip>

namespace Json {

#define ARRAY_NODE 0
#define MAP_NODE 1
#define NUMBER_NODE 2
#define STRING_NODE 3
#define BOOL_NODE 4

class Node : public std::variant<std::vector<Node>,
                                 std::map<std::string, Node>,
                                 double,
                                 std::string,
                                 bool> {
public:
  using variant::variant;

  const auto& AsArray() const {
    return std::get<std::vector<Node>>(*this);
  }
  const auto& AsMap() const {
    return std::get<std::map<std::string, Node>>(*this);
  }
  double AsDouble() const {
    return std::get<double>(*this);
  }
  const auto& AsString() const {
    return std::get<std::string>(*this);
  }
  const auto& AsBool() const {
    return std::get<bool>(*this);
  }
};

class Document {
public:
  explicit Document(Node root);

  const Node& GetRoot() const;

private:
  Node root;
};

Document Load(std::istream& input);
void Print(std::ostream& out, const Node& node);

}
