#ifndef TWEAKS_MENUS_H__
#define TWEAKS_MENUS_H__

#include <SDL/SDL_image.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "components/kbinput_wrapper.h"
#include "components/list.h"
#include "system/device_model.h"
#include "system/display.h"
#include "utils/apps.h"

#include "./actions.h"
#include "./appstate.h"
#include "./diags.h"
#include "./formatters.h"
#include "./icons.h"
#include "./network.h"
#include "./reset.h"
#include "./tools.h"
#include "./values.h"

void menu_systemStartup(void *_)
{
    if (!_menu_system_startup._created) {
        _menu_system_startup = list_createWithTitle(3, LIST_SMALL, "Khởi động");

        list_addItemWithInfoNote(&_menu_system_startup,
                                 (ListItem){
                                     .label = "Tự động tiếp tục trò chơi cuối cùng",
                                     .item_type = TOGGLE,
                                     .value = (int)settings.startup_auto_resume,
                                     .action = action_setStartupAutoResume},
                                 "Tính năng tự động tiếp tục xảy ra khi\n"
                                 "bạn tắt thiết bị trong khi trò chơi đang\n"
                                 "chạy. Khi khởi động, hệ thống sẽ tiếp\n"
                                 "tục từ nơi bạn dừng lại lần trước.");
        list_addItemWithInfoNote(&_menu_system_startup,
                                 (ListItem){
                                     .label = "Ứng dụng khởi động",
                                     .item_type = MULTIVALUE,
                                     .value_max = 3,
                                     .value_labels = {"MainUI", "GameSwitcher", "RetroArch", "AdvanceMENU"},
                                     .value = settings.startup_application,
                                     .action = action_setStartupApplication},
                                 "Với tùy chọn này, bạn có thể chọn\n"
                                 "giao diện mà bạn muốn khởi chạy khi\n"
                                 "khởi động.");
        list_addItemWithInfoNote(&_menu_system_startup,
                                 (ListItem){
                                     .label = "MainUI: Tab bắt đầu",
                                     .item_type = MULTIVALUE,
                                     .value_max = 5,
                                     .value_formatter = formatter_startupTab,
                                     .value = settings.startup_tab,
                                     .action = action_setStartupTab},
                                 "Tại đây bạn có thể thiết lập tab\n"
                                 "mà bạn muốn khi MainUI khởi chạy.");
    }
    menu_stack[++menu_level] = &_menu_system_startup;
    header_changed = true;
}

void menu_systemDisplay(void *_)
{
    if (!_menu_system_display._created) {
        _menu_system_display = list_createWithTitle(1, LIST_SMALL, "Hiển thị");
    }
    menu_stack[++menu_level] = &_menu_system_display;
    header_changed = true;
}

bool _writeDateString(char *label_out)
{
    char new_label[STR_MAX];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    strftime(new_label, STR_MAX - 1, "Bây giờ: %Y-%m-%d %H:%M:%S", &tm);
    if (strncmp(new_label, label_out, STR_MAX) != 0) {
        strcpy(label_out, new_label);
        return true;
    }
    return false;
}

void menu_datetime(void *_)
{
    if (!_menu_date_time._created) {
        _menu_date_time = list_create(6, LIST_SMALL);
        strcpy(_menu_date_time.title, "Ngày và giờ");
        list_addItem(&_menu_date_time,
                     (ListItem){
                         .label = "[DATESTRING]",
                         .disabled = 1,
                         .action = NULL});

        network_loadState();

        if (DEVICE_ID == MIYOO354 || network_state.ntp) {
            list_addItemWithInfoNote(&_menu_date_time,
                                     (ListItem){
                                         .label = "Tự động đồng bộ qua Internet",
                                         .item_type = TOGGLE,
                                         .value = (int)network_state.ntp,
                                         .action = network_setNtpState},
                                     "Sử dụng kết nối Internet để đồng bộ\n"
                                     "ngày và giờ khi khởi động.");
        }
        if (DEVICE_ID == MIYOO354) {
            list_addItemWithInfoNote(&_menu_date_time,
                                     (ListItem){
                                         .label = "Chờ đồng bộ khi khởi động",
                                         .item_type = TOGGLE,
                                         .disabled = !network_state.ntp,
                                         .value = (int)network_state.ntp_wait,
                                         .action = network_setNtpWaitState},
                                     "Chờ ngày và giờ được đồng bộ\n"
                                     "khi khởi động hệ thống."
                                     " \n"
                                     "Đảm bảo thời gian được đồng bộ\n"
                                     "trước khi trò chơi được khởi chạy.");
            list_addItemWithInfoNote(&_menu_date_time,
                                     (ListItem){
                                         .label = "Nhận múi giờ qua địa chỉ IP",
                                         .item_type = TOGGLE,
                                         .disabled = !network_state.ntp,
                                         .value = !network_state.manual_tz,
                                         .action = network_setTzManualState},
                                     "Nếu tính năng này được bật, hệ thống sẽ\n"
                                     "cố gắng lấy múi giờ từ địa chỉ IP\n"
                                     "của bạn."
                                     " \n"
                                     "Có thể hữu ích khi tắt tùy chọn này\n"
                                     "nếu bạn đang sử dụng VPN.");
            list_addItemWithInfoNote(&_menu_date_time,
                                     (ListItem){
                                         .label = "Chọn múi giờ",
                                         .item_type = MULTIVALUE,
                                         .disabled = !network_state.ntp || !network_state.manual_tz,
                                         .value_max = 48,
                                         .value_formatter = formatter_timezone,
                                         .value = value_timezone(),
                                         .action = network_setTzSelectState},
                                     "Cài đặt múi giờ theo cách thủ công.\n"
                                     "Bạn cũng cần phải điều chỉnh theo DST.");
        }
        list_addItemWithInfoNote(&_menu_date_time,
                                 (ListItem){
                                     .label = "Bỏ qua thời gian mô phỏng",
                                     .item_type = MULTIVALUE,
                                     .disabled = network_state.ntp,
                                     .value_max = 24,
                                     .value_formatter = formatter_timeSkip,
                                     .value = settings.time_skip,
                                     .action = action_setTimeSkip},
                                 "Nếu không có RTC, thời gian của\n"
                                 "hệ thống sẽ đứng yên khi thiết bị tắt.\n"
                                 "Tùy chọn này cho phép bạn thêm\n"
                                 "một lượng giờ cụ thể khi khởi động.");
    }
    _writeDateString(_menu_date_time.items[0].label);
    menu_stack[++menu_level] = &_menu_date_time;
    header_changed = true;
}

