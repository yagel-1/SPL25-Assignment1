#include "MixingEngineService.h"
#include <iostream>
#include <memory>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : decks{nullptr, nullptr}, active_deck(0), auto_sync(false), bpm_tolerance(0)
{
    std::cout << "[MixingEngineService] Initialized with 2 empty decks" << std::endl;
}

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    std::cout << "[MixingEngineService] Cleaning up decks..." << std::endl;
    for (AudioTrack*& deck : decks){
        delete deck;
        deck = nullptr;
    }
}
/*
* copy assigment operator
*/
MixingEngineService& MixingEngineService::operator=(const MixingEngineService& other){
    if (this != &other ){
        active_deck = other.active_deck;
        auto_sync = other.auto_sync;
        bpm_tolerance = other.bpm_tolerance;
        for (int i = 0; i < 2; ++i) {
            delete decks[i];            
            if (other.decks[i] != nullptr) {
                decks[i] = other.decks[i]->clone().release(); 
            } 
            else {
                decks[i] = nullptr;
            }
        }
    }
    return *this;
}

/*
* copy constructor
*/
MixingEngineService::MixingEngineService(const MixingEngineService& other): active_deck(other.active_deck), auto_sync(other.auto_sync),
                                                                            bpm_tolerance(other.bpm_tolerance), decks{nullptr,nullptr}{
    for(int i= 0; i < 2; i++){
        if(other.decks[i] != nullptr)
        decks[i] = other.decks[i]->clone().release();
    }
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    bool first_track = !decks[0] && !decks[1];
    size_t target;
    if (first_track){
        target = 0;
        active_deck = 0;
    }
    else{
        target = 1 - active_deck;
    }

    std::cout << "\n=== Loading Track to Deck ===" << std::endl;

    PointerWrapper<AudioTrack> wrap_track;
    wrap_track = track.clone();
    if (!wrap_track){
        std::cout << "[ERROR] Track: \""<< track.get_title() << "\" failed to clone" << std::endl;
        return -1;
    }

    std::cout << "[Deck Switch] Target deck: "<< target << std::endl;

    if (!first_track){
        if (decks[target]){
            delete decks[target];
            decks[target] = nullptr;
        }
    }

    wrap_track->load();
    wrap_track->analyze_beatgrid();

    bool active_deck_exists = decks[active_deck];
    if (active_deck_exists && auto_sync && !can_mix_tracks(wrap_track)){
        sync_bpm(wrap_track);
    }
    decks[target] = wrap_track.release();

    std::cout << "[Load Complete] " << decks[target]->get_title() << " is now loaded in deck "<< target << std::endl;
    if (!first_track && decks[active_deck]){
        std::cout << "[Unload] Unloading previous deck "<< active_deck << " (" << decks[active_deck]->get_title() << ")" << std::endl;
        delete decks[active_deck];
        decks[active_deck] = nullptr;
    }

    active_deck = target;
    std::cout << "[Active Deck] Switched to deck "<< target << std::endl;
    return target;
}
    


/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    if (!decks[active_deck] || !track){
        return false;
    }
    int deck_bpm = decks[active_deck]->get_bpm();
    int track_bpm = track->get_bpm(); 
    return (deck_bpm - track_bpm <= bpm_tolerance) || (track_bpm - deck_bpm <= bpm_tolerance);
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    if (decks[active_deck] && track){
        int track_bpm = track->get_bpm();
        int deck_bpm = decks[active_deck]->get_bpm();
        int avg_bpm = (track_bpm + deck_bpm) / 2;
        track->set_bpm(avg_bpm);
        std::cout << "[Sync BPM] Syncing BPM from " << track_bpm << " to " << avg_bpm << std::endl;
    }
}
