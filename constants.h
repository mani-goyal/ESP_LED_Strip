#define IS_KEY_PRESENT(mapToCheck, key) (mapToCheck.containsKey(key))

enum OpType{
    CHANGE_BRIGHTNESS = 1,
    CHANGE_LED_CONFIG,
    CHANGE_LED_COLOR,
    CHANGE_WIFI_SETTINGS,

};