void menu_system(void *_)
{
    if (!_menu_system._created) {
        _menu_system = list_createWithTitle(6, LIST_SMALL, "Hệ thống");
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Khởi động...",
                         .action = menu_systemStartup});
        // list_addItem(&_menu_system,
        //              (ListItem){
        //                  .label = "Display...",
        //                  .action = menu_systemDisplay});
        list_addItem(&_menu_system,
                     (ListItem){
                         .label = "Ngày và giờ...",
                         .action = menu_datetime});
        list_addItemWithInfoNote(&_menu_system,
                                 (ListItem){
                                     .label = "Cản báp pin yếu",
                                     .item_type = MULTIVALUE,
                                     .value_max = 5,
                                     .value_formatter = formatter_battWarn,
                                     .value = settings.low_battery_warn_at / 5,
                                     .action = action_setLowBatteryWarnAt},
                                 "Hiển thị cảnh báo biểu tượng pin màu đỏ\n"
                                 "ở góc trên bên phải khi pin ở mức hoặc\n"
                                 "thấp hơn giá trị này.");
        list_addItemWithInfoNote(&_menu_system,
                                 (ListItem){
                                     .label = "Pin yếu: Lưu và Thoát",
                                     .item_type = MULTIVALUE,
                                     .value_max = 5,
                                     .value_formatter = formatter_battExit,
                                     .value = settings.low_battery_autosave_at,
                                     .action = action_setLowBatteryAutoSave},
                                 "Đặt phần trăm pin mà hệ thống sẽ\n"
                                 "lưu và thoát khỏi RetroArch.");
        list_addItemWithInfoNote(&_menu_system,
                                 (ListItem){
                                     .label = "Cường độ rung",
                                     .item_type = MULTIVALUE,
                                     .value_max = 3,
                                     .value_labels = {"Tắt", "Nhẹ", "Thường", "Mạnh"},
                                     .value = settings.vibration,
                                     .action = action_setVibration},
                                 "Cài đặt cường độ rung cho phản hồi\n"
                                 "xúc giác khi nhấn phím tắt hệ thống.");
    }
    menu_stack[++menu_level] = &_menu_system;
    header_changed = true;
}

void menu_buttonActionMainUIMenu(void *_)
{
    if (!_menu_button_action_mainui_menu._created) {
        _menu_button_action_mainui_menu = list_create(3, LIST_SMALL);
        strcpy(_menu_button_action_mainui_menu.title, "MainUI: Nút Menu");
        list_addItemWithInfoNote(&_menu_button_action_mainui_menu,
                                 (ListItem){
                                     .label = "Nhấn một lần",
                                     .item_type = MULTIVALUE,
                                     .value_max = 2,
                                     .value_labels = BUTTON_MAINUI_LABELS,
                                     .value = settings.mainui_single_press,
                                     .action_id = 0,
                                     .action = action_setMenuButtonKeymap},
                                 "Cài đặt hành động nhấn một lần\n"
                                 "vào nút menu khi đang ở MainUI.");
        list_addItemWithInfoNote(&_menu_button_action_mainui_menu,
                                 (ListItem){
                                     .label = "Nhấn giữ",
                                     .item_type = MULTIVALUE,
                                     .value_max = 2,
                                     .value_labels = BUTTON_MAINUI_LABELS,
                                     .value = settings.mainui_long_press,
                                     .action_id = 1,
                                     .action = action_setMenuButtonKeymap},
                                 "Thiết lập hành động Nhấn giữ\n"
                                 "vào nút menu khi đang ở MainUI.");
        list_addItemWithInfoNote(&_menu_button_action_mainui_menu,
                                 (ListItem){
                                     .label = "Nhấn đúp",
                                     .item_type = MULTIVALUE,
                                     .value_max = 2,
                                     .value_labels = BUTTON_MAINUI_LABELS,
                                     .value = settings.mainui_double_press,
                                     .action_id = 2,
                                     .action = action_setMenuButtonKeymap},
                                 "Thiết lập hành động Nhấn đúp\n"
                                 "vào nút menu khi đang ở MainUI.");
    }
    menu_stack[++menu_level] = &_menu_button_action_mainui_menu;
    header_changed = true;
}

