## std::visit
Функция __std::visit__ из библиотеки __<variant>__ позволяет удобно работать с объектом __std::variant__ при необходимости вызвать некоторую функцию в зависимости от типа хранимого значения.
Первый пример её использования можно увидеть в функции __ProcessStatRequests__ файла __main.cpp__:

````
vector<Json::Node> ProcessStatRequests(const Database& transport_database, const vector<Json::Node>& stat_requests) {
  vector<Json::Node> responses;
  responses.reserve(stat_requests.size());
  for (const Json::Node& request_node : stat_requests) {
    Json::Dict dict = visit([&transport_database](const auto& request) {
                              return request.Process(transport_database);
                            },
                            Requests::ReadQuery(request_node.AsMap()));
    dict["request_id"] = Json::Node(request_node.AsMap().at("id").AsInt());
    responses.push_back(Json::Node(dict));
  }
  return responses;
}
````
### Разберём пошагово вызов функции __visit__.
* __request_node__ — __Json::Node__ с запросом, __request_node.AsMap()__ — содержимое этой ноды в виде словаря.
* __Requests::ReadQuery(request_node.AsMap())__ — вызов функции парсинга запроса, возвращает запрос к базе в виде __variant<Requests::Stop, Requests::Bus, Requests::Route>__.
* __request.Process(transport_database)__ — вызов метода обработки запроса. Такое выражение валидно для request, имеющего любой из типов __Requests::Stop__, __Requests::Bus__, __Requests::Route__. Но проблема в том, что сейчас у нас есть лишь __variant<Requests::Stop, Requests::Bus, Requests::Route>__. Можно ли вызвать метод __Process__, избежав цепочки условных операторов с вызовом __holds_alternative__? Да, и мы используем для этого функцию __visit__.
* Обернём вызов метода __Process__ в лямбду с заголовком __[&transport_database](const auto& request)__: она захватывает по ссылке __transport_database__ и принимает request типа __auto__. Как правило, __auto__ означает автоматическое выведение типа, но здесь тип __auto__ имеет параметр функции, и потому функция становится шаблонной: её можно вызвать от __request__ любого из типов __Requests::Stop__, __Requests::Bus__, __Requests::Route__. В __C++17__ auto в качестве типа параметра функции можно использовать лишь в лямбда-функциях.
* Вызовем функцию __visit__ от двух аргументов: лямбда-функции, принимающей любой из типов __Requests::Stop__, __Requests::Bus__, __Requests::Route__, и собственно __variant<Requests::Stop, Requests::Bus, Requests::Route>__.
  
По сути произойдёт следующее:
* — если __variant__ хранит __Requests::Stop__, из него извлечётся значение этого типа и для него вызовется лямбда-функция, а в ней — метод __Process__;
* — если __variant__ хранит __Requests::Bus__, из него извлечётся значение этого типа и для него вызовется лямбда-функция, а в ней — метод __Process__;
* — если __variant__ хранит __Requests::Route__, из него извлечётся значение этого типа и для него вызовется лямбда-функция, а в ней — метод __Process__.

Другой пример использования функции __visit__ — распечатка __Json::Node__ в файлах __json.h__ и __json.cpp__.  
Для начала, определяется функция __PrintValue__, позволяющая вывести значение любого из типов, которые могут храниться в __JSON__:
````
template <typename Value>
void PrintValue(const Value& value, std::ostream& output) {
  output << value;
}

template <>
void PrintValue<std::string>(const std::string& value, std::ostream& output);

template <>
void PrintValue<bool>(const bool& value, std::ostream& output);

template <>
void PrintValue<std::vector<Node>>(const std::vector<Node>& nodes, std::ostream& output);

template <>
void PrintValue<Dict>(const Dict& dict, std::ostream& output);
````
Здесь использован механизм специализации шаблонной функции: по умолчанию __PrintValue__ вызывает просто output __<< value__, но для типов __string__, __bool__, __std::vector<Node>__ и __Dict PrintValue__ реализован специальным образом.  
Функция __PrintNode__ состоит из одного лишь вызова __PrintValue__ с помощью функции __visit__ и шаблонной лямбды:
````
void PrintNode(const Json::Node& node, ostream& output) {
  visit([&output](const auto& value) { PrintValue(value, output); },
        node.GetBase());
}
````
Напомним, что __Json::Node__ — наследник __variant<vector<Node>, Dict, bool, int, double, string>__, так что и здесь речь идёт о вызове некоторой функции для определённой альтернативы из объекта __variant__.
