#include "request_handler.h"

void TC::RequestHandler::QueueRequest(base_request_t& request) {
    request_queue_.insert(std::move(request));
}