void menu_buttonActionInGameMenu(void *_)
{
    if (!_menu_button_action_ingame_menu._created) {
        _menu_button_action_ingame_menu = list_createWithTitle(3, LIST_SMALL, "Trong trò chơi: Nút Menu");
        list_addItemWithInfoNote(&_menu_button_action_ingame_menu,
                                 (ListItem){
                                     .label = "Nhấn một lần",
                                     .item_type = MULTIVALUE,
                                     .value_max = 3,
                                     .value_labels = BUTTON_INGAME_LABELS,
                                     .value = settings.ingame_single_press,
                                     .action_id = 3,
                                     .action = action_setMenuButtonKeymap},
                                 "Cài đặt hành động nhấn một lần\n"
                                 "vào nút menu khi đang trong trò chơi.");
        list_addItemWithInfoNote(&_menu_button_action_ingame_menu,
                                 (ListItem){
                                     .label = "Nhấn giữ",
                                     .item_type = MULTIVALUE,
                                     .value_max = 3,
                                     .value_labels = BUTTON_INGAME_LABELS,
                                     .value = settings.ingame_long_press,
                                     .action_id = 4,
                                     .action = action_setMenuButtonKeymap},
                                 "Cài đặt hành động nhấn giữ\n"
                                 "vào nút menu khi đang trong trò chơi.");
        list_addItemWithInfoNote(&_menu_button_action_ingame_menu,
                                 (ListItem){
                                     .label = "Nhấn đúp",
                                     .item_type = MULTIVALUE,
                                     .value_max = 3,
                                     .value_labels = BUTTON_INGAME_LABELS,
                                     .value = settings.ingame_double_press,
                                     .action_id = 5,
                                     .action = action_setMenuButtonKeymap},
                                 "Cài đặt hành động nhấn đúp\n"
                                 "vào nút menu khi đang trong trò chơi.");
    }
    menu_stack[++menu_level] = &_menu_button_action_ingame_menu;
    header_changed = true;
}

void menu_buttonAction(void *_)
{
    if (!_menu_button_action._created) {
        _menu_button_action = list_createWithTitle(6, LIST_SMALL, "Nút phím tắt");
        list_addItemWithInfoNote(&_menu_button_action,
                                 (ListItem){
                                     .label = "Rung khi nhấn nút Menu",
                                     .item_type = TOGGLE,
                                     .value = (int)settings.menu_button_haptics,
                                     .action = action_setMenuButtonHaptics},
                                 "Bật phản hồi xúc giác khi nhấn một\n"
                                 "lần và nhấn đúp vào nút menu.");
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "Trong trò chơi: Nút Menu...",
                         .action = menu_buttonActionInGameMenu});
        list_addItem(&_menu_button_action,
                     (ListItem){
                         .label = "MainUI: Nút Menu...",
                         .action = menu_buttonActionMainUIMenu});

        getInstalledApps(true);
        list_addItemWithInfoNote(&_menu_button_action,
                                 (ListItem){
                                     .label = "MainUI: Nút X",
                                     .item_type = MULTIVALUE,
                                     .value_max = installed_apps_count + NUM_TOOLS,
                                     .value = value_appShortcut(0),
                                     .value_formatter = formatter_appShortcut,
                                     .action_id = 0,
                                     .action = action_setAppShortcut},
                                 "Thiết lập hành động của nút X trong MainUI.");
        list_addItemWithInfoNote(&_menu_button_action,
                                 (ListItem){
                                     .label = "MainUI: Nút Y",
                                     .item_type = MULTIVALUE,
                                     .value_max = installed_apps_count + NUM_TOOLS + 1,
                                     .value = value_appShortcut(1),
                                     .value_formatter = formatter_appShortcut,
                                     .action_id = 1,
                                     .action = action_setAppShortcut},
                                 "Thiết lập hành động của nút Y trong MainUI.");
        list_addItemWithInfoNote(&_menu_button_action,
                                 (ListItem){
                                     .label = "Nhấn một lần nút nguồn",
                                     .item_type = MULTIVALUE,
                                     .value_max = 1,
                                     .value_labels = {"Chế độ chờ", "Tắt máy"},
                                     .value = (int)settings.disable_standby,
                                     .action = action_setDisableStandby},
                                 "Thay đổi hành động nhấn một lần nút nguồn\n"
                                 "thành 'Chế độ chờ' hoặc 'Tắt máy'.");
    }
    menu_stack[++menu_level] = &_menu_button_action;
    header_changed = true;
}

