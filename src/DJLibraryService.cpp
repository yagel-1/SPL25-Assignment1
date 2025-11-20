#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist) {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */
void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
    for (int i = 0; i < library_tracks.size(); i++){
        std::string type = library_tracks[i].type;
        if(type == "MP3"){
            library.push_back(new MP3Track(library_tracks[i].title,library_tracks[i].artists, 
                library_tracks[i].duration_seconds, library_tracks[i].bpm, library_tracks[i].extra_param1, library_tracks[i].extra_param2));
            std::cout << "MP3Track created: " << library_tracks[i].extra_param1 << " kbps" << std::endl;
        }
        else{
            library.push_back(new WAVTrack(library_tracks[i].title,library_tracks[i].artists, 
                library_tracks[i].duration_seconds, library_tracks[i].bpm, library_tracks[i].extra_param1, library_tracks[i].extra_param2));
            std::cout << "WAVTrack created: " << library_tracks[i].extra_param1 << "Hz/" << library_tracks[i].extra_param2 << "bit" << std::endl; 
        }
    }
    std::cout << "[INFO] Track library built: " << library_tracks.size() << " tracks loaded" << std::endl;
}

/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: " 
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    return playlist;
}

/**
 * TODO: Implement findTrack method
 * 
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    return playlist.find_track(track_title);
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name, 
                                                const std::vector<int>& track_indices) {
    std::cout << "[INFO] Loading playlist: " << playlist_name << std::endl;
    playlist = Playlist(playlist_name);
    for (int index : track_indices){
        if ((index-1) < 0 || (index-1) >= library.size()){
            std::cout << "[WARNING] Invalid track index: " << index << std::endl;
            continue;
        }
        PointerWrapper<AudioTrack> wrap_track = library[index-1]->clone();
        if (!wrap_track){
            std::cout << "[ERROR] Track: \""<< library[index-1]->get_title() << "\" failed to clone" << std::endl;
            continue;
        }
        AudioTrack* unwrap_track = wrap_track.release();
        unwrap_track->load();
        unwrap_track->analyze_beatgrid();
        playlist.add_track(unwrap_track);
        std::cout << "Added \'"<< unwrap_track->get_title() << "\' to playlist \'" << playlist_name << "\'" << std::endl;
    }
    std::cout << "[INFO] Playlist loaded: "<< playlist_name << " (" << playlist.get_track_count() << " tracks)" << std::endl;
}
/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    std::vector<std::string> tracks_titles = std::vector<std::string>();
    for (AudioTrack* track : playlist.getTracks()){
        tracks_titles.push_back(track->get_title());
    }
    return tracks_titles;
}
