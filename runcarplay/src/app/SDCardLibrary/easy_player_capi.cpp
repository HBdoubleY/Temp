#include "easy_player_capi.h"
#include "EasyPlayer.h"
#include <cstring>
#include <string>

// C++对象
struct EasyPlayerWrapper {
    MP4Player* player;
    
    EasyPlayerWrapper() : player(nullptr) {}
    ~EasyPlayerWrapper() {
        if (player) {
            delete player;
            player = nullptr;
        }
    }
};

static EasyPlayerResult convert_bool_to_result(bool success) {
    return success ? EASY_PLAYER_SUCCESS : EASY_PLAYER_ERROR_UNKNOWN;
}

static EasyPlayerResult validate_handle(EasyPlayerHandle handle) {
    if (!handle) {
        return EASY_PLAYER_ERROR_INVALID_HANDLE;
    }
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    if (!wrapper->player) {
        return EASY_PLAYER_ERROR_INVALID_HANDLE;
    }
    return EASY_PLAYER_SUCCESS;
}

static MP4Player::Rotation convert_c_rotation_to_cpp(EasyPlayerRotation rotation) {
    switch (rotation) {
        case EASY_PLAYER_ROTATION_0: return MP4Player::ROTATION_0;
        case EASY_PLAYER_ROTATION_90: return MP4Player::ROTATION_90;
        case EASY_PLAYER_ROTATION_180: return MP4Player::ROTATION_180;
        case EASY_PLAYER_ROTATION_270: return MP4Player::ROTATION_270;
        default: return MP4Player::ROTATION_0;
    }
}

static EasyPlayerRotation convert_cpp_rotation_to_c(MP4Player::Rotation rotation) {
    switch (rotation) {
        case MP4Player::ROTATION_0: return EASY_PLAYER_ROTATION_0;
        case MP4Player::ROTATION_90: return EASY_PLAYER_ROTATION_90;
        case MP4Player::ROTATION_180: return EASY_PLAYER_ROTATION_180;
        case MP4Player::ROTATION_270: return EASY_PLAYER_ROTATION_270;
        default: return EASY_PLAYER_ROTATION_0;
    }
}

// 修正：正确的状态转换函数
static EasyPlayerState convert_cpp_state_to_c(MP4Player::PlayerState state) {
    switch (state) {
        case MP4Player::STATE_IDLE: return EASY_PLAYER_STATE_IDLE;
        case MP4Player::STATE_INITIALIZED: return EASY_PLAYER_STATE_INITIALIZED;
        case MP4Player::STATE_PREPARED: return EASY_PLAYER_STATE_PREPARED;
        case MP4Player::STATE_PLAYING: return EASY_PLAYER_STATE_PLAYING;
        case MP4Player::STATE_PAUSED: return EASY_PLAYER_STATE_PAUSED;
        case MP4Player::STATE_STOPPED: return EASY_PLAYER_STATE_STOPPED;
        case MP4Player::STATE_ERROR: return EASY_PLAYER_STATE_ERROR;
        default: return EASY_PLAYER_STATE_IDLE;
    }
}

EasyPlayerHandle easy_player_create(void) {
    EasyPlayerWrapper* wrapper = new EasyPlayerWrapper();
    if (!wrapper) {
        return nullptr;
    }
    
    wrapper->player = new MP4Player();
    if (!wrapper->player) {
        delete wrapper;
        return nullptr;
    }
    
    return static_cast<EasyPlayerHandle>(wrapper);
}