void menu_batteryPercentage(void *_)
{
    if (!_menu_battery_percentage._created) {
        _menu_battery_percentage = list_createWithTitle(7, LIST_SMALL, "Phần trăm pin");
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Hiện",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = THEME_TOGGLE_LABELS,
                         .value = value_batteryPercentageVisible(),
                         .action = action_batteryPercentageVisible});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Phông chữ",
                         .item_type = MULTIVALUE,
                         .value_max = num_font_families,
                         .value_formatter = formatter_fontFamily,
                         .value = value_batteryPercentageFontFamily(),
                         .action = action_batteryPercentageFontFamily});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Cỡ chữ",
                         .item_type = MULTIVALUE,
                         .value_max = num_font_sizes,
                         .value_formatter = formatter_fontSize,
                         .value = value_batteryPercentageFontSize(),
                         .action = action_batteryPercentageFontSize});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Căn chỉnh chữ",
                         .item_type = MULTIVALUE,
                         .value_max = 3,
                         .value_labels = {"-", "Trái", "Giữa", "Phải"},
                         .value = value_batteryPercentagePosition(),
                         .action = action_batteryPercentagePosition});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Vị trí cố định",
                         .item_type = MULTIVALUE,
                         .value_max = 2,
                         .value_labels = THEME_TOGGLE_LABELS,
                         .value = value_batteryPercentageFixed(),
                         .action = action_batteryPercentageFixed});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Độ lệch ngang",
                         .item_type = MULTIVALUE,
                         .value_max = BATTPERC_MAX_OFFSET * 2 + 1,
                         .value_formatter = formatter_positionOffset,
                         .value = value_batteryPercentageOffsetX(),
                         .action = action_batteryPercentageOffsetX});
        list_addItem(&_menu_battery_percentage,
                     (ListItem){
                         .label = "Độ lệch dọc",
                         .item_type = MULTIVALUE,
                         .value_max = BATTPERC_MAX_OFFSET * 2 + 1,
                         .value_formatter = formatter_positionOffset,
                         .value = value_batteryPercentageOffsetY(),
                         .action = action_batteryPercentageOffsetY});
    }
    menu_stack[++menu_level] = &_menu_battery_percentage;
    header_changed = true;
}

void menu_themeOverrides(void *_)
{
    if (!_menu_theme_overrides._created) {
        _menu_theme_overrides = list_create(7, LIST_SMALL);
        strcpy(_menu_theme_overrides.title, "Ghi đè chủ đề");
        list_addItem(&_menu_theme_overrides,
                     (ListItem){
                         .label = "Phần trăm pin...",
                         .action = menu_batteryPercentage});
        list_addItemWithInfoNote(&_menu_theme_overrides,
                                 (ListItem){
                                     .label = "Tắt nhạc nền",
                                     .item_type = TOGGLE,
                                     .value = settings.bgm_mute,
                                     .action = action_toggleBackgroundMusic},
                                 "Tắt tiếng nhạc nền cho chủ đề");
        list_addItemWithInfoNote(&_menu_theme_overrides,
                                 (ListItem){
                                     .label = "Ẩn nhãn biểu tượng",
                                     .item_type = MULTIVALUE,
                                     .value_max = 2,
                                     .value_labels = THEME_TOGGLE_LABELS,
                                     .value = value_hideLabelsIcons(),
                                     .action = action_hideLabelsIcons},
                                 "Ẩn nhãn bên dưới các biểu tượng menu chính.");
        list_addItemWithInfoNote(&_menu_theme_overrides,
                                 (ListItem){
                                     .label = "Ẩn nhãn gợi ý",
                                     .item_type = MULTIVALUE,
                                     .value_max = 2,
                                     .value_labels = THEME_TOGGLE_LABELS,
                                     .value = value_hideLabelsHints(),
                                     .action = action_hideLabelsHints},
                                 "Ẩn các nhãn ở dưới màn hình.");
        // list_addItem(&_menu_theme_overrides, (ListItem){
        // 	.label = "[Title] Font size", .item_type = MULTIVALUE,
        // .value_max = num_font_sizes, .value_formatter = formatter_fontSize
        // });
        // list_addItem(&_menu_theme_overrides, (ListItem){
        // 	.label = "[List] Font size", .item_type = MULTIVALUE, .value_max
        // = num_font_sizes, .value_formatter = formatter_fontSize
        // });
        // list_addItem(&_menu_theme_overrides, (ListItem){
        // 	.label = "[Hint] Font size", .item_type = MULTIVALUE, .value_max
        // = num_font_sizes, .value_formatter = formatter_fontSize
        // });
    }
    menu_stack[++menu_level] = &_menu_theme_overrides;
    header_changed = true;
}

