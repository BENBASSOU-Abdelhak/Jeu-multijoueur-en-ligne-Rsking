#include "logic/player.h"

Player::Player(std::string tag) : 
    m_tag{tag}, 
    m_is_online{true},
    m_already_transfered{false},
    m_remaining_deploy_troops{0},
    m_last_attacked_square{0},
    m_square_from_last_attack{0},
    m_nb_troops{0},
    m_nb_square{0},
    m_nb_area{0},
    m_area_points{0}
{}


bool Player::operator==(Player const& player) const {
    return m_tag == player.m_tag;
}


bool Player::operator==(std::string const& gamertag) const {
    return !m_tag.compare(gamertag);
}


bool Player::operator!=(Player const& player) const {
    return m_tag != player.m_tag;
}


uint16_t Player::get_nb_troops() const {
    return m_nb_troops;
}


uint16_t Player::get_area_points() const {
    return m_area_points;
}


void Player::set_nb_troops(uint16_t nb_troops) {
    m_nb_troops = nb_troops;
}


void Player::set_area_points(uint16_t value) {
    m_area_points = value;
}


uint16_t Player::get_nb_square() const {
    return m_nb_square;
}


void Player::set_nb_square(uint16_t nb_square) {
    m_nb_square = nb_square;
}


void Player::set_nb_area(uint16_t nb_area) {
    m_nb_area = nb_area;
}


uint16_t Player::get_nb_area() const {
    return m_nb_area;
}


void Player::set_disconnect() {
    m_is_online = false;
    m_remaining_deploy_troops = 0; //pour ne pas empecher le passage aux autres joueurs
}


bool Player::is_online() const{
    return m_is_online;
}


bool Player::is_already_transfered() const {
    return m_already_transfered;
}


void Player::set_already_transfered(bool value) {
    m_already_transfered = value;
}


uint16_t Player::get_remaining_deploy_troops() const {
    return m_remaining_deploy_troops;
}


void Player::set_remaining_deploy_troops(uint16_t nb_troops) {
    m_remaining_deploy_troops = nb_troops;
}


std::string const& Player::get_tag() const {
    return m_tag;
}


uint16_t Player::get_last_atk_square() const {
    return m_last_attacked_square;
}

uint16_t Player::get_square_from_last_atk() const {
    return m_square_from_last_attack;
}

void Player::set_last_atk_square(uint16_t square) {
    m_last_attacked_square = square;
}

void Player::set_square_from_last_atk(uint16_t square) {
    m_square_from_last_attack = square;
}
