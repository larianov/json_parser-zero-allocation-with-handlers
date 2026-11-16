#include "default_handler.h"
#include "parser.h"
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {
struct Departure {
    std::string_view destination;
    std::string_view departure;
    std::string_view service;
    std::string_view arrival;
};

// A fixed-capacity, event-driven view of the example schema. All fields borrow
// the input buffer; no DOM or heap allocations are needed by this handler.
class BoardHandler : public Default_Handler {
    enum class Context { other, root, board, departures, departure };
    std::array<Context, 64> contexts{};
    std::size_t depth{};
    std::string_view key;

    Context context() const {
        return depth == 0 ? Context::other : contexts[depth - 1];
    }

public:
    std::array<Departure, 32> rows{};
    std::size_t count{};
    std::string_view station, date, period;
    bool valid{true};

    bool on_key(std::string_view value) override {
        key = value;
        return true;
    }

    bool on_object_start() override {
        Context next = Context::other;
        if (depth == 0) next = Context::root;
        else if (context() == Context::root && key == "board") next = Context::board;
        else if (context() == Context::board && key == "departures") next = Context::departures;
        else if (context() == Context::departures) {
            if (count == rows.size()) valid = false;
            else {
                rows[count] = {};
                next = Context::departure;
            }
        }
        contexts[depth++] = next; // Parser limits container nesting to 63.
        return true;
    }

    bool on_object_end() override {
        if (context() == Context::departure) {
            if (rows[count].destination.empty() || rows[count].departure.empty()) valid = false;
            ++count;
        }
        --depth;
        return true;
    }

    bool on_array_start() override {
        contexts[depth++] = Context::other;
        return true;
    }
    bool on_array_end() override { --depth; return true; }

    bool on_string(std::string_view value) override {
        if (context() == Context::board) {
            if (key == "station") station = value;
            else if (key == "date") date = value;
            else if (key == "period") period = value;
        } else if (context() == Context::departure) {
            auto& row = rows[count];
            if (key == "destination") row.destination = value;
            else if (key == "departure") row.departure = value;
            else if (key == "service") row.service = value;
            else if (key == "arrival") row.arrival = value;
        }
        return true;
    }
};

std::string_view or_dash(std::string_view value) {
    return value.empty() ? "-" : value;
}
}

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [departure-board.json]\n";
        return 2;
    }
    const char* filename = argc == 2 ? argv[1] : "examples/enschede-departures.json";
    std::ifstream file{filename, std::ios::binary};
    if (!file) {
        std::cerr << "Cannot open JSON file: " << filename << '\n';
        return 1;
    }
    std::string input;
    try {
        input.assign(std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
    } catch (const std::ios_base::failure&) {
        std::cerr << "Error reading JSON file\n";
        return 1;
    }
    if (file.bad()) {
        std::cerr << "Error reading JSON file\n";
        return 1;
    }

    BoardHandler handler;
    Parser parser{input.data(), input.size(), handler};
    const auto parsed = parser.parse();
    if (!parsed) {
        std::cerr << "Parse error at byte " << parsed.error().offset_of_error
                  << ": " << parsed.error().error_descrip << '\n';
        return 1;
    }
    if (!handler.valid || handler.station.empty() || handler.date.empty() ||
        handler.period.empty() || handler.count == 0) {
        std::cerr << "Expected board metadata and 1 to 32 departures with destination and departure\n";
        return 1;
    }

    std::cout << handler.station << " | " << handler.date << " | " << handler.period << '\n'
              << "SYNTHETIC EXAMPLE: times are unverified; not for travel planning.\n\n"
              << std::left << std::setw(11) << "Departure" << std::setw(20) << "Destination"
              << std::setw(9) << "Service" << "Arrival\n";
    for (std::size_t i = 0; i < handler.count; ++i) {
        const auto& row = handler.rows[i];
        std::cout << std::setw(11) << row.departure << std::setw(20) << row.destination
                  << std::setw(9) << or_dash(row.service) << or_dash(row.arrival) << '\n';
    }
}