void menu_blueLight(void *_)
{
    if (!_menu_user_blue_light._created) {
        network_loadState();
        _menu_user_blue_light = list_createWithTitle(6, LIST_SMALL, "Bộ lọc ánh sáng xanh");
        if (DEVICE_ID == MIYOO354) {
            list_addItem(&_menu_user_blue_light,
                         (ListItem){
                             .label = "[DATESTRING]",
                             .disabled = 1,
                             .action = NULL});
        }
        list_addItemWithInfoNote(&_menu_user_blue_light,
                                 (ListItem){
                                     .label = "Trạng thái",
                                     .disable_arrows = blf_changing,
                                     .disable_a_btn = blf_changing,
                                     .item_type = TOGGLE,
                                     .value = (int)settings.blue_light_state || exists("/tmp/.blfOn"),
                                     .action = action_blueLight},
                                 "Đặt cường độ đã chọn ngay bây giờ\n");
        if (DEVICE_ID == MIYOO354) {
            list_addItemWithInfoNote(&_menu_user_blue_light,
                                     (ListItem){
                                         .label = "",
                                         .disabled = !network_state.ntp,
                                         .item_type = TOGGLE,
                                         .value = (int)settings.blue_light_schedule,
                                         .action = action_blueLightSchedule},
                                     "Bật hoặc tắt lịch trình lọc ánh sáng xanh\n");
        }
        list_addItemWithInfoNote(&_menu_user_blue_light,
                                 (ListItem){
                                     .label = "Độ mạnh",
                                     .item_type = MULTIVALUE,
                                     .disable_arrows = blf_changing,
                                     .disable_a_btn = blf_changing,
                                     .value_max = 4,
                                     .value_labels = BLUELIGHT_LABELS,
                                     .action = action_blueLightLevel,
                                     .value = value_blueLightLevel()},
                                 "Thay đổi cường độ của bộ lọc\n"
                                 "ánh sáng xanh");
        if (DEVICE_ID == MIYOO354) {
            list_addItemWithInfoNote(&_menu_user_blue_light,
                                     (ListItem){
                                         .label = "Giờ (Bật)",
                                         .disabled = !network_state.ntp,
                                         .item_type = MULTIVALUE,
                                         .value_max = 95,
                                         .value_formatter = formatter_Time,
                                         .action = action_blueLightTimeOn,
                                         .value = value_blueLightTimeOn()},
                                     "Lịch trình thời gian bật bộ lọc ánh sáng xanh");
            list_addItemWithInfoNote(&_menu_user_blue_light,
                                     (ListItem){
                                         .label = "Giờ (Tắt)",
                                         .disabled = !network_state.ntp,
                                         .item_type = MULTIVALUE,
                                         .value_max = 95,
                                         .value_formatter = formatter_Time,
                                         .action = action_blueLightTimeOff,
                                         .value = value_blueLightTimeOff()},
                                     "Lịch trình thời gian tắt bộ lọc ánh sáng xanh");
        }
    }
    if (DEVICE_ID == MIYOO354) {
        _writeDateString(_menu_user_blue_light.items[0].label);
        char scheduleToggleLabel[100];
        strcpy(scheduleToggleLabel, exists("/tmp/.blfIgnoreSchedule") ? "Lịch trình (bỏ qua)" : "Lịch trình");
        strcpy(_menu_user_blue_light.items[2].label, scheduleToggleLabel);
    }
    menu_stack[++menu_level] = &_menu_user_blue_light;
    header_changed = true;
}

void menu_userInterface(void *_)
{
    settings.blue_light_state = config_flag_get(".blfOn");
    all_changed = true;
    if (!_menu_user_interface._created) {
        _menu_user_interface = list_createWithTitle(6, LIST_SMALL, "Giao diện");
        list_addItemWithInfoNote(&_menu_user_interface,
                                 (ListItem){
                                     .label = "Hiển thị gần đây",
                                     .item_type = TOGGLE,
                                     .value = settings.show_recents,
                                     .action = action_setShowRecents},
                                 "Chuyển đổi chế độ hiển thị của tab gần đây\n"
                                 "trong menu chính.");
        list_addItemWithInfoNote(&_menu_user_interface,
                                 (ListItem){
                                     .label = "Hiển thị chế độ chuyên gia",
                                     .item_type = TOGGLE,
                                     .value = settings.show_expert,
                                     .action = action_setShowExpert},
                                 "Chuyển đổi chế độ hiển thị của tab chuyên gia\n"
                                 "trong menu chính.");
        display_init();
        list_addItemWithInfoNote(&_menu_user_interface,
                                 (ListItem){
                                     .label = "Kích thước thanh OSD",
                                     .item_type = MULTIVALUE,
                                     .value_max = 15,
                                     .value_formatter = formatter_meterWidth,
                                     .value = value_meterWidth(),
                                     .action = action_meterWidth},
                                 "Thiết lập chiều rộng của 'thanh OSD'\n"
                                 "hiển thị ở phía bên trái màn hình khi\n"
                                 "điều chỉnh độ sáng hoặc âm lượng (MMP).");
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Bộ lọc ánh sáng xanh...",
                         .action = menu_blueLight});
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Ghi đè chủ đề...",
                         .action = menu_themeOverrides});
        list_addItem(&_menu_user_interface,
                     (ListItem){
                         .label = "Gói biểu tượng...",
                         .action = menu_icons});
    }
    menu_stack[++menu_level] = &_menu_user_interface;
    header_changed = true;
}

void menu_resetSettings(void *_)
{
    if (!_menu_reset_settings._created) {
        _menu_reset_settings = list_createWithTitle(7, LIST_SMALL, "Thiết lập lại cài đặt");
        list_addItemWithInfoNote(&_menu_reset_settings,
                                 (ListItem){
                                     .label = "Thiết lập lại các tinh chỉnh hệ thống",
                                     .action = action_resetTweaks},
                                 "Thiết lập lại tất cả các tùy chỉnh của\n"
                                 "hệ thống Onion, bao gồm cả thiết lập mạng.");
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Đặt lại ghi đè chủ đề",
                         .action = action_resetThemeOverrides});
        list_addItemWithInfoNote(&_menu_reset_settings,
                                 (ListItem){
                                     .label = "Đặt lại cài đặt MainUI",
                                     .action = action_resetMainUI},
                                 "Đặt lại các cài đặt được lưu trữ trên thiết bị,\n"
                                 "chẳng hạn như chủ đề, tùy chọn hiển thị và\n"
                                 "âm lượng. Cũng đặt lại cấu hình WiFi.");
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Đặt lại cấu hình chính của RetroArch",
                         .action = action_resetRAMain});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Đặt lại tất cả các ghi đè lõi RetroArch",
                         .action = action_resetRACores});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Đặt lại AdvanceMENU/MAME/MESS",
                         .action = action_resetAdvanceMENU});
        list_addItem(&_menu_reset_settings,
                     (ListItem){
                         .label = "Thiết lập lại mọi thứ", .action = action_resetAll});
    }
    menu_stack[++menu_level] = &_menu_reset_settings;
    header_changed = true;
}

