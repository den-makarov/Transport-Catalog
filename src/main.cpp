#include "stringparser.h"
#include "request.h"
#include "BusStopMap.h"
#include "json.h"

#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

//template <typename Number>
//Number ReadQueriesCount(istream& stream) {
//  Number number;
//  stream >> number;
//  string dummy;
//  getline(stream, dummy);
//  return number;
//}

const unordered_map<string_view, Request::Type> STR_TO_INPUT_REQUEST_TYPE = {
  {"Stop", Request::Type::STOP_DECLARATION},
  {"Bus", Request::Type::ROUTE_DEFINITION}
};

const unordered_map<string_view, Request::Type> STR_TO_OUTPUT_REQUEST_TYPE = {
  {"Bus", Request::Type::BUS_INFO},
  {"Stop", Request::Type::STOP_INFO}
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

//vector<Request::RequestHolder> ReadRequests(bool input, istream& in_stream = cin) {
//  const size_t request_count = ReadQueriesCount<size_t>(in_stream);

//  vector<Request::RequestHolder> requests;
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

//void ProcessInputRequests(vector<Request::RequestHolder>& requests,
//                          BusStopMap& map) {
//  for (auto& request_holder : requests) {
//    auto& request = static_cast<ModifyRequest&>(*request_holder);
//    if(request.type == Request::Type::STOP_DECLARATION) {
//      request.Process(map);
//    }
//  }

//  for (auto& request_holder : requests) {
//    auto& request = static_cast<ModifyRequest&>(*request_holder);
//    if(request.type == Request::Type::ROUTE_DEFINITION) {
//      request.Process(map);
//    }
//  }
//}

//vector<string> ProcessOutputRequests(const vector<Request::RequestHolder>& requests,
//                                     const BusStopMap& map) {
//  vector<string> responses;
//  for (const auto& request_holder : requests) {
//    const auto& request = static_cast<const PrintRequest<string>&>(*request_holder);
//    responses.push_back(request.Process(map));
//  }
//  return responses;
//}

//void PrintResponses(const vector<string>& responses, ostream& stream = cout) {
//  for (const auto& response : responses) {
//    stream << response << endl;
//  }
//}

#define ARRAY_NODE 0
#define MAP_NODE 1
#define STRING_NODE 2
#define NUMBER_NODE 3

#define OUTPUT_REQUEST false
#define INPUT_REQUEST true

void ParseRequest(const map<string, Json::Node>& request, vector<Request::RequestHolder>& requests) {
  const auto type_it = request.find("type");
  if(type_it != request.end()) {
    if(type_it->second.index() == STRING_NODE) {
      const auto request_type = ConvertRequestTypeFromString(type_it->second.AsString(), INPUT_REQUEST);
      if (request_type) {
        Request::RequestHolder request = Request::Create(*request_type);
        if (request) {
          // Parse request Data
          //request->ParseFrom(request_str);
          requests.push_back(move(request));
        };
      }
    }
  }
}

void ReadInputRequests(const Json::Node& node, vector<Request::RequestHolder>& requests) {
  auto index = node.index();
  if(index == MAP_NODE) {
    // Only one input request { request }
    ParseRequest(node.AsMap(), requests);
  } else if(index == ARRAY_NODE) {
    // Vector of input requests [ request1, request2, ...]
    for(const auto& item : node.AsArray()) {
      if(item.index() == MAP_NODE) {
        ParseRequest(item.AsMap(), requests);
      }
    }
  }
}

void ReadRequests(const map<string, Json::Node>& request_nodes, vector<Request::RequestHolder>& requests) {
  for(const auto& [key, value] : request_nodes) {
    if(key == "base_requests") {
      ReadInputRequests(value, requests);
    } else if(key == "stat_requests") {

    }
  }
}

vector<Request::RequestHolder> FindRequests(const Json::Document& doc) {
  vector<Request::RequestHolder> requests;
  /* Try to find any requsts as map or as vector of maps */
  const auto& root = doc.GetRoot();
  if(auto index = root.index(); index != variant_npos) {
    if(index == MAP_NODE) {
      ReadRequests(root.AsMap(), requests);
//    } else if (index == ARRAY_NODE) {
//      for(const auto& item : root.AsArray()) {
//        auto request_array_index = item.index();
//        if(request_array_index == MAP_NODE) {
//          ReadRequests(item.AsMap(), requests);
//        }
//      }
    }
  }

  return requests;
}

int main() {
  BusStopMap map;

  Json::Document doc = Json::Load(cin);

  auto requests = FindRequests(doc);

//

//  auto input_requests = ReadRequests(INPUT_REQUEST);
//  ProcessInputRequests(input_requests, map);
  
//  const auto output_requests = ReadRequests(OUTPUT_REQUEST);
//  const auto responses = ProcessOutputRequests(output_requests, map);
//  PrintResponses(responses);

  return 0;
}
