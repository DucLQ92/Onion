#ifndef TWEAKS_RESET_H__
#define TWEAKS_RESET_H__

#include <stdio.h>

#include "system/device_model.h"
#include "system/keymap_sw.h"
#include "theme/render/dialog.h"
#include "theme/sound.h"

#include "./appstate.h"

#define RESET_CONFIGS_PAK "/mnt/SDCARD/.tmp_update/config/configs.pak"

bool _confirmReset(const char *title_str, const char *message_str)
{
    bool retval = false;
    bool confirm_quit = false;
    SDLKey changed_key = SDLK_UNKNOWN;

    keys_enabled = false;

    background_cache = SDL_CreateRGBSurface(SDL_HWSURFACE, 640, 480, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, NULL, background_cache, NULL);

    theme_renderDialog(screen, title_str, message_str, true);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);

    while (!confirm_quit) {
        if (updateKeystate(keystate, &confirm_quit, true, &changed_key)) {
            if (changed_key == SW_BTN_B && keystate[SW_BTN_B] == PRESSED)
                confirm_quit = true;
            else if (changed_key == SW_BTN_A && keystate[SW_BTN_A] == PRESSED) {
                retval = true;
                confirm_quit = true;
            }
        }
    }

    if (changed_key != SDLK_UNKNOWN)
        sound_change();

    if (retval) {
        SDL_BlitSurface(screen, NULL, background_cache, NULL);
        theme_renderDialog(screen, title_str, "Đang thiết lập lại...", false);
        SDL_BlitSurface(screen, NULL, video, NULL);
        SDL_Flip(video);
    }
    else {
        keys_enabled = true;
        all_changed = true;
    }

    return retval;
}

void _notifyResetDone(const char *title_str)
{
    SDL_BlitSurface(background_cache, NULL, screen, NULL);
    theme_renderDialog(screen, title_str, "Xong", false);
    SDL_BlitSurface(screen, NULL, video, NULL);
    SDL_Flip(video);
    msleep(300);

    SDL_FreeSurface(background_cache);
    keys_enabled = true;
    all_changed = true;

    sync();
}

void action_resetTweaks(void *pt)
{
    const char title_str[] = "Thiết lập lại các tinh chỉnh hệ thống";
    if (!_disable_confirm && !_confirmReset(title_str, "Bạn có chắc chắn muốn\nthiết lập lại các tinh chỉnh hệ thống không?"))
        return;
    rename(RESET_CONFIGS_PAK, "/mnt/SDCARD/.tmp_update/temp");
    system("rm -rf /mnt/SDCARD/.tmp_update/config && mkdir -p /mnt/SDCARD/.tmp_update/config");
    system("7z x /mnt/SDCARD/.tmp_update/temp -o/mnt/SDCARD/ -ir!.tmp_update/config/*");
    rename("/mnt/SDCARD/.tmp_update/temp", RESET_CONFIGS_PAK);
    reset_menus = true;
    settings_load();
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetThemeOverrides(void *pt)
{
    const char title_str[] = "Đặt lại ghi đè chủ đề";
    if (!_disable_confirm && !_confirmReset(title_str, "Bạn có chắc chắn muốn\nđặt lại ghi đè chủ đề không?"))
        return;
    system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/theme/*");
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetMainUI(void *pt)
{
    const char title_str[] = "Đặt lại cài đặt MainUI";

    if (!_disable_confirm && !_confirmReset(title_str, "Bạn có chắc chắn muốn\nthiết lập lại cài đặt MainUI không?"))
        return;

    system("rm -f /mnt/SDCARD/system.json");

    char cmd_str[80];
    sprintf(cmd_str, "cp /mnt/SDCARD/.tmp_update/res/miyoo%d_system.json /mnt/SDCARD/system.json", DEVICE_ID);
    system(cmd_str);

    if (DEVICE_ID == MIYOO354) {
        system("rm -f /appconfigs/wpa_supplicant.conf");
        system("cp /mnt/SDCARD/.tmp_update/res/wpa_supplicant.reset /appconfigs/wpa_supplicant.conf");
    }

    reset_menus = true;
    settings_load();
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetRAMain(void *pt)
{
    const char title_str[] = "Đặt lại cấu hình RetroArch";
    if (!_disable_confirm && !_confirmReset(title_str, "Bạn có chắc chắn muốn thiết lập lại cấu hình chính của RetroArch không?"))
        return;
    system("7z x -aoa " RESET_CONFIGS_PAK " -o/mnt/SDCARD/ -ir!RetroArch/*");
    reset_menus = true;
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetRACores(void *pt)
{
    const char title_str[] = "Đặt lại tất cả các lõi RA đã ghi đè";
    if (!_disable_confirm && !_confirmReset(title_str, "Bạn có chắc chắn muốn\nthiết lập lại tất cả lõi RetroArch được ghi đè không?"))
        return;
    system("rm -rf /mnt/SDCARD/Saves/CurrentProfile/config/*");
    system("7z x " RESET_CONFIGS_PAK " -o/mnt/SDCARD/ -ir!Saves/CurrentProfile/config/*");
    reset_menus = true;
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetAdvanceMENU(void *pt)
{
    const char title_str[] = "Đặt lại AdvanceMENU/MAME/MESS";
    if (!_disable_confirm && !_confirmReset(title_str, "Bạn có chắc chắn muốn\nđặt lại AdvanceMENU/MAME/MESS không?"))
        return;
    system("7z x -aoa " RESET_CONFIGS_PAK " -o/mnt/SDCARD/ -ir!BIOS/.advance/*");
    reset_menus = true;
    if (!_disable_confirm)
        _notifyResetDone(title_str);
}

void action_resetAll(void *pt)
{
    const char title_str[] = "Thiết lập lại mọi thứ";
    if (!_confirmReset(title_str, "Bạn có chắc chắn muốn\nthiết lập lại mọi thứ không?"))
        return;
    _disable_confirm = true;
    action_resetTweaks(pt);
    action_resetThemeOverrides(pt);
    action_resetMainUI(pt);
    action_resetRAMain(pt);
    action_resetRACores(pt);
    action_resetAdvanceMENU(pt);
    _disable_confirm = false;
    reset_menus = true;
    settings_load();
    _notifyResetDone(title_str);
}

#endif // TWEAKS_RESET_H__