void menu_diagnostics(void *pt)
{
    if (!_menu_diagnostics._created) {
        diags_getEntries();

        _menu_diagnostics = list_createWithSticky(1 + diags_numScripts, "Chẩn đoán");
        list_addItemWithInfoNote(&_menu_diagnostics,
                                 (ListItem){
                                     .label = "Cho phép ghi nhật ký",
                                     .sticky_note = "Cho phép ghi nhật ký hệ thống",
                                     .item_type = TOGGLE,
                                     .value = (int)settings.enable_logging,
                                     .action = action_setEnableLogging},
                                 "Bật ghi nhật ký,\n"
                                 "cho hệ thống và mạng.\n \n"
                                 "Nhật ký sẽ được tạo trong\n"
                                 "SD: /.tmp_update/logs.");
        for (int i = 0; i < diags_numScripts; i++) {
            ListItem diagItem = {
                .label = "",
                .payload_ptr = &scripts[i].filename,
                .action = action_runDiagnosticScript,
            };

            const char *prefix = "";
            if (strncmp(scripts[i].filename, "util", 4) == 0) {
                prefix = "Util: ";
            }
            else if (strncmp(scripts[i].filename, "fix", 3) == 0) {
                prefix = "Fix: ";
            }

            snprintf(diagItem.label, DIAG_MAX_LABEL_LENGTH - 1, "%s%.62s", prefix, scripts[i].label);
            strncpy(diagItem.sticky_note, "Nhàn rỗi: Tập lệnh đã chọn không chạy", STR_MAX - 1);

            char *parsed_Tooltip = diags_parseNewLines(scripts[i].tooltip);
            list_addItemWithInfoNote(&_menu_diagnostics, diagItem, parsed_Tooltip);
            free(parsed_Tooltip);
        }
    }

    menu_stack[++menu_level] = &_menu_diagnostics;
    header_changed = true;
}

void menu_advanced(void *_)
{
    if (!_menu_advanced._created) {
        _menu_advanced = list_createWithTitle(7, LIST_SMALL, "Nâng cao");
        list_addItemWithInfoNote(&_menu_advanced,
                                 (ListItem){
                                     .label = "Hoán đổi các nút (L<>L2, R<>R2)",
                                     .item_type = TOGGLE,
                                     .value = value_getSwapTriggers(),
                                     .action = action_advancedSetSwapTriggers},
                                 "Hoán đổi chức năng của L<>L2 và R<>R2\n"
                                 "(chỉ ảnh hưởng đến các hành động trong trò chơi).");
        if (DEVICE_ID == MIYOO283) {
            list_addItemWithInfoNote(&_menu_advanced,
                                     (ListItem){
                                         .label = "Chỉnh độ sáng",
                                         .item_type = MULTIVALUE,
                                         .value_max = 1,
                                         .value_labels = {"SELECT+R2/L2",
                                                          "MENU+UP/DOWN"},
                                         .value = config_flag_get(".altBrightness"),
                                         .action = action_setAltBrightness},
                                     "Thay đổi phím tắt để tăng/giảm độ sáng.");
        }
        list_addItemWithInfoNote(&_menu_advanced,
                                 (ListItem){
                                     .label = "Tốc độ tua nhanh",
                                     .item_type = MULTIVALUE,
                                     .value_max = 50,
                                     .value = value_getFrameThrottle(),
                                     .value_formatter = formatter_fastForward,
                                     .action = action_advancedSetFrameThrottle},
                                 "Đặt tốc độ tua nhanh tối đa.");
        list_addItemWithInfoNote(&_menu_advanced,
                                 (ListItem){
                                     .label = "Tần số PWM",
                                     .item_type = MULTIVALUE,
                                     .value_max = 9,
                                     .value_labels = PWM_FREQUENCIES,
                                     .value = value_getPWMFrequency(),
                                     .action = action_advancedSetPWMFreqency},
                                 "Thay đổi tần số PWM\n"
                                 "Giá trị thấp hơn để giảm nhiễu\n"
                                 "Tính năng thử nghiệm");
        if (DEVICE_ID == MIYOO354) {
            list_addItemWithInfoNote(&_menu_advanced,
                                     (ListItem){
                                         .label = "LCD hạ thế",
                                         .item_type = MULTIVALUE,
                                         .value_max = 4,
                                         .value_labels = {"Tắt", "-0.1V", "-0.2V", "-0.3V", "-0.4V"},
                                         .value = value_getLcdVoltage(),
                                         .action = action_advancedSetLcdVoltage},
                                     "Sử dụng tùy chọn này nếu bạn thấy\n"
                                     "có hiện tượng nhiễu nhỏ trên màn hình.");
        }
        if (exists(RESET_CONFIGS_PAK)) {
            list_addItem(&_menu_advanced,
                         (ListItem){
                             .label = "Thiết lập lại cài đặt...",
                             .action = menu_resetSettings});
        }
        list_addItem(&_menu_advanced,
                     (ListItem){
                         .label = "Chẩn đoán...",
                         .action = menu_diagnostics});
    }
    menu_stack[++menu_level] = &_menu_advanced;
    header_changed = true;
}

