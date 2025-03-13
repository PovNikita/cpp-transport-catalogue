#include <iostream>
#include <sstream>

#include "request_handler.h"
#include "json_reader.h"

int main() {

    Handler handler;
    InputReader json_reader;
    StatAnswer json_answer;

    handler.FormCatalogueFromJson(std::cin, dynamic_cast<InputInterface*>(&json_reader));
    handler.FormRequestsFromJson(std::cin, dynamic_cast<InputInterface*>(&json_reader));
    handler.HandleRequestsJson(std::cout, dynamic_cast<OutputInterface*>(&json_answer));
}