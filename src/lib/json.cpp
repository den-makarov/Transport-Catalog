#include "json.h"

using namespace std;

namespace Json {

  Document::Document(Node root) : root(move(root)) {
  }

  const Node& Document::GetRoot() const {
    return root;
  }

  Node LoadNode(istream& input);

  ostream& operator<<(ostream& out, const vector<Node>& array) {
    out << "[\n  ";
    for(const auto& item : array) {
      Print(out, item);
      out << ",\n";
    }
    out << "  ]";
    return out;
  }
  
  ostream& operator<<(ostream& out, const map<string, Node>& map) {
    out << "{\n  ";
    for(const auto& [key, value] : map) {
      out << "\"" << key << "\": ";
      Print(out, value);
      out << ",\n";
    }
    out << "  }";
    return out;
    return out;
  }

  Node LoadArray(istream& input) {
    vector<Node> result;

    for (char c; input >> c && c != ']'; ) {
      if (c != ',') {
        input.putback(c);
      }
      result.push_back(LoadNode(input));
    }

    return Node(move(result));
  }

  Node LoadDouble(istream& input) {
    double result = 0;
    while (isdigit(input.peek())) {
      result *= 10;
      result += input.get() - '0';
    }
    
    if(char c = input.get(); c == '.') {
      double frac = 0;
      int count = 0;
      while (isdigit(input.peek())) {
        frac *= 10;
        frac += static_cast<double>(input.get() - '0');
        count++;
      }

      while(count != 0) {
        frac /= 10;
        count--;
      }
      
      result += frac;
    } else {
      input.putback(c);
    }
    return Node(result);
  }

  Node LoadNegativeDouble(istream& input) {
    char c;
    while(input.get(c) && isspace(c));
    input.putback(c);
    return Node( -1.0 * LoadDouble(input).AsDouble());
  }

  Node LoadPositiveDouble(istream& input) {
    char c;
    while(input.get(c) && isspace(c));
    input.putback(c);
    return Node(LoadDouble(input).AsDouble());
  }

  Node LoadString(istream& input) {
    string line;
    getline(input, line, '"');
    return Node(move(line));
  }

  Node LoadDict(istream& input) {
    map<string, Node> result;

    for (char c; input >> c && c != '}'; ) {
      input.putback(c);
      while(input.get(c) && isspace(c));
      if (c == ',') {
        while(input.get(c) && isspace(c));
      }
      string key = LoadString(input).AsString();
      input >> c;
      result.emplace(move(key), LoadNode(input));
    }
    return Node(move(result));
  }

  Node LoadBool(istream& input, char c) {
    bool state = c == 'f' ? false: true;
    if(state) {
      input.ignore(3);
    } else {
      input.ignore(4);
    }
    return Node(state);
  }

  Node LoadNode(istream& input) {
    char c;
    while(input.get(c) && isspace(c));

    if (c == '[') {
      return LoadArray(input);
    } else if (c == '{') {
      return LoadDict(input);
    } else if (c == '"') {
      return LoadString(input);
    } else if (c == 'f' || c == 't') {
      return LoadBool(input, c);
    } else if (c == '-') {
      return LoadNegativeDouble(input);
    } else if (c == '+') {
      return LoadPositiveDouble(input);
    } else {
      input.putback(c);
      return LoadDouble(input);
    }
  }

  Document Load(istream& input) {
    return Document{LoadNode(input)};
  }

  void Print(ostream& out, const Node& node) {
    auto index = node.index();
    switch (index) {
      case ARRAY_NODE: out << node.AsArray(); break;
      case MAP_NODE: out << node.AsMap(); break;
      case NUMBER_NODE: out << fixed << node.AsDouble(); break;
      case STRING_NODE: out << "\"" << node.AsString() << "\""; break;
      default:;
    }
  }

}