void menu_screen_recorder(void *pt)
{
    if (!_menu_screen_recorder._created) {
        _menu_screen_recorder = list_createWithSticky(7, "Thiết lập trình ghi màn hình");
        list_addItemWithInfoNote(&_menu_screen_recorder,
                                 (ListItem){
                                     .label = "Bắt đầu/dừng ghi",
                                     .sticky_note = "Trạng thái:...",
                                     .action = tool_screenRecorder},
                                 "Bắt đầu hoặc dừng ghi màn hình");
        list_addItemWithInfoNote(&_menu_screen_recorder,
                                 (ListItem){
                                     .label = "Đếm ngược (giây)",
                                     .sticky_note = "Chỉ định đếm ngược",
                                     .item_type = MULTIVALUE,
                                     .value_max = 10,
                                     .value = (int)settings.rec_countdown,
                                     .action = action_toggleScreenRecCountdown},
                                 "Đếm ngược khi bắt đầu ghi.\n\n"
                                 "Màn hình sẽ nhấp nháy màu trắng n lần\n"
                                 "để báo hiệu quá trình ghi đã bắt đầu/dừng");
        list_addItemWithInfoNote(&_menu_screen_recorder,
                                 (ListItem){
                                     .label = "Biểu tượng cảnh báo",
                                     .sticky_note = "Bật/tắt Biểu tượng cảnh báo",
                                     .item_type = TOGGLE,
                                     .value = (int)settings.rec_indicator,
                                     .action = action_toggleScreenRecIndicator},
                                 "Bật/tắt hiển thị biểu tượng\n"
                                 "nhấp nháy để nhắc nhở bạn\n"
                                 "rằng bạn vẫn đang ghi màn hình.");
        list_addItemWithInfoNote(&_menu_screen_recorder,
                                 (ListItem){
                                     .label = "Chuyển đổi phím nóng",
                                     .sticky_note = "Bật/tắt phím nóng (Menu+A)",
                                     .item_type = TOGGLE,
                                     .value = (int)settings.rec_hotkey,
                                     .action = action_toggleScreenRecHotkey},
                                 "Bật chức năng phím nóng.\n\n"
                                 "Có thể bắt đầu/dừng ghi hình\n"
                                 "bằng Menu+A");
        list_addItemWithInfoNote(&_menu_screen_recorder,
                                 (ListItem){
                                     .label = "Đặt lại trình ghi màn hình",
                                     .sticky_note = "Tắt hẳn ffmpeg nếu nó bị lỗi",
                                     .action = action_hardKillFFmpeg},
                                 "Thực hiện lệnh tắt ffmpeg.\n\n"
                                 "CẢNH BÁO: Nếu bạn đang ghi hình,\n"
                                 "bạn có thể bị mất tệp!");
        list_addItemWithInfoNote(&_menu_screen_recorder,
                                 (ListItem){
                                     .label = "Xóa tất cả bản ghi hình",
                                     .sticky_note = "Xóa tất cả bản ghi hình",
                                     .action = action_deleteAllRecordings},
                                 "Xóa tất cả video đã ghi.\n\n"
                                 "CẢNH BÁO: Không thể hoàn tác\n"
                                 "hành động này!");
    }

    int isRecordingActive = exists("/tmp/recorder_active");
    const char *recordingStatus = isRecordingActive ? "Trạng thái: Đang ghi..." : "Trạng thái: Rảnh.";
    strncpy(_menu_screen_recorder.items[0].sticky_note, recordingStatus, sizeof(_menu_screen_recorder.items[0].sticky_note) - 1);
    _menu_screen_recorder.items[0].sticky_note[sizeof(_menu_screen_recorder.items[0].sticky_note) - 1] = '\0';
    menu_stack[++menu_level] = &_menu_screen_recorder;
    header_changed = true;
}

void menu_tools_m3uGenerator(void *_)
{
    if (!_menu_tools_m3uGenerator._created) {
        _menu_tools_m3uGenerator = list_createWithTitle(2, LIST_SMALL, "Tạo tệp M3U");
        list_addItemWithInfoNote(&_menu_tools_m3uGenerator,
                                 (ListItem){
                                     .label = "Nhiều thư mục (.Game_Name)",
                                     .action = tool_generateM3uFiles_md},
                                 "Một thư mục cho mỗi trò chơi \".Game_Name\"");
        list_addItemWithInfoNote(&_menu_tools_m3uGenerator,
                                 (ListItem){
                                     .label = "Thư mục đơn (.multi-disc)",
                                     .action = tool_generateM3uFiles_sd},
                                 "Một thư mục duy nhất (\".multi-disc\")\n"
                                 "sẽ chứa tất cả tập tin của game nhiều đĩa");
    }
    menu_stack[++menu_level] = &_menu_tools_m3uGenerator;
    header_changed = true;
}

