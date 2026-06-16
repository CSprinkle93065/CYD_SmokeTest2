/**
 * @file coin_logic.cpp
 * @brief Coin-toss business logic.
 */

#include "state_machine.h"
#include <Arduino.h>

namespace app {

coin_side_t coin_flip() {
    return (esp_random() % 2u == 0u) ? coin_side_t::HEADS : coin_side_t::TAILS;
}

const char* coin_side_to_string(coin_side_t side) {
    switch (side) {
        case coin_side_t::HEADS: return "Heads";
        case coin_side_t::TAILS: return "Tails";
    }
    return "Unknown";
}

void log_coin_result(coin_side_t side) {
    Serial.print("COIN: ");
    Serial.println(coin_side_to_string(side));
}

} // namespace app
