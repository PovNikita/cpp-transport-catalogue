#include "request_handler.h"
#include "json_reader.h"


void Handler::FormCatalogueFromJson(std::istream& input, InputInterface* interface) {
    interface->FormCatalogue(input, catalogue_, &render_settings_);
}

void Handler::FormRequestsFromJson(std::istream& input, InputInterface* interface) {
    interface->FormRequsts(input, requests_);
}

void Handler::HandleRequestsJson(std::ostream& output, OutputInterface* interface) {
    interface->HandleRequests(output, requests_, catalogue_, render_settings_);
}