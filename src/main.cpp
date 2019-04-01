//#include "stringparser.h"
#include "request.h"
#include "BusStopMap.h"
#include "json.h"

#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>

#include <algorithm>

using namespace std;

using Requests = list<Request::RequestHolder>;

#define OUTPUT_REQUEST false
#define INPUT_REQUEST true

const unordered_map<string_view, Request::Type> STR_TO_INPUT_REQUEST_TYPE = {
  {"Stop", Request::Type::STOP_DECLARATION},
  {"Bus", Request::Type::ROUTE_DEFINITION}
};

const unordered_map<string_view, Request::Type> STR_TO_OUTPUT_REQUEST_TYPE = {
  {"Bus", Request::Type::BUS_INFO},
  {"Stop", Request::Type::STOP_INFO},
  {"Route", Request::Type::ROUTE_INFO}
};

optional<Request::Type> ConvertRequestTypeFromString(string_view type_str, bool input) {
  const unordered_map<string_view, Request::Type>* typesMap = nullptr;
  if(input) {
    typesMap = &STR_TO_INPUT_REQUEST_TYPE;
  } else {
    typesMap = &STR_TO_OUTPUT_REQUEST_TYPE;
  }
  
  if (const auto it = typesMap->find(type_str);
      it != typesMap->end()) {
    return it->second;
  } else {
    return nullopt;
  }
}

void ParseRequest(const map<string, Json::Node>& request, Requests& requests, bool input) {
  const auto type_it = request.find("type");
  if(type_it != request.end()) {
    if(type_it->second.index() == STRING_NODE) {
      const auto request_type = ConvertRequestTypeFromString(type_it->second.AsString(), input);
      if (request_type) {
        Request::RequestHolder new_request = Request::Create(*request_type);
        if (new_request) {
          new_request->ParseFrom(request);
          requests.push_back(move(new_request));
        };
      }
    }
  }
}

void ReadTypedRequests(const Json::Node& node, Requests& requests, bool input) {
  auto index = node.index();
  if(index == MAP_NODE) {
    // Only one input request { request }
    ParseRequest(node.AsMap(), requests, input);
  } else if(index == ARRAY_NODE) {
    // Vector of input requests [ request1, request2, ...]
    for(const auto& item : node.AsArray()) {
      if(item.index() == MAP_NODE) {
        ParseRequest(item.AsMap(), requests, input);
      }
    }
  }
}

void ReadRequests(const map<string, Json::Node>& request_nodes, Requests& requests) {
  for(const auto& [key, value] : request_nodes) {
    if(key == "base_requests") {
      ReadTypedRequests(value, requests, INPUT_REQUEST);
    } else if(key == "stat_requests") {
      ReadTypedRequests(value, requests, OUTPUT_REQUEST);
    } else if(key == "routing_settings") {
      if(value.index() == MAP_NODE) {
        Request::RequestHolder new_request = Request::Create(Request::Type::ROUTE_SETTINGS);
        new_request->ParseFrom(value.AsMap());
        requests.push_back(move(new_request));
      }
    }
  }
}

Requests FindRequests(const Json::Document& doc) {
  Requests requests;
  /* Try to find any requsts as map or as vector of maps */
  const auto& root = doc.GetRoot();
  if(auto index = root.index(); index != variant_npos) {
    if(index == MAP_NODE) {
      ReadRequests(root.AsMap(), requests);
    /*} else if (index == ARRAY_NODE) {
      for(const auto& item : root.AsArray()) {
        auto request_array_index = item.index();
        if(request_array_index == MAP_NODE) {
          ReadRequests(item.AsMap(), requests);
        }
      }*/
    }
  }

  return requests;
}

vector<string> ProcessRequests(Requests& requests, BusStopMap& map) {
  vector<Request::Type> types = {Request::Type::STOP_DECLARATION, 
                                 Request::Type::ROUTE_DEFINITION};
  for(auto type : types) {
    auto it = requests.begin();
    while(it != requests.end()) {
      auto& request_holder = *it;
      if(request_holder->type == type) {
        auto& request = static_cast<ModifyRequest&>(*request_holder);
        request.Process(map);
        it = requests.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  for(auto& request_holder : requests) {
    if(request_holder->type == Request::Type::ROUTE_SETTINGS) {
      auto& request = static_cast<ModifyRequest&>(*request_holder);
      request.Process(map);
      break;
    }
  }

  vector<string> responses;

  for (const auto& request_holder : requests) {
    if(request_holder->type == Request::Type::BUS_INFO
       || request_holder->type == Request::Type::STOP_INFO
       || request_holder->type == Request::Type::ROUTE_INFO) {
      const auto& request = static_cast<const PrintRequest<string>&>(*request_holder);
      responses.push_back(request.Process(map));
    }
  }
  return responses;
}

void PrintResponses(const vector<string>& responses, ostream& stream = cout) {
  stream << "[\n";
  size_t size = responses.size();
  for (const auto& response : responses) {
    stream << "  {\n";
    stream << response << endl;
    stream << "  }";
    if(--size != 0) {
      stream << ",";
    }
    stream << "\n";
  }
  stream << "]\n";
}

int main() {
  BusStopMap map;

  Json::Document doc = Json::Load(cin);

  auto requests = FindRequests(doc);
  const auto responses = ProcessRequests(requests, map);
  PrintResponses(responses);

  return 0;
}

/*---------------------------------------------------------------------*/
/**
 * @brief: Requests parses for standard input/output stream
 */

//template <typename Number>
//Number ReadQueriesCount(istream& stream) {
//  Number number;
//  stream >> number;
//  string dummy;
//  getline(stream, dummy);
//  return number;
//}

//Request::RequestHolder ParseRequest(const string& request_str, bool input) {
//  const auto request_type = ConvertRequestTypeFromString(request_str, input);
//  if (!request_type) {
//    return nullptr;
//  }
//  Request::RequestHolder request = Request::Create(*request_type);
//  if (request) {
//    request->ParseFrom(request_str);
//  };
//  return request;
//}

//Requests ReadRequests(bool input, istream& in_stream = cin) {
//  const size_t request_count = ReadQueriesCount<size_t>(in_stream);

//  Requests requests;
//  requests.reserve(request_count);

//  for (size_t i = 0; i < request_count; ++i) {
//    string request_str;
//    getline(in_stream, request_str);
//    if (auto request = ParseRequest(request_str, input)) {
//      requests.push_back(move(request));
//    }
//  }
//  return requests;
//}

// int main() {
//   BusStopMap map;
//   auto input_requests = ReadRequests(INPUT_REQUEST);
//   ProcessRequests(input_requests, map);
  
//   const auto output_requests = ReadRequests(OUTPUT_REQUEST);
//   const auto responses = ProcessOutputRequests(output_requests, map);
//   PrintResponses(responses);

//   return 0;
// }

