
#pragma once
#include "shared.h"
#include "menus.h"
#include <Arduino.h>

enum MenuMode
{

    MENU_MODE_MENU_CHOOSER,
    MENU_MODE_EDIT_MODE,
};

class MenuManager
{
private:
    MenuID currentMenu;
    MenuMode menuMode;
    int selectedMenu;
    int textOffset = 0;
    bool verticalAlignment = true; // Set this flag to true for vertical alignment, false for horizontal
public:
    MenuManager()
    {
        currentMenu = MENU_ROOT;
        menuMode = MENU_MODE_MENU_CHOOSER;
        selectedMenu = 0;
        Serial.println("MenuManager initialized");
    }
    bool handleSensorMessage(sensor_message message);
    bool handleTextMessage(std::string message)
    {
        if (message.find("confirmMenu") == 0)
        {
            confirmMenus();
            return true;
        }
        return false;
    }
    std::vector<MenuID> getChildrenOfMenu(MenuID type);
    std::string getMenuName(MenuID type, int MaxSize = 6);

    std::string getMenuPath(MenuID type, MenuID root);
    MenuID getParentMenu(MenuID type);

    std::vector<ParameterID> getParametersForMenu(MenuID type);
    int numMenus();
    void confirmMenus();
    void updateMenu();
};