void menu_tools(void *_)
{
    if (!_menu_tools._created) {
        _menu_tools = list_create(NUM_TOOLS, LIST_SMALL);
        strcpy(_menu_tools.title, "Tools");
        list_addItemWithInfoNote(&_menu_tools,
                                 (ListItem){
                                     .label = "Tạo tệp CUE cho trò chơi BIN",
                                     .action = tool_generateCueFiles},
                                 "Rom PSX ở định dạng '.bin'\n"
                                 "cần tệp '.cue' phù hợp.\n"
                                 "Sử dụng công cụ này để tự động tạo chúng.");
        list_addItemWithInfoNote(&_menu_tools,
                                 (ListItem){
                                     .label = "Tạo tệp M3U cho trò chơi nhiều đĩa",
                                     .action = menu_tools_m3uGenerator},
                                 "Rom nhiều đĩa PSX yêu cầu tạo một tệp\n"
                                 "danh sách phát (.m3u). Nó cho phép chỉ\n"
                                 "có một mục nhập cho mỗi trò chơi nhiều đĩa\n"
                                 "và một tệp lưu duy nhất cho mỗi trò chơi");
        list_addItemWithInfoNote(&_menu_tools,
                                 (ListItem){
                                     .label = "Tạo danh sách trò chơi cho các rom tên ngắn",
                                     .action = tool_buildShortRomGameList},
                                 "Công cụ này thay thế tên viết tắt trong\n"
                                 "bộ nhớ đệm trò chơi bằng tên thật tương đương.\n"
                                 "Điều này đảm bảo danh sách được\n"
                                 "sắp xếp đúng.");
        list_addItemWithInfoNote(&_menu_tools,
                                 (ListItem){
                                     .label = "Tạo miyoogamelist với tên tóm tắt",
                                     .action = tool_generateMiyoogamelists},
                                 "Sử dụng công cụ này để xóa têntrò chơi\n"
                                 "của bạn mà không cần phải đổi tên tệp rom\n"
                                 "(xóa dấu ngoặc đơn, thứ hạng và nhiều thứ khác).\n"
                                 "Công cụ này sẽ tạo ra tệp 'miyoogamelist.xml'\n"
                                 "đi kèm với một số hạn chế,\n"
                                 "chẳng hạn như không hỗ trợ thư mục con.");
        list_addItem(&_menu_tools,
                     (ListItem){
                         .label = "Trình ghi màn hình...",
                         .action = menu_screen_recorder});
        list_addItemWithInfoNote(&_menu_tools,
                                 (ListItem){
                                     .label = "Sắp xếp danh sách ứng dụng A-Z",
                                     .action = tool_sortAppsAZ},
                                 "Sử dụng công cụ này để sắp xếp danh sách\n"
                                 "Ứng dụng của bạn theo thứ tự tăng dần từ A đến Z.\n");
        list_addItemWithInfoNote(&_menu_tools,
                                 (ListItem){
                                     .label = "Sắp xếp danh sách ứng dụng Z-A",
                                     .action = tool_sortAppsZA},
                                 "Sử dụng công cụ này để sắp xếp danh sách\n"
                                 "Ứng dụng của bạn theo thứ tự giảm dần từ Z đến A.\n");
    }
    menu_stack[++menu_level] = &_menu_tools;
    header_changed = true;
}

void *_get_menu_icon(const char *name)
{
    char path[STR_MAX * 2] = {0};
    const char *config_path = "/mnt/SDCARD/App/Tweaks/config.json";

    if (is_file(config_path)) {
        cJSON *config = json_load(config_path);
        char icon_path[STR_MAX];
        if (json_getString(config, "icon", icon_path))
            snprintf(path, STR_MAX * 2 - 1, "%s/%s.png", dirname(icon_path),
                     name);
    }

    if (!is_file(path))
        snprintf(path, STR_MAX * 2 - 1, "res/%s.png", name);

    return (void *)IMG_Load(path);
}

void menu_main(void)
{
    if (!_menu_main._created) {
        _menu_main = list_createWithTitle(6, LIST_LARGE, "Điều chỉnh");
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Hệ thống",
                         .description = "Khởi động, lưu & thoát, rung",
                         .action = menu_system,
                         .icon_ptr = _get_menu_icon("tweaks_system")});
        if (DEVICE_ID == MIYOO354) {
            list_addItem(&_menu_main,
                         (ListItem){
                             .label = "Mạng",
                             .description = "Cài đặt mạng",
                             .action = menu_network,
                             .icon_ptr = _get_menu_icon("tweaks_network")});
        }
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Phím tắt",
                         .description = "Tuỳ chỉnh hành động của các nút",
                         .action = menu_buttonAction,
                         .icon_ptr = _get_menu_icon("tweaks_menu_button")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Giao diện",
                         .description = "Hiển thị menu, ghi đè chủ đề",
                         .action = menu_userInterface,
                         .icon_ptr = _get_menu_icon("tweaks_user_interface")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Nâng cao",
                         .description = "Điều chỉnh trình giả lập, thiết lập lại cài đặt",
                         .action = menu_advanced,
                         .icon_ptr = _get_menu_icon("tweaks_advanced")});
        list_addItem(&_menu_main,
                     (ListItem){
                         .label = "Công cụ",
                         .description = "Tệp tin yêu thích, dọn dẹp",
                         .action = menu_tools,
                         .icon_ptr = _get_menu_icon("tweaks_tools")});
    }
    menu_level = 0;
    menu_stack[0] = &_menu_main;
    header_changed = true;
}

void menu_resetAll(void)
{
    int current_state[10][2];
    int current_level = menu_level;
    for (int i = 0; i <= current_level; i++) {
        current_state[i][0] = menu_stack[i]->active_pos;
        current_state[i][1] = menu_stack[i]->scroll_pos;
    }
    menu_free_all();
    menu_main();
    for (int i = 0; i <= current_level; i++) {
        menu_stack[i]->active_pos = current_state[i][0];
        menu_stack[i]->scroll_pos = current_state[i][1];
        if (i < current_level)
            list_activateItem(menu_stack[i]);
    }
    reset_menus = false;
}

#endif
