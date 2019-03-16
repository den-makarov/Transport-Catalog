#include "stringparser.h"
#include "request.h"
#include "BusStopMap.h"

#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

template <typename Number>
Number ReadQueriesCount(istream& stream) {
  Number number;
  stream >> number;
  string dummy;
  getline(stream, dummy);
  return number;
}

const unordered_map<string_view, Request::Type> STR_TO_INPUT_REQUEST_TYPE = {
    {"Stop", Request::Type::STOP_DECLARATION},
    {"Bus", Request::Type::ROUTE_DEFINITION}
};

const unordered_map<string_view, Request::Type> STR_TO_OUTPUT_REQUEST_TYPE = {
    {"Bus", Request::Type::BUS_INFO}
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

Request::RequestHolder ParseRequest(string_view request_str, bool input) {
  const auto request_type = ConvertRequestTypeFromString(ReadToken(request_str), input);
  if (!request_type) {
    return nullptr;
  }
  Request::RequestHolder request = Request::Create(*request_type);
  if (request) {
    request->ParseFrom(request_str);
  };
  return request;
}

vector<Request::RequestHolder> ReadRequests(bool input, istream& in_stream = cin) {
  const size_t request_count = ReadQueriesCount<size_t>(in_stream);

  vector<Request::RequestHolder> requests;
  requests.reserve(request_count);

  for (size_t i = 0; i < request_count; ++i) {
    string request_str;
    getline(in_stream, request_str);
    if (auto request = ParseRequest(request_str, input)) {
      requests.push_back(move(request));
    }
  }
  return requests;
}

void ProcessInputRequests(const vector<Request::RequestHolder>& requests, 
                          BusStopMap& map) {
  for (const auto& request : requests) {
     request.Process(map);
  }
//    if (request_holder->type == Request::Type::COMPUTE_INCOME) {
//      const auto& request = static_cast<const ComputeIncomeRequest&>(*request_holder);
//      responses.push_back(request.Process(manager));
//    } else {
//      const auto& request = static_cast<const ModifyRequest&>(*request_holder);
//      request.Process(manager);
//    }
}

vector<double> ProcessOutputRequests(const vector<Request::RequestHolder>& requests,
                                     BusStopMap& map) {
  vector<double> responses;
//  BusStopMap stops(0);
//  for (const auto& request_holder : requests) {
//    if (request_holder->type == Request::Type::COMPUTE_INCOME) {
//      const auto& request = static_cast<const ComputeIncomeRequest&>(*request_holder);
//      responses.push_back(request.Process(manager));
//    } else {
//      const auto& request = static_cast<const ModifyRequest&>(*request_holder);
//      request.Process(manager);
//    }
//  }
  return responses;
}

void PrintResponses(const vector<double>& responses, ostream& stream = cout) {
  for (const double response : responses) {
    stream << response << endl;
  }
}

#define INPUT_REQUEST true 
#define OUTPUT_REQUEST false

int main() {
  BusStopMap map;

  const auto input_requests = ReadRequests(INPUT_REQUEST);
  ProcessInputRequests(input_requests, map);
  
  const auto output_requests = ReadRequests(OUTPUT_REQUEST);
  const auto responses = ProcessOutputRequests(output_requests, map);
  PrintResponses(responses);

  return 0;
}