EasyPlayerResult easy_player_destroy(EasyPlayerHandle handle) {
    if (!handle) {
        return EASY_PLAYER_ERROR_INVALID_HANDLE;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    delete wrapper;
    
    return EASY_PLAYER_SUCCESS;
}

EasyPlayerResult easy_player_initialize(EasyPlayerHandle handle, int displayWidth, int displayHeight) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->Initialize(displayWidth, displayHeight);
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_set_display_rect(EasyPlayerHandle handle, int x, int y, int width, int height) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->SetDisplayRect(x, y, width, height);
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_play_file(EasyPlayerHandle handle, const char* filePath) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    if (!filePath) {
        return EASY_PLAYER_ERROR_INVALID_PARAM;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    std::string filePathStr(filePath);
    bool success = wrapper->player->PlayFile(filePathStr);
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_stop(EasyPlayerHandle handle) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->StopPlay();
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_pause(EasyPlayerHandle handle) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->Pause();
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_resume(EasyPlayerHandle handle) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->Resume();
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_seek_to(EasyPlayerHandle handle, int milliseconds) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->SeekTo(milliseconds);
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_set_volume(EasyPlayerHandle handle, float leftVolume, float rightVolume) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->SetVolume(leftVolume, rightVolume);
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_set_rotation(EasyPlayerHandle handle, EasyPlayerRotation rotation) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    MP4Player::Rotation cppRotation = convert_c_rotation_to_cpp(rotation);
    bool success = wrapper->player->SetRotation(cppRotation);
    
    return convert_bool_to_result(success);
}

EasyPlayerResult easy_player_get_current_position(EasyPlayerHandle handle, int* position) {
    if (!position) {
        return EASY_PLAYER_ERROR_INVALID_PARAM;
    }
    
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    *position = wrapper->player->GetCurrentPosition();
    
    return EASY_PLAYER_SUCCESS;
}

EasyPlayerResult easy_player_get_duration(EasyPlayerHandle handle, int* duration) {
    if (!duration) {
        return EASY_PLAYER_ERROR_INVALID_PARAM;
    }
    
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    *duration = wrapper->player->GetDuration();
    
    return EASY_PLAYER_SUCCESS;
}

EasyPlayerResult easy_player_get_state(EasyPlayerHandle handle, EasyPlayerState* state) {
    if (!state) {
        return EASY_PLAYER_ERROR_INVALID_PARAM;
    }
    
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    MP4Player::PlayerState cppState = wrapper->player->GetState();
    *state = convert_cpp_state_to_c(cppState);
    
    return EASY_PLAYER_SUCCESS;
}

EasyPlayerResult easy_player_get_rotation(EasyPlayerHandle handle, EasyPlayerRotation* rotation) {
    if (!rotation) {
        return EASY_PLAYER_ERROR_INVALID_PARAM;
    }
    
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    MP4Player::Rotation cppRotation = wrapper->player->GetRotation();
    *rotation = convert_cpp_rotation_to_c(cppRotation);
    
    return EASY_PLAYER_SUCCESS;
}

int easy_player_is_playing(EasyPlayerHandle handle) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return -1;
    }
    
    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool isPlaying = wrapper->player->IsPlaying();
    
    return isPlaying ? 1 : 0;
}

EasyPlayerResult easy_player_set_video_speed(EasyPlayerHandle handle, float num) {
    EasyPlayerResult result = validate_handle(handle);
    if (result != EASY_PLAYER_SUCCESS) {
        return result;
    }

    EasyPlayerWrapper* wrapper = static_cast<EasyPlayerWrapper*>(handle);
    bool success = wrapper->player->SetVideoSpeed(num);

    return convert_bool_to_result(success);
}

const char* easy_player_get_error_string(EasyPlayerResult result) {
    switch (result) {
        case EASY_PLAYER_SUCCESS: return "Success";
        case EASY_PLAYER_ERROR_INVALID_HANDLE: return "Invalid player handle";
        case EASY_PLAYER_ERROR_INIT_FAILED: return "Player initialization failed";
        case EASY_PLAYER_ERROR_FILE_NOT_EXIST: return "File does not exist";
        case EASY_PLAYER_ERROR_PLAY_FAILED: return "Play operation failed";
        case EASY_PLAYER_ERROR_INVALID_PARAM: return "Invalid parameter";
        case EASY_PLAYER_ERROR_WRONG_STATE: return "Player in wrong state for operation";
        case EASY_PLAYER_ERROR_UNKNOWN: return "Unknown error";
        default: return "Unknown error code";
    }
}