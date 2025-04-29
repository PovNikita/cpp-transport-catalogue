#include "request_handler.h"


void Handler::FormCatalogueFromJson(std::istream& input, InputInterface* interface) {
    interface->FormCatalogue(input, catalogue_, &render_settings_, router_);
}

void Handler::FormRequestsFromJson(std::istream& input, InputInterface* interface) {
    interface->FormRequsts(input, requests_);
}

void Handler::HandleRequestsJson(std::ostream& output, OutputInterface* interface) {
    interface->HandleRequests(output, requests_, catalogue_, render_settings_, router_);
}

void Handler::DrawMap(std::ostream& output)
{
    map_render::Render render;
    render.DrawMap(output, catalogue_.GetAllRoutes(), &render_settings_);    
}